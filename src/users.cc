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

#define _USERS_CC

#include "users.hh"

#include <iostream>
#include <fstream>
#include <map>

#include "users_sets.hh"
#include "parser.hh"
#include "stringutils.hh"
#include "groups_interface.hh"
#include "dump_bin.hh"

void VectorUsers::lock() {
	mutex.lock();
}

bool VectorUsers::trylock() {
	return mutex.trylock();
}

void VectorUsers::unlock() {
	mutex.unlock();
}

void VectorUsers::clear() {
	lock();
	list.clear();
	unlock();
}

unsigned int VectorUsers::group_count(Filter &filter) {
	unsigned int count = 0;
	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted()) {
			PMutex *mutex =	(*it)->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				count++;
			}
			mutex->unlock();
		}
	}
	unlock();
	return count;
}

bool VectorUsers::top(std::stringstream &out, Filter &filter, TopJoinItems &join, int const field_id, int const size, OutputType const type, int const rule, bool const inversed) {
	Top top(size);

	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted()) {
			PMutex *mutex =	(*it)->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				UserScore score = (*it)->field[field_id]->score(rule);
				if (inversed)
					score = score * -1;
				top.add(*it, score);
			}
			mutex->unlock();
		}
	}
	unlock();
	top.finalize();
	if (inversed) {
		top.inverse_scores();
	}

	switch (type) {
		case TEXT:
			top.show(out);
			return true;
		default:
			top.serialize_php(out, join);
			return true;
	}
}

void VectorUsers::rank(Contest *contest, Filter &filter, int const field_id, int const rule, bool const inversed) {
	contest->clear();
	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted()) {
			PMutex *mutex =	(*it)->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				UserScore score = (*it)->field[field_id]->score(rule);
				if (score != -1) {
					if (inversed)
						score = score * -1;
					contest->add(*it, score);
				}
			}
			mutex->unlock();
		}
	}
	unlock();
	contest->finalize();
	if (inversed) {
		contest->inverse_scores();
	}
}

bool VectorUsers::report(std::stringstream &out, Filter &filter, int const field_id, OutputType const type) {
	ReportField *report = NULL;

	switch (fields.get_type(field_id)) {
		case  Fields::INT:
			report = new ReportFieldInt;
			break;
		case  Fields::UINT:
			report = new ReportFieldInt;
			break;
		case Fields::EVENTS:
			report = new ReportFieldEvents;
			break;
		default:
			return false;
			break;
	}
	
	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted()) {
			PMutex *mutex =	(*it)->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				report->add((*it)->field[field_id]);
			}
			mutex->unlock();
		}
	}
	unlock();

	switch (type) {
		case TEXT:
			report->show(out);
			return true;
		default:
			report->serialize_php(out);
			return true;
	}

	delete report;
	return true;
}

void VectorUsers::clear(Filter &filter, int const field_id) {
	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted()) {
			PMutex *mutex =	(*it)->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				(*it)->field[field_id]->clear();
			}
			mutex->unlock();
		}
	}
	unlock();
}

int VectorUsers::count_active(Filter &filter, int const field_id, time_t const limit, int &total) {
	total = 0;
	int result = 0;

	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		User *user = (*it);
		if (!user->is_deleted()) {
			PMutex *mutex =	user->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				total++;
				if (user->field[field_id]->last_update() > limit)
					result++;
			}
			mutex->unlock();
		}
	}
	unlock();
	return result;
}

int VectorUsers::cleanup(Filter &filter, int const field_id, time_t const limit, int &total) {
	total = 0;
	int result = 0;

	lock();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		User *user = (*it);
		if (!user->is_deleted()) {
			PMutex *mutex =	user->lock();
			if (!filter.is_defined() or filter.eval(*it)) {
				total++;
				if (user->field[field_id]->last_update() < limit) {
					result++;
					user->del();
				}
			}
			mutex->unlock();
		}
	}
	unlock();
	return result;
}

void VectorUsers::select(Filter &filter, VectorUsers &result) {
	lock();
	result.lock();
	result.list.clear();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted()) {
			if (!filter.is_defined() or filter.eval(*it))
				result.list.push_back(*it);
		}
	}
	result.unlock();
	unlock();
}

void VectorUsers::select_all(VectorUsers &result) {
	lock();
	result.lock();
	result.list.clear();
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (!(*it)->is_deleted())
			result.list.push_back(*it);
	}
	result.unlock();
	unlock();
}

void VectorUsers::dump(std::filebuf &output) {
	lock();

	output.sputc('U');
	output.sputc('{');
	std::string str;
	str = StringUtils::to_string((unsigned int) list.size());
	output.sputn(str.data(), str.size());
	output.sputc(':');
	output.sputc('\n');

	User *user;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		user = *it;

		PMutex *mutex = user->lock();
		if (user->is_deleted())
			user->undel();

		str = user->dump();
		mutex->unlock();

		output.sputn(str.data(), str.size());
	}
	output.sputc('}');
	output.sputc('\n');

	unlock();
}

void VectorUsers::dump_bin(FILE *f) {
	lock();

	//number of users
	uint32_t count = list.size();
	DUMP_BIN(count, f);

	//users
	User *user;
	for (VectorUsers::List::iterator it = list.begin(); it != list.end(); it++) {
		dump_bin_magic(f, DUMP_BIN_USER_MAGIC);
		user = *it;
		PMutex *mutex = user->lock();
		if (user->is_deleted())
			user->undel();

		user->dump_bin(f);
		mutex->unlock();
	}
	unlock();
}

