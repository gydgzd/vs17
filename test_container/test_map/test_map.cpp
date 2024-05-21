#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <map>
#include "gtest/gtest.h"
using namespace std;

class MAP_TEST : public testing::Test {
protected:
    MAP_TEST() {
        m_map.insert(std::pair<int, char>(1, '0'));
        m_map.insert(std::pair<int, char>(2, 'a'));
        m_map.insert(std::pair<int, char>(3, 'b'));
    }

    std::map<int, char> m_map;

};

TEST_F(MAP_TEST, test_initialize) {
    // initialize
    std::map<int, char> map_t{ {3,'a'}, {2,'b'}, {4, 'e'}};
    EXPECT_EQ(map_t.size(), 3);

    // visit by iter
    std::map<int, char>::iterator iter = map_t.begin();

    // map is ordered
    EXPECT_EQ(iter->first, 2);
    EXPECT_EQ(iter->second, 'b');
}

TEST_F(MAP_TEST, test_add) {
    m_map.insert({ 11, 'a' });
    m_map.insert({ 13, 'z' });
    EXPECT_EQ(m_map.size(), 5);

    m_map.insert({ 13, 'c' });
    EXPECT_EQ(m_map.size(), 5);

    m_map.emplace(5, ' ');
    EXPECT_EQ(m_map[5], ' ');
    EXPECT_EQ(m_map.size(), 6);

    m_map.emplace(3, ' ');
    EXPECT_EQ(m_map[3], 'b');
    EXPECT_EQ(m_map.size(), 6);
}