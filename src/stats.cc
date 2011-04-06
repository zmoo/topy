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

#define _STATS_CC

#include "stats.hh"
#include <sstream>

void Stats::set(std::string const field, Counter const value) {
	mutex.lock();

	List::iterator it = list.find(field);
	if (it != list.end())
		it->second = value;
	else
		list.insert(std::pair<std::string, Counter> (field, value));

	mutex.unlock();
}

void Stats::inc(std::string const field) {
	mutex.lock();

	List::iterator it = list.find(field);
	if (it != list.end())
		it->second++;
	else
		list.insert(std::pair<std::string, Counter> (field, 1));

	mutex.unlock();
}

void Stats::show(std::stringstream &out, std::string const prefix) {
	mutex.lock();

	for (List::iterator it = list.begin(); it != list.end(); it++) {
		out << "STAT " << prefix << it->first << " " << it->second << "\n";
	}

	mutex.unlock();
}

void Stats::serialize_php(std::stringstream &out) {
	mutex.lock();

	out << "a:" << list.size() << ":{";
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		out << "s:" << it->first.size() << ":\"" << it->first << "\";i:" << it->second << ";";
	}
	out << "}";

	mutex.unlock();
}
