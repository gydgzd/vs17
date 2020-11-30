/*
 * Logger.h
 *
 *  Created on: Aug 10, 2016
 *      Author: gyd
 */
#pragma once
#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include <queue>
#include <vector>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>

#include <mutex>
#include <atomic>
#include <condition_variable>
using namespace std;

class Logger
{
public:
	Logger();
	virtual ~Logger();
	//Prevent copy construction and assignment
	Logger(const Logger& src) = delete;
	Logger& operator=(const Logger& rhs) = delete;

	void log(const std::string& logMsg);

private:
	void processEntries();
	//Mutex and condition variable to protect access to the queue.
	std::mutex mMutex;
	std::condition_variable mCondVar;
	std::queue<string> mQueue;
	std::atomic<bool> mExit;
	//background thread
	std::thread mThread;
};
#endif /* LOGGER_H_ */
