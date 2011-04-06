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

#ifndef _FILTER_HH
#define _FILTER_HH

#include <string>
#include "expr_bool.hh"
#include "result.hh"
#include "user.hh"
#include "topy.h"

class Filter {
private:
	ExprContext context;
	ExprParser parser;
	ExprBool *expr;

public:
	std::string get_error();
	bool compile(WordsParser *words_parser);
	bool eval(User *user);
	bool is_defined();

	Filter();
	~Filter();
};

bool parse_where(WordsParser *parser, Filter *filter, ClientResult &result);

#endif
