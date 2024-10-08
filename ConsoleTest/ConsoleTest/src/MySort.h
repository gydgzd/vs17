/*
 * MySort.h
 *
 *  Created on: Nov 23, 2017
 *      Author: gyd
 */

#ifndef SRC_MYSORT_H_
#define SRC_MYSORT_H_
#include "MyTimer.h"
#ifdef WINVER 
#include <windows.h> 
#endif
#include <iostream>
using namespace std;
template<typename T>
class MySort {
public:
	MySort();
	~MySort();

    virtual int quickSort(T a[], int low, int high) { return 0; };

	int insertionSort(T a[], int n);
	int selectSort(T a[], int n);
    int bubbleSort(T a[], int n);

    int shellSort(T a[], int n);
//    int quickSort(T a[], int low, int high);
protected:
	MyTimer m_timer;
    int m_steps;
    long long m_timeCost;
    void swap(T& a, T& b);
};

template<typename T>
MySort<T>::MySort():m_steps(0), m_timeCost(0){
    m_timer.start();

}
template<typename T>
MySort<T>::~MySort() {

}

template<typename T>
void MySort<T>::swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

template<typename T>
int MySort<T>::insertionSort(T a[], int n)
{
	m_timer.start();
    int steps = 0;
	int index_m,index_n;
	T smaller;
    for (index_m = 1; index_m < n; index_m++)
    {
        if (a[index_m] > a[index_m - 1])     // improved
            continue;
        smaller = a[index_m];
        for (index_n = index_m - 1; index_n >= 0 && a[index_n] > smaller; index_n--)
        {
            a[index_n + 1] = a[index_n];
            steps++;
        }
        a[index_n + 1] = smaller;
        steps++;
    }

	cout <<"InsertionSort cost time: "<<m_timer.stop()<< " ms, " << steps << " steps" << endl;
	return 0;
}

template<typename T>
int MySort<T>::selectSort(T a[], int n)
{
    m_timer.start();
    int steps = 0;
    for (int i = 0; i < n; i++)
    {
        int smallest = i;
        for (int j = i; j < n; j++)
        {
            if (a[j] < a[smallest])
                smallest = j;
        }
        swap(a[smallest], a[i]);
        steps += 3;
    }
    cout << "selectSort cost time " << m_timer.stop() << " ms, " << steps << " steps" << endl;
    return 0;
}

template<typename T>
int MySort<T>::bubbleSort(T a[], int n)
{
    m_timer.start();
    int steps = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = n - 1; j > i; j--)
        {
            if (a[j - 1] > a[j])
            {
                swap(a[j - 1], a[j]);
                steps += 3;
            }
        }
    }
    cout << "bubbleSort cost time " << m_timer.stop() << " ms, " << steps << " steps" << endl;
    return 0;
}

template<typename T>
int MySort<T>::shellSort(T a[], int n)
{
    m_timer.start();
    int steps = 0;
    register int index_m, index_n, h;
    int increments[20], k;
    // Create an appropriate number of increments h
    for (h = 1,index_m = 0; h < n && index_m < 20; index_m++)
    {
        increments[index_m] = h;
        h = 3 * h + 1;
    }
    // loop on different h
    for (index_m--; index_m >= 0; index_m--)
    {
        h = increments[index_m];
        for (k = h; k < n; k++)   // insertionSort
        {
            if (a[k - h] < a[k])
                continue;
            T tmp = a[k];
            for (index_n = k - h; index_n >= 0 && a[index_n] > tmp; index_n -= h)
            {
                a[index_n + h] = a[index_n];
                steps++;
            }
            a[index_n + h] = tmp;
            steps++;
        }
    }
    cout << "shellSort cost time: " << m_timer.stop() << " ms, " << steps << " steps" << endl;
    return 0;
}

//
template<typename T>
class QuickSort : public MySort<T>
{
public:
    QuickSort() {};
    virtual ~QuickSort() {};

    int quickSort(T a[], int low, int high);
 
    int nonRecursion_quickSort(T a[], int low, int high);

private:
    int sortPartition(T a[], int low, int high);   
};

template<typename T>
int QuickSort<T>::sortPartition(T a[], int low, int high)
{
    T pivot = a[low];
    while (low < high)
    {
        // move high index
        while (low < high && a[high] > pivot)
        {
            high--;
        }
        if (low < high)
        {
            a[low] = a[high];
        }
        // move low index
        while (low < high && a[low] < pivot)
        {
            low++;
        }
        if (low < high)
        {
            a[high] = a[low];
        }
    }
    a[low] = pivot;
    return low;
}

template<typename T>
int QuickSort<T>::quickSort(T a[], int low, int high)
{
    if (low < high)
    {
        int key = sortPartition(a, low, high);
        quickSort(a, low, key - 1);
        quickSort(a, key + 1, high);
    }
    return 0;
}

template<typename T>
inline int QuickSort<T>::nonRecursion_quickSort(T a[], int low, int high)
{
    int key = 0;
    std::stack<int> values;
    if (low < high)
    {
        values.push(low);
        values.push(high);
    }
    else
        return 0;

    while (!values.empty())
    {
        int right = values.top();
        values.pop();
        int left = values.top();
        values.pop();
        key = sortPartition(a, left, right);
        if (left < key - 1)
        {
            values.push(left);
            values.push(key - 1);
        }
        if (key + 1 < right)
        {
            values.push(key + 1);
            values.push(right);
        }

    }
    cout << "nonRecursion_quickSort cost time: " << m_timer.stop() << " ms, " << m_steps << " steps" << endl;
    return 0;
}
#include <vector>
template<typename T>
class HeapSort : public MySort<T>
{
public:
    HeapSort();
    virtual ~HeapSort();

    void heapSort(vector<T> &arr);
private:
    void adjust(vector<T> &arr, int len, int index);
};

template<typename T>
HeapSort<T>::HeapSort()
{

}

template<typename T>
HeapSort<T>::~HeapSort()
{

}

template<typename T>
void HeapSort<T>::heapSort(vector<T> &arr)
{
    int size = arr.size();
    // 构建大根堆（从最后一个非叶子节点向上）
    for (int i = size / 2 - 1; i >= 0; i--)
    {
        adjust(arr, size, i);
    }

    // 调整大根堆
    for (int i = size - 1; i >= 1; i--)
    {
        swap(arr[0], arr[i]);           // 将当前最大的放置到数组末尾
        adjust(arr, i, 0);              // 将未完成排序的部分继续进行堆排序
    }
}

template<typename T>
void HeapSort<T>::adjust(vector<T> &arr, int len, int index)
{
    int left = 2 * index + 1; // index的左子节点
    int right = 2 * index + 2;// index的右子节点

    int maxIdx = index;
    if (left < len && arr[left] > arr[maxIdx])     
        maxIdx = left;
    if (right < len && arr[right] > arr[maxIdx])     
        maxIdx = right;

    if (maxIdx != index)
    {
        swap(arr[maxIdx], arr[index]);
        adjust(arr, len, maxIdx);
    }
}
#endif /* SRC_MYSORT_H_ */
