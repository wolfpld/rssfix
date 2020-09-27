#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include <memory>
#include <vector>

#include "Handler.hpp"

struct ini_t;

class Engine
{
public:
    bool Initialize( ini_t* config );

private:
    template<class T>
    bool AddHandler( ini_t* config, const char* name );

    std::vector<std::unique_ptr<Handler>> m_handlers;
};

#endif
