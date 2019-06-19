// ConsoleTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#define CRTDBG_MAP_ALLOC    
//#include <stdlib.h>    
//#include <crtdbg.h>        // 内存检测
#ifdef _DEBUG  
#define New   new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif

#include <windows.h>
#include <iostream>
#include <vector>
#include <locale.h>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include "getDate.h"
#include "sql_conn_cpp.h"  // my sql class
#include "MySort.h"
#include "testMultiThread.h"
#include "testStack.cpp"
//#include "testValist.cpp"
using namespace std;
int sockerServer();

char* testLeak()
{
	char *test = new char[4];
	return test;
}
char * testLocal()
{
	char * a = "hahaha";
	//	char a[] = "hahaha";  //返回指向局部变量的地址，编译警告
	return a;
}
extern int readFile();
extern int str_replace(char str[], int size, int strlenth);
extern int str_compare(char *str1, char *str2);
extern int logException(sql::SQLException &e, const char* file, const char* func, const int& line);

extern int log(const char * fmt, ...);
extern void testValist();
extern void printf_t(FILE *m_file, const char *fmt ...);

extern void simpleInterativeReverse();
extern void interativeReverse();
extern int parse_liveout();
extern int testSet();
extern int readFileStream();
extern int writeFileStream();
extern int testHashMap();
extern int testList();
int _tmain(int argc, _TCHAR* argv[])
{
//	testList();
//	testHashMap();
//	testSet();
	//	parse_liveout();
//	writeFileStream();
	sockerServer();
//	testValist();

/*	wcout.imbue(locale("")); 
	char * lcname = setlocale(LC_ALL, "chs");
	if (NULL == lcname)
	{
		printf("setlocale() from environment failed.\n");
	}
	wchar_t szError[1024] = L"世界"; // _T("你好");
//	swprintf_s(szError, L"世界");
	memcpy_s(szError, 10, L"世界", sizeof("世界"));
	_stprintf_s(szError, L"世界,你好！");
	wcout << szError << endl;
	printf("printf你好\n");
*/
//	_CrtSetBreakAlloc(752);
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
//	char *test = testLeak();
//	_CrtDumpMemoryLeaks();

	
//	Mycounter mc1; 
//	mc1.testThread();
 /*
 */
	// test of cin
 /*
	char str[30];
	string s1;
	getline(cin, s1);
	cout << s1 << endl;
	cin.ignore(100,'\n');    // 清除\n之前100个字符

	cin.getline(str,30);
	cout << str << endl;
	cin.ignore(100, '\n');   //调用getline后会激活键盘输入，输入'\n'，调用get不会
	

	char array[12] = { 0x01 , 0x02 , 0x03 , 0x04 , 0x05 , 0x06 , 0x07 , 0x08 };
	short *pshort = (short *)array;
	int *pint = (int *)array;
	_int64 *pint64 = (_int64 *)array;
	printf("0x%x , 0x%x , 0x%llx , 0x%llx", *pshort, *(pshort + 2), *pint64, *(pint + 2));
	*/
/*
	myqueue sta1;
	sta1.push(1);
	sta1.push(5);
	sta1.push(3);
	sta1.push(7);
	cout << sta1.pop() << endl;
	cout << sta1.pop() << endl;
	cout << sta1.pop() << endl;
	cout << sta1.pop() << endl;
	*/
//	interativeReverse();   // cin.get不会用
 

	// test of sort
/*	float fa[44] = { 1.2, 0.5, 3.6, 0.1, 3.4, 1.9, 33, 22, 35, 23, 755, 23, 121, 232.12, 193.23, 1201.23,122,232.21,21213,1242,11,242.24,2313824,232313,4242,32,42,423,43564,4755,2325,92.81 };
	float *pa = fa;
	MySort<float> mSort;
	clock_t t1 = clock();
	mSort.insertionSort(fa, sizeof(fa) / sizeof(fa[0]));
	clock_t t2 = clock();
	float tt = (float)(t2 - t1)/ CLOCKS_PER_SEC;
	for (int i = 0; i<sizeof(fa) / sizeof(fa[0]); i++)
		cout << fa[i] << "  ";
	cout << tt << "s"<< endl;
	cout << pa[12] << *(fa + 12)<< endl;
	*/

#ifdef __linux
	printf("Linux\n");
#endif
#ifdef WINVER
	printf("Windows\n");
#endif
	system("pause");
	return 0;
}
