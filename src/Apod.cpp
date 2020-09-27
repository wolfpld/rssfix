#include <stdio.h>

#include "Apod.hpp"
#include "Color.hpp"
#include "Curl.hpp"

#include "../contrib/ini/ini.h"

Apod::Apod()
    : m_numArticles( 10 )
{
}

bool Apod::InitializeImpl( ini_t* config )
{
    ini_sget( config, "global", "articles", "%d", &m_numArticles );
    ini_sget( config, "apod", "articles", "%d", &m_numArticles );

    const bool status = m_numArticles > 0;
    PrintStatus( status, "APOD", "Initialization: configured for %i articles", m_numArticles );
    return status;
}

bool Apod::FirstFetch()
{
    auto page = Curl::Get( m_curl, "https://apod.nasa.gov/apod/astropix.html" );
    return true;
}
