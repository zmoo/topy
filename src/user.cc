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

#define _USER_CC

#include <sstream>
#include <iostream>
#include <typeinfo>

#include "user.hh"
#include "stats.hh"
#include "groups_interface.hh"
#include "stringutils.hh"
#include "macros.hh"
#include "replicator.hh"
#include "dump_bin.hh"
#include "help.hh"

#ifdef USER_ID_STR
#include <glib/ghash.h>
#endif

void User::serialize_php(std::stringstream &out) {
	out << "a:" << 2 + fields.size() << ":{";
	out << "s:2:\"id\";";

#ifdef USER_ID_STR
	std::string str = USER_ID_TO_STRING(id);
	out << "s:" << str.size() << ":\"" << str << "\";";
#else
	out << "i:" << id << ";";
#endif
	out << "s:5:\"group\";i:" << group << ";";

	std::string field_name;
	for (int i = 0; i < fields.size(); i++) {
		field_name = fields.get_name(i); 
		out << "s:" << field_name.size() << ":\"" << field_name << "\";";
		field[i]->update();
		field[i]->serialize_php(out);
	}

	out << "}";
}

/** \brief dump information about a User object
 *
 */
std::string User::dump() {
	std::stringstream buf;
	buf << "u{";
#ifdef USER_ID_STR
	buf << "\"" << StringUtils::addslashes(id) << "\"";
#else
	buf << id;
#endif
	buf << ":" << group;

	for (int i = 0; i < fields.size(); i++) {
		buf << ":";
		field[i]->dump(buf);
	}

	buf << "}\n";
	return buf.str();
}

void User::dump_bin(FILE *f) {
#ifdef USER_ID_STR
	DUMP_BIN_CSTR(id, f);
#else
	DUMP_BIN(id, f);
#endif
	DUMP_BIN(group, f);
	for (int i = 0; i < fields.size(); i++) {
		field[i]->dump_bin(f);
	}
}

void User::restore(Parser &parser) {
	parser.waitfor('u');
	parser.waitfor('{');
#ifdef USER_ID_STR
	std::string str = parser.read_str();
	USER_ID_FROM_STRING(id, str);
#else
#ifdef USER_ID_INT64
	id = parser.read_uint64();
#else
	id = parser.read_int();
#endif
#endif
	parser.waitfor(':');
	group = parser.read_int();

	for (int i = 0; i < fields.size(); i++) {
		parser.waitfor(':');
		field[i]->restore(parser);
	}

	parser.waitfor('}');
}

void User::restore_bin(FILE *f) {
#ifdef USER_ID_STR
	RESTORE_BIN_CSTR(id, f);
#else
	RESTORE_BIN(id, f);
#endif
	RESTORE_BIN(group, f);
	for (int i = 0; i < fields.size(); i++) {
		field[i]->restore_bin(f);
	}
}

PMutex *User::lock() {
#ifdef USER_ID_STR
	unsigned int i = g_str_hash(id);
#else
	unsigned int i = id;
#endif
	PMutex *mutex = &user_lock[i & 0x3FF];
	mutex->lock();
	return mutex;
}

std::string User::summary() {
	std::stringstream result;
	for (FieldId i = 0; i < fields.size(); i++) {
		result << "\t " << fields.get_name(i) << " : " << field[i]->summary();
	}
	return result.str();
}

void User::show(std::stringstream &out) {
	out << "Id: " << id << std::endl;
	out << "Group: #" << group << " (" << groups.get_name(group) << ")" << std::endl;

	for (FieldId i = 0; i < fields.size(); i++) {
		field[i]->update();

		out << fields.get_name(i) << " :" << std::endl;
		out << "=======(last update: " << field[i]->last_update() << ")" << std::endl;
		field[i]->show(out);
	}
}

void User::init() {
	group = GROUP_UNDEF;
	deleted = false;
}

void User::clear() {
	init();
	for (FieldId i = 0; i < fields.size(); i++) {
		field[i]->clear();
	}
}

