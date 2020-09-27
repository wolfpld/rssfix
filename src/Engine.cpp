#include <stdio.h>
#include <thread>

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
