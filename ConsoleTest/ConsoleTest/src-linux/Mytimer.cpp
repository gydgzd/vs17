/*
 * Mytimer.cpp
 *
 *  Created on: Nov 25, 2017
 *      Author: gyd
 */
/*
 * struct timeval can also get a milisecond,which may need #include <sys/time.h>
 * timeval tm1;
 * gettimeofday(&tm1, 0);
 * int myms1 = tm1.tv_sec*1000 + tm1.tv_usec/1000;
 */
#include "Mytimer.h"

Mytimer::Mytimer() {
	elapsed_time = 0;
	m_timeBegin = 0;
	m_timeEnd = 0;
	countNum = 0;
	start();
}

Mytimer::~Mytimer() {
	// TODO Auto-generated destructor stub
}

void Mytimer::start()
{
	struct timeb t;
	ftime(&t);
	m_timeBegin = 1000*t.time + t.millitm;
	countNum++;
}
long long Mytimer::stop()
{
	struct timeb t;
	ftime(&t);
	m_timeEnd = 1000*t.time + t.millitm;
	elapsed_time = m_timeEnd - m_timeBegin;
	return elapsed_time;
}
/*
 * check if msec arrived; 1: arrived; 0: not arrived
 */
int Mytimer::count(long long msec)
{
    struct timeb t;
    ftime(&t);
    m_timeEnd = 1000*t.time + t.millitm;
    elapsed_time = m_timeEnd - m_timeBegin;
    if(elapsed_time >= msec)
        return 1;     // time arrived
    return 0;         // not arrived
}
