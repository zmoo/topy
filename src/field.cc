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

#include "field.hh"

#include "stringutils.hh"
#include "timer.hh"
#include "macros.hh"
#include "stats.hh"
#include "replicator.hh"
#include "dump_bin.hh"
#include "help.hh"

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <typeinfo>

/** \brief get number of samples
 *
 *  \return the number of samples
 */
unsigned int StatsVectorBase::get_count() {
	return count;
}

StatsVectorBase::~StatsVectorBase() {
}

/** \brief constructor
 */
template <typename type_s, int len_s, bool is_unsigned>
StatsVector<type_s, len_s, is_unsigned>::StatsVector() {
	count = 0;
}

/** \brief reinitialisate a vector with samples from another
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::clear() {
	count = 0;
}

/** \brief dump human readable information about vector to standard output
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::dump(std::stringstream &output, bool const with_format) {
	if (with_format)
		output << sizeof(type_s) << ":";
	output << (int) count << ":";
	for (int i = 0; i < count; i++) {
		if (i != 0)
			output << ",";
		output << (int) samples[i];
	}
}

template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::dump_bin(FILE *f, bool const with_format) {
	if (with_format) {
		uint8_t samples_size = sizeof(type_s);
		DUMP_BIN(samples_size, f);
	}
	DUMP_BIN(count, f);
	fwrite(&samples, 1, sizeof(type_s) * count, f);
}

template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::restore(Parser &parser) {
	count = parser.read_int();
	count = MIN(count, len_s);
	parser.waitfor(':');
	for (int i = 0; i < count; i++) {
		samples[i] = parser.read_int();
		if (i < count - 1)
			parser.waitfor(',');
	}
}

template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::restore_bin(FILE *f) {
	RESTORE_BIN(count, f);
	count = MIN(count, len_s);
	fread(&samples, 1, sizeof(type_s) * count, f);
}

template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::serialize_php(std::stringstream &out) {
	out << "a:" << (int) count << ":{";
	for (int i = 0; i < count; i++)
		out << "i:" << i <<";i:" << (int) samples[i] << ";";
	out << "}";
}

/** \brief get a sample by index
 *
 *  \param n sample index
 *  \return sample
 */
template <typename type_s, int len_s, bool is_unsigned>
int StatsVector<type_s, len_s, is_unsigned>::get_sample(unsigned int const n) {
	return (n < count) ? samples[n] : 0;
}

/** \brief set value of a sample
 *
 *  \param n sample index
 *  \param value sample value
 *  \return true if success
 */
template <typename type_s, int len_s, bool is_unsigned>
bool StatsVector<type_s, len_s, is_unsigned>::set_sample(unsigned int n, int const value) {
	if (n < count) {
		samples[n] = value;
		return true;
	}
	return false;
}

/** \brief get sample size
 *
 * \return ssample size in bytes (1, 2 or 4)
 */
template <typename type_s, int len_s, bool is_unsigned>
unsigned int StatsVector<type_s, len_s, is_unsigned>::get_type_s() {
	return sizeof(type_s);
}

/** \brief get max number of samples in vector
 *
 *  \return max number of samples
 */
template <typename type_s, int len_s, bool is_unsigned>
unsigned int StatsVector<type_s, len_s, is_unsigned>::get_len_s() {
	return len_s;
}

/** \brief inc first sample of a vector
 *
 *  \param n value to add to the first sample (default value is 1)
 *  \return true or false if overflow
 */
template <typename type_s, int len_s, bool is_unsigned>
bool StatsVector<type_s, len_s, is_unsigned>::inc(int const n) {
	if (count == 0) {
		count++;
		samples[0] = 0;
	}
	int value = samples[0] + n;

	if (is_unsigned and value < 0) {
		samples[0] = 0;
		return true;
	}

	if (value != (int) ((type_s) value)) //overflow detection
		return false;

	samples[0] = value;
	return true;
}

template <typename type_s, int len_s, bool is_unsigned>
bool StatsVector<type_s, len_s, is_unsigned>::multiply(float const coef) {
	for (int i = 0; i < count; i++) {
		samples[i] = (type_s) ((float) samples[i] * coef);
	}
	return true;
}

template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::add(StatsVectorBase *vector) {
	int size = vector->get_count();

	if (count < size) {
		for (int i = count; i < size; i++) {
			samples[i] = 0;
		}
		count = size;
	}
	for (int i = 0; i < size; i++) {
		samples[i] += vector->get_sample(i);
	}
}

template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::count_active(StatsVectorBase *vector) {
	int size = vector->get_count();

	if (count < size) {
		for (int i = count; i < size; i++) {
			samples[i] = 0;
		}
		count = size;
	}
	for (int i = 0; i < size; i++) {
		if (vector->get_sample(i) != 0)
			samples[i]++;
	}
}

