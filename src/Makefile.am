# Copyright (C) 2007 Nicolas Vion <nico@picapo.net>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = topy

topy_SOURCES = \
	utopy.cc

noinst_HEADERS = \
	client_thread.hh \
	config_file.hh \
	top.hh \
	expr_bool.hh \
	field.hh \
	filter.hh \
	ghash++.hh \
	groups.hh \
	groups_interface.hh \
	macros.hh \
	parser.hh \
	pthread++.hh \
	result.hh \
	events.hh \
	server.hh \
	udp.hh \
	stats.hh \
	stringutils.hh \
	timer.hh \
	topy.h \
	user.hh \
	users.hh \
	users_parser.hh \
	words_parser.hh \
	help.hh \
	users_sets.hh \
	contests.hh \
	contest.hh \
	log.hh \
	threads.hh \
	autodump.hh \
	dump_bin.hh \
	fields.hh \
	replicator.hh \
	words_parser.hh \
	expr_bool.hh \
	groups.hh \
	stringutils.hh \
	parser.hh \
	config_file.hh

EXTRA_DIST = \
	ghash++.tcc \
	pthread++.cc \
	stats.cc \
	filter.cc \
	client_thread.cc \
	groups_interface.cc \
	top.cc \
	events.cc \
	server.cc \
	udp.cc \
	result.cc \
	field.cc \
	user.cc \
	users.cc \
	timer.cc \
	topy.cc	\
	users_sets.cc \
	contests.cc \
	contest.cc \
	log.cc \
	threads.cc \
	autodump.cc \
	dump_bin.cc \
	fields.cc \
	replicator.cc \
	words_parser.cc \
	expr_bool.cc \
	groups.cc \
	stringutils.cc \
	parser.cc \
	config_file.cc \
	php/make.php

AM_CXXFLAGS = @GLIB_CFLAGS@
topy_LDADD = @GLIB_LIBS@
