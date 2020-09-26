#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>

#include "Color.hpp"
#include "OpenSslThreading.hpp"
#include "Version.hpp"

int main( int argc, char** argv )
{
    printf( BOLDYELLOW "RssFix %i.%i.%i" RESET "\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );

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
