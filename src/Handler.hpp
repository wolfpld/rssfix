#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <pugixml.hpp>
#include <string>

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

    std::unique_ptr<pugi::xml_document> FetchDom( const char* url, bool tidy = true );
    void PrintDom( const std::unique_ptr<pugi::xml_document>& dom );

    void PrintStatus( bool status, const char* format, ... ) const;
    void PrintError( const char* context, const char* err, ... ) const;

    std::string m_title, m_description;

private:
    bool ParseHtml( const char* data, char*& out );

    CURL* m_curl;

    mutable std::mutex m_stdoutLock;
    const char* m_unit;
};

#endif
