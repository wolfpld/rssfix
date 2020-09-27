#ifndef __HANDLER_HPP__
#define __HANDLER_HPP__

struct ini_t;

class Handler
{
public:
    virtual bool Initialize( ini_t* config ) = 0;
    virtual bool FirstFetch() = 0;

protected:
    void PrintStatus( bool status, const char* format, ... ) const;
};

#endif
