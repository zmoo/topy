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


template <typename T>
void HashTableInt<T>::add(Key *key, T *item) {
	g_hash_table_insert(table, (void *) key, item);
}

template <typename T>
T *HashTableInt<T>::lookup(Key const key) {
	return (T *) g_hash_table_lookup(table, &key);
}

template <typename T>
void HashTableInt<T>::clear() {
	g_hash_table_remove_all(table);
}

template <typename T>
unsigned int HashTableInt<T>::size() {
	return g_hash_table_size(table);
}

template <typename T>
HashTableInt<T>::HashTableInt() {
	table = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, NULL);
}

template <typename T>
HashTableInt<T>::~HashTableInt() {
	g_hash_table_destroy(table);
}

//----

gboolean uint64_equal(gconstpointer v1, gconstpointer v2) {
	return *((const uint64_t *) v1) == *((const uint64_t *) v2);
}

guint uint64_hash(gconstpointer v) {
	return (guint) *(const uint64_t *) v;
}

template <typename T>
void HashTableInt64<T>::add(Key *key, T *item) {
	g_hash_table_insert(table, (void *) key, item);
}

template <typename T>
T *HashTableInt64<T>::lookup(Key const key) {
	return (T *) g_hash_table_lookup(table, &key);
}

template <typename T>
void HashTableInt64<T>::clear() {
	g_hash_table_remove_all(table);
}

template <typename T>
unsigned int HashTableInt64<T>::size() {
	return g_hash_table_size(table);
}

template <typename T>
HashTableInt64<T>::HashTableInt64() {
	table = g_hash_table_new_full(uint64_hash, uint64_equal, NULL, NULL);
}

template <typename T>
HashTableInt64<T>::~HashTableInt64() {
	g_hash_table_destroy(table);
}

//----

template <typename T>
void HashTableStr<T>::add(Key *key, T *item) {
	g_hash_table_insert(table, (void *) *key, item);
}

template <typename T>
T *HashTableStr<T>::lookup(Key const key) {
	return (T *) g_hash_table_lookup(table, key);
}

template <typename T>
void HashTableStr<T>::clear() {
	g_hash_table_remove_all(table);
}

template <typename T>
unsigned int HashTableStr<T>::size() {
	return g_hash_table_size(table);
}

template <typename T>
HashTableStr<T>::HashTableStr() {
	table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
}

template <typename T>
HashTableStr<T>::~HashTableStr() {
	g_hash_table_destroy(table);
}

