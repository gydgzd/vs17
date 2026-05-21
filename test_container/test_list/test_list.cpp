#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <list>
#include <forward_list>
#include "gtest/gtest.h"
using namespace std;

class LIST_TEST : public testing::Test {
protected:
    LIST_TEST() {
        list1.insert(list1.begin(), 0);
        list1.insert(list1.begin(), 1);
        list1.insert(list1.begin(), 2);
        list2.insert_after(list2.before_begin(), 0);
        list2.insert_after(list2.before_begin(), 1);
        list2.insert_after(list2.begin(), 2);
    }

    std::list<int> list1;
    std::forward_list<int> list2;
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

    list1.insert(list1.begin(), 1);

    // push
    list1.push_back(2);
    list1.push_front(0);

    // emplace
    list1.emplace_back(3);
    list1.emplace_front(-1);
    list1.emplace(list1.begin(), -2);

    EXPECT_EQ(list1.size(), 6);
    
    std::list<int>::iterator iter1 = list1.begin();
    int start = -2;
    for (iter1; iter1 != list1.end(); iter1++) {
        EXPECT_EQ(*iter1, start++);
    }
}

TEST_F(LIST_TEST, test_remove) {
    EXPECT_EQ(list1.size(), 3);
    list1.remove(3);
    // The element 3 is not in the list, so size should not change.
	EXPECT_EQ(list1.size(), 3);  

    list1.remove(2);
    EXPECT_EQ(list1.size(), 2);

    list1.pop_back();
    EXPECT_EQ(list1.size(), 1);

    list1.pop_front();
    EXPECT_EQ(list1.size(), 0);

	// tranverse and remove
    std::list<int> list_int{ 1, 2, 3, 4, 5 };
    for (auto iter = list_int.begin(); iter != list_int.end(); ) {
        if (*iter == 2) {
            list_int.erase(iter++);
        }
        else
            iter++;
    }
    EXPECT_EQ(list_int.size(), 4);
}

TEST_F(LIST_TEST, test_rbegin) {
    auto iter = list1.begin();
    EXPECT_EQ(*iter, 2);

    auto riter = list1.rbegin();
    EXPECT_EQ(*riter, 0);

    list1.sort();
    EXPECT_EQ(list1.front(), 0);
    EXPECT_EQ(list1.back(), 2);
}

TEST_F(LIST_TEST, test_forward_list) {
    auto iter1 = list2.begin();
    EXPECT_EQ(*iter1, 1);

    auto iter2 = list2.before_begin();
    EXPECT_NE(iter2, iter1);

    list2.sort();
    EXPECT_EQ(list2.front(), 1);

    std::forward_list<int> list3 = {2, 4, 6};
    list2.merge(list3);
    // There is no size() for forward_list.
}


