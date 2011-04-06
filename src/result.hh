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

#ifndef _RESULT_HH
#define _RESULT_HH

#include <sstream>

#include <sys/types.h>
#include "events.hh"

#define ERROR_DEFAULT 1

typedef enum {
	TEXT,
	PHP_SERIALIZE,
	NONE
} OutputType;

class ClientResult {
private:
	Client *client;

public:
	int tid;
	OutputType type;
	int error_code;
	std::stringstream data;
	bool replicated;
	bool quiet;

public:
	Client *get_client();
	void error(std::string const msg = "", int const code = ERROR_DEFAULT);
	void msg(std::string const msg);
	void send();

	ClientResult(Client *client);
};

#define RETURN_PARSE_ERROR(result, msg) { \
	stats.inc("unvalid"); \
	result.error(msg); \
	result.send(); \
	return false; \
}

#define RETURN_PARSE_ERROR_T(result, msg, thread) { \
	delete thread; \
	RETURN_PARSE_ERROR(result, msg); \
}

#define PARSING_ENDED(parser, result) { \
	if (parser->current != "") { \
		result.error("Unexpected: " + parser->current); \
		result.send(); \
		return false; \
	} \
}

#define PARSING_ENDED_T(parser, result, thread) { \
	if (parser->current != "") { \
		delete thread; \
		result.error("Unexpected: " + parser->current); \
		result.send(); \
		return false; \
	} \
}

#define PARSING_END(parser, result) { \
	parser->next(); \
	PARSING_ENDED(parser, result) \
}

#define PARSING_END_T(parser, result, thread) { \
	parser->next(); \
	PARSING_ENDED_T(parser, result, thread) \
}

#define RETURN_NOT_VALID_CMD(result) RETURN_PARSE_ERROR(result, "Not a valid command. Send command 'help' for more information.");

#endif
