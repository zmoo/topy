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

#define _USERS_SETS_CC

#include "users_sets.hh"

#include "stats.hh"
#include "filter.hh"
#include "client_thread.hh"


VectorUsers *UsersSets::add(std::string const name, VectorUsers* set) {
	std::pair<List::iterator, bool> result;
	result = list.insert(std::pair<std::string, VectorUsers*> (name, set));
	return (result.second) ? (result.first->second) : NULL;
}

VectorUsers *UsersSets::find(std::string const name) {
	List::iterator it;
	it = list.find(name);

	if (it != list.end())
		return it->second;

	return NULL;
}

VectorUsers *UsersSets::find_or_create(std::string const name) {
	VectorUsers *result = find(name);
	if (!result) {
		VectorUsers *list_users = new VectorUsers;
		result = add(name, list_users);
	}
	return result;
}

VectorUsers *UsersSets::del(std::string const name, bool const free) {
	List::iterator it = list.find(name);
	if (it == list.end())
		return NULL;

	list.erase(it);
	VectorUsers *vector = it->second;
	if (free)
		delete vector;

	return vector;
}

void UsersSets::clear(bool const free) {
	if (free) {
		for (List::iterator it = list.begin(); it != list.end(); it++) {
			VectorUsers *vector = it->second;
			delete vector;
		}
	}
	list.clear();
}

void UsersSets::show(std::stringstream &out) {
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it != list.begin())
			out << std::endl;
		out << it->first << "\t" << it->second->list.size();
	}
}

void UsersSets::serialize_php(std::stringstream &out) {
	out << "a:" << list.size() << ":{";
	int i = 0;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		out << "i:" << i << ";";
		out << "a:2:{";
		out << "s:4:\"name\";";
		out << "s:" << it->first.size() << ":\"" << it->first << "\";";
		out << "s:5:\"count\";";
		out << "i:" << it->second->list.size() << ";";
		out << "}";
		i++;
	}
	out << "}";
}

bool UsersSets::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type) {
	//!select into <name> where <expr>
	//!	Select all users that match <expr> into set <name>
	if (parser->current == "select") {
		stats.inc(cmd_prefix + "select");

		if (parser->next() != "into") {
			RETURN_PARSE_ERROR(result, "Expected: 'into'");
		}
		std::string name = parser->next();

		SetsSelectThread *thread = new SetsSelectThread(result.get_client());
		thread->vector = find_or_create(name);

		parser->next();
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!delete <name>
	//!	Delete user set
	else if (parser->current == "delete") {
		stats.inc(cmd_prefix + "delete");

		std::string name = parser->next();
		PARSING_END(parser, result);

		VectorUsers *vector = del(name, false);
		if (vector == NULL) {
			RETURN_PARSE_ERROR(result, "Not a valid users set name");
		}

		SetsClearThread *thread = new SetsClearThread(result.get_client());
		thread->vector = vector;
		thread->run();
		return true;
	}

	//!list
	//!	Show list of sets
	else if (parser->current == "list") {
		stats.inc(cmd_prefix + "list");

		PARSING_END(parser, result);
		result.type = type;
		switch (type) {
			case TEXT:
				show(result.data);
				break;
			default:
				serialize_php(result.data);
				break;
		}
		result.send();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_USERS_SETS;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void UsersSets::lock() {
	mutex.lock();
}

void UsersSets::unlock() {
	mutex.unlock();
}

