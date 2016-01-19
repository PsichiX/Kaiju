#include "../include/program.h"

namespace Kaiju
{
    namespace Compiler
    {
        Program::Program( ASTNode* n )
        : Convertible( this, n )
        {
        }

        Program::~Program()
        {
        }

        Program::Variable::Variable( Program* p, ASTNode* n )
        : Convertible( p, n )
        {
        }

        Program::Variable::~Variable()
        {
        }

        Program::Method::Method( Program* p, ASTNode* n )
        : Convertible( p, n )
        {
        }

        Program::Method::~Method()
        {
        }

        Program::Class::Class( Program* p, ASTNode* n )
        : Convertible( p, n )
        {
        }

        Program::Class::~Class()
        {
        }
    }
}
