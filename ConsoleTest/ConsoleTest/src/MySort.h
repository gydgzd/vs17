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
template<class T>
class MySort {
public:
	MySort();
	~MySort();

	int insertionSort(T a[], int n);
	int selectSort(T a[], int n);
    int bubbleSort(T a[], int n);

    int shellSort(T a[], int n);
    int quickSort(T a[], int n);
private:
	MyTimer m_timer;
    void swap(T& a, T& b);
};

template<class T>
MySort<T>::MySort() {


}
template<class T>
MySort<T>::~MySort() {

}

template<typename T>
void MySort<T>::swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

template<class T>
int MySort<T>::insertionSort(T a[], int n)
{
	m_timer.start();

	int index_m,index_n;
	T smaller;
	for( index_m = 1; index_m< n; index_m++)
	{
        if (a[index_m] >= a[index_m - 1])      //improved
            continue;
		smaller = a[index_m];
		for(index_n = index_m - 1; index_n >= 0 && a[index_n] > smaller; index_n--)
			a[index_n + 1] = a[index_n];
		a[index_n + 1] = smaller;
	}
	cout <<"InsertionSort cost time: "<<m_timer.stop()<<" ms"<<endl;
	return 0;
}

template<class T>
int MySort<T>::selectSort(T a[], int n)
{
    m_timer.start();

    for (int i = 0; i < n; i++)
    {
        int smallest = i;
        for (int j = i; j < n; j++)
        {
            if (a[smallest] > a[j])
                smallest = j;
        }
        swap(a[smallest], a[i]);
    }
    cout << "selectSort cost time " << m_timer.stop() << "" << endl;
    return 0;
}

template<typename T>
int MySort<T>::bubbleSort(T a[], int n)
{
    m_timer.start();

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i; j < n - 1; j++)
        {
            if (a[j] > a[j + 1])
                swap(a[j], a[j + 1]);
        }

    }
    cout << "bubbleSort cost time " << m_timer.stop() << "" << endl;
    return 0;
}

template<class T>
int MySort<T>::shellSort(T a[], int n)
{
    m_timer.start();
    register int indx_m, indx_n, hCnt, h;
    int increments[20], k;
    // Create an appropriate number of increments h
    for (h = 1, indx_m = 0; h < n; indx_m++)
    {
        increments[indx_m] = h;
        h = 3 * h + 1;
    }
    // loop on different h
    for (indx_m--; indx_m >= 0; indx_m--)
    {
        h = increments[indx_m];
        //sort sub array
        for (hCnt = h; hCnt < 2 * h; hCnt++)
        {
            for (indx_n = hCnt; indx_n < n; )
            {
                T tmp = a[indx_n];
                k = indx_n;
                while (k - h >= 0 && tmp < a[k - h]) {
                    a[k] = a[k - h];
                    k -= h;
                }
                a[k] = tmp;
                indx_n += h;
            }
        }
    }
    cout << "shellSort cost time: " << m_timer.stop() << " ms" << endl;
    return 0;
}




#endif /* SRC_MYSORT_H_ */
