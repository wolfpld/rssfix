#include <stdio.h>
#include "../contrib/ini/ini.h"

#include "Apod.hpp"
#include "Color.hpp"
#include "Engine.hpp"

bool Engine::Initialize( ini_t* config )
{
    int enableApod = 0;
    ini_sget( config, "feeds", "apod", "%d", &enableApod );

    if( !enableApod )
    {
        fprintf( stderr, BOLDRED "No feeds enabled in configuration!" RESET "\n" );
        return false;
    }

    if( enableApod )
    {
        if( !AddHandler<Apod>( config ) ) return false;
    }

    return true;
}

template<class T>
bool Engine::AddHandler( ini_t* config )
{
    auto ptr = std::make_unique<T>();
    if( !ptr->Initialize( config ) ) return false;
    m_handlers.push_back( std::move( ptr ) );
    return true;
}
