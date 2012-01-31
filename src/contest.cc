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

#include "contest.hh"

void ContestGetThread::main() {
	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	contest->lock();
	contest->serialize_php(result.data, join, limit, from);
	contest->unlock();
	result.send();
}

ContestGetThread::ContestGetThread(Client *_client) : ClientThread(_client) {
}

void ContestClearThread::main() {
	contest->lock();
	contest->clear();
	contest->unlock();
	ClientResult result(client);
	result.send();
}

ContestClearThread::ContestClearThread(Client *_client) : ClientThread(_client) {
}

void ContestFindThread::main() {
	contest->lock();
	int position = contest->find(user);
	contest->unlock();

	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	result.data << "a:2:{";
	result.data << "s:8:\"position\";i:" << position << ";";
	result.data << "s:5:\"total\";i:" << contest->size() << ";";
	result.data << "}";
	result.send();
}

ContestFindThread::ContestFindThread(Client *_client) : ClientThread(_client) {
}

void ContestSizeThread::main() {
	contest->lock();
	int size = contest->size();
	contest->unlock();

	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	result.data << "i:" << size << ";";
	result.send();
}

ContestSizeThread::ContestSizeThread(Client *_client) : ClientThread(_client) {
}

void Contest::add(User *user, UserScore const score) {
	TopBase::add(user, score);
}

void Contest::sort() {
	list.sort(compare_items);
}

void Contest::serialize_php(std::stringstream &out, TopJoinItems &join, unsigned int const limit, unsigned int const from) {
	TopBase::serialize_php(out, join, limit, list.size(), from);
}

void Contest::finalize() {
	sort();
}

int Contest::size() {
	return list.size();
}

void Contest::show(std::stringstream &out, int const size) {
	TopBase::show(out, size, list.size());
}

int Contest::find(User *user) {
	int i = 0;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it->user == user)
			return i;
		i++;
	}
	return -1;
}

bool Contest::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type) {
	//!get [from <int = 0>] [limit <int = 128> [join (<field>, *|<rule>) (<field>, *|<rule>)]]
	//!	Get contest content
	if (parser->current == "get") {
		stats.inc(cmd_prefix + "get");

		ContestGetThread *thread = new ContestGetThread(result.get_client());
		thread->contest = this;
		thread->limit = 128;
		thread->from = 0;

		parser->next();
		if (parser->current == "from" and parser->next() != "") {
			thread->from = StringUtils::to_uint(parser->current);
			parser->next();
		}

		if (parser->current == "limit" and parser->next() != "") {
			thread->limit = StringUtils::to_uint(parser->current);
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

	//!find <user>
	//!	Find the rank of a given user
	else if (parser->current == "find") {
		stats.inc(cmd_prefix + "find");

		UserId id;
		USER_ID_FROM_STRING(id, parser->next());
		PARSING_END(parser, result);

		User *user = users.user_find(id);
		USER_ID_FREE(id);

		if (!user) {
			RETURN_PARSE_ERROR(result, "Unknown user");
		}

		ContestFindThread *thread = new ContestFindThread(result.get_client());
		thread->contest = this;
		thread->user = user;
		thread->run();
		return true;
	}

	//!clear
	//!	Clear contest
	else if (parser->current == "clear") {
		stats.inc("clear");

		PARSING_END(parser, result);
		ContestClearThread *thread = new ContestClearThread(result.get_client());
		thread->contest = this;
		thread->run();
		return true;
	}

	//!size
	//!	Return size of contest
	else if (parser->current == "size") {
		stats.inc("size");

		PARSING_END(parser, result);
		ContestSizeThread *thread = new ContestSizeThread(result.get_client());
		thread->contest = this;
		thread->run();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_CONTEST;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void Contest::lock() {
	mutex.lock();
}

void Contest::unlock() {
	mutex.unlock();
}

