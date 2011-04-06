/*
 *  php-topy      Php binding for Topy client function
 *  Copyright (c) 2008 Vion Nicolas <nico@picapo.net>
 *
 *  Topy is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Topy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Topy; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_topy.h"

static function_entry topy_functions[] = {
	PHP_FE(topy_connect, NULL)
	PHP_FE(topy_pconnect, NULL)
	PHP_FE(topy_reconnect, NULL)
	PHP_FE(topy_info, NULL)
	PHP_FE(topy_error, NULL)
	PHP_FE(topy_close, NULL)
	PHP_FE(topy_query, NULL)
	PHP_FE(topy_query_send, NULL)
	PHP_FE(topy_query_read, NULL)
	PHP_FE(topy_query_wait, NULL)

	{NULL, NULL, NULL}
};

zend_module_entry topy_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_TOPY_EXTNAME,
	topy_functions,
	PHP_MINIT(topy),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(topy),
	PHP_TOPY_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_TOPY
ZEND_GET_MODULE(topy)
#endif

#define STR(s) #s
#define XSTR(s) STR(s)

PHP_MINFO_FUNCTION(topy) {
	php_info_print_table_start();
	php_info_print_table_row(2, "Topy support", "enabled");
	php_info_print_table_row(2, "Topy PHP extension version", PHP_TOPY_VERSION);
	php_info_print_table_row(2, "Topy default timeout", XSTR(TOPY_TIMEOUT));
	php_info_print_table_end();
}

//-------- Socket operations ---------//
int socket_open(char const *host, char const *port, long int const timeout_sec) {
	struct addrinfo hints, *res;

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_addr = NULL;

	//fetch host/port
	int fodder, fd;
	if ((fodder = getaddrinfo(host, port, &hints, &res)) != 0) {
		return -1;
	}

	//create socket
	if ((fd = socket(res->ai_family, hints.ai_socktype, hints.ai_protocol)) == -1) {
		freeaddrinfo(res);
		return -1;
	}

	//set socket options
	struct timeval timeout;
	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
	if (
		(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) != 0) || 
		(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) != 0)
	) {
		freeaddrinfo(res);
		close(fd);
		return -1;
	}

	//bind
	if (connect(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		freeaddrinfo(res);
		close(fd);
		return -1;
	}

	freeaddrinfo(res);
	return fd;
}

//-------- connection ---------//
topy_connection_t *connection_new(int persist, int key_len, char *key, int socket, char *host, char *port, long int timeout) {
//	php_printf("NEW %i\n", persist);

	topy_connection_t *connection;
	connection = pemalloc(sizeof(topy_connection_t), persist);
	connection->persist = persist;
	connection->key = (key) ? pestrdup(key, persist) : NULL;
	connection->key_len = key_len;
	connection->socket = socket;
	connection->tid = 0;
	connection->host = pestrdup(host, persist);
	connection->port = pestrdup(port, persist);
	connection->timeout = timeout;
	connection->error_code = 0;
	connection->error_msg = NULL;

	return connection;
}

void connection_free(topy_connection_t *connection) {
//	php_printf("FREE %i\n", connection->persist);

	int persist = connection->persist;

	if (connection->key) pefree(connection->key, persist);
	if (connection->error_msg) pefree(connection->error_msg, persist);
	if (connection->host) pefree(connection->host, persist);
	if (connection->port) pefree(connection->port, persist);
	if (connection->socket != -1) close(connection->socket);
	pefree(connection, persist);
}

void connection_error_set(topy_connection_t *connection, int const code, char const *msg) {
	connection->error_code = code;
	if (connection->error_msg) pefree(connection->error_msg, connection->persist);
	connection->error_msg = (msg != NULL) ? pestrdup(msg, connection->persist) : NULL;
}

void connection_error_clear(topy_connection_t *connection) {
	connection_error_set(connection, -1, NULL);
}

void connection_error_ok(topy_connection_t *connection) {
	connection_error_set(connection, 0, NULL);
}

void connection_close(topy_connection_t *connection) {
	if (connection->socket != -1) 
		return;

//	php_printf("CLOSE\n");
	close(connection->socket);
	connection->socket = -1;
}

int connection_reconnect(topy_connection_t *connection) {
//	php_printf("RECONNECT\n");

	connection_close(connection);
	connection->socket = socket_open(connection->host, connection->port, connection->timeout);
	return connection->socket;
}

void connection_clear(topy_connection_t *connection) {
	struct pollfd fds;
	fds.fd = connection->socket;
	fds.events = POLLIN;
	int res = poll(&fds, 1, 0);
	if (res > 0 && (fds.revents & POLLIN)) {
		char *buffer = malloc(TOPY_BUFFER_SIZE);
		while (read(connection->socket, buffer, sizeof(buffer)) > 0);
		free(buffer);
	}
}

int connection_query_send(topy_connection_t *connection, int send_tid, char *data, size_t size) {
	int res;
	if (send_tid != 0) {
		connection->tid = (connection->tid + 1) % 65535;
		char *query;
		int query_size = spprintf(&query, 0, "TID %i %s", connection->tid, data);
		res = write(connection->socket, query, query_size);
		efree(query);
	}
	else {
		res = write(connection->socket, data, size);
	}

	if (res == -1)
		return -1;

	struct pollfd fds;
	fds.fd = connection->socket;
	fds.events = POLLOUT;
	res = poll(&fds, 1, 0);
	if ((fds.revents & POLLHUP) || (fds.revents & POLLERR))
		return -1;

	return 0;
}

int connection_query_send_safe(topy_connection_t *connection, int send_tid, char *data, size_t size) {
	connection_clear(connection);
	if (connection_query_send(connection, send_tid, data, size) == -1) {
		//Reconnect if needed
		if (
			connection_reconnect(connection) == -1 ||
			connection_query_send(connection, send_tid, data, size) == -1
		) {
			php_error(E_WARNING, "topy_query() error: Could not send query.");
			connection_close(connection);
			return -1;
		}
	}
	return 0;
}

void connection_query_read_result(topy_connection_t *connection, zval *return_value) {
	size_t buffer_size = TOPY_BUFFER_SIZE;
	char *buffer = malloc(buffer_size);

	size_t size = 0;

	//Read answer
	for (;;) {
		size_t readen = read(connection->socket, buffer + size, buffer_size - size);

		//Connection closed (End Of File)
		if (readen == 0) { 
			php_error(E_WARNING, "topy_query() error: EOF.");
			connection_reconnect(connection);
			free(buffer);
			RETURN_FALSE;
		}

		//Error (for example timeout)
		if (readen == -1) {
			php_error(E_WARNING, "topy_query() error: %i.", errno);
			free(buffer);
			RETURN_FALSE;
		}

		size += readen;
		if (size >= 2 && buffer[size - 2] == '\r' && buffer[size - 1] == '\n') 
			break;	

		//Resize buffer
		if (size == buffer_size) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
		}
	}
	buffer[size - 2] = '\0';
	
	//parse header
	char *line, *next;
	line = strtok_r(buffer, "\n", &next);

	if (strncmp(line, "TID: ", 5) == 0) {
		int tid = atoi(line + 5);
		//printf("Transaction ID is: %i\n", tid);
		if (tid != connection->tid) {
			php_error(E_WARNING, "Wrong Transaction Id");
			connection_reconnect(connection);
			free(buffer);
			RETURN_FALSE;
		}
		line = strtok_r(NULL, "\n", &next);
	}

	if (strncmp(line, "ERROR ", 6) == 0) {
		char *code, *msg;
		code = strtok_r(line + 6, " ", &msg);
		if (code && msg) connection_error_set(connection, atoi(code), msg);
		free(buffer);
		RETURN_FALSE;
	}

	if (strcmp(line, "OK") != 0) {
		free(buffer);
		RETURN_FALSE;
	}

	line = strtok_r(NULL, "\n", &next);

	//if no data
	if (line == NULL) {
		connection_error_ok(connection);
		free(buffer);
		RETURN_TRUE;
	}

	//parse data
	if (next == NULL) {
		free(buffer);
		RETURN_FALSE;
	}

	if (strcmp(line, "DATA: TEXT") == 0) {
		connection_error_ok(connection);
		RETVAL_STRING(next, 1);
		free(buffer);
		return;
	}
	else if (strcmp(line, "DATA: PHP_SERIALIZE") == 0) {
		const unsigned char *p = (const unsigned char*) next;
		int buf_len = strlen(next);
		php_unserialize_data_t var_hash;

		PHP_VAR_UNSERIALIZE_INIT(var_hash);
		if (!php_var_unserialize(&return_value, &p, p + buf_len,  &var_hash TSRMLS_CC)) {
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
			zval_dtor(return_value);
			php_error(E_WARNING, "Not a valid php serialize string");
			free(buffer);
			RETURN_FALSE;
		}
		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		connection_error_ok(connection);
		free(buffer);
		return;
	}
	else {
		free(buffer);
		RETURN_FALSE;
	}
}

//-------- Destructors ---------//
static void php_topy_connection_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
//	php_printf("DESTROY\n");
	connection_free((topy_connection_t *) rsrc->ptr);
}

static void php_topy_connection_persist_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
//	php_printf("DESTROY PERSIST\n");
	connection_free((topy_connection_t *) rsrc->ptr);
}

//-------- Initialization ---------//
PHP_MINIT_FUNCTION(topy) {
	le_topy_connection = zend_register_list_destructors_ex(php_topy_connection_dtor, NULL, PHP_TOPY_CONNECTION_RES_NAME, module_number);
	le_topy_connection_persist = zend_register_list_destructors_ex(NULL, php_topy_connection_persist_dtor, PHP_TOPY_CONNECTION_RES_NAME, module_number);

	REGISTER_LONG_CONSTANT("TOPY_TIMEOUT", TOPY_TIMEOUT, CONST_CS|CONST_PERSISTENT);
	return SUCCESS;
}

PHP_FUNCTION(topy_connect) {
	char *host, *port = NULL;
	int host_len = 0, port_len = 0, timeout = TOPY_TIMEOUT;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l", &host, &host_len, &port, &port_len, &timeout) == FAILURE) {
		RETURN_FALSE;
	}

	int socket = socket_open(host, port, timeout);
	if (socket == -1) {
		RETURN_FALSE;
	}

	topy_connection_t *connection = connection_new(0, 0, NULL, socket, host, port, timeout);
	ZEND_REGISTER_RESOURCE(return_value, connection, le_topy_connection);
}

PHP_FUNCTION(topy_pconnect) {
	char *host, *port = NULL;
	int host_len = 0, port_len = 0, timeout = TOPY_TIMEOUT;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l", &host, &host_len, &port, &port_len, &timeout) == FAILURE) {
		RETURN_FALSE;
	}

	char *key;
	int key_len;
	key_len = spprintf(&key, 0, "topy:%s:%s", host, port) + 1;

	//retrieve and check connection resource
	list_entry *le;
	if (zend_hash_find(&EG(persistent_list), key, key_len, (void **) &le) == SUCCESS) {
		//php_printf("RETRIEVE\n");
		topy_connection_t *connection = le->ptr;
		if (connection->socket == -1)
			connection_reconnect(connection);

		ZEND_REGISTER_RESOURCE(return_value, connection, le_topy_connection_persist);
		efree(key);
		return;
	}

	//..or create new connection resource
	int socket = socket_open(host, port, timeout);
	if (socket == -1) {
		efree(key);
		RETURN_FALSE;
	}

	topy_connection_t *connection = connection_new(1, key_len, key, socket, host, port, timeout);
	ZEND_REGISTER_RESOURCE(return_value, connection, le_topy_connection_persist);

	list_entry new_le;
	new_le.ptr = connection;
	new_le.type = le_topy_connection_persist;

//	php_printf("ADD\n");
	zend_hash_add(&EG(persistent_list), key, key_len, &new_le, sizeof(list_entry), NULL);
	efree(key);
}

PHP_FUNCTION(topy_reconnect) {
	zval *_zval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &_zval) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	if (connection_reconnect(connection) != 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

#define ADD_ASSOC_STRING_OR_NULL(var, key, str) if (str == NULL) add_assoc_null(var, key); else add_assoc_string(var, key, str, 1);

PHP_FUNCTION(topy_info) {
	zval *_zval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &_zval) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	array_init(return_value);
	add_assoc_bool(return_value, "persist", connection->persist);
	ADD_ASSOC_STRING_OR_NULL(return_value, "key", connection->key);
	ADD_ASSOC_STRING_OR_NULL(return_value, "host", connection->host);
	ADD_ASSOC_STRING_OR_NULL(return_value, "port", connection->port);
	add_assoc_long(return_value, "timeout", connection->timeout);
	add_assoc_long(return_value, "socket", connection->socket);
	add_assoc_long(return_value, "tid", connection->tid);
	add_assoc_long(return_value, "error_code", connection->error_code);
	ADD_ASSOC_STRING_OR_NULL(return_value, "error_msg", connection->error_msg);
}

PHP_FUNCTION(topy_error) {
	zval *_zval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &_zval) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	if (connection->error_code == 0) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_assoc_long(return_value, "code", connection->error_code);
	ADD_ASSOC_STRING_OR_NULL(return_value, "msg", connection->error_msg);
}

PHP_FUNCTION(topy_close) {
	zval *_zval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &_zval) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	connection_close(connection);

	RETURN_TRUE;
}

PHP_FUNCTION(topy_query) {
	zval *_zval;
	char *str;
	int str_len = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &_zval, &str, &str_len) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	connection_error_clear(connection);

	if (connection->socket == -1) {
		php_error(E_WARNING, "topy_query() error: connection was closed.");
		RETURN_FALSE;
	}

	//Send query
	if (connection_query_send_safe(connection, 1, str, str_len) == -1) {
		RETURN_FALSE;
	}

	connection_query_read_result(connection, return_value);
}

PHP_FUNCTION(topy_query_send) {
	zval *_zval;
	char *str;
	int str_len = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &_zval, &str, &str_len) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	if (connection->socket == -1) {
		php_error(E_WARNING, "topy_query() error: connection was closed.");
		RETURN_FALSE;
	}

	//Send query
	if (connection_query_send_safe(connection, 1, str, str_len) == -1) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

PHP_FUNCTION(topy_query_read) {
	zval *_zval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &_zval) == FAILURE) {
		RETURN_FALSE;
	}

	topy_connection_t *connection;
	ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, &_zval, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

	connection_error_clear(connection);
	connection_query_read_result(connection, return_value);
}

#define SOCKETS_MAX_NB 256
PHP_FUNCTION(topy_query_wait) {
	zval *arr;
	int timeout = TOPY_TIMEOUT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|l", &arr, &timeout) == FAILURE) {
		RETURN_FALSE;
	}

	HashTable *arr_hash = Z_ARRVAL_P(arr);
	HashPosition position;
	zval **value;
	struct pollfd fds[SOCKETS_MAX_NB];
	long resources[SOCKETS_MAX_NB];

	int fds_size = 0;
	for (zend_hash_internal_pointer_reset_ex(arr_hash, &position); zend_hash_get_current_data_ex(arr_hash, (void**) &value, &position) == SUCCESS && fds_size <= SOCKETS_MAX_NB; zend_hash_move_forward_ex(arr_hash, &position)) {
		topy_connection_t *connection;
		ZEND_FETCH_RESOURCE2(connection, topy_connection_t*, value, -1, PHP_TOPY_CONNECTION_RES_NAME, le_topy_connection, le_topy_connection_persist);

		resources[fds_size] = Z_LVAL_PP(value);
		fds[fds_size].fd = connection->socket;
		fds[fds_size].events = POLLIN;
		fds_size++;
	}

	int res = poll(fds, fds_size, timeout);
	if (res <= 0) {
		RETURN_FALSE;
	}

	int i;
	for (i = 0; i < fds_size; i++) {
		if (fds[i].revents & POLLIN) {
			RETURN_RESOURCE(resources[i]);
		}
	}
}
