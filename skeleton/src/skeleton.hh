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

#ifndef _SKELETON_HH
#define _SKELETON_HH

#include <string>
#include <sys/types.h>
#include <event.h>

class ClientBuffer {
private:
	bufferevent *ev_buffer;
public:
	void write(void *buffer, size_t size);
	size_t read(void *buffer, size_t size);
	void empty();

	ClientBuffer(int fd, evbuffercb readcb, evbuffercb writecb, everrorcb errorcb, void *cbarg);
	~ClientBuffer();
};

class Client {
	friend void client_read(bufferevent *ev_buffer, void *data);
	friend void client_error(bufferevent *ev_buffer, short what, void *data);
private:
	int fd;
	bool verbose;

	ClientBuffer buffer;
	bool end;

	virtual void read();
	void error(short what);
	void write(void *buffer, size_t size);

	void disconnect();
	void exit();
	
public:
	Client(int const _fd, bool const _verbose = true);
};

class Server {
	friend void server_new_client(const int listen_fd, short event, void *data);
private:
	bool verbose;

	std::string port;
	std::string address;

	int fd;
	event ev;

	void new_client(const int listen_fd, short event);

public:
	void go(bool const _verbose = true);
	bool open(std::string const _address, std::string const _port);
};

#endif
