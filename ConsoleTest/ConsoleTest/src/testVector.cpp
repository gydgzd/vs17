#include "stdafx.h"
#include <vector>
#include <iostream>
#include <time.h>
#include <algorithm>
using namespace std;

/*
vec is sorted ascending
find num in vec, return the index
*/
int binarySearch(vector<int> &vec, int num)
{
    int left = 0;
    int right = vec.size() - 1;
    int mid = 0;
    if (vec[0] >= num)
        return 0;
    if (vec[right] <= num)
        return right;
    while (left <= right)
    {
        mid = (left + right) / 2;
        if (vec[mid] == num)
            return mid;
        else if (vec[mid] < num)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}
/*
vec is sorted ascending
find the count of elements in vec, that smaller then num
*/
int binaryCompare(vector<int> &vec, int num)
{
    int left = 0;
    int right = vec.size() - 1;
    int mid = 0;
    if (vec[0] >= num)
        return 0;
    if (vec[right] <= num)
        return right;
    while (left <= right)
    {
        mid = (left + right) / 2;
        if (vec[mid - 1] <= num && vec[mid] > num)
        {
            return mid;
        }
        else if (vec[mid] < num)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return 0;
}

void testVector()
{
	int array[6] = {3, 5, 12, 46, 13};
	int nSize = sizeof(array) / sizeof(array[0]);
	std::vector<int> vi(array, array+5);
	vi.emplace(vi.end(), 5);
	vi.emplace(vi.end(), 5);
	vi.insert(vi.end()-1, 22);
	int *p = vi.data();
	*p = 32;
    sort(vi.begin(), vi.end());
    cout << binarySearch(vi, 15) << endl;
    cout << binarySearch(vi, 13) << endl;
    cout << binaryCompare(vi, 13) << endl;
    cout << binaryCompare(vi, 5) << endl;
    cout << binaryCompare(vi, 45) << endl;
	// reserve 影响存储空间capacity, resize影响实际元素
	vi.reserve(8);   
    vi.reserve(12);
	vi.resize(10);
	vi.resize(10, 3);
	vi.resize(8);
	vi.resize(10, 3);
	std::vector<int>::reverse_iterator it;     // auto it;
	for (it = vi.rbegin(); it < vi.rend(); it++)
		cout << *it << endl;
	// 测试时间(比较push_back在空间不够时分配和先分配后使用的时间)
	clock_t start, end;
	start = clock();
	vector<int> vb;
	for (int i = 0; i < 10240; i++)
	{
	//	cout << "Capacity: " << vb.capacity() << " Size: " << vb.size() << endl;
		vb.push_back(i);
	}
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

    start = clock();
    vector<int> va;
    va.reserve(10240);
	for (int i = 0; i < 10240; i++)
        va.push_back(i);

	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
}