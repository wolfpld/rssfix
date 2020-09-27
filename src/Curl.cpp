#include <assert.h>
#include <curl/curl.h>

#include "Curl.hpp"
#include "OpenSslThreading.hpp"

static Curl* s_instance = nullptr;

Curl::Curl( const ticket_t& )
{
    assert( !s_instance );
    s_instance = this;
}

Curl::~Curl()
{
    curl_global_cleanup();
    OpenSslThreadCleanup();

    assert( s_instance );
    s_instance = nullptr;
}

std::unique_ptr<Curl> Curl::Initialize()
{
    assert( !s_instance );
    OpenSslThreadInit();
    if( curl_global_init( CURL_GLOBAL_ALL ) != 0 ) return nullptr;
    return std::make_unique<Curl>( ticket_t {} );
}
