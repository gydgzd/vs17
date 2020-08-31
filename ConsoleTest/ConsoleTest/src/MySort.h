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
	int quickSort();
private:
	MyTimer m_timer;
};

template<class T>
MySort<T>::MySort() {


}
template<class T>
MySort<T>::~MySort() {

}
template<class T>
int MySort<T>::insertionSort(T a[], int n)
{
	m_timer.start();

	int indx_m,indx_n;
	T smaller;
	for( indx_m = 1; indx_m< n; indx_m++)
	{
		smaller = a[indx_m];
		for( indx_n = indx_m - 1; indx_n >= 0 && a[indx_n] > smaller; indx_n--)
			a[indx_n + 1] = a[indx_n];
		a[indx_n + 1] = smaller;
	}
	cout <<"InsertionSort cost time: "<<m_timer.stop()<<" ms"<<endl;
	return 0;
}

#endif /* SRC_MYSORT_H_ */
