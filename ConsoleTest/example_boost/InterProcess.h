#pragma once
#include <boost/interprocess/managed_shared_memory.hpp>
#include <cstdlib> //std::system
#include <sstream>
#include<windows.h>
class InterProcess
{
public:
    InterProcess();
    virtual ~InterProcess();

    int memPool(int argc);
    int namedShrMemory(int argc);

    int testSemapthore();
private:

};
