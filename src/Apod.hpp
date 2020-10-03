#ifndef __APOD_HPP__
#define __APOD_HPP__

#include <memory>
#include <pugixml.hpp>

#include "Handler.hpp"

class Apod : public Handler
{
public:
    Apod();

private:
    bool InitializeImpl( ini_t* config ) final;
    bool FetchImpl( bool first ) final;

    bool ProcessArticle( const std::unique_ptr<pugi::xml_document>& article, const char* url, uint64_t timestamp );

    const std::string m_baseUrl;
};

#endif
