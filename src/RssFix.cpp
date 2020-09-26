#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "Color.hpp"
#include "OpenSslThreading.hpp"
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
            return -2;
        }
    }

    auto config = ini_load( configFile );
    if( !config )
    {
        fprintf( stderr, BOLDRED "Unable to open config file %s!" RESET "\n", configFile );
        return -3;
    }

    OpenSslThreadInit();
    if( curl_global_init( CURL_GLOBAL_ALL ) != 0 )
    {
        fprintf( stderr, BOLDRED "Unable to initialize libcurl!" RESET "\n" );
        ini_free( config );
        return -1;
    }


    ini_free( config );
    curl_global_cleanup();
    OpenSslThreadCleanup();
    return 0;
}
