#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <memory>
#include <vector>

#include "Handler.hpp"

struct ini_t;
struct mg_connection;

class Engine
{
public:
    Engine();
    ~Engine();

    bool Initialize( ini_t* config );
    void RunServer();

    const std::vector<std::unique_ptr<Handler>>& GetHandlers() const { return m_handlers; }

private:
    template<class T>
    bool AddHandler( ini_t* config, const char* name );

    std::vector<std::unique_ptr<Handler>> m_handlers;

    const char* m_bind;
    const char* m_port;
};

#endif
