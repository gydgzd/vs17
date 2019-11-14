#pragma once

#define HAVE_REMOTE
#include "pcap.h"
#include "Win32-Extensions.h"

class TestWinPcap
{
public:
	TestWinPcap();
	~TestWinPcap();

	int process();
};