/** \brief calculate sum
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
int StatsVector<type_s, len_s, is_unsigned>::sum(int const n) {
	int j = (n != 0) ? MIN(n, count) : count;
	unsigned int sum = 0;
	for (int i = 0; i < j; i++)
		sum += samples[i];
	return sum;
}

/** \brief translate samples of n position
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::translate(unsigned int const n) {
	int delta = MIN(n, len_s);

	int i;
	for(i = MIN(count, len_s - delta) - 1; i >= 0; i--)
		samples[i + delta] = samples[i];

	for (i = 0; i < delta; i++)
		samples[i] = 0;

	count = MIN(count + delta, len_s);
}

/** \brief delete sample at position n
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
bool StatsVector<type_s, len_s, is_unsigned>::del(unsigned int const n) {
	if (n >= count)
		return false;

	for(int i = n; i < count - 1; i++)
		samples[i] = samples[i + 1];

	count--;
	return true;
}

/** \brief retrun position of a sample n (-1 if could not find)
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
int StatsVector<type_s, len_s, is_unsigned>::find(int const n) {
	for (int i = 0; i < count; i++)
		if (samples[i] == (type_s) n)
			return i;
	return -1;
}

/** \brief output debug information about a vector
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::debug() {
	std::cout << "Samples: ";
	for (int i = 0; i < count; i++) {
		if (i != 0)
			std::cout << ", ";
		std::cout << (int) samples[i];
	}
	std::cout << std::endl;
	std::cout << "is_unsigned: " << is_unsigned << std::endl;
	std::cout << "type: " << sizeof(type_s) << " x " << len_s << std::endl;
	std::cout << "total size: " << sizeof(StatsVector<type_s, len_s, is_unsigned>) << std::endl;
}

/** \brief return information about a vector
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::show(std::stringstream &out) {
	for (int i = 0; i < count; i++) {
		if (i != 0)
			out << ", ";
		out << (int) samples[i];
	}
}

/** \brief reinitialisate a vector with samples from another
 *
 */
template <typename type_s, int len_s, bool is_unsigned>
void StatsVector<type_s, len_s, is_unsigned>::copy(StatsVectorBase *src) {
	count = src->get_count();
	for (int i = 0; i < count; i++)
		samples[i] = src->get_sample(i);
}

//-------------------------------- Field --------------------------------//

bool Field::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {
	//!add <n>
	//!	Add n to field value
	if (parser->current == "add") {
		stats.inc(cmd_prefix + "add");

		int value = StringUtils::to_int(parser->next());
		PARSING_END(parser, result);

		add(value);

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " add " << value;
			replicator.add(replication_query);
		}

		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		result.send();
		return true;
	}

	//!get
	//!	Get field value
	else if (parser->current == "get") {
		stats.inc(cmd_prefix + "get");
		PARSING_END(parser, result);

		result.type = PHP_SERIALIZE;
		update();
		serialize_php(result.data);
		result.send();
		return true;
	}

	//!set <value>
	//!	Set field value
	else if (parser->current == "set") {
		stats.inc(cmd_prefix + "set");

		std::string value = parser->next();
		if (value == "") {
			RETURN_PARSE_ERROR(result, "expected <value>");
		}
		PARSING_END(parser, result);

		if (!set(value)) {
			RETURN_PARSE_ERROR(result, "Coult not set field");
		}

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " set " << value;
			replicator.add(replication_query);
		}

		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		result.send();
		return true;
	}

	//!rules
	//!	List score rules
	else if (parser->current == "rules") {
		stats.inc(cmd_prefix + "rules");
		PARSING_END(parser, result);

		result.type = PHP_SERIALIZE;
		rules_serialize_php(result.data);
		result.send();
		return true;
	}

	//!help
	//!	Show commands list
	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_FIELD;
		result.send();
		return true;
	}

	RETURN_NOT_VALID_CMD(result);
}

void Field::get_score_rules(ScoreRulesList &result) {
	result.clear();
}

int Field::get_rule_id(std::string name) {
	ScoreRulesList rules;
	get_score_rules(rules);
	ScoreRulesList::iterator it = rules.find(name);
	return (it == rules.end()) ? -1 : it->second;
}

void Field::rules_serialize_php(std::stringstream &out) {
	ScoreRulesList list;
	get_score_rules(list);

	int i = 0;
	out << "a:" << list.size() << ":{";
	for (ScoreRulesList::iterator it = list.begin(); it != list.end(); it++) {
		std::string name = it->first;
		out << "i:" << i << ";s:" << name.size() << ":\"" << name << "\";";
		++i;
	}
	out << "}";
}

Field::~Field() {
}

#define INC_OR_EXPAND(vector, size, n, type2, type4, is_unsigned) \
	if (!vector->inc(n)) { \
		StatsVectorBase *new_vector; \
		switch (vector->get_type_s()) { \
			case 1 :  \
				new_vector = new StatsVector<type2, size, is_unsigned>; \
				new_vector->copy(vector); \
				delete vector; \
				vector = new_vector; \
				vector->inc(n); \
				break; \
			case 2 : \
				new_vector = new StatsVector<type4, size, is_unsigned>; \
				new_vector->copy(vector); \
				delete vector; \
				vector = new_vector; \
				vector->inc(n); \
				break; \
			case 4 : \
				std::cerr << "Overflow" << std::endl; \
				break; \
			default : \
				std::cerr << "Not a valid sample size" << std::endl; \
		} \
	}

#define RESTORE_VECTOR(parser, vector, size, type1, type2, type4, is_unsigned) \
	parser.waitfor('{'); \
	switch (parser.read_int()) { \
		case 1 : \
			vector = new StatsVector<type1, size, is_unsigned>; \
			break; \
		case 2 : \
			vector = new StatsVector<type2, size, is_unsigned>; \
			break; \
		case 4 : \
			vector = new StatsVector<type4, size, is_unsigned>; \
			break; \
	} \
	parser.waitfor(':'); \
	vector->restore(parser); \
	parser.waitfor('}');

#define RESTORE_BIN_VECTOR(f, vector, size, type1, type2, type4, is_unsigned) \
	{ \
		uint8_t samples_size; \
		if (RESTORE_BIN_SAFE(samples_size, f)) { \
			switch (samples_size) { \
				case 1 : \
					vector = new StatsVector<type1, size, is_unsigned>; \
					break; \
				case 2 : \
					vector = new StatsVector<type2, size, is_unsigned>; \
					break; \
				case 4 : \
					vector = new StatsVector<type4, size, is_unsigned>; \
					break; \
			} \
			vector->restore_bin(f); \
		} \
	}

//-------------------------------- FieldEvents --------------------------------//

