#include <map>
#include <string>
#include <sstream>
#include "ast.h"

namespace Kaiju
{
    namespace Compiler
    {
        template< typename T >
        static void Delete( T*& p ) { if( p ) delete p; p = 0; };

        class Program;

        class Convertible
        {
        public:
            Convertible( Program* p, ASTNode* n ) : program( p ), isValid( false ) {};
            virtual ~Convertible() { Delete( program ); isValid = false; };

            bool convertToISC( std::stringstream& output ) { return false; };
            std::string getErrors() { return m_errors.str(); };
            bool hasErrors() { return m_errors.rdbuf()->in_avail() != 0; };

            Program* program;
            bool isValid;

        protected:
            void appendError( ASTNode* node, const std::string& message ) { if( node ) { m_errors << "[" << node->line << "(" << node->column << "):" << node->position << "-" << (node->position + node->size) << "]" << std::endl << message << std::endl << std::endl; } else { m_errors << "[]" << std::endl << message << std::endl << std::endl; } };
            void appendError( Convertible* c ) { if( c ) m_errors << c->m_errors.str(); };

        private:
            std::stringstream m_errors;
        };

        class Program : public Convertible
        {
        public:
            class Class;
            class Variable;

            Program( ASTNode* n );
            virtual ~Program();

            bool convertToISC( std::stringstream& output ) { return false; };

            std::map< int, std::string > constInts;
            std::map< float, std::string > constFloats;
            std::map< std::string, std::string > constStrings;
            std::map< std::string, Class* > classes;

            class Variable : public Convertible
            {
            public:
                Variable( Program* p, ASTNode* n );
                virtual ~Variable();

                bool convertToISC( std::stringstream& output ) { return false; };
            };

            class Method : public Convertible
            {
            public:
                Method( Program* p, ASTNode* n );
                virtual ~Method();

                bool convertToISC( std::stringstream& output ) { return false; };
            };

            class Class : public Convertible
            {
            public:
                Class( Program* p, ASTNode* n );
                virtual ~Class();

                bool convertToISC( std::stringstream& output ) { return false; };

                std::map< std::string, Variable > fields;
                std::map< std::string, Method > methods;
            };
        };
    }
}
