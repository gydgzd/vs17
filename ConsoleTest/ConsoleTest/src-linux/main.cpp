//============================================================================
// Name        : test.cpp
// Author      : gyd
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
using namespace std;

#include "myProgram.h"
#include "zmq_wrapper.h"
#include "testMultithread.h"
#include "zlib.h"
#include "Logger.h"
#include "zconf.h"
#include "testMysqlclient.h"
#include "MyHttpServer.h"
#include "MyHttpClient.h"
#include "TestPcap.h"
#include "easylogging++.h"    // v9.96.7
//#include "nf-queue.c"
#include "nfqnl_test.c"
#include "ping.cpp"
#include "RawSocket.h"
#include "myTunTap.h"
#include "Mylog.h"
#include "Mytimer.h"
#include "HttpServerLibevent.h"
INITIALIZE_EASYLOGGINGPP      // needed by easylogging
void LogInit()
{
    el::Configurations conf("log.conf");
    el::Loggers::reconfigureAllLoggers(conf);
}
//using namespace MyClass;
extern int my_zmq_server ();
extern int my_zmq_client ();
extern int testSQLite(int argc, char **argv);
string gzip_uncompress(const char *src, size_t length);
extern char * strReturn(void);
extern char* create_monitor(void);

extern int parse(const char * const str);
extern char* createJSON(void);
extern int utf82gbk(char *gbkStr, const char *srcStr, int maxGbkStrlen);
extern int testGets();
extern int test_unorderedMap();
extern int testMap();
extern int socket_server();
extern int test_udpclient();
extern int testPointerOfFunction();
extern void testFunction();
extern int testRapidJson();
extern int testCallback();
extern void testCPPCallback();
extern int testTimer();
extern int testAmqpcpp();
extern int test_popen();
struct test
{
    int a;
    char b;
    int c;
};
void sigHandler(int signum)
{
    printf("catch a signal SIGINT, program exit!\n");
    exit(0);
}
extern void myprint();
int nfqnl_test();

//int test_nf_queue();
int main(int argc, char ** argv)
{
    char path[128] = "";
#ifdef WINVER
    cout << _getcwd(path, 128) << endl;
#elif __linux
    cout << getcwd(path, 128) << endl;
#endif
    std::string fileName = "/proc/self/exe";
    size_t linksize = 256;
    char buffer[256] = {0};
    if(readlink(fileName.c_str() , buffer, linksize) !=-1 )
    {
        fileName = buffer;
    }
    printf("Application File name = %s\n", fileName.c_str());

    LogInit();                // easylogging++ init
    HttpServerLibevent lvhttp;
    lvhttp.testLibevent();

/*
    myTunTap tun1;
    tun1.dev_write();

    RawSocket ms;
    for(int i = 0; i< 1; i++)
    {
        ms.sendPkt();
    }
    //test ping
    char dstIP[] = "192.168.95.227";
    try
    {
        boost::asio::io_service io_service;
        pinger p(io_service, dstIP);
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
*/

    Mylog mylog;
    Mytimer t1;
    t1.start();
    for(int i = 0; i < 100; i++)
        LOG(INFO) <<"hei, log";
    cout << t1.stop() << endl;

    Mytimer tt;
    tt.start();
    for(int i = 0; i < 100; i++)
    {
        mylog.log("hei, log queue");
    }
    cout << tt.stop() << endl;

/**/
//    testAmqpcpp();
//    test_popen();

//    TestPcap mypcap;
//    mypcap.testPcap();
//    nfqnl_test();
//    test_nf_queue();
    string operations = "hi;peters";
    string::size_type pos1 = operations.find(';');
    string op1 = operations.substr(0, pos1);

    cout << testTimer() << endl;
//    cout << testCallback() << endl;
//    testCPPCallback();
    testRapidJson();
    char* buf = new char[512];
    buf = createJSON();
      cout << buf << endl;
      delete []buf;
 //   testPointerOfFunction();
//    testFunction();
//    test_udpclient();
 //   socket_server();
/*
    Mycounter mc1;
    std::thread th{ &Mycounter::counter, &mc1,10, 1 };
    th.detach();
    sleep(1);
    test_unorderedMap();
    signal(SIGINT, sigHandler);
    struct timeval tv;
    for(int i = 0; i< 5; i++)
    {
        gettimeofday(&tv,NULL);
        cout << tv.tv_sec  << "  " << tv.tv_usec << endl;
        sleep(1);
    }
    */
/*    char sqlstr[512] = "select now() from test";
    testMysqlclient mycqlconn;
    mycqlconn.mysqlconnect("10.1.24.141", "root", "123456", "test");
    mycqlconn.mysql_execute(sqlstr, 0);
*/

//    testMap();
/*	test_unorderedMap();
	testGets();
	char c = 0xff;
	char d = 0xfe;
	if( 0 == memcmp(&c,&d, 1))
		cout<< "true" << endl;
	cout<< "false" <<endl;
	test t1;
	t1.a = 9;
	t1.b = 'a';
	t1.c = 12;
	printf("%x\n", &t1.a);
	printf("%64x, %d\n", t1, sizeof(t1));
	*/
//	char * str = create_monitor();
//	str = strReturn();
//	cout << str << endl;

//	testSQLite(argc, argv);
/*
	map<int, string> mtest;
    mtest[1].assign("name");
    if (0 == strcmp(argv[1], "s"))  // return 0 if equal
    	my_zmq_server ();
    else if (0 == strcmp(argv[1], "c"))
    	my_zmq_client ();
    else
    	cout << "param error!" << endl;
*/
/*
	cZmqWrapper zw;
	zw.client("tcp://10.1.24.63:14200");
	char *buf = new char[512];
	while(true)
	{
		int rc=zw.recv(buf, 0x10000);
		if(rc<1) continue;
		else
		{

		//	string ss = gzip_uncompress(buf + 8, (unsigned long)rc);  // there is a t_live_head here,which is 8 bytes
		//	printf("%s\n", ss.c_str());
		//	printf("%s\n", buf);
		}
		buf = createJSON();
		cout << buf << endl;
	}
	delete[] buf;
*/
	/*
	getVolum("/");
	getVolum("/boot");
	getMapper();
	searchPartTable("/dev/sda1");
	searchPartTable("/dev/mapper/centos-root");
	cout<<"├"<<endl;
	*/
/*	try
	{
		int x=3;
		cout<<x<<endl;
		throw(x);
	}
	catch(int)
	{
		cout<<"catch"<<endl;
	}
	char sa[5]="aa",sb[6]="bb";
	scanf("%s%s", sa, sb);
	cout<<sa<<" "<<sb<<endl;
	*/

	return 0;
}