bool FieldEvents::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {
	if (parser->current == "total") {
		parser->next();

		//events!total get
		//events!	Get total
		if (parser->current == "get") {
			stats.inc(cmd_prefix + "total::get");

			PARSING_END(parser, result);

			result.type = PHP_SERIALIZE;
			result.data << "i:" << total << ";";
			result.send();
			return true;
		}

		//events!total set <value>
		//events!	Set total
		else if (parser->current == "set") {
			stats.inc(cmd_prefix + "total::set");

			if (parser->next() == "") {
				RETURN_PARSE_ERROR(result, "expected <value>");
			}
			unsigned int value = StringUtils::to_uint(parser->current);
			PARSING_END(parser, result);

			total = value;

			//replication
			if (replicator.opened and !result.replicated) {
				replication_query << " total set " << total;
				replicator.add(replication_query);
			}

			result.type = PHP_SERIALIZE;
			result.data << "i:" << total << ";";
			result.send();
			return true;
		}

		RETURN_NOT_VALID_CMD(result);
	}

	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_FIELD << HELP_FIELD_EVENTS;
		result.send();
		return true;
	}

	return Field::parse_query(result, cmd_prefix, parser, replication_query);
}

/** \brief inc counter for actual month & day
 *
 *  \param n
 */
void FieldEvents::add(int const n) {
	update();
	total = MAX(0, (int) (total + n));
	INC_OR_EXPAND(hours, EVENTS_HOURS_LEN, n, uint16_t, uint32_t, true);
	INC_OR_EXPAND(days, EVENTS_DAYS_LEN, n, uint16_t, uint32_t, true);
	INC_OR_EXPAND(months, EVENTS_MONTHS_LEN, n, uint16_t, uint32_t, true);
	last_inc = timer.now;
}

void FieldEvents::get_score_rules(ScoreRulesList &result) {
	result.clear();
	result.insert(std::pair<std::string, int> ("hours::sum", FIELD_EVENTS_HOURS_SUM));
	result.insert(std::pair<std::string, int> ("hours::last", FIELD_EVENTS_HOURS_LAST));
	result.insert(std::pair<std::string, int> ("hours::penultimate", FIELD_EVENTS_HOURS_PENULTIMATE));
	result.insert(std::pair<std::string, int> ("hours::last2", FIELD_EVENTS_HOURS_LAST2));
	result.insert(std::pair<std::string, int> ("hours::last3", FIELD_EVENTS_HOURS_LAST3));
	result.insert(std::pair<std::string, int> ("hours::last6", FIELD_EVENTS_HOURS_LAST6));
	result.insert(std::pair<std::string, int> ("hours::last12", FIELD_EVENTS_HOURS_LAST12));

	result.insert(std::pair<std::string, int> ("days::sum", FIELD_EVENTS_DAYS_SUM));
	result.insert(std::pair<std::string, int> ("days::last", FIELD_EVENTS_DAYS_LAST));
	result.insert(std::pair<std::string, int> ("days::penultimate", FIELD_EVENTS_DAYS_PENULTIMATE));
	result.insert(std::pair<std::string, int> ("days::last2", FIELD_EVENTS_DAYS_LAST2));
	result.insert(std::pair<std::string, int> ("days::last7", FIELD_EVENTS_DAYS_LAST7));
	result.insert(std::pair<std::string, int> ("days::last15", FIELD_EVENTS_DAYS_LAST15));

	result.insert(std::pair<std::string, int> ("months::sum", FIELD_EVENTS_MONTHS_SUM));
	result.insert(std::pair<std::string, int> ("months::last", FIELD_EVENTS_MONTHS_LAST));
	result.insert(std::pair<std::string, int> ("months::penultimate", FIELD_EVENTS_MONTHS_PENULTIMATE));
	result.insert(std::pair<std::string, int> ("months::last2", FIELD_EVENTS_MONTHS_LAST2));
	result.insert(std::pair<std::string, int> ("months::last3", FIELD_EVENTS_MONTHS_LAST3));
	result.insert(std::pair<std::string, int> ("months::last6", FIELD_EVENTS_MONTHS_LAST6));

	result.insert(std::pair<std::string, int> ("total", FIELD_EVENTS_TOTAL));
}

/** \brief calculate score for visits
 *
 */
UserScore FieldEvents::score(int const rule) {
	update();
	switch (rule) {
		case FIELD_EVENTS_HOURS_SUM:
			return hours->sum();
		case FIELD_EVENTS_HOURS_LAST:
			return hours->get_sample(0);
		case FIELD_EVENTS_HOURS_PENULTIMATE:
			return hours->get_sample(1);
		case FIELD_EVENTS_HOURS_LAST2:
			return hours->sum(2);
		case FIELD_EVENTS_HOURS_LAST3:
			return hours->sum(3);
		case FIELD_EVENTS_HOURS_LAST6:
			return hours->sum(6);
		case FIELD_EVENTS_HOURS_LAST12:
			return hours->sum(12);

		case FIELD_EVENTS_DAYS_SUM:
			return days->sum();
		case FIELD_EVENTS_DAYS_LAST:
			return days->get_sample(0);
		case FIELD_EVENTS_DAYS_PENULTIMATE:
			return days->get_sample(1);
		case FIELD_EVENTS_DAYS_LAST2:
			return days->sum(2);
		case FIELD_EVENTS_DAYS_LAST7:
			return days->sum(7);
		case FIELD_EVENTS_DAYS_LAST15:
			return days->sum(15);

		case FIELD_EVENTS_MONTHS_SUM:
			return months->sum();
		case FIELD_EVENTS_MONTHS_LAST:
			return months->get_sample(0);
		case FIELD_EVENTS_MONTHS_PENULTIMATE:
			return months->get_sample(1);
		case FIELD_EVENTS_MONTHS_LAST2:
			return months->sum(2);
		case FIELD_EVENTS_MONTHS_LAST3:
			return months->sum(3);
		case FIELD_EVENTS_MONTHS_LAST6:
			return months->sum(6);

		case FIELD_EVENTS_TOTAL:
			return total;
	}
	return -1;
}

