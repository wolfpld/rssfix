#ifndef __JOBSYSTEM_HPP__
#define __JOBSYSTEM_HPP__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class Handler;

class JobSystem
{
    struct JobQueue
    {
        int64_t runAt;
        void(*task)(Handler*);
        Handler* hnd;
    };

public:
    JobSystem( int threads );
    ~JobSystem();

    void Enqueue( int64_t runAt, void(*task)(Handler*), Handler* hnd );

private:
    void Controller();
    void Worker();

    std::atomic<bool> m_exit;
    std::thread m_controller;
    std::vector<std::thread> m_threadPool;

    std::mutex m_waitLock;
    std::condition_variable m_waitCv;
    std::vector<JobQueue> m_waitQueue;

    std::mutex m_runLock;
    std::condition_variable m_runCv;
    std::vector<JobQueue> m_runQueue;
};

#endif
