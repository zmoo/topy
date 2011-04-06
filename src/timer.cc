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

#define _TIMER_CC

#include "timer.hh"
#include <iostream>

#define YEARS_TABLE_COUNT 20
static unsigned int first_day[YEARS_TABLE_COUNT] = {
	0,    //2008 (366 days)
	366,  //2009 (365 days)
	731,  //2010 (365 days)
	1096, //2011 (365 days)
	1461, //2012 (366 days)
	1827, //2013 (365 days)
	2192, //2014 (365 days)
	2557, //2015 (365 days)
	2922, //2016 (366 days)
	3288, //2017 (365 days)
	3653, //2018 (365 days)
	4018, //2019 (365 days)
	4383, //2020 (366 days)
	4749, //2021 (365 days)
	5114, //2022 (365 days)
	5479, //2023 (365 days)
	5844, //2024 (366 days)
	6210, //2025 (365 days)
	6575, //2026 (365 days)
	6940  //2027 (365 days)
};

time_t Timer::refresh() {
	if (lock_count > 0)
		return now;

	now = time(NULL);
	if (now - update < 10)
		return now;

	update = now;
	tm *gmt = gmtime(&now);
	
	// day 0 is 01/01/2008
	year = gmt->tm_year;
	if (year < 108) {
		std::cerr << "date has to be older than 1 janv 2008 (" << (year + 1900) << ")" << std::endl;
		return now;
	}

	int year_idx = year - 108;
	if (year_idx > YEARS_TABLE_COUNT) {
		std::cerr << "can not manage date after 2028" << std::endl;
		return now;
	}

	month = (year_idx * 12) + gmt->tm_mon;
	day = first_day[year_idx] + gmt->tm_yday;
	hour = day * 24 + gmt->tm_hour;
	return now;
}

void Timer::debug(std::stringstream &out) {
	out << "time() : " << time(NULL) << std::endl;
	out << "timer.lock_count : " << lock_count << std::endl;
	out << "timer.now : " << now << std::endl;
	out << "timer.hour : " << hour << std::endl;
	out << "timer.day : " << day << std::endl;
	out << "timer.month : " << month << std::endl;
	out << "timer.year : " << year << std::endl;
}

void Timer::serialize_php(std::stringstream &out) {
	out << "a:5:{";
	out << "s:3:\"now\";i:" << now << ";";
	out << "s:4:\"hour\";i:" << hour << ";";
	out << "s:3:\"day\";i:" << day << ";";
	out << "s:5:\"month\";i:" << month << ";";
	out << "s:4:\"year\";i:" << year << ";";
	out << "}";
}

void Timer::lock() {
	refresh();
	lock_count++;
}

void Timer::unlock() {
	if (lock_count > 0)
		lock_count--;
}

Timer::Timer() {
	lock_count = 0;
	update = 0;
}
