#include <stdarg.h>
#include <stdio.h>

#include "Color.hpp"
#include "Handler.hpp"

Handler::~Handler()
{
}

void Handler::PrintStatus( bool status, const char* format, ... ) const
{
    char tmp[4096];
    va_list args;
    va_start( args, format );
    vsnprintf( tmp, 4096, format, args );
    va_end( args );
    printf( BOLDWHITE "  [%s" BOLDWHITE "]" RESET " %s\n", status ? BOLDGREEN "✓" : BOLDRED "✗", tmp );
}
