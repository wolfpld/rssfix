#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <pugixml.hpp>

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

    std::unique_ptr<pugi::xml_document> FetchDom( const char* url );

    void PrintStatus( bool status, const char* format, ... ) const;
    void PrintError( const char* context, const char* err, ... ) const;

private:
    bool ParseHtml( const char* data, char*& out );

    CURL* m_curl;

    mutable std::mutex m_stdoutLock;
    const char* m_unit;
};

#endif
