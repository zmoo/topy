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

#define _SERVER_CC

#include "config.h"
#include "server.hh"

#include "topy.h"
#include "log.hh"
#include "timer.hh"
#include "macros.hh"
#include "client_thread.hh"
#include "groups_interface.hh"
#include "users_sets.hh"
#include "help.hh"
#include "autodump.hh"
#include "dump_bin.hh"

#include <cstdio>

bool parse_from(WordsParser *parser, VectorUsers **from, ClientResult &result) {
	if (parser->current == "from" and parser->next() != "") {
		sets.lock();
		*from = sets.find(parser->current);
		sets.unlock();
		if (*from == NULL)  {
			RETURN_PARSE_ERROR(result, "Not a valid users set name");
		}
		parser->next();
	}
	else {
		*from = users.get_vector();
	}
	return true;
}

bool parse_join(WordsParser *parser, TopJoinItems &join, ClientResult &result) {
	if (parser->current == "join") {
		User user;
		while (parser->next() == "(" and parser->next() != "") {
			FieldId join_field_id = parse_field_id(parser);
			if (join_field_id == FIELD_ID_UNKNOWN) {
				RETURN_PARSE_ERROR(result, "Not a valid field name.");
			}

			int join_rule_id = 0;
			if (parser->current == "," and (parser->next()) != "") {
				join_rule_id = parser->current == "*" ? TOP_JOIN_ITEM_ALL : user.field[join_field_id]->get_rule_id(parser->current);
				if (join_rule_id == -1) {
					RETURN_PARSE_ERROR(result, "Not a valid rule name.");
				}
				parser->next();
			}

			if (parser->current != ")") {
				RETURN_PARSE_ERROR(result, "Unexpected '" + parser->current + "'. Expected : ')'");
			}

			join.push_back(TopJoinItem(join_field_id, join_rule_id));
		}
	}
	return true;
}

bool parse_query(bool &parsed, ClientResult &result, WordsParser *parser, OutputType const mode) {
	parsed = false;

	//!TID <n> <command> 
	//!	Give a Transaction Id that will be returned with the command's result
	if (parser->current == "TID") {
		result.tid = StringUtils::to_uint(parser->next());
		parser->next();
	}

	std::stringstream replication_query;
	replication_query << "#";

	//Do not replicate query
	if (parser->current == "#") {
		result.replicated = true;
		parser->next();
	}

	//Do not send result data
	if (parser->current == "!") {
		result.quiet = true;
		parser->next();
	}

	//!user <user_id> <command> [args]
	//!	Execute a command on a existing user
	//!	See "user <user_id> help" for more information
	//!user* <user_id> <command> [args]
	//!	Execute a command on a user.
	//!	Create a new user if <user_id> doesn't exists
	if (parser->current == "user") {
		parsed = true;

		parser->next();
		bool find_or_create = false;
		if (parser->current == "*") {
			find_or_create = true;
			parser->next();
		}

		UserId id;
		USER_ID_FROM_STRING(id, parser->current);
		User *user = (find_or_create) ? users.user_find_or_create(id) : users.user_find(id);
		parser->next();

		if (!user) {
			USER_ID_FREE(id);
			RETURN_PARSE_ERROR(result, "Not a valid user id.");
		}

		replication_query << "user *" << id;
		USER_ID_FREE(id);

		PMutex *mutex = user->lock();
		bool res = user->parse_query(result, "user::", parser, replication_query);

		mutex->unlock();
		return res;
	} 

	//!groups <command>
	//!	Execute a command on groups
	//!	See "groups help" for more information
	else if (parser->current == "groups") {
		parsed = true;
		parser->next();
		groups.lock();
		replication_query << "groups";
		bool res = groups.parse_query(result, "groups::", parser, mode, replication_query);
		groups.unlock();
		return res;
	}
	return false;
}

