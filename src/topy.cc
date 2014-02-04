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

#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sys/resource.h>
#include <csignal>

#include "config.h"
#include "config_file.hh"
#include "stringutils.hh"

#include "fields.hh"
#include "server.hh"
#include "log.hh"
#include "autodump.hh"
#include "replicator.hh"

typedef struct {
	std::string address;
	std::string port;
	std::string udp_address;
	std::string udp_port;
	std::string slave_address;
	std::string slave_port;
	std::string restore;
	std::string pidfile;
	std::string config_file;
	std::string autodump_target;
	int autodump_delay;
	bool restore_autodump;
	bool verbose;
} Args;

void help() {
	std::cout << "Usage:" << std::endl;
	std::cout << "  topy [OPTION...]" << std::endl;
	std::cout << std::endl;
	std::cout << "Topy Options:" << std::endl;
	std::cout << "  -a, --address          IP adress of tcp server" << std::endl;
	std::cout << "  -p, --port             Port of tcp server" << std::endl;
	std::cout << "  -A, --udp-address      IP adress of udp server" << std::endl;
	std::cout << "  -P, --udp-port         Port of udp server" << std::endl;
	std::cout << "  -z, --slave-address    IP adress of slave server" << std::endl;
	std::cout << "  -Z, --slave-port       Port of slave server" << std::endl;
	std::cout << "  -r, --restore          Restore memory from dump file" << std::endl;
	std::cout << "  -R, --restore-autodump Restore memory from autodump file" << std::endl;
	std::cout << "  -T, --autodump-target  Set path for autodump target" << std::endl;
	std::cout << "  -D, --autodump-delay   Set pause after each autodump" << std::endl;
	std::cout << "  -i, --pidfile          Write the pid of the process to the given file" << std::endl;
	std::cout << "  -v, --verbose          Turn on verbose output" << std::endl;
	std::cout << "  -c, --conf             Set path of configuration file" << std::endl;
	std::cout << "  -h, --help             Show help options and exit" << std::endl;
	std::cout << "  -V, --version          Show the version number and exit" << std::endl;
}

void version() {
	std::cout << "Topy " VERSION << std::endl;
}

