#include "stdafx.h"
#include "MyTimer.h"

MyTimer::MyTimer()
{
	elapsed_time = 0;
	m_timeBegin = 0;
	m_timeEnd = 0;
}

MyTimer::~MyTimer()
{

}

int MyTimer::setTimer()
{
	m_time = time(0);
	return 0;
}
/* return 1 when second count reached ,else return 0 */
int MyTimer::checkTimer(int sec_count)
{
	_int64 now_time = time(0);
	if (now_time - m_time < sec_count)
		return 0;
	else
		return 1;
}

void MyTimer::reset()
{
	m_time = time(0);
	return ;
}
void MyTimer::start()
{
	struct timeb t;
	ftime(&t);
	m_timeBegin = 1000 * t.time + t.millitm;
	cout << "Begin:" << m_timeBegin << endl;
}
// ms
long long MyTimer::stop() 
{
	struct timeb t;
	ftime(&t);
	m_timeEnd = 1000 * t.time + t.millitm;
	cout << "End:" << m_timeEnd << endl;
	elapsed_time = m_timeEnd - m_timeBegin;
	return elapsed_time;
}