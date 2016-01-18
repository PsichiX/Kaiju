#ifndef __AST_GENERATOR_H__
#define __AST_GENERATOR_H__

#include "../include/ast.h"
#include <string>
#include <stack>
#include <map>

#define REGISTER_TYPE( type, id, final )    ASTGenerator::registerType( pegtl::internal::demangle< type >(), id, final )

namespace Kaiju
{
    namespace Compiler
    {

        class ASTGenerator
        {
        public:
            struct TypeInfo
            {
                TypeInfo() : acceptValue( false ) {};
                TypeInfo( const std::string& id, bool av ) : ruleId( id ), acceptValue( av ) {};

                std::string     ruleId;
                bool            acceptValue;
            };

            ASTGenerator() : m_root( 0 ) {};
            ~ASTGenerator() { release(); };

            static ASTGenerator*                        getInstance();
            static void                                 destroyInstance();
            static void                                 registerType( const std::string& ruleTypeName, const std::string& ruleId, bool ignoreValue );
            static void                                 unregisterType( const std::string& ruleTypeName );
            static bool                                 hasType( const std::string& ruleTypeName );
            static const TypeInfo&                      getType( const std::string& ruleTypeName );

            ASTNode*                                    getRoot() { return m_root; };

            bool                                        acquire( ASTNode* target, const char* data );
            void                                        release();
            void                                        pushNode( const std::string& type );
            void                                        popNode( const std::string& type );
            void                                        acceptNode( const std::string& type, const std::string& value, unsigned int line, unsigned int column, const char* begin, const char* end );
            void                                        assignUIDs();

        private:
            void                                        _assignUIDs( ASTNode* target, unsigned int& uidGenerator );

            static ASTGenerator*                        s_instance;
            static std::map< std::string, TypeInfo >    s_types;
            static TypeInfo                             s_unknownType;

            ASTNode*                                    m_root;
            std::stack< ASTNode* >                      m_stack;
            const char*                                 m_data;
        };

    }
}

#endif
