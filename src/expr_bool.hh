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

#ifndef _EXPR_BOOL_HH
#define _EXPR_BOOL_HH

#include <string>
#include <list>
#include <map>

#include "words_parser.hh"
#include "groups.hh"

typedef int ExprVar;

class ExprVars {
private:
	typedef std::map<std::string, ExprVar*> List;
	List list;
public:
	void set(std::string const str, int *value);
	ExprVar *get(std::string const str) const;
};

class ExprContext {
public:
	Groups *groups;
	GroupId group;
	char *id;
//	ExprVars vars;
};

class ExprBool {
public:
	std::string get_str(ExprContext const *context);
	virtual bool get(ExprContext const *context) = 0;
	virtual void show(ExprContext const *context) = 0;

	virtual ~ExprBool();
};

class ExprBoolNot : public ExprBool {
private:
	ExprBool *child;
public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolNot(ExprBool *_child);
	~ExprBoolNot();
};

class ExprBoolConst : public ExprBool {
private:
	bool value;
public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolConst(bool _value);
};

class ExprBoolVar : public ExprBool {
private:
	ExprVar *value;
public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolVar(ExprVar *_value);
};

class ExprBoolFilter : public ExprBool {
private:
	Groups::Filter filter;

public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolFilter(Groups::Filter const _filter);
};

class ExprBoolPrefix : public ExprBool {
private:
	std::string str;

public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolPrefix(std::string const str);
};

class ExprBoolSuffix : public ExprBool {
private:
	std::string str;

public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolSuffix(std::string const str);
};

class ExprBoolOp : public ExprBool {
protected:
	ExprBool *child[2];

public:
	~ExprBoolOp();
};

class ExprBoolAnd : public ExprBoolOp {
public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolAnd(ExprBool *a, ExprBool *b);
};

class ExprBoolOr : public ExprBoolOp {
public:
	bool get(ExprContext const *context);
	void show(ExprContext const *context);

	ExprBoolOr(ExprBool *a, ExprBool *b);
};

class ExprParser {
private:
	WordsParser *words;
	typedef std::list<ExprBool*> Stack;
	Stack stack;
	ExprBool* pop();
	std::string error_msg;
	ExprContext const *context;

private:
	void push(ExprBool* const obj);

	bool parse_element();
	bool parse_expr();
	bool parse_filter(Groups::Filter &filter);

	void error(std::string const msg);

public:
	ExprBool *parse(std::string const str);
	ExprBool *parse(WordsParser *words_parser);
	std::string get_error();

	ExprParser(ExprContext const *_context);
	~ExprParser();
};

#endif
