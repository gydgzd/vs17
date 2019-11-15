#pragma once

#define HAVE_REMOTE
#include "pcap.h"
#include "Win32-Extensions.h"
#include "Packet32.h"
#include "ntddndis.h"
#include <atlconv.h>    // for T2A ,USES_CONVERSION

#pragma comment(lib, "Packet.lib")
#pragma comment(lib, "wpcap.lib")
class TestWinPcap
{
public:
	TestWinPcap();
	~TestWinPcap();

	int MyPcap_init();
	int MyPcap_getMac(char *if_name, char *mac);
	int MyPcap_send();
	int MyPcap_recv();
	int process();

public:
	void TestWinPcap::printError_Win(char * msg);
};

