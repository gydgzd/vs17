/*
 * myURL.h
 *
 *  Created on: Sep 15, 2017
 *      Author: gyd
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "curl/curl.h"
#include "curl/easy.h"
#include "sql_conn_cpp.h"
#include "getDate.h"
#include "Mylog.h"
using namespace std;

#ifndef MYURL_H_
#define MYURL_H_
int myexec(std::string cmd, int& status ,string& errmsg); //popen execute an shell cmd

class MyURL {
public:
	MyURL();
	virtual ~MyURL();
	int downloadFile(const char *url, const char * savePath);  // save to tmppath
	int saveURL(const char *url,const char *savefile);

	int mvToDest(string despath);
	int setTaskStatus( int status, long taskID, long batchID);

	long m_taskID,m_batchID;
	string mstr_downPath;     // 临时存放地址
	Mylog m_log;

private:
	string mstr_url;         // 文件url地址
	const string mstr_response_file;  // 返回结果，包含下载文件的url


};

#endif /* MYURL_H_ */
