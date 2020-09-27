#ifndef __APOD_HPP__
#define __APOD_HPP__

#include "Handler.hpp"

class Apod : public Handler
{
public:
    Apod();

    bool FirstFetch() final;

private:
    bool InitializeImpl( ini_t* config ) final;

    int m_numArticles;
};

#endif
