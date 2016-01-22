#include <algorithm>
#include "../include/program.h"
#include "../include/std_extension.h"

namespace Kaiju
{
    namespace Compiler
    {
        void Convertible::appendError( ASTNode* node, const std::string& message )
        {
            if( node )
                m_errors << "[" << node->line << "(" << node->column << "):" << node->position << "-" << (node->position + node->size) << "]" << std::endl
                << message << std::endl << std::endl << program->subInput(node->position, node->size) << std::endl << std::endl;
            else
                m_errors << message << std::endl << std::endl;
        }

        unsigned int Program::s_uidGenerator = 0;

        Program::Program( ASTNode* node, const std::string& input, ContentLoader* contentLoader )
        : Convertible( "program", this, node )
        , stackSize( 8 * 1024 )
        , registersI( 8 )
        , registersF( 8 )
        , m_uid( s_uidGenerator++ )
        , m_contentLoader( contentLoader )
        , m_input( (std::string*)&input )
        , m_uidGenerator( 0 )
        , m_pstUidGenerator( 0 )
        {
            for( auto sn : node->nodes )
            {
                if( sn.type == "statement_outter" )
                {
                    if( sn.hasType( "directive.statement" ) )
                    {
                        ASTNode* nd = sn.findByType( "directive.statement" );
                        Directive* d = new Directive( this, nd );
                        if( !d->isValid )
                        {
                            appendError( d );
                            Delete( d );
                            m_input = 0;
                            return;
                        }
                        if( d->id != "entry" &&
                            d->id != "use" )
                        {
                            std::stringstream ss;
                            ss << "Unexpected directive: " << d->id;
                            appendError( nd, ss.str() );
                            Delete( d );
                            m_input = 0;
                            return;
                        }
                        Delete( d );
                    }
                    else if( sn.hasType( "class.definition_statement" ) )
                    {
                        Class* c = new Class( this, sn.findByType( "class.definition_statement" ) );
                        if( !c->isValid )
                        {
                            appendError( c );
                            Delete( c );
                            m_input = 0;
                            return;
                        }
                        if( classes.count( c->id ) )
                        {
                            std::stringstream ss;
                            ss << "Class already exists: " << c->id;
                            appendError( &sn, ss.str() );
                            Delete( c );
                            m_input = 0;
                            return;
                        }
                        classes[ c->id ] = c;
                    }
                    else
                    {
                        appendError( &sn, "AST node is not either class definition statement or directive statement!" );
                        m_input = 0;
                        return;
                    }
                }
                else
                {
                    appendError( &sn, "AST node is not type of statement_outter!" );
                    m_input = 0;
                    return;
                }
            }
            if( !entryPoint.empty() )
            {
                size_t f = entryPoint.find( '.' );
                if( f >= 0 )
                {
                    std::string cn = entryPoint.substr( 0, f );
                    std::string mn = entryPoint.substr( f + 1 );
                    if( !classes.count( cn ) )
                    {
                        std::stringstream ss;
                        ss << "Entry point class does not exists: " << cn;
                        appendError( 0, ss.str() );
                        return;
                    }
                    if( !classes[ cn ]->methods.count( mn ) )
                    {
                        std::stringstream ss;
                        ss << "Entry point method of class: " << cn << " does not exists: " << mn;
                        appendError( 0, ss.str() );
                        return;
                    }
                }
            }
            isValid = true;
            m_contentLoader = 0;
            m_input = 0;
        }

        Program::~Program()
        {
            stackSize = 0;
            registersI = 0;
            registersF = 0;
            constInts.clear();
            constFloats.clear();
            constStrings.clear();
            for( auto& kv : classes )
                Delete( kv.second );
            classes.clear();
            m_contentLoader = 0;
            m_input = 0;
            m_uidGenerator = 0;
            m_pstUidGenerator = 0;
            entryPoint.clear();
        }

        bool Program::convertToPST( std::stringstream& output, int level )
        {
            m_pstUidGenerator = 0;
            std::string lvl( level, '-' );
            output << "[" << nextUIDpst() << "]" << lvl << "(program)" << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(program.stackSize)" << stackSize << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(program.registersI)" << registersI << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(program.registersF)" << registersF << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(constInts)" << std::endl;
            for( auto& kv : constInts )
                output << "[" << nextUIDpst() << "]" << lvl << "--(" << kv.second << ")" << kv.first << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(constFloats)" << std::endl;
            for( auto& kv : constFloats )
                output << "[" << nextUIDpst() << "]" << lvl << "--(" << kv.second << ")" << kv.first << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(constStrings)" << std::endl;
            for( auto& kv : constStrings )
                output << "[" << nextUIDpst() << "]" << lvl << "--(" << kv.second << ")" << kv.first << std::endl;
            output << "[" << nextUIDpst() << "]" << lvl << "-(classes)" << std::endl;
            for( auto& kv : classes )
                kv.second->convertToPST( output, level + 2 );
            return true;
        }

        bool Program::convertToISC( std::stringstream& output )
        {
            output << "#!/usr/bin/env intuicio" << std::endl;
            output << "!intuicio" << std::endl;
            output << "!stack " << stackSize << std::endl;
            output << "!registers-i " << registersI << std::endl;
            output << "!registers-f " << registersF << std::endl;
            for( auto& kv : constInts )
                output << "!data int " << kv.second << " " << kv.first << std::endl;
            for( auto& kv : constFloats )
                output << "!data float " << kv.second << " " << kv.first << std::endl;
            for( auto& kv : constStrings )
                output << "!data bytes " << kv.second << " \"" << kv.first << "\", 0" << std::endl;
            for( auto& kv : classes )
                output << "!data address " << kv.first << "/type 0" << std::endl;
            output << "!start" << std::endl;
            output << "#counter _jump_ 0" << std::endl;
            output << "#counter _goto_ 0" << std::endl;
            output << "goto @___CODE_%_goto_%" << std::endl;
            output << "#increment _goto_" << std::endl;
            output << "!exit" << std::endl;
            for( auto& kv : classes )
                kv.second->convertToISC( output );
            output << "!start" << std::endl;
            output << "!jump ___CODE_%_jump_%" << std::endl;
            output << "#increment _jump_" << std::endl;
            if( !entryPoint.empty() )
            {
                // TODO: convert program arguments to array and push on stack (external data/stack).
                size_t f = entryPoint.find( '.' );
                output << "call @" << (f < 0 ? entryPoint : entryPoint.substr( 0, f ) + "/" + entryPoint.substr( f + 1 )) << std::endl;
                // TODO: pop returned value from stack and set it as application exit code (external data/stack).
            }
            output << "!exit" << std::endl;
            return true;
        }

