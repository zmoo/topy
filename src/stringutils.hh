/*
 *  Copyright (C) 2008 Nicolas Vion <nico@picapo.net>
 *
 *   This file is part of Topy.
 *
 *   Swac-scan is free software; you can redistribute it and/or modify
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

#ifndef _STRINGUTILS_HH
#define _STRINGUTILS_HH

#include <string>
#include <vector>

using namespace std;

namespace StringUtils {
	int to_int(string const input);
	int64_t to_int64(string const input);
	unsigned int to_uint(string const input);
	uint64_t to_uint64(string const input);
	time_t to_time(string const input);
	float to_float(string const input);
	string to_string(int const input);
	string to_string(unsigned int const input);
	string to_string(float const input);
	string to_string(double const input);
	string to_string(uint64_t const input);
	void split(const std::string str, const char separator, std::vector<std::string> &result);
	string chomp(string const inString);
	string rtrim(string const str);
	string upper(string const str);
	string lower(string const str);
	string addslashes(string const str);
	string stripslashes(string const str);
}

#endif
