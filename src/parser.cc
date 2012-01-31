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

#include "parser.hh"

#include <sstream>


int Parser::get_line() {
	return line;
}

std::string Parser::get_error_msg() {
	return error_msg;
}

void Parser::set_error_msg(std::string str) {
	error_msg = str;
}

bool Parser::eof() {
	return (fb.sgetc() != std::filebuf::traits_type::eof());
}

bool Parser::open(std::string const path) {
	line = 1;
	fb.open(path.c_str(), std::ios_base::in);
	return fb.is_open();
}

void Parser::close() {
	fb.close();
}

std::string Parser::char_name(char const c) {
	if (c == '\n')
		return "\\n";
	return std::string(1, c);
}

int Parser::read_int() {
	std::stringstream oss;
	char next;

	while ((next = fb.sbumpc()) != std::filebuf::traits_type::eof() and ((next >= '0' and next <= '9') or next == '-'))
		oss << next;

	fb.sputbackc(next);

	int i = -1;
	oss >> i;
	return i;
}

uint64_t Parser::read_uint64() {
	std::stringstream oss;
	char next;

	while ((next = fb.sbumpc()) != std::filebuf::traits_type::eof() and ((next >= '0' and next <= '9') or next == '-'))
		oss << next;

	fb.sputbackc(next);

	uint64_t i = -1;
	oss >> i;
	return i;
}

std::string Parser::read_str() {
	std::stringstream oss;

	char c = fb.sbumpc();
	if (c != '"') {
		return "";
	}

	char previous = fb.sbumpc();
	if (previous == '"') {
		return "";
	}

	while ((c = fb.sbumpc()) != std::filebuf::traits_type::eof() and !(previous != '\\' and c == '"')) {
		if (!(previous == '\\' and c == '"'))
			oss << previous;
		previous = c;
	}
	oss << previous;
	return oss.str();
}

std::string Parser::read_until(char const c) {
	std::stringstream oss;
	char next;

	while ((next = fb.sbumpc()) != std::filebuf::traits_type::eof() and next != c)
		oss << next;

	fb.sputbackc(next);
	return oss.str();
}

void Parser::waitfor(char const c) {
	char next = fb.sbumpc();
	if (next == '\n')
		line++;

	if (next != c) {
		error_msg = "unexpected \"" + char_name(next) + "\" expected \"" + char_name(c) + "\"";
		throw 1;
	}
}

bool Parser::test_next(char const c) {
	char next = fb.sbumpc();
	if (next == c)
		return true;
	fb.sputbackc(next);
	return false;
}

