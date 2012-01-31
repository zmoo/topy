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

#define _CONTESTS_CC

#include "contests.hh"

#include "stats.hh"
#include "filter.hh"
#include "server.hh"
#include "client_thread.hh"

void RankThread::main() {
	timer.lock();
	contest->lock();
	from->rank(contest, filter, field_id, rule, inversed);
	contest->unlock();
	timer.unlock();
	ClientResult result(client);
	result.send();
}

RankThread::RankThread(Client *_client) : ClientThread(_client) {
}

Contest *Contests::add(std::string const name, Contest* contest) {
	std::pair<List::iterator, bool> result;
	result = list.insert(std::pair<std::string, Contest*> (name, contest));
	return (result.second) ? (result.first->second) : NULL;
}

Contest *Contests::find(std::string const name) {
	List::iterator it;
	it = list.find(name);

	if (it != list.end())
		return it->second;

	return NULL;
}

Contest *Contests::find_or_create(std::string const name) {
	Contest *result = find(name);
	if (!result) {
		Contest *contest = new Contest;
		result = add(name, contest);
	}
	return result;
}

Contest *Contests::del(std::string const name, bool const free) {
	List::iterator it = list.find(name);
	if (it == list.end())
		return NULL;

	list.erase(it);
	Contest *contest = it->second;
	if (free)
		delete contest;

	return contest;
}

void Contests::clear(bool const free) {
	if (free) {
		for (List::iterator it = list.begin(); it != list.end(); it++) {
			Contest *contest = it->second;
			delete contest;
		}
	}
	list.clear();
}

void Contests::show(std::stringstream &out) {
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		if (it != list.begin())
			out << std::endl;
		out << it->first << "\t" << it->second->size();
	}
}

void Contests::serialize_php(std::stringstream &out) {
	out << "a:" << list.size() << ":{";
	int i = 0;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		out << "i:" << i << ";";
		out << "a:2:{";
		out << "s:4:\"name\";";
		out << "s:" << it->first.size() << ":\"" << it->first << "\";";
		out << "s:5:\"count\";";
		out << "i:" << it->second->size() << ";";
		out << "}";
		i++;
	}
	out << "}";
}

bool Contests::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const type) {
	//!:: <name> <command>
	//!	Execute <command> on contest <name>
	if (parser->current == "::") {
		std::string name = parser->next();
		Contest *contest = find(name);
		if (!contest) {
			RETURN_PARSE_ERROR(result, "Not a valid contest name.");
		}

		parser->next();
		return contest->parse_query(result, cmd_prefix + name + "::", parser, type);
	}

	//!generate <name> <field> [rule <rule name>] [inversed] [from <set>] [where <expr>]]
	//!	Generate contest
	else if (parser->current == "generate") {
		stats.inc(cmd_prefix + "generate");

		std::string name = parser->next();

		parser->next();
		int field_id = parse_field_id(parser);
		if (field_id == FIELD_ID_UNKNOWN) {
			RETURN_PARSE_ERROR(result, "Not a valid field name.");
		}

		RankThread *thread = new RankThread(result.get_client());
		thread->field_id = field_id;
		thread->rule = 0;
		thread->inversed = false;

		if (parser->current == "rule") {
			parser->next();

			User user;
			Field::ScoreRulesList rules;
			user.field[field_id]->get_score_rules(rules);
			Field::ScoreRulesList::iterator it = rules.find(parser->current);
			if (it == rules.end()) {
				RETURN_PARSE_ERROR_T(result, "Not a valid score rule.", thread);
			}
			thread->rule = it->second;
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
		if (!parse_where(parser, &thread->filter, result)) {
			delete thread;
			return false;
		}
		thread->contest = find_or_create(name);

		PARSING_ENDED_T(parser, result, thread);
		thread->run();
		return true;
	}

	//!delete <name>
	//!	Delete user set
	else if (parser->current == "delete") {
		stats.inc(cmd_prefix + "delete");

		std::string name = parser->next();
		PARSING_END(parser, result);

		Contest *contest = del(name, true);
		if (contest == NULL) {
			RETURN_PARSE_ERROR(result, "Not a valid users set name");
		}
		return true;
	}

	//!list
	//!	Show list of sets
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

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_CONTESTS;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void Contests::lock() {
	mutex.lock();
}

void Contests::unlock() {
	mutex.unlock();
}

