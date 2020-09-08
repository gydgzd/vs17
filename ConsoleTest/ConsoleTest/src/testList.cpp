#include <list>
#include <forward_list>
#include <string>
#include <iostream>
using namespace std;

int testList()
{
    forward_list<string> for_list;
    for_list.emplace_front("hello");
    for_list.emplace_after(for_list.begin(), "pretty");
    for_list.emplace_after(for_list.begin(), "nice");

	list<list<string>> mylist;

	list<string> ls1;
	list<string> ls2;
	ls1.push_back("ho");
	mylist.push_back(ls1);
	list<list<string>>::iterator iterll = mylist.begin();
	iterll->insert(iterll->end(), "gyd");
	iterll->insert(iterll->end(), "hello");
	iterll->insert(iterll->end(), "world");
	iterll->insert(iterll->end(), "I'm");
	iterll->insert(iterll->end(), "comming");
    ls2.push_front("begin");
	ls2.push_back("gzd");
	ls2.push_back("ljr");
	mylist.push_back(ls2);

	list<string>::iterator iter;           // auto iter
	list<string>::reverse_iterator iter2 = ls1.rbegin();
	for (iterll = mylist.begin(); iterll != mylist.end(); iterll++)
	{
		for (iter = iterll->begin(); iter != iterll->end(); iter++)
			printf("%s--", iter->c_str());
		printf("\n");
	}
/*
	for (iterll = mylist.begin(); iterll != mylist.end(); iterll++)
	{
		for (iter = iterll->begin(); iter != iterll->end(); iter++)
			if (!strcmp(iter->c_str(), "gyd"))
			{
				iterll->erase(iter);      // erase 后迭代器失效，无法继续++操作，运行崩溃
			}
		printf("\n");
	}
*/	
	for (iterll = mylist.begin(); iterll != mylist.end(); iterll++)
	{
		for (iter = iterll->begin(); iter != iterll->end(); )
			if (!strcmp(iter->c_str(), "gyd"))
			{
				iterll->erase(iter++);      
			}
			else
				iter++;

		printf("\n");
	}
	return 0;
}