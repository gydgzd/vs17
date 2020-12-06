#include <iostream> 
#include <boost/thread.hpp>

void wait(int seconds)
{
    boost::this_thread::sleep(boost::posix_time::seconds(seconds));
}
boost::mutex mutex;
boost::shared_mutex shr_mutex;
int g_i = 0;
void thread()
{
    try {
        for (g_i = 0; g_i < 5; ++g_i)
        {
            wait(1);
            boost::unique_lock<boost::shared_mutex> lock(shr_mutex);
            std::cout << "tid:" << boost::this_thread::get_id() << " " << g_i <<std::endl;
        }
    }
    catch (boost::thread_interrupted*)
    {
        std::cout << "tid:" << boost::this_thread::get_id() << " Get an exception." << std::endl;
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