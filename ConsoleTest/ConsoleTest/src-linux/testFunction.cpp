/*
 * testFunction.cpp
 *
 *  Created on: May 11, 2019
 *      Author: gyd
 */
#include <iostream>
#include <functional>
using namespace std;

float add(int a, float b)
{
    return a+b;
}
//template<class T>
int runFunc(int a, float b, std::function<float (int a, float b)> plus )
{
   return  plus(a, b);
}


void testFunction()
{
    try
    {
        std::function<float (int, float)> plus = [](int a, float b){return add(a,b);} ;
        cout << plus(2,3) << endl;
        std::cout << runFunc( 2, 3, plus) << std::endl;;

    }catch(std::bad_function_call &e)
    {
        std::cout << e.what() << std::endl;
    }

}

