#include <stdio.h>
#include <unistd.h>

#include "Color.hpp"
#include "Curl.hpp"
#include "Engine.hpp"
#include "Version.hpp"

#include "../contrib/ini/ini.h"

void PrintHelp()
{
    printf( "\nUsage:\n" );
    printf( "   -h              this help message\n" );
    printf( "   -c config.ini   set path to configuration file\n" );
}

int main( int argc, char** argv )
{
    printf( BOLDYELLOW "RssFix %i.%i.%i" RESET "\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );
    fflush( stdout );

    const char* configFile = "../config.ini";

    int c;
    while( ( c = getopt( argc, argv, "hc:" ) ) != -1 )
    {
        switch( c )
        {
        case 'h':
            PrintHelp();
            return 0;
        case 'c':
            configFile = optarg;
            break;
        default:
            PrintHelp();
            return -2;
        }
    }

    auto config = ini_load( configFile );
    if( !config )
    {
        fprintf( stderr, BOLDRED "Unable to open config file %s!" RESET "\n", configFile );
        fflush( stderr );
        return -3;
    }

    auto curl = Curl::Initialize();
    if( !curl )
    {
        fprintf( stderr, BOLDRED "Unable to initialize libcurl!" RESET "\n" );
        fflush( stderr );
        ini_free( config );
        return -1;
    }

    Engine engine;
    if( !engine.Initialize( config ) )
    {
        fprintf( stderr, BOLDRED "RssFix initialization failed!" RESET "\n" );
        fflush( stderr );
        ini_free( config );
        return -4;
    }

    engine.RunServer();

    ini_free( config );
    return 0;
}
