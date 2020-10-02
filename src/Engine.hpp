#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <functional>
#include <memory>
#include <vector>

#include "Handler.hpp"
#include "JobSystem.hpp"

struct ini_t;
struct mg_connection;

class Engine
{
public:
    Engine();
    ~Engine();

    static Engine* Instance();

    bool Initialize( ini_t* config );
    void RunServer();

    const std::vector<std::unique_ptr<Handler>>& GetHandlers() const { return m_handlers; }
    bool ShouldAdvertise() const { return m_advertise; }

    void Enqueue( int64_t runAt, void(*task)(Handler*), Handler* hnd ) { m_jobSystem->Enqueue( runAt, task, hnd ); }

private:
    template<class T>
    bool AddHandler( ini_t* config, const char* name );

    std::unique_ptr<JobSystem> m_jobSystem;
    std::vector<std::unique_ptr<Handler>> m_handlers;

    const char* m_bind;
    const char* m_port;
    int m_advertise;
    int m_threads;
};

#endif
