// example_boost.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "testAsio.h"
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
//#include <algorithm>

int main()
{
	using namespace boost::lambda;
	typedef std::istream_iterator<int> in;
	std::cout << "Input some number please:" << std::endl;
//	std::for_each(
//		in(std::cin), in(), std::cout << (_1 * 3) << " ");

	testAsio myAsio;
	myAsio.test();
	system("pause");
	return 0;
}
