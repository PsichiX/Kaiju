#ifndef __STD_EXTENSION_H__
#define __STD_EXTENSION_H__

#include <string>

namespace std
{

#ifdef __USE_STD_EXTENSION__TO_STRING__
    string to_string( int val );
    string to_string( long val );
    string to_string( long long val );
    string to_string( unsigned val );
    string to_string( unsigned long val );
    string to_string( unsigned long long val );
    string to_string( float val );
    string to_string( double val );
    string to_string( long double val );
#endif

    string string_replace( const string& val, const string& oldval, const string& newval );
    string string_ltrim( const string &str );
    string string_rtrim( const string &str );
    string string_trim( const string &str );

}

#endif
