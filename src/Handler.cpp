#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Color.hpp"
#include "Handler.hpp"

Handler::Handler( const char* unit )
    : m_curl( curl_easy_init() )
    , m_unit( unit )
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

void Handler::PrintStatus( bool status, const char* format, ... ) const
{
    char tmp[4096];
    va_list args;
    va_start( args, format );
    vsnprintf( tmp, 4096, format, args );
    va_end( args );

    std::lock_guard lock( m_stdoutLock );
    printf( BOLDWHITE "  [%s" BOLDWHITE "] " BOLDMAGENTA "%s" RESET " %s\n", status ? BOLDGREEN "✓" : BOLDRED "✗", m_unit, tmp );
}

void Handler::PrintError( const char* context, const char* err, ... ) const
{
    char tmp[4096];
    va_list args;
    va_start( args, err );
    vsnprintf( tmp, 4096, err, args );
    va_end( args );

    std::lock_guard lock( m_stdoutLock );
    printf( BOLDWHITE "  [" BOLDYELLOW "!" BOLDWHITE "] " BOLDMAGENTA "%s" BOLDRED " Error: " RESET "%s\n", m_unit, tmp );
    if( context && *context )
    {
        const auto sz = strlen( context );
        if( context[sz-1] == '\n' )
        {
            printf( "%s", context );
        }
        else
        {
            printf( "%s\n", context );
        }
    }
}
