#include "stdafx.h"
#include <iostream>
//using namespace std;
void simpleInterativeReverse()
{
	char stack[60];
	register int top = 0;
	std::cin.getline(stack, 60);
	for (top = strlen(stack) - 1; top >= 0; std::cout.put(stack[top --]));

}

void interativeReverse()
{
	char stack[60];
	register int top = 0;
	std::cin.get(stack,top);
	while (stack[top] != '\n' && top < 59)
		std::cin.get(stack[++top]);
	for (top -= 2; top >= 0; std::cout.put(stack[top--]));
}