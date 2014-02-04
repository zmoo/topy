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

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "commands.h"
#include "commands_send.h"

int open_socket(char const *address, char const *port) {
	struct addrinfo hints, *res;
	struct sockaddr client_addr;

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_addr = NULL;

	//get port/adress parameters
	int fodder, fd;
	if ((fodder = getaddrinfo(address, port, &hints, &res)) != 0) {
		fprintf(stderr, "%s\n", gai_strerror(fodder));
		return -1;
	}

	//create socket
	if ((fd = socket(res->ai_family, hints.ai_socktype, hints.ai_protocol)) == -1) {
		freeaddrinfo(res);
		perror("socket");
		return -1;
	}

	//set socket options
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (
		(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) != 0) |
		(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) != 0)
	) {
		perror("Can not set socket options\n");
		close(fd);
		return -1;
	}

	//bind
	if (connect(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		perror("connect");
		close(fd);
		freeaddrinfo(res);
		return -1;
	}

	return fd;
}

int main(int argc, char **argv) {
	int socket = open_socket("127.0.0.1", "6969");
	if (socket == -1) {
		fprintf(stderr, "Can not open socket\n");
		return 1;
	}

	int i;
	CmdVisitsIncRes visits;
	for (i = 0; i < 10; i++)
		topy_visits_inc(socket, 0, &visits);

	CmdVisitsGetRes res;
	if (topy_get_visits(socket, 0, &res) == 0) {
		printf("res: %i\n", GET_UINT8(res.error));
		printf("days size: %i\n", GET_UINT8(res.days_size));
		printf("days[0]: %i\n", GET_UINT32(res.days[0]));
	}

	printf("count: %i\n", topy_count(socket));
	printf("Ok\n");

	close(socket);
}
