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

#include "result.hh"

#include <sys/socket.h>

void ClientResult::error(std::string const msg, int const code) {
	error_code = code;
	data << msg;
}

void ClientResult::msg(std::string const msg) {
	type = TEXT;
	data << msg << std::endl;
}

Client *ClientResult::get_client() {
	return client;
}

void ClientResult::send() {
	if (!client)
		return;

	std::stringstream answer;
	if (tid != -1) {
		answer << "TID: " << tid << std::endl;
	}
	if (error_code != 0) {
		answer << "ERROR " << error_code << " " << data.str() << std::endl;
	}
	else {
		answer << "OK" << std::endl;
		if (!quiet and type != NONE and data.tellg() != data.tellp()) {
			std::string body = data.str();
			answer << "DATA: " << ((type == TEXT) ? "TEXT" : "PHP_SERIALIZE") << std::endl << body;
		}
	}
	answer << "\r\n";
	client->write(answer);
}

ClientResult::ClientResult(Client *_client) : client(_client), tid(-1), type(TEXT), error_code(0), replicated(false), quiet(false) {
	data.precision(16);
}


