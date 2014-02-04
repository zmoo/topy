#define _MAIN

#include "user.hh"
#include <iostream>
#include <stdlib.h>


class Class {
	char buf[128];
};

int main(int argc, char **argv) {

	#define SIZE 100
	User *users[SIZE];

	for (int i = 0; i < (SIZE); i++) {
		users[i] = new User();
	}


	for (int i = 0; i < SIZE; i++)
		delete users[i];

	char c;
	std::cin >> c;

/*
	//CHART

	User user;
	user.visits.update();
	user.visits.inc(4);
	user.visits.translate(3, 0);
	user.visits.inc(1);
	user.visits.debug();
	std::cout << user.visits.serialize_php() << std::endl;

	Canvas canvas;
	int nsamples = user.visits.days->get_count();

	canvas.vline(0, 0, nsamples, '|');
	for (int i  = 0; i < nsamples; i++) {
		int value = user.visits.days->get_sample(i);
		canvas.hline(1, i, value, '=');
		canvas.put(value + 2, i, value);
	}
	std::cout << canvas.export_area(0, 0, 20, nsamples) << std::endl;
*/
}
