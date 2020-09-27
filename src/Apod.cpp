#include <stdio.h>

#include "Apod.hpp"
#include "Color.hpp"

#include "../contrib/ini/ini.h"

Apod::Apod()
    : m_numArticles( 10 )
{
}

bool Apod::Initialize( ini_t* config )
{
    ini_sget( config, "global", "articles", "%d", &m_numArticles );
    ini_sget( config, "apod", "articles", "%d", &m_numArticles );

    const bool status = m_numArticles > 0;
    PrintStatus( status, "APOD initialization: configured for %i articles", m_numArticles );
    return status;
}

bool Apod::FirstFetch()
{
    return true;
}
