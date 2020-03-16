#pragma once

#include "./rapidjson/document.h"
#include "./rapidjson/prettywriter.h"
#include "./rapidjson/filereadstream.h"
#include "./rapidjson/filewritestream.h"
#include "./rapidjson/stringbuffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <windows.h>     // there's winsock.h included, so winsock2.h should be place before
#include <tchar.h>
using namespace std;
#define  GBYTES  1073741824  
#define  MBYTES  1048576  
#define  KBYTES  1024  
#define  DKBYTES 1024.0 

//https://docs.microsoft.com/zh-cn/windows/desktop/api/winerror/nf-winerror-succeeded
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#define UpdateDelta(DltMgr, NewValue) ((DltMgr)->Delta = (NewValue) - (DltMgr)->Value, (DltMgr)->Value = (NewValue))
namespace V2Vnms
{
	typedef struct _UINT64_DELTA
	{
		ULONG64 Value = 0;
		ULONG64 Delta = 0;
	} UINT64_DELTA, *PUINT64_DELTA;
class CDevInfo
{
public:
	CDevInfo();
	virtual ~CDevInfo();
	
	int Init();    //  @brief 创建一个tcp监听socket，epoll ;成功return 0，其他值失败
	
	void UnInit(); //  @brief 释放申请的资源
	
	virtual void Append(CDevInfo *pDev);
	
	virtual void Erase(CDevInfo *pDev);
	
	virtual int GetDevInfo(std::string &strDevInfo);
	
private:
	std::vector<CDevInfo *> m_DevVector;

};

}// namespace

