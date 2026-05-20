
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <functional> 
#include "gtest/gtest.h"

using namespace std;

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
	*/	
	v2vMSG() {
		memset(this, 0, sizeof(*this));
	}
	v2vMSG(const char* cmd, const char* subcmd) {
		memset(this, 0, sizeof(*this));
		strncpy(this->cmd, cmd, 7);
		strncpy(this->subcmd, subcmd, 7);
	}
	bool operator==(const v2vMSG& p) const {
		if (!strcmp(this->cmd, p.cmd) && !strcmp(this->subcmd, p.subcmd))
			return true;
		return false;
	}
	//	const int operator[](const v2vMSG &) const;
};

// 自定义类型需要重写hash和equal_to模板，或者直接使用自定义的hash函数和equal_to函数对象
struct equalT
{
	bool operator () (const v2vMSG& lhs, const v2vMSG& rhs) const
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
		memcpy(total + len, pmsg.cmd, sizeof(pmsg.cmd));    
		len += sizeof(pmsg.cmd);
		memcpy(total + len, pmsg.subcmd, sizeof(pmsg.subcmd)); 
		len += sizeof(pmsg.subcmd);
		return hash<string>()(total);
	}
};

/*  重写模板
namespace std
{  
	template <>
	class hash<v2vMSG>
	{
	public:	std::size_t operator()(const v2vMSG &key) const
		{
			using std::size_t;
			using std::hash;

			// Compute hash value.
			return ((hash<string>()(key.cmd)
				^ (hash<string>()(key.subcmd) << 1)) >> 1);
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

class v2vMSGTest : public testing::Test {
protected:
	v2vMSGTest() {
		char aa[8] = "add";
		char ab[8] = "float";
		msg1 = v2vMSG{aa, ab};
	}

	v2vMSG msg1; 
	v2vMSG msg2;
	unordered_map<v2vMSG, std::string, myHashFun, equalT> cmdMap;
	unordered_multimap<v2vMSG, std::string, myHashFun, equalT> cmdMMap;

};

TEST_F(v2vMSGTest, IsEmptyInitially) {
	EXPECT_STREQ(msg1.cmd, "add");
	EXPECT_STREQ(msg1.subcmd, "float");

	EXPECT_STREQ(msg2.cmd, "");
	EXPECT_STREQ(msg2.subcmd, "");

	strcpy(msg2.cmd, "add");
	strcpy(msg2.subcmd, "int");
	EXPECT_STREQ(msg2.cmd, "add");
	EXPECT_STREQ(msg2.subcmd, "int");
	EXPECT_EQ(cmdMap.size(), 0);
}

TEST_F(v2vMSGTest, insertMap) {
	v2vMSG v1("sub", "int");
	v2vMSG v2("mul", "int");
	v2vMSG v3("add", "int");
	v2vMSG v4("min", "int");
	cmdMap.insert(std::pair<v2vMSG, std::string>(v1, v1.cmd));
	cmdMap.insert(std::pair<v2vMSG, std::string>(v2, v2.cmd));
	cmdMap.insert(std::pair<v2vMSG, std::string>(v3, v3.cmd));
	cmdMap.insert(std::pair<v2vMSG, std::string>(v4, v4.cmd));
	cmdMap.insert(std::pair<v2vMSG, std::string>(msg1, msg1.cmd));
	cmdMap.insert(std::pair<v2vMSG, std::string>(v3, v3.cmd));
	EXPECT_EQ(cmdMap.size(), 5);   // no repeated key allowed, so v3 only inserted once
	v2vMSG v5("add", "double");
	cmdMap.emplace(v5, v5.cmd);
	EXPECT_EQ(cmdMap.size(), 6);

	for (auto vmsg : cmdMap) {
		std::cout << vmsg.first.cmd << ": " << vmsg.first.subcmd << endl;
	}
	// find 
	auto iter = cmdMap.find(v1);
	EXPECT_STREQ(iter->first.cmd, "sub");
	EXPECT_STREQ(iter->first.subcmd, "int");
	// erase
	cmdMap.erase(v1);
	EXPECT_EQ(cmdMap.size(), 5);
	cmdMap.clear();
	EXPECT_EQ(cmdMap.size(), 0);
}

TEST_F(v2vMSGTest, insertMMap) {
	v2vMSG v1("sub", "int");
	v2vMSG v2("mul", "int");
	v2vMSG v3("add", "int");
	v2vMSG v4("min", "int");
	cmdMMap.insert(std::pair<v2vMSG, std::string>(v1, v1.cmd));
	cmdMMap.insert(std::pair<v2vMSG, std::string>(v2, v2.cmd));
	cmdMMap.insert(std::pair<v2vMSG, std::string>(v3, v3.cmd));
	cmdMMap.insert(std::pair<v2vMSG, std::string>(v4, v4.cmd));
	cmdMMap.insert(std::pair<v2vMSG, std::string>(msg1, msg1.cmd));
	cmdMMap.insert(std::pair<v2vMSG, std::string>(v3, "v3_dup"));
	EXPECT_EQ(cmdMMap.size(), 6);   // can have repeated key, v3 inserted twice
	v2vMSG v5("add", "double");
	cmdMMap.emplace(v5, v5.cmd);
	EXPECT_EQ(cmdMMap.size(), 7);

	for (auto vmsg : cmdMMap) {
		std::cout << vmsg.first.cmd << ": " << vmsg.first.subcmd << endl;
	}
	// find duplicate key, find() only returns the first one
	// method 1: use count() and find() to find all duplicates
	std::unordered_multimap<v2vMSG, std::string>::size_type count = cmdMMap.count(v3);
	std::unordered_multimap<v2vMSG, std::string>::iterator  iter1 = cmdMMap.find(v3);
	for (; count > 0; count--, iter1++)
	{
		printf("method1 found :%s  %s\n", iter1->first.cmd, iter1->second.c_str());
	}

	// method 2: use equal_range
	auto range = cmdMMap.equal_range(v3);
	for (auto it = range.first; it != range.second; ++it) {
		printf("method2 found :%s  %s\n", it->first.cmd, it->second.c_str());
	}

	// here can use tie	
	std::unordered_multimap<v2vMSG, std::string>::iterator  iterBegin, iterEnd;
	std::tie(iterBegin, iterEnd) = cmdMMap.equal_range(v3);
	while (iterBegin != iterEnd)
	{
		printf("method2 found :%s  %s\n", iterBegin->first.cmd, iterBegin->second.c_str());
		iterBegin++;
	}

	auto iter = cmdMMap.find(v1);
	EXPECT_STREQ(iter->first.cmd, "sub");
	EXPECT_STREQ(iter->first.subcmd, "int");
	// erase
	cmdMMap.erase(v1);
	EXPECT_EQ(cmdMMap.size(), 6);
	cmdMMap.clear();
	EXPECT_EQ(cmdMMap.size(), 0);
}