#pragma once
#include <boost/interprocess/managed_shared_memory.hpp>
#include <cstdlib> //std::system
#include <sstream>

class InterProcess
{
public:
    InterProcess();
    InterProcess();

    int memPool(int argc);

private:

};

