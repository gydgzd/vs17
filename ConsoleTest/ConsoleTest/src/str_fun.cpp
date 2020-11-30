#include "stdafx.h"
#include "str_fun.h"
#if defined(__linux)
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <locale.h>
#elif (defined WINVER ||defined WIN32)
#include <windows.h>
#endif


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

#if defined(__linux)
/*
 * DESCRIPTION: 实现由utf8编码到gbk编码的转换 
 * Input: gbkStr,转换后的字符串;  
 * Implement by mbstowcs and wcstombs
 * utf8->unicode->GBK
 * srcStr,待转换的字符串; 
 * maxGbkStrlen, gbkStr的最大长度 
 * Output: gbkStr 
 * Returns: -1,fail;>0,success
 */
int utf82gbk(char *gbkStr, const char *srcStr, int maxGbkStrlen)
{
    if (NULL == srcStr)
    {
        printf("Bad Parameter\n");
        return -1;
    }
    //首先先将utf8编码转换为unicode编码 
    if (NULL == setlocale(LC_ALL, "zh_CN.utf8")) //设置转换为unicode前的码,当前为utf8编码           
    {
        printf("Bad Parameter\n");
        return -1;
    }
    int unicodeLen = mbstowcs(NULL, srcStr, 0);
    //计算转换后的长度  
    if (unicodeLen <= 0)
    {
        printf("Can not Transfer!!!\n");
        return -1;
    }
    wchar_t *unicodeStr = (wchar_t *)calloc(sizeof(wchar_t), unicodeLen + 1);
    mbstowcs(unicodeStr, srcStr, strlen(srcStr));
    //将utf8转换为unicode   
    //将unicode编码转换为gbk编码     
    if (NULL == setlocale(LC_ALL, "zh_CN.gbk")) //设置unicode转换后的码,当前为gbk           
    {
        printf("Bad Parameter\n");
        return -1;
    }
    int gbkLen = wcstombs(NULL, unicodeStr, 0);
    //计算转换后的长度  
    if (gbkLen <= 0)
    {
        printf("Can not Transfer!!!\n");
        return -1;
    }
    else if (gbkLen >= maxGbkStrlen) //判断空间是否足够           
    {
        printf("Dst Str memory not enough\n");
        return -1;
    }
    wcstombs(gbkStr, unicodeStr, gbkLen);
    gbkStr[gbkLen] = 0; //添加结束符     
    free(unicodeStr);
    return gbkLen;
}

/* 
 * function: gbk2utf8 
 * description: 实现由gbk编码到utf8编码的转换  
 * Implement by mbstowcs and wcstombs
 * GBK->unicode->utf8
 * input: utfstr,转换后的字符串;  srcstr,待转换的字符串; maxutfstrlen, utfstr的最大长度 
 * output: utfstr 
 * returns: -1,fail;>0,success 
 */
int gbk2utf8(char *destStr, const char *srcstr, size_t maxutfstrlen)
{
    if (NULL == srcstr)
    {
        printf(" bad parameter\n");
        return -1;
    }
    //首先先将gbk编码转换为unicode编码  
    if (NULL == setlocale(LC_ALL, "zh_CN.gbk"))//设置转换为unicode前的码,当前为gbk编码  
    {
        printf("setlocale to zh_CN.gbk error!\n");
        return -1;
    }
    size_t unicodelen = mbstowcs(NULL, srcstr, 0);//计算转换后的长度  
    if (unicodelen <= 0)
    {
        printf("can not transfer!!!\n");
        return -1;
    }
    wchar_t *pstr_unicode = NULL;
    pstr_unicode = (wchar_t *)calloc(sizeof(wchar_t), unicodelen + 1);
    mbstowcs(pstr_unicode, srcstr, strlen(srcstr));//将gbk转换为unicode  


    //将unicode编码转换为utf8编码  
    if (NULL == setlocale(LC_ALL, "zh_CN.utf8"))//设置unicode转换后的码,当前为utf8  
    {
        printf("bad parameter\n");
        if (pstr_unicode != NULL)
            free(pstr_unicode);
        return -1;
    }
    size_t destlen = wcstombs(NULL, pstr_unicode, 0);//计算转换后的长度  
    if (destlen <= 0)
    {
        printf("can not transfer!!!\n");
        if (pstr_unicode != NULL)
            free(pstr_unicode);
        return -1;
    }
    else if (destlen >= maxutfstrlen)//判断空间是否足够  
    {
        printf("dst str memory not enough\n");
        if (pstr_unicode != NULL)
            free(pstr_unicode);
        return -1;
    }
    wcstombs(destStr, pstr_unicode, destlen);
    destStr[destlen] = 0;           //添加结束符  
    if (pstr_unicode != NULL)
        free(pstr_unicode);
    return destlen;
}

