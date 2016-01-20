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
            class Value;

            Program( ASTNode* n );
            virtual ~Program();

            bool convertToISC( std::stringstream& output ) { return false; };
            const std::string& constantInt( int v );
            const std::string& constantFloat( float v );
            const std::string& constantString( const std::string& v );

            std::map< int, std::string > constInts;
            std::map< float, std::string > constFloats;
            std::map< std::string, std::string > constStrings;
            std::map< std::string, Class* > classes;
            std::vector< Convertible* > statements;

        private:
            unsigned int m_uidGenerator;

        public:
            class Directive : public Convertible
            {
            public:
                Directive( Program* p, ASTNode* n );
                virtual ~Directive();

                bool convertToISC( std::stringstream& output ) { return false; };

                std::string id;
                std::vector< Value* > arguments;
            };

            class Variable : public Convertible
            {
            public:
                Variable( Program* p, ASTNode* n );
                virtual ~Variable();

                bool convertToISC( std::stringstream& output ) { return false; };

                bool isStatic;
                std::string id;
                Value* value;
            };

            class Block : public Convertible
            {
            public:
                Block( Program* p, ASTNode* n, bool oneStatement = false );
                virtual ~Block();

                bool convertToISC( std::stringstream& output ) { return false; };

                std::map< std::string, Variable* > variables;
                std::vector< Convertible* > statements;

            private:
                bool processStatement( ASTNode* n );
            };

            class ObjectDestruction : public Convertible
            {
            public:
                ObjectDestruction( Program* p, ASTNode* n );
                virtual ~ObjectDestruction();

                bool convertToISC( std::stringstream& output ) { return false; };

                Value* value;
            };

            class ControlFlowWhileLoop : public Convertible
            {
            public:
                ControlFlowWhileLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowWhileLoop();

                bool convertToISC( std::stringstream& output ) { return false; };

                Value* condition;
                Block* statements;
            };

            class ControlFlowForLoop : public Convertible
            {
            public:
                ControlFlowForLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowForLoop();

                bool convertToISC( std::stringstream& output ) { return false; };

                Variable* init;
                Value* condition;
                Value* iteration;
                Block* statements;
            };

            class ControlFlowForeachLoop : public Convertible
            {
            public:
                ControlFlowForeachLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowForeachLoop();

                bool convertToISC( std::stringstream& output ) { return false; };

                std::string iteratorId;
                Value* collection;
                Block* statements;
            };

            class ControlFlowCondition : public Convertible
            {
            public:
                ControlFlowCondition( Program* p, ASTNode* n );
                virtual ~ControlFlowCondition();

                bool convertToISC( std::stringstream& output ) { return false; };

                std::vector< std::pair< Value*, Block* > > stages;
            };

            class ControlFlowReturn : public Convertible
            {
            public:
                ControlFlowReturn( Program* p, ASTNode* n );
                virtual ~ControlFlowReturn();

                bool convertToISC( std::stringstream& output ) { return false; };

                Value* value;
            };

            class ControlFlowContinue : public Convertible
            {
            public:
                ControlFlowContinue( Program* p, ASTNode* n );
                virtual ~ControlFlowContinue() {};

                bool convertToISC( std::stringstream& output ) { return false; };
            };

            class ControlFlowBreak : public Convertible
            {
            public:
                ControlFlowBreak( Program* p, ASTNode* n );
                virtual ~ControlFlowBreak() {};

                bool convertToISC( std::stringstream& output ) { return false; };
            };

            class Method : public Convertible
            {
            public:
                Method( Program* p, ASTNode* n );
                virtual ~Method();

                bool convertToISC( std::stringstream& output ) { return false; };

                bool isStatic;
                std::string id;
                std::vector< std::string > arguments;
                Block* statements;

                class Call : public Convertible
                {
                public:
                    Call( Program* p, ASTNode* n );
                    virtual ~Call();

                    bool convertToISC( std::stringstream& output ) { return false; };

                    std::vector< std::string > identifier;
                    std::vector< Value* > arguments;
                };
            };

            class Value : public Convertible
            {
            public:
                enum Type
                {
                    T_UNDEFINED,
                    T_OBJECT_CREATE,
                    T_METHOD_CALL,
                    T_NUMBER_INT,
                    T_NUMBER_FLOAT,
                    T_STRING,
                    T_NULL,
                    T_IDENTIFIER
                };

                Value( Program* p, ASTNode* n );
                virtual ~Value();

                bool convertToISC( std::stringstream& output ) { return false; };

                Type type;
                std::string id;
                Method::Call* methodCall;
                Value* accessValue;
            };

            class Class : public Convertible
            {
            public:
                Class( Program* p, ASTNode* n );
                virtual ~Class();

                bool convertToISC( std::stringstream& output ) { return false; };

                std::string id;
                std::string inheritance;
                std::map< std::string, Variable* > fields;
                std::map< std::string, Method* > methods;
            };
        };
    }
}