/** \brief
 *
 */
void FieldEvents::update() {
	timer.refresh();

	if (date.hour != -1) {
		int delta = timer.hour - date.hour;
		if (delta > 0)
			hours->translate(delta);
	}

	if (date.day != -1) {
		int delta = timer.day - date.day;
		if (delta > 0)
			days->translate(delta);
	}

	if (date.month != -1) {
		int delta = timer.month - date.month;
		if (delta > 0)
			months->translate(delta);
	}

	date.day = timer.day;
	date.hour = timer.hour;
	date.month = timer.month;
}

/** \brief output debug information about a vectors
 *
 */
void FieldEvents::debug() {
	std::cout << "----[ Hours ]----" << std::endl;
	hours->debug();
	std::cout << "----[ Days ]----" << std::endl;
	days->debug();
	std::cout << "----[ Months ]----" << std::endl;
	months->debug();
}

/** \brief translate samples of vectors months & days
 *
 *  \param delta_days
 *  \param delta_months
 */
void FieldEvents::translate(unsigned int const delta_hours, unsigned int const delta_days, unsigned int const delta_months) {
	hours->translate(delta_hours);
	days->translate(delta_days);
	months->translate(delta_months);
}

void FieldEvents::serialize_php(std::stringstream &out) {
	out << "a:6:{";
	out << "s:2:\"ts\";i:" << timer.now << ";";
	out << "s:4:\"last\";i:" << last_inc << ";";
	out << "s:5:\"total\";i:" << total << ";";
	out << "s:5:\"hours\";";
	hours->serialize_php(out);
	out << "s:4:\"days\";";
	days->serialize_php(out);
	out << "s:6:\"months\";";
	months->serialize_php(out);
	out << "}";
}

/** \brief dump human readable information about a Events object
 *
 */
void FieldEvents::dump(std::stringstream &output) {
	output << "v{" << "t{" << date.hour << ":" << date.day << ":" << date.month << "}:" << last_inc << ":" << total;
	output << ":h{";
	hours->dump(output);
	output << "}";
	output << ":d{";
	days->dump(output);
	output << "}";
	output << ":m{";
	months->dump(output);
	output << "}";
	output << "}";
}

void FieldEvents::dump_bin(FILE *f) {
	DUMP_BIN(date.hour, f);
	DUMP_BIN(date.day, f);
	DUMP_BIN(date.month, f);
	DUMP_BIN(last_inc, f);
	DUMP_BIN(total, f);
	hours->dump_bin(f);
	days->dump_bin(f);
	months->dump_bin(f);
}

/** \brief restore
 *
 */
void FieldEvents::restore(Parser &parser) {
	parser.waitfor('v');
	parser.waitfor('{');
	parser.waitfor('t');
	parser.waitfor('{');
	date.hour = parser.read_int();
	parser.waitfor(':');
//	date.hour = -1;

	date.day = parser.read_int();
	parser.waitfor(':');
	date.month = parser.read_int();
	parser.waitfor('}');
	parser.waitfor(':');
	last_inc = parser.read_int();
	parser.waitfor(':');
	total = parser.read_int();
	parser.waitfor(':');
	parser.waitfor('h');
	RESTORE_VECTOR(parser, hours, EVENTS_HOURS_LEN, uint8_t, uint16_t, uint32_t, true);
//	hours = new StatsVector<uint8_t, EVENTS_HOURS_LEN, true>;

	parser.waitfor(':');
	parser.waitfor('d');
	RESTORE_VECTOR(parser, days, EVENTS_DAYS_LEN, uint8_t, uint16_t, uint32_t, true);
	parser.waitfor(':');
	parser.waitfor('m');
	RESTORE_VECTOR(parser, months, EVENTS_MONTHS_LEN, uint8_t, uint16_t, uint32_t, true);
	parser.waitfor('}');
}

void FieldEvents::restore_bin(FILE *f) {
	RESTORE_BIN(date.hour, f);
	RESTORE_BIN(date.day, f);
	RESTORE_BIN(date.month, f);
	RESTORE_BIN(last_inc, f);
	RESTORE_BIN(total, f);
	RESTORE_BIN_VECTOR(f, hours, EVENTS_HOURS_LEN, uint8_t, uint16_t, uint32_t, true);
	RESTORE_BIN_VECTOR(f, days, EVENTS_DAYS_LEN, uint8_t, uint16_t, uint32_t, true);
	RESTORE_BIN_VECTOR(f, months, EVENTS_MONTHS_LEN, uint8_t, uint16_t, uint32_t, true);
}

void FieldEvents::show(std::stringstream &out) {
	out << "Last inc: " << last_inc << std::endl;
	out << "Last hours: ";
	hours->show(out);
	out << std::endl;
	out << "Last days: ";
	days->show(out);
	out << std::endl;
	out << "Last months: ";
	months->show(out);
	out << std::endl;
	out << "Total: " << total << std::endl;
}

std::string FieldEvents::summary() {
	std::stringstream result;
	result << hours->get_sample(0) << "\t" << days->get_sample(0) << "\t" << months->get_sample(0) << "\t" << total;
	return result.str();
}

bool FieldEvents::set(std::string const value) {
	return false;
}

/** \brief constructor
 *
 */
FieldEvents::FieldEvents(bool const alloc) {
	init();
	if (alloc) {
		hours = new StatsVector<uint8_t, EVENTS_HOURS_LEN, true>;
		days = new StatsVector<uint8_t, EVENTS_DAYS_LEN, true>;
		months = new StatsVector<uint16_t, EVENTS_MONTHS_LEN, true>;
	}
	else {
		hours = NULL;
		days = NULL;
		months = NULL;
	}
}

