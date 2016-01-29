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
            virtual void setProgram( Program* p ) = 0;

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
            const std::string& constantInt( int v );
            const std::string& constantFloat( float v );
            const std::string& constantString( const std::string& v );
            const std::string& constantHash( unsigned int v );
            int constantIntValue( const std::string& id );
            float constantFloatValue( const std::string& id );
            std::string constantStringValue( const std::string& id );
            unsigned int constantHashValue( const std::string& id );
            unsigned int nextUIDpst() { return m_pstUidGenerator++; };
            unsigned int nextUIDisc() { return m_iscUidGenerator++; };
            std::string subInput( size_t start, size_t length ) { return m_input ? m_input->substr(start, length) : ""; };
            bool loadContent( const std::string& path );
            bool absorbFrom( Program* p );
            Class* findClass( const std::string& id );
            virtual void setProgram( Program* p ) {};

            unsigned int stackSize;
            unsigned int registersI;
            unsigned int registersF;
            std::map< int, std::string > constInts;
            std::map< float, std::string > constFloats;
            std::map< std::string, std::string > constStrings;
            std::map< unsigned int, std::string > constHash;
            std::map< std::string, Class* > classes;
            std::string entryPoint;
            std::map< std::string, std::string > libraries;

        private:
            ContentLoader* m_contentLoader;
            std::string* m_input;
            unsigned int m_uidGenerator;
            unsigned int m_pstUidGenerator;
            unsigned int m_iscUidGenerator;

        public:
            class Directive : public Convertible
            {
            public:
                Directive( Program* p, ASTNode* n, Convertible* c = 0 );
                virtual ~Directive();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

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
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );
                unsigned int getUID() { return m_uid; };

                Type type;
                bool isStatic;
                std::string id;
                Value* valueL;
                Value* valueR;

            private:
                unsigned int m_uid;
            };

            class Block : public Convertible
            {
            public:
                Block( Program* p, ASTNode* n, bool oneStatement = false );
                Block( Program* p );
                virtual ~Block();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

                std::vector< std::string > variables;
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
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

                Value* value;
            };

            class ControlFlowWhileLoop : public Convertible
            {
            public:
                ControlFlowWhileLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowWhileLoop();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

                Value* condition;
                Block* statements;
            };

            class ControlFlowForLoop : public Convertible
            {
            public:
                ControlFlowForLoop( Program* p, ASTNode* n );
                virtual ~ControlFlowForLoop();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

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
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

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
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

                std::vector< std::pair< Value*, Block* > > stages;
            };

            class ControlFlowReturn : public Convertible
            {
            public:
                ControlFlowReturn( Program* p, ASTNode* n );
                virtual ~ControlFlowReturn();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

                Value* value;
            };

            class ControlFlowContinue : public Convertible
            {
            public:
                ControlFlowContinue( Program* p, ASTNode* n );
                virtual ~ControlFlowContinue() {};

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p ) { program = p; };
            };

            class ControlFlowBreak : public Convertible
            {
            public:
                ControlFlowBreak( Program* p, ASTNode* n );
                virtual ~ControlFlowBreak() {};

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p ) { program = p; };
            };

            class BinaryOperation : public Convertible
            {
            public:
                BinaryOperation( Program* p, ASTNode* n );
                virtual ~BinaryOperation();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return false; };
                virtual void setProgram( Program* p );

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
                virtual void setProgram( Program* p );

                std::string type;
                Value* value;
            };

            class Method : public Convertible
            {
            public:
                Method( Program* p, ASTNode* n );
                Method( Program* p, const std::string& i, const std::vector< std::string >& a, bool s );
                virtual ~Method();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );
                unsigned int getUID() { return m_uid; };

                bool isStatic;
                std::string id;
                std::vector< std::string > arguments;
                bool argumentsParams;
                Block* statements;

            private:
                unsigned int m_uid;

            public:
                class Call : public Convertible
                {
                public:
                    Call( Program* p, ASTNode* n );
                    virtual ~Call();

                    bool convertToPST( std::stringstream& output, int level = 0 );
                    bool convertToISC( std::stringstream& output ) { return convertToISC( output, false ); };
                    bool convertToISC( std::stringstream& output, bool isConstructor );
                    virtual void setProgram( Program* p );

                    std::string id;
                    std::string classId;
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
                    T_LIBRARY_CALL,
                    T_BINARY_OPERATION,
                    T_UNARY_OPERATION,
                    T_NUMBER_INT,
                    T_NUMBER_FLOAT,
                    T_STRING,
                    T_FALSE,
                    T_TRUE,
                    T_NULL,
                    T_TYPEOF,
                    T_FIELD,
                    T_IDENTIFIER
                };

                Value( Program* p, ASTNode* n );
                virtual ~Value();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output ) { return convertToISC( output, 0 ); };
                bool convertToISC( std::stringstream& output, int level, bool isLeft = false );
                virtual void setProgram( Program* p );

                Type type;
                std::string id;
                std::string classId;
                Convertible* data;
                Value* accessValue;

            public:
                class Typeof : public Convertible
                {
                public:
                    Typeof( Program* p, ASTNode* n );
                    virtual ~Typeof();

                    bool convertToPST( std::stringstream& output, int level = 0 );
                    bool convertToISC( std::stringstream& output );
                    virtual void setProgram( Program* p );

                    std::string classId;
                    Value* value;
                };
            };

            class LibraryCall : public Convertible
            {
            public:
                LibraryCall( Program* p, ASTNode* n  );
                virtual ~LibraryCall();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                virtual void setProgram( Program* p );

                Method::Call* call;
            };

            class Class : public Convertible
            {
            public:
                Class( Program* p, ASTNode* n );
                virtual ~Class();

                bool convertToPST( std::stringstream& output, int level = 0 );
                bool convertToISC( std::stringstream& output );
                bool convertToISC_StructDef( std::stringstream& output );
                void getFieldsList( std::vector< std::string >& out, bool isStatic = false );
                virtual void setProgram( Program* p );

                std::string id;
                std::string inheritance;
                std::map< std::string, Variable* > fields;
                std::map< std::string, std::pair< std::string, int > > atomFields;
                std::map< std::string, Method* > methods;

            private:
                unsigned int m_uid;
            };
        };
    }
}
