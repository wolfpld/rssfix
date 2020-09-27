#include <stdio.h>
#include "../contrib/ini/ini.h"

#include "Apod.hpp"
#include "Color.hpp"
#include "Engine.hpp"

bool Engine::Initialize( ini_t* config )
{
    if( !AddHandler<Apod>( config, "apod" ) ) return false;

    if( m_handlers.empty() )
    {
        fprintf( stderr, BOLDRED "No feeds enabled in configuration!" RESET "\n" );
        return false;
    }
    return true;
}

template<class T>
bool Engine::AddHandler( ini_t* config, const char* name )
{
    int enable = 0;
    ini_sget( config, "feeds", name, "%d", &enable );
    if( !enable ) return true;
    auto ptr = std::make_unique<T>();
    if( !ptr->Initialize( config ) ) return false;
    m_handlers.push_back( std::move( ptr ) );
    return true;
}
