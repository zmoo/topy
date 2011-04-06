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

#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>

#include "skeleton.hh"

#include "config.h"

void ClientBuffer::write(void *buffer, size_t size) {
	bufferevent_write(ev_buffer, buffer, size);
}

size_t ClientBuffer::read(void *buffer, size_t size) {
	return bufferevent_read(ev_buffer, buffer, size);
}

void ClientBuffer::empty() {
	char buf[1024];
	size_t readen;
	do {
		readen = read(&buf, sizeof(buf));
	} while (readen != sizeof(buf));
}

ClientBuffer::ClientBuffer(int fd, evbuffercb readcb, evbuffercb writecb, everrorcb errorcb, void *cbarg) {
	ev_buffer = bufferevent_new(fd, readcb, writecb, errorcb, cbarg);
	bufferevent_enable(ev_buffer, EV_READ);
}

ClientBuffer::~ClientBuffer() {
	bufferevent_free(ev_buffer);
}

void Client::read() {
	char buf[1024];
	size_t readen = buffer.read(&buf, sizeof(buf));

	if (readen ==  0) {
		exit();
		return;
	}

	if (readen == sizeof(buf))
		buffer.empty();

	try {
		std::string str(buf, readen);
		if (str.substr(0, 4) == "quit")
			exit();
		else {		
			char answer[6] = "OK\n\r\n";
			buffer.write(&answer, sizeof(answer) - 1);
		}
	}
	catch (...) {
		std::cerr << "Not a valid command" << std::endl;
	}
}

void client_write(bufferevent *ev_buffer, void *data) {
}

void client_read(bufferevent *ev_buffer, void *data) {
	Client *client = (Client *) data;
	client->read();
	if (client->end) 
		client->disconnect();
}

void Client::exit() {
	end = true;
}

void Client::disconnect() {
	if (verbose)
		std::cout << " * Disconnect #" << fd << std::endl;

	close(fd);
	delete this;
}

void Client::error(short what) {
	if (verbose)
		std::cerr << " * Error " << what << " on #" << fd << std::endl;
}

void client_error(bufferevent *ev_buffer, short what, void *data) {
	Client *client = (Client *) data;
	client->error(what);
	client->disconnect();
}

Client::Client(int const _fd, bool const _verbose) : buffer(_fd, client_read, client_write, client_error, this) {
	fd = _fd;
	verbose = _verbose;

	if (verbose)
		std::cout << " * Connect #" << fd << std::endl;

	end = false;
}

void Server::new_client(const int listen_fd, short event) {
	sockaddr_storage client_sa;
	socklen_t client_sa_len = sizeof(client_sa);

	int client_fd = accept(listen_fd, (sockaddr *) &client_sa, &client_sa_len);

	if (client_fd < 0 or client_sa_len == 0)
		return;

	if (client_sa_len != 0)
		new Client(client_fd, verbose);
}

void server_new_client(const int listen_fd, short event, void *data) {
	Server *server = (Server *) data;
	server->new_client(listen_fd, event);
}

bool Server::open(std::string const _address, std::string const _port) {
	address = _address;
	port = _port;

	std::cout << "Open socket on " << address << ":" << port << std::endl;

	addrinfo hints, *res;

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_addr = NULL;

	//get address/port
	int fodder;
	if ((fodder = getaddrinfo(address.c_str(), port.c_str(), &hints, &res)) != 0) {
		std::cerr << gai_strerror(fodder) << std::endl;
		return false;
	}

	//create socket
	if ((fd = socket(res->ai_family, hints.ai_socktype, hints.ai_protocol)) == -1) {
		freeaddrinfo(res);
		std::cerr << "Can not create socket" << std::endl;
		return false;
	}

	//set socket options
	int on = 1;
	if (
		(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) != 0) |
		(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on)) != 0)
	) {
		std::cerr << "Can not set socket options" << std::endl;
		close(fd);
		return -1;
	}

	//bind
	if (bind(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		std::cerr << "Can not bind socket" << std::endl;
		close(fd);
		freeaddrinfo(res);
		return false;
	}

	return true;
}

void Server::go(bool const _verbose) {
	std::cout << "Listening..." << std::endl;
	listen(fd, 128);

	event_init();
	event_set(&ev, fd, EV_READ | EV_PERSIST, server_new_client, this);
	event_add(&ev, NULL);
	event_dispatch();

	close(fd);
}

