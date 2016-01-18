#include "../include/std_extension.h"

namespace std
{

#ifdef __USE_STD_EXTENSION__TO_STRING__

    string to_string( int val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( long val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( long long val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( unsigned val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( unsigned long val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( unsigned long long val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( float val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( double val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    string to_string( long double val )
    {
        std::stringstream ss;
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

}
