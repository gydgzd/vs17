
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <unordered_set>
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
		msg1 = v2vMSG{ aa, ab };
	}

	v2vMSG msg1;
	v2vMSG msg2;
	unordered_set<v2vMSG, myHashFun, equalT> cmdSet;
	unordered_multiset<v2vMSG, myHashFun, equalT> cmdMSet;

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
	EXPECT_EQ(cmdSet.size(), 0);
	EXPECT_EQ(cmdMSet.size(), 0);
}

TEST_F(v2vMSGTest, insertSet) {
	v2vMSG v1("sub", "int");
	v2vMSG v2("mul", "int");
	v2vMSG v3("add", "int");
	v2vMSG v4("min", "int");
	v2vMSG v5("add", "double");
	cmdSet.insert(v1);
	cmdSet.insert(v2);
	cmdSet.insert(v3);
	cmdSet.insert(v4);
	cmdSet.insert(msg1);
	cmdSet.insert(v3);
	EXPECT_EQ(cmdSet.size(), 5);   // no repeated key allowed, so v3 only inserted once
	cmdSet.emplace(v5);
	EXPECT_EQ(cmdSet.size(), 6);

	for (auto &vmsg : cmdSet) {
		std::cout << vmsg.cmd << ": " << vmsg.subcmd << endl;
	}
	// find 
	auto iter = cmdSet.find(v1);
	EXPECT_STREQ(iter->cmd, "sub");
	EXPECT_STREQ(iter->subcmd, "int");
	// erase
	cmdSet.erase(v1);
	EXPECT_EQ(cmdSet.size(), 5);
	cmdSet.clear();
	EXPECT_EQ(cmdSet.size(), 0);
}

TEST_F(v2vMSGTest, insertMSet) {
	v2vMSG v1("sub", "int");
	v2vMSG v2("mul", "int");
	v2vMSG v3("add", "int");
	v2vMSG v4("min", "int");
	v2vMSG v5("add", "double");
	cmdMSet.insert(v1);
	cmdMSet.insert(v2);
	cmdMSet.insert(v3);
	cmdMSet.insert(v4);
	cmdMSet.insert(msg1);
	cmdMSet.insert(v3);
	EXPECT_EQ(cmdMSet.size(), 6);   // can have repeated key, v3 inserted twice
	cmdMSet.emplace(v5);
	EXPECT_EQ(cmdMSet.size(), 7);

	for (auto &vmsg : cmdMSet) {
		std::cout << vmsg.cmd << ": " << vmsg.subcmd << endl;
	}
	// find duplicate key, find() only returns the first one
	// method 1: use count() and find() to find all duplicates
	std::unordered_multiset<v2vMSG, myHashFun, equalT>::size_type count = cmdMSet.count(v3);
	std::unordered_multiset<v2vMSG, myHashFun, equalT>::iterator  iter1 = cmdMSet.find(v3);
	for (; count > 0; count--, iter1++)
	{
		printf("method1 found :%s  %s\n", iter1->cmd, iter1->subcmd);
	}

	// method 2: use equal_range
	auto range = cmdMSet.equal_range(v3);
	for (auto it = range.first; it != range.second; ++it) {
		printf("method2 found :%s  %s\n", it->cmd, it->subcmd);
	}

	// here can use tie	
	std::unordered_multiset<v2vMSG, myHashFun, equalT>::iterator  iterBegin, iterEnd;
	std::tie(iterBegin, iterEnd) = cmdMSet.equal_range(v3);
	while (iterBegin != iterEnd)
	{
		printf("method2 found :%s  %s\n", iterBegin->cmd, iterBegin->subcmd);
		iterBegin++;
	}

	auto iter = cmdMSet.find(v1);
	EXPECT_STREQ(iter->cmd, "sub");
	EXPECT_STREQ(iter->subcmd, "int");
	// erase
	cmdMSet.erase(v1);
	EXPECT_EQ(cmdMSet.size(), 6);
	cmdMSet.clear();
	EXPECT_EQ(cmdMSet.size(), 0);
}