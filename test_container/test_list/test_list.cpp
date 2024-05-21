#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <list>
#include "gtest/gtest.h"
using namespace std;

class LIST_TEST : public testing::Test {
protected:
    LIST_TEST() {
        list1.insert(list1.begin(), 0);
        list1.insert(list1.begin(), 1);
        list1.insert(list1.begin(), 2);
    }

    std::list<int> list1;

};

//  mixing TEST_F and TEST in the same test suite is illegal
TEST_F(LIST_TEST, test_initialize) {
    // initialize
    std::list<int> list_int{1, 2, 3, 4, 5};
    EXPECT_EQ(list_int.size(), 5);

    // visit by iter
    std::list<int>::iterator iter = list_int.begin();
    for (int i = 0; i < 5; i++, iter++) {
        EXPECT_EQ(*iter, i + 1);
    }
}

TEST_F(LIST_TEST, test_add) {
    // insert
    list1.clear();
    EXPECT_EQ(list1.size(), 0);

    list1.insert(list1.begin(), 0);
    list1.insert(list1.begin(), 1);
    list1.insert(list1.begin(), 2);

    // push
    list1.push_back(-1);
    list1.push_back(-2);

    list1.push_front(3);
    list1.push_front(4);

    // emplace
    list1.emplace_back(-3);
    list1.emplace_back(-4);

    list1.emplace_front(5);
    list1.emplace_front(6);

    list1.emplace(list1.begin(), 7);

    EXPECT_EQ(list1.size(), 12);
    
    std::list<int>::iterator iter1 = list1.begin();
    int start = 7;
    for (iter1; iter1 != list1.end(); iter1++) {
        EXPECT_EQ(*iter1, start--);
    }
}

TEST_F(LIST_TEST, test_remove) {
    EXPECT_EQ(list1.size(), 3);
    list1.remove(3);
    EXPECT_EQ(list1.size(), 3);

    list1.remove(2);
    EXPECT_EQ(list1.size(), 2);
    int start = 1;
    for (auto iter1 = list1.begin(); iter1 != list1.end(); iter1++) {
        EXPECT_EQ(*iter1, start--);
    }
}

TEST_F(LIST_TEST, test_rbegin) {
    auto iter = list1.begin();
    EXPECT_EQ(*iter, 2);

    auto riter = list1.rbegin();
    EXPECT_EQ(*riter, 0);

    list1.sort();
    iter = list1.begin();
    for (int i = 0; i < 3; i++, iter++) {
        EXPECT_EQ(*iter, i);
    }
}