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

#include "config.h"
#include "threads.hh"

#include "server.hh"
#include "timer.hh"

void DumpThread::main() {
	ClientResult result(client);
	if (!server.dump(target))
		result.error();
	result.send();
}

DumpThread::DumpThread(Client *_client) : ClientThread(_client) {
}

void TopThread::main() {
	ClientResult result(client);
	timer.lock();
	from->top(result.data, filter, join, field_id, size, type, rule, inversed);
	timer.unlock();
	result.type = type;
	result.send();
}

TopThread::TopThread(Client *_client) : ClientThread(_client) {
	inversed = false;
}

void ReportThread::main() {
	ClientResult result(client);
	result.type = type;
	timer.lock();
	if (!from->report(result.data, filter, field_id, type))
		result.error("Could not generate report on such a field.");
	timer.unlock();
	result.send();
}

ReportThread::ReportThread(Client *_client) : ClientThread(_client) {
}

void ClearThread::main() {
	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	from->clear(filter, field_id);
	result.send();
}

ClearThread::ClearThread(Client *_client) : ClientThread(_client) {
}

void CountActiveThread::main() {
	timer.lock();
	int total;
	int active_users = from->count_active(filter, field_id, limit, total);
	timer.unlock();

	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	result.data << "a:2:{";
	result.data << "s:6:\"active\";i:" << active_users << ";";
	result.data << "s:5:\"total\";i:" << total << ";";
	result.data << "}";
	result.send();
}

CountActiveThread::CountActiveThread(Client *_client) : ClientThread(_client) {
}

void CleanupThread::main() {
	timer.lock();
	int total;
	int deleted = from->cleanup(filter, field_id, limit, total);
	timer.unlock();

	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	result.data << "a:2:{";
	result.data << "s:7:\"deleted\";i:" << deleted << ";";
	result.data << "s:5:\"total\";i:" << total << ";";
	result.data << "}";
	result.send();
}

CleanupThread::CleanupThread(Client *_client) : ClientThread(_client) {
}

void GroupCountThread::main() {
	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	result.data << "i:";
	result.data << from->group_count(filter);
	result.data << ";";
	result.send();
}

GroupCountThread::GroupCountThread(Client *_client) : ClientThread(_client) {
}

void GroupsStatsThread::main() {
	ClientResult result(client);
	users.groups_stats(result.data);
	result.send();
}

GroupsStatsThread::GroupsStatsThread(Client *_client) : ClientThread(_client) {
}

void GroupsClearThread::main() {
	ClientResult result(client);
	if (!users.groups_clear())
		result.error();
	result.send();
}

GroupsClearThread::GroupsClearThread(Client *_client) : ClientThread(_client) {
}

void GroupDelThread::main() {
	ClientResult result(client);
	if (!users.group_del(name))
		result.error();
	result.send();
}

GroupDelThread::GroupDelThread(Client *_client) : ClientThread(_client) {
}

void SetsSelectThread::main() {
	ClientResult result(client);
	users.select(filter, *vector);
	result.send();
}

SetsSelectThread::SetsSelectThread(Client *_client) : ClientThread(_client) {
}

void SetsClearThread::main() {
	ClientResult result(client);
	vector->clear();
	result.send();
}

SetsClearThread::SetsClearThread(Client *_client) : ClientThread(_client) {
}

