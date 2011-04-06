/*
 *  Copyright (C) 2009 Nicolas Vion <nico@picapo.net>
 *
 *   This file is part of Fluxy.
 *
 *   Fluxy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Fluxy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Fluxy; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define _REPLICATOR_CC

#include "replicator.hh"

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>

#include "log.hh"

bool Replicator::open(std::string const address, std::string const port) {
	log.msg(LOG_NOTICE, "Start replication to " + address + ":" + port, true);

	int result = getaddrinfo(address.c_str(), port.c_str(), NULL, &target);
	if (result != 0) {
		log.msg(LOG_ERR, gai_strerror(result), true);
		return false;
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0) ) == -1) {
		log.msg(LOG_ERR, "Can not open socket for replication");
		return false;
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		log.msg(LOG_ERR, "Can not set 0_NONBLOCK attribut to file descriptor for replication.", true);
	}
	return opened = true;
}

void Replicator::add(std::stringstream &query) {
	if (!opened)
		return;

	char buf[4096];
	query.read((char *) &buf, sizeof(buf));
	size_t len = query.gcount();

	sendto(fd, &buf, len, MSG_NOSIGNAL, target->ai_addr, target->ai_addrlen);
}

Replicator::Replicator() : opened(false) {
}
