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
#include "stringutils.hh"

#include <iostream>

std::string ConfigFile::get(std::string const name) {
	Vars::iterator it = vars.find(name);
	return (it != vars.end()) ? it->second : "";
}

int ConfigFile::get_int(std::string const name) {
	Vars::iterator it = vars.find(name);
	return (it != vars.end()) ? StringUtils::to_int(it->second) : 0;
}

bool ConfigFile::isset(std::string const name) {
	Vars::iterator it = vars.find(name);
	return (it != vars.end());
}

std::string ConfigFile::parse_str() {
	if (parser.current == "\"") {
		return parser.until('"');
	}
	else {
		std::cerr << "Waiting for string. Given: " << parser.current << std::endl;
		return "";
	}
}

std::string ConfigFile::next() {
	parser.next();
	if (parser.current == "#") {
		parser.until('\n');
		return next();
	}
	return parser.current;
}

bool ConfigFile::parse_fields() {
	if (!parser.waiting_for("{")) return false;

	while (next() != "") {
		if (parser.current == "}") return true;

		std::string type = parser.current;
		if (next() == "") return false;
		std::string name = parser.current;
		if (!parser.waiting_for(";")) break;

		fields.push_back(std::pair<std::string, std::string> (name, type));
	}
	return false;
}

bool ConfigFile::parse(std::string const path) {
	file.open(path.c_str(), std::ifstream::in);
	if (!file.is_open()) {
		std::cerr << "Could not open configuration file: " << path << std::endl;
		return false;
	}
	while (next() != "") {
		if (parser.current == "fields") {
			if (!parser.waiting_for("=")) break;
			if (!parse_fields()) break;
			if (!parser.waiting_for(";")) break;
		}
		else {
			std::string name = parser.current;
			if (!parser.waiting_for("=")) break;
			if (parser.next() == "") break;
			std::string value = parse_str();
			if (!parser.waiting_for(";")) break;
			vars.insert(std::pair<std::string, std::string> (name, value));
		}
	}
	file.close();
	return true;
}

ConfigFile::ConfigFile() : parser(&file) {
}
