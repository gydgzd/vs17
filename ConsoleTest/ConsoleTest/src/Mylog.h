/*
 * Mylog.h
 *
 *  Created on: Sep 18, 2017
 *      Author: gyd
 */
#pragma once
#ifndef MYLOG_H_
#define MYLOG_H_

#include <stdlib.h>   // fopen
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>   // strlen strerror
#include <errno.h>    // errno
#include <stdarg.h>   // va_list
#ifdef __linux
#include <sys/stat.h> // mkdir stat
#include <sys/time.h>
#elif (defined WINVER || defined WIN32 || defined _WIN32)
#include <direct.h>    // _mkdir
#include <Winsock2.h>        // timeval 
#include <windows.h>
#endif
#include <time.h>

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
// my head files
#include "thirdLib/sql_conn_cpp.h"
#include "getDate.h"
//using namespace std;
extern const char* LOG_FILENAME;

//Mutex and condition variable to protect access to the queue.


extern std::atomic<bool> g_log_Exit;

class Mylog {
    
public:
	Mylog();
	Mylog(const char *filename);
	virtual ~Mylog();
    //Prevent copy construction and assignment
    //Mylog(const Mylog& src) = delete;
    //Mylog& operator=(const Mylog& rhs) = delete;

	void setLogFile(const char *filename);
	void setMaxFileSize(long maxsize);
	int logException(const std::string& logMsg);
	void log(const char * fmt, ...);
	int logException(sql::SQLException &e, const char* file, const char* func, const int& line);
	int checkSize();
	int shrinkLogFile();

    std::mutex g_log_Mutex;
    std::condition_variable g_log_CondVar;
private:
	long m_filesize;
	long max_filesize;
    std::string mstr_logfile;
    std::shared_ptr<char> msp_linebuffer;
    void processEntries();        //background thread
    std::queue<string> mQueue;
};


#endif /* MYLOG_H_ */
