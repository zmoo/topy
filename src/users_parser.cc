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

#include "users_parser.hh"
#include "topy.h"

#include <sstream>

std::string UsersParser::get_error_msg() {
	return error_msg;
}

unsigned int UsersParser::get_error_line() {
	return error_line;
}

bool UsersParser::open(std::string const path) {
	fb.open(path.c_str(), std::ios_base::in);
	return fb.is_open();
}

std::string UsersParser::char_name(char const c) {
	if (c == '\n')
		return "\\n";
	return std::string(1, c);
}

int UsersParser::read_int() {
	std::stringstream oss;
	char next;

	while ((next = fb.sbumpc()) != std::filebuf::traits_type::eof() and next >= '0' and next <= '9')
		oss << next;

	fb.sputbackc(next);

	int i = -1;
	oss >> i;
	return i;
}

inline void UsersParser::waitfor(char const c) {
	char next= fb.sbumpc();
	if (next != c) {
		error_msg = "unexpected \"" + char_name(next) + "\" expected \"" + char_name(c) + "\"";
		throw 1;
	}
}

void UsersParser::parse_stats_vector() {
	waitfor('{');
	int type = read_int();
	waitfor(':');
	int count = read_int();
	waitfor(':');
	for (int i = 0; i < count; i++) {
		int sample = read_int();
		if (i < count - 1)
			waitfor(',');
	}
	waitfor('}');
}

void UsersParser::parse_user() {
	waitfor('u');
	waitfor('{');
	UserId id = read_int();
	waitfor(':');
	waitfor('v');
	waitfor('{');
	unsigned int total = read_int();
	waitfor(':');
	waitfor('t');
	waitfor('{');
	unsigned int day = read_int();
	waitfor(':');
	unsigned int month = read_int();
	waitfor('}');
	waitfor('d');
	parse_stats_vector();
	waitfor('m');
	parse_stats_vector();
	waitfor('}');
	waitfor('}');
}

bool UsersParser::parse() {
	unsigned int line = 1;
	try {
		while (fb.sgetc() != std::filebuf::traits_type::eof()) {
			parse_user();
			waitfor('\n');
			line++;
		}
		return true;
	}

	catch (...) {
		error_line = line;
		return false;
	}
}

