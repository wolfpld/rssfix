#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "Color.hpp"
#include "OpenSslThreading.hpp"
#include "Version.hpp"

void PrintHelp()
{
    printf( "\nUsage:\n" );
    printf( "   -h              this help message\n" );
    printf( "   -c config.ini   set path to configuration file\n" );
}

int main( int argc, char** argv )
{
    printf( BOLDYELLOW "RssFix %i.%i.%i" RESET "\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );

    const char* configFile = "config.ini";

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

    OpenSslThreadInit();
    if( curl_global_init( CURL_GLOBAL_ALL ) != 0 )
    {
        fprintf( stderr, BOLDRED "Unable to initialize libcurl!" RESET "\n" );
        return -1;
    }


    curl_global_cleanup();
    OpenSslThreadCleanup();
    return 0;
}
