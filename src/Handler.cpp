#include <algorithm>
#include <assert.h>
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

Handler::Handler( const char* unit, const char* sourceUrl )
    : m_numArticles( 10 )
    , m_refresh( 3600 )
    , m_failureRefresh( 60 )
    , m_curl( curl_easy_init() )
    , m_pageHash( {} )
    , m_unit( unit )
    , m_sourceUrl( sourceUrl )
    , m_ready( false )
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
        fflush( stderr );
        return false;
    }

    m_feedUrlShort = "/apod";
    m_feedUrl = std::string( ini_get( config, "server", "url" ) ) + m_feedUrlShort;

    ini_sget( config, "global", "initial", "%d", &m_initialArticles );
    ini_sget( config, "global", "articles", "%d", &m_numArticles );
    ini_sget( config, "global", "refresh", "%d", &m_refresh );
    ini_sget( config, "global", "failure", "%d", &m_failureRefresh );
    return InitializeImpl( config );
}

bool Handler::Fetch( bool first )
{
    const auto status = FetchImpl( first );
    if( status )
    {
        SortArticles();
        TrimArticles();
        CacheFeed();
        if( first )
        {
            PrintStatus( true, "Fetch: %s (%i/%i articles)", GetTitle().c_str(), (int)m_articles.size(), m_numArticles );
            m_ready.store( true, std::memory_order_release );
        }
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
    fflush( stdout );
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
    fflush( stdout );
}


const char* Handler::FormatTime( int time ) const
{
    static char buf[64];
    if( time < 61 )
    {
        snprintf( buf, 64, "%i s", time );
    }
    else if( time < 60*60 )
    {
        int minutes = time / 60;
        time -= time * 60;
        snprintf( buf, 64, "%02i:%02i", minutes, time );
    }
    else if( time < 24*60*60 )
    {
        int hours = time / (60*60);
        time -= hours * (60*60);
        int minutes = time / 60;
        time -= minutes * 60;
        snprintf( buf, 64, "%02i:%02i:%02i", hours, minutes, time );
    }
    else
    {
        int days = time / (24*60*60);
        time -= days * (24*60*60);
        int hours = time / (60*60);
        time -= hours * (60*60);
        int minutes = time / 60;
        time -= minutes * 60;
        snprintf( buf, 64, "%id %02i:%02i:%02i", days, hours, minutes, time );
    }
    return buf;
}

std::vector<uint8_t> Handler::FetchPage( const char* url )
{
    auto page = Curl::Get( m_curl, url );
    if( page.empty() )
    {
        PrintError( nullptr, "Cannot download %s", url );
        return page;
    }
    page.emplace_back( '\0' );
    return page;
}

bool Handler::PageHashChanged( const std::vector<uint8_t>& page )
{
    const auto hash = XXH3_128bits( page.data(), page.size() );
    if( memcmp( &hash, &m_pageHash, sizeof( hash ) ) == 0 ) return false;
    m_pageHash = hash;
    return true;
}

std::unique_ptr<pugi::xml_document> Handler::FetchDom( const std::vector<uint8_t>& page, bool tidy )
{
    if( page.empty() ) return nullptr;

    char* xhtml = nullptr;
    if( tidy )
    {
        auto res = ParseHtml( (const char*)page.data(), xhtml );
        if( !res )
        {
            PrintError( xhtml, "Cannot parse web page" );
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
    fflush( stdout );
}

bool Handler::ContainsArticle( const std::string& uid ) const
{
    for( auto& v : m_articles ) if( v.uid == uid ) return true;
    return false;
}

void Handler::AddArticle( ArticleData&& article )
{
    assert( !ContainsArticle( article.uid ) );
    printf( BOLDWHITE "  [" BOLDBLUE "+" BOLDWHITE "] " BOLDMAGENTA "%s" RESET " New article: %s\n", m_unit, article.uid.c_str() );
    fflush( stdout );
    m_articles.emplace_back( std::move( article ) );
}

void Handler::SortArticles()
{
    std::sort( m_articles.begin(), m_articles.end(), []( const auto& l, const auto& r ) { return l.timestamp > r.timestamp; } );
}

void Handler::TrimArticles()
{
    if( m_articles.size() > m_numArticles ) m_articles.resize( m_numArticles );
}

void Handler::CacheFeed()
{
    char tbuf[64];
    strftime( tbuf, 64, "%FT%TZ", gmtime( (time_t*)&m_articles[0].timestamp ) );

    auto feed = std::make_unique<pugi::xml_document>();
    auto root = feed->append_child( "feed" );
    root.append_attribute( "xmlns" ).set_value( "http://www.w3.org/2005/Atom" );

    root.append_child( "title" ).append_child( pugi::node_pcdata ).set_value( m_title.c_str() );
    root.append_child( "subtitle" ).append_child( pugi::node_pcdata ).set_value( m_description.c_str() );
    root.append_child( "id" ).append_child( pugi::node_pcdata ).set_value( m_feedUrl.c_str() );
    root.append_child( "updated" ).append_child( pugi::node_pcdata ).set_value( tbuf );

    auto self = root.append_child( "link" );
    self.append_attribute( "type" ).set_value( "self" );
    self.append_attribute( "href" ).set_value( m_feedUrl.c_str() );
    auto altr = root.append_child( "link" );
    altr.append_attribute( "type" ).set_value( "alternate" );
    altr.append_attribute( "href" ).set_value( m_sourceUrl.c_str() );
    if( !m_icon.empty() ) root.append_child( "icon" ).append_child( pugi::node_pcdata ).set_value( m_icon.c_str() );

    for( auto& v : m_articles )
    {
        root.append_copy( v.doc->first_child() );
    }

    std::stringstream ss;
    feed->save( ss );
    m_feedString = ss.str();
}

void Handler::FixupLink( pugi::xml_attribute attr, const char* baseUrl )
{
    auto str = attr.as_string();
    if( strncmp( str, "http://", 7 ) == 0 ) return;
    if( strncmp( str, "https://", 8 ) == 0 ) return;
    auto fixed = std::string( baseUrl ) + str;
    attr.set_value( fixed.c_str() );
}
