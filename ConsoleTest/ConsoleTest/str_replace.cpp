#include "stdafx.h"
#include <string>
#include <vector>
using namespace std;

/*
'\' �� ' ǰ�油1��'\',��ֹ��\ ' ���²������ݿ����
*/
int str_replace(char str[], int size, int strlenth)
{
	if (strchr(str, 39) == nullptr && strchr(str, 34) == nullptr && strchr(str, 92) == nullptr)  //  '\'92  '39 "34
		return 0;
	int i = 0, j = 0;
	int free_space = 0;  //str �еĿ��пռ�
	vector<int> index;
	for (i = 0; i < strlenth; i++)
	{
		if (str[i] == '\'' || str[i] == '\\' || str[i] == '\"')
		{
			index.push_back(i);
		}
	}
	free_space = size - strlenth - 1;
	int npos = (int)index.size();     //����� \ ' ��������ÿ��λ�ò�1��'\'
	if (npos - free_space > 0)
	{
		printf("�ַ����ռ䲻�㣬�滻�ַ�ʧ�ܣ�");
		return -1;
	}
	index.push_back(strlenth + 1);     //���ַ���ĩβ��ʼ��λ
									   //�����ʼ������λ��ֻ����һ��
	while (npos >= 1)
	{
		for (j = index[npos] - 1; j >= index[npos - 1]; j--)
			str[j + npos] = str[j];
		str[index[npos - 1] + npos - 1] = '\\';

		npos--;
	}
	return 0;
}

char *strRemoveBlank(char *str)
{
	int nlen = strlen(str);
	int nCountBlank = 0;
	for (int i = 0; i < nlen; i++)
	{
		if (*(str + i) != ' ')
		{
			nCountBlank = 0;
			continue;
		}
		nCountBlank++;
		if (nCountBlank >= 2)
		{
			int j = 0;
			for (j = i; j < nlen; j++)
			{
				*(str + j) = *(str + j + 1);
			}
			i--;
		}
	}

	return str;
}