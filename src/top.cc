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

#include <iostream>

#include "users.hh"
#include "top.hh"
#include "stringutils.hh"

bool compare_items(TopItem first, TopItem second) {
	return (first.score > second.score);
}

TopBase::List::iterator TopBase::find_user(User *user) {
	for (List::iterator it = list.begin(); it != list.end(); it++)
		if (it->user == user)
			return it;
	return list.end();
}

void TopBase::add(User *user, UserScore score) {
	TopItem item;
	item.user = user;
	item.score = score;
	list.push_back(item);
}

bool TopBase::del(User *user) {
	List::iterator it = find_user(user);
	if (it == list.end()) 
		return false;
	list.erase(it);
	return true;
}

void TopBase::serialize_php(std::stringstream &out, TopJoinItems &join, int unsigned const size, int const users_count, int unsigned const from) {
	unsigned int final_size = (from < list.size()) ? MIN(size, list.size() - from) : 0;

	out << "a:2:{";
	out << "s:11:\"users_count\";i:" << users_count << ";";
	out << "s:4:\"list\";a:" << final_size << ":{";

	if (final_size != 0) {
		List::iterator it = list.begin();
		for (unsigned int i = 0; i < from; i++) {
			it++;
		}

		bool output_join = (join.size() != 0);

		for (unsigned int i = 0; i < final_size; i++) {
			User *user = it->user;
			PMutex *mutex = user->lock();

			out << "i:" << (i + from) << ";";
			if (!user->is_deleted()) {
				out << "a:" << (output_join ? 3 : 2) << ":{";
				out << "s:5:\"score\";i:" << it->score << ";";
				out << "s:4:\"user\";a:1:{s:2:\"id\";";
				USER_ID_SERIALIZE(out, user->id);
				out << "}";

				if (output_join) {
					out << "s:4:\"join\";a:" << join.size() << ":{";
					int i = 0;
					for (TopJoinItems::iterator it2 = join.begin(); it2 != join.end(); it2++) {
						out << "i:" << i << ";";
						if (it2->second == TOP_JOIN_ITEM_ALL)
							it->user->field[it2->first]->serialize_php(out);
						else
							out << "i:" << it->user->field[it2->first]->score(it2->second) << ";";
						++i;
					}
					out << "}";
				}
				out << "}";
			}
			else {
				out << "b:0;";
			}

			mutex->unlock();
			it++;
		}
	}
	out << "}}";
}

void TopBase::show(std::stringstream &out, int const size, int const users_count) {
	List::iterator it = list.begin();
	for (int i = 0; i < size and it != list.end(); i++) {
		out << "user: " << it->user->id << "\tscore: " << it->score << it->user->summary() << std::endl;
		it++;
	}

	out << "total users: " << users_count;
}

void TopBase::inverse_scores() {
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		it->score = it->score * -1;
	}
}

void TopBase::clear() {
	list.clear();
}

void TopBase::dump(std::filebuf &output) {
	std::string str;

	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it != list.begin())
			output.sputc(',');

		str = USER_ID_TO_STRING(it->user->id);
		output.sputn(str.data(), str.size());
	}
}

int TopBase::count() {
	return list.size();
}

void Top::normalize() {
	list.sort(compare_items);
	
	if (list.size() > size)
		list.resize(size);

	worse_score_def = true;
	worse_score = list.back().score;
}

void Top::add(User *user, UserScore const score) {
	users_count++;

	if (!worse_score_def or score > worse_score) {
		TopBase::add(user, score);
		if (list.size() > size * 2)
			normalize();
	}
}

void Top::finalize() {
	normalize();
}

void Top::show(std::stringstream &out) {
	TopBase::show(out, size, users_count);
}

void Top::serialize_php(std::stringstream &out, TopJoinItems &join) {
	TopBase::serialize_php(out, join, size, users_count);
}

Top::Top(int const _size) {
	size = _size;
	users_count = 0;
	worse_score_def = false;
}

