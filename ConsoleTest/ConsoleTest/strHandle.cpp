
#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
/*
 *���ַ���src��ɾ���ַ���str
 */
int strDel(char *src, char *str)
{	
	char *pos = src;
	vector<char *> index;
	for( ; pos!=NULL; pos+=strlen(str))   // ���Ȼ�ȡȫ��λ��
	{
		pos = strstr(pos, str);
		if(pos==NULL)
			break;
		index.push_back(pos);
	}
	int len = strlen(src);

	for(int n = index.size()-1; n>=0; n--) // �����һ��λ�ÿ�ʼ�ƶ�
		for(char *tmp = index[n]; tmp< src + len - 1; tmp++)
			*tmp = *(tmp+strlen(str));
	return 0;
}