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

#ifndef _DUMP_BIN_HH
#define _DUMP_BIN_HH

#define DUMP_BIN_VERSION 1
#define DUMP_BIN_FIELD_MAGIC 3287
#define DUMP_BIN_GROUP_MAGIC 1984
#define DUMP_BIN_USER_MAGIC 4242

#define DUMP_BIN(var, f) fwrite(&var, 1, sizeof(var), f)
#define RESTORE_BIN(var, f) fread(&var, 1, sizeof(var), f)
#define RESTORE_BIN_SAFE(var, f) (fread(&var, 1, sizeof(var), f) == sizeof(var))

#define DUMP_BIN_STR(str, f) { \
	uint8_t size = str.size(); \
	DUMP_BIN(size, f); \
	fwrite(str.data(), 1, size, f); \
}
	
#define DUMP_BIN_CSTR(str, f) { \
	uint8_t size = strlen(str); \
	DUMP_BIN(size, f); \
	fwrite(str, 1, size, f); \
}

#define RESTORE_BIN_STR(str, f) { \
	char buf[256]; \
	uint8_t size = 0; \
	if (RESTORE_BIN_SAFE(size, f) and size < 256) { \
		fread(&buf, 1, size, f); \
		str = std::string((char *) &buf, (size_t) size); \
	} \
	else str = "";\
}

#define RESTORE_BIN_CSTR(str, f) { \
	uint8_t size = 0; \
	if (RESTORE_BIN_SAFE(size, f) and size < 256) { \
		str = (char *) malloc(size + 1); \
		fread(str, 1, size, f); \
		str[size] = '\0'; \
	} \
	else str = NULL;\
}

void dump_bin_magic(FILE *f, uint32_t const c);
bool restore_bin_magic(FILE *f, uint32_t const c);
void restore_bin_error(std::string const error);

#endif
