/*
 * Mylog.h
 *
 *  Created on: Sep 18, 2017
 *      Author: gyd
 */
#pragma once
#ifndef MYLOG_H_
#define MYLOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>   // fopen

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>   // strlen strerror
#include <errno.h>    // errno

#ifdef __linux
#include <sys/stat.h> // mkdir stat
#elif WINVER || WIN32
#include <direct.h>    // _mkdir
#endif

// my head files
#include "sql_conn_cpp.h"
#include "getDate.h"
//using namespace std;
/*
 * lock
 */
class Mylog {
public:
	Mylog();
	Mylog(const char *filename);
	~Mylog();
	void setLogFile(const char *filename);
	void setMaxFileSize(long maxsize);
	int logException_ofs(const std::string& logMsg);
	int logException_fopen(const std::string& logMsg);
	int logException(const unsigned char * logMsg, int length);  // mainly for log hexadecimal
	int logException(sql::SQLException &e, const char* file, const char* func, const int& line);
	int checkSize();
	int shrinkLogFile();

	std::string mstr_logfile;
private:
	long m_filesize;
	long max_filesize;
	FILE *m_fp;
	std::ofstream m_ofs;
	unique_ptr<FILE> m_ufp;
};


#endif /* MYLOG_H_ */
