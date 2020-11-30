/*
 * testUsing.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: gyd
 */
#include <stdio.h>
#include <map>
#include <string>
namespace Test
{
    int print(int n)
    {
        printf("In namespace Test: %d\n", n);
        return 0;
    }
}
int print(double n)
{
    printf("  %f\n", n);
    return 0;
}
using mymap = std::map<std::string, std::string>;
void myprint()
{
    mymap aa;
 //   using namespace Test;
    aa["hello"] = "world";
    printf("%s\n", aa["hello"].c_str());
    print(3);
}
