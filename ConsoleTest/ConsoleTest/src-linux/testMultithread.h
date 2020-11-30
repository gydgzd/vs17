/*
 * testMultithread.h
 *
 *  Created on: Feb 1, 2019
 *      Author: gyd
 */

#ifndef TESTMULTITHREAD_H_
#define TESTMULTITHREAD_H_


#pragma once

#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>
using namespace std;

extern int g_num;
extern std::mutex g_num_mutex;
class Mycounter
{
public:
    Mycounter();
    ~Mycounter();
    static int count(int n, int id);
    int mtdCount();
    void counter(int iterations, int id);
    void testThread();
};


#endif /* TESTMULTITHREAD_H_ */
