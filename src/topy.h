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

#ifndef _TOPYD_HH
#define _TOPYD_HH

#include <stdint.h>
#include "config.h"

#define USER_FIELDS_COUNT 8

#define ULOG_LEN 50
#define LOG_LEN 5000

#define EVENTS_HOURS_LEN 24
#define EVENTS_DAYS_LEN 31
#define EVENTS_MONTHS_LEN 12

#define MARKS_MONTHS_LEN 6

#ifdef HAVE_STRID
#define USER_ID_STR
#endif

#ifdef HAVE_INT64ID
#define USER_ID_INT64
#endif

#ifdef USER_ID_STR
typedef char * UserId;
#else
#ifdef USER_ID_INT64
typedef uint64_t UserId;
#else
typedef uint32_t UserId;
#endif
#endif

#endif
