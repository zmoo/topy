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

#ifndef _CONTESTS_HH
#define _CONTESTS_HH

#include "topy.h"

#include <string>
#include <fstream>
#include <map>
#include "parser.hh"
#include "fields.hh"
#include "contest.hh"
#include "client_thread.hh"

class RankThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;
	Contest *contest;
	int rule;
	int field_id;
	bool inversed;

	void main();
	RankThread(Client *client);
};

class Contests {
private:
	typedef std::map<std::string, Contest*> List;
	List list;
	PMutex mutex;

public:
	Contest *add(std::string const name, Contest* set);
	Contest *del(std::string const name, bool const free = true);
	Contest *find(std::string const name);
	Contest *find_or_create(std::string const name);

	void clear(bool const free = true);
	void show(std::stringstream &out);
	void serialize_php(std::stringstream &out);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type);

	void lock();
	void unlock();
};

#ifdef _CONTESTS_CC
Contests contests;
#else
extern Contests contests; 
#endif

#endif

