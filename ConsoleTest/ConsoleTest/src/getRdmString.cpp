#include "stdafx.h"
#include <string>
#include <stdio.h>
using namespace std;

string getRdmString(int maxLenth)
{
//	string rmdStr;
//	srand(time(0));
	
	//�������0-25�����֣���Ӧ26����ĸ
/*	for(int i=0; i<60; i++)
	{
		int num = rand()%26;
	//	cout<<num<<endl;
		printf("%c\n",num+65);  //��д��ĸ A-Z 65-90   Сд��ĸ a-z 97-122
	}
	*/
	//�������һ����ĸ
/*	char *letters = new char[53];
	int index = 0;
	for(int i=0; i<26; i++)
	{
		*(letters+index) = char(i+65);
		index++;
	}
	for(int i=0; i<26; i++)
	{
		*(letters+index) = char(i+97);
		index++;
	}
	*(letters+52) = '\0';
	*/	
	char letters[53]= 
	{'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','\0'};
	int nLable = 0;
	char* tmp_str = new char[maxLenth];
	string rmdStr = "";
	for (int i = 0; i < maxLenth-1; i++)
	{
		nLable = rand()%52;
		tmp_str[i] = letters[nLable];
	}
	tmp_str[maxLenth-1] = '\0';
	rmdStr = tmp_str;
//	cout<<rmdStr<<endl;

	delete[] tmp_str;
	//delete[] letters;
	return rmdStr;
}