void FieldEvents::init() {
	date.hour = -1;
	date.day = -1;
	date.month = -1;

	total = 0;
	last_inc = 0;
}

/** \brief destructor
 *
 */
FieldEvents::~FieldEvents() {
	if (hours) delete hours;
	if (days) delete days;
	if (months) delete months;
}

void FieldEvents::clear() {
	init();
	hours->clear();
	days->clear();
	months->clear();
}

std::string FieldEvents::name() {
	return FIELD_EVENTS_TYPE_NAME;
}

time_t FieldEvents::last_update() {
	return last_inc;
}

//-------------------------------- FieldMarks --------------------------------//
void FieldMarks::update() {
	timer.refresh();

	if (date.month != -1) {
		int delta = timer.month - date.month;
		if (delta > 0)
			translate(delta);
	}

	date.month = timer.month;
}

void FieldMarks::add(int const n) {
	update();
	INC_OR_EXPAND(months_num, MARKS_MONTHS_LEN, n, int16_t, int32_t, false);
	INC_OR_EXPAND(months_denom, MARKS_MONTHS_LEN, 1, uint16_t, uint32_t, true);
}

void FieldMarks::debug() {
	std::cout << "----[ Numerator ]----" << std::endl;
	months_num->debug();
	std::cout << "----[ Denominator ]----" << std::endl;
	months_denom->debug();
}

void FieldMarks::translate(unsigned int const delta_months) {
	months_num->translate(delta_months);
	months_denom->translate(delta_months);
}

void FieldMarks::serialize_php(std::stringstream &out) {
	out << "a:3:{";
	out << "s:2:\"ts\";i:" << timer.now << ";";
	out << "s:9:\"numerator\";";
	months_num->serialize_php(out);
	out << "s:11:\"denominator\";";
	months_denom->serialize_php(out);
	out << "}";
}

void FieldMarks::dump(std::stringstream &output) {
	output << "ma{" << "t{" << date.month << "}:nu{";
	months_num->dump(output);
	output << "}:de{";
	months_denom->dump(output);
	output << "}}";
}

void FieldMarks::dump_bin(FILE *f) {
	DUMP_BIN(date.month, f);
	months_num->dump_bin(f);
	months_denom->dump_bin(f);
}


/** \brief restore
 *
 */
void FieldMarks::restore(Parser &parser) {
	parser.waitfor('m');
	parser.waitfor('a');
	parser.waitfor('{');
	parser.waitfor('t');
	parser.waitfor('{');
	date.month = parser.read_int();
	parser.waitfor('}');
	parser.waitfor(':');
	parser.waitfor('n');
	parser.waitfor('u');
	RESTORE_VECTOR(parser, months_num, MARKS_MONTHS_LEN, int8_t, int16_t, int32_t, false);
	parser.waitfor(':');
	parser.waitfor('d');
	parser.waitfor('e');
	RESTORE_VECTOR(parser, months_denom, MARKS_MONTHS_LEN, uint8_t, uint16_t, uint32_t, true);
	parser.waitfor('}');
}

void FieldMarks::restore_bin(FILE *f) {
	RESTORE_BIN(date.month, f);
	RESTORE_BIN_VECTOR(f, months_num, MARKS_MONTHS_LEN, int8_t, int16_t, int32_t, false);
	RESTORE_BIN_VECTOR(f, months_denom, MARKS_MONTHS_LEN, uint8_t, uint16_t, uint32_t, true);
}

void FieldMarks::show(std::stringstream &out) {
	out << "Last months numerator: ";
	months_num->show(out);
	out << std::endl;
	out << "Last months denominator: ";
	months_denom->show(out);
	out << std::endl;
}

std::string FieldMarks::summary() {
	std::stringstream result;
	int denom = months_denom->sum();
	if (denom == 0)
		result << "UNDEF";
	else
		result << (float) months_num->sum() / (float) denom;
	result << " (" << denom << ")";
	return result.str();
}

void FieldMarks::get_score_rules(ScoreRulesList &result) {
	result.clear();
	result.insert(std::pair<std::string, int> ("last::average", FIELD_MARKS_LAST_AVERAGE));
	result.insert(std::pair<std::string, int> ("last2::average", FIELD_MARKS_LAST2_AVERAGE));
	result.insert(std::pair<std::string, int> ("last3::average", FIELD_MARKS_LAST3_AVERAGE));
	result.insert(std::pair<std::string, int> ("last6::average", FIELD_MARKS_LAST6_AVERAGE));
	result.insert(std::pair<std::string, int> ("sum::average", FIELD_MARKS_SUM_AVERAGE));

	result.insert(std::pair<std::string, int> ("last::count", FIELD_MARKS_LAST_COUNT));
	result.insert(std::pair<std::string, int> ("last2::count", FIELD_MARKS_LAST2_COUNT));
	result.insert(std::pair<std::string, int> ("last3::count", FIELD_MARKS_LAST3_COUNT));
	result.insert(std::pair<std::string, int> ("last6::count", FIELD_MARKS_LAST6_COUNT));
	result.insert(std::pair<std::string, int> ("sum::count", FIELD_MARKS_SUM_COUNT));

	result.insert(std::pair<std::string, int> ("last::skyverage", FIELD_MARKS_LAST_SKYVERAGE));
	result.insert(std::pair<std::string, int> ("last2::skyverage", FIELD_MARKS_LAST2_SKYVERAGE));
	result.insert(std::pair<std::string, int> ("last3::skyverage", FIELD_MARKS_LAST3_SKYVERAGE));
	result.insert(std::pair<std::string, int> ("last6::skyverage", FIELD_MARKS_LAST6_SKYVERAGE));
	result.insert(std::pair<std::string, int> ("sum::skyverage", FIELD_MARKS_SUM_SKYVERAGE));
}

