
#include <iostream>
#include "skeleton.hh"

int main(int argc, char *argv[]) {
	Server server;
	if (server.open((argc == 3) ? argv[1] : "0.0.0.0", (argc == 3) ? argv[2] : "4242"))
		server.go();
}
