#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stack>
#include <algorithm>
#include <time.h>
#include <compiler.h>
#include <program.h>

enum Mode
{
    M_CMD,
    M_INPUT,
    M_OUTPUT,
    M_FORMAT_INPUT,
    M_FORMAT_OUTPUT,
    M_SILENT
};

enum Format
{
    F_UNKNOWN,
    F_AUTO,
    F_KJ,
    F_AST,
    F_PST,
    F_ISC
};

Format getFormatFromExtension( const std::string& path )
{
    size_t f = path.rfind( '.' );
    if( f < 0 )
        return F_UNKNOWN;
    std::string ext = path.substr( f );
    if( ext == ".ast" )
        return F_AST;
    else if( ext == ".pst" )
        return F_PST;
    else if( ext == ".isc" )
        return F_ISC;
    else if( ext == ".kj" )
        return F_KJ;
    return F_UNKNOWN;
}

std::string getDirectoryPath( const std::string& path )
{
    size_t f = path.find_last_of( "\\/" );
    return f < 0 ? path : path.substr( 0, f );
}

class ContentLoader : public Kaiju::Compiler::ContentLoader
{
public:
    virtual ~ContentLoader() {};

    Kaiju::Compiler::Program* onContentLoad( const std::string& path, std::string& errors )
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
            std::stringstream ss;
            ss << "Program under given path is already used: " << usedPath << std::endl;
            errors = ss.str();
            return 0;
        }
        usedPaths.push_back( usedPath );
        std::string content;
        file.seekg( 0, std::ios::end );
        content.reserve( file.tellg() );
        file.seekg( 0, std::ios::beg );
        content.assign( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >() );
        file.close();
        Kaiju::Compiler::ASTNode ast;
        std::stringstream log;
        if( !Kaiju::Compiler::compileToAST( content, ast, log ) )
        {
            errors = log.str();
            return 0;
        }
        std::stringstream outputContent;
        history.push( getDirectoryPath( usedPath ) );
        Kaiju::Compiler::Program* program = new Kaiju::Compiler::Program( &ast, content, this );
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

    std::vector< std::string > paths;
    std::stack< std::string > history;
    std::vector< std::string > usedPaths;
};

