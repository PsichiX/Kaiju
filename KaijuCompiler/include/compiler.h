#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "ast.h"
#include <sstream>

namespace Kaiju
{
    namespace Compiler
    {

        bool compileToAST( const std::string& input, ASTNode& output, std::stringstream& log );
        void convertASTNodeToIRVT( ASTNode& input, std::stringstream& output );

    }
}

#endif
