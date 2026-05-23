#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING  
#include <iostream>
#include <time.h>

#include <queue>  // for queue and priority_queue
#include <deque>
//#include <concurrent_priority_queue.h>
#include "gtest/gtest.h"
using namespace std;

class QUEUE_TEST : public testing::Test {
protected:
    QUEUE_TEST() {
        queue1.push(2);
        queue1.push(1);
        queue1.push(0);
        queue3.push(2);
        queue3.push(1);
        queue3.push(0);
    }

	std::queue<int> queue1;  // can't use initializer list to initialize queue
    std::deque<int> queue2{ 0, 1, 2 };

    struct compare {
        bool operator()(int a, int b) {
            return a > b; // min heap
        }
	}; // std::greater<int> can also be used for min heap

	std::priority_queue<int, vector<int>, compare> queue3;
	//   std::priority_queue<int> queue3;  
	// default is max heap. equal to :
    // std::priority_queue<int, vector<int>, std::less<int>> queue3;
};

//  mixing TEST_F and TEST in the same test suite is illegal
TEST_F(QUEUE_TEST, test_queue) {
    queue1.push(3);
    queue1.push(4);
    EXPECT_EQ(queue1.size(), 5);

    queue1.emplace(5);
    EXPECT_EQ(queue1.size(), 6);
    EXPECT_EQ(queue1.front(), 2);
	// no iterator for queue, so we can only visit by front() and pop()
    cout << "normal queue: ";
    for (int i = 0; i < 6; i++) {
        int value = queue1.front();
        queue1.pop();
		cout << value << " ";
    }
	cout << endl;
}

TEST_F(QUEUE_TEST, test_pqueue) {
    queue3.push(3);
    queue3.push(4);
    EXPECT_EQ(queue3.size(), 5);

    queue3.emplace(5);
    EXPECT_EQ(queue3.size(), 6);
    EXPECT_EQ(queue3.top(), 5);
    // no iterator for queue, so we can only visit by top() and pop()
    cout << "priority queue: ";
    for (int i = 0; i < 6; i++) {
        int value = queue3.top();
        queue3.pop();
        cout << value << " ";
    }
    cout << endl;
}

TEST_F(QUEUE_TEST, test_deque) {
    queue2.push_back(4);
    queue2.push_front(5);
    EXPECT_EQ(queue2.size(), 5);

    queue2.emplace_back(6);
    queue2.emplace_front(7);
    EXPECT_EQ(queue2.size(), 7);
    // has iterator for deque
	auto iter = queue2.begin();
    EXPECT_EQ(*iter, 7);

    queue2.pop_front();
    EXPECT_EQ(queue2.front(), 5);
    
    auto iter2 = queue2.end() - 1;
    EXPECT_EQ(*iter2, 6);
    queue2.pop_back();
    EXPECT_EQ(queue2.back(), 4);
}


