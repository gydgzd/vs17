/*
 * readPart.cpp
 *
 *  Created on: Jul 22, 2016
 *      Author: Gyd
 */
#include <sys/statfs.h>
#include <stdio.h>
#include <string>
using namespace std;
int getVolum(string path)  //get disk space by mount path
{

	struct statfs diskInfo;
	statfs(path.c_str(),&diskInfo);
	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long ttlsize = blocksize * diskInfo.f_blocks;

	unsigned long long free = diskInfo.f_bfree * blocksize;
	printf("Free=%llu MB,total=%llu MB \n",free/1024/1024,ttlsize/1024/1024);
	return 0;
}

