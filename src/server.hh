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

#ifndef _SERVER_HH
#define _SERVER_HH

#include <sstream>
#include <string>

#include "events.hh"
#include "stats.hh"
#include "words_parser.hh"
#include "filter.hh"
#include "result.hh"
#include "pthread++.hh"
#include "users.hh"
#include "fields.hh"

#include "config.h"

class ClientTopy : public Client {
private:
	OutputType mode;

public:
	bool parse_query(WordsParser *parser);
	void receive();

	ClientTopy(int const client_fd);
};

class ServerTopy : public Server {
protected:
	void new_client(int const fd);

public:
	bool dump(std::string const path);
	bool dump_txt(std::string const path);
	bool dump_bin(std::string const path);

	bool restore_txt(std::string const path);
	bool restore_bin(std::string const path);
	bool restore(std::string const path);

	ServerTopy();
};

bool parse_from(WordsParser *parser, VectorUsers **from, ClientResult &result);
bool parse_join(WordsParser *parser, TopJoinItems &join, ClientResult &result);
bool parse_query(bool &parsed, ClientResult &result, WordsParser *parser, OutputType const mode);

#ifdef _SERVER_CC
ServerTopy server;
#else
extern ServerTopy server;
#endif

#endif
