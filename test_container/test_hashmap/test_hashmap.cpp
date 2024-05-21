
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
	v2vMSG(char* cmd, char* subcmd) {
		strcpy(this->cmd, cmd);
		strcpy(this->subcmd, subcmd);
	}
	bool operator==(const v2vMSG* p) const {
		if (!strcmp(this->cmd, p->cmd) && !strcmp(this->subcmd, p->subcmd))
			return true;
		return false;
	}
	//	const int operator[](const v2vMSG &) const;
};
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

class v2vMSGTest : public testing::Test {
protected:
	v2vMSGTest() {
		char aa[8] = "gg";
		char ab[8] = "test";
		msg1 = v2vMSG{aa, aa};
	}

	v2vMSG msg1; 
	v2vMSG msg2;
};

TEST_F(v2vMSGTest, IsEmptyInitially) {
	EXPECT_STREQ(msg1.cmd, "gg");
	EXPECT_STREQ(msg2.cmd, "");
}