#endif

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


/* //  *用str替换指定字符串从起始位置pos开始长度为len的字符串  
    //  *string& replace(size_t pos, size_t len, const string& str);
'\' 或 ' 前面补1个'\',防止有\ ' 导致插入数据库错误
*/
int str_replace(char str[], int size, int strlenth)
{
	if (strchr(str, 39) == nullptr && strchr(str, 34) == nullptr && strchr(str, 92) == nullptr)  //  '\'92  '39 "34
		return 0;
	int i = 0, j = 0;
	int free_space = 0;  //str 中的空闲空间
	vector<int> index;
	for (i = 0; i < strlenth; i++)
	{
		if (str[i] == '\'' || str[i] == '\\' || str[i] == '\"')
		{
			index.push_back(i);
		}
	}
	free_space = size - strlenth - 1;
	int npos = (int)index.size();     //计算出 \ ' 的数量，每个位置补1个'\'
	if (npos - free_space > 0)
	{
		printf("字符串空间不足，替换字符失败！");
		return -1;
	}
	index.push_back(strlenth + 1);     //从字符串末尾开始移位
									   //从最后开始往后移位，只遍历一次
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
/*
 *从字符串src中删除字符串str
 */
int strDel(char *src, char *str)
{
    char *pos = src;
    vector<char *> index;
    for (; pos != NULL; pos += strlen(str))   // 首先获取全部位置
    {
        pos = strstr(pos, str);
        if (pos == NULL)
            break;
        index.push_back(pos);
    }
    int len = strlen(src);

    for (int n = index.size() - 1; n >= 0; n--) // 从最后一个位置开始移动
        for (char *tmp = index[n]; tmp < src + len - 1; tmp++)
            *tmp = *(tmp + strlen(str));
    return 0;
}
/*
从字符串中获取不包含重复字符的最大子串
*/
string maxSubStr(string str)
{
    string substr;
    unsigned int maxLen = 0;
    std::unordered_map<char, int> indexMap;
    int posBegin = 0;
    int lastBegin = 0;
    for (unsigned int i = 0; i < str.length(); )
    {
        auto iter = indexMap.find(str.at(i));
        if (iter != indexMap.end())    // found replicate char
        {
            if (maxLen < i - posBegin)
            {
                substr = str.substr(posBegin, i - posBegin);
                maxLen = i - posBegin;
            }
            lastBegin = posBegin;
            posBegin = iter->second + 1;
            
            // remove elements before posBegin in map 
            for (int tmp = lastBegin; tmp < posBegin; tmp++)
            {
                auto it = indexMap.find(str.at(tmp));
                if(it != indexMap.end())
                    indexMap.erase(indexMap.find(str.at(tmp)));
            }
        }
        else
        {
            indexMap.insert(std::pair<char, int>(str.at(i), i));
            i++;
        }
    }
    if (maxLen < str.length() - posBegin)
    {
        substr = str.substr(posBegin, str.length() - posBegin);
        maxLen = str.length() - posBegin;
    }
    return substr;
}