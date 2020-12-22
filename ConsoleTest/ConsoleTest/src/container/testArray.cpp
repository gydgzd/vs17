#include <array>
#include <iostream>
#include <time.h>
using namespace std;

void testArray()
{
	std::array<int,100> ai;
	clock_t start, end;
    // visit by at()
	start = clock();
	for (int i = 0; i < 100; i++)
		ai.at(i) = i;
	for (int i = 0; i < 100; i++)
		cout<< ai.at(i) <<"  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // visit by [] (result shows that [] can be faster than at())
	start = clock();
	for (int i = 0; i < 100; i++)
		ai[i] = i;
	for (int i = 0; i < 100; i++)
		cout << ai[i] << "  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

}