void User::del() {
	fields_delete();
	deleted = true;
}

void User::undel() {
	fields_init();
	init();
}

bool User::is_deleted() {
	return deleted;
}

void User::fields_init(bool const alloc) {
	fields.lock();
	fields.freeze();
	for (FieldId i = 0; i < fields.size(); i++) {
		Fields::FieldType type = fields.get_type(i);
		switch (type) {
			case Fields::MARKS:
				field[i] = new FieldMarks(alloc);
				break;
			case Fields::EVENTS:
				field[i] = new FieldEvents(alloc);
				break;
			case Fields::INT:
				field[i] = new FieldInt();
				break;
			case Fields::UINT:
				field[i] = new FieldInt(true);
				break;
			case Fields::ULOG:
				field[i] = new FieldUlog();
				break;
			case Fields::LOG:
				field[i] = new FieldLog();
				break;
			case Fields::TIMESTAMP:
				field[i] = new FieldTimestamp();
				break;
			case Fields::UNKNOWN:
				break;
		}
	}
	fields.unlock();
}

void User::fields_delete() {
	for (FieldId i = 0; i < fields.size(); i++) {
		delete field[i];
	}
}

User::User(UserId const _id, bool const alloc) {
	init();
	USER_ID_COPY(id, _id);
	fields_init(alloc);
}

User::~User() {
	USER_ID_FREE(id);
	fields_delete();
}

bool User::set_group(std::string const name) {
	if (name == "") {
		group = GROUP_UNDEF;
		return true;
	}

	groups.lock();
	Groups::Filter filter = groups.filter_parse(name);
	groups.unlock();

	if (filter.id == GROUP_UNKNOWN)
		return false;

	group = filter.id;
	return true;
}


bool User::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {
	//!:: <field> <command>
	//!	Execute <command> on field <field> 
	if (parser->current == "::") {
		std::string field_name = parser->next();
		FieldId field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}

		replication_query << " :: " << field_name;
		return field[field_id]->parse_query(result, cmd_prefix + field_name + "::", parser, replication_query);
	}

	//!get
	//!	Get user data
	else if (parser->current == "get") {
		stats.inc(cmd_prefix + "get");

		PARSING_END(parser, result);
		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		result.send();
		return true;
	}

	//!show
	//!	Show user data
	else if (parser->current == "show") {
		stats.inc(cmd_prefix + "show");

		PARSING_END(parser, result);
		show(result.data);
		result.send();
		return true;
	}

	else if (parser->current == "group") {
		parser->next();

		//!group set <name>
		//!	Set user group
		if (parser->current == "set") {
			stats.inc(cmd_prefix + "group::set");

			std::string name = parser->next();
			PARSING_END(parser, result);

			if (!set_group(name)) {
				RETURN_PARSE_ERROR(result, "Not a valid field name.");
			}

			//replication
			if (replicator.opened and !result.replicated) {
				replication_query << " group set " << name;
				replicator.add(replication_query);
			}

			result.send();
			return true;
		}

		//!group get
		//!	Get user group
		else if (parser->current == "get") {
			stats.inc(cmd_prefix + "group::get");

			PARSING_END(parser, result);
			/*
			result.type = PHP_SERIALIZE;
			std::string str = groups.get_name(group);
			result.data << "s:" << str.size() << ":\"" << str.data() << "\";";
			*/
			result.type = TEXT;
			result.data << groups.get_name(group);
			result.send();
			return true;
		}
		RETURN_NOT_VALID_CMD(result);
	}

	//!delete
	//!	Delete user
	else if (parser->current == "delete") {
		stats.inc(cmd_prefix + "delete");

		PARSING_END(parser, result);
		del();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " delete";
			replicator.add(replication_query);
		}

		result.send();
		return true;
	}

	//!clear
	//!	Clear user
	else if (parser->current == "clear") {
		stats.inc(cmd_prefix + "clear");

		PARSING_END(parser, result);
		clear();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " clear";
			replicator.add(replication_query);
		}
		result.send();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_USER;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