int main( int argc, char** argv )
{
    std::string input;
    std::string output;
    Format inputFormat = F_AUTO;
    Format outputFormat = F_AUTO;
    bool pauseOnExit = false;
    bool printOutput = false;
    bool silent = false;

    std::string arg;
    Mode mode = M_CMD;
    for( int i = 0; i < argc; ++i )
    {
        arg = argv[ i ];
        if( mode == M_CMD )
        {
            if( arg == "-i" || arg == "--input" )
                mode = M_INPUT;
            else if( arg == "-o" || arg == "--output" )
                mode = M_OUTPUT;
            else if( arg == "-if" || arg == "--input-format" )
                mode = M_FORMAT_INPUT;
            else if( arg == "-of" || arg == "--output-format" )
                mode = M_FORMAT_OUTPUT;
            else if( arg == "-poe" || arg == "--pause-on-exit" )
                pauseOnExit = true;
            else if( arg == "-po" || arg == "--print-output" )
                printOutput = true;
            else if( arg == "-s" || arg == "--silent" )
                silent = true;
        }
        else if( mode == M_INPUT )
        {
            input = arg;
            mode = M_CMD;
        }
        else if( mode == M_OUTPUT )
        {
            output = arg;
            mode = M_CMD;
        }
        else if( mode == M_FORMAT_INPUT )
        {
            if( arg == "kj")
                inputFormat = F_KJ;
            mode = M_CMD;
        }
        else if( mode == M_FORMAT_OUTPUT )
        {
            if( arg == "ast")
                outputFormat = F_AST;
            else if( arg == "pst")
                outputFormat = F_PST;
            else if( arg == "isc")
                outputFormat = F_ISC;
            mode = M_CMD;
        }
    }

    if( !silent )
        std::cout << "Kaiju Compiler v1.0" << std::endl;

    if( input.empty() )
    {
        std::cout << "Input file path is missing!" << std::endl;
        if( pauseOnExit ) std::getchar();
        return EXIT_FAILURE;
    }
    if( !silent )
    {
        std::cout << "Input file: " << input << std::endl;
        if( !output.empty() )
            std::cout << "Output file: " << output << std::endl;
    }

    std::ifstream inputFile( input.c_str(), std::ifstream::in | std::ifstream::binary );
    if( !inputFile.is_open() )
    {
        std::cout << "Input file cannot be open to read: " << input << std::endl;
        if( pauseOnExit ) std::getchar();
        return EXIT_FAILURE;
    }
    std::ofstream outputFile( output.empty() ? "" : output.c_str(), std::ofstream::out | std::ofstream::binary );
    if( !output.empty() && !outputFile.is_open() )
    {
        inputFile.close();
        std::cout << "Output file cannot be open to write: " << output << std::endl;
        if( pauseOnExit ) std::getchar();
        return EXIT_FAILURE;
    }

    std::string inputContent;
    inputFile.seekg( 0, std::ios::end );
    inputContent.reserve( inputFile.tellg() );
    inputFile.seekg( 0, std::ios::beg );
    inputContent.assign( std::istreambuf_iterator< char >( inputFile ), std::istreambuf_iterator< char >() );
    inputFile.close();

    if( inputFormat == F_AUTO )
        inputFormat = getFormatFromExtension( input );
    if( outputFormat == F_AUTO )
        outputFormat = getFormatFromExtension( output );

    clock_t start;
    clock_t stop;
    clock_t dt;
    double dtms;
    double dts;
    if( inputFormat == F_KJ )
    {
        if( outputFormat == F_AST || outputFormat == F_PST || outputFormat == F_ISC )
        {
            start = clock();
            if( !silent )
                std::cout << "Producing AST..." << std::endl;
            Kaiju::Compiler::ASTNode ast;
            std::stringstream log;
            if( Kaiju::Compiler::compileToAST( inputContent, ast, log ) )
            {
                if( !silent )
                    std::cout << log.str();
                std::stringstream outputContent;
                if( outputFormat == F_AST )
                {
                    Kaiju::Compiler::convertASTNodeToIRVT( ast, outputContent );
                    ast.clear();
                }
                else if( outputFormat == F_PST || outputFormat == F_ISC )
                {
                    stop = clock();
                    dt = stop - start;
                    dtms = ( dt * 1000.0 ) / CLOCKS_PER_SEC;
                    dts = dtms * 0.001;
                    start = stop;
                    if( !silent )
                    {
                        std::cout << "Done! Time consumed by processing: " << (int)dtms << "ms (" << dts << "s)" << std::endl;
                        std::cout << "Processing AST..." << std::endl;
                    }
                    ContentLoader* loader = new ContentLoader();
                    loader->history.push( getDirectoryPath( input ) );
                    loader->usedPaths.push_back( input );
                    Kaiju::Compiler::Program program( &ast, inputContent, loader );
                    Delete( loader );
                    stop = clock();
                    dt = stop - start;
                    dtms = ( dt * 1000.0 ) / CLOCKS_PER_SEC;
                    dts = dtms * 0.001;
                    if( !silent )
                        std::cout << "Done! Time consumed by processing: " << (int)dtms << "ms (" << dts << "s)" << std::endl;
                    ast.clear();
                    if( !program.isValid )
                    {
                        outputFile.close();
                        std::cout << "Invalid AST tree!" << std::endl;
                        if( program.hasErrors() )
                            std::cout << "Errors:" << std::endl << std::endl << program.getErrors();
                        if( pauseOnExit ) std::getchar();
                        return EXIT_FAILURE;
                    }
                    start = clock();
                    if( outputFormat == F_PST )
                    {
                        if( !silent )
                            std::cout << "Producing PST..." << std::endl;
                        program.convertToPST( outputContent );
                    }
                    else if( outputFormat == F_ISC )
                    {
                        if( !silent )
                            std::cout << "Producing ISC..." << std::endl;
                        program.convertToISC( outputContent );
                    }
                }
                else
                {
                    outputFile.close();
                    std::cout << "Unknown output format!" << std::endl;
                    if( pauseOnExit ) std::getchar();
                    return EXIT_FAILURE;
                }
                if( !output.empty() )
                {
                    outputFile << outputContent.str();
                    outputFile.close();
                }
                stop = clock();
                dt = stop - start;
                dtms = ( dt * 1000.0 ) / CLOCKS_PER_SEC;
                dts = dtms * 0.001;
                if( !silent )
                    std::cout << "Done! Time consumed by processing: " << (int)dtms << "ms (" << dts << "s)" << std::endl;
                if( printOutput )
                {
                    if( !silent )
                        std::cout << ">>>OUTPUT>>>" << std::endl;
                    std::cout << outputContent.str();
                    if( !silent )
                        std::cout << "<<<OUTPUT<<<" << std::endl;
                }
                if( pauseOnExit ) std::getchar();
                return EXIT_SUCCESS;
            }
            else
            {
                outputFile.close();
                std::cout << log.str();
                if( pauseOnExit ) std::getchar();
                return EXIT_FAILURE;
            }
        }
        else
        {
            outputFile.close();
            std::cout << "Unknown output format!" << std::endl;
            if( pauseOnExit ) std::getchar();
            return EXIT_FAILURE;
        }
    }
    else
    {
        outputFile.close();
        std::cout << "Unknown input format!" << std::endl;
        if( pauseOnExit ) std::getchar();
        return EXIT_FAILURE;
    }
}
