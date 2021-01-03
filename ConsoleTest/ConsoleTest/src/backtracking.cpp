#include "stdafx.h"
#include <vector>
#include <iostream>

using namespace std;


// get subset of vector nums
// method 1: bit map
void subset()
{
    vector<int> nums{ 3,5,12,31,44};
    int n = nums.size();
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
// convert a decimal number to binary
long long decimalToBinary(long long dec)
{  
    int bin = 0;   
    int remainder = 0;
    int i = 1;
    while (dec != 0)
    {
        remainder = dec % 2;
        bin += remainder * i;
        i *= 10;
        dec /= 2;
    }
    return bin;
}
// convert a binary number to decimal
long long binaryToDecimal(long long bin)
{
    int dec = 0;
    int remainder = 0;
    int i = 1;
    while (bin != 0)
    {
        remainder = bin % 10;
        dec += remainder * i;
        i *= 2;
        bin /= 10;
    }
    return dec;
}