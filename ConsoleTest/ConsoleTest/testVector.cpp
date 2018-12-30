#include "stdafx.h"
#include <vector>
#include <iostream>
#include <time.h>
using namespace std;

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
	// reserve 影响存储空间capacity, resize影响实际元素
	vi.reserve(8);     
	vi.resize(10);
	vi.resize(10, 3);
	vi.resize(8);
	vi.resize(10, 3);
	std::vector<int>::reverse_iterator it;     // auto it;
	for (it = vi.rbegin(); it < vi.rend(); it++)
		cout << *it << endl;
	// 测试时间
	vi.reserve(10);
	clock_t start, end;
	start = clock();
	vector<int> vb;
	for (int i = 0; i < 16; i++)
	{
		cout << "Capacity: " << vb.capacity() << " Size: " << vb.size() << endl;
		vb.push_back(i);
	}
	cout << "sizeof(vb) = " << sizeof(vector<int>) << endl;
//	for (int i = 0; i < 100; i++)
//		cout << vi.at(i) << "  ";      // std::out_of_range
//		cout << vi[i] << endl;         // vector subscript out of range
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

	start = clock();
	for (int i = 0; i < 100; i++)
		vi.push_back(i);

	for (int i = 0; i < 100; i++)
		cout << vi[i] << "  ";
	cout << endl;
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
}