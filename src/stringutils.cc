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


#include "stringutils.hh"
#include <sstream>

int StringUtils::to_int(string const input) {
	std::stringstream oss(input);
	int rv;
	oss >> rv;
	return (oss.fail()) ? 0 : rv;
}

int64_t StringUtils::to_int64(string const input) {
	std::stringstream oss(input);
	int64_t rv;
	oss >> rv;
	return (oss.fail()) ? 0 : rv;
}

unsigned int StringUtils::to_uint(string const input) {
	std::stringstream oss(input);
	unsigned int rv;
	oss >> rv;
	return (oss.fail()) ? 0 : rv;
}

uint64_t StringUtils::to_uint64(string const input) {
	std::stringstream oss(input);
	int64_t rv;
	oss >> rv;
	return (oss.fail()) ? 0 : rv;
}

time_t StringUtils::to_time(string const input) {
	std::string str = input;

	int factor = 1;
	if (str.size() > 1) {
		char last = str.at(str.size() - 1);
		if (last == 'd' or last == 'h' or last == 'm' or last == 's') {
			str = str.substr(0, str.size() - 1);
			switch (last) {
				case 's' :
					factor = 1;
					break;
				case 'm' :
					factor = 60;
					break;
				case 'h' :
					factor = 60 * 60;
					break;
				case 'd' :
					factor = 60 * 60 * 24;
					break;
			}
		}
	}

	std::stringstream oss(str);
	time_t rv;
	oss >> rv;
	return (oss.fail()) ? 0 : rv * factor;
}

float StringUtils::to_float(string const input) {
	std::stringstream oss(input);
	float rv;
	oss >> rv;
	return (oss.fail()) ? 0 : rv;
}

string StringUtils::to_string(double const input) {
	std::stringstream oss;
	oss << input;
	return oss.str();
}

string StringUtils::to_string(int const input) {
	std::stringstream oss;
	oss << input;
	return oss.str();
}

string StringUtils::to_string(uint64_t const input) {
	std::stringstream oss;
	oss << input;
	return oss.str();
}

string StringUtils::to_string(unsigned int const input) {
	std::stringstream oss;
	oss << input;
	return oss.str();
}

string StringUtils::to_string(const float input) {
	std::stringstream oss;
	oss << input;
	return oss.str();
}

void StringUtils::split(const std::string str, const char separator, std::vector<std::string> &result) {
	std::string buf("");
	int size = str.size();
	char c;
	result.clear();
	for (int i = 0; i < size; i++) {
		if ((c = str[i]) == separator) {
			result.push_back(buf);
			buf = "";
		}
		else
			buf += c;
	}
	result.push_back(buf);
}

string StringUtils::chomp(string const str) {
	int i = str.length();
	while (i > 0 and (str[i - 1] == '\n' or str[i - 1] == '\r'))
		i--;
	return str.substr(0, i);
}

string StringUtils::rtrim(string const str) {
	int i = str.length();
	while (i > 0 and (str[i - 1] == '\n' or str[i - 1] == '\r'  or str[i - 1] == ' '))
		i--;
	return str.substr(0, i);
}

string StringUtils::upper(string const str) {
	string result = str;
	for (unsigned int i = 0; i < str.size(); i++) {
		result[i] = toupper(str[i]);
	}
	return result;
}

string StringUtils::lower(string const str) {
	string result = str;
	for (unsigned int i = 0; i < str.size(); i++) {
		result[i] = tolower(str[i]);
	}
	return result;
}

string StringUtils::addslashes(string const str) {
	stringstream result;
	for (unsigned int i = 0; i < str.length(); i++) {
		char c = str[i];
		if (c == '"') {
			result << "\\\"";
		}
		else {
			result << c;
		}
	}
	return (result.str());
}

string StringUtils::stripslashes(string const str) {
	stringstream result;
	size_t size = str.length();
	unsigned int i = 0;
	for (i = 0; i < size - 1; i++) {
		char c = str[i];
		if (c == '\\' and str[i + 1] == '"') {
			result << '"';
			i++;
		}
		else {
			result << c;
		}
	}
	std::cout << size - i << std::endl;
	if (size - i > 0)
		result << str.substr(i, size - i);

	return (result.str());
}

