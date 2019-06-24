// test_rabbitmq.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "myrabbit.h"
// add boost test
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

int main()
{
	const char *hostname = "10.1.24.141";
	int port = 5672;
	char *username = "topo";
	char *passwd = "topo";

	int nCount = 3;
	MyRabbit myrabbit;
	int ret = 0;
/*	while ((ret = myrabbit.connectRabbit(hostname, port, username, passwd)) != 0)
	{
		Sleep(5000);
	}
	while ((ret = myrabbit.createQueue("test", "direct", "test", "test")) != 0)
	{
		Sleep(5000);
	}
	while (nCount != 0)
	{
		myrabbit.sendString("", "test", "hello nicle");
		nCount--;
	}
	myrabbit.closeRabbit();
*/
	char *buf = new char[2048 * 4000];
	int len = 0;
	myrabbit.connectRabbit(hostname, port, username, passwd);
	myrabbit.createQueue("FunExchange", "direct", "FunctionName", "cat");
	myrabbit.sendString("FunExchange", "cat", "This is a test.");
//	myrabbit.getString("FunctionName", buf, len);
	myrabbit.closeRabbit();

	FILE *fp;
	errno_t err;
	if ((err = fopen_s(&fp, "buf.txt", "wb")) != NULL)     //判断文件打开
	{
		char szLog[1280] = "";
		char szError[512] = "";
		printf_s(szLog, "Couldn't open %s!\n\n %s", "buf.txt", szError);
		exit(-1);    // #include <stdlib.h>                     
	}

	fwrite(buf, sizeof(char), len+1, fp );
	fclose(fp);
	delete[] buf;
/*		*/

	// test boost
	using namespace boost::lambda;
	typedef std::istream_iterator<int> in;
	std::for_each( in(std::cin), in(), std::cout << (_1 * 3) << " ");

	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	system("pause");
    return 0;
}

