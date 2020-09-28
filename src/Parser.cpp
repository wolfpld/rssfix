#include <string.h>
#include <tidy.h>
#include <tidybuffio.h>

#include "Parser.hpp"

bool ParseHtml( const char* data, const char*& out )
{
    TidyBuffer err = {};
    tidyBufInit( &err );

    TidyDoc td = tidyCreate();
    tidyOptSetBool( td, TidyXhtmlOut, yes );
    tidyOptSetBool( td, TidyBodyOnly, yes );
    tidyOptSetBool( td, TidyLowerLiterals, yes );
    tidySetErrorBuffer( td, &err );

    if( tidyParseString( td, data ) == 2 )
    {
        out = strdup( (const char*)err.bp );
        tidyBufFree( &err );
        tidyRelease( td );
        return false;
    }

    TidyBuffer buf = {};
    tidyBufInit( &buf );
    tidyCleanAndRepair( td );
    tidySaveBuffer( td, &buf );

    out = strdup( (const char*)buf.bp );

    tidyBufFree( &buf );
    tidyBufFree( &err );
    tidyRelease( td );
    return true;
}
