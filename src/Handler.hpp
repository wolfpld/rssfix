#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#include <curl/curl.h>
#include <mutex>

struct ini_t;

class Handler
{
public:
    Handler( const char* unit );
    virtual ~Handler();

    bool Initialize( ini_t* config );
    virtual bool FirstFetch() = 0;

protected:
    virtual bool InitializeImpl( ini_t* config ) = 0;

    void PrintStatus( bool status, const char* format, ... ) const;
    void PrintError( const char* context, const char* err, ... ) const;

    CURL* m_curl;

    bool ParseHtml( const char* data, const char*& out );

private:
    mutable std::mutex m_stdoutLock;
    const char* m_unit;
};

#endif
