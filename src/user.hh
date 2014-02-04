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

#ifndef _USER_HH
#define _USER_HH

#include <string>
#include <sstream>

#include "topy.h"
#include "field.hh"
#include "parser.hh"
#include "pthread++.hh"
#include "result.hh"
#include "groups.hh"
#include "words_parser.hh"


class User {
private:
	void init();
	bool deleted;

public:
	UserId id;
	GroupId group;

	Field *field[USER_FIELDS_COUNT];

	std::string dump();
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void serialize_php(std::stringstream &out);
	void show(std::stringstream &out);
	std::string summary();
	void clear();
	void del();
	void undel();
	bool is_deleted();
	bool set_group(std::string const name);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);
	void fields_delete();
	void fields_init(bool const alloc = true);

	PMutex *lock();

	User(UserId const id = 0, bool const alloc = true);
	~User();
};


#ifdef _USER_CC
PMutex user_lock[1024];
#else
extern PMutex user_lock[1024];
#endif

#ifdef USER_ID_STR
	#include <cstring>
	#include <cstdlib>

	#define USER_ID_TYPE_NAME "STRING"
	#define USER_ID_COPY(id, value) id = (value) ? strdup(value) : NULL;
	#define USER_ID_TO_STRING(id) std::string(id)
	#define USER_ID_FROM_STRING(id, value) id = strndup(value.data(), value.size());
	#define USER_ID_FREE(id) if (id) free(id);
	#define USER_ID_NULL NULL
	#define USER_ID_SERIALIZE(s, id) s << "s:" << strlen(id) << ":\"" << id << "\";";
	#define HASH_TABLE_KEY(p) p
	#define HASH_TABLE_USER_ID HashTableStr<User>
#else
#ifdef USER_ID_INT64
	#define USER_ID_TYPE_NAME "INTEGER 64"
	#define USER_ID_COPY(id, value) id = value;
	#define USER_ID_TO_STRING(id) StringUtils::to_string(id)
	#define USER_ID_FROM_STRING(id, value) id = StringUtils::to_uint64(value);
	#define USER_ID_FREE(id) id = 0;
	#define USER_ID_NULL 0
	#define USER_ID_SERIALIZE(s, id) s << "i:" << id << ";";
	#define HASH_TABLE_KEY(p) p
	#define HASH_TABLE_USER_ID HashTableInt64<User>
#else
	#define USER_ID_TYPE_NAME "INTEGER"
	#define USER_ID_COPY(id, value) id = value;
	#define USER_ID_TO_STRING(id) StringUtils::to_string(id)
	#define USER_ID_FROM_STRING(id, value) id = StringUtils::to_uint(value);
	#define USER_ID_FREE(id) id = 0;
	#define USER_ID_NULL 0
	#define USER_ID_SERIALIZE(s, id) s << "i:" << id << ";";
	#define HASH_TABLE_KEY(p) (int *) p
	#define HASH_TABLE_USER_ID HashTableInt<User>
#endif
#endif

#endif
