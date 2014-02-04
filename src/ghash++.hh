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


#ifndef _GHASH_PP_HH
#define _GHASH_PP_HH

#include "config.h"

#include <glib/ghash.h>

template <typename T>
class HashTableInt {
public:
	GHashTable *table;
	typedef int Key;

	void add(Key *key, T *item);
	T *lookup(Key const key);
	void clear();
	unsigned int size();

	HashTableInt();
	~HashTableInt();
};

template <typename T>
class HashTableInt64 {
public:
	GHashTable *table;
	typedef uint64_t Key;

	void add(Key *key, T *item);
	T *lookup(Key const key);
	void clear();
	unsigned int size();

	HashTableInt64();
	~HashTableInt64();
};

template <typename T>
class HashTableStr {
public:
	GHashTable *table;
	typedef char *Key;

	void add(Key *key, T *item);
	T *lookup(Key const key);
	void clear();
	unsigned int size();

	HashTableStr();
	~HashTableStr();
};

#include "ghash++.tcc"

#endif

