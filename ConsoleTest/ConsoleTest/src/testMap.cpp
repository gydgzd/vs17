#include "stdafx.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

void testMap()
{
	std::map<string, string> argMap;
	std::map<string, string>::iterator myiter;

	argMap["2019-03-08 23:02:46:341685"].assign("hik");
	argMap["2019-03-08 23:02:46:611809"].assign("hs");
	argMap["2019-03-08 22:27:55:802280"].assign("hia");
	argMap["2019-03-08 22:27:55:80514"].assign("hib"); 
	argMap["2019-03-08 22:27:54:805778"].assign("hib"); 
	argMap.insert(std::pair<string, string>("-l","nice"));
	myiter = argMap.find("-k");

	if (myiter != argMap.end())
		std::cout << myiter->first << myiter->second << endl;
	for(auto iter = argMap.begin(); iter != argMap.end(); iter++)
		std::cout << iter->first << " " << iter->second << endl;
	return;
}