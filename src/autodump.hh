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

#ifndef _AUTODUMP_HH
#define _AUTODUMP_HH

#include <string>
#include "pthread++.hh"
#include "words_parser.hh"
#include "result.hh"
#include "help.hh"

#define PROPERTY(type, var) \
	type _ ## var;\
	void set_ ## var(type const value) { \
		_ ## var = value; \
	}; \
	type get_ ## var() { \
		return _ ## var; \
	};


class AutoDumpData {
private:
	PMutex mutex;
	bool _force;

public:
	//dump settings
	PROPERTY(bool, enabled);
	PROPERTY(std::string, target);
	PROPERTY(int, delay);

	//last dump	
	PROPERTY(time_t, last_at);
	PROPERTY(std::string, last_message);
	PROPERTY(bool, last_result);
	PROPERTY(bool, last_setted);

public:
	void set(bool const _enabled, std::string const _target, int const _wait);

	void last_dump(bool const result, std::string const message);

	void force();
	bool is_forced();

	void serialize_php(std::stringstream &out);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, OutputType const mode);

	void lock();
	void unlock();

	AutoDumpData();
};

class AutoDump : public PThread {
private:
	PMutex mutex;

private:
	void main();
	void wait();
	bool dump(std::string const target);

public:
	bool force();
	AutoDumpData data;
};

#ifdef _AUTODUMP_CC
AutoDump autodump;
#else
extern AutoDump autodump;
#endif

#endif
