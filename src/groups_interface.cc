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

#define _GROUPS_INTERFACE_CC

#include "groups_interface.hh"
#include "client_thread.hh"
#include "stats.hh"
#include "result.hh"
#include "stringutils.hh"
#include "replicator.hh"
#include "help.hh"

bool GroupsInterface::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type, std::stringstream &replication_query) {
	//!add <name> [<id> <mask>]
	//!	Create new group
	if (parser->current == "add") {
		stats.inc(cmd_prefix + "add");
		std::string name = parser->next();

		GroupId id = GROUP_UNDEF, mask = 0;
		if (parser->next() != "") {
			id = StringUtils::to_uint(parser->current);
			mask = StringUtils::to_uint(parser->next());
		}
		PARSING_END(parser, result);

		if (id == GROUP_UNDEF)
			id = groups.add(name);
		else	
			groups.add(name, id, mask);

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " add " << name << " " << id << " " << mask;
			replicator.add(replication_query);
		}

		result.send();
		return true;
	}

	//!delete <name>
	//!	Delete group
	else if (parser->current == "delete") {
		stats.inc(cmd_prefix + "delete");

		std::string name = parser->next();
		PARSING_END(parser, result);

		GroupDelThread *thread = new GroupDelThread(result.get_client());
		thread->name = name;
		thread->run();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " delete " << parser->current;
			replicator.add(replication_query);
		}

		return true;
	}

	//!list
	//!	Show list of groups
	else if (parser->current == "list") {
		stats.inc(cmd_prefix + "list");

		PARSING_END(parser, result);
		result.type = type;
		switch (type) {
			case TEXT:
				show(result.data);
				break;
			default:
				serialize_php(result.data);
				break;
		}
		result.send();
		return true;
	}

	//!clear
	//!	Clear groups
	else if (parser->current == "clear") {
		stats.inc(cmd_prefix + "clear");

		PARSING_END(parser, result);

		GroupsClearThread *thread = new GroupsClearThread(result.get_client());
		thread->run();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " clear";
			replicator.add(replication_query);
		}

		return true;
	}

	//!stats
	//!	Show number of user in each group
	else if (parser->current == "stats") {
		stats.inc(cmd_prefix + "stats");

		PARSING_END(parser, result);
		GroupsStatsThread *thread = new GroupsStatsThread(result.get_client());
		thread->run();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_GROUPS;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void GroupsInterface::lock() {
	mutex.lock();
}

void GroupsInterface::unlock() {
	mutex.unlock();
}



