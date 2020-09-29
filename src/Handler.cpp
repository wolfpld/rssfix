#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tidy.h>
#include <tidybuffio.h>

#include "../contrib/ini/ini.h"

#include "Color.hpp"
#include "Curl.hpp"
#include "Handler.hpp"

Handler::Handler( const char* unit )
    : m_numArticles( 10 )
    , m_curl( curl_easy_init() )
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
    ini_sget( config, "global", "articles", "%d", &m_numArticles );
    return InitializeImpl( config );
}

bool Handler::FirstFetch()
{
    const auto status = FirstFetchImpl();
    if( status )
    {
        PrintStatus( true, "Fetch: %s (%i/%i articles)", GetTitle().c_str(), (int)m_articles.size(), m_numArticles );
    }
    else
    {
        PrintStatus( false, "Fetch failed" );
    }
    return status;
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

std::unique_ptr<pugi::xml_document> Handler::FetchDom( const char* url, bool tidy )
{
    auto page = Curl::Get( m_curl, url );
    if( page.empty() )
    {
        PrintError( nullptr, "Cannot download %s", url );
        return nullptr;
    }
    page.emplace_back( '\0' );

    char* xhtml = nullptr;
    if( tidy )
    {
        auto res = ParseHtml( (const char*)page.data(), xhtml );
        if( !res )
        {
            PrintError( xhtml, "Cannot parse %s", url );
            free( xhtml );
            return nullptr;
        }
    }

    auto doc = std::make_unique<pugi::xml_document>();
    auto xmlres = doc->load_string( tidy ? xhtml : (const char*)page.data() );
    free( xhtml );
    if( !xmlres )
    {
        PrintError( xmlres.description(), "Cannot build XML tree" );
        return nullptr;
    }
    return doc;
}

bool Handler::ParseHtml( const char* data, char*& out )
{
    TidyBuffer err = {};
    tidyBufInit( &err );

    TidyDoc td = tidyCreate();
    tidyOptSetBool( td, TidyXhtmlOut, yes );
    tidyOptSetBool( td, TidyLowerLiterals, yes );
    tidyOptSetBool( td, TidyMark, no );
    tidySetErrorBuffer( td, &err );

    if( tidyParseString( td, data ) == 2 )
    {
        out = strdup( (const char*)err.bp );
        tidyBufFree( &err );
        tidyRelease( td );
        return false;
    }

    TidyBuffer buf = {};
    tidyBufInit( &buf );
    tidyCleanAndRepair( td );
    tidySaveBuffer( td, &buf );

    out = strdup( (const char*)buf.bp );

    tidyBufFree( &buf );
    tidyBufFree( &err );
    tidyRelease( td );
    return true;
}

void Handler::PrintDom( const std::unique_ptr<pugi::xml_document>& dom )
{
    printf( CYAN "------------------ Debug DOM print begins here ----------->8----" RESET "\n" );
    std::stringstream ss;
    dom->save( ss, "  " );
    printf( "%s\n", ss.str().c_str() );
    printf( CYAN "----------->8----- Debug DOM print ends here -------------------" RESET "\n" );
}
