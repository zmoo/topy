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

#ifndef _TIME_HH
#define _TIME_HH

#include <ctime>
#include <sstream>

struct Timer {
private:
	time_t update;
	int lock_count;

public:
	time_t now;
	int hour;
	int day;
	int week;
	int month;
	int year;

	time_t refresh();
	void serialize_php(std::stringstream &out);
	void debug(std::stringstream &out);
	void lock();
	void unlock();

	Timer();
};

#ifdef _TIMER_CC
Timer timer;
#else
extern Timer timer;
#endif

#endif