        const std::string& Program::constantInt( int v )
        {
            if( !constInts.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_INT_" << m_uid << "_" << (m_uidGenerator++);
                constInts[ v ] = ss.str();
            }
            return constInts[ v ];
        }

        const std::string& Program::constantFloat( float v )
        {
            if( !constFloats.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_FLOAT_" << m_uid << "_" << (m_uidGenerator++);
                constFloats[ v ] = ss.str();
            }
            return constFloats[ v ];
        }

        const std::string& Program::constantString( const std::string& v )
        {
            std::string s = v.length() >= 2 && v[ 0 ] == '"' && v[ v.length() - 1 ] == '"' ? v.substr( 1, v.length() - 2 ) : v;
            if( !constStrings.count( s ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_STRING_" << m_uid << "_" << (m_uidGenerator++);
                constStrings[ s ] = ss.str();
            }
            return constStrings[ s ];
        }

        int Program::constantIntValue( const std::string& id )
        {
            for( auto& kv : constInts )
                if( kv.second == id )
                    return kv.first;
            return 0;
        };

        float Program::constantFloatValue( const std::string& id )
        {
            for( auto& kv : constFloats )
                if( kv.second == id )
                    return kv.first;
            return 0;
        };

        std::string Program::constantStringValue( const std::string& id )
        {
            for( auto& kv : constStrings )
                if( kv.second == id )
                    return kv.first;
            return "";
        };

        bool Program::loadContent( const std::string& path )
        {
            if( !m_contentLoader )
            {
                appendError( 0, "Content loader is not attached to compiler!" );
                return false;
            }
            std::string errors;
            Program* p = m_contentLoader->onContentLoad( path, errors );
            if( !p )
            {
                appendError( 0, errors );
                return false;
            }
            bool status = this->absorbFrom( p );
            Delete( p );
            return status;
        }

        bool Program::absorbFrom( Program* p )
        {
            if( !p )
            {
                appendError( 0, "There is no program data to absorb from!" );
                return false;
            }
            if( !p->entryPoint.empty() )
            {
                appendError( 0, "Cannot absorb program data which specifies entry point!" );
                return false;
            }
            if( p->stackSize > stackSize )
                stackSize = p->stackSize;
            if( p->registersI > registersI )
                registersI = p->registersI;
            if( p->registersF > registersF )
                registersF = p->registersF;
            for( auto& kv : p->constInts )
            {
                if( constInts.count( kv.first ) )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant int value found during program absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constInts[ kv.first ] = kv.second;
            }
            p->constInts.clear();
            for( auto& kv : p->constFloats )
            {
                if( constFloats.count( kv.first ) )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant float value found during programs absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constFloats[ kv.first ] = kv.second;
            }
            p->constFloats.clear();
            for( auto& kv : p->constStrings )
            {
                if( constStrings.count( kv.first ) )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant string value found during programs absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constStrings[ kv.first ] = kv.second;
            }
            p->constStrings.clear();
            for( auto& kv : p->classes )
            {
                if( classes.count( kv.first ) )
                {
                    std::stringstream ss;
                    ss << "Duplicate of class found during programs absorbing: " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                classes[ kv.first ] = kv.second;
            }
            p->classes.clear();
            return true;
        }

        Program::Class* Program::findClass( const std::string& id )
        {
            return classes.count( id ) ? classes[ id ] : 0;
        }

        Program::Directive::Directive( Program* p, ASTNode* n )
        : Convertible( "directive", p, n )
        {
            if( n->type != "directive.statement" )
            {
                appendError( n, "AST node is not type of directive statement!" );
                return;
            }
            if( !n->hasType( "identifier" ) )
            {
                appendError( n, "Directive does not have identifier!" );
                return;
            }
            ASTNode* nid = n->findByType( "identifier" );
            id = nid->value;
            if( n->hasType( "directive.argument_list" ) )
            {
                ASTNode* nal = n->findByType( "directive.argument_list" );
                for( auto nala : nal->nodes )
                {
                    if( nala.type != "value" )
                    {
                        appendError( &nala, "Directive argument is not type of value!" );
                        return;
                    }
                    Value* v = new Value( p, &nala );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    arguments.push_back( v );
                }
            }
            if( id == "entry" )
            {
                if( !program->entryPoint.empty() )
                {
                    appendError( n, "Program already have entry point defined!" );
                    return;
                }
                if( arguments.size() != 1 )
                {
                    appendError( n, "Entry point directive does not have exactly one argument!" );
                    return;
                }
                if( arguments[ 0 ]->type != Value::T_STRING )
                {
                    appendError( n, "Entry point directive first argument is not type of string!" );
                    return;
                }
                p->entryPoint = program->constantStringValue( arguments[ 0 ]->id );
            }
            else if( id == "use" )
            {
                if( arguments.size() != 1 )
                {
                    appendError( n, "Entry point directive does not have exactly one argument!" );
                    return;
                }
                if( arguments[ 0 ]->type != Value::T_STRING )
                {
                    appendError( n, "Entry point directive first argument is not type of string!" );
                    return;
                }
                std::string path = program->constantStringValue( arguments[ 0 ]->id );
                if( !program->loadContent( path ) )
                {
                    std::stringstream ss;
                    ss << "Cannot load program file content from path: " << path;
                    appendError( n, ss.str() );
                    return;
                }
            }
            else
            {
                appendError( n, "Unknown directive!" );
                return;
            }
            isValid = true;
        }

        Program::Directive::~Directive()
        {
            id.clear();
            for( auto a : arguments )
                Delete( a );
            arguments.clear();
        }

        bool Program::Directive::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(directive)" << id << std::endl;
            for( auto v : arguments )
                v->convertToPST( output, level + 1 );
            return true;
        }

        Program::Variable::Variable( Program* p, ASTNode* n )
        : Convertible( "variable", p, n )
        , type( T_UNDEFINED )
        , isStatic( false )
        , valueL( 0 )
        , valueR( 0 )
        {
            if( n->type != "variable" )
            {
                appendError( n, "AST node is not type of variable!" );
                return;
            }
            if( n->hasType( "variable.declaration" ) )
            {
                if( !n->hasType( "variable.declaration" ) )
                {
                    appendError( n, "Variable does not have declaration!" );
                    return;
                }
                ASTNode* nd = n->findByType( "variable.declaration" );
                if( nd->hasType( "variable.prefix" ) )
                    isStatic = false;
                else if( nd->hasType( "variable.prefix_static" ) )
                    isStatic = true;
                else
                {
                    appendError( nd, "Variable declaration does not specify if it's static or not!" );
                    return;
                }
                if( !nd->hasType( "identifier" ) )
                {
                    appendError( nd, "Variable declaration does not have identifier!" );
                    return;
                }
                ASTNode* nid = nd->findByType( "identifier" );
                id = nid->value;
                type = T_DECLARATION;
                isValid = true;
            }
            else if( n->hasType( "variable.assignment" ) )
            {
                if( !n->hasType( "variable.assignment" ) )
                {
                    appendError( n, "Variable does not have assignment!" );
                    return;
                }
                ASTNode* na = n->findByType( "variable.assignment" );
                if( na->nodes[ 0 ].type != "value" )
                {
                    appendError( &na->nodes[ 0 ], "Variable assignment first AST node is not a value!" );
                    return;
                }
                if( na->nodes[ 1 ].type != "value" )
                {
                    appendError( &na->nodes[ 1 ], "Variable assignment first AST node is not a value!" );
                    return;
                }
                Value* v = new Value( p, &na->nodes[ 0 ] );
                if( !v->isValid )
                {
                    appendError( v );
                    return;
                }
                valueL = v;
                v = new Value( p, &na->nodes[ 1 ] );
                if( !v->isValid )
                {
                    appendError( v );
                    return;
                }
                valueR = v;
                type = T_ASSIGNMENT;
                isValid = true;
            }
            else if( n->hasType( "variable.declaration_assignment" ) )
            {
                ASTNode* nda = n->findByType( "variable.declaration_assignment" );
                if( !nda->hasType( "variable.declaration" ) )
                {
                    appendError( nda, "Variable does not have declaration!" );
                    return;
                }
                if( !nda->hasType( "value" ) )
                {
                    appendError( nda, "Variable does not have value!" );
                    return;
                }
                ASTNode* nd = nda->findByType( "variable.declaration" );
                if( nd->hasType( "variable.prefix" ) )
                    isStatic = false;
                else if( nd->hasType( "variable.prefix_static" ) )
                    isStatic = true;
                else
                {
                    appendError( nd, "Variable declaration does not specify if it's static or not!" );
                    return;
                }
                if( !nd->hasType( "identifier" ) )
                {
                    appendError( nd, "Variable declaration does not have identifier!" );
                    return;
                }
                ASTNode* nid = nd->findByType( "identifier" );
                id = nid->value;
                Value* v = new Value( p, nda->findByType( "value" ) );
                if( !v->isValid )
                {
                    appendError( v );
                    Delete( v );
                    return;
                }
                valueR = v;
                type = T_DECLARATION_ASSIGNMENT;
                isValid = true;
            }
            else
            {
                appendError( n, "Variable is not either declaration, assignment or declaration with assignment!" );
                return;
            }
        }

        Program::Variable::~Variable()
        {
            isStatic = false;
            id.clear();
            Delete( valueL );
            Delete( valueR );
        }

        bool Program::Variable::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(variable)" << id << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(variable.static)" << isStatic << std::endl;
            if( valueL )
            {
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(valueLeft)" << std::endl;
                valueL->convertToPST( output, level + 1 );
            }
            if( valueR )
            {
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(valueRight)" << std::endl;
                valueR->convertToPST( output, level + 1 );
            }
            return true;
        }

