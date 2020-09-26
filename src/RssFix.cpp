#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>

#include "OpenSslThreading.hpp"
#include "Version.hpp"

int main( int argc, char** argv )
{
    printf( "RssFix %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );

    OpenSslThreadInit();
    if( curl_global_init( CURL_GLOBAL_ALL ) != 0 )
    {
        fprintf( stderr, "Unable to initialize libcurl!\n" );
        return -1;
    }


    curl_global_cleanup();
    OpenSslThreadCleanup();
    return 0;
}
