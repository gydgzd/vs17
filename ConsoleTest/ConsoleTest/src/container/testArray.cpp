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
    array<std::pair<int, int>, 4> dir{ std::make_pair(1, 0), std::make_pair(-1, 0), std::make_pair(0, 1), std::make_pair(0, -1) };
    array<std::pair<int, int>, 4> dir{ {{1, 0}, {-1, 0}, {0, 1}, {0, -1}} };
}