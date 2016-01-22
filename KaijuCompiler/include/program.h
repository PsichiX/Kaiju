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

        class ContentLoader
        {
        public:
            virtual ~ContentLoader() {};
            virtual Program* onContentLoad( const std::string& path, std::string& errors ) = 0;
        };

        class Convertible
        {
        public:
            Convertible( const std::string& t, Program* p, ASTNode* n ) : isValid( false ), program( p ), m_type( t ) {};
            virtual ~Convertible() { program = 0; isValid = false; };

            virtual bool convertToPST( std::stringstream& output, int level = 0 ) = 0;
            virtual bool convertToISC( std::stringstream& output ) = 0;
            std::string getErrors() { return m_errors.str(); };
            bool hasErrors() { return m_errors.rdbuf()->in_avail() != 0; };
            const std::string& getType() { return m_type; };

            bool isValid;

        protected:
            void appendError( ASTNode* node, const std::string& message );
            void appendError( Convertible* c ) { if( c ) m_errors << c->m_errors.str(); };

            Program* program;

        private:
            std::string m_type;
            std::stringstream m_errors;
        };

        class Program : public Convertible
        {
        public:
            class Class;
            class Value;

            Program( ASTNode* node, const std::string& input, ContentLoader* contentLoader = 0 );
            virtual ~Program();

            bool convertToPST( std::stringstream& output, int level = 0 );
            bool convertToISC( std::stringstream& output );
            unsigned int getUID() { return m_uid; };
            const std::string& constantInt( int v );
            const std::string& constantFloat( float v );
            const std::string& constantString( const std::string& v );
            int constantIntValue( const std::string& id );
            float constantFloatValue( const std::string& id );
            std::string constantStringValue( const std::string& id );
            unsigned int nextUIDpst() { return m_pstUidGenerator++; };
            std::string subInput(size_t start, size_t length) { return m_input ? m_input->substr(start, length) : ""; };
            bool loadContent( const std::string& path );
            bool absorbFrom( Program* p );
            Class* findClass( const std::string& id );

            unsigned int stackSize;
            unsigned int registersI;
            unsigned int registersF;
            std::map< int, std::string > constInts;
            std::map< float, std::string > constFloats;
            std::map< std::string, std::string > constStrings;
            std::map< std::string, Class* > classes;
            std::string entryPoint;

        private:
            static unsigned int s_uidGenerator;

            unsigned int m_uid;
            ContentLoader* m_contentLoader;
            std::string* m_input;
            unsigned int m_uidGenerator;
            unsigned int m_pstUidGenerator;

        public:
            class Directive : public Convertible
            {
            public:
                Directive( Program* p, ASTNode* n );
                virtual ~Directive();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                std::string id;
                std::vector< Value* > arguments;
            };

            class Variable : public Convertible
            {
            public:
                enum Type
                {
                    T_UNDEFINED,
                    T_DECLARATION,
                    T_ASSIGNMENT,
                    T_DECLARATION_ASSIGNMENT
                };

                Variable( Program* p, ASTNode* n );
                virtual ~Variable();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                Type type;
                bool isStatic;
                std::string id;
                Value* valueL;
                Value* valueR;
            };

            class Block : public Convertible
            {
            public:
                Block( Program* p, ASTNode* n, bool oneStatement = false );
                virtual ~Block();

                bool convertToPST( std::stringstream& output, int level = 0 );
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

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                Value* value;
            };

            class ControlFlowWhileLoop : public Convertible
            {
            public:
                ControlFlowWhileLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowWhileLoop();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                Value* condition;
                Block* statements;
            };

            class ControlFlowForLoop : public Convertible
            {
            public:
                ControlFlowForLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowForLoop();

                bool convertToPST( std::stringstream& output, int level = 0 );
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

                bool convertToPST( std::stringstream& output, int level = 0 );
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

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                std::vector< std::pair< Value*, Block* > > stages;
            };

            class ControlFlowReturn : public Convertible
            {
            public:
                ControlFlowReturn( Program* p, ASTNode* n );
                virtual ~ControlFlowReturn();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                Value* value;
            };

            class ControlFlowContinue : public Convertible
            {
            public:
                ControlFlowContinue( Program* p, ASTNode* n );
                virtual ~ControlFlowContinue() {};

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };
            };

            class ControlFlowBreak : public Convertible
            {
            public:
                ControlFlowBreak( Program* p, ASTNode* n );
                virtual ~ControlFlowBreak() {};

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };
            };

            class BinaryOperation : public Convertible
            {
            public:
                BinaryOperation( Program* p, ASTNode* n );
                virtual ~BinaryOperation();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                std::string type;
                Value* valueL;
                Value* valueR;
            };

            class UnaryOperation : public Convertible
            {
            public:
                UnaryOperation( Program* p, ASTNode* n );
                virtual ~UnaryOperation();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                std::string type;
                Value* value;
            };

            class Method : public Convertible
            {
            public:
                Method( Program* p, ASTNode* n );
                virtual ~Method();

                bool convertToPST( std::stringstream& output, int level = 0 );
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

                    bool convertToPST( std::stringstream& output, int level = 0 );
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
                    T_BINARY_OPERATION,
                    T_UNARY_OPERATION,
                    T_NUMBER_INT,
                    T_NUMBER_FLOAT,
                    T_STRING,
                    T_NULL,
                    T_IDENTIFIER
                };

                Value( Program* p, ASTNode* n );
                virtual ~Value();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };

                Type type;
                std::string id;
                Convertible* data;
                Value* accessValue;
            };

            class Class : public Convertible
            {
            public:
                Class( Program* p, ASTNode* n );
                virtual ~Class();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                void getFieldsList( std::vector< std::string >& out );

                std::string id;
                std::string inheritance;
                std::map< std::string, Variable* > fields;
                std::map< std::string, Method* > methods;

            private:
                static unsigned int s_uidGenerator;

                unsigned int m_uid;
            };
        };
    }
}
