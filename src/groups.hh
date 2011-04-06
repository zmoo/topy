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

#ifndef _GROUPS_HH
#define _GROUPS_HH

#include "topy.h"

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "parser.hh"


typedef uint32_t GroupId;
#define GROUP_UNDEF 0
#define GROUP_UNKNOWN (GroupId) ~0

class Groups {
public:
	typedef struct {
		GroupId id;
		GroupId mask;
	} Group;

private:
	GroupId index_counter;
	typedef std::map<std::string, Group> List;
	List list;

	List::iterator find(GroupId const id);

public:
	typedef Group Filter;

	Group get(std::string const name);
	GroupId get_id(std::string const name);
	GroupId get_mask(std::string const name);

	std::string get_name(GroupId const id);
	Filter filter_parse(std::string const str);

	void clear();
	GroupId add(std::string const name);
	GroupId add(std::string const name, GroupId const id, GroupId const mask);
	GroupId del(std::string const name);
	void show(std::stringstream &out);
	void serialize_php(std::stringstream &out);
	void dump(std::filebuf &output);
	void dump_bin(FILE *f);
	void restore_group(std::string const name, GroupId const id, GroupId const mask);
	void restore(Parser &parser);
	void restore_bin(FILE *f);

	Groups();
};

#endif