        Program::Block::Block( Program* p, ASTNode* n, bool oneStatement )
        : Convertible( "block", p, n )
        {
            if( oneStatement )
            {
                if( n->type != "statement_inner" )
                {
                    appendError( n, "AST node is not type of statement_inner!" );
                    return;
                }
                if( !processStatement( n ) )
                    return;
            }
            else
            {
                if( n->type != "block" )
                {
                    appendError( n, "AST node is not type of block!" );
                    return;
                }
                for( auto sn : n->nodes )
                {
                    if( sn.type == "statement_inner" )
                    {
                        if( !processStatement( &sn ) )
                            return;
                    }
                    else
                    {
                        appendError( &sn, "AST node is not type of statement_inner!" );
                        return;
                    }
                }
            }
            isValid = true;
        }

        Program::Block::~Block()
        {
            variables.clear(); // do not destroy pointers contained here - they're part of statements!
            for( auto s : statements )
                Delete( s );
            statements.clear();
        }

        bool Program::Block::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(block)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(variables)" << std::endl;
            for( auto& kv : variables )
                kv.second->convertToPST( output, level + 2 );
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(statements)" << std::endl;
            for( auto s : statements )
                s->convertToPST( output, level + 2 );
            return true;
        }

        bool Program::Block::processStatement( ASTNode* n )
        {
            if( n->type != "statement_inner" )
            {
                appendError( n, "AST node is not type of statement_inner!" );
                return false;
            }
            if( n->hasType( "block" ) )
            {
                Block* b = new Block( program, n->findByType( "block" ) );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return false;
                }
                statements.push_back( b );
            }
            else if( n->hasType( "directive.statement" ) )
            {
                Directive* d = new Directive( program, n->findByType( "directive.statement" ) );
                if( !d->isValid )
                {
                    appendError( d );
                    Delete( d );
                    return false;
                }
                statements.push_back( d );
            }
            else if( n->hasType( "variable" ) )
            {
                ASTNode* nv = n->findByType( "variable" );
                Variable* v = new Variable( program, nv );
                if( !v->isValid )
                {
                    appendError( v );
                    Delete( v );
                    return false;
                }
                if( v->type == Variable::T_DECLARATION )
                {
                    if( variables.count( v->id ) )
                    {
                        appendError( nv, "Variable already declared in this scope!" );
                        Delete( v );
                        return false;
                    }
                    variables[ v->id ] = v;
                }
                statements.push_back( v );
            }
            else if( n->hasType( "object_destroy_statement" ) )
            {
                ObjectDestruction* d = new ObjectDestruction( program, n->findByType( "object_destroy_statement" ) );
                if( !d->isValid )
                {
                    appendError( d );
                    Delete( d );
                    return false;
                }
                statements.push_back( d );
            }
            else if( n->hasType( "control_flow.while_statement" ) )
            {
                ControlFlowWhileLoop* w = new ControlFlowWhileLoop( program, n->findByType( "control_flow.while_statement" ) );
                if( !w->isValid )
                {
                    appendError( w );
                    Delete( w );
                    return false;
                }
                statements.push_back( w );
            }
            else if( n->hasType( "control_flow.for_statement" ) )
            {
                ControlFlowForLoop* f = new ControlFlowForLoop( program, n->findByType( "control_flow.for_statement" ) );
                if( !f->isValid )
                {
                    appendError( f );
                    Delete( f );
                    return false;
                }
                statements.push_back( f );
            }
            else if( n->hasType( "control_flow.foreach_statement" ) )
            {
                ControlFlowForeachLoop* f = new ControlFlowForeachLoop( program, n->findByType( "control_flow.foreach_statement" ) );
                if( !f->isValid )
                {
                    appendError( f );
                    Delete( f );
                    return false;
                }
                statements.push_back( f );
            }
            else if( n->hasType( "control_flow.condition_statement" ) )
            {
                ControlFlowCondition* c = new ControlFlowCondition( program, n->findByType( "control_flow.condition_statement" ) );
                if( !c->isValid )
                {
                    appendError( c );
                    Delete( c );
                    return false;
                }
                statements.push_back( c );
            }
            else if( n->hasType( "control_flow.return_statement" ) )
            {
                ControlFlowReturn* r = new ControlFlowReturn( program, n->findByType( "control_flow.return_statement" ) );
                if( !r->isValid )
                {
                    appendError( r );
                    Delete( r );
                    return false;
                }
                statements.push_back( r );
            }
            else if( n->hasType( "control_flow.continue_statement" ) )
            {
                ControlFlowContinue* c = new ControlFlowContinue( program, n->findByType( "control_flow.continue_statement" ) );
                if( !c->isValid )
                {
                    appendError( c );
                    Delete( c );
                    return false;
                }
                statements.push_back( c );
            }
            else if( n->hasType( "control_flow.break_statement" ) )
            {
                ControlFlowBreak* b = new ControlFlowBreak( program, n->findByType( "control_flow.break_statement" ) );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return false;
                }
                statements.push_back( b );
            }
            else if( n->hasType( "class.method.call" ) )
            {
                Method::Call* c = new Method::Call( program, n->findByType( "class.method.call" ) );
                if( !c->isValid )
                {
                    appendError( c );
                    Delete( c );
                    return false;
                }
                statements.push_back( c );
            }
            else if( n->hasType( "value" ) )
            {
                Value* v = new Value( program, n->findByType( "value" ) );
                if( !v->isValid )
                {
                    appendError( v );
                    Delete( v );
                    return false;
                }
                statements.push_back( v );
            }
            else
            {
                appendError( n, "AST node is not either a code block, directive statement, variable statement, object destruction, while-loop, for-loop, foreach-loop, condition statement, return statement, continue statement, break statement, method call or value expression!" );
                return false;
            }
            return true;
        }

        Program::ObjectDestruction::ObjectDestruction( Program* p, ASTNode* n )
        : Convertible( "objectDestruction", p, n )
        , value( 0 )
        {
            if( n->type != "object_destroy_statement" )
            {
                appendError( n, "AST node is not type of object_destroy_statement!" );
                return;
            }
            if( !n->hasType( "value" ) )
            {
                appendError( n, "Object destruction does not provide expression!" );
                return;
            }
            Value* v = new Value( p, n->findByType( "value" ) );
            if( !v->isValid )
            {
                appendError( v );
                Delete( v );
                return;
            }
            value = v;
            isValid = true;
        }

        Program::ObjectDestruction::~ObjectDestruction()
        {
            Delete( value );
        }

        bool Program::ObjectDestruction::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(objectDestruction)" << std::endl;
            value->convertToPST( output, level + 1 );
            return true;
        }

        Program::ControlFlowWhileLoop::ControlFlowWhileLoop( Program* p, ASTNode* n )
        : Convertible( "whileLoop", p, n )
        , condition( 0 )
        , statements( 0 )
        {
            if( n->type != "control_flow.while_statement" )
            {
                appendError( n, "AST node is not type of control_flow.while_statement!" );
                return;
            }
            if( !n->hasType( "value" ) )
            {
                appendError( n, "While-loop does not have condition expression!" );
                return;
            }
            Value* v = new Value( p, n->findByType( "value" ) );
            if( !v->isValid )
            {
                appendError( v );
                Delete( v );
                return;
            }
            condition = v;
            if( n->hasType( "block" ) )
            {
                Block* b = new Block( p, n->findByType( "block" ) );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return;
                }
                statements = b;
            }
            else if( n->hasType( "statement_inner" ) )
            {
                Block* b = new Block( p, n->findByType( "statement_inner" ), true );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return;
                }
                statements = b;
            }
            else
            {
                appendError( n, "While-loop does not have code block!" );
                return;
            }
            isValid = true;
        }

        Program::ControlFlowWhileLoop::~ControlFlowWhileLoop()
        {
            Delete( condition );
            Delete( statements );
        }

        bool Program::ControlFlowWhileLoop::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(whileLoop)" << std::endl;
            condition->convertToPST( output, level + 1 );
            statements->convertToPST( output, level + 1 );
            return true;
        }

        Program::ControlFlowForLoop::ControlFlowForLoop( Program* p, ASTNode* n )
        : Convertible( "forLoop", p, n )
        , init( 0 )
        , condition( 0 )
        , iteration( 0 )
        , statements( 0 )
        {
            if( n->type != "control_flow.for_statement" )
            {
                appendError( n, "AST node is not type of control_flow.for_statement!" );
                return;
            }
            if( n->hasType( "control_flow.for_stage_init" ) )
            {
                ASTNode* ns = n->findByType( "control_flow.for_stage_init" );
                if( ns->hasType( "variable" ) )
                {
                    Variable* v = new Variable( p, ns->findByType( "variable" ) );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    init = v;
                }
                else
                {
                    appendError( ns, "For-loop init stage does not have variable!" );
                    return;
                }
            }
            if( n->hasType( "control_flow.for_stage_condition" ) )
            {
                ASTNode* ns = n->findByType( "control_flow.for_stage_condition" );
                if( ns->hasType( "value" ) )
                {
                    Value* v = new Value( p, ns->findByType( "value" ) );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    condition = v;
                }
                else
                {
                    appendError( ns, "For-loop condition stage does not have expression!" );
                    return;
                }
            }
            if( n->hasType( "control_flow.for_stage_iteration" ) )
            {
                ASTNode* ns = n->findByType( "control_flow.for_stage_iteration" );
                if( ns->hasType( "value" ) )
                {
                    Value* v = new Value( p, ns->findByType( "value" ) );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    iteration = v;
                }
                else
                {
                    appendError( ns, "For-loop iteration stage does not have expression!" );
                    return;
                }
            }
            if( n->hasType( "block" ) )
            {
                Block* b = new Block( p, n->findByType( "block" ) );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return;
                }
                statements = b;
            }
            else if( n->hasType( "statement_inner" ) )
            {
                Block* b = new Block( p, n->findByType( "statement_inner" ), true );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return;
                }
                statements = b;
            }
            else
            {
                appendError( n, "For-loop does not have code block!" );
                return;
            }
            isValid = true;
        }

        Program::ControlFlowForLoop::~ControlFlowForLoop()
        {
            Delete( init );
            Delete( condition );
            Delete( iteration );
            Delete( statements );
        }

        bool Program::ControlFlowForLoop::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(forLoop)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(init)" << std::endl;
            if( init )
                init->convertToPST( output, level + 2 );
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(condition)" << std::endl;
            if( condition )
                condition->convertToPST( output, level + 2 );
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(iteration)" << std::endl;
            if( iteration )
                iteration->convertToPST( output, level + 2 );
            statements->convertToPST( output, level + 1 );
            return true;
        }

        Program::ControlFlowForeachLoop::ControlFlowForeachLoop( Program* p, ASTNode* n )
        : Convertible( "foreachLoop", p, n )
        , collection( 0 )
        , statements( 0 )
        {
            if( n->type != "control_flow.foreach_statement" )
            {
                appendError( n, "AST node is not type of control_flow.foreach_statement!" );
                return;
            }
            if( !n->hasType( "control_flow.foreach_stage" ) )
            {
                appendError( n, "Foreach-loop does not have stage!" );
                return;
            }
            ASTNode* ns = n->findByType( "control_flow.foreach_stage" );
            if( !ns->hasType( "identifier" ) )
            {
                appendError( ns, "Foreach-loop does not have iterator identifier!" );
                return;
            }
            if( !ns->hasType( "value" ) )
            {
                appendError( ns, "Foreach-loop does not have collection!" );
                return;
            }
            ASTNode* nid = ns->findByType( "identifier" );
            iteratorId = nid->value;
            Value* v = new Value( p, ns->findByType( "value" ) );
            if( !v->isValid )
            {
                appendError( v );
                Delete( v );
                return;
            }
            collection = v;
            if( n->hasType( "block" ) )
            {
                Block* b = new Block( p, n->findByType( "block" ) );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return;
                }
                statements = b;
            }
            else if( n->hasType( "statement_inner" ) )
            {
                Block* b = new Block( p, n->findByType( "statement_inner" ), true );
                if( !b->isValid )
                {
                    appendError( b );
                    Delete( b );
                    return;
                }
                statements = b;
            }
            else
            {
                appendError( n, "For-loop does not have code block!" );
                return;
            }
            isValid = true;
        }

        Program::ControlFlowForeachLoop::~ControlFlowForeachLoop()
        {
            iteratorId.clear();
            Delete( collection );
            Delete( statements );
        }

        bool Program::ControlFlowForeachLoop::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(foreachLoop)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(foreachLoop.iterator)" << iteratorId << std::endl;
            collection->convertToPST( output, level + 1 );
            statements->convertToPST( output, level + 1 );
            return true;
        }

        Program::ControlFlowCondition::ControlFlowCondition( Program* p, ASTNode* n )
        : Convertible( "condition", p, n )
        {
            if( n->type != "control_flow.condition_statement" )
            {
                appendError( n, "AST node is not type of control_flow.foreach_statement!" );
                return;
            }
            for( auto ns : n->nodes )
            {
                if( ns.type == "control_flow.if_statement" )
                {
                    if( !ns.hasType( "value" ) )
                    {
                        appendError( &ns, "Condition does not have expression!" );
                        return;
                    }
                    Value* v = new Value( p, ns.findByType( "value" ) );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    Block* b = 0;
                    if( ns.hasType( "block" ) )
                        b = new Block( p, ns.findByType( "block" ) );
                    else if( ns.hasType( "statement_inner" ) )
                        b = new Block( p, ns.findByType( "statement_inner" ), true );
                    else
                    {
                        appendError( &ns, "Condition does not have code block!" );
                        Delete( v );
                        return;
                    }
                    if( !b->isValid )
                    {
                        appendError( b );
                        Delete( b );
                        Delete( v );
                        return;
                    }
                    stages.push_back( std::make_pair( v, b ) );
                }
                else if( ns.type == "control_flow.else_statement" )
                {
                    Block* b = 0;
                    if( ns.hasType( "block" ) )
                        b = new Block( p, ns.findByType( "block" ) );
                    else if( ns.hasType( "statement_inner" ) )
                        b = new Block( p, ns.findByType( "statement_inner" ), true );
                    else
                    {
                        appendError( &ns, "Condition does not have code block!" );
                        return;
                    }
                    if( !b->isValid )
                    {
                        appendError( b );
                        Delete( b );
                        return;
                    }
                    stages.push_back( std::make_pair( (Value*)0, b ) );
                }
                else
                {
                    std::stringstream ss;
                    ss << "Unknown condition stage type: " << ns.type;
                    appendError( &ns, ss.str() );
                    return;
                }
            }
            isValid = true;
        }

        Program::ControlFlowCondition::~ControlFlowCondition()
        {
            for( auto kv : stages )
            {
                Delete( kv.first );
                Delete( kv.second );
            }
            stages.clear();
        }

        bool Program::ControlFlowCondition::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(condition)" << std::endl;
            for( auto s : stages )
            {
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(stage)" << std::endl;
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(stage.condition)" << (bool)s.first << std::endl;
                if( s.first )
                    s.first->convertToPST( output, level + 2 );
                s.second->convertToPST( output, level + 1 );
            }
            return true;
        }

        Program::ControlFlowReturn::ControlFlowReturn( Program* p, ASTNode* n )
        : Convertible( "return", p, n )
        , value( 0 )
        {
            if( n->type != "control_flow.return_statement" )
            {
                appendError( n, "AST node is not type of control_flow.return_statement!" );
                return;
            }
            if( n->hasType( "value" ) )
            {
                Value* v = new Value( p, n->findByType( "value" ) );
                if( !v->isValid )
                {
                    appendError( v );
                    return;
                }
                value = v;
            }
            isValid = true;
        }

        Program::ControlFlowReturn::~ControlFlowReturn()
        {
            Delete( value );
        }

        bool Program::ControlFlowReturn::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(return)" << std::endl;
            if( value )
                value->convertToPST( output, level + 1 );
            return true;
        }

        Program::ControlFlowContinue::ControlFlowContinue( Program* p, ASTNode* n )
        : Convertible( "continue", p, n )
        {
            if( n->type != "control_flow.continue_statement" )
            {
                appendError( n, "AST node is not type of control_flow.continue_statement!" );
                return;
            }
            isValid = true;
        }

        bool Program::ControlFlowContinue::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(continue)" << std::endl;
            return true;
        }

        Program::ControlFlowBreak::ControlFlowBreak( Program* p, ASTNode* n )
        : Convertible( "break", p, n )
        {
            if( n->type != "control_flow.break_statement" )
            {
                appendError( n, "AST node is not type of control_flow.break_statement!" );
                return;
            }
            isValid = true;
        }

        bool Program::ControlFlowBreak::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(break)" << std::endl;
            return true;
        }

        Program::BinaryOperation::BinaryOperation( Program* p, ASTNode* n )
        : Convertible( "binaryOperation", p, n )
        , valueL( 0 )
        , valueR( 0 )
        {
            if( n->type != "operator.binary_operation" )
            {
                appendError( n, "AST node is not type of operator.binary_operation!" );
                return;
            }
            if( n->nodes[ 0 ].type != "value" )
            {
                appendError( &n->nodes[ 0 ], "Binary operation first AST node is not a value!" );
                return;
            }
            if( n->nodes[ 1 ].type != "operator.binary_operator" )
            {
                appendError( &n->nodes[ 1 ], "Binary operation second AST node is not a binary operator!" );
                return;
            }
            if( n->nodes[ 2 ].type != "value" )
            {
                appendError( &n->nodes[ 2 ], "Binary operation third AST node is not a value!" );
                return;
            }
            type = n->nodes[ 1 ].value;
            Value* v = new Value( p, &n->nodes[ 0 ] );
            if( !v->isValid )
            {
                appendError( v );
                Delete( v );
                return;
            }
            valueL = v;
            v = new Value( p, &n->nodes[ 2 ] );
            if( !v->isValid )
            {
                appendError( v );
                Delete( v );
                return;
            }
            valueR = v;
            isValid = true;
        }

        Program::BinaryOperation::~BinaryOperation()
        {
            type.clear();
            Delete( valueL );
            Delete( valueR );
        }

        bool Program::BinaryOperation::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(binaryOperation)" << type << std::endl;
            valueL->convertToPST( output, level + 1 );
            valueR->convertToPST( output, level + 1 );
            return true;
        }

        Program::UnaryOperation::UnaryOperation( Program* p, ASTNode* n )
        : Convertible( "unaryOperation", p, n )
        , value( 0 )
        {
            if( n->type != "operator.unary_operation" )
            {
                appendError( n, "AST node is not type of operator.binary_operation!" );
                return;
            }
            if( !n->hasType( "operator.unary_operator" ) )
            {
                appendError( n, "Unary operation does not have operator!" );
                return;
            }
            if( !n->hasType( "value" ) )
            {
                appendError( n, "Unary operation does not have value!" );
                return;
            }
            type = n->findByType( "operator.unary_operator" )->value;
            Value* v = new Value( p, n->findByType( "value" ) );
            if( !v->isValid )
            {
                appendError( v );
                Delete( v );
                return;
            }
            value = v;
            isValid = true;
        }

        Program::UnaryOperation::~UnaryOperation()
        {
            type.clear();
            Delete( value );
        }

        bool Program::UnaryOperation::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(unaryOperation)" << type << std::endl;
            value->convertToPST( output, level + 1 );
            return true;
        }

        Program::Method::Method( Program* p, ASTNode* n )
        : Convertible( "method", p, n )
        , isStatic( false )
        , statements( 0 )
        {
            if( n->type != "class.method.definition_statement" )
            {
                appendError( n, "AST node is not type of class.method.definition_statement!" );
                return;
            }
            if( n->hasType( "class.method.prefix" ) )
                isStatic = false;
            else if( n->hasType( "class.method.prefix_static" ) )
                isStatic = true;
            else
            {
                appendError( n, "Method definition does not specify if it's static or not!" );
                return;
            }
            if( !n->hasType( "identifier" ) )
            {
                appendError( n, "Method definition does not have identifier!" );
                return;
            }
            if( !n->hasType( "class.method.definition.argument_list" ) )
            {
                appendError( n, "Method definition does not have arguments list!" );
                return;
            }
            if( !n->hasType( "block" ) )
            {
                appendError( n, "Method definition does not have body!" );
                return;
            }
            ASTNode* nid = n->findByType( "identifier" );
            id = nid->value;
            ASTNode* nal = n->findByType( "class.method.definition.argument_list" );
            for( auto nala : nal->nodes )
            {
                if( nala.type == "identifier" )
                    arguments.push_back( nala.value );
                else
                {
                    appendError( &nala, "Method argument is not an identifier!" );
                    return;
                }
            }
            Block* b = new Block( p, n->findByType( "block" ) );
            if( !b->isValid )
            {
                appendError( b );
                Delete( b );
                return;
            }
            statements = b;
            isValid = true;
        }

        Program::Method::~Method()
        {
            isStatic = false;
            id.clear();
            arguments.clear();
            Delete( statements );
        }

        bool Program::Method::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(method)" << id << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(method.static)" << isStatic << std::endl;
            for( auto a : arguments )
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(argument)" << a << std::endl;
            statements->convertToPST( output, level + 1 );
            return true;
        }

        Program::Method::Call::Call( Program* p, ASTNode* n )
        : Convertible( "methodCall", p, n )
        {
            if( n->type != "class.method.call" )
            {
                appendError( n, "AST node is not type of class.method.call!" );
                return;
            }
            if( !n->hasType( "field" ) )
            {
                appendError( n, "Method calling does not have method path!" );
                return;
            }
            if( !n->hasType( "class.method.call.argument_list" ) )
            {
                appendError( n, "Method calling does not have arguments list!" );
                return;
            }
            ASTNode* nf = n->findByType( "field" );
            for( auto nfi : nf->nodes )
            {
                if( nfi.type != "identifier" )
                {
                    appendError( &nfi, "Method calling path part does not have identifier!" );
                    return;
                }
                identifier.push_back( nfi.value );
            }
            ASTNode* nal = n->findByType( "class.method.call.argument_list" );
            for( auto nala : nal->nodes )
            {
                if( nala.type != "value" )
                {
                    appendError( &nala, "Method calling argument is not type of value!" );
                    return;
                }
                Value* v = new Value( p, &nala );
                if( !v->isValid )
                {
                    appendError( v );
                    return;
                }
                arguments.push_back( v );
            }
            isValid = true;
        }

        Program::Method::Call::~Call()
        {
            identifier.clear();
            for( auto a : arguments )
                Delete( a );
            arguments.clear();
        }

        bool Program::Method::Call::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(methodCall)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(identifierPath)" << std::endl;
            for( auto i : identifier )
                output << "[" << program->nextUIDpst() << "]" << lvl << "--(identifier)" << i << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(arguments)" << std::endl;
            for( auto a : arguments )
                a->convertToPST(  output, level + 2 );
            return true;
        }

        Program::Value::Value( Program* p, ASTNode* n )
        : Convertible( "value", p, n )
        , type( T_UNDEFINED )
        , data( 0 )
        , accessValue( 0 )
        {
            if( n->type != "value" )
            {
                appendError( n, "AST node is not type of value!" );
                return;
            }
            if( n->hasType( "object_create" ) )
            {
                ASTNode* nc = n->findByType( "object_create" );
                if( !nc->hasType( "class.method.call" ) )
                {
                    appendError( nc, "Object creation does not provide class constructor call!" );
                    return;
                }
                Method::Call* c = new Method::Call( p, nc->findByType( "class.method.call" ) );
                if( !c->isValid )
                {
                    appendError( c );
                    Delete( c );
                    return;
                }
                data = c;
                type = T_OBJECT_CREATE;
            }
            else if( n->hasType( "class.method.call" ) )
            {
                Method::Call* c = new Method::Call( p, n->findByType( "class.method.call" ) );
                if( !c->isValid )
                {
                    appendError( c );
                    Delete( c );
                    return;
                }
                data = c;
                type = T_METHOD_CALL;
            }
            else if( n->hasType( "operator.binary_operation" ) )
            {
                BinaryOperation* o = new BinaryOperation( p, n->findByType( "operator.binary_operation" ) );
                if( !o->isValid )
                {
                    appendError( o );
                    Delete( o );
                    return;
                }
                data = o;
                type = T_BINARY_OPERATION;
            }
            else if( n->hasType( "operator.unary_operation" ) )
            {
                UnaryOperation* o = new UnaryOperation( p, n->findByType( "operator.unary_operation" ) );
                if( !o->isValid )
                {
                    appendError( o );
                    Delete( o );
                    return;
                }
                data = o;
                type = T_UNARY_OPERATION;
            }
            else if( n->hasType( "number" ) )
            {
                ASTNode* nn = n->findByType( "number" );
                if( nn->hasType( "number.integer_literal" ) )
                {
                    ASTNode* nni = nn->findByType( "number.integer_literal" );
                    std::stringstream is( nni->value );
                    int v = 0;
                    if( !(is >> v) )
                    {
                        std::stringstream ss;
                        ss << "Could not convert value to integer: " << nni->value;
                        appendError( nni, ss.str() );
                        return;
                    }
                    id = p->constantInt( v );
                    type = T_NUMBER_INT;
                }
                else if( nn->hasType( "number.float_literal" ) )
                {
                    ASTNode* nnf = nn->findByType( "number.float_literal" );
                    std::stringstream is( nnf->value );
                    float v = 0;
                    if( !(is >> v) )
                    {
                        std::stringstream ss;
                        ss << "Could not convert value to integer: " << nnf->value;
                        appendError( nnf, ss.str() );
                        return;
                    }
                    id = p->constantInt( v );
                    type = T_NUMBER_FLOAT;
                }
                else
                {
                    appendError( nn, "Value number is not either integer or float!" );
                    return;
                }
            }
            else if( n->hasType( "string" ) )
            {
                ASTNode* ns = n->findByType( "string" );
                std::string v = ns->value.size() > 1 ? ns->value.substr( 1, ns->value.size() - 2 ) : ns->value;
                id = p->constantString( v );
                type = T_STRING;
            }
            else if( n->hasType( "null_value" ) )
            {
                type = T_NULL;
            }
            else if( n->hasType( "identifier" ) )
            {
                ASTNode* nid = n->findByType( "identifier" );
                id = nid->value;
                type = T_IDENTIFIER;
            }
            else
            {
                appendError( n, "Value does not have either object creator, class method call, binary operation, unary operation, number, string, null value or identifier!" );
                return;
            }
            if( n->hasType( "access_value" ) )
            {
                ASTNode* nav = n->findByType( "access_value" );
                if( !nav->hasType( "value" ) )
                {
                    appendError( nav, "Value access value does not have value!" );
                    return;
                }
                Value* v = new Value( p, nav->findByType( "value" ) );
                if( !v->isValid )
                {
                    appendError( v );
                    return;
                }
                accessValue = v;
            }
            isValid = true;
        }

        Program::Value::~Value()
        {
            type = T_UNDEFINED;
            id.clear();
            Delete( data );
            Delete( accessValue );
        }

        bool Program::Value::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(value)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(value.type)" << type << std::endl;
            if( !id.empty() )
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(value.id)" << id << std::endl;
            if( data )
                data->convertToPST( output, level + 1 );
            if( accessValue )
                accessValue->convertToPST( output, level + 1 );
            return true;
        }

        unsigned int Program::Class::s_uidGenerator = 0;

        Program::Class::Class( Program* p, ASTNode* n )
        : Convertible( "class", p, n )
        , m_uid( s_uidGenerator++ )
        {
            if( n->type != "class.definition_statement" )
            {
                appendError( n, "AST node is not type of class.definition_statement!" );
                return;
            }
            if( !n->hasType( "identifier" ) )
            {
                appendError( n, "Class does not have identifier!" );
                return;
            }
            if( !n->hasType( "class.body" ) )
            {
                appendError( n, "Class does not have body!" );
                return;
            }
            ASTNode* nid = n->findByType( "identifier" );
            id = nid->value;
            if( n->hasType( "class.inheritance" ) )
            {
                ASTNode* nin = n->findByType( "class.inheritance" );
                if( !nin->hasType( "identifier" ) )
                {
                    appendError( nin, "Class inheritance does not have identifier!" );
                    return;
                }
                nid = nin->findByType( "identifier" );
                inheritance = nid->value;
            }
            ASTNode* nb = n->findByType( "class.body" );
            for( auto nbs : nb->nodes )
            {
                if( nbs.type == "variable" )
                {
                    Variable* v = new Variable( p, &nbs );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    if( fields.count( v->id ) )
                    {
                        std::stringstream ss;
                        ss << "Field already exists: " << v->id << " for class: " << id;
                        appendError( &nbs, ss.str() );
                        Delete( v );
                        return;
                    }
                    fields[ v->id ] = v;
                }
                else if( nbs.type == "class.method.definition_statement" )
                {
                    Method* m = new Method( p, &nbs );
                    if( !m->isValid )
                    {
                        appendError( m );
                        Delete( m );
                        return;
                    }
                    if( methods.count( m->id ) )
                    {
                        std::stringstream ss;
                        ss << "Method already exists: " << m->id << " for class: " << id;
                        appendError( &nbs, ss.str() );
                        Delete( m );
                        return;
                    }
                    methods[ m->id ] = m;
                }
                else
                {
                    appendError( &nbs, "AST node is not either variable statement or class method definition statement!" );
                    return;
                }
            }
            if( inheritance.empty() )
                inheritance = "Object";
            isValid = true;
        }

        Program::Class::~Class()
        {
            id.clear();
            inheritance.clear();
            for( auto& kv : fields )
                Delete( kv.second );
            fields.clear();
            for( auto& kv : methods )
                Delete( kv.second );
            methods.clear();
        }

        bool Program::Class::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(class)" << id << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(class.inheritance)" << inheritance << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(fields)" << std::endl;
            for( auto& kv : fields )
                kv.second->convertToPST( output, level + 2 );
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(methods)" << std::endl;
            for( auto& kv : methods )
                kv.second->convertToPST( output, level + 2 );
            return true;
        }

        bool Program::Class::convertToISC( std::stringstream& output )
        {
            output << "!namespace " << id << std::endl;
            output << "!data bytes ___CLASS_NAME \"" << id << "\", 0" << std::endl;
            output << "!data int ___CLASS_NAMELEN " << (id.length() + 1) << std::endl;
            output << "!data int ___CLASS_UID " << m_uid << std::endl;
            for( auto& kv : fields )
            {
                output << "!data bytes ___FIELD_NAME_" << kv.first << " \"" << kv.first << "\", 0" << std::endl;
                output << "!data int ___FIELD_NAMELEN_" << kv.first << " " << (kv.first.length() + 1) << std::endl;
            }
            for( auto& kv : methods )
            {
                output << "!data bytes ___METHOD_NAME_" << kv.first << " \"" << kv.first << "\", 0" << std::endl;
                output << "!data int ___METHOD_NAMELEN_" << kv.first << " " << (kv.first.length() + 1) << std::endl;
            }
            output << "!struct-def ___FieldMetaInfo" << std::endl;
            output << "!field byte name 128" << std::endl;
            output << "!field int namelen 1" << std::endl;
            output << "!field int uid 1" << std::endl;
            output << "!field int static 1" << std::endl;
            output << "!field address owner 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!struct-def ___MethodMetaInfo" << std::endl;
            output << "!field byte name 128" << std::endl;
            output << "!field int namelen 1" << std::endl;
            output << "!field int uid 1" << std::endl;
            output << "!field int static 1" << std::endl;
            output << "!field address owner 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!struct-def ___ClassMetaInfo" << std::endl;
            output << "!field byte name 128" << std::endl;
            output << "!field int namelen 1" << std::endl;
            output << "!field int uid 1" << std::endl;
            output << "!field address inheritanceMetaInfo 1" << std::endl;
            output << "!field " << id << "/___FieldMetaInfo fields " << fields.size() << std::endl;
            output << "!field " << id << "/___MethodMetaInfo methods " << methods.size() << std::endl;
            output << "!struct-end" << std::endl;
            output << "!struct-def ___Data" << std::endl;
            output << "!field address ___classMetaInfo 1" << std::endl;
            std::vector< std::string > fl;
            getFieldsList( fl );
            for( auto n : fl )
                output << "!field address " << n << " 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!start" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___CODE_%_jump_%" << std::endl;
            output << "#increment _jump_" << std::endl;
            output << "!namespace " << id << std::endl;
            output << "!data int ___count 1" << std::endl;
            output << "movi regi:0 $" << id << "/___count" << std::endl;
            output << "new $" << id << "/type " << id << "/___ClassMetaInfo 0" << std::endl;
            output << "movi regi:0 $" << id << "/___CLASS_NAMELEN" << std::endl;
            output << "movi :$" << id << "/type->" << id << "/___ClassMetaInfo.namelen $" << id << "/___CLASS_NAMELEN" << std::endl;
            output << "mov :$" << id << "/type->" << id << "/___ClassMetaInfo.name $" << id << "/___CLASS_NAME byte 0" << std::endl;
            output << "movi :$" << id << "/type->" << id << "/___ClassMetaInfo.uid $" << id << "/___CLASS_UID" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "goto @___CODE_%_goto_%" << std::endl;
            output << "#increment _goto_" << std::endl;
            output << "!exit" << std::endl;
            return true;
        }

        void Program::Class::getFieldsList( std::vector< std::string >& out )
        {
            Class* ic = program->findClass( inheritance );
            if( ic )
                ic->getFieldsList( out );
            for( auto& kv : fields )
                if( std::find( out.begin(), out.end(), kv.first ) == out.end() )
                    out.push_back( kv.first );
        }
    }
}
