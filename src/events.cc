/*
 *  Copyright (C) 2008 Nicolas Vion <nico@picapo.net>
 *
 *   This file is part of Topy.
 *
 *   Topy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Topy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Topy; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "events.hh"
#include "config.h"

#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>

#include "log.hh"

#define BUFFER_SIZE 4096

bool Client::write(void *buffer, size_t size) {
	if (opened) {
		struct pollfd item;
		item.fd = fd;
		item.events = POLLOUT;
		poll(&item, 1, 1000);

		if (item.revents & POLLOUT) {
			return (::write(fd, buffer, size) == (ssize_t) size);
		}
		else {
			log.msg(LOG_ERR, "Socket error: could not write data");
			return false;
		}
	}
	else {
		log.msg(LOG_NOTICE, "Connection was closed");
		return false;
	}
	return false;
}

bool Client::write(std::stringstream &stream) {
	char buf[BUFFER_SIZE];
	do {
		stream.read((char *) &buf, sizeof(buf));
		if (!write(buf, stream.gcount()))
			return false;
	} while (!stream.eof());
	return true;
}

size_t Client::read(void *buffer, size_t size) {
	return (ev_buffer != NULL and opened) ? bufferevent_read(ev_buffer, buffer, size) : 0;
}

void Client::clear() {
	char buf[BUFFER_SIZE];
	size_t readen;
	do {
		readen = read(&buf, sizeof(buf));
	} while (readen == sizeof(buf));
}

void Client::read(std::stringstream &stream) {
	char buf[1024];
	size_t readen;
	do {
		readen = read(&buf, sizeof(buf));
		if (readen > 0) {
			stream.write(buf, readen);
		}
	} while (readen == sizeof(buf));
}

void Client::receive() {
	try {
		std::stringstream stream;
		read(stream);

		std::string str = stream.str();
		if (str.substr(0, 4) == "quit")
			exit();
		else {		
			char answer[6] = "OK\n\r\n";
			write(&answer, sizeof(answer) - 1);
		}
	}
	catch (...) {
		log.msg(LOG_ERR, "Not a valid command");
	}
}

void client_write(bufferevent *ev_buffer, void *data) {
}

void client_read(bufferevent *ev_buffer, void *data) {
	Client *client = (Client *) data;
	client->receive();
	if (client->end) 
		client->close();
}

void Client::exit() {
	end = true;
}

void Client::error(short what) {
	log.msg(LOG_NOTICE, "Socket error " + StringUtils::to_string(what) + " on #" + StringUtils::to_string(fd));
}

void client_error(bufferevent *ev_buffer, short what, void *data) {
	Client *client = (Client *) data;
	client->error(what);
	client->close();
}

void Client::ref() {
	ref_count++;
	//std::cout << "ref_count++ >> " << ref_count << std::endl;
}

void Client::unref() {
	if (ref_count < 1)
		return;

	ref_count--;
	//std::cout << "ref_count-- >> " << ref_count << std::endl;

	if (ref_count == 0)
		delete this;
}

void Client::close() {
	opened = false;
	if (ev_buffer != NULL) {
		bufferevent_disable(ev_buffer, EV_READ);
		bufferevent_free(ev_buffer);
		ev_buffer = NULL;
	}

	log.msg(LOG_NOTICE, "Close #" + StringUtils::to_string(fd));
	::close(fd);

	unref();
}

bool Client::open() {
	log.msg(LOG_NOTICE, "Open #" + StringUtils::to_string(fd));

	ev_buffer = bufferevent_new(fd, client_read, client_write, client_error, this);
	if (ev_buffer == NULL) {
		log.msg(LOG_ERR, "Could not create bufferevent.");
		delete this;
		return false;
	}

	bufferevent_enable(ev_buffer, EV_READ);
	ref();
	return true;
}

int Client::get_fd() {
	return fd;
}

Client::Client(int const _fd) {
	end = false;
	opened = true;
	ref_count = 0;
	fd = _fd;
	open();
}

Client::~Client() {
}

void Server::new_client(int const _fd) {
	new Client(_fd);
}

void Server::connect(const int listen_fd, short event) {
	sockaddr_storage client_sa;
	socklen_t client_sa_len = sizeof(client_sa);

	int client_fd = accept(listen_fd, (sockaddr *) &client_sa, &client_sa_len);

	if (client_fd < 0 or client_sa_len == 0)
		return;

	if (client_sa_len != 0) {
		if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
			log.msg(LOG_ERR, "Can not set 0_NONBLOCK attribut to file descriptor #" + StringUtils::to_string(client_fd) + ".");

		new_client(client_fd);
	}
}

void server_connect(const int listen_fd, short event, void *data) {
	Server *server = (Server *) data;
	server->connect(listen_fd, event);
}

bool Server::open(std::string const _address, std::string const _port) {
	address = _address;
	port = _port;

	log.msg(LOG_NOTICE, "Open socket on " + address + ":" + port, true);

	addrinfo hints, *res;

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_addr = NULL;

	//get address/port
	int fodder;
	if ((fodder = getaddrinfo(address.c_str(), port.c_str(), &hints, &res)) != 0) {
		log.msg(LOG_ERR, gai_strerror(fodder), true);
		return false;
	}

	//create socket
	if ((fd = socket(res->ai_family, hints.ai_socktype, hints.ai_protocol)) == -1) {
		freeaddrinfo(res);
		log.msg(LOG_ERR, "Can not create socket", true);
		return false;
	}

	//set socket options
	int on = 1;
	if (
		(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) != 0) |
		(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on)) != 0)
	) {
		freeaddrinfo(res);
		close(fd);
		log.msg(LOG_ERR, "Can not set socket options", true);
		return -1;
	}

	//bind
	if (bind(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		freeaddrinfo(res);
		close(fd);
		log.msg(LOG_ERR, "Can not bind socket", true);
		return false;
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		log.msg(LOG_ERR, "Can not set 0_NONBLOCK attribut to file descriptor #" + StringUtils::to_string(fd) + ".", true);

	freeaddrinfo(res);
	return true;
}

double Server::uptime() {
	return (started_at != 0) ? difftime(time(NULL), started_at) : 0;
}

void Server::listen() {
	started_at = time(NULL);

	log.msg(LOG_NOTICE, "Listening...");

	::listen(fd, 128);

	event_set(&ev, fd, EV_READ | EV_PERSIST, server_connect, this);
	event_add(&ev, NULL);
}

Server::~Server() {
	close(fd);
}

