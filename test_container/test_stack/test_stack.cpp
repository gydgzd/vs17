#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <stack>
#include "gtest/gtest.h"
using namespace std;

class STACK_TEST : public testing::Test {
protected:
    STACK_TEST() {
        m_stack.push(2);
        m_stack.push(1);
        m_stack.push(3);
    }

    std::stack<int> m_stack;
};

TEST_F(STACK_TEST, test_initialize) {
    EXPECT_EQ(m_stack.size(), 3);

    // stack has no iterator
    int a = m_stack.top();
    EXPECT_EQ(a, 3);
}

TEST_F(STACK_TEST, test_add) {

    m_stack.push(13);
    EXPECT_EQ(m_stack.size(), 4);

    m_stack.emplace(5);
    EXPECT_EQ(m_stack.size(), 5);

    m_stack.pop();
    EXPECT_EQ(m_stack.top(), 13);
    EXPECT_EQ(m_stack.size(), 4);

    m_stack.pop();
    EXPECT_EQ(m_stack.top(), 3);
    EXPECT_EQ(m_stack.size(), 3);
}

TEST_F(STACK_TEST, test_modify) {
    // stack elements cannot be modified directly, need to pop and re-push
    if (m_stack.top() == 3) {
        m_stack.pop();
        m_stack.push(10); // example modification
    }
    EXPECT_EQ(m_stack.top(), 10);
}
