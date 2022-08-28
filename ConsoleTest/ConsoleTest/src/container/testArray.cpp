#include <iostream>
#include <time.h>
#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <array>
#include "gtest/gtest.h"
using namespace std;

TEST(ARRAY_TEST, test_initialize) {
    // initialize
    std::array<int, 100> arr_int{ 1, 2, 3, 4, 5 };
    clock_t start, end;
    // visit by at()
    start = clock();
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(arr_int.at(i), i+1);
    for (int i = 5; i < 100; i++)
        EXPECT_EQ(arr_int.at(i), 0);
}
void testArray()
{
    // initialize
    std::array<int, 100> arr_int{1, 2, 3, 4, 5};
	clock_t start, end;
    // visit by at()
	start = clock();
	for (int i = 0; i < 100; i++)
        arr_int.at(i) = i;
	for (int i = 0; i < 100; i++)
		cout<< arr_int.at(i) <<"  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // visit by [] (result shows that [] can be faster than at())
	start = clock();
	for (int i = 0; i < 100; i++)
        arr_int[i] = i;
	for (int i = 0; i < 100; i++)
		cout << arr_int[i] << "  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // visit by iterator
    int i = 0;
    start = clock();
    for (auto iter = arr_int.begin() ; iter != arr_int.end(); iter++)
        *iter = i++;
    for (auto iter = arr_int.begin(); iter != arr_int.end(); iter++)
        cout << *iter << "  ";
    cout << endl;
    end = clock();
    cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    //
    std::array<std::pair<int, int>, 4> arr_pair{ std::make_pair(1, 0), std::make_pair(-1, 0), std::make_pair(0, 1), std::make_pair(0, -1) };
    // assign(): change every element to one value
    arr_pair.assign(std::make_pair(2, 2));
    // data() : return the pointer of the first element
    std::pair<int, int> *px = arr_pair.data();
    cout << "visit by data:" << px->first << endl;
    cout << "array front:" << arr_pair.front().first << endl;
}