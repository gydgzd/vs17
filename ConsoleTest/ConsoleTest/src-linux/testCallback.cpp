/*
 * testCallback.cpp
 *
 *  Created on: May 15, 2019
 *      Author: gyd
 */
#include <iostream>

int callback1(int x)
{
    std::cout << "This is callback1 with param:" << x << std::endl ;
    return 0;
}

int callback2(int x)
{
    std::cout << "This is callback2 with param:" << x << std::endl ;
    return 0;
}

int Handle(int x, int (*Callback)(int))
{
    std::cout << "Entering Handle Function. " << std::endl;
    Callback(x);
    std::cout << "Leaving Handle Function. " << std::endl;
    return 0;
}
int testCallback()
{
    int ret = Handle(4,callback1);

    return (ret == 0);
}


template < class Class, typename ReturnType, typename Parameter >
class SingularCallBack
{
public:
    typedef ReturnType (Class::*Method)(Parameter);
    SingularCallBack(Class* _class_instance, Method _method)
    {
       //取得对象实例地址,及调用方法地址
       class_instance = _class_instance;
       method        = _method;
    };
    ReturnType operator()(Parameter parameter)
    {
       // 调用对象方法
       return (class_instance->*method)(parameter);
    };
    ReturnType execute(Parameter parameter)
    {
       // 调用对象方法
       return operator()(parameter);
    };

   private:
    Class*  class_instance;
    Method  method;
};

class A
{
public:
    void output()
    {
        std::cout << "This is class A" << std::endl;
    }
};
class B
{
public:
    bool methodB(A a)
    {
        a.output();
        return true;
    }
};

void testCPPCallback()
{
    A a;
    B b;
    SingularCallBack<B, bool, A> *cp;
    cp = new SingularCallBack<B, bool, A>(&b, &B::methodB);
    if(cp->execute(a))
    {
        std::cout << "Call back successfully!" << std::endl;
    }else
    {
        std::cout << "Call back unsuccessfully!" << std::endl;
    }
    delete cp;
}

