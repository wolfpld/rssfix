#include <stdio.h>
#include "../contrib/ini/ini.h"

#include "Color.hpp"
#include "Engine.hpp"

bool Engine::Initialize( ini_t* config )
{
    int enableApod = 0;
    ini_sget( config, "feeds", "apod", "%d", &enableApod );

    if( !enableApod )
    {
        fprintf( stderr, BOLDRED "No feeds enabled in configuration!" RESET "\n" );
        return false;
    }

    return true;
}
