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

#include "expr_bool.hh"
#include "groups.hh"
#include <iostream>
#include <cstring>

std::string ExprBool::get_str(ExprContext const *context) {
	return (get(context)) ? "true" : "false";
}

ExprBool::~ExprBool() {
}

bool ExprBoolConst::get(ExprContext const *) {
	return value;
}

void ExprBoolConst::show(ExprContext const *context) {
	std::cout << get_str(context);
}

ExprBoolConst::ExprBoolConst(bool _value) {
	value = _value;
}

bool ExprBoolVar::get(ExprContext const *) {
	return *value != 0;
}

void ExprBoolVar::show(ExprContext const *) {
	std::cout << "(" << *value << " != 0)";
}

ExprBoolVar::ExprBoolVar(ExprVar *_value) {
	value = _value;
}

void ExprBoolNot::show(ExprContext const *context) {
	std::cout << "!";
	std::cout << "(";
	child->show(context);
	std::cout << ")";
}

bool ExprBoolNot::get(ExprContext const *context) {
	return not(child->get(context));
}

ExprBoolNot::ExprBoolNot(ExprBool *_child) {
	child = _child;
}

ExprBoolNot::~ExprBoolNot() {
	delete child;
}

ExprBoolOp::~ExprBoolOp() {
	delete child[0];
	delete child[1];
}

void ExprBoolAnd::show(ExprContext const *context) {
	std::cout << "(";
	child[0]->show(context);
	std::cout << " AND ";
	child[1]->show(context);
	std::cout << ")";
}

ExprBoolAnd::ExprBoolAnd(ExprBool *a, ExprBool *b) {
	child[0] = b;
	child[1] = a;
}

bool ExprBoolAnd::get(ExprContext const *context) {
	return (child[0]->get(context) and child[1]->get(context));
}

void ExprBoolOr::show(ExprContext const *context) {
	std::cout << "(";
	child[0]->show(context);
	std::cout << " OR ";
	child[1]->show(context);
	std::cout << ")";
}

bool ExprBoolOr::get(ExprContext const *context) {
	return (child[0]->get(context) or child[1]->get(context));
}

ExprBoolOr::ExprBoolOr(ExprBool *a, ExprBool *b) {
	child[0] = b;
	child[1] = a;
}

bool ExprBoolFilter::get(ExprContext const *context) {
	return (filter.mask == 0) ? (context->group == filter.id) : ((context->group & filter.mask) == filter.id);
}

void ExprBoolFilter::show(ExprContext const *context) {
	if (filter.mask == 0)
		std::cout << "(" << context->group << " == " << filter.id << ")";
	else
		std::cout << "((" << context->group << " & " << filter.mask << ") == " << filter.id << ")";
}

ExprBoolFilter::ExprBoolFilter(Groups::Filter const _filter) : filter(_filter) {
}

bool ExprBoolPrefix::get(ExprContext const *context) {
	return (context->id != NULL and (strncmp(context->id, str.c_str(), str.size()) == 0));
}

void ExprBoolPrefix::show(ExprContext const *context) {
	std::cout << context->id << "^" << str;
}

ExprBoolPrefix::ExprBoolPrefix(std::string const _str) : str(_str) {
}

bool ExprBoolSuffix::get(ExprContext const *context) {
	if (!context->id)
		return false;

	size_t len = strlen(context->id);
	size_t suffix_len = str.size();
	if (len < suffix_len)
		return false;

	return (strncmp(context->id + len - suffix_len, str.c_str(), suffix_len) == 0);
}

void ExprBoolSuffix::show(ExprContext const *context) {
	std::cout << context->id << "$" << str;
}

ExprBoolSuffix::ExprBoolSuffix(std::string const _str) : str(_str) {
}

//------------------ BOOL EXPR CONTEXT ----------------------//
void ExprVars::set(std::string const str, int *value) {
	list.insert(std::pair<std::string, int*> (str, value));
}

