#include <stdio.h>

#include "Version.hpp"

int main( int argc, char** argv )
{
    printf( "RssFix %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );

    return 0;
}
