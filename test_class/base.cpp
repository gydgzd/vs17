#include "base.h"
#include <iostream>

class Base;
class Base1;
class Base2;

/* Base */
int Base::get() {
	std::cout << "Base::get()" << std::endl;
	return memb;
}

void Base::set(int n) {
	std::cout << "Base::set()" << std::endl;
	memb = n;
}

/* Base1 */
int Base1::get() {
	std::cout << "Base1::get()" << std::endl;
	return b1;
}

void Base1::set(int n) {
	std::cout << "Base1::set()" << std::endl;
	b1 = n;
}

/* Base2 */
int Base2::get() {
	std::cout << "Base2::get()" << std::endl;
	return b2;
}

void Base2::set(int n) {
	std::cout << "Base2::set()" << std::endl;
	b2 = n;
}

int Base2::print() {
	std::cout << "Base2::print()" << std::endl;
	return 0
		;
}