void Users::user_add(User *user) {
	hash_table.add(HASH_TABLE_KEY(&user->id), user);

	if (vector.trylock()) {
		vector_update();
		vector.list.push_back(user);
		vector.unlock();
	}
	else {
		vector_new_users.lock();
		vector_new_users.list.push_back(user);
		vector_new_users.unlock();
	}
}

User *Users::lookup(UserId const id) {
	return hash_table.lookup(id);
}

void Users::dump(std::filebuf &output) {
	vector.lock();
	vector_update();
	vector.unlock();

	VectorUsers vector_copy;
	vector.select_all(vector_copy);

	vector_copy.dump(output);
}

void Users::dump_bin(FILE *f) {
	vector.lock();
	vector_update();
	vector.unlock();

	VectorUsers vector_copy;
	vector.select_all(vector_copy);

	vector_copy.dump_bin(f);
}

void Users::restore(Parser &parser) {
	User *user;

	parser.waitfor('U');
	parser.waitfor('{');
	int count = parser.read_int();
	parser.waitfor(':');
	parser.waitfor('\n');

	vector.lock();
	for (int i = 0; i < count; i++) {
		user = new User(USER_ID_NULL, false);
		user->restore(parser);
		hash_table.add(HASH_TABLE_KEY(&user->id), user);
		vector.list.push_back(user);
		parser.waitfor('\n');
	}
	vector.unlock();

	parser.waitfor('}');
	parser.waitfor('\n');
}

void Users::restore_bin(FILE *f) {
	User *user;

	uint32_t count;
	RESTORE_BIN(count, f);

	vector.lock();
	for (uint32_t i = 0; i < count; i++) {
		if (!restore_bin_magic(f, DUMP_BIN_USER_MAGIC))
			restore_bin_error("Invalid magic number");

		user = new User(0, false);
		user->restore_bin(f);
		hash_table.add(HASH_TABLE_KEY(&user->id), user);
		vector.list.push_back(user);
	}
	vector.unlock();
}

void Users::vector_update() {
	vector_new_users.lock();
	for (VectorUsers::List::iterator it = vector_new_users.list.begin(); it != vector_new_users.list.end(); it++) {
		vector.list.push_back(*it);
	}
	vector_new_users.list.clear();
	vector_new_users.unlock();
}

void Users::select(Filter &filter, VectorUsers &result) {
	vector.lock();
	vector_update();
	vector.unlock();

	vector.select(filter, result);
}

VectorUsers *Users::get_vector() {
	return &vector;
}

bool Users::group_del(std::string const name) {
	groups.lock();
	GroupId group = groups.del(name);
	groups.unlock();

	if (group == GROUP_UNKNOWN)
		return false;

	vector.lock();
	vector_update();
	for (VectorUsers::List::iterator it = vector.list.begin(); it != vector.list.end(); it++) {
		if ((*it)->group == group)
			(*it)->group = GROUP_UNDEF;
	}
	vector.unlock();
	return true;
}

bool Users::groups_clear() {
	groups.lock();
	groups.clear();
	groups.unlock();

	vector.lock();
	vector_update();
	for (VectorUsers::List::iterator it = vector.list.begin(); it != vector.list.end(); it++) {
		(*it)->group = GROUP_UNDEF;
	}
	vector.unlock();
	return true;
}

void Users::groups_stats(std::stringstream &out) {
	typedef std::map<GroupId, unsigned int> Stats;
	Stats stats;

	//count users in each group
	vector.lock();
	for (VectorUsers::List::iterator it = vector.list.begin(); it != vector.list.end(); it++) {
		Stats::iterator group = stats.find((*it)->group);
		if (group == stats.end()) {
			stats.insert(std::pair<GroupId, unsigned int> ((*it)->group, 1));
		}
		else {
			group->second++;
		}
	}
	vector.unlock();

	//output result
	groups.lock();
	for (Stats::iterator it = stats.begin(); it != stats.end(); it++)  {
		if (it != stats.begin())
			out << "\n";
		out << ((it->first == GROUP_UNDEF) ? "UNDEFINED" : groups.get_name(it->first)) << "\t#" << it->first << "\t" << it->second;
	}
	groups.unlock();
}

unsigned int Users::count() {
	return hash_table.size();
}

User *Users::user_find(UserId const id) {
	User *user = hash_table.lookup(id);
	if (!user or user->is_deleted()) {
		return NULL;
	}
	return user;
}

User *Users::user_find_or_create(UserId const id) {
	User *user = hash_table.lookup(id);
	if (!user) {
		user = new User(id);
		user_add(user);
	}
	else if (user->is_deleted())
		user->undel();

	return user;
}

void Users::debug(std::stringstream &out) {
	out << "vector.size() : " << vector.list.size() << std::endl;
	out << "vector.capacity() : " << vector.list.capacity() << std::endl;
	out << "vector_new_users.size() : " << vector_new_users.list.size() << std::endl;
	out << "vector_new_users.capacity() : " << vector_new_users.list.capacity() << std::endl;
	out << "hash_table.size() : " << hash_table.size() << std::endl;
}

