#include <pthread.h>
#include <stdio.h>

#include "OpenSslThreading.hpp"
#include "Version.hpp"

int main( int argc, char** argv )
{
    printf( "RssFix %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );
    OpenSslThreadInit();

    OpenSslThreadCleanup();
    return 0;
}
