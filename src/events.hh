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

#ifndef _EVENTS_HH
#define _EVENTS_HH

#include <string>
#include <sys/types.h>
#include <event.h>

class Client {
	friend void client_read(bufferevent *ev_buffer, void *data);
	friend void client_error(bufferevent *ev_buffer, short what, void *data);

private:
	bool end;
	bool opened;
	bufferevent *ev_buffer;
	int ref_count;
	int fd;

	virtual void receive();
	void error(short what);

protected:
	void clear();
	void exit();
	
public:
	int get_fd();
	bool write(void *buffer, size_t size);
	bool write(std::stringstream &stream);
	size_t read(void *buffer, size_t size);
	void read(std::stringstream &stream);

	void ref();
	void unref();

	void close();
	bool open();

	Client(int const fd);

	virtual ~Client();
};

class Server {
	friend void server_connect(const int listen_fd, short event, void *data);

private:
	int fd;
	event ev;
	time_t started_at;

	void connect(const int listen_fd, short event);

protected:
	std::string port;
	std::string address;

	virtual void new_client(int const _fd);

public:
	void listen();
	bool open(std::string const _address, std::string const _port);
	double uptime();

	virtual ~Server();
};

#endif
