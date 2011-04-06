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

#ifndef _TOP_HH
#define _TOP_HH

#include "topy.h"
#include "user.hh"

#include <list>
#include <map>
#include <string>
#include <fstream>

typedef struct {
	User *user;
	UserScore score;
} TopItem;

typedef std::pair<int, int> TopJoinItem;
typedef std::list<TopJoinItem> TopJoinItems;
#define TOP_JOIN_ITEM_ALL -2

class TopBase {
protected:
	typedef std::list<TopItem> List;
	List list;
	List::iterator find_user(User *user);
public:
	void add(User *user, UserScore score);
	bool del(User *user);

	void show(std::stringstream &out, int const size, int const users_count);
	void serialize_php(std::stringstream &out, TopJoinItems &join, int unsigned const size, int const users_count, int unsigned const from = 0);
	void inverse_scores();

	void dump(std::filebuf &output);
	int count();
	void clear();
};

class Top : public TopBase {
private:
	UserScore worse_score;
	bool worse_score_def;

	unsigned int size;
	unsigned int users_count;
	void normalize();

public:
	void add(User *user, UserScore const score);
	void finalize();
	void show(std::stringstream &out);
	void serialize_php(std::stringstream &out, TopJoinItems &join);
	Top(int const _size);
};

#endif
