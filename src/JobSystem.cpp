#include <algorithm>
#include <assert.h>
#include <chrono>
#include <stdio.h>

#include "JobSystem.hpp"

JobSystem::JobSystem( int threads )
    : m_exit( false )
{
    assert( threads > 0 );
    printf( "Job system init with %i threads\n", threads );
    fflush( stdout );
    for( int i=0; i<threads; i++ ) m_threadPool.emplace_back( std::thread( [this] { Worker(); } ) );
}

JobSystem::~JobSystem()
{
    printf( "Job system shutdown\n" );
    fflush( stdout );
    m_exit.store( true, std::memory_order_release );
    {
        std::lock_guard lock( m_jobLock );
        m_jobCv.notify_all();
    }
    for( auto& v : m_threadPool ) v.join();
    printf( "Job system shutdown done\n" );
    fflush( stdout );
}

void JobSystem::Enqueue( int64_t runAt, void(*task)(Handler*), Handler* hnd )
{
    std::lock_guard lock( m_jobLock );
    m_jobQueue.emplace_back( JobQueue { runAt, task, hnd } );
    std::push_heap( m_jobQueue.begin(), m_jobQueue.end(), [] ( const auto& l, const auto& r ) { return l.runAt > r.runAt; } );
    m_jobCv.notify_all();
}

void JobSystem::Worker()
{
    std::unique_lock lock( m_jobLock );
    for(;;)
    {
        if( m_exit.load( std::memory_order_acquire ) ) return;
        bool shouldWait = true;
        const auto ct = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
        if( !m_jobQueue.empty() )
        {
            if( m_jobQueue.front().runAt <= ct )
            {
                shouldWait = false;
                auto job = std::move( m_jobQueue.front() );
                pop_heap( m_jobQueue.begin(), m_jobQueue.end(), [] ( const auto& l, const auto& r ) { return l.runAt > r.runAt; } );
                m_jobQueue.pop_back();
                lock.unlock();
                job.task( job.hnd );
                lock.lock();
                if( m_exit.load( std::memory_order_acquire ) ) return;
            }
        }
        if( shouldWait )
        {
            if( m_jobQueue.empty() )
            {
                m_jobCv.wait( lock );
            }
            else
            {
                const auto ft = m_jobQueue.front().runAt;
                assert( ft > ct );
                m_jobCv.wait_for( lock, std::chrono::seconds( ft - ct ) );
            }
        }
    }
}
