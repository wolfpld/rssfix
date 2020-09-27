#ifndef __CURL_HPP__
#define __CURL_HPP__

#include <curl/curl.h>
#include <memory>
#include <stdint.h>
#include <vector>

class Curl
{
    struct ticket_t {};

public:
    static std::unique_ptr<Curl> Initialize();

    Curl( const ticket_t& );
    ~Curl();

    static std::vector<uint8_t> Get( CURL*& curl, const char* url, bool reconnect = false );
};

#endif
