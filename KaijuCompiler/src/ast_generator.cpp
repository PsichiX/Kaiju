#include "../include_private/ast_generator.h"

namespace Kaiju
{
    namespace Compiler
    {

        ASTGenerator* ASTGenerator::s_instance = 0;
        std::map< std::string, ASTGenerator::TypeInfo > ASTGenerator::s_types = std::map< std::string, ASTGenerator::TypeInfo >();
        ASTGenerator::TypeInfo ASTGenerator::s_unknownType( "", false );

        ASTGenerator* ASTGenerator::getInstance()
        {
            if( s_instance )
                return s_instance;
            s_instance = new ASTGenerator();
            return s_instance;
        }

        void ASTGenerator::destroyInstance()
        {
            if( s_instance )
            {
                delete s_instance;
                s_instance = 0;
            }
            s_types.clear();
        }

        void ASTGenerator::registerType( const std::string& ruleTypeName, const std::string& ruleId, bool ignoreValue )
        {
            s_types[ ruleTypeName ] = TypeInfo( ruleId, !ignoreValue );
        }

        void ASTGenerator::unregisterType( const std::string& ruleTypeName )
        {
            if( s_types.count( ruleTypeName ) )
                s_types.erase( ruleTypeName );
        }

        bool ASTGenerator::hasType( const std::string& ruleTypeName )
        {
            return s_types.count( ruleTypeName );
        }

        const ASTGenerator::TypeInfo& ASTGenerator::getType( const std::string& ruleTypeName )
        {
            if( s_types.count( ruleTypeName ) )
                return s_types[ ruleTypeName ];
            else
                return s_unknownType;
        }

        bool ASTGenerator::acquire( ASTNode* target, const char* data )
        {
            release();
            if( !target )
                return false;
            m_root = target;
            m_stack.push( target );
            m_data = data;
            return true;
        }

        void ASTGenerator::release()
        {
            m_root = 0;
            m_data = 0;
        }

        void ASTGenerator::pushNode( const std::string& type )
        {
            if( !hasType( type ) )
                return;
            if( m_stack.empty() )
                return;
            ASTNode* top = m_stack.top();
            if( !top )
                return;
            const TypeInfo& ti = getType( type );
            top->nodes.push_back( ASTNode( ti.ruleId ) );
            m_stack.push( &top->nodes.back() );
        }

        void ASTGenerator::popNode( const std::string& type )
        {
            if( !hasType( type ) )
                return;
            m_stack.pop();
            if( m_stack.empty() )
                return;
            ASTNode* top = m_stack.top();
            const TypeInfo& ti = getType( type );
            if( !top || top->nodes.back().type != ti.ruleId )
                return;
            top->nodes.pop_back();
        }

        void ASTGenerator::acceptNode( const std::string& type, const std::string& value, unsigned int line, unsigned int column, const char* begin, const char* end )
        {
            if( !hasType( type ) )
                return;
            if( m_stack.empty() )
                return;
            ASTNode* top = m_stack.top();
            const TypeInfo& ti = getType( type );
            if( !top || top->type != ti.ruleId )
                return;
            if( ti.acceptValue )
                top->value = value;
            top->line = line;
            top->column = column;
            top->position = begin - m_data;
            top->size = end - begin;
            m_stack.pop();
        }

        void ASTGenerator::assignUIDs()
        {
            if( !m_root )
                return;
            unsigned int uidGenerator = 0;
            _assignUIDs( m_root, uidGenerator );
        }

        void ASTGenerator::_assignUIDs( ASTNode* target, unsigned int& uidGenerator )
        {
            if( !target )
                return;
            target->uid = ++uidGenerator;
            for( std::vector< ASTNode >::iterator it = target->nodes.begin(); it != target->nodes.end(); ++it )
                _assignUIDs( &(*it), uidGenerator );
        }

    }
}
