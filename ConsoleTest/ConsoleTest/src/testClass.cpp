#include "stdafx.h"
#include <iostream>
using namespace std;

class Base
{
public:
    Base() { cout << "Base constructor\n"; };
    virtual ~Base() { cout << "Base destructor\n"; };

    virtual void print() { cout << "This is Base\n"; };
private:
    
};

class Derived : public Base
{
public:
    Derived() 
    { 
        m_buff = nullptr;
        cout << "Derived constructor\n"; 
    };
    Derived(char* buff)
    {
        m_buff = buff;
        cout << "Derived constructor\n";
    };
    Derived(Derived& obj) 
    { 
        m_buff = nullptr;
        if (obj.m_buff != nullptr)
        {
            int len = strlen(obj.m_buff);
            m_buff = new char[len + 1];
            memset(m_buff, 0, len + 1);
            memcpy(m_buff, obj.m_buff, len);
        }
        cout << "Copy constructor\n"; 
    };
    Derived(Derived&& obj)
    {
        m_buff = obj.m_buff;
        obj.m_buff = nullptr;
        cout << "move constructor\n";
    };
    Derived& operator=(Derived& obj)
    {
        m_buff = nullptr;
        if (obj.m_buff != nullptr)
        {
            int len = strlen(obj.m_buff);
            m_buff = new char[len + 1];
            memset(m_buff, 0, len + 1);
            memcpy(m_buff, obj.m_buff, len);
        }
        cout << "= constructor\n";
        return *this;
    }

    ~Derived() 
    { 
        if (m_buff != nullptr)
        {
            delete m_buff;
            m_buff = nullptr;
        }
        cout << "Derived destructor\n"; 
    };

    void print() { cout << this << ": This is Derived\n"; };
    void derivefun() { cout << this << endl; };

private:
    char *m_buff;
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
    Derived *db = new Derived();
    db->derivefun();
    db->print();
    Derived dc(*db);
    dc.derivefun();
    dc.print();
    // std::move produces an xvalue expression that identifies its argument. 
    // It is exactly equivalent to a static_cast to an rvalue reference type
    // see https://en.cppreference.com/w/cpp/utility/move
    Derived dd = std::move(Derived(new char[32]));  
    dd.derivefun();
    dd.print();
//    cout << "The address of dd.print()=" << dd.print() << endl;
    Derived de = Derived(new char[32]);
//    cout << "The address of de.print()=" << de.print() << endl;
    dd = Derived(new char[32]);
    delete db;
    // test dynamic_cast
    Base *pbase = new Derived();
    pbase->print();
    // 对于“向下转型”有两种情况。
    // 一种是基类指针所指对象是派生类类型的，这种转换是安全的；
    // 另一种是基类指针所指对象为基类类型，在这种情况下dynamic_cast在运行时做检查，转换失败，返回结果为nullptr;
    // static_cast下行转换时不做动态类型检查；不能转换掉const、volitale、或者__unaligned属性

    Derived * pderived1 = dynamic_cast<Derived *>(new Base()); // error:  new Base();  // static_cast works;
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

    delete pbase;
    pbase = new Base();
    Derived *  pd = new Derived();
    pd = dynamic_cast<Derived *>(pbase);
    pbase = pd;
    // 
    typedef void (Base::*fun)();
    fun myprint;
    myprint = &Base::print;
}
