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

#ifndef _WORDS_PARSER_HH
#define _WORDS_PARSER_HH

#include <sstream>
#include <string>

#define CHAR_CLASSE_UNDEF 255
#define CHAR_CLASSE_SPACE 0
#define CHAR_CLASSE_SEPARATOR 1

#define CHAR_CLASSE_1 2
#define CHAR_CLASSE_2 3
#define CHAR_CLASSE_3 4
#define CHAR_CLASSE_4 5


class WordsParser {
private:
	std::string get_next(bool const allchars);

public:
	uint8_t char_classe[255];

	std::istream *buffer;
	std::string current;

	std::string next(bool const allchars = false);
	char next_char();
	std::string until(char const last);
	std::string until_end();

	bool waiting_for(std::string const str);
	void set_char_classe(std::string const chars, uint8_t const value);
	WordsParser(std::istream *in);
};

#endif
