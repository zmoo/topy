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

#ifndef _USERS_HH
#define _USERS_HH

#include "config.h"

#include <string>
#include <vector>

#include "ghash++.hh"
#include "pthread++.hh"
#include "top.hh"
#include "contest.hh"
#include "groups.hh"
#include "filter.hh"
#include "topy.h"
#include "user.hh"

class VectorUsers {
private:
	PMutex mutex;
public:
	typedef std::vector<User*> List;
	List list;

	void clear();
	unsigned int group_count(Filter &filter);
	bool top(std::stringstream &out, Filter &filter, TopJoinItems &join, int const field_id, int const size, OutputType const type, int const rule, bool const inversed = false);
	void rank(Contest *contest, Filter &filter, int const field_id, int const rule, bool const inversed = false);
	bool report(std::stringstream &out, Filter &filter, int const field_id, OutputType const type);
	int count_active(Filter &filter, int const field_id, time_t const limit, int &total);
	int cleanup(Filter &filter, int const field_id, time_t const limit, int &total);
	void select(Filter &filter, VectorUsers &result);
	void select_all(VectorUsers &result);
	void clear(Filter &filter, int const field_id);

	void dump_bin(FILE *f);
	void dump(std::filebuf &output);

	void lock();
	bool trylock();
	void unlock();
};

class Users {
private:
	typedef HASH_TABLE_USER_ID HashTableUsers;
	HashTableUsers hash_table;

	VectorUsers vector;
	VectorUsers vector_new_users;

	void vector_update();
	void user_add(User *user);
	User *lookup(UserId const id);

public:
	unsigned int count();
	VectorUsers *get_vector();

	void dump(std::filebuf &output);
	void dump_bin(FILE *f);

	void restore(Parser &parser);
	void restore_bin(FILE *f);

	void select(Filter &filter, VectorUsers &vector);

	User *user_find(UserId const id);
	User *user_find_or_create(UserId const id);

	bool group_del(std::string const name);
	bool groups_clear();

	void groups_stats(std::stringstream &out);
	void debug(std::stringstream &out);
};

#ifdef _USERS_CC
Users users;
#else
extern Users users;
#endif

#endif
