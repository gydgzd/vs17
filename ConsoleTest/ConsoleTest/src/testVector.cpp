#include "stdafx.h"
#include <vector>
#include <iostream>
#include <time.h>
using namespace std;

int binaryFind(vector<int>& array, int num) // return the count of element in array that less then num
{
    int left = 0;
    int right = array.size();
    int mid = 0;
    while (left != right)
    {
        mid = (left + right) / 2;
        if ((array[mid + 1] > num && array[mid - 1] <= num) || (array[mid + 1] >= num && array[mid - 1] < num))
            break;
        else if (array[mid] < num)
        {
            left = mid;
        }
        else if (array[mid] > num)
        {
            right = mid;
        }
    }
    return mid + 1;
}

void testVector()
{
	int array[6] = {3, 5, 12, 46, 13};
	int nSize = sizeof(array) / sizeof(array[0]);
	std::vector<int> vi(array, array+4);
	vi.emplace(vi.end(), 5);
	vi.emplace(vi.end(), 5);
	vi.insert(vi.end()-1, 22);
	int *p = vi.data();
	*p = 32;
    binaryFind(vi, 15);
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