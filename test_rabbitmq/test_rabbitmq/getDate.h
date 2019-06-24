/*
 * getDate.h
 *
 *  Created on: Jul 18, 2016
 *      Author: Gyd
 */
#pragma once
#ifndef GETDATE_H_
#define GETDATE_H_
#include <time.h>
#include <stdio.h>
#include <string>
using namespace std;

string getSqlTime();
string getLocalTime();
string getYMD();
string getYear();

#endif /* GETDATE_H_ */
