#pragma once

#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <iostream>

using namespace boost::asio;

class testAsio
{
public:
	testAsio();
	virtual ~testAsio();


	int test();
};

