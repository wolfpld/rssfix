#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <pugixml.hpp>
#include <string>
#include <vector>

struct ini_t;

struct ArticleData
{
    uint64_t timestamp;
    std::unique_ptr<pugi::xml_document> doc;
    std::string uid;
};

class Handler
{
public:
    Handler( const char* unit, const char* sourceUrl );
    virtual ~Handler();

    bool Initialize( ini_t* config );
    bool FirstFetch();

    const std::string& GetTitle() const { return m_title; }
    const std::string& GetDescription() const { return m_description; }

protected:
    virtual bool InitializeImpl( ini_t* config ) = 0;
    virtual bool FirstFetchImpl() = 0;

    std::unique_ptr<pugi::xml_document> FetchDom( const char* url, bool tidy = true );
    void PrintDom( const std::unique_ptr<pugi::xml_document>& dom );

    void PrintStatus( bool status, const char* format, ... ) const;
    void PrintError( const char* context, const char* err, ... ) const;

    void AddArticle( ArticleData&& article );
    bool ContainsArticle( const std::string& uid ) const;

    std::string m_icon;

    int m_numArticles;
    std::string m_title, m_description;
    std::vector<ArticleData> m_articles;

private:
    bool ParseHtml( const char* data, char*& out );

    CURL* m_curl;

    mutable std::mutex m_stdoutLock;
    const char* m_unit;

    std::string m_feedUrl, m_sourceUrl;
};

#endif
