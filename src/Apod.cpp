#include <stdio.h>

#include "Apod.hpp"
#include "Color.hpp"
#include "Curl.hpp"
#include "Parser.hpp"

#include "../contrib/ini/ini.h"

Apod::Apod()
    : Handler( "APOD" )
    , m_numArticles( 10 )
{
}

bool Apod::InitializeImpl( ini_t* config )
{
    ini_sget( config, "global", "articles", "%d", &m_numArticles );
    ini_sget( config, "apod", "articles", "%d", &m_numArticles );

    const bool status = m_numArticles > 0;
    PrintStatus( status, "Initialization: configured for %i articles", m_numArticles );
    return status;
}

bool Apod::FirstFetch()
{
    const char* url = "https://apod.nasa.gov/apod/astropix.html";
    auto page = Curl::Get( m_curl, url );
    if( page.empty() )
    {
        PrintError( nullptr, "Cannot download %s", url );
        return false;
    }
    page.emplace_back( '\0' );
    const char* xhtml;
    auto res = ParseHtml( (const char*)page.data(), xhtml );
    if( !res ) PrintError( xhtml, "Cannot parse %s", url );
    return res;
}
