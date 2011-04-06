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

#include "config_file.hh"

#include <iostream>

int main(int argc, char *argv[]) {
	ConfigFile config;
	config.parse("topy.conf");

	if (config.isset("address")) 
		std::cout << "address: " << config.get("address") << std::endl;

	if (config.isset("port")) 
		std::cout << "port: " << config.get("port") << std::endl;

	ConfigFile::Fields::iterator it;
	for (it = config.fields.begin(); it != config.fields.end(); it++) {
		std::cout << it->first << " (" << it->second << ")" << std::endl;
	}

}
