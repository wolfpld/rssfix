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
    void Worker();

    std::atomic<bool> m_exit;
    std::vector<std::thread> m_threadPool;

    std::mutex m_jobLock;
    std::condition_variable m_jobCv;
    std::vector<JobQueue> m_jobQueue;
};

#endif
