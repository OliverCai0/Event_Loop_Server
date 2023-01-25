#ifndef THREAD_POOL
#define THREAD_POOL

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <chrono>

/*
 * Struct to hold job and its arguments
 * - job: function that returns a boolean and takes in
 *        std::vector<T> args as an argument
 */
template <typename T>
struct job_package
{
    int (*job)(std::vector<T>);
    std::vector<T> args;
};

/*
 * Thread pool class
 * Multiple worker threads synchronized on a single queue
 */
template <typename T>
class threadPool
{
private:
    /*
     * Vector holding threads
     */
    std::vector<std::thread> pool;

    /*
     * Mutex for job queue
     */
    std::mutex job_queue_mutex;

    /*
     * Holds job_package structs
     */
    std::queue<job_package<T>> job_queue;

    /*
     * Conditional variable to sync up worker threads
     */
    std::condition_variable cv;

    /*
     * Used to signal deconstruction of threadPool
     */
    int teardown;

    /*
     * Defines the worker functionality
     */
    void worker();

public:
    /*
     * Initializes a pool of num_threads threads
     */
    threadPool(int num_threads);

    /*
     * Cleans up thread pool
     */
    ~threadPool();

    /*
     * Queues a new job
     */
    void queueJob(struct job_package<T> &job);

    /*
     * Checks if job queue is non-empty
     */
    int queueBusy();
};

#endif