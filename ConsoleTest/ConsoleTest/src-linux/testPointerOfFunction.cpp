/*
 * testPointerOfFunction.cpp
 *
 *  Created on: May 11, 2019
 *      Author: gyd
 */

#include <iostream>
template <class T>
int max(T a, T b)
{
    if(a > b)
        return a;
    else
        return b;
}

int testPointerOfFunction()
{
    int (*pfun)(int , int )  = max;
    std::cout << pfun(3,6) << std::endl;
    //

    return 0;
}







