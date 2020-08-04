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
#ifdef __linux
#include <sys/stat.h> // mkdir stat
#elif (defined WINVER ||defined WIN32)
#include <direct.h>    // _mkdir
#endif

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
// my head files
#include "sql_conn_cpp.h"
#include "getDate.h"
//using namespace std;
#define LOG_FILENAME "log/program.log"
//Mutex and condition variable to protect access to the queue.
extern std::mutex g_log_Mutex;
extern std::condition_variable g_log_CondVar;
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
	void log(const char * logMsg);
	int logException(sql::SQLException &e, const char* file, const char* func, const int& line);
	int checkSize();
	int shrinkLogFile();

	std::string mstr_logfile;
private:
	long m_filesize;
	long max_filesize;

    void processEntries();        //background thread
    std::queue<string> mQueue;
};


#endif /* MYLOG_H_ */
