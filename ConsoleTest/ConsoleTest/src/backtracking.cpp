#include "stdafx.h"
#include <vector>
#include <iostream>
using namespace std;


// get subset of vector nums
// method 1: bit map
void subset()
{
    vector<int> nums{ 3,5,12,31,44,11 };
    int n = 6;
    for (int mask = 0; mask < (1 << n); ++mask)
    {
        for (int i = 0; i < n; ++i)
        {
            if (mask & (1 << i))
            {
                cout << nums[i] << " ";
            }
        }
        cout << endl;
    }
    

}
// method 2: backtracing
vector<int> t;
vector<vector<int>> ans;
void backtracing(int cur, vector<int>& nums)
{
    if (cur == nums.size())
    {
        ans.push_back(t);
        return;
    }
    t.push_back(nums[cur]);
    backtracing(cur + 1, nums);
    t.pop_back();
    backtracing(cur + 1, nums);
}