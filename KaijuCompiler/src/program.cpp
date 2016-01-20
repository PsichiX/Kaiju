#include "../include/program.h"
#include "../include/std_extension.h"

namespace Kaiju
{
    namespace Compiler
    {
        Program::Program( ASTNode* n )
        : Convertible( this, n )
        , m_uidGenerator( 0 )
        {
            for( auto sn : n->nodes )
            {
                if( sn.type == "statement_outter" )
                {
                    if( sn.hasType( "directive.statement" ) )
                    {
                        Directive* d = new Directive( this, sn.findByType( "directive.statement" ) );
                        if( !d->isValid )
                        {
                            appendError( d );
                            Delete( d );
                            return;
                        }
                        statements.push_back( d );
                    }
                    else if( sn.hasType( "class.definition_statement" ) )
                    {
                        Class* c = new Class( this, sn.findByType( "class.definition_statement" ) );
                        if( !c->isValid )
                        {
                            appendError( c );
                            Delete( c );
                            return;
                        }
                        if( classes.count( c->id ) )
                        {
                            std::stringstream ss;
                            ss << "Class already exists: " << c->id;
                            appendError( &sn, ss.str() );
                            Delete( c );
                            return;
                        }
                        classes[ c->id ] = c;
                    }
                    else
                    {
                        appendError( &sn, "AST node is not either class definition statement or directive statement!" );
                        return;
                    }
                }
                else
                {
                    appendError( &sn, "AST node is not type of statement_outter!" );
                    return;
                }
            }
            isValid = true;
        }

        Program::~Program()
        {
            constInts.clear();
            constFloats.clear();
            constStrings.clear();
            for( auto& kv : classes )
                Delete( kv.second );
            classes.clear();
            for( auto s : statements )
                Delete( s );
            statements.clear();
            m_uidGenerator = 0;
        }

        const std::string& Program::constantInt( int v )
        {
            if( !constInts.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_INT_" << (m_uidGenerator++);
                constInts[ v ] = ss.str();
            }
            return constInts[ v ];
        }

        const std::string& Program::constantFloat( float v )
        {
            if( !constFloats.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_FLOAT_" << (m_uidGenerator++);
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
                ss << "__COSTANT_STRING_" << (m_uidGenerator++);
                constStrings[ s ] = ss.str();
            }
            return constStrings[ s ];
        }

        Program::Directive::Directive( Program* p, ASTNode* n )
        : Convertible( p, n )
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
            if( n->hasType( "directive.arguments_list" ) )
            {
                ASTNode* nal = n->findByType( "directive.arguments_list" );
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
            isValid = true;
        }

        Program::Directive::~Directive()
        {
            id.clear();
            for( auto a : arguments )
                Delete( a );
            arguments.clear();
        }

        Program::Variable::Variable( Program* p, ASTNode* n )
        : Convertible( p, n )
        , isStatic( false )
        , value( 0 )
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
                value = v;
                isValid = true;
            }
            else
            {
                appendError( n, "Variable is not either declaration or declaration with assignment!" );
                return;
            }
        }

        Program::Variable::~Variable()
        {
            isStatic = false;
            id.clear();
            Delete( value );
        }

        Program::Block::Block( Program* p, ASTNode* n, bool oneStatement )
        : Convertible( p, n )
        {
            if( oneStatement )
            {
                if( n->type != "statement_inner" )
                {
                    appendError( n, "AST node is not type of statement_inner!" );
                    return;
                }
                if( !processStatement( n ) );
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

        bool Program::Block::processStatement( ASTNode* n )
        {
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
                if( variables.count( v->id ) )
                {
                    appendError( nv, "Variable already declared in this scope!" );
                    Delete( v );
                    return false;
                }
                variables[ v->id ] = v;
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
                appendError( n, "AST node is not either block, directive statement, variable statement, method call!" );
                return false;
            }
            return true;
        }

        Program::ObjectDestruction::ObjectDestruction( Program* p, ASTNode* n )
        : Convertible( p, n )
        , value( 0 )
        {
            if( n->type != "object_destroy_statement" )
            {
                appendError( n, "AST node is not type of object_destroy_statement!" );
                return;
            }
            if( n->hasType( "value" ) )
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

        Program::ControlFlowWhileLoop::ControlFlowWhileLoop( Program* p, ASTNode* n )
        : Convertible( p, n )
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

        Program::ControlFlowForLoop::ControlFlowForLoop( Program* p, ASTNode* n )
        : Convertible( p, n )
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

        Program::ControlFlowForeachLoop::ControlFlowForeachLoop( Program* p, ASTNode* n )
        : Convertible( p, n )
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
            ASTNode* ns = n->findByType( "control_flow.foreach_state" );
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

        Program::ControlFlowCondition::ControlFlowCondition( Program* p, ASTNode* n )
        : Convertible( p, n )
        {
            if( n->type != "control_flow.foreach_statement" )
            {
                appendError( n, "AST node is not type of control_flow.foreach_statement!" );
                return;
            }
            for( auto ns : n->nodes )
            {

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

        Program::Method::Method( Program* p, ASTNode* n )
        : Convertible( p, n )
        , isStatic( false )
        {
            if( n->type != "class.method.definition_statement" )
            {
                appendError( n, "AST node is not type of class.method.definition_statement!" );
                return;
            }
            if( n->hasType( "variable.prefix" ) )
                isStatic = false;
            else if( n->hasType( "variable.prefix_static" ) )
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
            if( !n->hasType( "class.method.arguments_list" ) )
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
            id = nid->value + "__";
            ASTNode* nal = n->findByType( "class.method.arguments_list" );
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
            id += std::to_string( arguments.size() );
            Block* b = new Block( p, n->findByType( "block" ) );
            if( !b->isValid )
            {
                appendError( b );
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

        Program::Method::Call::Call( Program* p, ASTNode* n )
        : Convertible( p, n )
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
            if( !n->hasType( "class.method.call.arguments_list" ) )
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
            ASTNode* nal = n->findByType( "class.method.call.arguments_list" );
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

        Program::Value::Value( Program* p, ASTNode* n )
        : Convertible( p, n )
        , type( T_UNDEFINED )
        , methodCall( 0 )
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
                methodCall = c;
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
                methodCall = c;
                type = T_METHOD_CALL;
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
                appendError( n, "Value does not have either object creator, class method call, number, string, null value or identifier!" );
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
            Delete( methodCall );
            Delete( accessValue );
        }

        Program::Class::Class( Program* p, ASTNode* n )
        : Convertible( p, n )
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
            if( !n->hasType( "class.inheritance" ) )
            {
                appendError( n, "Class does not have inheritance specifier!" );
                return;
            }
            if( !n->hasType( "class.body" ) )
            {
                appendError( n, "Class does not have body!" );
                return;
            }
            ASTNode* nid = n->findByType( "identifier" );
            id = nid->value;
            ASTNode* nin = n->findByType( "class.inheritance" );
            if( !nin->hasType( "identifier" ) )
            {
                appendError( nin, "Class inheritance does not have identifier!" );
                return;
            }
            nid = nin->findByType( "identifier" );
            inheritance = nid->value;
            ASTNode* nb = n->findByType( "class.body" );
            for( auto nbs : nb->nodes )
            {
                if( nbs.type == "variable_statement" )
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
    }
}
