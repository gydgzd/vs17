#include "stdafx.h"
#include <iostream>
using namespace std;

class EmptyClass
{
public:
	int ca;
	int cb;
	int cc;
	static int cd;
	virtual void setCa() { ca = 5; };    // sizeof(EmptyClass)  ���麯��ռ��4B
	virtual void setCb() { cb = 5; };
	EmptyClass(int x1, int x2)
	{
		ca = x1;
		cb = x2;

	};

	static EmptyClass fb;
	int getCa()
	{
		cout << ca << endl;
		return 0;
	}
	static int getCb()
	{
		cout << cd << endl;
		return 0;
	}
};
int EmptyClass::cd = 6;
struct EmptyStruct
{
	int cd;
	int ce;
	static int cf;
};
int EmptyStruct::cf;
void testClass()
{
	EmptyClass emp1(2, 5);
	EmptyStruct empSt = { 8,9 };
	int(*p1)() = &EmptyClass::getCb;  //
	int (EmptyClass::*p2)() = &EmptyClass::getCa;  // ������ַ���Ͳ�ͬ
	emp1.getCb();
	empSt.cf = 99;                         // �޷��������ⲿ���� "public: static int EmptyStruct::cf"  
	cout << (EmptyStruct::cf = 99) << endl;
	cout << "�����С�� " << sizeof(EmptyClass) << "ʵ�������С�� " << sizeof(emp1) << endl;
	cout << "�սṹ��С�� " << sizeof(EmptyStruct) << "ʵ�������С�� " << sizeof(empSt) << endl;
}