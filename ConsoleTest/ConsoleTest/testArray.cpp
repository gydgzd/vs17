#include "stdafx.h"
#include <array>
#include <iostream>
#include <time.h>
using namespace std;

void testArray()
{
	std::array<int,100> ai;
	clock_t start, end;
	start = clock();
	for (int i = 0; i < 100; i++)
		ai.at(i) = i;

	for (int i = 0; i < 100; i++)
		cout<< ai.at(i) <<"  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

	start = clock();
	for (int i = 0; i < 100; i++)
		ai[i] = i;

	for (int i = 0; i < 100; i++)
		cout << ai[i] << "  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
}