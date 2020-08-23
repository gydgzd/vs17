/*
 * Mylog.cpp
 *
 *  Created on: Sep 18, 2017
 *      Author: gyd
 */

#include "Mylog.h"
std::mutex g_log_Mutex;
std::condition_variable g_log_CondVar;
std::atomic<bool> g_log_Exit;
std::thread logth;
Mylog::Mylog(): m_filesize(-1),max_filesize(204800000)
{
    g_log_Exit = false;
    mstr_logfile = LOG_FILENAME;
    logth = std::thread{&Mylog::processEntries, this};
}
Mylog::Mylog(const char * filename):m_filesize(-1), max_filesize(204800000)
{
    g_log_Exit = false;
	mstr_logfile = filename;
	logth = std::thread{&Mylog::processEntries, this};
}
Mylog::~Mylog()
{
    //shut down the thread by setting mExit
 //   unique_lock<mutex> lock(g_log_Mutex);
    g_log_Exit = true;
 //   lock.unlock();
    g_log_CondVar.notify_all();
    logth.join();
}
void Mylog::setLogFile(const char *filename)
{
	mstr_logfile = filename;
	return ;
}
void Mylog::setMaxFileSize(long maxsize)
{
	max_filesize = maxsize;
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
	FILE *fp;
	if (!(fp=fopen(mstr_logfile.c_str(),"r+")))     //判断文件打开  #include <stdlib.h>
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror(errno));
		return -1;
	}
	char tmp[2048] = "";
	fseek(fp, -(sizeof(tmp)-1), SEEK_END);
	size_t rsize = fread(tmp, sizeof(char),sizeof(tmp)-1,fp);
	char *pos = strchr(tmp, '\n');
	pos++;
	fclose(fp);
	// open again to write
	if (!(fp=fopen(mstr_logfile.c_str(),"w+")))     //判断文件打开  #include <stdlib.h>
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror(errno));
		return -1;
	}
	rsize = fwrite(pos, sizeof(char),strlen(pos),fp);
	fclose(fp);
	if( rsize != strlen(pos) )
	{
		this->logException("ERROR: write size does not match when shrink the file!");
	}
	return 0;
}
// put log msg into the queue
void Mylog::log(const char * logMsg)
{
    //Lock mutex and add entry to the queue.
    unique_lock<mutex> lock(g_log_Mutex);
    mQueue.emplace(logMsg);
    lock.unlock();
    //notify condition variable to wake up threads
//    g_log_CondVar.notify_all();    // it will add two locks here
    g_log_CondVar.notify_one();
	return ;
}
int Mylog::logException(const std::string& logMsg)
{
	//open log file
#ifdef __linux
		mkdir("./log", S_IRWXU | S_IRWXG);   //S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#elif (defined WINVER ||defined WIN32)
	_mkdir("./log");
#endif
	std::ofstream ofs(mstr_logfile.c_str(), std::ios::app);  //c++11 support ofs(mstr_logFileName ,ios::app)
	if (ofs.fail())
	{
	    std::cerr << "Failed to open log file. " <<mstr_logfile<<":"<<strerror(errno)<< std::endl;
	    return -1;
	}
	std::string mytime = getLocalTimeUs("%Y-%m-%d %H:%M:%S");
	ofs << mytime <<"  ";
	ofs << logMsg << std::endl;
	ofs.close();
	checkSize();
	return 0;
}

int Mylog::logException(sql::SQLException &e, const char* file, const char* func, const int& line)
{
	//open log file
#ifdef __linux
	mkdir("./log", S_IRWXU | S_IRWXG);   //S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#elif (defined WINVER ||defined WIN32)
	_mkdir("./log");
#endif
	//string fullPath = "./log"+logFileName;
	std::ofstream ofs(mstr_logfile.c_str(), std::ios::app);
	if (ofs.fail())
	{
	    std::cerr << "Failed to open log file. " <<mstr_logfile<<":"<<strerror(errno)<< std::endl;
		return -1;
	}
	std::string mytime = getLocalTimeUs("%Y-%m-%d %H:%M:%S");
	ofs << mytime <<"  ";
	ofs << "# ERR: SQLException in " << file;
	ofs << "(" << func << ") on line " << line << std::endl;
	ofs << "                     # ERR: " << e.what();
	ofs << " (MySQL error code: " << e.getErrorCode();
	ofs << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	ofs << std::endl;
	ofs.close();
	checkSize();
	return 0;
}

