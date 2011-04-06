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

#ifndef _FIELD_HH
#define _FIELD_HH

#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "topy.h"
#include "parser.hh"
#include "result.hh"
#include "words_parser.hh"

typedef int UserScore;

class StatsVectorBase {
protected:
	uint8_t count;
public:
	unsigned int get_count();
	virtual int get_sample(unsigned int n) = 0;
	virtual bool set_sample(unsigned int n, int const value) = 0;
	virtual unsigned int get_type_s() = 0;
	virtual unsigned int get_len_s() = 0;
	virtual int sum(int const n = 0) = 0;
	virtual bool multiply(float const coef) = 0;
	virtual bool inc(int const n) = 0;
	virtual void add(StatsVectorBase *vector) = 0;
	virtual void count_active(StatsVectorBase *vector) = 0;

	virtual void debug() = 0;
	virtual void show(std::stringstream &out) = 0;
	virtual void serialize_php(std::stringstream &out) = 0;
	virtual void dump(std::stringstream &output, bool const with_format = true) = 0;
	virtual void dump_bin(FILE *f, bool const with_format = true) = 0;
	virtual void restore(Parser &parser) = 0;
	virtual void restore_bin(FILE *f) = 0;

	virtual void translate(unsigned int const n) = 0;
	virtual bool del(unsigned int const n) = 0;
	virtual int find(int const n) = 0;
	virtual void copy(StatsVectorBase *src) = 0;
	virtual void clear() = 0;

	virtual ~StatsVectorBase();
};

template <typename type_s, int len_s = 64, bool is_unsigned = false> 
class StatsVector : public StatsVectorBase {
private:
	type_s samples[len_s];
public:
	int get_sample(unsigned int const n);
	bool set_sample(unsigned int n, int const value);
	unsigned int get_type_s();
	unsigned int get_len_s();
	int sum(int const n = 0);
	bool multiply(float const coef);
	bool inc(int const n);
	void add(StatsVectorBase *vector);
	void count_active(StatsVectorBase *vector);
	void translate(unsigned int const n);
	bool del(unsigned int const n);
	int find(int const n);

	void debug();
	void show(std::stringstream &out);
	void serialize_php(std::stringstream &out);
	void dump(std::stringstream &output, bool const with_format = true);
	void dump_bin(FILE *f, bool const with_format = true);
	void restore(Parser &parser);
	void restore_bin(FILE *f);

	void copy(StatsVectorBase *src);
	void clear();

	StatsVector();
};

class Field {
public:
	typedef std::map<std::string, int> ScoreRulesList;

	virtual void update() = 0;
	virtual void add(int const n) = 0;
	virtual bool set(std::string const value) = 0;
	virtual void serialize_php(std::stringstream &out) = 0;
	virtual void dump(std::stringstream &output) = 0;
	virtual void dump_bin(FILE *f) = 0;
	virtual void restore(Parser &parser) = 0;
	virtual void restore_bin(FILE *f) = 0;
	virtual void show(std::stringstream &out) = 0;
	virtual std::string summary() = 0;
	virtual void get_score_rules(ScoreRulesList &result);
	int get_rule_id(std::string name);
	void rules_serialize_php(std::stringstream &out);
	virtual UserScore score(int const rule = 0) = 0; 
	virtual bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);
	virtual void clear() = 0;
	virtual std::string name() = 0;
	virtual time_t last_update() = 0;
	virtual ~Field();
};

#define FIELD_EVENTS_HOURS_SUM 0
#define FIELD_EVENTS_HOURS_LAST 1
#define FIELD_EVENTS_HOURS_PENULTIMATE 2
#define FIELD_EVENTS_HOURS_LAST2 3
#define FIELD_EVENTS_HOURS_LAST3 4
#define FIELD_EVENTS_HOURS_LAST6 5
#define FIELD_EVENTS_HOURS_LAST12 6
 
#define FIELD_EVENTS_DAYS_SUM 7
#define FIELD_EVENTS_DAYS_LAST 8
#define FIELD_EVENTS_DAYS_PENULTIMATE 9
#define FIELD_EVENTS_DAYS_LAST2 10
#define FIELD_EVENTS_DAYS_LAST7 11
#define FIELD_EVENTS_DAYS_LAST15 12

#define FIELD_EVENTS_MONTHS_SUM 13
#define FIELD_EVENTS_MONTHS_LAST 14
#define FIELD_EVENTS_MONTHS_PENULTIMATE 15
#define FIELD_EVENTS_MONTHS_LAST2 16
#define FIELD_EVENTS_MONTHS_LAST3 17
#define FIELD_EVENTS_MONTHS_LAST6 18

#define FIELD_EVENTS_TOTAL 19

#define FIELD_EVENTS_TYPE_NAME "events"
class FieldEvents : public Field {
private:
	friend class ReportFieldEvents;

	struct {
		int hour;
		int day;
		int month;
	} date;

	void init();

public:
	time_t last_inc;

	StatsVectorBase *hours;
	StatsVectorBase *days;
	StatsVectorBase *months;
	unsigned int total;

	void debug();
	void translate(unsigned int const delta_hours, unsigned int const delta_days, unsigned int const delta_months);

	FieldEvents(bool const alloc = true);
	~FieldEvents();

public:
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);
	void update();
	void add(int const n);
	bool set(std::string const value);
	void serialize_php(std::stringstream &out);
	void dump(std::stringstream &output);
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void show(std::stringstream &out);
	std::string summary();
	void get_score_rules(ScoreRulesList &result);
	UserScore score(int const rule = 0);
	void clear();
	std::string name();
	time_t last_update();
};

