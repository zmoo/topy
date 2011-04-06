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

#ifndef _USERS_PARSER_HH
#define _USERS_PARSER_HH

#include <string>
#include <fstream>

struct UsersParser {
private:
	std::string error_msg;
	unsigned int error_line;

	std::filebuf fb;
	inline void waitfor(char const c);
	void parse_stats_vector();
	void parse_user();
	std::string char_name(char const c);
	int read_int();

public:
	std::string get_error_msg();
	unsigned int get_error_line();

	bool open(std::string const path);
	bool parse();
};

#endif
