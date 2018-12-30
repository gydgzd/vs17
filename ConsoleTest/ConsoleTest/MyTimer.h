/*
 * getDate.h
 *
 *  Created on: Jul 18, 2016
 *      Author: Gyd
 */
#pragma once
#ifndef MYTIMER_H_
#define MYTIMER_H_
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <string>
#include <iostream>
using namespace std;

#endif /* MYTIMER_H_ */
class MyTimer
{
public:
	MyTimer();
	~MyTimer();
	int setTimer();
	int checkTimer(int sec_count);
	void reset();



private:
	_int64 m_time;

public:

	void start();
	long long stop();

private:
	long long  m_timeBegin, m_timeEnd;
	long long  elapsed_time;

};