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

#include <string>
#include <sstream>

#ifndef _SERIALIZER_HH
#define _SERIALIZER_HH

class Serializer {
protected:
	std::stringstream *out;

public:
	virtual void string(std::string const str) = 0;
	virtual void integer(int const value) = 0;
	virtual void boolean(bool const value) = 0;
	virtual void array_open(int const size) = 0;
	virtual void array_close() = 0;
};

class SerializerPhp : public Serializer {
public:
	void string(std::string const str);
	void integer(int const value);
	void boolean(bool const value);
	void array_open(int const size);
	void array_close();

	SerializerPhp(std::stringstream *_out);
};

#endif
