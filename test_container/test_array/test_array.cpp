#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <array>
#include "gtest/gtest.h"
using namespace std;

TEST(ARRAY_TEST, test_initialize) {
    // initialize
    std::array<int, 100> arr_int{ 1, 2, 3, 4, 5 };
    EXPECT_EQ(arr_int.size(), 100);

    // visit by at()
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(arr_int.at(i), i+1);
    for (int i = 5; i < 100; i++)
        EXPECT_EQ(arr_int.at(i), 0);
}

TEST(ARRAY_TEST, test_visit) {
    // initialize
    std::array<int, 1000> arr_int{ 1, 2, 3, 4, 5 };
    clock_t start, end;

    // visit by at()
    start = clock();
    for (int i = 0; i < 1000; i++) {
        arr_int.at(i) = i;
    }
    end = clock();
    cout << "visit by at() cost " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // arr_int.at(1001) = 0;   //  "invalid array<T, N> subscript"

    // visit by [] (result shows that [] can be faster than at())
    start = clock();
    for (int i = 0; i < 1000; i++) {
        arr_int[i] = i;
    }
        
    end = clock();
    cout << "visit by [] cost " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // arr_int[1001] = 0;      // array subscript out of range

    // visit by iterator
    int i = 0;
    start = clock();
    for (auto iter = arr_int.begin(); iter != arr_int.end(); iter++) {
        *iter = i++;
    }
       
    end = clock();
    cout << "visit by iterator cost " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
}

TEST(ARRAY_TEST, test_modify) {
    // initialize
    std::array<int, 1000> arr_int{ 1, 2, 3, 4, 5 };

    // assign(): change every element to one value
    arr_int.assign(12);
    for (int i = 0; i < arr_int.size(); i++) {
        EXPECT_EQ(arr_int[i], 12);
    }
    int* p = arr_int.data();
    *p = 5;
    EXPECT_EQ(arr_int[0], 5);

}

TEST(ARRAY_TEST, test_pair_array) {
    //
    std::array<std::pair<int, int>, 4> arr_pair{ std::make_pair(1, 0), std::make_pair(-1, 0), std::make_pair(0, 1), std::make_pair(0, -1) };
    // assign(): change every element to one value
    arr_pair.assign(std::make_pair(2, 2));

    // data() : return the pointer of the first element
    std::pair<int, int>* px = arr_pair.data();
    cout << "visit by data:" << px->first << endl;
    cout << "array front:" << arr_pair.front().first << endl;

}

