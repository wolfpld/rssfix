#ifndef __CURL_HPP__
#define __CURL_HPP__

#include <memory>

class Curl
{
    struct ticket_t {};

public:
    static std::unique_ptr<Curl> Initialize();

    Curl( const ticket_t& );
    ~Curl();

private:
};

#endif
