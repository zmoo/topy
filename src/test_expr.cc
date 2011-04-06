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

#include "expr_bool.hh"
#include "groups.hh"
#include <iostream>
#include <sstream>


int main(int argc, char *argv[]) {

	Groups groups;
	groups.add("c:FR", 1, 255);
	groups.add("c:UK", 2, 255);
	groups.add("c:RU", 3, 255);

	groups.add("s:m", 262144, 786432);
	groups.add("s:f", 524288, 786432);
	groups.add("s:n", 786432, 786432);

	ExprParser parser;
	int toto = 42, titi = 0;
	GroupId group = 0;

	parser.vars.set("toto", &toto);
	parser.vars.set("titi", &titi);
	parser.group = &group;
	parser.groups = &groups;

	group = groups.get_id("c:FR") + groups.get_id("s:f");

	ExprBool *expr = parser.parse("true and (toto or titi) and !![c:FR]");

	if (expr) {
		if (expr != NULL) {
			expr->show();
			std::cout << " => " << expr->get_str() << std::endl;

			//evalutate it 10 000 000 times
			for (int i = 0; i < 10000000; i++)
				expr->get();
		}
		delete expr;
	}
}