ExprVar *ExprVars::get(std::string const str) const {
	List::const_iterator it = list.find(str);
	if (it != list.end())
		return it->second;
	return NULL;
}

//------------------ BOOL EXPR PARSER ----------------------//
void ExprParser::error(std::string const msg) {
	error_msg = msg;
}

std::string ExprParser::get_error() {
	return error_msg;
}

void ExprParser::push(ExprBool* const obj) {
	stack.push_back(obj);
}

ExprBool* ExprParser::pop() {
	ExprBool* result = stack.back();
	stack.pop_back();
	return result;
}


bool ExprParser::parse_filter(Groups::Filter &filter) {
	if (context->groups == NULL) {
		return false;
	}
	Groups::Group group;

	while ((words->next()) != "]") {
		if (words->current == "") {
			return false;
		}
		group = context->groups->get(words->current);
		if (group.id == GROUP_UNKNOWN) {
			error("Unknown group: " + words->current);
			return false;
		}
		filter.mask |= group.mask;
		filter.id |= group.id;
	}
	return true;
}

bool ExprParser::parse_element() {
	if (words->next() == "") {
		return false;
	}

	if (words->current == ")") {
		return false;
	}
	else if (words->current == "(") {
		return (parse_expr() and words->current == ")");
	}
	else if (words->current == "[") {
		Groups::Filter filter = {0, 0};
		if (parse_filter(filter)) {
			ExprBoolFilter *obj = new ExprBoolFilter(filter);
			push(obj);
		}
		else
			return false;
	}
	else if (words->current.length() > 1 and words->current.at(0) == '^') {
		ExprBoolPrefix *obj = new ExprBoolPrefix(words->current.substr(1));
		push(obj);
	}
	else if (words->current.length() > 1 and words->current.at(words->current.length() - 1) == '$') {
		ExprBoolSuffix *obj = new ExprBoolSuffix(words->current.substr(0, words->current.length() - 1));
		push(obj);
	}
	else if (words->current == "true") {
		ExprBoolConst *obj = new ExprBoolConst(true);
		push(obj);
	}
	else if (words->current == "false") {
		ExprBoolConst *obj = new ExprBoolConst(false);
		push(obj);
	}
	else if (words->current == "!" or words->current == "not") {
		if (parse_element()) {
			ExprBoolNot *obj = new ExprBoolNot(pop());
			push(obj);
		}
		else
			return false;
	}
	else if (words->current == "and" or words->current == "&&") {
		if (stack.size() > 0 and parse_element()) {
			ExprBoolAnd *obj = new ExprBoolAnd(pop(), pop());
			push(obj);
		}
		else
			return false;
	}
	else if (words->current == "or" or words->current == "||") {
		if (stack.size() > 0 and parse_element()) {
			ExprBoolOr *obj = new ExprBoolOr(pop(), pop());
			push(obj);
		}
		else
			return false;
	}
/*
	ExprVar *var;
	else if ((var = context->vars.get(words->current)) != NULL) {
		ExprBoolVar *obj = new ExprBoolVar(var);
		push(obj);
	}
*/
	else {
		error("Unexpected: " + words->current);
		return false;
	}
	return true;
}

bool ExprParser::parse_expr() {
	while (true) {
		if (!parse_element()) {
			return (words->current == ")" or words->current == "");
		}
	}
	return true;
}

ExprBool *ExprParser::parse(std::string const str) {
	std::stringstream buffer;
	buffer << str;

	WordsParser words_parser(&buffer);
	words = &words_parser;
	if (parse_expr() and stack.size() == 1) {
		return pop();
	}
	return NULL;
}

ExprBool *ExprParser::parse(WordsParser *words_parser) {
	words = words_parser;
	if (parse_element() and stack.size() == 1) {
		return pop();
	}
	return NULL;
}

ExprParser::~ExprParser() {
	for (Stack::iterator it = stack.begin(); it != stack.end(); it++) {
		delete *it;
	}
}

ExprParser::ExprParser(ExprContext const *_context) : context(_context) {
}
