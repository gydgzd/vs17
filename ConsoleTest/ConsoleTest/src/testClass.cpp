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
	virtual void setCa() { ca = 5; };    // sizeof(EmptyClass)  中虚函数占用4B
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
	int (EmptyClass::*p2)() = &EmptyClass::getCa;  // 函数地址类型不同
    p1();
    (emp1.*p2)();
	emp1.getCb();
	empSt.cf = 99;                         // 无法解析的外部符号 "public: static int EmptyStruct::cf"  
	cout << (EmptyStruct::cf = 99) << endl;
	cout << "空类大小： " << sizeof(EmptyClass) << "实例化后大小： " << sizeof(emp1) << endl;
	cout << "空结构大小： " << sizeof(EmptyStruct) << "实例化后大小： " << sizeof(empSt) << endl;
    // test dynamic_cast
    Base *pbase = new Derived();
    pbase->print();
    // 对于“向下转型”有两种情况。
    // 一种是基类指针所指对象是派生类类型的，这种转换是安全的；
    // 另一种是基类指针所指对象为基类类型，在这种情况下dynamic_cast在运行时做检查，转换失败，返回结果为nullptr；

    //Derived * pderived = dynamic_cast<Derived *>(new Base()); // error:  new Base();  // static_cast works;
    Derived * pderived = dynamic_cast<Derived *>(pbase);
    if (pderived != nullptr)                                   // dynamic_cast may failed (运行时dynamic_cast的操作数必须包含多态类类型)
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
