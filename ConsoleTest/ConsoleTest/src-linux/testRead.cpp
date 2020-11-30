/*
 * testRead.cpp
 *
 *  Created on: 2018年11月12日
 *      Author: gyd
 */

#include <stdio.h>
extern int utf82gbk(char *gbkStr, const char *srcStr, int maxGbkStrlen);
extern int gbk2utf8(char *destStr,const char *srcstr,size_t maxutfstrlen);
int testGets()
{
	char str[256];
	char strgbk[512];
	int lineNumber = 0;
	/* 打开用于读取的文件 */
	FILE *fp = fopen("/home/gyd/Documents/2.txt" , "r");
    if(fp == NULL) {
	    perror("打开文件时发生错误");
        return(-1);
	}
	while( fgets (str, 60, fp)!=NULL ) {
      /* 向标准输出 stdout 写入内容 */
		printf("第%2d 行\n", ++lineNumber);
		gbk2utf8(strgbk, str, 512);
		puts(strgbk);
    }

    fclose(fp);
    return 0;
}
