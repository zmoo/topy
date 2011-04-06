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

#include "server.hh"
#include "udp.hh"

bool UdpServer::open(std::string const address, std::string const port) {
	log.msg(LOG_NOTICE, "Start udp server on " + address + ":" + port, true);

	addrinfo hints, *res;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_socktype = SOCK_DGRAM;
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

	//bind
	if (bind(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		log.msg(LOG_ERR, "Can not bind socket", true);
		close(fd);
		freeaddrinfo(res);
		return false;
	}

	return true;
}

size_t UdpServer::read(void *buffer, size_t size) {
	return (ev_buffer) ? bufferevent_read(ev_buffer, buffer, size) : 0;
}

void UdpServer::read(std::stringstream &stream) {
	char buf[1024];
	size_t readen;
	do {
		readen = read(&buf, sizeof(buf));
		if (readen > 0) {
			stream.write(buf, readen);
		}
	} while (readen == sizeof(buf));
}

void UdpServer::read() {
	try {
		std::stringstream stream;
		read(stream);
		WordsParser parser(&stream);
		parser.next();

		ClientResult result(NULL);
		bool parsed;
		parse_query(parsed, result, &parser, NONE);
	}
	catch (...) {
		log.msg(LOG_ERR, "Not a valid command buffer");
	}
}

void udp_server_write(bufferevent *ev_buffer, void *data) {
}

void udp_server_read(bufferevent *ev_buffer, void *data) {
	UdpServer *udp_server = (UdpServer *) data;
	udp_server->read();
}

void udp_server_error(bufferevent *ev_buffer, short what, void *data) {
	log.msg(LOG_ERR, "Bufferevent error");
}

void UdpServer::listen() {
	ev_buffer = bufferevent_new(fd, udp_server_read, udp_server_write, udp_server_error, this);
	if (ev_buffer == NULL) {
		return;
	}
	bufferevent_enable(ev_buffer, EV_READ);
}

UdpServer::UdpServer() {
	ev_buffer = NULL;
}
