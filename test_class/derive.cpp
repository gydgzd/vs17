#include "derive.h"
#include <iostream>


/* Derive */
int Derive::get() {
	std::cout << "Derive::get()" << std::endl;
	return d1;
}

void Derive::set(int n) {
	std::cout << "Derive::set()" << std::endl;
	d1 = n;
}

/* Derive1 */
int Derive1::get() {
	std::cout << "Derive1::get()" << std::endl;
	return d1;
}

void Derive1::set(int n) {
	std::cout << "Derive1::set()" << std::endl;
	d1 = n;
}