#define FIELD_MARKS_LAST_AVERAGE 0
#define FIELD_MARKS_LAST2_AVERAGE 1
#define FIELD_MARKS_LAST3_AVERAGE 2
#define FIELD_MARKS_LAST6_AVERAGE 3
#define FIELD_MARKS_SUM_AVERAGE 4

#define FIELD_MARKS_LAST_SKYVERAGE 5
#define FIELD_MARKS_LAST2_SKYVERAGE 6
#define FIELD_MARKS_LAST3_SKYVERAGE 7
#define FIELD_MARKS_LAST6_SKYVERAGE 8
#define FIELD_MARKS_SUM_SKYVERAGE 9

#define FIELD_MARKS_LAST_COUNT 10
#define FIELD_MARKS_LAST2_COUNT 11
#define FIELD_MARKS_LAST3_COUNT 12
#define FIELD_MARKS_LAST6_COUNT 13
#define FIELD_MARKS_SUM_COUNT 14

#define FIELD_MARKS_TYPE_NAME "marks"
class FieldMarks : public Field {
private:
	struct {
		int month;
	} date;

	void init();
	StatsVectorBase *months_num;
	StatsVectorBase *months_denom;

public:
	void debug();
	void translate(unsigned int const delta_months);

	FieldMarks(bool const alloc = true);
	~FieldMarks();

public:
	void update();
	void add(int const n);
	bool set(std::string const value);
	void serialize_php(std::stringstream &out);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void dump(std::stringstream &output);
	void dump_bin(FILE *f);
	void show(std::stringstream &out);
	std::string summary();
	void get_score_rules(ScoreRulesList &result);
	UserScore score(int const rule = 0); 
	void clear();
	std::string name();
	time_t last_update();
};

#define FIELD_INT_TYPE_NAME "int"
#define FIELD_UINT_TYPE_NAME "uint"
class FieldInt : public Field {
private:
	friend class ReportFieldInt;
	int value;
	bool is_unsigned;
	void set(int const _value);

public:
	FieldInt(bool const is_unsigned = false);

public:
	void update();
	void add(int const n);
	bool set(std::string const value);
	void serialize_php(std::stringstream &out);
	void dump(std::stringstream &output);
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void show(std::stringstream &out);
	std::string summary();
	UserScore score(int const rule = 0);
	void clear();
	std::string name();
	time_t last_update();
};

#define FIELD_TIMESTAMP_TYPE_NAME "timestamp"
class FieldTimestamp : public Field {
private:
	time_t value;

public:
	FieldTimestamp();

public:
	void update();
	void add(int const n);
	bool set(std::string const value);
	void serialize_php(std::stringstream &out);
	void dump(std::stringstream &output);
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void show(std::stringstream &out);
	std::string summary();
	UserScore score(int const rule = 0);
	void clear();
	std::string name();
	time_t last_update();
};

#define FIELD_ULOG_TYPE_NAME "ulog"
class FieldUlog : public Field {
private:
	StatsVector<uint32_t, ULOG_LEN, true> items;
	StatsVector<time_t, ULOG_LEN, true> dates;

public:
	FieldUlog();
	~FieldUlog();

public:
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);
	void update();
	void add(int const n);
	void insert(int const n, time_t const date);
	void del(int const n);
	bool set(std::string const value);
	void serialize_php(std::stringstream &out);
	void dump(std::stringstream &output);
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void show(std::stringstream &out);
	std::string summary();
	UserScore score(int const rule = 0); 
	void clear();
	std::string name();
	time_t last_update();
};

#define FIELD_LOG_TYPE_NAME "log"
class FieldLog : public Field {
private:
	StatsVector<uint32_t, LOG_LEN, true> items;
	StatsVector<time_t, LOG_LEN, true> dates;

public:
	FieldLog();
	~FieldLog();

public:
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);
	void update();
	void add(int const n);
	void insert(int const n, time_t const date);
	void del(int const n);
	bool set(std::string const value);
	void serialize_php(std::stringstream &out);
	void dump(std::stringstream &output);
	void dump_bin(FILE *f);
	void restore(Parser &parser);
	void restore_bin(FILE *f);
	void show(std::stringstream &out);
	std::string summary();
	UserScore score(int const rule = 0); 
	void clear();
	std::string name();
	time_t last_update();
};

class ReportField {
public:
	virtual bool add(Field const *field) = 0;
	virtual void serialize_php(std::stringstream &out) = 0; 
	virtual void show(std::stringstream &out) = 0;
	virtual ~ReportField();
};

class ReportFieldInt : public ReportField {
private:
	unsigned int count;
	unsigned int total;
public:
	bool add(Field const *field);
	void serialize_php(std::stringstream &out);
	void show(std::stringstream &out);

	ReportFieldInt();
};

class ReportFieldEvents : public ReportField {
private:
	unsigned int count;

	StatsVectorBase *hours;
	StatsVectorBase *days;
	StatsVectorBase *months;

	StatsVectorBase *active_hours;
	StatsVectorBase *active_days;
	StatsVectorBase *active_months;

	unsigned int total;

public:
	bool add(Field const *field);
	void serialize_php(std::stringstream &out);
	void show(std::stringstream &out);

	ReportFieldEvents();
	~ReportFieldEvents();
};

#endif
