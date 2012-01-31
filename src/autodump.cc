/*
 *  Copyright (C) 2009 Nicolas Vion <nico@picapo.net>
 *
 *   This file is part of Fluxy.
 *
 *   Fluxy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Fluxy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Fluxy; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define _AUTODUMP_CC

#include "autodump.hh"

#include "log.hh"
#include "server.hh"
#include "stringutils.hh"
#include "stats.hh"

//-- Job ----

void AutoDump::wait() {
	for (;;) {
		data.lock();
		bool quit = data.is_forced() or (time(NULL) >= data.get_last_at() + data.get_delay() and data.get_enabled());
		data.unlock();

		if (quit) break;
		sleep(2);
	}
}

void AutoDump::main() {
	for (;;) {
		wait();

		data.lock();
		std::string target = data.get_target();
		data.unlock();

		dump(target);
	}
}

bool AutoDump::force() {
	data.lock();
	std::string target = data.get_target();
	bool enabled = data.get_enabled();
	data.unlock();

	if (!enabled or target == "")
		return false;

	return dump(target);
}

bool AutoDump::dump(std::string const target) {
	mutex.lock();
	log.msg(LOG_NOTICE, "Start autodump into '" + target + "'.");

	bool result = server.dump(target);

	std::string message = (result) ? "Autodump successful." : "Could not dump data to '" + target + "'.";
	log.msg((result) ? LOG_NOTICE : LOG_ERR, message);
	mutex.unlock();

	data.lock();
	data.last_dump(result, message);
	data.unlock();
	return result;
}

//-- Data ----
void AutoDumpData::lock() {
	mutex.lock();
}

void AutoDumpData::unlock() {
	mutex.unlock();
}

void AutoDumpData::set(bool const enabled, std::string const target, int const delay) {
	set_enabled(enabled);
	set_target(target);
	set_delay(delay);
	log.msg(LOG_NOTICE, "Autodump settings: " + std::string(enabled ? "enabled" : "disabled") + ", \"" + target + "\", " + StringUtils::to_string(delay));
}

void AutoDumpData::force() {
	_force = true;
}

bool AutoDumpData::is_forced() {
	if (_force) {
		_force = false;
		return true;
	}
	return false;
}

void AutoDumpData::last_dump(bool const result, std::string const message) {
	set_last_setted(true);
	set_last_at(time(NULL));
	set_last_message(message);
}

void AutoDumpData::serialize_php(std::stringstream &out) {
	out << "a:5:{";
	out << "s:7:\"enabled\";b:" << (_enabled ? "1" : "0") << ";";
	out << "s:6:\"target\";s:" << _target.size() << ":\"" << _target << "\";";
	out << "s:5:\"delay\";i:" << _delay << ";";
	out << "s:4:\"next\";";
	if (_enabled)
		out << "i:" << _last_at + _delay - time(NULL) << ";";
	else
		out << "b:0;";
	out << "s:4:\"last\";";

	if (!_last_setted) {
		out << "b:0;";
	}
	else {
		out << "a:3:{";
		out << "s:2:\"at\";i:" << _last_at << ";";
		out << "s:6:\"result\";b:" << ((_last_result) ? 1 : 0) << ";";
		out << "s:7:\"message\";s:" << _last_message.size() << ":\""  << _last_message << "\";";
		out << "}";
	}
	out << "}";
}

bool AutoDumpData::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const mode) {
	//!stats
	//!	Get stats about last autodump
	if (parser->current == "stats") {
		stats.inc(cmd_prefix + "stats");

		PARSING_END(parser, result);
		serialize_php(result.data);
		result.type = PHP_SERIALIZE;
		result.send();
		return true;
	}

	//!set <target> <delay>
	//!	Set autodump settings
	if (parser->current == "set") {
		stats.inc(cmd_prefix + "set");

		set_target(parser->next(true));
		int delay = StringUtils::to_time(parser->next());
		set_delay(delay != 0 ? delay : 3600);
		PARSING_END(parser, result);

		result.send();
		return true;
	}

	//!enable <1/0>
	//!	Enable autodump
	if (parser->current == "enable") {
		stats.inc(cmd_prefix + "enable");

		set_enabled(parser->next() == "1");
		PARSING_END(parser, result);
		result.send();
		return true;
	}

	//!force
	//!	Force autodump
	if (parser->current == "force") {
		stats.inc(cmd_prefix + "force");

		PARSING_END(parser, result);
		force();
		result.send();
		return true;
	}

	//!help
	//!	Show commands list
	if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_AUTODUMP;
		result.send();
		return true;
	}

	//not a valid command
	RETURN_NOT_VALID_CMD(result);
}

AutoDumpData::AutoDumpData() {
	set_last_at(time(NULL));
	set_last_setted(false);
}


