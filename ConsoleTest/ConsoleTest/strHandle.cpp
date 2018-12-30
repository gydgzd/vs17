
#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
/*
 *从字符串src中删除字符串str
 */
int strDel(char *src, char *str)
{	
	char *pos = src;
	vector<char *> index;
	for( ; pos!=NULL; pos+=strlen(str))   // 首先获取全部位置
	{
		pos = strstr(pos, str);
		if(pos==NULL)
			break;
		index.push_back(pos);
	}
	int len = strlen(src);

	for(int n = index.size()-1; n>=0; n--) // 从最后一个位置开始移动
		for(char *tmp = index[n]; tmp< src + len - 1; tmp++)
			*tmp = *(tmp+strlen(str));
	return 0;
}