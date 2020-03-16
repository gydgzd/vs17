/*
 * Mylog.cpp
 *
 *  Created on: Sep 18, 2017
 *      Author: gyd
 */
#include "stdafx.h"
#include "Mylog.h"

Mylog::Mylog():max_filesize(2048000)
{
	m_filesize = -1;
	mstr_logfile = "log/program.log";
}

Mylog::Mylog(const char * filename):max_filesize(2048000)
{
	m_filesize = -1;
	mstr_logfile = filename;
}

Mylog::~Mylog()
{

}
void Mylog::setLogFile(const char *filename)
{
	mstr_logfile = filename;
	return ;
}
/*
 * check the size of log
 * shrink it if it is too big
 */
int Mylog::checkSize()
{
	struct stat buf;
	if((stat(mstr_logfile.c_str(),&buf)!=-1))
	{
		m_filesize = buf.st_size;
		if(m_filesize > (1.2*max_filesize) )
		{
			shrinkLogFile();
		}
	}
	else
	{
		perror(mstr_logfile.c_str());
		return 1;
	}
	return 0;
}
/*
 * shrink the log file
 * shrink to about 2048 byte
 */
int Mylog::shrinkLogFile()
{
	char errmsg[1024] = "";
	FILE *fp;
#ifdef WINVER
	errno_t err;
	if ((err = fopen_s(&fp, mstr_logfile.c_str(), "r+")) != NULL)     //判断文件打开
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror_s(errmsg, errno));
		return -1;
	}
#endif
#ifdef __LINUX
	if (!(fp=fopen(mstr_logfile.c_str(),"r+")))     //判断文件打开  #include <stdlib.h>
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror(errno));
		return -1;
	}
#endif
	char tmp[2048] = "";
	fseek(fp, -(long)(sizeof(tmp)-1), SEEK_END);
	size_t rsize = fread(tmp, sizeof(char),sizeof(tmp)-1,fp);
	char *pos = strchr(tmp, '\n');
	pos++;
	fclose(fp);

	// open again to write
#ifdef WINVER
	if ((err = fopen_s(&fp, mstr_logfile.c_str(), "w+")) != NULL)     //判断文件打开
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror_s(errmsg, errno));
		return -1;
	}
#endif
#ifdef __LINUX
	if (!(fp = fopen(mstr_logfile.c_str(), "w+")))     //判断文件打开  #include <stdlib.h>
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror(errno));
		return -1;
	}
#endif

	rsize = fwrite(pos, sizeof(char),strlen(pos),fp);
	fclose(fp);
	if( rsize != strlen(pos) )
	{
		this->logException("ERROR: write size does not match when shrink the file!");
	}
	return 0;
}

int Mylog::logException(const string& logMsg)
{
	char errmsg[1024] = "";
	//open log file
#ifdef WINVER
	_mkdir("./log");
	ofstream ofs(mstr_logfile.c_str(), ios::app);  //c++11 support ofs(mstr_logFileName ,ios::app)
	if (ofs.fail())
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror_s(errmsg, errno));
		return -1;
	}
#endif
	//open log file
#ifdef __linux
	mkdir("./log", S_IRWXU | S_IRWXG);   //S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
	ofstream ofs(mstr_logfile.c_str(), ios::app);  //c++11 support ofs(mstr_logFileName ,ios::app)
	if (ofs.fail())
	{
		cerr << "Failed to open log file. " << mstr_logfile << ":" << strerror(errno) << endl;
		return -1;
	}
#endif

	string mytime = getLocalTime();
	ofs << mytime <<"  ";
	ofs << logMsg << endl;
	ofs.close();
	checkSize();
	return 0;
}

int Mylog::logException(sql::SQLException &e, const char* file, const char* func, const int& line)
{
	char errmsg[1024] = "";
	//open log file
#ifdef __linux
	mkdir("./log", S_IRWXU | S_IRWXG);   //S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#endif
#ifdef WINVER
	_mkdir("./log");
#endif
	//string fullPath = "./log"+logFileName;
	ofstream ofs(mstr_logfile.c_str(), ios::app);
	if (ofs.fail())
	{
#ifdef __LINUX
		cout << "Failed to open log file. " << mstr_logfile << ":" << strerror(errno) << endl;
#endif
#ifdef WINVER
		strerror_s(errmsg, errno);
		cout << "Failed to open log file. " << mstr_logfile << ":" << errmsg << endl;
#endif
		return -1;
	}
	string mytime = getLocalTime();
	ofs << mytime <<"  ";
	ofs << "# ERR: SQLException in " << file;
	ofs << "(" << func << ") on line " << line << endl;
	ofs << "                     # ERR: " << e.what();
	ofs << " (MySQL error code: " << e.getErrorCode();
	ofs << ", SQLState: " << e.getSQLState() << " )" << endl;
	ofs << endl;
	ofs.close();
	checkSize();
	return 0;
}
