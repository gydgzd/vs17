#pragma once
#include "base.h"


class Derive : public Base {
public:
	int get();
	void set(int n);
private:
	int d1;

};

class Derive1 : public Base1 , public Base2{
public:
	int get();
	void set(int n);
private:
	int d1;

};