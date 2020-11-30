/*
 * Logger.cpp
 *
 *  Created on: Aug 10, 2016
 *      Author: gyd
 */
#include "Logger.h"

Logger::Logger():mExit(false)
{
	//Start background thread.
	mThread = std::thread(&Logger::processEntries, this);
}

Logger::~Logger()
{
    //shut down the thread by setting mExit
    unique_lock<mutex> lock(mMutex);
    mExit = true;
    mCondVar.notify_all();
	lock.unlock();
	mThread.join();
}

void Logger::log(const string& msg)
{
	//Lock mutex and add entry to the queue.
	unique_lock<mutex> lock(mMutex);
	mQueue.push(msg);
	//notify condition variable to wake up threads
	mCondVar.notify_all();
}

void Logger::processEntries()
{

	//start process loop
	unique_lock<mutex> lock(mMutex);
	while(true)
	{
        //Only wait for notification if we don't have to exit
        if(!mExit)
        {
            mCondVar.wait(lock);  //wait for a notification
        }
        //notified, so something might be in the queue.
        lock.unlock();
        //open log file
        ofstream ofs("SysMon_log.txt");
        if(ofs.fail())
        {
            cerr<<"Failed to open log file."<<endl;
            return;
        }
        while(true)
        {
            lock.lock();
            if(mQueue.empty() )
            {
                break;
            }
			else
			{
			    ofs<<mQueue.front()<<endl;
			    mQueue.pop();
			}
            lock.unlock();
        }
        ofs.close();
        if(mExit)
            break;
    }
}

////test
void logSomeMessages(int id, Logger& logger)
{
    for(int i=0; i<10; i++)
    {
	    stringstream ss;
		ss<<"Log entry"<<i;
		ss<<" from thread "<<id;
        logger.log(ss.str() );
    }

}
int test()
{
    Logger logger;
    vector<thread> threads;
    for(int i=0 ; i<3; i++)
    {
	    threads.emplace_back(logSomeMessages, i ,ref(logger));
    }
    //wait for all threads to finish
    for(auto& t:threads)
    {
	    t.join();
    }
    return 0;
}
