#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <set>
#include "gtest/gtest.h"
using namespace std;

class SET_TEST : public testing::Test {
protected:
    SET_TEST() {
        m_set.insert(2);
        m_set.insert(1);
        m_set.insert(3);

        m_mset.insert(2);
        m_mset.insert(1);
        m_mset.insert(3);
    }
    // set is ordered
    std::set<int> m_set;
	std::multiset<int> m_mset;
};

TEST_F(SET_TEST, test_initialize) {
    EXPECT_EQ(m_set.size(), 3);
    EXPECT_EQ(m_mset.size(), 3);

    // visit by iterator, set is ordered
    std::set<int>::iterator iter = m_set.begin();
    EXPECT_EQ(*iter, 1);
    std::multiset<int>::iterator iter_m = m_mset.begin();
    EXPECT_EQ(*iter_m, 1);
}

TEST_F(SET_TEST, test_add) {
	// set doesn't allow duplicate key
    m_set.insert(13);
    EXPECT_EQ(m_set.size(), 4);

    m_set.insert(13);      // not working, because key 13 already exists
    EXPECT_EQ(m_set.size(), 4);

    m_set.emplace(5);
    EXPECT_EQ(m_set.size(), 5);

    m_set.emplace(3);         // not working, because key 3 already exists
    EXPECT_EQ(m_set.find(3) != m_set.end(), true);
    EXPECT_EQ(m_set.size(), 5);

    m_set.erase(5);
    EXPECT_EQ(m_set.find(5), m_set.end());
    EXPECT_EQ(m_set.size(), 4);

	// multiset allows duplicate key
    m_mset.insert(13);
    EXPECT_EQ(m_mset.size(), 4);

    m_mset.insert(13); 
    EXPECT_EQ(m_mset.size(), 5);
}

TEST_F(SET_TEST, test_modify) {
    // set elements cannot be modified directly, need to remove and re-insert
    if (m_set.find(2) != m_set.end()) {
        m_set.erase(2);
        m_set.insert(10); // example modification
    }
    EXPECT_EQ(m_set.find(10) != m_set.end(), true);

    // visit by iterator
    std::set<int>::iterator iter = m_set.begin();
    for (iter = m_set.begin(); iter != m_set.end(); ) {
        if (*iter == 1) {
            iter = m_set.erase(iter);
        }
        else {
            ++iter;
        }
    }
    EXPECT_EQ(m_set.find(1), m_set.end());

    // m_mset
	m_mset.insert(2);
    EXPECT_EQ(m_mset.size(), 4);
	auto iter_m = m_mset.find(2); 
	int count = m_mset.count(2);
    for (int i = 0; i < count; i++) {
        iter_m = m_mset.erase(iter_m);
	}
    EXPECT_EQ(m_mset.size(), 2);
	// equal_range returns a pair of iterators
    m_mset.insert(3);
    auto range = m_mset.equal_range(3);
    while (range.first != range.second) {
        range.first = m_mset.erase(range.first);
	}
	EXPECT_EQ(range.first, range.second); // no more 3 in the multiset
}

TEST_F(SET_TEST, test_bound) {
    // lower_bound: returns an iterator to the first element not less than the given key
    auto iter1 = m_set.lower_bound(1);
    EXPECT_EQ(*iter1, 1);

    // upper_bound: returns an iterator to the first element greater than the given key
    auto iter2 = m_set.upper_bound(1);
    EXPECT_EQ(*iter2, 2);
}