// test_class.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "base.h"
#include "derive.h"
#include <iostream>

// 根据平台使用合适的函数指针类型
#if defined(_M_X64) || defined(__x86_64__)
typedef int(*Fun)(Base1*);        // x64: this 通过第一个参数传递（RCX）
#else
typedef int(__thiscall* Fun)(Base1*); // x86 MSVC: __thiscall 隐性 this
#endif

int main()
{
    Base b;
    b.get();
    b.set(3);
    std::cout << &b << std::endl;


    Base1 b1;
	b1.get();
	b1.set(1);
	std::cout << &b1 << std::endl;
    void** vptr = *(void***)&b1;
    Fun fun1 = (Fun)vptr[0];
    fun1(&b1);

    Derive d;
    d.get();
    d.set(2);

	Base* pb1 = &d;
    pb1->get();
    pb1->set(4);

    Derive1 d1;
    d1.get();
    d1.set(3);
    d1.print();


    Base1* pb2 = &d1;
    pb2->get();
    pb2->set(5);    

    Base2* pb3 = &d1;
    pb3->get();
    pb3->set(5);
	pb3->print();
    return 0;
}

