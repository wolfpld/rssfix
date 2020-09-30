#include <stdio.h>
#include <time.h>

#include "Apod.hpp"

#include "../contrib/ini/ini.h"

Apod::Apod()
    : Handler( "APOD", "https://apod.nasa.gov/apod/astropix.html" )
    , m_baseUrl( "https://apod.nasa.gov/apod/" )
{
    m_icon = "https://apod.nasa.gov/favicon.ico";
}

bool Apod::InitializeImpl( ini_t* config )
{
    ini_sget( config, "apod", "articles", "%d", &m_numArticles );
    ini_sget( config, "apod", "refresh", "%d", &m_refresh );

    const bool status = m_numArticles > 0;
    PrintStatus( status, "Initialization: configured for %i articles, refresh: %s", m_numArticles, FormatTime( m_refresh ) );
    return status;
}

bool Apod::FirstFetchImpl()
{
    auto page = FetchPage( "https://apod.nasa.gov/apod/astropix.html" );
    if( !PageHashChanged( page ) ) return true;
    auto article = FetchDom( page );
    if( !article ) return false;

    auto title = article->select_node( "/html/head/title" );
    m_title = title ? title.node().text().as_string() : "APOD";

    auto desc = article->select_node( "/html/head/meta[@name='description']" );
    m_description = desc ? desc.node().attribute( "content" ).as_string() : "";

    std::string url;
    {
        auto next = article->select_node( "//a[text()='<']" );
        url = m_baseUrl + next.node().attribute( "href" ).as_string();
        auto tmp = FetchDom( FetchPage( url.c_str() ) );
        if( !tmp ) return false;
        auto prev = tmp->select_node( "//a[text()='>']" );
        url = m_baseUrl + prev.node().attribute( "href" ).as_string();
    }

    int num = 0;
    for(;;)
    {
        int y, m, d;
        sscanf( url.c_str() + 29, "%02d%02d%02d", &y, &m, &d );
        if( y < 70 ) y += 100;
        m--;

        struct tm tm = {};
        tm.tm_year = y;
        tm.tm_mon = m;
        tm.tm_mday = d;
        uint64_t timestamp = timegm( &tm );

        if( !ContainsArticle( url ) )
        {
            ProcessArticle( article, url.c_str(), timestamp );
        }

        auto next = article->select_node( "//a[text()='<']" );
        if( !next ) return !m_articles.empty();
        if( ++num == m_numArticles ) return true;

        url = m_baseUrl + next.node().attribute( "href" ).as_string();
        article = FetchDom( FetchPage( url.c_str() ) );
        if( !article ) return true;
    }
}

void Apod::ProcessArticle( const std::unique_ptr<pugi::xml_document>& article, const char* url, uint64_t timestamp )
{
    char tbuf[64];
    strftime( tbuf, 64, "%FT%TZ", gmtime( (time_t*)&timestamp ) );

    auto srcTitle = article->select_node( "/html/body/center[2]/b[1]" );

    auto doc = std::make_unique<pugi::xml_document>();
    auto root = doc->append_child( "entry" );
    root.append_child( "title" ).append_child( pugi::node_pcdata ).set_value( srcTitle.node().text().as_string() );
    root.append_child( "link" ).append_attribute( "href" ).set_value( url );
    root.append_child( "id" ).append_child( pugi::node_pcdata ).set_value( url );
    root.append_child( "updated" ).append_child( pugi::node_pcdata ).set_value( tbuf );

    auto authors = article->select_nodes( "/html/body/center[2]/a" );
    for( auto& author : authors )
    {
        auto node = root.append_child( "author" );
        node.append_child( "name" ).append_child( pugi::node_pcdata ).set_value( author.node().text().as_string() );
        auto contact = author.node().attribute( "href" ).as_string();
        if( strncmp( contact, "mailto:", 7 ) == 0 )
        {
            node.append_child( "email" ).append_child( pugi::node_pcdata ).set_value( contact );
        }
        else
        {
            node.append_child( "uri" ).append_child( pugi::node_pcdata ).set_value( contact );
        }
    }

    auto content = root.append_child( "content" );
    content.append_attribute( "type" ).set_value( "xhtml" );
    auto cdiv = content.append_child( "div" );
    cdiv.append_attribute( "xmlns" ).set_value( "http://www.w3.org/1999/xhtml" );
    auto srcImg = article->select_node( "/html/body/center[1]/p[2]/a" );
    if( srcImg )
    {
        cdiv.append_copy( srcImg.node() );
        FixupLink( cdiv.child( "a" ).attribute( "href" ), m_baseUrl.c_str() );
        FixupLink( cdiv.child( "a" ).child( "img" ).attribute( "src" ), m_baseUrl.c_str() );
    }
    else
    {
        auto srcIFrame = article->select_node( "/html/body/center[1]/p[2]/iframe" );
        cdiv.append_copy( srcIFrame.node() );
    }

    auto expl = article->select_node( "/html/body/p[1]" );
    for( auto& v : expl.node().select_nodes( "a/@href" ) )
    {
        FixupLink( v.attribute(), m_baseUrl.c_str() );
    }
    cdiv.append_copy( expl.node() );

    auto srcTags = article->select_node( "/html/head/meta[@name='keywords']" );
    auto tptr = srcTags.node().attribute( "content" ).as_string();
    auto tend = tptr;
    for(;;)
    {
        while( *tend != '\0' && *tend != ',' ) tend++;
        if( tend == tptr ) break;
        std::string tmp( tptr, tend );
        root.append_child( "category" ).append_attribute( "term" ).set_value( tmp.c_str() );
        while( *tend != '\0' && ( *tend == ',' || *tend == ' ' ) ) tend++;
        tptr = tend;
    }

    AddArticle( ArticleData { timestamp, std::move( doc ), std::string( url ) } );
}
