#include "stdafx.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

void testMap(int argc, _TCHAR* argv[])
{
	std::vector<string> args;
	for (int i = 0; i < argc; i++)
	{
		cout << argv[i];
	}
	std::map<string, string> argMap;
	std::map<string, string>::iterator myiter;

	argMap["-k"].assign("hiphop");
	argMap.insert(std::pair<string, string>("-l","nice"));
	myiter = argMap.find("-k");

	if (myiter != argMap.end())
		std::cout << myiter->first << myiter->second << endl;
	return;
}