#define RETURN_SCORE_MARKS_SKYVERAGE(n) \
	return (months_num->sum(n) * 1000) /  (months_denom->sum(n) + 1);

#define RETURN_SCORE_MARKS_COUNT(n) \
	return months_denom->sum(n);

#define RETURN_SCORE_MARKS_AVERAGE(n) \
	denom = months_denom->sum(n); \
	return (denom != 0) ? ((months_num->sum(n) * 1000) /  denom) : 0;

UserScore FieldMarks::score(int const rule) {
	update();
	int denom;
	switch (rule) {
		case FIELD_MARKS_LAST_AVERAGE:  RETURN_SCORE_MARKS_AVERAGE(1)
		case FIELD_MARKS_LAST2_AVERAGE: RETURN_SCORE_MARKS_AVERAGE(2)
		case FIELD_MARKS_LAST3_AVERAGE: RETURN_SCORE_MARKS_AVERAGE(3)
		case FIELD_MARKS_LAST6_AVERAGE: RETURN_SCORE_MARKS_AVERAGE(6)
		case FIELD_MARKS_SUM_AVERAGE:   RETURN_SCORE_MARKS_AVERAGE(0)

		case FIELD_MARKS_LAST_SKYVERAGE:  RETURN_SCORE_MARKS_SKYVERAGE(1)
		case FIELD_MARKS_LAST2_SKYVERAGE: RETURN_SCORE_MARKS_SKYVERAGE(2)
		case FIELD_MARKS_LAST3_SKYVERAGE: RETURN_SCORE_MARKS_SKYVERAGE(3)
		case FIELD_MARKS_LAST6_SKYVERAGE: RETURN_SCORE_MARKS_SKYVERAGE(6)
		case FIELD_MARKS_SUM_SKYVERAGE:   RETURN_SCORE_MARKS_SKYVERAGE(0)

		case FIELD_MARKS_LAST_COUNT:  RETURN_SCORE_MARKS_COUNT(1)
		case FIELD_MARKS_LAST2_COUNT: RETURN_SCORE_MARKS_COUNT(2)
		case FIELD_MARKS_LAST3_COUNT: RETURN_SCORE_MARKS_COUNT(3)
		case FIELD_MARKS_LAST6_COUNT: RETURN_SCORE_MARKS_COUNT(6)
		case FIELD_MARKS_SUM_COUNT:   RETURN_SCORE_MARKS_COUNT(0)
	}
	return -1;
}

bool FieldMarks::set(std::string const value) {
	return false;
}

/** \brief constructor
 *
 */
FieldMarks::FieldMarks(bool const alloc) {
	init();
	if (alloc) {
		months_num = new StatsVector<int8_t, MARKS_MONTHS_LEN, false>;
		months_denom = new StatsVector<uint8_t, MARKS_MONTHS_LEN, true>;
	}
	else {
		months_num = NULL;
		months_denom = NULL;
	}
}

void FieldMarks::init() {
	date.month = -1;
}

/** \brief destructor
 *
 */
FieldMarks::~FieldMarks() {
	if (months_num) delete months_num;
	if (months_denom) delete months_denom;
}

void FieldMarks::clear() {
	init();
	months_num->clear();
	months_denom->clear();
}

std::string FieldMarks::name() {
	return FIELD_MARKS_TYPE_NAME;
}

time_t FieldMarks::last_update() {
	return 0;
}

//-------------------------------- FieldInt --------------------------------//
void FieldInt::set(int const _value) {
	value = _value;
	if (is_unsigned and value < 0)
		value = 0;
}

void FieldInt::add(int const n) {
	set(value + n);
}

bool FieldInt::set(std::string const _value) {
	set(StringUtils::to_int(_value));
	return true;
}

void FieldInt::serialize_php(std::stringstream &out) {
	out << "i:" << value << ";";
}

void FieldInt::dump(std::stringstream &output) {
	output << "i{" << value << "}";
}

void FieldInt::dump_bin(FILE *f) {
	DUMP_BIN(value, f);
}

void FieldInt::restore(Parser &parser) {
	parser.waitfor('i');
	parser.waitfor('{');
	value = parser.read_int();
	parser.waitfor('}');
}

void FieldInt::restore_bin(FILE *f) {
	RESTORE_BIN(value, f);
}

void FieldInt::show(std::stringstream &out) {
	out << value << std::endl;
}

std::string FieldInt::summary() {
	std::stringstream result;
	result << value;
	return result.str();
}

UserScore FieldInt::score(int const rule) {
	return value;
}

void FieldInt::update() {
}

FieldInt::FieldInt(bool const _is_unsigned) {
	is_unsigned = _is_unsigned;
	value = 0;
}

void FieldInt::clear() {
	value = 0;
}

std::string FieldInt::name() {
	return (is_unsigned ? FIELD_UINT_TYPE_NAME : FIELD_INT_TYPE_NAME);
}

time_t FieldInt::last_update() {
	return 0;
}

//-------------------------------- FieldTimestamp --------------------------------//
void FieldTimestamp::add(int const n) {
	value = timer.refresh();
}

bool FieldTimestamp::set(std::string const _value) {
	value = StringUtils::to_uint(_value);
	return true;
}

void FieldTimestamp::serialize_php(std::stringstream &out) {
	out << "i:" << value << ";";
}

void FieldTimestamp::dump(std::stringstream &output) {
	output << "t{" << value << "}";
}

void FieldTimestamp::dump_bin(FILE *f) {
	DUMP_BIN(value, f);
}

