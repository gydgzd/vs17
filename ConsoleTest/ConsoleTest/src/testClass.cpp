#include "stdafx.h"
#include <iostream>
using namespace std;

class Base
{
public:
    Base() {};
    ~Base() {};

    virtual void print() { cout << "This is Base\n"; };
private:
    
};

class Derived : public Base
{
public:
    Derived() {};
    ~Derived() {};

    void print() { cout << "This is Derived\n"; };
private:

};


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
    p1();
    (emp1.*p2)();
	emp1.getCb();
	empSt.cf = 99;                         // �޷��������ⲿ���� "public: static int EmptyStruct::cf"  
	cout << (EmptyStruct::cf = 99) << endl;
	cout << "�����С�� " << sizeof(EmptyClass) << "ʵ�������С�� " << sizeof(emp1) << endl;
	cout << "�սṹ��С�� " << sizeof(EmptyStruct) << "ʵ�������С�� " << sizeof(empSt) << endl;
    // test dynamic_cast
    Base *pbase = new Derived();
    pbase->print();
    // ���ڡ�����ת�͡������������
    // һ���ǻ���ָ����ָ���������������͵ģ�����ת���ǰ�ȫ�ģ�
    // ��һ���ǻ���ָ����ָ����Ϊ�������ͣ������������dynamic_cast������ʱ����飬ת��ʧ�ܣ����ؽ��Ϊnullptr��

    //Derived * pderived = dynamic_cast<Derived *>(new Base()); // error:  new Base();  // static_cast works;
    Derived * pderived = dynamic_cast<Derived *>(pbase);
    if (pderived != nullptr)                                   // dynamic_cast may failed (����ʱdynamic_cast�Ĳ��������������̬������)
    {
        cout << "dynamic_cast successed" << endl;
        pderived->print();
    }
    else
        cout << "dynamic_cast failed" << endl;

    pderived = (Derived *)pbase;
    pderived->print();
    // 
    typedef void (Base::*fun)();
    fun myprint;
    myprint = &Base::print;
}
