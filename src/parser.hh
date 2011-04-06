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

#ifndef _PARSER_HH
#define _PARSER_HH

#include <string>
#include <fstream>

struct Parser {
private:
	std::string error_msg;
	int line;	

	std::filebuf fb;
	std::string char_name(char const c);

public:
	std::string get_error_msg();
	void set_error_msg(std::string str);
	int get_line();

	std::string read_until(char const c);
	int read_int();
	uint64_t read_uint64();
	std::string read_str();
	void waitfor(char const c);
	bool test_next(char const c);
	bool eof();
	bool open(std::string const path);
	void close();
};

#endif
