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
    bool FirstFetchImpl() final;

    void ProcessArticle( const std::unique_ptr<pugi::xml_document>& article, const char* url, uint64_t timestamp );
};

#endif
