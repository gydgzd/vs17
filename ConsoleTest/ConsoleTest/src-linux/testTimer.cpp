/*
 * testTimer.cpp
 *
 *  Created on: May 17, 2019
 *      Author: gyd
 */
#include <iostream>
#include "Mytimer.h"

using namespace std;

string getRdmString(int maxLenth)
{
    //随机生成0-25的数字，对应26个字母
/*  for(int i=0; i<60; i++)
    {
        int num = rand()%26;
    //  cout<<num<<endl;
        printf("%c\n",num+65);  //大写字母 A-Z 65-90   小写字母 a-z 97-122
    }
    */
    //随机生成一个字母
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
//  cout<<rmdStr<<endl;

    delete[] tmp_str;
    return rmdStr;
}

int testTimer()
{
    Mytimer t1;
    t1.start();

    int z = 0;
    string tmp;
    for(int n = 0; n < 10000; n++)
    {
        tmp = getRdmString(5);
    //    cout << tmp << endl;
        if( "" == tmp)
            z++;
    }
    cout << "Time cost " << t1.stop() << "ms for compare with \"\"" << endl;

    t1.start();
    for(int n = 0; n < 10000; n++)
    {
        tmp = getRdmString(5);
    //    cout << tmp << endl;
        if( 0 == tmp.length())
            z++;
    }
    cout << "Time cost " << t1.stop() << "ms for compare with length" << endl;
    return 0;
}



