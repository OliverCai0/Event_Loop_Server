#include "threadPool.h"
#include <iostream>

template <typename T>
void threadPool<T>::worker()
{
    for (;;)
    {
        struct job_package<T> p;
        {
            std::unique_lock<std::mutex> m(job_queue_mutex);
            cv.wait_for(m, std::chrono::milliseconds(100), [this]
                        { return teardown || !job_queue.empty(); });
            if (teardown)
            {
                return;
            }
            if (!job_queue.empty())
            {
                p = job_queue.front();
                job_queue.pop();
            }
            else
            {
                continue;
            }
        }
        p.job(p.args);
    }
}

template <typename T>
void threadPool<T>::queueJob(struct job_package<T> &job)
{
    job_queue.push(job);
}

template <typename T>
threadPool<T>::threadPool(int num_threads)
{
    teardown = 0;
    num_threads = num_threads > std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        std::thread t([this]()
                      { this->worker(); });
        pool.push_back(move(t));
    }
}

template <typename T>
threadPool<T>::~threadPool()
{
    teardown = 1;
    cv.notify_all();
    for (std::thread &t : pool)
    {
        t.join();
    }
    pool.clear();
};

template <typename T>
int threadPool<T>::queueBusy()
{
    return !job_queue.empty();
}

int main()
{
    /*
     * Attempts to construct the threadPool class, run a loop for three jobs, and deconstruct the class
     */

    struct job_package<int> j1;
    struct job_package<int> j2;
    struct job_package<int> j3;

    std::function<void(struct job_package<int> &, int x, int y)> setJobPackage = [](struct job_package<int> &j, int x, int y)
    {
        j.args = {x, y};
        j.job = static_cast<int (*)(std::vector<int>)>([](std::vector<int> v)
                                                       {
            std::cout << v[0] + v[1] << std::endl;
            return 0; });
    };

    setJobPackage(j1, 1, 2);
    setJobPackage(j2, 3, 2);
    setJobPackage(j3, 3, 4);

    threadPool<int> t(2);

    std::cout << "Enqueing jobs" << std::endl;
    t.queueJob(j1);
    t.queueJob(j2);
    t.queueJob(j3);

    while (t.queueBusy())
    {
    }
}