void parse_args(int argc, char **argv, Args &args) {
	struct option long_options[] = {
		{ "conf", 1, NULL, 'c' },
		{ "address", 1, NULL, 'a' },
		{ "port", 1, NULL, 'p' },
		{ "udp-port", 1, NULL, 'P' },
		{ "udp-address", 1, NULL, 'A' },
		{ "slave-address", 1, NULL, 'z' },
		{ "slave-port", 1, NULL, 'Z' },
		{ "restore", 1, NULL, 'r' },
		{ "restore-autodump", 0, NULL, 'R' },
		{ "autodump-target", 1, NULL, 'T' },
		{ "autodump-delay", 1, NULL, 'D' },
		{ "pidfile", 1, NULL, 'i' },
		{ "verbose", 0, NULL, 'v' },
		{ "help", 0, NULL, 'h' },
		{ "version", 0, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	args.verbose = false;
	args.restore_autodump = false;
	args.autodump_delay = 0;

	int option_index, c;
	while ((c = getopt_long(argc, argv, "a:p:A:P:z:Z:c:r:RT:D:i:vhV", long_options, &option_index)) != -1) {
		switch (c) {
			case 'a':
				args.address = optarg;
				break;
			case 'p':
				args.port = optarg;
				break;
			case 'A':
				args.udp_address = optarg;
				break;
			case 'P':
				args.udp_port = optarg;
				break;
			case 'z':
				args.slave_address = optarg;
				break;
			case 'Z':
				args.slave_port = optarg;
				break;
			case 'c':
				args.config_file = optarg;
				break;
			case 'r':
				args.restore = optarg;
				break;
			case 'R':
				args.restore_autodump = true;
				break;
			case 'T':
				args.autodump_target = optarg;
				break;
			case 'D':
				args.autodump_delay = StringUtils::to_uint(optarg);
				break;
			case 'i':
				args.pidfile = optarg;
				break;
			case 'v':
				args.verbose = true;
				break;
			case 'h':
				help();
				exit(0);
				break;
			case 'V':
				version();
				exit(0);
				break;
		}
        }
}

void check_ulimit() {
	rlimit rlim;
	getrlimit(RLIMIT_NOFILE, &rlim);
	log.msg(LOG_NOTICE, "Max number of file descriptors: " + StringUtils::to_string((int) rlim.rlim_max), true);
}

void signal_handler(int sig) {
	bool halt = false;
	std::string name;
	switch (sig) {
		case SIGPIPE:
			name = "SIGPIPE";
			break;
		case SIGINT:
			name = "SIGINT";
			halt = true;
			break;
		case SIGKILL:
			name = "SIGKILL";
			halt = true;
			break;
		case SIGTERM:
			name = "SIGTERM";
			halt = true;
			break;
		default:
			name = "unknown";
	}
	log.msg(LOG_NOTICE, "Received signal number: " + StringUtils::to_string(sig) + " (" + name +")", true);
	if (halt) {
		autodump.force();
		exit(0);
	}
}

void signals_handle() {
	struct sigaction sa;
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
}

void write_pid(std::string const path) {
	std::fstream f(path.c_str(), std::ios_base::out);
	if (f.is_open()) {
		f << getpid();
		f.close();
	}
	else
		log.msg(LOG_ERR, "Could not open output file '" + path + "'", true);
}

int main(int argc, char **argv) {
	Args args;
	parse_args(argc, argv, args);

	//Log
	log.open();

	ConfigFile config;
	if (!config.parse((args.config_file != "") ? args.config_file : "/etc/topy.conf"))
		return -1;

	if (!fields.init(config.fields)) {
		log.msg(LOG_ERR, "Error in fields definition", true);
		return -1;
	}

	std::string address = (args.address != "") ? args.address :
		((config.isset("adress")) ? config.get("address") : "0.0.0.0");
	std::string port = (args.port != "") ? args.port :
		((config.isset("port")) ? config.get("port") : "6969");
	std::string udp_address = (args.udp_address != "") ? args.udp_address :
		((config.isset("udp_address")) ? config.get("udp_address") : "");
	std::string udp_port = (args.udp_port != "") ? args.udp_port :
		((config.isset("udp_port")) ? config.get("udp_port") : "");
	std::string slave_address = (args.slave_address != "") ? args.slave_address :
		((config.isset("slave_address")) ? config.get("slave_address") : "");
	std::string slave_port = (args.slave_port != "") ? args.slave_port :
		((config.isset("slave_port")) ? config.get("slave_port") : "");
	std::string pidfile = (args.pidfile != "") ? args.pidfile :
		((config.isset("pidfile")) ? config.get("pidfile") : "");
	std::string autodump_target = (args.autodump_target != "") ? args.autodump_target :
		((config.isset("autodump_target")) ? config.get("autodump_target") : "");
	int autodump_delay = (args.autodump_delay != 0) ? args.autodump_delay :
		((config.isset("autodump_delay")) ? config.get_int("autodump_delay") : 3600);
	log.verbose = args.verbose;

	//Save pid
	if (pidfile != "")
		write_pid(pidfile);

	//Restore data
	std::string restore = (args.restore_autodump) ? autodump_target : args.restore;
	if (restore != "") {
		log.msg(LOG_NOTICE, "Start restoring data", true);
		bool result = server.restore(restore);
		if (!result) {
			log.msg(LOG_ERR, "Restore has failed", true);
			return -1;
		}
	}

	signals_handle();

	//Start replicator
	if (slave_address != "" and slave_port != "") {
		replicator.open(slave_address, slave_port);
	}

	//Start Autodump
	autodump.data.set(autodump_target != "", autodump_target, autodump_delay != 0 ? autodump_delay : 3600);
	autodump.run();

	//Init libevent
	event_init();

	//Start udp server
	UdpServer udp_server;
	if (udp_address != "" and udp_port != "" and udp_server.open(udp_address, udp_port)) {
		udp_server.listen();
	}

	//Start tcp server
	check_ulimit();
	if (server.open(address, port)) {
		server.listen();
	}

	//Start libevent main loop
	event_dispatch();

	log.msg(LOG_NOTICE, "Topy was stopped");
	log.close();
}
