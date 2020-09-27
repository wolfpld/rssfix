#ifndef __APOD_HPP__
#define __APOD_HPP__

#include "Handler.hpp"

class Apod : public Handler
{
public:
    Apod();

    bool Initialize( ini_t* config ) final;
    bool FirstFetch() final;

private:
    int m_numArticles;
};

#endif
