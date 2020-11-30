/*
 * testMap.cpp
 *
 *  Created on: Feb 25, 2019
 *      Author: gyd
 */

#include <map>
#include <queue>
#include <list>

using namespace std;
int testMap()
{
    list<string> mylist;
    mylist.insert(mylist.end(), "2018-02-11");
    mylist.insert(mylist.end(), "2018-02-12");
    mylist.insert(mylist.end(), "2018-02-13");
    mylist.push_back("2018-02-15");
    list<string> mylist1;
    mylist1.insert(mylist1.end(), "2018-02-12");
    mylist1.insert(mylist1.end(), "2018-02-13");
    mylist1.insert(mylist1.end(), "2018-02-14");

    list<string> mylist2;
    mylist2.insert(mylist2.end(), "2018-02-12");
    mylist2.insert(mylist2.end(), "2018-02-12");
    mylist2.insert(mylist2.end(), "2018-02-15");

    list<string> mylist3;
    mylist3.insert(mylist3.end(), "2018-02-13");
    mylist3.insert(mylist3.end(), "2018-02-14");
    list<string> mylist4;
    mylist4.insert(mylist4.end(), "2018-02-15");
    mylist4.insert(mylist4.end(), "2018-02-15");
    mylist4.insert(mylist4.end(), "2018-02-15");
    list<string> mylist5;
    mylist5.insert(mylist5.end(), "2018-02-16");
    mylist5.insert(mylist5.end(), "2018-02-15");
    list<string> mylist6;
    mylist6.insert(mylist6.end(), "2018-02-17");
    map<string,list<string>>    mymaplist;
    mymaplist.insert(pair<string,list<string>>(*mylist.begin(),mylist));
    mymaplist.insert(pair<string,list<string>>(*mylist4.begin(),mylist1));
    mymaplist.insert(pair<string,list<string>>(*mylist5.begin(),mylist2));
    mymaplist.insert(pair<string,list<string>>(*mylist6.begin(),mylist3));
    mymaplist.insert(pair<string,list<string>>(*mylist1.begin(),mylist1));
    mymaplist.insert(pair<string,list<string>>(*mylist2.begin(),mylist2));
    mymaplist.insert(pair<string,list<string>>(*mylist3.begin(),mylist3));

    for(auto iter = mymaplist.rbegin(); iter != mymaplist.rend(); iter++)
    {
        printf("%s\n", iter->first.c_str());
    }
    return 0;
}


