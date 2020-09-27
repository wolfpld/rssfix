#include <assert.h>

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

static size_t WriteFn( void* _data, size_t size, size_t num, void* ptr )
{
    const auto data = (uint8_t*)_data;
    const auto sz = size*num;
    auto& v = *(std::vector<uint8_t>*)ptr;
    v.insert( v.end(), data, data+sz );
    return sz;
}

std::vector<uint8_t> Curl::Get( CURL*& curl, const char* url, bool reconnect )
{
    assert( s_instance );

    std::vector<uint8_t> buf;
    if( reconnect )
    {
        curl_easy_cleanup( curl );
        curl = curl_easy_init();
        if( !curl ) return buf;
    }
    buf.reserve( 64*1024 );

    curl_easy_setopt( curl, CURLOPT_URL, url );
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteFn );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &buf );
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, 120 );

    if( curl_easy_perform( curl ) != CURLE_OK ) buf.clear();
    return buf;
}
