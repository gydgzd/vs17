// example_boost.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
//#include <algorithm>
#include <chrono>
#include <boost/shared_ptr.hpp>

//test
#include "testAsio.h"
#include "InterProcess.h"
extern void testMultiIndex();
extern int testThread();

void Precision_Sleep(int interval)
{
    using namespace std::chrono;
    std::string buffer;
    buffer.assign(256, 0);
    buffer.clear();
    int i = 10;
    uint64_t total_used = 0;
    while (i) {
        i--;
        steady_clock::time_point time_begin = steady_clock::now();
        Sleep(interval);
        steady_clock::time_point time_end = steady_clock::now();
        char tmp[128] = { 0 };
        uint64_t used = duration_cast<microseconds>(time_end - time_begin).count();
        snprintf(tmp, 128, "%s Sleep %d ms, time used : %lld us\n",  __FUNCTION__, interval, used);
        total_used += used;
        buffer += tmp;
    }
    printf("%s\n", buffer.c_str());
    printf("%s Sleep %d ms, average %lld us\n\n",  __FUNCTION__, interval, total_used / 10);

}
int main()
{
	using namespace boost::lambda;
	typedef std::istream_iterator<int> in;
	std::cout << "Input some number please:" << std::endl;
//	std::for_each(
//		in(std::cin), in(), std::cout << (_1 * 3) << " ");
    Precision_Sleep(1);
    Precision_Sleep(10);
    testThread();
    testMultiIndex();


	testAsio myAsio;
//	myAsio.test();

    InterProcess iproc;
    iproc.memPool(0);

	system("pause");
	return 0;
}
