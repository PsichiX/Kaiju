#include <iostream>
#include <vector>
#include <Runtime.h>

int main( int argc, char** argv )
{
    std::string path = argc > 1 ? argv[ 1 ] : "";
    std::vector< std::string > args;
    for( int i = 1; i < argc; ++i )
        args.push_back( std::string( argv[ i ] ) );

    Kaiju::Runtime* runtime = new Kaiju::Runtime();
    int code = runtime->run( path, args );
    delete runtime;

    return code;
}
