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

#include "groups.hh"

#include <sstream>

#include "stringutils.hh"
#include "dump_bin.hh"

GroupId Groups::del(std::string const name) {
	List::iterator it = list.find(name);
	if (it == list.end())
		return GROUP_UNKNOWN;

	GroupId id = it->second.id;
	list.erase(it);

	return id;
}

GroupId Groups::add(std::string const name) {
	std::pair<List::iterator, bool> result;
	Group group = {index_counter, 0};
	result = list.insert(std::pair<std::string, Group> (name, group));

	if (result.second)
		index_counter++;

	return result.first->second.id;
}

GroupId Groups::add(std::string const name, GroupId const id, GroupId const mask) {
	std::pair<List::iterator, bool> result;
	result = list.insert(std::pair<std::string, Group> (name, (Group) {id, mask}));

	return result.first->second.id;
}

GroupId Groups::get_id(std::string const name) {
	List::iterator it = list.find(name);
	if (it == list.end())
		return GROUP_UNKNOWN;
	return it->second.id;
}

GroupId Groups::get_mask(std::string const name) {
	List::iterator it = list.find(name);
	if (it == list.end())
		return 0;
	return it->second.mask;
}

Groups::Group Groups::get(std::string const name) {
	List::iterator it = list.find(name);
	if (it == list.end())
		return (Group) {GROUP_UNKNOWN, 0};
	return it->second;
}

Groups::List::iterator Groups::find(GroupId const id) {
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it->second.id == id)
			return it;
	}
	return list.end();
}

std::string Groups::get_name(GroupId const id) {
	List::iterator it = find(id);
	if (it != list.end() and it->second.mask == 0) {
		return it->first;
	}

	int count = 0;
	std::stringstream result;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it->second.mask != 0 and ((id & it->second.mask) == it->second.id)) {
			count++;
			if (count > 1)
				result << "+";
			result << it->first;
		}
	}
	return result.str();
}

Groups::Filter Groups::filter_parse(std::string const str) {
	std::vector<std::string> tbl;
	StringUtils::split(str, '+', tbl);

	Filter filter = {0, 0};
	for (std::vector<std::string>::iterator part_it = tbl.begin(); part_it != tbl.end(); part_it++) {
		List::iterator group_it = list.find(*part_it);
		if (group_it != list.end()) {
			Group group = group_it->second;
			if (group.mask == 0)
				return group;

			filter.mask |= group.mask;
			filter.id |= group.id;
		}
		else {
			return (Filter) {GROUP_UNKNOWN, 0};
		}
	}
	return filter;
}

void Groups::serialize_php(std::stringstream &out) {
	out << "a:" << list.size() << ":{";
	int i = 0;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		out << "i:" << i << ";";
		out << "a:3:{";
		out << "s:4:\"name\";";
		out << "s:" << it->first.size() << ":\"" << it->first << "\";";
		out << "s:2:\"id\";";
		out << "i:" << it->second.id << ";";
		out << "s:4:\"mask\";";
		out << "i:" << it->second.mask << ";";
		out << "}";
		i++;
	}
	out << "}";
}

void Groups::show(std::stringstream &out) {
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it != list.begin())
			out << std::endl;
		out << it->first << "\t#" << it->second.id;
		if (it->second.mask != 0)
			out << "\t(mask: " << it->second.mask << ")";
	}
}

void Groups::dump(std::filebuf &output) {
	std::string str;

	output.sputc('G');
	output.sputc('{');
	str = StringUtils::to_string((int) list.size());
	output.sputn(str.data(), str.size());
	output.sputc(':');
	output.sputc('\n');

	for (List::iterator it = list.begin(); it != list.end(); it++) {
		output.sputn("g{", 2);
		output.sputc('"');
		output.sputn(it->first.data(), it->first.size());
		output.sputc('"');
		output.sputc(':');
		str = StringUtils::to_string(it->second.id);
		output.sputn(str.data(), str.size());
		output.sputc(':');
		str = StringUtils::to_string(it->second.mask);
		output.sputn(str.data(), str.size());
		output.sputn("}\n", 2);
	}

	output.sputc('}');
	output.sputc('\n');
}

void Groups::dump_bin(FILE *f) {
	uint32_t count = list.size();
	DUMP_BIN(count, f);
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		dump_bin_magic(f, DUMP_BIN_GROUP_MAGIC);
		DUMP_BIN_STR(it->first, f);
		GroupId id, mask;
		id = it->second.id;
		mask = it->second.mask;
		DUMP_BIN(id, f);
		DUMP_BIN(mask, f);
	}
}

void Groups::restore_group(std::string const name, GroupId const id, GroupId const mask) {
	Group group = {id, mask};
	list.insert(std::pair<std::string, Group> (name, group));
	if (index_counter <= id)
		index_counter = id + 1;
}

void Groups::restore(Parser &parser) {
	parser.waitfor('G');
	parser.waitfor('{');
	unsigned int count = parser.read_int();
	parser.waitfor(':');
	parser.waitfor('\n');

	for (unsigned int i = 0; i < count; i++) {
		parser.waitfor('g');
		parser.waitfor('{');
		parser.waitfor('"');
		std::string name = parser.read_until('"');
		parser.waitfor('"');
		parser.waitfor(':');
		GroupId id = parser.read_int();
		parser.waitfor(':');
		GroupId mask = parser.read_int();

		restore_group(name, id, mask);

		parser.waitfor('}');
		parser.waitfor('\n');
	}

	parser.waitfor('}');
	parser.waitfor('\n');
}

void Groups::restore_bin(FILE *f) {
	uint32_t count = 0;
	RESTORE_BIN(count, f);
	for (uint32_t i = 0; i < count; i++) {
		restore_bin_magic(f, DUMP_BIN_GROUP_MAGIC);
		std::string name;
		RESTORE_BIN_STR(name, f);
		GroupId id , mask;
		RESTORE_BIN(id, f);
		RESTORE_BIN(mask, f);
		restore_group(name, id, mask);
	}
}

void Groups::clear() {
	list.clear();
	Groups();
}

Groups::Groups() {
	index_counter = 1;
}
