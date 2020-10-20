
#include <unordered_map>
#include <map>
#include <string>
#include <functional> 
using namespace std;
#include <iostream>
struct v2vMSG
{
	char cmd[8];
	char subcmd[8];
/*	char srcMAC[16];
	char dstMAC[16];
	char srcVMSaddr[16];
	char dstVMSaddr[16];
	char datetime[32];
	int sec;
	int usec;
	int hashVal1;
	int hashVal2;
*/	v2vMSG()
	{
		memset(this, 0, sizeof(*this));
	}
	v2vMSG(char *cmd, char *subcmd)
	{
		strcpy(this->cmd, cmd);
		strcpy(this->subcmd, subcmd);
	}
	bool operator==(const v2vMSG * p) const
	{
		if (!strcmp(this->cmd, p->cmd) && !strcmp(this->subcmd, p->subcmd))
			return true;
		return false;
	}
//	const int operator[](const v2vMSG &) const;
};
struct equalT
{
	bool operator () (const v2vMSG &lhs, const v2vMSG &rhs) const
	{
		if (!strcmp(lhs.cmd, rhs.cmd) && !strcmp(lhs.subcmd, rhs.subcmd))
			return true;
		return false;
	}
};
struct myHashFun
{
	
	std::size_t operator() (const v2vMSG pmsg) const
	{
		char total[128] = "";
		int len = 0;
		memcpy(total + len, pmsg.cmd, sizeof(pmsg.cmd));    len += sizeof(pmsg.cmd);
		memcpy(total + len, pmsg.subcmd, sizeof(pmsg.subcmd)); len += sizeof(pmsg.subcmd);
		/*		memcpy(total + len, pmsg->dstMAC, sizeof(pmsg->dstMAC)); len += sizeof(pmsg->dstMAC);
		memcpy(total + len, pmsg->srcMAC, sizeof(pmsg->srcMAC)); len += sizeof(pmsg->srcMAC);
		memcpy(total + len, pmsg->dstVMSaddr, sizeof(pmsg->dstVMSaddr)); len += sizeof(pmsg->dstVMSaddr);
		memcpy(total + len, pmsg->srcVMSaddr, sizeof(pmsg->srcVMSaddr)); len += sizeof(pmsg->srcVMSaddr);
		*/
		return hash<string>()(total);
	}
};
/*
namespace std
{   // 重写模板
	template <>
	class hash<v2vMSG>
	{
	public:	std::size_t operator()(const v2vMSG &key) const
		{
			using std::size_t;
			using std::hash;

			// Compute individual hash values for first,
			// second and third and combine them using XOR
			// and bit shifting:

			return ((hash<string>()(key.cmd)
				^ (hash<string>()(key.subcmd) << 1)) >> 1);
			//	^ (hash<int>()(key.third) << 1);
		}
	};
	template <>
	class equal_to<v2vMSG>
	{
	public:	bool operator () (const v2vMSG &lhs, const v2vMSG &rhs) const
		{
			if (!strcmp(lhs.cmd, rhs.cmd) && !strcmp(lhs.subcmd, rhs.subcmd))
				return true;
			return false;
		}
	};

}*/
std::size_t myhashfun(const v2vMSG &key)
{
	using std::size_t;
	using std::hash;

	// Compute individual hash values for first,
	// second and third and combine them using XOR
	// and bit shifting:

	return ((hash<string>()(key.cmd)
		^ (hash<string>()(key.subcmd) << 1)) >> 1);
	//	^ (hash<int>()(key.third) << 1);
}
struct v2vCMD
{
	char cmd[8];
	char subcmd[8];
	bool operator<(const v2vCMD &vcmd) const
	{
		return (cmd < vcmd.cmd || (!(vcmd.cmd < cmd) && subcmd < vcmd.subcmd));
	}

};
struct compareCMD
{
	bool operator () (const v2vCMD &lhs, const v2vCMD &rhs) const
	{		
		if (!(0 == strcmp(lhs.cmd, rhs.cmd) && 0 == strcmp(lhs.subcmd, rhs.subcmd)))
			return lhs < rhs;
		return false;
	}
};
int testHashMap()
{
	cout << "aa,bb " << strcmp("aa", "bb") << "bb,aa" << strcmp("bb", "aa") << endl;
	int aa = strcmp("a", "c");
	if (aa)
		cout << "true" << endl;
	v2vCMD cmd1,cmd2;
	strcpy(cmd1.cmd, "8a01");
	strcpy(cmd1.subcmd, "0000");
	map<v2vCMD, std::string, compareCMD> myvmap;
	myvmap.insert(pair<v2vCMD,string>(cmd1, "开始") );
	strcpy(cmd2.cmd, "8a23");
	strcpy(cmd2.subcmd, "0001");
	myvmap.insert(pair<v2vCMD, string>(cmd2, "结束"));
	cout << myvmap[cmd1] << endl;
	//
	unordered_map<string, int> mymap;
	string name = "Lisa";
	mymap.insert(pair<string, int>(name, 28));
	name = "anna";
	mymap.insert(pair<string, int>(name, 23));
	name = "NICK";
	mymap.insert(pair<string, int>(name, 18));
	name = "koko";
	mymap.insert(pair<string, int>(name, 19));
	mymap.insert(pair<string, int>(name, 21));
	mymap[name] = 21;
	std::unordered_map<string, int>::iterator mapiterator;
	for (mapiterator = mymap.begin(); mapiterator != mymap.end(); mapiterator++)
		printf("%s, %d \n", mapiterator->first.c_str(), mapiterator->second);
	printf("%d\n", mymap["Lisa"]);
	// multimap
	multimap<string, int> mmap;
	mmap.insert(pair<string, int>("Lisa", 28));
	mmap.insert(pair<string, int>("Anna", 23));
	mmap.insert(pair<string, int>("Nick", 18));
	mmap.insert(pair<string, int>("Nick", 19));
	std::multimap<string, int>::iterator mmapiterator;
	for (mmapiterator = mmap.begin(); mmapiterator != mmap.end(); mmapiterator++)
		printf("%s, %d \n", mmapiterator->first.c_str(), mmapiterator->second);
//	printf("%d\n", mmap["Lisa"]);
	printf("\n");
	//unordered_multimap
	unordered_multimap<string, int> mymtmap;
	mymtmap.insert(pair<string, int>("Lisa", 28));
	mymtmap.insert(pair<string, int>("Anna", 23));
	mymtmap.insert(pair<string, int>("Nick", 18));
	mymtmap.insert(pair<string, int>("Nick", 19));
	mymtmap.insert(pair<string, int>("Nick", 1));
	std::unordered_multimap<string, int>::iterator mtmapiterator;
	for (mtmapiterator = mymtmap.begin(); mtmapiterator != mymtmap.end(); mtmapiterator++)
	{
		printf("%s, %d \n", mtmapiterator->first.c_str(), mtmapiterator->second);
	}
	printf("\n");
	unordered_multimap<int, int> mymtmap_num; 
	mymtmap_num.insert(mymtmap_num.begin(),pair<int, int>(5, 2));
	mymtmap_num.insert(mymtmap_num.begin(),pair<int, int>(4, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(3, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(1, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(2, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(3, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(4, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(5, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(6, 2));
	mymtmap_num.insert(mymtmap_num.begin(), pair<int, int>(7, 2));
	std::unordered_multimap<int, int>::iterator mtmap_num_iter;
	for (size_t n = mymtmap_num.size(); n > 0; n--)   //逆序输出
	{
		mtmap_num_iter = mymtmap_num.begin();    
		for (unsigned int i = 0; i < n - 1; i++)
			mtmap_num_iter++;
		printf("%d, %d \n", mtmap_num_iter->first, mtmap_num_iter->second);
	}
	for (mtmap_num_iter = mymtmap_num.begin(); mtmap_num_iter != mymtmap_num.end(); mtmap_num_iter++)
	{
		printf("%d, %d \n", mtmap_num_iter->first, mtmap_num_iter->second);
	}
	printf("\n");
	// 查找多个重复值
	// 方法1
	std::unordered_multimap<string, int>::size_type count = mymtmap.count("Nick");
	std::unordered_multimap<string, int>::iterator  iter = mymtmap.find("Nick");
	for (; count > 0; count--, iter++)
	{
		printf("found :%s  %d\n", iter->first.c_str() , iter->second );
	}
	// 方法2
	std::unordered_multimap<string, int>::iterator  iterBegin = mymtmap.lower_bound("Nick");
	std::unordered_multimap<string, int>::iterator  iterEnd = mymtmap.upper_bound("Nick");
	for (; iterBegin != iterEnd; iterBegin++)
	{
		printf("found :%s  %d\n", iterBegin->first.c_str(), iterBegin->second);
	}
	// 方法3
	auto ret = mymtmap.equal_range("Nick");
	auto it = ret.first;
	while (it != ret.second)
	{
		printf("found :%s  %d\n", it->first.c_str(), it->second);
		++it;
	}
	//// self define type
	v2vMSG msg("8e01", "1111");
	v2vMSG msg1("8e02", "2222");
	v2vMSG msg2("8e03", "3333");
//	printf("Hashval = %d\n",myHashFun()(&msg));
	// 方法一 使用自定义方法
	unordered_multimap<v2vMSG, std::string, myHashFun, equalT> myhash;
	myhash.insert(std::pair<v2vMSG, std::string>(msg, "aa"));
	myhash.insert(std::pair<v2vMSG, std::string>(msg1, "bb"));
	myhash.insert(std::pair<v2vMSG, std::string>(msg2, "cc"));
	auto iters = myhash.begin();
	for(; iters != myhash.end(); iters++)
		printf("%s %s ", iters->first.cmd, iters->second.c_str());
	for (iters = myhash.begin(); iters != myhash.end(); )
		if (!strcmp(iters->first.cmd, "8e01"))
			myhash.erase(iters++);    // erase 后迭代器失效，无法继续++操作，运行崩溃
		else
			iters++;
	// 方法二 重写模板方法
/*	unordered_multimap<v2vMSG, std::string> myOverHash;
	myOverHash.insert(std::pair<v2vMSG, std::string>(msg, "bbaa"));
	auto iter2 = myOverHash.begin();
	printf("%s %s \n", iter2->first.cmd, iter2->second.c_str());
	*/
//	myhash[msg] = "1231";
//	printf("%d\n", mymtmap["Lisa"]);
    printf("\n");
	return 0;
}
