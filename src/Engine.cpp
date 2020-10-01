#include <assert.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>

#include "../contrib/ini/ini.h"
#include "../contrib/mongoose/mongoose.h"

#include "Apod.hpp"
#include "Color.hpp"
#include "Engine.hpp"
#include "Version.hpp"

Engine* s_instance = nullptr;


Engine::Engine()
    : m_bind( "127.0.0.1" )
    , m_port( "4001" )
    , m_advertise( true )
{
    assert( !s_instance );
    s_instance = this;
}

Engine::~Engine()
{
    assert( s_instance );
    s_instance = nullptr;
}

static void TrySet( const char*& value, ini_t* config, const char* section, const char* key )
{
    auto tmp = ini_get( config, section, key );
    if( tmp ) value = tmp;
}

bool Engine::Initialize( ini_t* config )
{
    auto url = ini_get( config, "server", "url" );
    if( !url )
    {
        fprintf( stderr, BOLDRED "Feed URL must be set!" RESET "\n" );
        return false;
    }
    TrySet( m_bind, config, "server", "bind" );
    TrySet( m_port, config, "server", "port" );
    ini_sget( config, "server", "advertise", "%d", &m_advertise );

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
    printf( "Base address: %s\n", url );

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

static void ConnectionHandler( struct mg_connection* nc, int ev, void* data )
{
    if( ev == MG_EV_HTTP_REQUEST )
    {
        auto hm = (struct http_message*)data;
        char remoteAddr[100];
        mg_sock_to_str( nc->sock, remoteAddr, sizeof(remoteAddr), MG_SOCK_STRINGIFY_REMOTE | MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT );
        std::string uri( hm->uri.p, hm->uri.len );
        auto ua = mg_get_http_header( hm, "User-Agent" );
        int code = 0, size = 0;
        if( uri == "/" )
        {
            if( s_instance->ShouldAdvertise() )
            {
                std::ostringstream resp;
                resp << "<!doctype html>\n<html><head><meta charset=\"utf-8\"><title>RssFix</title></head><body><h1>RssFix " \
                    << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH \
                    << "</h1><h2>Available sources:</h2>";
                for( auto& v : s_instance->GetHandlers() )
                {
                    if( v->IsReady() )
                    {
                        char tbuf[64];
                        const auto ts = v->GetTimestamp();
                        strftime( tbuf, 64, "%F %T %z", localtime( (time_t*)&ts ) );

                        resp << "<p><b>" << v->GetTitle() << "</b></br>" \
                            << "<a href=\"" << v->GetFeedUrl() << "\">" << v->GetFeedUrl() << "</a><br/>" \
                            << "<i>" << v->GetDescription() << "</i><br/>" \
                            << "Messages: " << v->GetArticlesCount() << "/" << v->GetArticlesMax() << "<br/>" \
                            << "Newest article at " << tbuf << "<br/>" \
                            << "Article refresh every " << v->GetRefreshRate() \
                            << "</p>";
                    }
                    else
                    {
                        resp << "<p><b>" << v->GetTitle() << "</b></br>" \
                            << "Feed is still initializing...<br/>" \
                            << "<i>" << v->GetDescription() << "</i><br/>" \
                            << "Messages: 0/" << v->GetArticlesMax() << "<br/>" \
                            << "Article refresh every " << v->GetRefreshRate() \
                            << "</p>";
                    }
                }
                resp << "</body></html>";

                code = 200;
                size = (int)resp.str().size();
                mg_send_head( nc, 200, size, "Content-Type: text/html" );
                mg_printf( nc, "%.*s", size, resp.str().c_str() );
            }
            else
            {
                code = 404;
                mg_http_send_error( nc, 404, nullptr );
            }
        }
        else
        {
            for( auto& v : s_instance->GetHandlers() )
            {
                if( uri == v->GetFeedUrlShort() )
                {
                    if( v->IsReady() )
                    {
                        auto& feed = v->GetFeed();
                        code = 200;
                        size = feed.size();
                        mg_send_head( nc, 200, size, "Content-Type: text/xml" );
                        mg_printf( nc, "%.*s", size, feed.c_str() );
                    }
                    else
                    {
                        code = 503;
                        mg_http_send_error( nc, 503, nullptr );
                    }
                    break;
                }
            }
            if( code == 0 )
            {
                code = 404;
                mg_http_send_error( nc, 404, nullptr );
            }
        }
        printf( CYAN "%s " GREEN "\"%.*s %.*s\" " YELLOW "%i " MAGENTA "%i " RED "\"%.*s\"" RESET "\n", remoteAddr, (int)hm->method.len, hm->method.p, (int)hm->uri.len, hm->uri.p, code, size, ua ? (int)ua->len : 0, ua ? ua->p : "" );
    }
}

void Engine::RunServer()
{
    char bind[1024];
    snprintf( bind, 1024, "%s:%s", m_bind, m_port );

    struct mg_mgr mgr;
    mg_mgr_init( &mgr, this );
    auto conn = mg_bind( &mgr, bind, ConnectionHandler );
    if( !conn )
    {
        fprintf( stderr, BOLDRED "Cannot bind to %s!" RESET "\n", bind );
        return;
    }
    mg_set_protocol_http_websocket( conn );

    printf( "Listening on %s...\n", bind );
    for(;;) mg_mgr_poll( &mgr, 1000 );
    mg_mgr_free( &mgr );
}
