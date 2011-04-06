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

#define _FIELDS_CC

#include <iostream>
#include "log.hh"
#include "stats.hh"
#include "stringutils.hh"

bool Fields::add(std::string const name, FieldType const type) {
	if (frozen) {
		log.msg(LOG_ERR, "Can not create field. Fields structure is frozen.");
		return false;
	}

	if (count >= USER_FIELDS_COUNT) {
		log.msg(LOG_ERR, "Can not create field \"" + name + "\". Max fields number is: " + StringUtils::to_string(USER_FIELDS_COUNT));
		return false;
	}

	if (get_id(name) != FIELD_ID_UNKNOWN) {
		log.msg(LOG_ERR, "Field name already exists");
		return false;
	}

	fields_def[count].name = name;
	fields_def[count].type = type;
	fields_index.insert(std::pair<std::string, int> (name, count));
	count++;
	return true;
}

FieldId Fields::get_id(std::string const name) {
	FieldsIndex::iterator it = fields_index.find(name);
	if (it == fields_index.end())
		return FIELD_ID_UNKNOWN;
	return it->second;
}

Fields::FieldType Fields::get_type(FieldId const id) {
	if (id < count)
		return fields_def[id].type;
	return UNKNOWN;
}

std::string Fields::get_name(FieldId const id) {
	if (id < count)
		return fields_def[id].name;
	return "UNDEF";
}

FieldId Fields::size() {
	return count;
}

void Fields::serialize_php(std::stringstream &out) {
	out << "a:" << (int) count << ":{";

	for (FieldId i = 0; i < count; i++) {
		std::string name = fields_def[i].name;
		std::string type_name = get_type_name(fields_def[i].type);

		out << "s:" << name.size() << ":\"" << name << "\";";
		out << "s:" << type_name.size() << ":\"" << type_name << "\";";
	}
	out << "}";
}

std::string Fields::get_type_name(FieldType type) {
	switch (type) {
		case EVENTS:
			return FIELD_EVENTS_TYPE_NAME;
		case MARKS:
			return FIELD_MARKS_TYPE_NAME;
		case INT:
			return FIELD_INT_TYPE_NAME;
		case UINT:
			return FIELD_UINT_TYPE_NAME;
		case ULOG:
			return FIELD_ULOG_TYPE_NAME;
		case LOG:
			return FIELD_LOG_TYPE_NAME;
		case TIMESTAMP:
			return FIELD_TIMESTAMP_TYPE_NAME;
		default:
			return "UNKNOWN";
	}
}

Fields::FieldType Fields::get_type(std::string name) {
	if (name == FIELD_EVENTS_TYPE_NAME)
		return EVENTS;
	else if (name == FIELD_MARKS_TYPE_NAME)
		return MARKS;
	else if (name == FIELD_INT_TYPE_NAME)
		return INT;
	else if (name == FIELD_UINT_TYPE_NAME)
		return UINT;
	else if (name == FIELD_ULOG_TYPE_NAME)
		return ULOG;
	else if (name == FIELD_LOG_TYPE_NAME)
		return LOG;
	else if (name == FIELD_TIMESTAMP_TYPE_NAME)
		return TIMESTAMP;
	else
		return UNKNOWN;
}

bool Fields::init(Conf &list) {
	Conf::iterator it;

	for (it = list.begin(); it != list.end(); it++) {
		std::string name = it->first;
		std::string type_name = it->second;

		FieldType type = get_type(type_name);
		if (type == UNKNOWN) {
			std::cerr << "Unknown field type: " << type_name << " (" << name << ")" << std::endl;
			return false;
		}

		add(name, type);
	}
	return true;
}

void Fields::dump(std::filebuf &output) {
	std::string str;

	output.sputc('F');
	output.sputc('{');
	str = StringUtils::to_string(count);
	output.sputn(str.data(), str.size());
	output.sputc(':');
	output.sputc('\n');

	for (FieldId i = 0; i < count; i++) {

		std::string name = fields_def[i].name;
		std::string type_name = get_type_name(fields_def[i].type);

		output.sputn("f{", 2);
		output.sputc('"');
		output.sputn(name.data(), name.size());
		output.sputc('"');
		output.sputc(':');
		output.sputc('"');
		output.sputn(type_name.data(), type_name.size());
		output.sputc('"');
		output.sputn("}\n", 2);
	}

	output.sputc('}');
	output.sputc('\n');
}

void Fields::dump_bin(FILE *f) {
	DUMP_BIN(count, f);
	for (FieldId i = 0; i < count; i++) {
		std::string name = fields_def[i].name;
		std::string type_name = get_type_name(fields_def[i].type);

		restore_bin_magic(f, DUMP_BIN_FIELD_MAGIC);
		DUMP_BIN_STR(name, f);
		DUMP_BIN_STR(type_name, f);
	}
}

void Fields::restore(Parser &parser) {
	if (!parser.test_next('F'))
		return;

	parser.waitfor('{');
	FieldId c = parser.read_int();
	parser.waitfor(':');
	parser.waitfor('\n');

	for (FieldId i = 0; i < c; i++) {
		parser.waitfor('f');
		parser.waitfor('{');
		parser.waitfor('"');
		std::string name = parser.read_until('"');
		parser.waitfor('"');
		parser.waitfor(':');
		parser.waitfor('"');
		std::string type_name = parser.read_until('"');
		parser.waitfor('"');
		parser.waitfor('}');
		parser.waitfor('\n');

		if (i >= count or get_type(type_name) != fields_def[i].type) {
			log.msg(LOG_ERR, "Invalid fields structure", true);
		}
		else if (name != fields_def[i].name) {
			log.msg(LOG_WARNING, "Field name has changed: '" + name + "' != '" + fields_def[i].name + "'", true);
		}
	}

	parser.waitfor('}');
	parser.waitfor('\n');
}

void Fields::restore_bin(FILE *f) {
	FieldId c;
	RESTORE_BIN(c, f);
	for (FieldId i = 0; i < c; i++) {
		std::string name, type_name;
		RESTORE_BIN_STR(name, f);
		RESTORE_BIN_STR(type_name, f);

		if (i >= count or get_type(type_name) != fields_def[i].type) {
			restore_bin_error("Invalid fields structure");
		}
		else if (name != fields_def[i].name) {
			log.msg(LOG_WARNING, "Field name has changed: '" + name + "' != '" + fields_def[i].name + "'", true);
		}
	}
}

bool Fields::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type) {
	//!add <name> <type>
	//!	Add field
	if (parser->current == "add") {
		stats.inc(cmd_prefix + "add");

		std::string name = parser->next();
		std::string type_name = parser->next();
		PARSING_END(parser, result);

		FieldType type = get_type(type_name);
		if (type == UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Invalid field type");
		}
		bool done = add(name, type);

		result.type = PHP_SERIALIZE;
		result.data << "b:" << (done ? 1 : 0) << ";";
		result.send();
		return true;
	}

	//!list
	//!	List fields
	else if (parser->current == "list") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		result.send();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_FIELDS;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void Fields::freeze() {
	frozen = true;
}

void Fields::lock() {
	mutex.lock();
}

void Fields::unlock() {
	mutex.unlock();
}

Fields::Fields() : frozen(false) {
	count = 0;
}

FieldId parse_field_id(WordsParser *parser) {
	fields.lock();
	FieldId id = fields.get_id(parser->current);
	fields.unlock();
	parser->next();
	return id;
}

