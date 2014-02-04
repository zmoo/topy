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

#ifndef _THREADS_HH
#define _THREADS_HH

#include "threads.hh"
#include "result.hh"
#include "users.hh"

class DumpThread : public ClientThread {
public:
	std::string target;

	void main();
	DumpThread(Client *client);
};

class TopThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;
	VectorUsers set;
	int rule;
	int size;
	int field_id;
	OutputType type;
	bool inversed;
	TopJoinItems join;

	void main();
	TopThread(Client *client);
};

class ReportThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;
	int field_id;
	OutputType type;

	void main();
	ReportThread(Client *client);
};

class ClearThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;
	int field_id;
	OutputType type;

	void main();
	ClearThread(Client *client);
};

class CountActiveThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;
	int field_id;
	time_t limit;

	void main();
	CountActiveThread(Client *client);
};

class CleanupThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;
	int field_id;
	time_t limit;

	void main();
	CleanupThread(Client *client);
};

class GroupCountThread : public ClientThread {
public:
	Filter filter;
	VectorUsers *from;

	void main();
	GroupCountThread(Client *client);
};

class GroupsStatsThread : public ClientThread {
public:
	void main();
	GroupsStatsThread(Client *client);
};

class GroupsClearThread : public ClientThread {
public:
	void main();
	GroupsClearThread(Client *client);
};

class GroupDelThread : public ClientThread {
private:
	void main();

public:
	std::string name;
	GroupDelThread(Client *client);
};

class SetsSelectThread : public ClientThread {
private:
	void main();

public:
	Filter filter;
	VectorUsers *vector;
	SetsSelectThread(Client *client);
};

class SetsClearThread : public ClientThread {
private:
	void main();

public:
	VectorUsers *vector;
	SetsClearThread(Client *client);
};

#endif
