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

#ifndef _FIELDS_HH
#define _FIELDS_HH

#include <map>
#include <list>
#include <string>
#include <fstream>

#include "topy.h"
#include "result.hh"
#include "words_parser.hh"

typedef uint8_t FieldId;
#define FIELD_ID_UNKNOWN 255

class Fields {
public:
	typedef enum {
		EVENTS,
		MARKS,
		INT,
		UINT,
		ULOG,
		LOG,
		TIMESTAMP,
		UNKNOWN
	} FieldType;

private:
	typedef struct {
		std::string name;
		FieldType type;		
	} FieldDef;

	FieldDef fields_def[USER_FIELDS_COUNT];

	typedef std::map<std::string, int> FieldsIndex;
	FieldsIndex fields_index;

	FieldId count;
	bool frozen;
	PMutex mutex;

public:
	void freeze();
	bool add(std::string const name, FieldType const type);
	FieldId get_id(std::string const name);
	std::string get_name(FieldId const id);
	FieldType get_type(FieldId const id);
	std::string get_type_name(FieldType type);
	FieldType get_type(std::string name);
	void serialize_php(std::stringstream &out);
	void dump(std::filebuf &output);
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	FieldId size();
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type);
	void lock();
	void unlock();


	typedef std::list< std::pair<std::string, std::string> > Conf; 
	bool init(Conf &list);
	
	Fields();
};

FieldId parse_field_id(WordsParser *parser);

#ifdef _FIELDS_CC
Fields fields;
#else
extern Fields fields;
#endif

#endif
