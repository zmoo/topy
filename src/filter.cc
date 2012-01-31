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

#include "filter.hh"
#include "groups_interface.hh"
#include <iostream>


bool parse_where(WordsParser *parser, Filter *filter, ClientResult &result) {
	if (parser->current == "where") {
		if (!filter->compile(parser)) {
			result.error("[boolean expression error] " + filter->get_error());
			result.send();
			return false;
		}
	}
	return true;
}

std::string Filter::get_error() {
	return parser.get_error();
}

bool Filter::compile(WordsParser *words_parser) {
	context.groups = &groups;
	groups.lock();
	expr = parser.parse(words_parser);
	groups.unlock();

	if (expr != NULL) {
		words_parser->next();
		return true;
	}
	return false;
}

bool Filter::eval(User *user) {
#ifdef USER_ID_STR
	context.id = user->id;
#else
	context.id = NULL;
#endif
	context.group = user->group;
	return expr->get(&context);
}

bool Filter::is_defined() {
	return (expr != NULL);
}

Filter::Filter() : parser(&context) {
	expr = NULL;
}

Filter::~Filter() {
	if (expr != NULL)
		delete expr;
}

