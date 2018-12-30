#include "stdafx.h"
#include <set>
#include <vector>
#include <iostream>

using namespace std;
int testSet()
{
	set<int> sdpair;
	sdpair.insert(2);
	sdpair.insert(3);
	sdpair.insert(4);
	sdpair.insert(6);

	set<int> sdpair1;
	sdpair1.insert(6);
	sdpair1.insert(3);
	sdpair1.insert(4);
	sdpair1.insert(2);
	if (sdpair == sdpair1)
		cout << "same" << endl;
	set<int>::iterator ipos;
	ipos = sdpair.find(4);
	if (ipos != sdpair.end())
		cout << "found." << endl;
	set<set<int>> ss;
	ss.insert(sdpair);
	ss.insert(sdpair1);
	set<int> sdpair2;
	sdpair2.insert(2);
	sdpair2.insert(6);
	sdpair2.insert(4);
	sdpair2.insert(3);
	sdpair2.erase(3);
	set<set<int>>::iterator ipos1;
	ipos1 = ss.find(sdpair2);
	if (ipos1 != ss.end())
		cout << "found." << endl;
	return 0;
}