#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <map>
#include "gtest/gtest.h"
using namespace std;

class MAP_TEST : public testing::Test {
protected:
    MAP_TEST() {
        m_map.insert(std::pair<int, char>(2, '0'));
        m_map.insert(std::pair<int, char>(1, 'a'));
        m_map.insert(std::pair<int, char>(3, 'b'));
        m_mmap.insert(std::pair<int, char>(2, '0'));
        m_mmap.insert(std::pair<int, char>(1, 'a'));
        m_mmap.insert(std::pair<int, char>(3, 'b'));
        EXPECT_EQ(m_map.size(), 3);
        EXPECT_EQ(m_mmap.size(), 3);
    }
    // map is ordered
    std::map<int, char> m_map;
	std::multimap<int, char> m_mmap;
};

TEST_F(MAP_TEST, test_initialize) {
    // initialize
    std::map<int, char> map_t{ {3,'a'}, {2,'b'}, {4, 'e'}};
    EXPECT_EQ(map_t.size(), 3);

	// visit by iterator, map is ordered by key
    std::map<int, char>::iterator iter = map_t.begin();
    EXPECT_EQ(iter->first, 2);
    EXPECT_EQ(iter->second, 'b');

    // multimap
    std::multimap<int, char> mmap_t{ {3,'a'}, {2,'b'}, {4, 'e'} };
    EXPECT_EQ(mmap_t.size(), 3);
    std::multimap<int, char>::iterator iter1 = mmap_t.begin();
    EXPECT_EQ(iter1->first, 2);
    EXPECT_EQ(iter1->second, 'b');
}

TEST_F(MAP_TEST, test_add) {
    m_map.insert({13, 'z'});
    EXPECT_EQ(m_map.size(), 4);

	m_map.insert({ 13, 'c' });      // not working, because key 13 already exists
    EXPECT_EQ(m_map.size(), 4);

    m_map.emplace(5, ' ');
    EXPECT_EQ(m_map.size(), 5);
	m_map.emplace(3, ' ');         // not working, because key 3 already exists
    EXPECT_EQ(m_map[3], 'b');
    EXPECT_EQ(m_map.size(), 5);

	m_map.erase(5);
    EXPECT_EQ(m_map.find(5), m_map.end());
    EXPECT_EQ(m_map.size(), 4);

	// multimap allows duplicate keys
    m_mmap.insert({ 13, 'z' });
    EXPECT_EQ(m_mmap.size(), 4);
    m_mmap.insert({ 13, 'c' });
	EXPECT_EQ(m_mmap.size(), 5);
    m_mmap.erase(13);
    EXPECT_EQ(m_mmap.size(), 3);
}

TEST_F(MAP_TEST, test_modify) {
    EXPECT_EQ(m_map[2], 'a');
    for (auto& member : m_map) {
        if (member.first == 2) {
            member.second = 'x';
        }
    }
    EXPECT_EQ(m_map[2], 'x');

    // visit by iterator
    std::map<int, char>::iterator iter = m_map.begin();
    for (iter = m_map.begin(); iter != m_map.end(); ) {
        if (iter->first == 1) {
			iter = m_map.erase(iter);
        } else {
            ++iter;
        }
    }
    EXPECT_EQ(m_map.find(1), m_map.end());
}

TEST_F(MAP_TEST, test_bound) {
	// lower_bound: returns an iterator to the first element not less than the given key
	auto iter1 = m_map.lower_bound(1);
    EXPECT_EQ(iter1->first, 1);
    EXPECT_EQ(iter1->second, '0');

	// upper_bound: returns an iterator to the first element greater than the given key
    auto iter2 = m_map.upper_bound(1);
    EXPECT_EQ(iter2->first, 2);
    EXPECT_EQ(iter2->second, 'a');
}