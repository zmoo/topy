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

#ifndef _UDP_HH
#define _UDP_HH

#include <event.h>

class UdpServer {
	friend void udp_server_read(bufferevent *ev_buffer, void *data);

private:
	bufferevent *ev_buffer;
	int fd;
	size_t read(void *buffer, size_t size);
	void read(std::stringstream &stream);
	void read();

public:
	bool open(std::string const address, std::string const port);
	void listen();
	UdpServer();
};

#endif
