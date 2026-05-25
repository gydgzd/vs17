#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <bitset>
#include "gtest/gtest.h"
using namespace std;

class BITSET_TEST : public testing::Test {
protected:
    BITSET_TEST() : m_bitset(15), m_bitset1("11100000") {

    }
    std::bitset<64> m_bitset;
    std::bitset<8> m_bitset1;
};

TEST_F(BITSET_TEST, test_set) {
    EXPECT_EQ(m_bitset.size(), 64);
    EXPECT_EQ(m_bitset.count(), 4);

    EXPECT_EQ(m_bitset.test(0), true);
	m_bitset.reset(0);
	EXPECT_EQ(m_bitset.test(0), false);

    for (unsigned int i = 0; i < m_bitset.size(); ++i) {
        m_bitset.set(i);
    }
    EXPECT_EQ(m_bitset.count(), 64);
    for (unsigned int i = 0; i < m_bitset.size(); ++i) {
        EXPECT_EQ(m_bitset.test(i), true);
    }

    m_bitset.reset();
    EXPECT_EQ(m_bitset.count(), 0);
	EXPECT_EQ(m_bitset.all(), false); // all: check if all bits are set
	EXPECT_EQ(m_bitset.any(), false); // any: check if any bit is set
    EXPECT_EQ(m_bitset.none(), true); // none: check if no bits are set
}

TEST_F(BITSET_TEST, test_convert) {
	EXPECT_STREQ(m_bitset1.to_string().c_str(),"11100000"); // convert to string
	EXPECT_EQ(m_bitset1.to_ulong(), 224); // convert to unsigned long

    m_bitset1.flip(7);
	EXPECT_STREQ(m_bitset1.to_string().c_str(), "01100000");
    m_bitset1.flip();
    EXPECT_STREQ(m_bitset1.to_string().c_str(), "10011111");
}

TEST_F(BITSET_TEST, test_bitoperation) {
    std::bitset<8> m_bitset1("00001111");
	m_bitset1 = m_bitset1 & std::bitset<8>("11110000");
    EXPECT_STREQ(m_bitset1.to_string().c_str(), "00000000"); // convert to string
    EXPECT_EQ(m_bitset1.to_ulong(), 0); // convert to unsigned long

    m_bitset1 = m_bitset1 | std::bitset<8>("11110000");
    EXPECT_STREQ(m_bitset1.to_string().c_str(), "11110000");
    EXPECT_EQ(m_bitset1.to_ulong(), 240);

	m_bitset1 = m_bitset1 << 4;
    EXPECT_STREQ(m_bitset1.to_string().c_str(), "00000000");
	EXPECT_EQ(m_bitset1.to_ulong(), 0);
}
