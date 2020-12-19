/*
 * Mylog.cpp
 *
 *  Created on: Sep 18, 2017
 *      Author: gyd
 */

#include "Mylog.h"
const char* LOG_FILENAME = "log/program.log";
std::atomic<bool> g_log_Exit;
std::thread logth;
Mylog::Mylog(): m_filesize(-1),max_filesize(204800000), mstr_logfile(LOG_FILENAME)
{
    g_log_Exit = false;
    // get a linebuffer
    msp_linebuffer = std::shared_ptr<char>(new char[65536], [](char *p) { if(p != nullptr) delete[] p; });
    memset(msp_linebuffer.get(), 0, 65536);
    logth = std::thread{ &Mylog::processEntries, this }; // std::move(std::thread{ &Mylog::processEntries, this });
}
Mylog::Mylog(const char * filename):m_filesize(-1), max_filesize(204800000), mstr_logfile(filename)
{
    g_log_Exit = false;
    // get a linebuffer
    msp_linebuffer = std::shared_ptr<char>(new char[65536], [](char *p) { if (p != nullptr) delete[] p; });
    memset(msp_linebuffer.get(), 0, 65536);
	logth = std::thread{&Mylog::processEntries, this};
}
Mylog::~Mylog()
{
    //shut down the thread by setting mExit
 //   unique_lock<mutex> lock(g_log_Mutex);
    g_log_Exit = true;
 //   lock.unlock();
    g_log_CondVar.notify_all();
    if (logth.joinable())
    {
        logth.join();
    }
    msp_linebuffer.reset();
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
	fseek(fp, -((int)sizeof(tmp)-1), SEEK_END);
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
void Mylog::log(const char * fmt, ...)
{
    // convert variable param into buffer
    char * buffer = msp_linebuffer.get();
    memset(buffer, 0, 65536);
    va_list ap = nullptr;
    va_start(ap, fmt);
    vsnprintf(buffer, 65535, fmt, ap);
    va_end(ap);

    //Lock mutex and add entry to the queue.
    unique_lock<mutex> lock(g_log_Mutex);
    mQueue.emplace(buffer);
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
        _mkdir("./log");
#endif
        ofstream ofs(mstr_logfile.c_str(), std::ios::app);
        if(ofs.fail())
        {
            cerr<<"Failed to open log file."<<endl;
            return;
        }
        
        while(!g_log_Exit)
        {
            lock_guard<std::mutex> lock(g_log_Mutex);
            if (!mQueue.empty())
            {
                ofs << getLocalTimeUs("%Y-%m-%d %H:%M:%S") << "  ";
                ofs << mQueue.front() << std::endl;
                mQueue.pop();
            }
            else
                break;
        }
        ofs.close();
        checkSize();
        if(g_log_Exit)
            break;
    }
    printf("Thread exit.\n");
}