void FieldTimestamp::restore(Parser &parser) {
	parser.waitfor('t');
	parser.waitfor('{');
	value = parser.read_int();
	parser.waitfor('}');
}

void FieldTimestamp::restore_bin(FILE *f) {
	RESTORE_BIN(value, f);
}

void FieldTimestamp::show(std::stringstream &out) {
	out << value << std::endl;
}

std::string FieldTimestamp::summary() {
	std::stringstream result;
	result << value;
	return result.str();
}

UserScore FieldTimestamp::score(int const rule) {
	return value;
}

void FieldTimestamp::update() {
}

void FieldTimestamp::clear() {
	value = 0;
}

std::string FieldTimestamp::name() {
	return FIELD_TIMESTAMP_TYPE_NAME;
}

FieldTimestamp::FieldTimestamp() {
	value = 0;
}

time_t FieldTimestamp::last_update() {
	return value;
}

//-------------------------------- FieldUlog --------------------------------//
bool FieldUlog::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {
	//ulog!insert [unique] <id> [,<date>]
	//ulog!	Insert item
	if (parser->current == "insert") {
		stats.inc(cmd_prefix + "insert");

		parser->next();
		bool unique = false;
		if (parser->current == "unique") {
			unique = true;
			parser->next();
		}

		int n = StringUtils::to_int(parser->current);
		parser->next();
		time_t date = (parser->current == ",") ? StringUtils::to_int(parser->next()) : timer.refresh();
		PARSING_END(parser, result);

		if (unique)
			del(n);
		insert(n, date);

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " insert " << ((unique) ? "unique " : "") << n << "," << date;
			replicator.add(replication_query);
		}

		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		result.send();
		return true;
	}

	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_FIELD << HELP_FIELD_ULOG;
		result.send();
		return true;
	}

	return Field::parse_query(result, cmd_prefix, parser, replication_query);
}

void FieldUlog::del(int const n) {
	int pos = items.find(n);
	if (n != -1) {
		items.del(pos);
		dates.del(pos);
	}
}

void FieldUlog::insert(int const n, time_t const date) {
	items.translate(1);
	items.set_sample(0, n);
	dates.translate(1);
	dates.set_sample(0, date);
}

void FieldUlog::add(int const n) {
	insert(n, timer.refresh());
}

bool FieldUlog::set(std::string const value) {
	return true;
}

void FieldUlog::serialize_php(std::stringstream &out) {
	out << "a:2:{";
	out << "s:5:\"items\";";
	items.serialize_php(out);
	out << "s:5:\"dates\";";
	dates.serialize_php(out);
	out << "}";
}

void FieldUlog::dump(std::stringstream &output) {
	output << "uL{i{";
	items.dump(output, false);
	output << "}:d{";
	dates.dump(output, false);
	output << "}}";
}

void FieldUlog::dump_bin(FILE *f) {
	items.dump_bin(f, false);
	dates.dump_bin(f, false);
}

void FieldUlog::restore(Parser &parser) {
	parser.waitfor('u');
	parser.waitfor('L');
	parser.waitfor('{');
	parser.waitfor('i');
	parser.waitfor('{');
	items.restore(parser);
	parser.waitfor('}');
	parser.waitfor(':');
	parser.waitfor('d');
	parser.waitfor('{');
	dates.restore(parser);
	parser.waitfor('}');
	parser.waitfor('}');
}

void FieldUlog::restore_bin(FILE *f) {
	items.restore_bin(f);
	dates.restore_bin(f);
}

void FieldUlog::show(std::stringstream &out) {
	out << "Items: ";
	items.show(out);
	out << std::endl;
	out << "Dates: ";
	dates.show(out);
	out << std::endl;
}

std::string FieldUlog::summary() {
	std::stringstream result;
	result << items.get_count();
	return result.str();
}

UserScore FieldUlog::score(int const rule) {
	return items.get_count();
}

FieldUlog::FieldUlog() {
}

FieldUlog::~FieldUlog() {
}

void FieldUlog::update() {
}

void FieldUlog::clear() {
	items.clear();
	dates.clear();
}

std::string FieldUlog::name() {
	return FIELD_ULOG_TYPE_NAME;
}

time_t FieldUlog::last_update() {
	return dates.get_sample(0);
}

//-------------------------------- FieldLog --------------------------------//
bool FieldLog::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {
	//log!insert [unique] <id> [,<date>]
	//log!	Insert item
	if (parser->current == "insert") {
		stats.inc(cmd_prefix + "insert");

		parser->next();
		bool unique = false;
		if (parser->current == "unique") {
			unique = true;
			parser->next();
		}

		int n = StringUtils::to_int(parser->current);
		parser->next();
		time_t date = (parser->current == ",") ? StringUtils::to_int(parser->next()) : timer.refresh();
		PARSING_END(parser, result);

		if (unique)
			del(n);
		insert(n, date);

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << " insert " << ((unique) ? "unique " : "") << n << "," << date;
			replicator.add(replication_query);
		}

		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		result.send();
		return true;
	}

	else if (parser->current == "help") {
		stats.inc("misc");

		PARSING_END(parser, result);
		result.data << HELP_FIELD << HELP_FIELD_LOG;
		result.send();
		return true;
	}

	return Field::parse_query(result, cmd_prefix, parser, replication_query);
}

void FieldLog::del(int const n) {
	int pos = items.find(n);
	if (n != -1) {
		items.del(pos);
		dates.del(pos);
	}
}

void FieldLog::insert(int const n, time_t const date) {
	items.translate(1);
	items.set_sample(0, n);
	dates.translate(1);
	dates.set_sample(0, date);
}

void FieldLog::add(int const n) {
	insert(n, timer.refresh());
}

bool FieldLog::set(std::string const value) {
	return true;
}

