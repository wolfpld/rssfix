#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <pugixml.hpp>
#include <string>
#include <vector>
#include <xxh3.h>

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
    const std::string& GetFeedUrl() const { return m_feedUrl; }
    const std::string& GetFeedUrlShort() const { return m_feedUrlShort; }
    size_t GetArticlesCount() const { return m_articles.size(); }
    size_t GetArticlesMax() const { return m_numArticles; }
    uint64_t GetTimestamp() const { return m_articles[0].timestamp; }
    const char* GetRefreshRate() const { return FormatTime( m_refresh ); }

    const std::string& GetFeed() const { return m_feedString; }

protected:
    virtual bool InitializeImpl( ini_t* config ) = 0;
    virtual bool FirstFetchImpl() = 0;

    std::vector<uint8_t> FetchPage( const char* url );
    bool PageHashChanged( const std::vector<uint8_t>& page );
    std::unique_ptr<pugi::xml_document> FetchDom( const std::vector<uint8_t>& page, bool tidy = true );
    void PrintDom( const std::unique_ptr<pugi::xml_document>& dom );

    void PrintStatus( bool status, const char* format, ... ) const;
    void PrintError( const char* context, const char* err, ... ) const;

    const char* FormatTime( int time ) const;

    void AddArticle( ArticleData&& article );
    bool ContainsArticle( const std::string& uid ) const;

    void FixupLink( pugi::xml_attribute attr, const char* baseUrl );

    std::string m_icon;

    int m_numArticles;
    int m_refresh;

    std::string m_title, m_description;
    std::vector<ArticleData> m_articles;

private:
    bool ParseHtml( const char* data, char*& out );
    void SortArticles();
    void CacheFeed();

    CURL* m_curl;
    XXH128_hash_t m_pageHash;

    mutable std::mutex m_stdoutLock;
    const char* m_unit;

    std::string m_feedUrl, m_feedUrlShort, m_sourceUrl;
    std::string m_feedString;
};

#endif
