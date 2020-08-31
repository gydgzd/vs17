#include "stdafx.h"
# include <stack>
using namespace std;

// get a queue by two stacks
class myqueue
{
public:
	void push(int node) {
		while (!stack2.empty())
		{
			stack1.push(stack2.top());
			stack2.pop();
		}
		stack1.push(node);
		
		while (!stack1.empty())
		{
			stack2.push(stack1.top());
			stack1.pop();
		}

	}

	int pop() {
		int n;
		if (stack2.size() != 0)
		{
			n = stack2.top();
			stack2.pop();
		}
		return n;
	}

private:
	stack<int> stack1;
	stack<int> stack2;
};