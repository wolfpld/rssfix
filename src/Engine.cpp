#include <stdio.h>
#include <thread>

#include "../contrib/ini/ini.h"
#include "../contrib/mongoose/mongoose.h"

#include "Apod.hpp"
#include "Color.hpp"
#include "Engine.hpp"

Engine::Engine()
    : m_bind( "127.0.0.1" )
    , m_port( "4001" )
{
}

static void TrySet( const char*& value, ini_t* config, const char* section, const char* key )
{
    auto tmp = ini_get( config, section, key );
    if( tmp ) value = tmp;
}

bool Engine::Initialize( ini_t* config )
{
    auto url = ini_get( config, "global", "url" );
    if( !url )
    {
        fprintf( stderr, BOLDRED "Feed URL must be set!" RESET "\n" );
        return false;
    }

    TrySet( m_bind, config, "global", "bind" );
    TrySet( m_port, config, "global", "port" );

    if( !AddHandler<Apod>( config, "apod" ) ) return false;

    if( m_handlers.empty() )
    {
        fprintf( stderr, BOLDRED "No feeds enabled in configuration!" RESET "\n" );
        return false;
    }

    printf( "Starting initial fetch\n" );
    std::vector<std::thread> initJobs;
    std::vector<char> initStatus( m_handlers.size(), 0 );
    initJobs.reserve( m_handlers.size() );
    int idx = 0;
    for( auto& hnd : m_handlers )
    {
        initJobs.emplace_back( std::thread( [&hnd, &status = initStatus[idx]] { status = hnd->FirstFetch(); } ) );
        idx++;
    }
    for( auto& job : initJobs ) job.join();
    for( auto& status : initStatus )
    {
        if( !status )
        {
            fprintf( stderr, BOLDRED "Initial fetch failed!" RESET "\n" );
            return false;
        }
    }
    printf( "Fetch done\n" );

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
