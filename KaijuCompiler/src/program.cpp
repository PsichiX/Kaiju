#include <iostream>
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

        Program::Program( ASTNode* node, const std::string& input, ContentLoader* contentLoader )
        : Convertible( "program", this, node )
        , stackSize( 8 * 1024 )
        , registersI( 8 )
        , registersF( 8 )
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
                size_t f = entryPoint.find( ':' );
                if( f >= 0 )
                {
                    std::string cn = string_trim(entryPoint.substr( 0, f ));
                    std::string mn = string_trim(entryPoint.substr( f + 1 ));
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
            constHash.clear();
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
            output << "[" << nextUIDpst() << "]" << lvl << "-(constHash)" << std::endl;
            for( auto& kv : constHash )
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
            output << "!external ___APPLICATION_ARGUMENTS_CSTR_PTR" << std::endl;
            output << "!external ___APPLICATION_RESULT_INT" << std::endl;
            output << "!data int ___ZERO 0" << std::endl;
            output << "!data float ___ZEROF 0.0" << std::endl;
            output << "!data address ___NULL 0" << std::endl;
            output << "!data bytes ___NULLS 0" << std::endl;
            output << "!data int ___ONE 1" << std::endl;
            output << "!data float ___ONEF 1.0" << std::endl;
            output << "!data bytes ___NEW_LINE 10, 0" << std::endl;
            output << "!data address ___valueL 0" << std::endl;
            output << "!data address ___valueR 0" << std::endl;
            for( auto& kv : constInts )
                output << "!data int " << kv.second << " " << kv.first << std::endl;
            for( auto& kv : constFloats )
                output << "!data float " << kv.second << " " << kv.first << std::endl;
            for( auto& kv : constStrings )
                output << "!data bytes " << kv.second << " \"" << kv.first << "\", 0" << std::endl;
            for( auto& kv : constHash )
                output << "!data address " << kv.second << " " << kv.first << std::endl;
            for( auto& kv : classes )
                output << "!data address " << kv.first << "/type 0" << std::endl;
            output << "!struct-def ___FieldMetaInfo" << std::endl;
            output << "!field byte name 128" << std::endl;
            output << "!field int namelen 1" << std::endl;
            output << "!field address uid 1" << std::endl;
            output << "!field int static 1" << std::endl;
            output << "!field int offset 1" << std::endl;
            output << "!field address owner 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!struct-def ___MethodMetaInfo" << std::endl;
            output << "!field byte name 128" << std::endl;
            output << "!field int namelen 1" << std::endl;
            output << "!field address uid 1" << std::endl;
            output << "!field int static 1" << std::endl;
            output << "!field int address 1" << std::endl;
            output << "!field address owner 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!struct-def ___ClassMetaInfo" << std::endl;
            output << "!field byte name 128" << std::endl;
            output << "!field int namelen 1" << std::endl;
            output << "!field address uid 1" << std::endl;
            output << "!field address inheritanceMetaInfo 1" << std::endl;
            output << "!field int fieldsCount 1" << std::endl;
            output << "!field int methodsCount 1" << std::endl;
            output << "!field address fields 1" << std::endl;
            output << "!field address methods 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!struct-def ___Atom" << std::endl;
            output << "!field address ___classMetaInfo 1" << std::endl;
            output << "!struct-end" << std::endl;
            output << "!start" << std::endl;
            output << "#counter _jump_ 0" << std::endl;
            output << "#counter _goto_ 0" << std::endl;
            output << "call @___CLASSES_META_INITIALIZATION" << std::endl;
            output << "goto @___CODE_%_goto_%" << std::endl;
            output << "#increment _goto_" << std::endl;
            output << "!exit" << std::endl;
            for( auto& kv : classes )
                kv.second->convertToISC( output );
            output << "!start" << std::endl;
            output << "!jump ___GET_BOOL_FROM_ATOM" << std::endl;
            output << "!namespace ___funcGetBoolFromAtom" << std::endl;
            output << "!data int value 0" << std::endl;
            output << "!data address this 0" << std::endl;
            output << "popi $value" << std::endl;
            output << "movi regi:0 $___ONE" << std::endl;
            output << "mobj $this" << std::endl;
            output << "mnew $this Bool/___Data 0 @Bool/___Finalizer" << std::endl;
            output << "mova :*$this->Bool/___Data.___classMetaInfo $Bool/type" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "call @Bool/___Creator" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "pshi $" << constantInt( 0 ) << std::endl;
            output << "call @Bool/Constructor" << std::endl;
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mdel $___valueL" << std::endl;
            output << "movi :*$this->Bool/___Data.___data $value" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___GET_INT_FROM_ATOM" << std::endl;
            output << "!namespace ___funcGetIntFromAtom" << std::endl;
            output << "!data int value 0" << std::endl;
            output << "!data address this 0" << std::endl;
            output << "popi $value" << std::endl;
            output << "movi regi:0 $___ONE" << std::endl;
            output << "mobj $this" << std::endl;
            output << "mnew $this Int/___Data 0 @Int/___Finalizer" << std::endl;
            output << "mova :*$this->Int/___Data.___classMetaInfo $Int/type" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "call @Int/___Creator" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "pshi $" << constantInt( 0 ) << std::endl;
            output << "call @Int/Constructor" << std::endl;
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mdel $___valueL" << std::endl;
            output << "movi :*$this->Int/___Data.___data $value" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___GET_FLOAT_FROM_ATOM" << std::endl;
            output << "!namespace ___funcGetFloatFromAtom" << std::endl;
            output << "!data float value 0.0" << std::endl;
            output << "!data address this 0" << std::endl;
            output << "popf $value" << std::endl;
            output << "movi regi:0 $___ONE" << std::endl;
            output << "mobj $this" << std::endl;
            output << "mnew $this Float/___Data 0 @Float/___Finalizer" << std::endl;
            output << "mova :*$this->Float/___Data.___classMetaInfo $Float/type" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "call @Float/___Creator" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "pshi $" << constantInt( 0 ) << std::endl;
            output << "call @Float/Constructor" << std::endl;
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mdel $___valueL" << std::endl;
            output << "movf :*$this->Float/___Data.___data $value" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___GET_STRING_FROM_ATOM" << std::endl;
            output << "!namespace ___funcGetStringFromAtom" << std::endl;
            output << "!data address value 0" << std::endl;
            output << "!data address this 0" << std::endl;
            output << "popa $value" << std::endl;
            output << "movi regi:0 $___ONE" << std::endl;
            output << "mobj $this" << std::endl;
            output << "mnew $this String/___Data 0 @String/___Finalizer" << std::endl;
            output << "mova :*$this->String/___Data.___classMetaInfo $String/type" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "call @String/___Creator" << std::endl;
            output << "mpsh $this" << std::endl;
            output << "pshi $" << constantInt( 0 ) << std::endl;
            output << "call @String/Constructor" << std::endl;
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mdel $___valueL" << std::endl;
            //output << "del :*$this->String/___Data.___data" << std::endl;
            //output << "new :*$this->String/___Data.___data " << std::endl;
            output << "mpsh $this" << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___FIND_HASHED_METHOD_OF" << std::endl;
            output << "!namespace ___funcFindHashedMethodOf" << std::endl;
            output << "!data address this 0" << std::endl;
            output << "!data address uid 0" << std::endl;
            output << "!data address ptr 0" << std::endl;
            output << "mpop $this" << std::endl;
            output << "popa $uid" << std::endl;
            output << "mova $ptr :*$this->Object/___Data.___classMetaInfo" << std::endl;
            output << "movi regi:0 :$ptr->___ClassMetaInfo.methodsCount" << std::endl;
            output << "mova $ptr :$ptr->___ClassMetaInfo.methods" << std::endl;
            output << "movi regi:2 $___ONE" << std::endl;
            output << "!jump ___begin" << std::endl;
            output << "movi regi:1 :$ptr->___MethodMetaInfo.static" << std::endl;
            output << "jifi 1 @___check @___nonstatic" << std::endl;
            output << "!jump ___nonstatic" << std::endl;
            output << "eadr 1 :$ptr->___MethodMetaInfo.uid $uid" << std::endl;
            output << "jifi 1 @___good @___check" << std::endl;
            output << "!jump ___good" << std::endl;
            output << "pshi :$ptr->___MethodMetaInfo.address" << std::endl;
            output << "ret" << std::endl;
            output << "!jump ___check" << std::endl;
            output << "jifi 0 @___next @___end" << std::endl;
            output << "!jump ___next" << std::endl;
            output << "sadr $ptr ___MethodMetaInfo 2" << std::endl;
            output << "deci 0 0" << std::endl;
            output << "goto @___begin" << std::endl;
            output << "!jump ___end" << std::endl;
            output << "pshi $___ZERO" << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___FIND_HASHED_FIELD_OF" << std::endl;
            output << "!namespace ___funcFindHashedFieldOf" << std::endl;
            output << "!data address this 0" << std::endl;
            output << "!data address uid 0" << std::endl;
            output << "!data address ptr 0" << std::endl;
            output << "mpop $this" << std::endl;
            output << "popa $uid" << std::endl;
            output << "mova $ptr :*$this->Object/___Data.___classMetaInfo" << std::endl;
            output << "movi regi:0 :$ptr->___ClassMetaInfo.fieldsCount" << std::endl;
            output << "mova $ptr :$ptr->___ClassMetaInfo.fields" << std::endl;
            output << "movi regi:2 $___ONE" << std::endl;
            output << "!jump ___begin" << std::endl;
            output << "movi regi:1 :$ptr->___FieldMetaInfo.static" << std::endl;
            output << "jifi 1 @___check @___nonstatic" << std::endl;
            output << "!jump ___nonstatic" << std::endl;
            output << "eadr 1 :$ptr->___FieldMetaInfo.uid $uid" << std::endl;
            output << "jifi 1 @___good @___check" << std::endl;
            output << "!jump ___good" << std::endl;
            output << "pshi :$ptr->___FieldMetaInfo.offset" << std::endl;
            output << "ret" << std::endl;
            output << "!jump ___check" << std::endl;
            output << "jifi 0 @___next @___end" << std::endl;
            output << "!jump ___next" << std::endl;
            output << "sadr $ptr ___FieldMetaInfo 2" << std::endl;
            output << "deci 0 0" << std::endl;
            output << "goto @___begin" << std::endl;
            output << "!jump ___end" << std::endl;
            output << "pshi $___ZERO" << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___CLASSES_META_INITIALIZATION" << std::endl;
            for( auto& kv : classes )
            {
                output << "movi regi:0 $___ONE" << std::endl;
                output << "new $" << kv.first << "/type ___ClassMetaInfo 0" << std::endl;
                output << "movi regi:0 $" << kv.first << "/___CLASS_FIELDS_COUNT" << std::endl;
                output << "new :$" << kv.first << "/type->___ClassMetaInfo.fields ___FieldMetaInfo 0" << std::endl;
                output << "movi regi:0 $" << kv.first << "/___CLASS_METHODS_COUNT" << std::endl;
                output << "new :$" << kv.first << "/type->___ClassMetaInfo.methods ___MethodMetaInfo 0" << std::endl;
            }
            output << "ret" << std::endl;
            output << "!jump ___CODE_%_jump_%" << std::endl;
            output << "#increment _jump_" << std::endl;
            if( !entryPoint.empty() )
            {
                // TODO: convert program arguments to array and push on stack (external data/stack).
                output << "mpsh $___NULL" << std::endl;
                output << "pshi $___ZERO" << std::endl;
                size_t f = entryPoint.find( ':' );
                output << "call @" << (f == std::string::npos ? entryPoint : string_trim(entryPoint.substr( 0, f )) + "/" + string_trim(entryPoint.substr( f + 1 ))) << std::endl;
                output << "mpop $___valueL" << std::endl;
                output << "mfin $___valueL" << std::endl;
                output << "mdel $___valueL" << std::endl;
                // TODO: pop returned value from stack and set it as application exit code (external data/stack).
            }
            output << "!jump ___PROGRAM_EXIT" << std::endl;
            for( auto& kv : classes )
            {
                output << "del :$" << kv.first << "/type->___ClassMetaInfo.fields" << std::endl;
                output << "del :$" << kv.first << "/type->___ClassMetaInfo.methods" << std::endl;
                output << "del $" << kv.first << "/type" << std::endl;
            }
            output << "!exit" << std::endl;
            return true;
        }

        const std::string& Program::constantInt( int v )
        {
            if( !constInts.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_INT_" << std::hash< int >()( v );
                constInts[ v ] = ss.str();
            }
            return constInts[ v ];
        }

        const std::string& Program::constantFloat( float v )
        {
            if( !constFloats.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_FLOAT_" << std::hash< float >()( v );
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
                ss << "__COSTANT_STRING_" << std::hash< std::string >()( v );
                constStrings[ s ] = ss.str();
            }
            return constStrings[ s ];
        }

        const std::string& Program::constantHash( unsigned int v )
        {
            if( !constHash.count( v ) )
            {
                std::stringstream ss;
                ss << "__COSTANT_HASH_" << std::hash< unsigned int >()( v );
                constHash[ v ] = ss.str();
            }
            return constHash[ v ];
        }

        int Program::constantIntValue( const std::string& id )
        {
            for( auto& kv : constInts )
                if( kv.second == id )
                    return kv.first;
            return 0;
        }

        float Program::constantFloatValue( const std::string& id )
        {
            for( auto& kv : constFloats )
                if( kv.second == id )
                    return kv.first;
            return 0;
        }

        std::string Program::constantStringValue( const std::string& id )
        {
            for( auto& kv : constStrings )
                if( kv.second == id )
                    return kv.first;
            return "";
        }

        unsigned int Program::constantHashValue( const std::string& id )
        {
            for( auto& kv : constHash )
                if( kv.second == id )
                    return kv.first;
            return 0;
        }

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
                if( errors.empty() )
                    return true;
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
                if( constInts.count( kv.first ) && constInts[ kv.first ] != kv.second )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant int value with different id found during program absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constInts[ kv.first ] = kv.second;
            }
            p->constInts.clear();
            for( auto& kv : p->constFloats )
            {
                if( constFloats.count( kv.first ) && constFloats[ kv.first ] != kv.second )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant int value with different id found during program absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constFloats[ kv.first ] = kv.second;
            }
            p->constFloats.clear();
            for( auto& kv : p->constStrings )
            {
                if( constStrings.count( kv.first ) && constStrings[ kv.first ] != kv.second )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant string value with different id found during program absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constStrings[ kv.first ] = kv.second;
            }
            p->constStrings.clear();
            for( auto& kv : p->constHash )
            {
                if( constHash.count( kv.first ) && constHash[ kv.first ] != kv.second )
                {
                    std::stringstream ss;
                    ss << "Duplicate of constant hash value with different id found during program absorbing: " << kv.second << " = " << kv.first;
                    appendError( 0, ss.str() );
                    return false;
                }
                constHash[ kv.first ] = kv.second;
            }
            p->constHash.clear();
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
                kv.second->setProgram( this );
            }
            p->classes.clear();
            return true;
        }

        Program::Class* Program::findClass( const std::string& id )
        {
            return classes.count( id ) ? classes[ id ] : 0;
        }

        unsigned int Program::Directive::s_uidGenerator = 0;

        Program::Directive::Directive( Program* p, ASTNode* n, Convertible* c )
        : Convertible( "directive", p, n )
        , m_uid( ++s_uidGenerator )
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
            else if( id == "atomField" && c )
            {
                if( arguments.size() != 3 )
                {
                    appendError( n, "Atom field directive does not have exactly three arguments!" );
                    return;
                }
                if( arguments[ 0 ]->type != Value::T_IDENTIFIER )
                {
                    appendError( n, "Atom field directive first argument is not type of identifier!" );
                    return;
                }
                if( arguments[ 1 ]->type != Value::T_IDENTIFIER )
                {
                    appendError( n, "Atom field directive second argument is not type of identifier!" );
                    return;
                }
                if( arguments[ 2 ]->type != Value::T_NUMBER_INT )
                {
                    appendError( n, "Atom field directive third argument is not type of integer number!" );
                    return;
                }
                std::string tid = arguments[ 0 ]->id;
                std::string nid = arguments[ 1 ]->id;
                if( tid != "int" && tid != "float" && tid != "byte" && tid != "address" )
                {
                    appendError( n, "Atom field directive type is not either int, float, byte or address!" );
                    return;
                }
                Class* cls = (Class*)c;
                if( cls->atomFields.count( nid ) )
                {
                    std::stringstream ss;
                    ss << "Class: " << cls->id << " already have an atom field: " << nid;
                    appendError( n, ss.str() );
                    return;
                }
                int v = program->constantIntValue( arguments[ 2 ]->id );
                if( v < 1 )
                {
                    appendError( n, "Atom field directive items count cannot be less than one!" );
                    return;
                }
                cls->atomFields[ nid ] = std::make_pair( tid, v );
            }
            else if( id == "inject" )
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
            }
            else if( id == "ensureType" )
            {
                if( arguments.size() != 2 )
                {
                    appendError( n, "Ensure type directive does not have exactly two argument!" );
                    return;
                }
                if( arguments[ 0 ]->type != Value::T_IDENTIFIER )
                {
                    appendError( n, "Entry point directive first argument is not type of identifier!" );
                    return;
                }
                if( arguments[ 1 ]->type != Value::T_IDENTIFIER )
                {
                    appendError( n, "Entry point directive second argument is not type of identifier!" );
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
            m_uid = 0;
        }

        bool Program::Directive::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(directive)" << id << std::endl;
            for( auto v : arguments )
                v->convertToPST( output, level + 1 );
            return true;
        }

        bool Program::Directive::convertToISC( std::stringstream& output )
        {
            if( id == "inject" )
                output << program->constantStringValue( arguments[ 0 ]->id ) << std::endl;
            else if( id == "ensureType" )
            {
                std::string vid = arguments[ 0 ]->id;
                std::string tid = arguments[ 1 ]->id;
                output << "!namespace ___ensureType" << m_uid << std::endl;
                output << "eadr 0 $" << tid << "/type :*$" << vid << "->___Atom.___classMetaInfo" << std::endl;
                output << "jifi 0 @___good @___bad" << std::endl;
                output << "!jump ___bad" << std::endl;
                output << "!data bytes ___message \"Variable: " << vid << " is not type of: " << tid << "\", 10, 0" << std::endl;
                output << "dbgb $___message" << std::endl;
                output << "mobj $___valueL" << std::endl;
                output << "mpsh $___valueL" << std::endl;
                output << "ret" << std::endl;
                output << "!jump ___good" << std::endl;
                output << "!namespace-end" << std::endl;
            }
            return true;
        }

        void Program::Directive::setProgram( Program* p )
        {
            program = p;
            for( auto v : arguments )
                v->setProgram( p );
        }

        Program::Variable::Variable( Program* p, ASTNode* n )
        : Convertible( "variable", p, n )
        , type( T_UNDEFINED )
        , isStatic( false )
        , valueL( 0 )
        , valueR( 0 )
        , m_uid( 0 )
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
                m_uid = std::hash< std::string >()( id );
                p->constantHash( m_uid );
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
                m_uid = std::hash< std::string >()( id );
                p->constantHash( m_uid );
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
            m_uid = 0;
        }

        bool Program::Variable::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(variable)" << id << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(variable.static)" << isStatic << std::endl;
            if( valueL )
            {
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(valueLeft)" << std::endl;
                valueL->convertToPST( output, level + 2 );
            }
            if( valueR )
            {
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(valueRight)" << std::endl;
                valueR->convertToPST( output, level + 2 );
            }
            return true;
        }

        bool Program::Variable::convertToISC( std::stringstream& output )
        {
            if( type == T_DECLARATION_ASSIGNMENT && valueR )
            {
                valueR->convertToISC( output );
                output << "mpop $___valueR" << std::endl;
                output << "mref $" << id << " $___valueR" << std::endl;
                output << "mfin $___valueR" << std::endl;
                output << "mdel $___valueR" << std::endl;
            }
            else if(type == T_ASSIGNMENT && valueL && valueR )
            {
                valueL->convertToISC( output, 0, true );
                valueR->convertToISC( output );
                output << "mpop $___valueR $___valueL" << std::endl;
                output << "mref $___valueL $___valueR" << std::endl;
                output << "mfin $___valueR" << std::endl;
                output << "mdel $___valueR" << std::endl;
            }
            return true;
        }

        void Program::Variable::setProgram( Program* p )
        {
            program = p;
            if( valueL )
                valueL->setProgram( p );
            if( valueR )
                valueR->setProgram( p );
        }

        unsigned int Program::Block::s_uidGenerator = 0;

        Program::Block::Block( Program* p, ASTNode* n, bool oneStatement )
        : Convertible( "block", p, n )
        , m_uid( s_uidGenerator++ )
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

        Program::Block::Block( Program* p )
        : Convertible( "block", p, 0 )
        , m_uid( s_uidGenerator++ )
        {
            isValid = true;
        }

        Program::Block::~Block()
        {
            variables.clear();
            for( auto s : statements )
                Delete( s );
            statements.clear();
            m_uid = 0;
        }

        bool Program::Block::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(block)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(variables)" << std::endl;
            for( auto v : variables )
                output << "[" << program->nextUIDpst() << "]" << lvl << "--(variable)" << v << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(statements)" << std::endl;
            for( auto s : statements )
                s->convertToPST( output, level + 2 );
            return true;
        }

        bool Program::Block::convertToISC( std::stringstream& output )
        {
            output << "!namespace ___scope" << m_uid << std::endl;
            for( auto v : variables )
            {
                output << "!data address " << v << " 0" << std::endl;
                output << "mobj $" << v << std::endl;
            }
            output << "call @___scopeStart" << std::endl;
            output << "goto @___scopeExit" << std::endl;
            output << "!jump ___scopeStart" << std::endl;
            for( auto s : statements )
                s->convertToISC( output );
            output << "mobj $___valueL" << std::endl;
            output << "mpsh $___valueL" << std::endl;
            output << "ret" << std::endl;
            output << "!jump ___scopeExit" << std::endl;
            for( auto v : variables )
            {
                output << "mfin $" << v << std::endl;
                output << "mdel $" << v << std::endl;
            }
            output << "!namespace-end" << std::endl;
            return true;
        }

        void Program::Block::setProgram( Program* p )
        {
            program = p;
            for( auto s : statements )
                s->setProgram( p );
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
                if( v->type == Variable::T_DECLARATION || v->type == Variable::T_DECLARATION_ASSIGNMENT )
                {
                    if( std::find( variables.begin(), variables.end(), v->id ) != variables.end() )
                    {
                        appendError( nv, "Variable already declared in this scope!" );
                        Delete( v );
                        return false;
                    }
                    variables.push_back( v->id );
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

        bool Program::ObjectDestruction::convertToISC( std::stringstream& output )
        {
            if( value )
                value->convertToISC( output, 0, true );
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mder $___valueL" << std::endl;
            return true;
        }

        void Program::ObjectDestruction::setProgram( Program* p )
        {
            program = p;
            if( value )
                value->setProgram( p );
        }

        unsigned int Program::ControlFlowWhileLoop::s_uidGenerator = 0;

        Program::ControlFlowWhileLoop::ControlFlowWhileLoop( Program* p, ASTNode* n )
        : Convertible( "whileLoop", p, n )
        , condition( 0 )
        , statements( 0 )
        , m_uid( ++s_uidGenerator )
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
            m_uid = 0;
        }

        bool Program::ControlFlowWhileLoop::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(whileLoop)" << std::endl;
            condition->convertToPST( output, level + 1 );
            statements->convertToPST( output, level + 1 );
            return true;
        }

        bool Program::ControlFlowWhileLoop::convertToISC( std::stringstream& output )
        {
            output << "!namespace ___while" << m_uid << std::endl;
            output << "!jump ___check" << std::endl;
            if( condition )
                condition->convertToISC( output );
            output << "mpop $___valueL" << std::endl;
            output << "eadr 0 $Bool/type :*$___valueL->___Atom.___classMetaInfo" << std::endl;
            output << "jifi 0 @___good @___bad" << std::endl;
            output << "!jump ___bad" << std::endl;
            output << "!data bytes ___message \"While loop condition is not type of Bool!\", 10, 0" << std::endl;
            output << "dbgb $___message" << std::endl;
            output << "mobj $___valueL" << std::endl;
            output << "mpsh $___valueL" << std::endl;
            output << "ret" << std::endl;
            output << "!jump ___good" << std::endl;
            output << "movi regi:0 :*$___valueL->Bool/___Data.___data" << std::endl;
            output << "jifi 0 @___do @___end" << std::endl;
            output << "!jump ___do" << std::endl;
            if( statements )
                statements->convertToISC( output );
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mdel $___valueL" << std::endl;
            output << "goto @___check" << std::endl;
            output << "!jump ___end" << std::endl;
            output << "!namespace-end" << std::endl;
            return true;
        }

        void Program::ControlFlowWhileLoop::setProgram( Program* p )
        {
            program = p;
            if( condition )
                condition->setProgram( p );
            if( statements )
                statements->setProgram( p );
        }

        unsigned int Program::ControlFlowForLoop::s_uidGenerator = 0;

        Program::ControlFlowForLoop::ControlFlowForLoop( Program* p, ASTNode* n )
        : Convertible( "forLoop", p, n )
        , init( 0 )
        , condition( 0 )
        , iteration( 0 )
        , statements( 0 )
        , m_uid( ++s_uidGenerator )
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
                    ASTNode* nv = ns->findByType( "variable" );
                    Variable* v = new Variable( p, nv );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    if( v->type != Variable::T_DECLARATION && v->type != Variable::T_DECLARATION_ASSIGNMENT )
                    {
                        appendError( nv, "For-loop init stage is not either variable declaration or variable declaration with assignment!" );
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
            m_uid = 0;
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

        bool Program::ControlFlowForLoop::convertToISC( std::stringstream& output )
        {
            output << "!namespace ___for" << m_uid << std::endl;
            if( init )
            {
                if( !init->id.empty() )
                {
                    output << "!data address " << init->id << " 0" << std::endl;
                    output << "mobj $" << init->id << std::endl;
                }
                init->convertToISC( output );
            }
            output << "!jump ___check" << std::endl;
            if( condition )
            {
                condition->convertToISC( output );
                output << "mpop $___valueL" << std::endl;
                output << "eadr 0 $Bool/type :*$___valueL->___Atom.___classMetaInfo" << std::endl;
                output << "jifi 0 @___good @___bad" << std::endl;
                output << "!jump ___bad" << std::endl;
                output << "!data bytes ___message \"While loop condition is not type of Bool!\", 10, 0" << std::endl;
                output << "dbgb $___message" << std::endl;
                output << "mobj $___valueL" << std::endl;
                output << "mpsh $___valueL" << std::endl;
                output << "ret" << std::endl;
                output << "!jump ___good" << std::endl;
                output << "movi regi:0 :*$___valueL->Bool/___Data.___data" << std::endl;
                output << "jifi 0 @___do @___end" << std::endl;
                output << "!jump ___do" << std::endl;
            }
            if( statements )
            {
                statements->convertToISC( output );
                output << "mpop $___valueL" << std::endl;
                output << "mfin $___valueL" << std::endl;
                output << "mdel $___valueL" << std::endl;
            }
            if( iteration )
            {
                iteration->convertToISC( output );
                output << "mpop $___valueL" << std::endl;
                if( init && !init->id.empty() )
                    output << "mref $" << init->id << " $___valueL" << std::endl;
                output << "mfin $___valueL" << std::endl;
                output << "mdel $___valueL" << std::endl;
            }
            output << "goto @___check" << std::endl;
            output << "!jump ___end" << std::endl;
            if( init && !init->id.empty() )
            {
                output << "mfin $" << init->id << std::endl;
                output << "mdel $" << init->id << std::endl;
            }
            output << "!namespace-end" << std::endl;
            return true;
        }

        void Program::ControlFlowForLoop::setProgram( Program* p )
        {
            program = p;
            if( init )
                init->setProgram( p );
            if( condition )
                condition->setProgram( p );
            if( iteration )
                iteration->setProgram( p );
            if( statements )
                statements->setProgram( p );
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

        void Program::ControlFlowForeachLoop::setProgram( Program* p )
        {
            program = p;
            if( collection )
                collection->setProgram( p );
            if( statements )
                statements->setProgram( p );
        }

        unsigned int Program::ControlFlowCondition::s_uidGenerator = 0;

        Program::ControlFlowCondition::ControlFlowCondition( Program* p, ASTNode* n )
        : Convertible( "condition", p, n )
        , m_uid( ++s_uidGenerator )
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
            m_uid = 0;
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

        bool Program::ControlFlowCondition::convertToISC( std::stringstream& output )
        {
            unsigned int step = 0;
            output << "!namespace ___condition" << m_uid << std::endl;
            output << "!data bytes ___message \"Condition is not type of Bool!\", 10, 0" << std::endl;
            for( auto s : stages )
            {
                if( s.first )
                {
                    s.first->convertToISC( output );
                    output << "mpop $___valueL" << std::endl;
                    output << "eadr 0 $Bool/type :*$___valueL->___Atom.___classMetaInfo" << std::endl;
                    output << "jifi 0 @___good" << step << " @___bad" << step << std::endl;
                    output << "!jump ___bad" << step << std::endl;
                    output << "dbgb $___message" << std::endl;
                    output << "mobj $___valueL" << std::endl;
                    output << "mpsh $___valueL" << std::endl;
                    output << "ret" << std::endl;
                    output << "!jump ___good" << step << std::endl;
                    output << "movi regi:0 :*$___valueL->Bool/___Data.___data" << std::endl;
                    output << "jifi 0 @___do" << step << " @___next" << step << std::endl;
                    output << "!jump ___do" << step << std::endl;
                }
                if( s.second )
                {
                    s.second->convertToISC( output );
                    output << "mpop $___valueL" << std::endl;
                    output << "mfin $___valueL" << std::endl;
                    output << "mdel $___valueL" << std::endl;
                }
                output << "goto @___end" << std::endl;
                output << "!jump ___next" << step << std::endl;
                ++step;
            }
            output << "!jump ___end" << std::endl;
            output << "!namespace-end" << std::endl;
            return true;
        }

        void Program::ControlFlowCondition::setProgram( Program* p )
        {
            program = p;
            for( auto s : stages )
            {
                if( s.first )
                    s.first->setProgram( p );
                if( s.second )
                    s.second->setProgram( p );
            }
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

        bool Program::ControlFlowReturn::convertToISC( std::stringstream& output )
        {
            if( value )
            {
                value->convertToISC( output );
                output << "mpop $___valueR" << std::endl;
                output << "mobj $___valueL" << std::endl;
                output << "mref $___valueL $___valueR" << std::endl;
                output << "mpsh $___valueL" << std::endl;
            }
            else
            {
                output << "mobj $___valueL" << std::endl;
                output << "mpsh $___valueL" << std::endl;
            }
            output << "ret" << std::endl;
            return true;
        }

        void Program::ControlFlowReturn::setProgram( Program* p )
        {
            program = p;
            if( value )
                value->setProgram( p );
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

        void Program::BinaryOperation::setProgram( Program* p )
        {
            program = p;
            if( valueL )
                valueL->setProgram( p );
            if( valueR )
                valueR->setProgram( p );
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

        void Program::UnaryOperation::setProgram( Program* p )
        {
            program = p;
            if( value )
                value->setProgram( p );
        }

        Program::Method::Method( Program* p, ASTNode* n )
        : Convertible( "method", p, n )
        , isStatic( false )
        , argumentsParams( false )
        , statements( 0 )
        , m_uid( 0 )
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
            if( !n->hasType( "block" ) )
            {
                appendError( n, "Method definition does not have body!" );
                return;
            }
            ASTNode* nid = n->findByType( "identifier" );
            id = nid->value;
            if( n->hasType( "class.method.definition.argument_params" ) )
                argumentsParams = true;
            else if( n->hasType( "class.method.definition.argument_list" ) )
            {
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
            }
            else
            {
                appendError( n, "Method definition does not have either arguments list or arguments params!" );
                return;
            }
            Block* b = new Block( p, n->findByType( "block" ) );
            if( !b->isValid )
            {
                appendError( b );
                Delete( b );
                return;
            }
            statements = b;
            m_uid = std::hash< std::string >()( id );
            p->constantHash( m_uid );
            isValid = true;
        }

        Program::Method::Method( Program* p, const std::string& i, const std::vector< std::string >& a, bool s )
        : Convertible( "method", p, 0 )
        , isStatic( s )
        , id( i )
        , arguments( a )
        , argumentsParams( false )
        , statements( 0 )
        , m_uid( 0 )
        {
            statements = new Block( p );
            m_uid = std::hash< std::string >()( id );
            p->constantHash( m_uid );
            isValid = true;
        }

        Program::Method::~Method()
        {
            isStatic = false;
            id.clear();
            arguments.clear();
            argumentsParams = false;
            Delete( statements );
            m_uid = 0;
        }

        bool Program::Method::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(method)" << id << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(method.static)" << isStatic << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(method.params)" << argumentsParams << std::endl;
            for( auto a : arguments )
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(argument)" << a << std::endl;
            statements->convertToPST( output, level + 1 );
            return true;
        }

        bool Program::Method::convertToISC( std::stringstream& output )
        {
            output << "!jump " << id << std::endl;
            output << "!namespace " << id << std::endl;
            output << "!data int ___argumentsSize " << arguments.size() << std::endl;
            if( argumentsParams )
            {
                output << "popi $___argumentsSize" << std::endl;
                output << "movi regi:0 $___argumentsSize" << std::endl;
                // TODO: create array of arguments here
                output << "!namespace ___argsLoop" << std::endl;
                output << "!jump ___argsTest" << std::endl;
                output << "jifi 0 @___argsPassed @___argsFailed" << std::endl;
                output << "!jump ___argsPassed" << std::endl;
                output << "mpop $___valueL" << std::endl;
                output << "deci 0 0" << std::endl;
                output << "goto @___argsTest" << std::endl;
                output << "!jump ___argsFailed" << std::endl;
                output << "!namespace-end" << std::endl;
            }
            else
            {
                output << "popi regi:0" << std::endl;
                output << "movi regi:1 $___argumentsSize" << std::endl;
                output << "teti 2 0 1" << std::endl;
                output << "jifi 2 @___argumentsCheckPassed @___argumentsCheckFailed" << std::endl;
                output << "!jump ___argumentsCheckFailed" << std::endl;
                output << "!data bytes ___argumentsCheckFailedMessage0 \"Method: " << id << " was called with wrong arguments count: \", 0" << std::endl;
                output << "!data bytes ___argumentsCheckFailedMessage1 \", it should be: \", 0" << std::endl;
                output << "dbgb $___argumentsCheckFailedMessage0" << std::endl;
                output << "dbgi regi:0" << std::endl;
                output << "dbgb $___argumentsCheckFailedMessage1" << std::endl;
                output << "dbgi regi:1" << std::endl;
                output << "dbgb $___NEW_LINE" << std::endl;
                output << "mobj $___valueL" << std::endl;
                output << "psha $___valueL" << std::endl;
                output << "ret" << std::endl;
                output << "!jump ___argumentsCheckPassed" << std::endl;
            }

            output << "!data address this 0" << std::endl;
            if( !arguments.empty() )
            {
                std::stringstream ss;
                for( auto& a : arguments )
                {
                    output << "!data address " << a << " 0" << std::endl;
                    ss << " $" << a;
                }
                output << "mpop $this" << ss.str() << std::endl;
            }
            else
                output << "mpop $this" << std::endl;
            if( isStatic )
                output << "mobj $this" << std::endl;
            if( statements )
                statements->convertToISC( output );
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            return true;
        }

        void Program::Method::setProgram( Program* p )
        {
            program = p;
            if( statements )
                statements->setProgram( p );
        }

        unsigned int Program::Method::Call::s_uidGenerator = 0;

        Program::Method::Call::Call( Program* p, ASTNode* n )
        : Convertible( "methodCall", p, n )
        , m_uid( ++s_uidGenerator )
        {
            if( n->type != "class.method.call" )
            {
                appendError( n, "AST node is not type of class.method.call!" );
                return;
            }
            if( !n->hasType( "class.method.call.argument_list" ) )
            {
                appendError( n, "Method calling does not have arguments list!" );
                return;
            }
            if( n->hasType( "identifier" ) )
            {
                ASTNode* nfi = n->findByType( "identifier" );
                id = nfi->value;
            }
            else if( n->hasType( "field" ) )
            {
                ASTNode* nff = n->findByType( "field" );
                if( nff->nodes.size() != 2 )
                {
                    appendError( nff, "Method calling field does not have 2 AST nodes!" );
                    return;
                }
                if( nff->nodes[ 0 ].type != "identifier" )
                {
                    appendError( &nff->nodes[ 0 ], "Method calling field first AST node is not a type of identifier!" );
                    return;
                }
                if( nff->nodes[ 1 ].type != "identifier" )
                {
                    appendError( &nff->nodes[ 1 ], "Method calling field first AST node is not a type of identifier!" );
                    return;
                }
                classId = nff->nodes[ 0 ].value;
                id = nff->nodes[ 1 ].value;
            }
            else
            {
                appendError( n, "Method calling does not have method identifier or field!" );
                return;
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
            id.clear();
            classId.clear();
            for( auto a : arguments )
                Delete( a );
            arguments.clear();
            m_uid = 0;
        }

        bool Program::Method::Call::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(methodCall)" << id << std::endl;
            if( !classId.empty() )
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(methodCall.class)" << classId << std::endl;
            for( auto a : arguments )
                a->convertToPST(  output, level + 1 );
            return true;
        }

        bool Program::Method::Call::convertToISC( std::stringstream& output, bool isConstructor )
        {
            if( isConstructor )
            {
                output << "!namespace ___objectConstruct" << m_uid << std::endl;
                output << "!data address ___this 0" << std::endl;
                output << "mobj $___this" << std::endl;
                output << "movi regi:0 $___ONE" << std::endl;
                output << "mnew $___this " << id << "/___Data 0 @" << id << "/___Finalizer" << std::endl;
                output << "mova :*$___this->" << id << "/___Data.___classMetaInfo $" << id << "/type" << std::endl;
                for( std::vector< Value* >::reverse_iterator it = arguments.rbegin(); it != arguments.rend(); ++it )
                    (*it)->convertToISC( output, 0, true );
                output << "mpsh $___this" << std::endl;
                output << "call @" << id << "/___Creator" << std::endl;
                output << "mpsh $___this" << std::endl;
                output << "pshi $" << program->constantInt( arguments.size() ) << std::endl;
                output << "call @" << id << "/Constructor" << std::endl;
                output << "mpop $___valueL" << std::endl;
                output << "mfin $___valueL" << std::endl;
                output << "mdel $___valueL" << std::endl;
                output << "mpsh $___this" << std::endl;
                output << "!namespace-end" << std::endl;
                return true;
            }
            if( classId.empty() )
            {
                output << "!namespace ___methodCall" << m_uid << std::endl;
                output << "!data address ___mcThis 0" << std::endl;
                output << "!data address ___mcAddr 0" << std::endl;
                output << "mpop $___mcThis" << std::endl;
                output << "psha $" << program->constantHash( std::hash< std::string >()( id ) ) << std::endl;
                output << "mpsh $___mcThis" << std::endl;
                output << "call @___FIND_HASHED_METHOD_OF" << std::endl;
                output << "popi $___mcAddr" << std::endl;
                output << "movi regi:0 $___mcAddr" << std::endl;
                output << "jifi 0 @___addressCheckPassed @___addressCheckFailed" << std::endl;
                output << "!jump ___addressCheckFailed" << std::endl;
                output << "!data bytes ___addressCheckFailedMessage \"Method: " << id << " was not found!\", 10, 0" << std::endl;
                output << "dbgb $___addressCheckFailedMessage" << std::endl;
                output << "mobj $___mcThis" << std::endl;
                output << "mpsh $___mcThis" << std::endl;
                output << "ret" << std::endl;
                output << "!jump ___addressCheckPassed " << std::endl;
                for( std::vector< Value* >::reverse_iterator it = arguments.rbegin(); it != arguments.rend(); ++it )
                    (*it)->convertToISC( output, 0, true );
                output << "mpsh $___mcThis" << std::endl;
                output << "pshi $" << program->constantInt( arguments.size() ) << std::endl;
                output << "jcal $___mcAddr" << std::endl;
                output << "!namespace-end" << std::endl;
            }
            else
            {
                for( std::vector< Value* >::reverse_iterator it = arguments.rbegin(); it != arguments.rend(); ++it )
                    (*it)->convertToISC( output );
                output << "mobj $___valueL" << std::endl;
                output << "mpsh $___valueL" << std::endl;
                output << "pshi $" << program->constantInt( arguments.size() ) << std::endl;
                output << "call @" << classId << "/" << id << std::endl;
            }
            return true;
        }

        void Program::Method::Call::setProgram( Program* p )
        {
            program = p;
            for( auto a : arguments )
                a->setProgram( p );
        }

        unsigned int Program::Value::s_uidGenerator = 0;

        Program::Value::Value( Program* p, ASTNode* n )
        : Convertible( "value", p, n )
        , type( T_UNDEFINED )
        , data( 0 )
        , accessValue( 0 )
        , m_uid( ++s_uidGenerator )
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
            else if( n->hasType( "false_value" ) )
            {
                type = T_FALSE;
            }
            else if( n->hasType( "true_value" ) )
            {
                type = T_TRUE;
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
            else if( n->hasType( "field" ) )
            {
                ASTNode* nf = n->findByType( "field" );
                if( nf->nodes.size() != 2 )
                {
                    appendError( nf, "Value field does not have 2 AST nodes!" );
                    return;
                }
                if( nf->nodes[ 0 ].type != "identifier" )
                {
                    appendError( &nf->nodes[ 0 ], "Value field first AST node is not a type of identifier!" );
                    return;
                }
                if( nf->nodes[ 1 ].type != "identifier" )
                {
                    appendError( &nf->nodes[ 1 ], "Value field first AST node is not a type of identifier!" );
                    return;
                }
                classId = nf->nodes[ 0 ].value;
                id = nf->nodes[ 1 ].value;
                type = T_FIELD;
            }
            else if( n->hasType( "identifier" ) )
            {
                ASTNode* nid = n->findByType( "identifier" );
                id = nid->value;
                type = T_IDENTIFIER;
            }
            else
            {
                appendError( n, "Value does not have either object creator, class method call, binary operation, unary operation, number, string, null value, field or identifier!" );
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
            classId.clear();
            Delete( data );
            Delete( accessValue );
            m_uid = 0;
        }

        bool Program::Value::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(value)" << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(value.type)" << type << std::endl;
            if( !id.empty() )
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(value.id)" << id << std::endl;
            if( !classId.empty() )
                output << "[" << program->nextUIDpst() << "]" << lvl << "-(value.class)" << classId << std::endl;
            if( data )
                data->convertToPST( output, level + 1 );
            if( accessValue )
                accessValue->convertToPST( output, level + 1 );
            return true;
        }

        bool Program::Value::convertToISC( std::stringstream& output, int level, bool isLeft )
        {
            if( type == T_OBJECT_CREATE )
            {
                if( data )
                    ((Method::Call*)data)->convertToISC( output, true );
            }
            else if( type == T_METHOD_CALL )
            {
                if( data )
                    data->convertToISC( output );
            }
            else if( type == T_BINARY_OPERATION )
            {
                // TODO
            }
            else if( type == T_UNARY_OPERATION )
            {
                // TODO
            }
            else if( type == T_FALSE )
            {
                output << "pshi $___ZERO" << std::endl;
                output << "call @___GET_BOOL_FROM_ATOM" << std::endl;
            }
            else if( type == T_TRUE )
            {
                output << "pshi $___ONE" << std::endl;
                output << "call @___GET_BOOL_FROM_ATOM" << std::endl;
            }
            else if( type == T_NUMBER_INT )
            {
                output << "pshi $" << id << std::endl;
                output << "call @___GET_INT_FROM_ATOM" << std::endl;
            }
            else if( type == T_NUMBER_FLOAT )
            {
                output << "pshf $" << id << std::endl;
                output << "call @___GET_FLOAT_FROM_ATOM" << std::endl;
            }
            else if( type == T_STRING )
            {
                output << "ptr $___valueL $" << id << std::endl;
                output << "psha $___valueL" << std::endl;
                output << "call @___GET_STRING_FROM_ATOM" << std::endl;
            }
            else if( type == T_NULL )
            {
                output << "mobj $___valueL" << std::endl;
                output << "mpsh $___valueL" << std::endl;
            }
            else if( type == T_FIELD )
            {
                if( isLeft && !accessValue )
                    output << "mpsh $" << classId << "/" << id << std::endl;
                else
                {
                    output << "mobj $___valueL" << std::endl;
                    output << "mref $___valueL $" << classId << "/" << id << std::endl;
                    output << "mpsh $___valueL" << std::endl;
                }
            }
            else if( type == T_IDENTIFIER )
            {
                if( level > 0 )
                {
                    output << "!namespace ___accessField" << m_uid << std::endl;
                    output << "!data address ___this 0" << std::endl;
                    output << "!data address ___field 0" << std::endl;
                    output << "!data int ___offset 0" << std::endl;
                    output << "mpop $___this" << std::endl;
                    output << "psha $" << program->constantHash( std::hash< std::string >()( id ) ) << std::endl;
                    output << "mpsh $___this" << std::endl;
                    output << "call @___FIND_HASHED_FIELD_OF" << std::endl;
                    output << "popi $___offset" << std::endl;
                    output << "movi regi:0 $___offset" << std::endl;
                    output << "jifi 0 @___offsetCheckPassed @___offsetCheckFailed" << std::endl;
                    output << "!jump ___offsetCheckFailed" << std::endl;
                    output << "!data bytes ___offsetCheckFailedMessage \"Field: " << id << " was not found!\", 10, 0" << std::endl;
                    output << "dbgb $___offsetCheckFailedMessage" << std::endl;
                    output << "mobj $___field" << std::endl;
                    output << "mpsh $___field" << std::endl;
                    output << "ret" << std::endl;
                    output << "!jump ___offsetCheckPassed " << std::endl;
                    output << "movi regi:0 $___offset" << std::endl;
                    output << "ptr $___field :*$___this->Object/___Data.___classMetaInfo" << std::endl;
                    output << "sadr $___field byte 0" << std::endl;
                    output << "mova $___field :$___field" << std::endl;
                    output << "mpsh $___field" << std::endl;
                    output << "!namespace-end" << std::endl;
                }
                else
                {
                    if( isLeft && !accessValue )
                        output << "mpsh $" << id << std::endl;
                    else
                    {
                        output << "mobj $___valueL" << std::endl;
                        output << "mref $___valueL $" << id << std::endl;
                        output << "mpsh $___valueL" << std::endl;
                    }
                }
            }
            if( accessValue )
                accessValue->convertToISC( output, level + 1, isLeft );
            return true;
        }

        void Program::Value::setProgram( Program* p )
        {
            program = p;
            if( data )
                data->setProgram( p );
            if( accessValue )
                accessValue->setProgram( p );
        }

        Program::Class::Class( Program* p, ASTNode* n )
        : Convertible( "class", p, n )
        , m_uid( 0 )
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
                if( nbs.type == "directive.statement" )
                {
                    Directive* d = new Directive( p, &nbs, this );
                    if( !d->isValid )
                    {
                        appendError( d );
                        Delete( d );
                        return;
                    }
                    Delete( d );
                }
                else if( nbs.type == "variable" )
                {
                    Variable* v = new Variable( p, &nbs );
                    if( !v->isValid )
                    {
                        appendError( v );
                        Delete( v );
                        return;
                    }
                    if( !v->isStatic && v->type != Variable::T_DECLARATION )
                    {
                        appendError( &nbs, "Non-static field can be only declared!" );
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
            m_uid = std::hash< std::string >()( id );
            p->constantHash( m_uid );
            if( !methods.count( "Constructor" ) )
                methods[ "Constructor" ] = new Method( p, "Constructor", std::vector< std::string >(), false );
            if( !methods.count( "Destructor" ) )
                methods[ "Destructor" ] = new Method( p, "Destructor", std::vector< std::string >(), false );
            isValid = true;
        }

        Program::Class::~Class()
        {
            id.clear();
            inheritance.clear();
            for( auto& kv : fields )
                Delete( kv.second );
            fields.clear();
            atomFields.clear();
            for( auto& kv : methods )
                Delete( kv.second );
            methods.clear();
            m_uid = 0;
        }

        bool Program::Class::convertToPST( std::stringstream& output, int level )
        {
            std::string lvl( level, '-' );
            output << "[" << program->nextUIDpst() << "]" << lvl << "(class)" << id << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(class.inheritance)" << inheritance << std::endl;
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(fields)" << std::endl;
            for( auto& kv : fields )
                kv.second->convertToPST( output, level + 2 );
            output << "[" << program->nextUIDpst() << "]" << lvl << "-(atomFields)" << std::endl;
            for( auto& kv : atomFields )
            {
                output << "[" << program->nextUIDpst() << "]" << lvl << "--(atomField)" << kv.first << std::endl;
                output << "[" << program->nextUIDpst() << "]" << lvl << "--(atomField.type)" << kv.second.first << std::endl;
                output << "[" << program->nextUIDpst() << "]" << lvl << "--(atomField.value)" << kv.second.second << std::endl;
            }
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
            output << "!data address ___CLASS_UID " << m_uid << std::endl;
            output << "!data int ___CLASS_FIELDS_COUNT " << fields.size() << std::endl;
            output << "!data int ___CLASS_METHODS_COUNT " << methods.size() << std::endl;
            for( auto& kv : fields )
            {
                output << "!data bytes ___FIELD_NAME_" << kv.first << " \"" << kv.first << "\", 0" << std::endl;
                output << "!data int ___FIELD_NAMELEN_" << kv.first << " " << (kv.first.length() + 1) << std::endl;
                output << "!data address ___FIELD_UID_" << kv.first << " " << kv.second->getUID() << std::endl;
                output << "!data int ___FIELD_STATIC_" << kv.first << " " << (int)kv.second->isStatic << std::endl;
            }
            for( auto& kv : methods )
            {
                output << "!data bytes ___METHOD_NAME_" << kv.first << " \"" << kv.first << "\", 0" << std::endl;
                output << "!data int ___METHOD_NAMELEN_" << kv.first << " " << (kv.first.length() + 1) << std::endl;
                output << "!data address ___METHOD_UID_" << kv.first << " " << kv.second->getUID() << std::endl;
                output << "!data int ___METHOD_STATIC_" << kv.first << " " << (int)kv.second->isStatic << std::endl;
            }
            output << "!struct-def ___Data" << std::endl;
            output << "!field address ___classMetaInfo 1" << std::endl;
            std::vector< std::string > fl;
            getFieldsList( fl );
            for( auto n : fl )
                output << "!field address " << n << " 1" << std::endl;
            for( auto& kv : atomFields )
                output << "!field " << kv.second.first << " " << kv.first << " " << kv.second.second << std::endl;
            output << "!struct-end" << std::endl;
            std::vector< std::string > sfl;
            getFieldsList( sfl, true );
            for( auto n : sfl )
                output << "!data address " << n << " 0" << std::endl;
            output << "!start" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___CODE_%_jump_%" << std::endl;
            output << "#increment _jump_" << std::endl;
            output << "!namespace " << id << std::endl;
            output << "!data address ___ptr 0" << std::endl;
            output << "movi regi:0 $" << id << "/___CLASS_NAMELEN" << std::endl;
            output << "movi regi:1 $___ONE" << std::endl;
            output << "movi :$" << id << "/type->___ClassMetaInfo.namelen $" << id << "/___CLASS_NAMELEN" << std::endl;
            output << "movb :$" << id << "/type->___ClassMetaInfo.name $" << id << "/___CLASS_NAME 0" << std::endl;
            output << "mova :$" << id << "/type->___ClassMetaInfo.uid $" << id << "/___CLASS_UID" << std::endl;
            output << "movi :$" << id << "/type->___ClassMetaInfo.fieldsCount $" << id << "/___CLASS_FIELDS_COUNT" << std::endl;
            output << "movi :$" << id << "/type->___ClassMetaInfo.methodsCount $" << id << "/___CLASS_METHODS_COUNT" << std::endl;
            Class* inh = program->findClass( inheritance );
            if( inh )
                output << "mova :$" << id << "/type->___ClassMetaInfo.inheritanceMetaInfo $" << inh->id << "/type" << std::endl;
            else
                output << "mova :$" << id << "/type->___ClassMetaInfo.inheritanceMetaInfo $___NULL" << std::endl;
            output << "mova $___ptr :$" << id << "/type->___ClassMetaInfo.fields" << std::endl;
            for( auto& kv : fields )
            {
                output << "movi :$___ptr->___FieldMetaInfo.namelen $" << id << "/___FIELD_NAMELEN_" << kv.first << std::endl;
                output << "movi regi:0 $" << id << "/___FIELD_NAMELEN_" << kv.first << std::endl;
                output << "movb :$___ptr->___FieldMetaInfo.name $" << id << "/___FIELD_NAME_" << kv.first <<" 0" << std::endl;
                output << "mova :$___ptr->___FieldMetaInfo.uid $" << id << "/___FIELD_UID_" << kv.first << std::endl;
                output << "movi :$___ptr->___FieldMetaInfo.static $" << id << "/___FIELD_STATIC_" << kv.first << std::endl;
                if( kv.second->isStatic )
                    output << "movi :$___ptr->___FieldMetaInfo.offset $___ZERO" << std::endl;
                else
                    output << "poff :$___ptr->___FieldMetaInfo.offset :$" << id << "/type->" << id << "/___Data." << kv.first << std::endl;
                output << "mova :$___ptr->___FieldMetaInfo.owner $" << id << "/type" << std::endl;
                output << "sadr $___ptr ___FieldMetaInfo 1" << std::endl;
            }
            output << "mova $___ptr :$" << id << "/type->___ClassMetaInfo.methods" << std::endl;
            for( auto& kv : methods )
            {
                output << "movi :$___ptr->___MethodMetaInfo.namelen $" << id << "/___METHOD_NAMELEN_" << kv.first << std::endl;
                output << "movi regi:0 $" << id << "/___METHOD_NAMELEN_" << kv.first << std::endl;
                output << "movb :$___ptr->___MethodMetaInfo.name $" << id << "/___METHOD_NAME_" << kv.first <<" 0" << std::endl;
                output << "mova :$___ptr->___MethodMetaInfo.uid $" << id << "/___METHOD_UID_" << kv.first << std::endl;
                output << "movi :$___ptr->___MethodMetaInfo.static $" << id << "/___METHOD_STATIC_" << kv.first << std::endl;
                output << "jadr :$___ptr->___MethodMetaInfo.address @" << id << "/" << kv.first << std::endl;
                output << "mova :$___ptr->___MethodMetaInfo.owner $" << id << "/type" << std::endl;
                output << "sadr $___ptr ___MethodMetaInfo 1" << std::endl;
            }
            for( auto n : sfl )
            {
                output << "mobj $" << id << "/" << n << std::endl;
                if( fields.count( n ) && fields[ n ]->isStatic && fields[ n ]->type == Variable::T_DECLARATION_ASSIGNMENT )
                {
                    fields[ n ]->valueR->convertToISC( output );
                    output << "mpop $___valueR" << std::endl;
                    output << "mref $" << id << "/" << n << " $___valueR" << std::endl;
                    output << "mfin $___valueR" << std::endl;
                    output << "mdel $___valueR" << std::endl;
                }
            }
            output << "goto @___CODE_%_goto_%" << std::endl;
            output << "#increment _goto_" << std::endl;
            for( auto& kv : methods )
                kv.second->convertToISC( output );
            output << "!jump ___Finalizer" << std::endl;
            output << "!namespace ___Finalizer" << std::endl;
            output << "!data address ___this 0" << std::endl;
            output << "mpop $___this" << std::endl;
            output << "mpsh $___this" << std::endl;
            output << "pshi $___ZERO" << std::endl;
            output << "call @" << id << "/Destructor" << std::endl;
            output << "mpop $___valueL" << std::endl;
            output << "mfin $___valueL" << std::endl;
            output << "mdel $___valueL" << std::endl;
            for( auto& kv : fields )
            {
                if( !kv.second->isStatic )
                {
                    output << "mdel :*$___this->" << id << "/___Data." << kv.first << std::endl;
                    output << "mova :*$___this->" << id << "/___Data." << kv.first << " $___NULL" << std::endl;
                }
            }
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!jump ___Creator" << std::endl;
            output << "!namespace ___Creator" << std::endl;
            output << "!data address ___this 0" << std::endl;
            output << "mpop $___this" << std::endl;
            for( auto& kv : fields )
                if( !kv.second->isStatic )
                    output << "mobj :*$___this->" << id << "/___Data." << kv.first << std::endl;
            output << "ret" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!namespace-end" << std::endl;
            output << "!exit" << std::endl;
            return true;
        }

        void Program::Class::getFieldsList( std::vector< std::string >& out, bool isStatic )
        {
            Class* ic = program->findClass( inheritance );
            if( ic && ic != this )
                ic->getFieldsList( out );
            for( auto& kv : fields )
                if( kv.second->isStatic == isStatic && std::find( out.begin(), out.end(), kv.first ) == out.end() )
                    out.push_back( kv.first );
        }

        void Program::Class::setProgram( Program* p )
        {
            program = p;
            for( auto& kv : fields )
                kv.second->setProgram( p );
            for( auto& kv : methods )
                kv.second->setProgram( p );
        }
    }
}
