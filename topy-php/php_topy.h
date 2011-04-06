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

#ifndef PHP_TOPY_H
#define PHP_TOPY_H

#define PHP_TOPY_VERSION "0.10"
#define PHP_TOPY_EXTNAME "topy"

#define PHP_TOPY_CONNECTION_RES_NAME "topy_connection"
int le_topy_connection;
int le_topy_connection_persist;

#define TOPY_TIMEOUT 30
#define TOPY_BUFFER_SIZE 1024

typedef struct {
	int persist;
	int socket;
	char *key;
	int key_len;
	int tid;
	char *host;
	char *port;
	long int timeout;
	char *error_msg;
	int error_code;
} topy_connection_t;

PHP_MINIT_FUNCTION(topy);
PHP_MINFO_FUNCTION(topy);

PHP_FUNCTION(topy_connect);
PHP_FUNCTION(topy_pconnect);
PHP_FUNCTION(topy_reconnect);
PHP_FUNCTION(topy_info);
PHP_FUNCTION(topy_error);
PHP_FUNCTION(topy_query);
PHP_FUNCTION(topy_query_send);
PHP_FUNCTION(topy_query_read);
PHP_FUNCTION(topy_query_wait);
PHP_FUNCTION(topy_close);

extern zend_module_entry topy_module_entry;
#define phpext_topy_ptr &topy_module_entry

#endif