bool ClientTopy::parse_query(WordsParser *parser) {
	ClientResult result(this);

	bool parsed;
	bool res = ::parse_query(parsed, result, parser, mode);
	if (parsed) {
		return res;
	}

	//!sets <command>
	//!	Execute a command on users sets
	//!	See "sets help" for more information
	if (parser->current == "sets") {
		parser->next();
		sets.lock();
		bool res = sets.parse_query(result, "sets::", parser, mode);
		sets.unlock();
		return res;
	}

	//!contests <command>
	//!	Execute a command on users contests
	//!	See "contests help" for more information
	else if (parser->current == "contests") {
		parser->next();
		contests.lock();
		bool res = contests.parse_query(result, "contests::", parser, mode);
		contests.unlock();
		return res;
	}

	//!fields <command>
	//!	Execute a command on fields
	//!	See "fields help" for more information
	else if (parser->current == "fields") {
		parser->next();
		fields.lock();
		bool res = fields.parse_query(result, "fields::", parser, mode);
		fields.unlock();
		return res;
	}

	//!autodump <command>
	//!	Execute a command on autodump
	//!	See "autodump help" for more information
	else if (parser->current == "autodump") {
		parser->next();
		autodump.data.lock();
		bool res = autodump.data.parse_query(result, "autodump::", parser, mode);
		autodump.data.unlock();	
		return res;
	}

	//!info 
	//!	Get server information
	else if (parser->current == "info") {
		stats.inc("info");

		PARSING_END(parser, result);
		result.data << "Topy version: " VERSION << std::endl;
		result.data << "Libevent version: " << event_get_version() << std::endl;
		result.data << "Users id type: " << USER_ID_TYPE_NAME << std::endl;
		result.send();
		return true;
	}
	
	//!stats 
	//!	Get statistics about server
	else if (parser->current == "stats") {
		stats.inc("stats");

		PARSING_END(parser, result);
		result.type = mode;
		switch (mode) {
			case TEXT:
				result.data << "STAT uptime " << server.uptime() << std::endl;
				result.data << "STAT users " << users.count() << std::endl;			
				stats.show(result.data, "commands::");
				break;
			default:
				result.data << "a:3:{";
				result.data << "s:6:\"uptime\";i:" << server.uptime() << ";";
				result.data << "s:5:\"users\";i:" << users.count() << ";";
				result.data << "s:8:\"commands\";";
				stats.serialize_php(result.data);
				result.data << "}";
				break;
		}
		result.send();
		return true;
	}

	//!dump <path>
	//!	Dump data in file <path>
	else if (parser->current == "dump") {
		stats.inc("dump");
		std::string path = parser->next(true);
		if (path == "") {
			RETURN_PARSE_ERROR(result, "Path is needed.");
		}
		PARSING_END(parser, result);

		DumpThread *thread = new DumpThread(this);
		thread->target = path;
		thread->run();
		return true;

	}

	//!quit
	//!	Close connection with client
	else if (parser->current == "quit") {
		stats.inc("misc");

		PARSING_END(parser, result);
		exit();
		return true;
	}

	//!halt 
	//!	Stop server 
	//!	(available if server was compiled with a recent libevent version)
	else if (parser->current == "halt") {
		stats.inc("misc");

		PARSING_END(parser, result);
#ifdef HAVE_LIBEVENT_LOOPBREAK
		if (event_loopbreak() != 0) {
			result.error("This command is not supported.");
		}
		else {
			result.msg("Server is now stoping...");
		}
		result.send();
		return true;

#else
		RETURN_PARSE_ERROR(result, "This command is not supported.");
#endif
	}

	//!mode <text|php_serialize|none> 
	//!	Set default output format
	else if (parser->current == "mode") {
		stats.inc("misc");

		std::string name = parser->next();
		PARSING_END(parser, result);

		if (name == "text") {
			mode = TEXT;
		}
		else if (name == "php_serialize") {
			mode = PHP_SERIALIZE;
		}
		else {
			RETURN_PARSE_ERROR(result, "Unknown output mode");
		}
		result.send();
		return true;
	}

	//!report <field> [from <set>] [where <expr>]
	//!	Return a report about a given field
	else if (parser->current == "report") {
		stats.inc("report");

		parser->next();
		FieldId field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}
		
		ReportThread *thread = new ReportThread(this);
		thread->field_id = field_id;
		thread->type = mode;
		if (!parse_from(parser, &thread->from, result)) {
			delete thread;
			return false;
		}
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!clear <field> [from <set>] [where <expr>]
	//!	Clear a given field to a group of users
	else if (parser->current == "clear") {
		stats.inc("clear");

		parser->next();
		FieldId field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}

		ClearThread *thread = new ClearThread(this);
		thread->field_id = field_id;
		thread->type = mode;
		if (!parse_from(parser, &thread->from, result)) {
			delete thread;
			return false;
		}
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!top <field> [rule <rule name>] [inversed] [set <user1>, <user2>, ...] [from <set>] [where <expr>] [size <n = 32>] [join (<field>, *|<rule>) (<field>, *|<rule>)...]
	//!	Show most active users of given groups
	else if (parser->current == "top") {
		stats.inc("top");

		parser->next();
		FieldId field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}

		TopThread *thread = new TopThread(this);
		thread->field_id = field_id;
		thread->size = 32;
		thread->rule = 0;
		thread->type = mode;
		thread->inversed = false;

		User user;
		if (parser->current == "rule") {
			std::string rule_name = parser->next();
			thread->rule = user.field[field_id]->get_rule_id(rule_name);
			if (thread->rule == -1) {
				RETURN_PARSE_ERROR_T(result, "Not a valid rule name : '" + rule_name + "'", thread);
			}
			parser->next();
		}

		if (parser->current == "inversed") {
			thread->inversed = true;
			parser->next();
		}

		if (!parse_from(parser, &thread->from, result)) {
			delete thread;
			return false;
		}

		if (parser->current == "set") {
			do {
				parser->next();
				UserId id;
				USER_ID_FROM_STRING(id, parser->current);
				User *user = users.user_find(id);
				if (user) thread->set.list.push_back(user);
				USER_ID_FREE(id);
			} while (parser->next() == ",");
			thread->from = &thread->set;
		}

		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		if (parser->current == "size") {
			thread->size = StringUtils::to_uint(parser->next());
			parser->next();
		}

		if (!parse_join(parser, thread->join, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!count [from <set>] [where <expr bool>]
	//!	Count users
	else if (parser->current == "count") {
		stats.inc("count");

		GroupCountThread *thread = new GroupCountThread(this);
		parser->next();
		if (!parse_from(parser, &thread->from, result)) {
			delete thread;
			return false;
		}
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!count_active <field> [limit <second = 5 * 60> OR since <gmt>] [from <set>] [where <expr bool>]
	//!	Count active users
	else if (parser->current == "count_active") {
		stats.inc("count_active");

		parser->next();
		FieldId field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}

		CountActiveThread *thread = new CountActiveThread(this);
		thread->field_id = field_id;
		thread->limit = time(NULL) - 5 * 60; //5 minutes
		
		if (parser->current == "limit" and parser->next() != "") {
			thread->limit = time(NULL) - StringUtils::to_int(parser->current);
			parser->next();			
		}
		else if (parser->current == "since" and parser->next() != "") {
			thread->limit = StringUtils::to_int(parser->current);
			parser->next();			
		}

		if (!parse_from(parser, &thread->from, result)) {
			delete thread;
			return false;
		}
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!cleanup <field> [limit <seconds = 31 * 3600 * 24> OR since <gmt>] [from <set>] [where <expr bool>]
	//!	Delete inactive users
	else if (parser->current == "cleanup") {
		stats.inc("cleanup");

		parser->next();
		FieldId field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}

		CleanupThread *thread = new CleanupThread(this);
		thread->field_id = field_id;
		thread->limit = time(NULL) - 31 * 3600 * 24; //31 days
		
		if (parser->current == "limit" and parser->next() != "") {
			thread->limit = time(NULL) - StringUtils::to_int(parser->current);
			parser->next();			
		}
		else if (parser->current == "since" and parser->next() != "") {
			thread->limit = StringUtils::to_int(parser->current);
			parser->next();			
		}

		if (!parse_from(parser, &thread->from, result)) {
			delete thread;
			return false;
		}
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!time
	//!	Return internal timer values
	else if (parser->current == "time") {
		stats.inc("time");

		PARSING_END(parser, result);
		result.type = PHP_SERIALIZE;
		timer.refresh();
		timer.serialize_php(result.data);
		result.send();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_SERVER;
		result.send();
		return true;
	}

	//!debug 
	//!	Show debug information
	else if (parser->current == "debug") {
		stats.inc("misc");

		PARSING_END(parser, result);
		users.debug(result.data);
		timer.debug(result.data);
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void ClientTopy::receive() {
	try {
		std::stringstream stream;
		read(stream);

		WordsParser parser(&stream);
		parser.next();
		ClientResult result(this);
		parse_query(&parser);
	}
	catch (...) {
		log.msg(LOG_ERR, "Not a valid command buffer");
	}
}

ClientTopy::ClientTopy(int const _client_fd) : Client(_client_fd) {
	mode = PHP_SERIALIZE;
}

void ServerTopy::new_client(int const _fd) {
	new ClientTopy(_fd);
}

bool ServerTopy::dump_txt(std::string const path) {
	std::filebuf fb;
	fb.open(path.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!fb.is_open()) 
		return false;

	fields.dump(fb);
	groups.dump(fb);
	users.dump(fb);

	fb.pubsync();
	fb.close();
	return true;
}

bool ServerTopy::dump_bin(std::string const path) {
	FILE *f = fopen(path.c_str(), "wb");
	if (!f) {
		log.msg(LOG_ERR, "Could not open file: " + path);
		return false;
	}
	char pattern[4] = {'T', 'o', 'p', 'y'};
	DUMP_BIN(pattern, f);

	uint8_t version = DUMP_BIN_VERSION;
	DUMP_BIN(version, f);

	fields.dump_bin(f);
	groups.dump_bin(f);
	users.dump_bin(f);

	DUMP_BIN(pattern, f);
	fclose(f);
	return true;
}

#define CMP_EXT(str, ext) (str.size() > sizeof(ext) and str.substr(str.size() - sizeof(ext), sizeof(ext) + 1) == "." ext)

bool ServerTopy::dump(std::string const path) {
	std::string tmp = path + ".tmp";
	if (CMP_EXT(path, "txt")) {
		if (!dump_txt(tmp))
			return false;
	}
	else {
		if (!dump_bin(tmp))
			return false;
	}
	return (rename(tmp.c_str(), path.c_str()) == 0);
}

bool ServerTopy::restore_txt(std::string const path) {
	Parser parser;
	if (!parser.open(path))
		return false;

	try {
		fields.restore(parser);
		groups.restore(parser);
		users.restore(parser);
	
		parser.close();
		return true;
	}

	catch (...) {
		log.msg(LOG_ERR, "Parse error: " + parser.get_error_msg() + " (at line : " + StringUtils::to_string(parser.get_line()) + ")");
		parser.close();
		return false;
	}
}

bool ServerTopy::restore_bin(std::string const path) {
	try {
		FILE *f = fopen(path.c_str(), "rb");
		if (!f) {
			log.msg(LOG_ERR, "Could not open file: " + path);
			return false;
		}

		char pattern[4];
		if (!RESTORE_BIN_SAFE(pattern, f) or pattern[0] != 'T' or pattern[1] != 'o' or pattern[2] != 'p' or pattern[3] != 'y')
			restore_bin_error("Invalid pattern at begining of file");

		uint8_t version;
		RESTORE_BIN(version, f);
		if (version != DUMP_BIN_VERSION)
			restore_bin_error("Invalid binary dump version number");

		fields.restore_bin(f);
		groups.restore_bin(f);
		users.restore_bin(f);

		if (!RESTORE_BIN_SAFE(pattern, f) or pattern[0] != 'T' or pattern[1] != 'o' or pattern[2] != 'p' or pattern[3] != 'y')
			restore_bin_error("Invalid pattern at end of file");

		fclose(f);
		return true;
	}

	catch (...) {
		log.msg(LOG_ERR, "Could not restore binary file");
		return false;
	}
}

bool ServerTopy::restore(std::string const path) {
	if (CMP_EXT(path, "txt"))
		return restore_txt(path);
	else
		return restore_bin(path);
}

ServerTopy::ServerTopy() {
}
