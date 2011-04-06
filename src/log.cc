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

#define _LOG_CC

#include "log.hh"

#include <iostream>

void Log::open() {
	openlog("topy", LOG_PID, LOG_USER);
}

void Log::msg(const int level, std::string const msg, bool const verbose_this) {

	std::string urgency;
	switch (level) {
	case LOG_INFO:
		urgency = "[INFO]";
		break;
	case LOG_WARNING:
		urgency = "[WARNING]";
		break;
	case LOG_ERR:
		urgency = "[ERROR]";
		break;
	case LOG_NOTICE:
		urgency = "[NOTICE]";
		break;
	case LOG_DEBUG:
		urgency = "[DEBUG]";
		break;
	default:
		urgency = "";
	}

	syslog(level, "%s %s", urgency.c_str(), msg.c_str());

	if (verbose or verbose_this) {
		if (level == LOG_ERR)
			std::cerr << urgency << " " << msg << std::endl;
		else
			std::cout << urgency << " " << msg << std::endl;
	}
}

void Log::close() {
	closelog();
}

Log::Log() : verbose(true) {
}
