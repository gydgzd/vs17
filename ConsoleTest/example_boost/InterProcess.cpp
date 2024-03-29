#include "stdafx.h"
#include "InterProcess.h"

InterProcess::InterProcess()
{
}

InterProcess::~InterProcess()
{
}

int InterProcess::memPool(int argc)
{
    using namespace boost::interprocess;
    if (argc == 0) {  //Parent process
       //Remove shared memory on construction and destruction
        struct shm_remove
        {
            shm_remove() { shared_memory_object::remove("MySharedMemory"); }
            ~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        } remover;

        //Create a managed shared memory segment
        managed_shared_memory segment(create_only, "MySharedMemory", 65536);

        //Allocate a portion of the segment (raw memory)
        managed_shared_memory::size_type free_memory = segment.get_free_memory();
        void * shptr = segment.allocate(1024/*bytes to allocate*/);

        //Check invariant
        if (free_memory <= segment.get_free_memory())
            return 1;

        //An handle from the base address can identify any byte of the shared
        //memory segment even if it is mapped in different base addresses
        managed_shared_memory::handle_t handle = segment.get_handle_from_address(shptr);
        std::stringstream s;
        s << " handle: " << handle;
        s << std::ends;
        //Launch child process
        if (0 != std::system(s.str().c_str()))
            return 1;
        //Check memory has been freed
        if (free_memory != segment.get_free_memory())
            return 1;
    }
    else {
        //Open managed segment
        managed_shared_memory segment(open_only, "MySharedMemory");

        //An handle from the base address can identify any byte of the shared
        //memory segment even if it is mapped in different base addresses
        managed_shared_memory::handle_t handle = 0;

        //Obtain handle value
        std::stringstream s; s << "handle: "; s >> handle;

        //Get buffer local address from handle
        void *msg = segment.get_address_from_handle(handle);

        //Deallocate previously allocated memory
        segment.deallocate(msg);
    }
    return 0;
}

int InterProcess::namedShrMemory(int argc)
{
    using namespace boost::interprocess;
    typedef std::pair<double, int> MyType;

    if (argc == 1) {  //Parent process
       //Remove shared memory on construction and destruction
        struct shm_remove
        {
            shm_remove() { shared_memory_object::remove("MySharedMemory"); }
            ~shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        } remover;

        //Construct managed shared memory
        managed_shared_memory segment(create_only, "MySharedMemory", 65536);

        //Create an object of MyType initialized to {0.0, 0}
        MyType *instance = segment.construct<MyType>
            ("MyType instance")  //name of the object
            (0.0, 0);            //ctor first argument

         //Create an array of 10 elements of MyType initialized to {0.0, 0}
        MyType *array = segment.construct<MyType>
            ("MyType array")     //name of the object
            [10]                 //number of elements
        (0.0, 0);            //Same two ctor arguments for all objects

     //Create an array of 3 elements of MyType initializing each one
     //to a different value {0.0, 0}, {1.0, 1}, {2.0, 2}...
        float float_initializer[3] = { 0.0, 1.0, 2.0 };
        int   int_initializer[3] = { 0, 1, 2 };

        MyType *array_it = segment.construct_it<MyType>
            ("MyType array from it")   //name of the object
            [3]                        //number of elements
        (&float_initializer[0]    //Iterator for the 1st ctor argument
            , &int_initializer[0]);    //Iterator for the 2nd ctor argument

         //Launch child process
        std::string s(""); s += " child ";//argv[0]
        if (0 != std::system(s.c_str()))
            return 1;


        //Check child has destroyed all objects
        if (segment.find<MyType>("MyType array").first ||
            segment.find<MyType>("MyType instance").first ||
            segment.find<MyType>("MyType array from it").first)
            return 1;
    }
    else {
        //Open managed shared memory
        managed_shared_memory segment(open_only, "MySharedMemory");

        std::pair<MyType*, managed_shared_memory::size_type> res;

        //Find the array
        res = segment.find<MyType>("MyType array");
        //Length should be 10
        if (res.second != 10) return 1;

        //Find the object
        res = segment.find<MyType>("MyType instance");
        //Length should be 1
        if (res.second != 1) return 1;

        //Find the array constructed from iterators
        res = segment.find<MyType>("MyType array from it");
        //Length should be 3
        if (res.second != 3) return 1;

        //We're done, delete all the objects
        segment.destroy<MyType>("MyType array");
        segment.destroy<MyType>("MyType instance");
        segment.destroy<MyType>("MyType array from it");
    }
    return 0;
}




int g_count = 100;
HANDLE g_hSem1 = INVALID_HANDLE_VALUE;
HANDLE g_hSem2 = INVALID_HANDLE_VALUE;
 static DWORD WINAPI threadProc(LPVOID IpParamter)
{
    int count = 0;
    while (count < g_count)
    {
        WaitForSingleObject(g_hSem1, INFINITE);
        printf("threadProc()\n");
        ReleaseSemaphore(g_hSem2, 1, NULL);
        Sleep(1);
    }
    return 0;
}
int InterProcess::testSemapthore()
{
    g_hSem1 = CreateSemaphore(NULL, 1, 1, NULL);
    g_hSem2 = CreateSemaphore(NULL, 0, 1, NULL);
    HANDLE hThread = CreateThread(NULL, 0, threadProc, NULL, 0, NULL);
    int count = 0;
    while (count < g_count)
    {
        WaitForSingleObject(g_hSem2, INFINITE);
        printf("main()\n");
        ReleaseSemaphore(g_hSem1, 1, NULL);
        Sleep(1);
    }

    return 0;
}