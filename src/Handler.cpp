#include <stdarg.h>
#include <stdio.h>

#include "Color.hpp"
#include "Handler.hpp"

Handler::Handler()
    : m_curl( curl_easy_init() )
{
}

Handler::~Handler()
{
    curl_easy_cleanup( m_curl );
}

bool Handler::Initialize( ini_t* config )
{
    if( !m_curl )
    {
        fprintf( stderr, BOLDRED "Failed to obtain CURL handle!" RESET "\n" );
        return false;
    }
    return InitializeImpl( config );
}

void Handler::PrintStatus( bool status, const char* unit, const char* format, ... ) const
{
    char tmp[4096];
    va_list args;
    va_start( args, format );
    vsnprintf( tmp, 4096, format, args );
    va_end( args );

    std::lock_guard lock( m_stdoutLock );
    printf( BOLDWHITE "  [%s" BOLDWHITE "] " BOLDMAGENTA "%s" RESET " %s\n", status ? BOLDGREEN "✓" : BOLDRED "✗", unit, tmp );
}
