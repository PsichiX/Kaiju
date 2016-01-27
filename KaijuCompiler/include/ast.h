#ifndef __AST_H__
#define __AST_H__

#include <string>
#include <vector>

namespace Kaiju
{
    namespace Compiler
    {

        struct ASTNode
        {
            ASTNode() : uid( 0 ), line( 0 ), column( 0 ), position( 0 ), size( 0 ) {};
            ASTNode( const std::string& t ) : uid( 0 ), type( t ), line( 0 ), column( 0 ), position( 0 ), size( 0 ) {};

            bool                    hasType( const std::string& t ) { for( std::vector< ASTNode >::iterator it = nodes.begin(); it != nodes.end(); ++it ) if( it->type == t ) return true; return false; };
            bool                    hasUID( unsigned int u ) { for( std::vector< ASTNode >::iterator it = nodes.begin(); it != nodes.end(); ++it ) if( it->uid == u ) return true; return false; };
            ASTNode*                findByType( const std::string& t ) { for( std::vector< ASTNode >::iterator it = nodes.begin(); it != nodes.end(); ++it ) if( it->type == t ) return &(*it); return 0; };
            ASTNode*                findByUID( unsigned int u ) { for( std::vector< ASTNode >::iterator it = nodes.begin(); it != nodes.end(); ++it ) if( it->uid == u ) return &(*it); return 0; };
            void                    clear() { uid = 0; type.clear(); line = 0; column = 0; position = 0; size = 0; value.clear(); nodes.clear(); };

            ASTNode*                operator[]( const std::string& t ) { return findByType( t ); };
            ASTNode*                operator[]( unsigned int u ) { return findByUID( u ); };

            unsigned int            uid;
            std::string             type;
            unsigned int            line;
            unsigned int            column;
            unsigned int            position;
            unsigned int            size;
            std::string             value;
            std::vector< ASTNode >  nodes;
        };

    }
}

#endif
