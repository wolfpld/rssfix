#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#include <curl/curl.h>
#include <mutex>

struct ini_t;

class Handler
{
public:
    Handler();
    virtual ~Handler();

    bool Initialize( ini_t* config );
    virtual bool FirstFetch() = 0;

protected:
    virtual bool InitializeImpl( ini_t* config ) = 0;

    void PrintStatus( bool status, const char* unit, const char* format, ... ) const;

    CURL* m_curl;

private:
    mutable std::mutex m_stdoutLock;
};

#endif
