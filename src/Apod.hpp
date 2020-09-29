#ifndef __APOD_HPP__
#define __APOD_HPP__

#include "Handler.hpp"

class Apod : public Handler
{
public:
    Apod();

private:
    bool InitializeImpl( ini_t* config ) final;
    bool FirstFetchImpl() final;
};

#endif
