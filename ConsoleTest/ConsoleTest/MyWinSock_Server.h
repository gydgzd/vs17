#pragma once
extern int g_Running ;
class MyWinSock_Server
{
public:
	MyWinSock_Server();
	~MyWinSock_Server();


private:
	int mn_recvLen;
	int mn_sendLen;
	TCHAR recvBuf[4096];
	TCHAR sendBuf[4096];
};

