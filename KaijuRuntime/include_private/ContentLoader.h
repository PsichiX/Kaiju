#ifndef ___CONTENT_LOADER___
#define ___CONTENT_LOADER___

#include <string>
#include <vector>
#include <stack>
#include <program.h>

namespace Kaiju
{
    class ContentLoader : public Compiler::ContentLoader
    {
    public:
        virtual ~ContentLoader() {};

        Compiler::Program* onContentLoad( const std::string& path, std::string& errors );

        std::vector< std::string > paths;
        std::stack< std::string > history;
        std::vector< std::string > usedPaths;
    };
}

#endif
