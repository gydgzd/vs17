#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <windows.h>
using namespace std;

extern int g_num;
extern std::mutex g_num_mutex;

class Mycounter
{
private:
    std::vector<int> m_numbers;
    std::mutex m_list_mutex;
    std::condition_variable m_cv;
public:
	Mycounter() {};
	~Mycounter() 
	{
		printf("~Mycounter()\n");
	};
	int push()
	{
		for (int i = 0; i < 10; i++)
		{
			std::unique_lock<std::mutex> guard(m_list_mutex);   // ��������֤�������ԭ��ִ��
            m_numbers.push_back(i);
			cout << "push: " << i << endl;
            guard.unlock();
            m_cv.notify_one();
		}
		return 0;
	}
	int get()
	{
		while(true)
		{
			std::unique_lock<std::mutex> guard(m_list_mutex);   // ��������֤�������ԭ��ִ��
            if (m_numbers.empty())
                m_cv.wait_for(guard, std::chrono::seconds(3));  // ����ǰ�ͷ������ñ���߳����У��յ�notify֪ͨ���ټ�������֤�Լ����� 
            //    m_cv.wait(guard);
            if (m_numbers.empty())
                break;
			cout << "get: " << m_numbers.front() << endl;
            m_numbers.erase(m_numbers.begin());
		}
		return 0;
	}
	void counter(int iterations, int id)
	{
		for (int i = 0; i < iterations; i++)
		{
			printf("%d %d\n", i, id);
		}
	}
	void testThread()
	{
		std::thread th_push{ &Mycounter::push, this };
        std::thread th_get{  &Mycounter::get, this };
        th_push.join();
        th_get.join();
		Sleep(10);
	}
};
