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

#include "serializer.hh"

void SerializerPhp::string(std::string const str) {
	(*out) << "s:" << str.size() << ":\"" << str << "\";";
}

void SerializerPhp::integer(int const value) {
	(*out) << "i:" << value << ";";
}

void SerializerPhp::boolean(bool const value) {
	(*out) << "b:" << ((value) ? 1 : 0) << ";";
}

void SerializerPhp::array_open(int const size) {
	(*out) << "a:" << size << ":{";
}

void SerializerPhp::array_close() {
	(*out) << "}";
}

SerializerPhp::SerializerPhp(std::stringstream *_out) {
	out = _out;
}


