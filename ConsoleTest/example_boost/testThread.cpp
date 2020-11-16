#include <iostream> 
#include <boost/thread.hpp>

void wait(int seconds)
{
    boost::this_thread::sleep(boost::posix_time::seconds(seconds));
}
boost::mutex mutex;
void thread()
{
    try {
        for (int i = 0; i < 5; ++i)
        {
            wait(1);
            mutex.lock();
            std::cout << "tid:" << boost::this_thread::get_id() << " " << i << std::endl;
            mutex.unlock();
        }
    }
    catch (boost::thread_interrupted& e)
    {
        std::cout << "Get an exception." << std::endl;
    }

}

int testThread()
{
    boost::thread t(thread);
    boost::thread t1(thread);
    wait(3);
    t.interrupt();
    t.join();
    t1.join();
    return 0;
}