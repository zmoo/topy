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

#ifndef _CONFIG_FILE_HH
#define _CONFIG_FILE_HH

#include <fstream>
#include <string>
#include <map>
#include <list>

#include "words_parser.hh"

class ConfigFile {
private:
	std::ifstream file;
	WordsParser parser;

	std::string parse_str();
	bool parse_fields();
	std::string next();

public:
	typedef std::map<std::string, std::string> Vars;
	Vars vars;

	typedef std::list< std::pair<std::string, std::string> > Fields;
	Fields fields;

	bool parse(std::string const path);

	std::string get(std::string const name);
	int get_int(std::string const name);
	bool isset(std::string const name);

	ConfigFile();
};

#endif

