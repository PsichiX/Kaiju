#include "../include_private/ContentLoader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <compiler.h>
#include <cstdlib>

namespace Kaiju
{
    std::string getDirectoryPath( const std::string& path )
    {
        size_t f = path.find_last_of( "\\/" );
        return f < 0 ? path : path.substr( 0, f );
    }

    Compiler::Program* ContentLoader::onContentLoad( const std::string& path, std::string& errors )
    {
        std::ifstream file;
        std::string usedPath;
        file.open( path.c_str(), std::ifstream::in | std::ifstream::binary );
        if( file.is_open() )
            usedPath = path;
        else if( !history.empty() )
        {
            std::string p = history.top() + "/" + path;
            file.open( p.c_str() );
            if( file.is_open() )
                usedPath = p;
        }
        if( !file.is_open() )
        {
            std::string p;
            for( std::vector< std::string >::iterator it = paths.begin(); it != paths.end(); ++it )
            {
                p = *it + "/" + path;
                file.open( p.c_str(), std::ifstream::in | std::ifstream::binary );
                if( file.is_open() )
                {
                    usedPath = p;
                    break;
                }
            }
        }
        if( !file.is_open() )
            return 0;
        if( std::find( usedPaths.begin(), usedPaths.end(), usedPath ) != usedPaths.end() )
        {
            //std::stringstream ss;
            //ss << "Program under given path is already used: " << usedPath << std::endl;
            //errors = ss.str();
            return 0;
        }
        usedPaths.push_back( usedPath );
        std::string content;
        file.seekg( 0, std::ios::end );
        content.reserve( file.tellg() );
        file.seekg( 0, std::ios::beg );
        content.assign( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >() );
        file.close();
        Compiler::ASTNode ast;
        std::stringstream log;
        if( !Compiler::compileToAST( content, ast, log ) )
        {
            errors = log.str();
            return 0;
        }
        std::stringstream outputContent;
        history.push( getDirectoryPath( usedPath ) );
        Compiler::Program* program = new Compiler::Program( &ast, content, this );
        history.pop();
        ast.clear();
        if( !program->isValid )
        {
            errors = program->getErrors();
            Delete( program );
            return 0;
        }
        return program;
    }
}