void Mylog::processEntries()
{
    //start process loop
    while(true)
    {
        std::unique_lock < std:: mutex > locker(g_log_Mutex);
        //Only wait for notification if we don't have to exit
        if(!g_log_Exit && mQueue.empty())
        {

            g_log_CondVar.wait(locker);  //wait for a notification
        }
        locker.unlock();      //  ******this position is important for the lock
        //notified, so something might be in the queue.
        //open log file
#if defined __linux
        mkdir("./log", S_IRWXU | S_IRWXG);   //S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#elif (defined WINVER ||defined WIN32)
        mkdir("./log");
#endif
        ofstream ofs(mstr_logfile.c_str(), std::ios::app);
        if(ofs.fail())
        {
            cerr<<"Failed to open log file."<<endl;
            return;
        }
        
        while(!g_log_Exit)
        {
            locker.lock();
            if (!mQueue.empty())
            {
                ofs << getLocalTimeUs("%Y-%m-%d %H:%M:%S") <<"  ";
                ofs << mQueue.front() << std::endl;
                mQueue.pop();
            }
            locker.unlock();
        }
        ofs.close();
        checkSize();
        if(g_log_Exit)
            break;
    }
    printf("Thread exit.\n");
}
/* get time from mysql server
string getSqlTime()
{
    string date_str;
    string dbname = "websiteman";
    string sql = "select date_format(now(),'%Y-%m-%d %H:%i:%s') time";    // date_format(now(6),'%Y-%m-%d %H:%i:%s %f')
    try
    {
       Csql_conn_cpp  sqlcon;
       sqlcon.sql_connect("10.1.122.140");
       sqlcon.setDBName(dbname);
       sqlcon.sql_execute(sql,1);  //put result into ss.res-> and there might be an exception here either
       if (sqlcon.m_res->next())
       {
           date_str = sqlcon.m_res->getString("time").c_str();  //getString()
       }
    }
    catch(sql::SQLException &e)
    {
        date_str = getLocalTime("%Y-%m-%d %H:%M:%S");    // get local time when failed
    }
    return date_str;
}
*/
/*
return the local time, like
"%Y-%m-%d %H:%M:%S",
"%Y-%m-%d",
"%Y"
*/
std::string getLocalTime(const char *format)
{
    time_t t = time(0);
    std::string date_str = "";
    char tmp[32] = {};

#ifdef __linux
    strftime(tmp, sizeof(tmp), format, localtime(&t));
#elif (defined WINVER ||defined WIN32)
    tm timeinfo;
    localtime_s(&timeinfo, &t);
    strftime(tmp, sizeof(tmp), format, &timeinfo);
#endif

    date_str = tmp;
    return date_str;
}
/*
return the local time, format is like
"%Y-%m-%d %H:%M:%S",
"%Y-%m-%d",
"%Y"
then add the us in the tail
*/
std::string getLocalTimeUs(const char *format)
{
    std::string date_str = "";
    char tmp[32] = {};
    struct timeval tv;
#ifdef __linux

    gettimeofday(&tv, NULL);
    strftime(tmp, sizeof(tmp), format, localtime(&tv.tv_sec));
#elif (defined WINVER ||defined WIN32)
    // 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME   (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    int64_t tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
    tt = (li.QuadPart - EPOCHFILETIME) / 10;
    tv.tv_sec = tt / 1000 / 1000;
    tv.tv_usec = tt - tv.tv_sec * 1000 * 1000;
    tm timeinfo;
    time_t sec = tv.tv_sec;
    localtime_s(&timeinfo, &sec);
    strftime(tmp, sizeof(tmp), format, &timeinfo);
#endif

    date_str = tmp;
    sprintf(tmp, " %06ld", tv.tv_usec);
    date_str += tmp;
    return date_str;
}
/*
 * change the string format date into seconds
 * return the seconds
 */
time_t dateToSeconds(const char *str)
{
    tm timeinfo;
    int year, month, day, hour, minute, second;
    sscanf(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = 0;

    time_t t_sec = mktime(&timeinfo); // change from tm to second since 1970-01-01 00:00:01 已经减了8个时区
    return t_sec; //秒时间
}
