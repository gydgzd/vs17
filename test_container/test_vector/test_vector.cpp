#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <vector>
#include <algorithm>   // for sort
#include <numeric>     // for accumulate
#include <execution>
#include "gtest/gtest.h"

using namespace std;

TEST(VECTOR_TEST, test_initialize) {
    // initialize
    std::vector<int> vec_int{ 1, 2, 3, 4, 5 };
    EXPECT_EQ(vec_int.size(), 5);

    // visit by at()
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(vec_int.at(i), i + 1);
}

TEST(VECTOR_TEST, test_visit) {
    // initialize
    std::vector<int> vec_int{ 1, 2, 3, 4, 5 };
    clock_t start, end;

    // visit by at()
    start = clock();
    for (int i = 0; i < vec_int.size(); i++) {
        vec_int.at(i) = i;
    }
    end = clock();
    cout << "visit by at() cost " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // vec_int.at(1001) = 0;   //  "invalid vector<T, N> subscript"

    // visit by [] (result shows that [] can be faster than at())
    start = clock();
    for (int i = 0; i < vec_int.size(); i++) {
        vec_int[i] = i;
    }

    end = clock();
    cout << "visit by [] cost " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
    // vec_int[1001] = 0;      // vector subscript out of range

    // visit by iterator
    int i = 0;
    start = clock();
    for (auto iter = vec_int.begin(); iter != vec_int.end(); iter++) {
        *iter = i++;
    }

    end = clock();
    cout << "visit by iterator cost " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
}

TEST(VECTOR_TEST, test_insert) {
    // initialize
    std::vector<int> vec_int{ 1, 2, 3};
    EXPECT_EQ(vec_int.size(), 3);
    EXPECT_EQ(vec_int.capacity(), 3);

	vec_int.insert(vec_int.end(), 4);
	vec_int.push_back(5);
	vec_int.emplace_back(6);
    EXPECT_EQ(vec_int.size(), 6);
    EXPECT_EQ(vec_int.capacity(), 6);

	vec_int.reserve(10);
    EXPECT_EQ(vec_int.capacity(), 10);
}

TEST(VECTOR_TEST, test_remove) {
    // initialize
    std::vector<int> vec_int{ 1, 2, 3, 4 };
    EXPECT_EQ(vec_int.size(), 4);
    EXPECT_EQ(vec_int.capacity(), 4);

    vec_int.erase(vec_int.begin());
	EXPECT_EQ(vec_int.size(), 3);
    EXPECT_EQ(vec_int.capacity(), 4);

    // [first, last)
    vec_int.erase(vec_int.begin(), vec_int.begin()+2);
    EXPECT_EQ(vec_int.size(), 1);
    EXPECT_EQ(vec_int.capacity(), 4);
}

TEST(VECTOR_TEST, test_modify) {
    // initialize
    std::vector<int> vec_int{ 1, 2, 3, 4, 5 };
    std::fill(vec_int.begin(), vec_int.end(), 12);
    EXPECT_EQ(vec_int[0], 12);

    for (int i = 0; i < vec_int.size(); i++) {
        EXPECT_EQ(vec_int[i], 12);
    }
    int* p = vec_int.data();
    *p = 5;
    EXPECT_EQ(vec_int[0], 5);
}

TEST(VECTOR_TEST, test_size) {
    // initialize
    std::vector<int> vec_int;
    EXPECT_EQ(vec_int.size(), 0);
    EXPECT_EQ(vec_int.empty(), true);
}

TEST(VECTOR_TEST, test_pair_vector) {
    //
    std::vector<std::pair<int, int>> arr_pair{ std::make_pair(1, 0), std::make_pair(-1, 0), std::make_pair(0, 1), std::make_pair(0, -1) };
    // assign(): change every element to one value
	std::fill(arr_pair.begin(), arr_pair.end(), std::make_pair(2, 2));

    // data() : return the pointer of the first element
    std::pair<int, int>* px = arr_pair.data();
    cout << "visit by data:" << px->first << endl;

    cout << "vector front:" << arr_pair.front().first << endl;
}

TEST(VECTOR_TEST, test_function) {
    //
    std::vector<int> vec_int{ 1, 4, 2, 5 };
    cout << "before sort:" << vec_int[0] << " " << vec_int[1] << " " << vec_int[2] << " " << vec_int[3] << endl;

    // sort   std::execution is supported after C++17,
#if (defined(__cplusplus) && __cplusplus >= 201703L) 
    || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
        std::sort(std::execution::seq, vec_int.begin(), vec_int.end());
#  else
    std::sort(vec_int.begin(), vec_int.end());
#  endif

    cout << "after  sort:" << vec_int[0] << " " << vec_int[1] << " " << vec_int[2] << " " << vec_int[3] << endl;

    EXPECT_EQ(vec_int[0], 1);
    EXPECT_EQ(vec_int[1], 2);
    EXPECT_EQ(vec_int[2], 4);
    EXPECT_EQ(vec_int[3], 5);

    // find
    std::vector<int> vec_int1{ 1, 4, 2, 5 };
    auto found = find(vec_int1.begin(), vec_int1.end(), 2);
    EXPECT_EQ(*found, 2);

    auto found1 = find(vec_int1.begin(), vec_int1.end(), 6);
    EXPECT_EQ(found1, vec_int1.end());

    // accumulate: calculate the sum of all elements
    int sum = accumulate(vec_int1.begin(), vec_int1.end(), 0);
    EXPECT_EQ(sum, 12);
}
