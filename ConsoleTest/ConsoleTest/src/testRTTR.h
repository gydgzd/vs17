#pragma once
#include <iostream>
#include <rttr/registration.h>
using namespace std;
using namespace rttr;

struct MyStruct 
{ 
    MyStruct() {}; 
    
    int data; 
    void func(double num) { std::cout << num << std::endl; };
};

class class2
{
    int data;
    void func(double num) { std::cout << num << std::endl; };
};


RTTR_REGISTRATION
{
    registration::class_<MyStruct>("MyStruct")
         .constructor<>()
         .property("data", &MyStruct::data)
         .method("func", &MyStruct::func);
}

class testRTTR
{
public:
    void test()
    {
        // traversal by rttr::type : get_properties() , get_methods()
        rttr::type t = rttr::type::get<MyStruct>();
        for (auto& prop : t.get_properties())
            std::cout << "name: " << prop.get_name() << std::endl;

        for (auto& meth : t.get_methods())
            std::cout << "name: " << meth.get_name() << std::endl;

        // get and set value by rttr::property
        MyStruct obj;
        property prop = type::get(obj).get_property("data");
        prop.set_value(obj, 23);

        variant var_prop = prop.get_value(obj);
        std::cout << var_prop.to_int() << std::endl;; // prints '23'

        // invoke method by rttr::method
        MyStruct obj1;
        method meth = type::get(obj1).get_method("func");
        meth.invoke(obj1, 42.0);

        variant var = type::get(obj1).create();
        meth.invoke(var, 42.0);

        //

    }
private:
};

