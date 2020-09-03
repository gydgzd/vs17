#include "stdafx.h"
#include "str_fun.h"
#include <windows.h>


std::string Utf8ToGbk(const char *src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    std::string strTemp(szGBK);
    if (wszGBK)
        delete[] wszGBK;
    if (szGBK)
        delete[] szGBK;
    return strTemp;
}


int str_compare(char *str1, char *str2)
{
	int ret = 0;
	while (*str1 != 0 && *str2 != 0)
	{
		ret = *str1++ - *str2++;
		if (ret != 0)
			return ret;
	}
	if (*str1 == 0 && *str2 == 0)
		return 0;
	else if (*str1 == 0)
		return *str2;
	else
		return *str1;
}


/* //  *��str�滻ָ���ַ�������ʼλ��pos��ʼ����Ϊlen���ַ���  
    //  *string& replace(size_t pos, size_t len, const string& str);
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