void FieldLog::serialize_php(std::stringstream &out) {
	out << "a:2:{";
	out << "s:5:\"items\";";
	items.serialize_php(out);
	out << "s:5:\"dates\";";
	dates.serialize_php(out);
	out << "}";
}

void FieldLog::dump(std::stringstream &output) {
	output << "bL{i{";
	items.dump(output, false);
	output << "}:d{";
	dates.dump(output, false);
	output << "}}";
}

void FieldLog::dump_bin(FILE *f) {
	items.dump_bin(f, false);
	dates.dump_bin(f, false);
}

void FieldLog::restore(Parser &parser) {
	parser.waitfor('b');
	parser.waitfor('L');
	parser.waitfor('{');
	parser.waitfor('i');
	parser.waitfor('{');
	items.restore(parser);
	parser.waitfor('}');
	parser.waitfor(':');
	parser.waitfor('d');
	parser.waitfor('{');
	dates.restore(parser);
	parser.waitfor('}');
	parser.waitfor('}');
}

void FieldLog::restore_bin(FILE *f) {
	items.restore_bin(f);
	dates.restore_bin(f);
}

void FieldLog::show(std::stringstream &out) {
	out << "Items: ";
	items.show(out);
	out << std::endl;
	out << "Dates: ";
	dates.show(out);
	out << std::endl;
}

std::string FieldLog::summary() {
	std::stringstream result;
	result << items.get_count();
	return result.str();
}

UserScore FieldLog::score(int const rule) {
	return items.get_count();
}

FieldLog::FieldLog() {
}

FieldLog::~FieldLog() {
}

void FieldLog::update() {
}

void FieldLog::clear() {
	items.clear();
	dates.clear();
}

std::string FieldLog::name() {
	return FIELD_LOG_TYPE_NAME;
}

time_t FieldLog::last_update() {
	return dates.get_sample(0);
}

//-------------------------------- Report --------------------------------//
ReportField::~ReportField() {
}

ReportFieldInt::ReportFieldInt() {
	count = 0;
	total = 0;
}

bool ReportFieldInt::add(Field const *field) {
	if (typeid(*field) == typeid(FieldInt)) {
		FieldInt *field_int = (FieldInt *) field;
		count++;
		total += field_int->value;
		return true;
	}
	return false;
}

void ReportFieldInt::serialize_php(std::stringstream &out) {
	out << "a:4:{";
	out << "s:2:\"ts\";i:" << timer.now << ";";
	out << "s:5:\"count\";i:" << count << ";";
	out << "s:5:\"total\";i:" << total << ";";
	out << "s:7:\"average\";";
	if (count != 0)
		out << "d:" << ((double) total / count) << ";";
	else
		out << "b:0;";

	out << "}";
}

void ReportFieldInt::show(std::stringstream &out) {
	out << "ts: " << timer.now << std::endl;
	out << "count: " << count << std::endl;
	out << "total: " << total << std::endl;
	if (count != 0)
		out << "average: " << ((double) total / count) << std::endl;
}

ReportFieldEvents::ReportFieldEvents() {
	count = 0;

	total = 0;
	months = new StatsVector<uint64_t, EVENTS_MONTHS_LEN, true>;
	days = new StatsVector<uint64_t, EVENTS_DAYS_LEN, true>;
	hours = new StatsVector<uint64_t, EVENTS_HOURS_LEN, true>;
	active_months = new StatsVector<uint32_t, EVENTS_MONTHS_LEN, true>;
	active_days = new StatsVector<uint32_t, EVENTS_DAYS_LEN, true>;
	active_hours = new StatsVector<uint32_t, EVENTS_HOURS_LEN, true>;
}

ReportFieldEvents::~ReportFieldEvents() {
	delete months;
	delete days;
	delete hours;
	delete active_months;
	delete active_days;
	delete active_hours;
}

bool ReportFieldEvents::add(Field const *field) {
	if (typeid(*field) == typeid(FieldEvents)) {
		FieldEvents *field_events = (FieldEvents *) field;
		field_events->update();

		count++;
		total += field_events->total;

		months->add(field_events->months);
		days->add(field_events->days);
		hours->add(field_events->hours);

		active_months->count_active(field_events->months);
		active_days->count_active(field_events->days);
		active_hours->count_active(field_events->hours);

		return true;
	}
	return false;
}

void ReportFieldEvents::serialize_php(std::stringstream &out) {
	out << "a:7:{";
	out << "s:2:\"ts\";i:" << timer.now << ";";
	out << "s:5:\"count\";i:" << count << ";";
	out << "s:5:\"total\";i:" << total << ";";
	out << "s:6:\"months\";";
	months->serialize_php(out);
	out << "s:4:\"days\";";
	days->serialize_php(out);
	out << "s:5:\"hours\";";
	hours->serialize_php(out);

	out << "s:6:\"active\";";
	out << "a:3:{";
	out << "s:6:\"months\";";
	active_months->serialize_php(out);
	out << "s:4:\"days\";";
	active_days->serialize_php(out);
	out << "s:5:\"hours\";";
	active_hours->serialize_php(out);
	out << "}";

	out << "}";
}

void ReportFieldEvents::show(std::stringstream &out) {
	out << "ts: " << timer.now << std::endl;
	out << "count: " << count << std::endl;
	out << "total: " << total << std::endl;

	out << "months: ";
	months->show(out);
	out << std::endl;
	out << "days: ";
	days->show(out);
	out << std::endl;
	out << "hours: ";
	hours->show(out);
	out << std::endl;

	out << "Active users by :" << std::endl;
	out << "* months: ";
	active_months->show(out);
	out << std::endl;
	out << "* days: ";
	active_days->show(out);
	out << std::endl;
	out << "* hours: ";
	active_hours->show(out);
	out << std::endl;
}

