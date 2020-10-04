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
    m_controller = std::thread( [this] { Controller(); } );
    for( int i=0; i<threads; i++ ) m_threadPool.emplace_back( std::thread( [this] { Worker(); } ) );
}

JobSystem::~JobSystem()
{
    printf( "Job system shutdown\n" );
    fflush( stdout );
    m_exit.store( true, std::memory_order_release );
    {
        std::lock_guard lock( m_waitLock );
        m_waitCv.notify_all();
    }
    {
        std::lock_guard lock( m_runLock );
        m_runCv.notify_all();
    }
    for( auto& v : m_threadPool ) v.join();
    m_controller.join();
    printf( "Job system shutdown done\n" );
    fflush( stdout );
}

void JobSystem::Enqueue( int64_t runAt, void(*task)(Handler*), Handler* hnd )
{
    const auto ct = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
    if( runAt <= ct )
    {
        std::lock_guard lock( m_runLock );
        m_runQueue.emplace_back( JobQueue { runAt, task, hnd } );
        m_runCv.notify_one();
    }
    else
    {
        std::lock_guard lock( m_waitLock );
        m_waitQueue.emplace_back( JobQueue { runAt, task, hnd } );
        std::push_heap( m_waitQueue.begin(), m_waitQueue.end(), [] ( const auto& l, const auto& r ) { return l.runAt > r.runAt; } );
        m_waitCv.notify_one();
    }
}

void JobSystem::Controller()
{
    std::unique_lock lock( m_waitLock );
    for(;;)
    {
        if( m_exit.load( std::memory_order_acquire ) ) return;
        if( m_waitQueue.empty() )
        {
            m_waitCv.wait( lock );
        }
        else
        {
            const auto ct = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
            const auto ft = m_waitQueue.front().runAt;
            if( ct < ft )
            {
                m_waitCv.wait_for( lock, std::chrono::seconds( ft - ct ) );
            }
            else
            {
                std::pop_heap( m_waitQueue.begin(), m_waitQueue.end(), [] ( const auto& l, const auto& r ) { return l.runAt > r.runAt; } );
                auto job = std::move( m_waitQueue.back() );
                m_waitQueue.pop_back();
                std::lock_guard runLock( m_runLock );
                m_runQueue.emplace_back( std::move( job ) );
                m_runCv.notify_one();
            }
        }
    }
}

void JobSystem::Worker()
{
    std::unique_lock lock( m_runLock );
    for(;;)
    {
        if( m_exit.load( std::memory_order_acquire ) ) return;
        if( m_runQueue.empty() )
        {
            m_runCv.wait( lock );
        }
        else
        {
            auto job = std::move( m_runQueue.back() );
            m_runQueue.pop_back();
            lock.unlock();
            job.task( job.hnd );
            lock.lock();
        }
    }
}
