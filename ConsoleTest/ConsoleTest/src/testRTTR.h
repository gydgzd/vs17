#pragma once
#include <iostream>
#include <rttr/registration.h>
using namespace std;
using namespace rttr;

struct MyStruct 
{ 
    MyStruct() {}; 
    
    int data; 
    double func(double num)
    { 
        std::cout << num++ << std::endl;
        return num;
    };
};

class class2
{
    int data;
    void func(double num) { std::cout << num << std::endl; };
};
static void fun()
{
    cout << "hello fun" << endl;
}
extern const double pi;
extern std::string global_text;
extern void set_text(const std::string& text);
extern const std::string& get_text();
enum class MetaData_Type
{
    SCRIPTABLE,
    GUI
};
extern int g_value;
RTTR_REGISTRATION
{
    registration::class_<MyStruct>("MyStruct")
         .constructor<>()
         .property("data", &MyStruct::data)
         .method("func", &MyStruct::func);
    registration::method("hello", &fun);

    registration::property("value", &g_value)
    (
        metadata(MetaData_Type::SCRIPTABLE, false),
        metadata("Description", "This is a value."));
           
    registration::property_readonly("PI", &pi);
    registration::property("global_text", &get_text, &set_text);
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
        //
        variant value = type::get_property_value("PI"); // remark the capitalization of "PI"
        if (value && value.is_type<double>())
            std::cout << value.get_value<double>() << std::endl; // outputs: "3.14159"
        prop = type::get_global_property("PI");
        if (prop)
        {
            value = prop.get_value(instance());
            if (value.is_valid() && value.is_type<double>())
                std::cout << value.get_value<double>() << std::endl; // outputs: "3.14159"
        }
        property prop1 = type::get_global_property("global_text");
        if (prop1)
        {
            prop1.set_value(global_text, std::string("text"));           // std::string is necessary
            variant str_value = prop1.get_value(instance());
            if (str_value.is_valid() && str_value.is_type<string>())
                std::cout << str_value.get_value<string>() << std::endl; // 
        }
        type::set_property_value("global_text", std::string("text11"));  // std::string is necessary
        variant str_value = type::get_property_value("global_text") ;
        if (str_value.is_valid() && str_value.is_type<string>())
        {
            std::cout << str_value.get_value<string>() << std::endl; // 
        }
        // invoke method by rttr::method
        MyStruct obj1;
        variant ret;
        method meth = type::get(obj1).get_method("func");
        ret = meth.invoke(obj1, 42.0);
        if (ret.is_valid() && ret.is_type<double>())           // double is the return type of the method
            std::cout << ret.get_value<double>() << std::endl; 

        variant var = type::get(obj1).create();
        ret = meth.invoke(var, 42.0);
        if (ret.is_valid() && ret.is_type<double>())
        {
            std::cout << ret.get_value<double>() << std::endl;
        }
        meth = type::get_global_method("hello");
        if (meth) // check if the function was found
        {
            ret = meth.invoke({}, 0); // invoke with empty instance
            if (ret.is_valid() && ret.is_type<double>())
                std::cout << ret.get_value<double>() << std::endl;
        }
        // invoke by type 
        ret = type::invoke("hello", {});
        if (ret.is_valid() && ret.is_type<double>())
            std::cout << ret.get_value<double>() << std::endl;

        type mt = type::get_by_name("MyStruct");
        mt.invoke("func", obj, {223.0});
        // 
        prop = type::get_global_property("value");
        prop.set_value(g_value, 111);
        value = prop.get_metadata(MetaData_Type::SCRIPTABLE);
        
        std::cout << value.get_value<bool>() << endl; // prints "false"

        value = prop.get_metadata("Description");
        std::cout << value.get_value<std::string>() << endl; // prints "This is a value."
        
    }
private:
};

