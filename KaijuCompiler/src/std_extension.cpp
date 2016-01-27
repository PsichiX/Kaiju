#include "../include/std_extension.h"
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace std
{

#ifdef __USE_STD_EXTENSION__TO_STRING__

    string to_string( int val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( long val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( long long val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( unsigned val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( unsigned long val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( unsigned long long val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( float val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( double val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( long double val )
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

#endif

    string string_replace( const string& val, const string& oldval, const string& newval )
    {
        if( val.empty() )
            return val;
        string r( val );
        size_t oldlen = oldval.length();
        size_t newlen = newval.length();
        size_t pos = r.find( oldval );
        while( pos != string::npos )
        {
            r = r.replace( pos, oldlen, newval );
            pos = r.find( oldval, pos + newlen );
        }
        return r;
    }

    string string_ltrim( const string &str )
    {
        string s = str;
        s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
        return s;
    }

    string string_rtrim( const string &str )
    {
        string s = str;
        s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
        return s;
    }

    string string_trim( const string &str )
    {
        string s = str;
        return string_ltrim(string_rtrim(s));
    }

}
