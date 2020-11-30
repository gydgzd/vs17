/*
 * testReturn.cpp
 *
 *  Created on: Nov 6, 2018
 *      Author: gyd
 */
struct stu
{
	char name[32];
	int  age;
	void init()
	{
		this->age = 0;
	}
};

#include <stdlib.h>
#include <string.h>
char * strReturn(void)
{
	//warning: address of local variable ‘str’ returned [-Wreturn-local-addr]
	char str[32] = "This is a test of return.";

	return str;
}

