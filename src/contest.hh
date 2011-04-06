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

#ifndef _CONTEST_HH
#define _CONTEST_HH

#include "user.hh"
#include "top.hh"
#include "parser.hh"
#include "client_thread.hh"

class Contest : public TopBase {
private:
	void sort();
	PMutex mutex;

public:
	int size();
	void add(User *user, UserScore const score);
	void finalize();
	void serialize_php(std::stringstream &out, TopJoinItems &join, unsigned int const limit, unsigned int const from = 0);
	void show(std::stringstream &out, int const size);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type);
	int find(User *user);

	void lock();
	void unlock();
};

class ContestGetThread : public ClientThread {
private:
	void main();
public:
	Contest *contest;
	unsigned int from;
	unsigned int limit;
	TopJoinItems join;

	ContestGetThread(Client *client);
};

class ContestClearThread : public ClientThread {
private:
	void main();

public:
	Contest *contest;
	ContestClearThread(Client *client);
};

class ContestFindThread : public ClientThread {
private:
	void main();

public:
	Contest *contest;
	User *user;
	ContestFindThread(Client *client);
};

class ContestSizeThread : public ClientThread {
private:
	void main();

public:
	Contest *contest;
	ContestSizeThread(Client *client);
};

#endif

