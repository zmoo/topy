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

#include <iostream>
#include "users_parser.hh"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "USAGE: <pat>" << std::endl;
		return -1;
	}
	
	UsersParser parser;
	if (!parser.open(argv[1])) {
		std::cerr << "Can not open input file" << std::endl;
		return -1;
	}

	if (!parser.parse()) {
		std::cerr << "ERROR " << parser.get_error_msg() << " (line: " << parser.get_error_line() << ")" << std::endl;
		return -1;
	}

	std::cout << "Yes!" << std::endl;
}

