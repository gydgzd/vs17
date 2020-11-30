#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
using namespace std;

int str_compare(char *str1, char *str2);

int str_replace(char str[], int size, int strlenth);

#if defined(__linux)
int utf82gbk(char *gbkStr, const char *srcStr, int maxGbkStrlen);
int gbk2utf8(char *destStr, const char *srcstr, size_t maxutfstrlen);
#elif (defined WINVER ||defined WIN32)
std::string Utf8ToGbk(const char *src_str);
#endif