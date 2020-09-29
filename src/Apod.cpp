#include <stdio.h>

#include "Apod.hpp"

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
    auto dom = FetchDom( url );
    if( !dom ) return false;

    auto title = dom->select_node( "/html/head/title" );
    m_title = title ? title.node().text().as_string() : "APOD";

    auto desc = dom->select_node( "/html/head/meta[@name=\"description\"]" );
    m_description = desc ? desc.node().attribute( "content" ).as_string() : "";

    return true;
}
