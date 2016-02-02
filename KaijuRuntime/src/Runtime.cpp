#include "../include/Runtime.h"
#include "../include/RuntimeTypes.h"
#include "../include_private/ContentLoader.h"
#include "../include_private/RuntimeInterception.h"
#include "../include_private/LibraryInterception.h"
#include "../include_private/StringInterception.h"
#include <XeCore/Intuicio/CompilerVM.h>
#include <XeCore/Intuicio/ContextVM.h>
#include <XeCore/Intuicio/IntuicioVM.h>
#include <XeCore/Intuicio/ProgramVM.h>
#include <XeCore/Intuicio/PrecompilerVM.h>
#include <iostream>
#include <fstream>
#include <map>

namespace Kaiju
{
    class IntuicioContentLoader
    : public XeCore::Intuicio::PrecompilerVM::OnLoadContentListener
    {
    public:
        std::vector< std::string > searchDirs;
        std::vector< std::string > paths;
        XeCore::Intuicio::PrecompilerVM::CrossPrecompilationData cpd;

        virtual ~IntuicioContentLoader() {};

        void pushPath( const std::string& path )
        {
            int p = path.find_last_of( "\\/" );
            paths.push_back( std::string( p < 0 ? "" : path.substr( 0, p ) ) );
        }

        void popPath()
        {
            if( !paths.empty() )
                paths.pop_back();
        }

        bool onLoadContent( const XeCore::Common::String& fname, XeCore::Common::String& outStream, XeCore::Common::String& outErrors )
        {
            std::stringstream sname;
            if( paths.empty() )
                sname << fname;
            else
            {
                if( paths.back().empty() )
                    sname << fname;
                else
                    sname << paths.back() << "/" << fname;
            }
            std::string name( sname.str() );
            std::ifstream* file = new std::ifstream( name.c_str(), std::ifstream::in | std::ifstream::binary );
            unsigned int fsi = 0;
            while( ( !file || !file->good() ) && fsi < searchDirs.size() )
            {
                std::stringstream sdname;
                sdname << searchDirs[ fsi ] << "/" << fname;
                if( file )
                    delete file;
                file = new std::ifstream( sdname.str().c_str(), std::ifstream::in | std::ifstream::binary );
                fsi++;
            }
            if( !file || !file->good() )
            {
                std::stringstream ss;
                ss << "Cannot open file: '" << name << "'.\n";
                outErrors += ss.str();
                return false;
            }
            file->seekg( 0, file->end );
            unsigned int fsize = file->tellg();
            file->seekg( 0, file->beg );
            XeCore::Common::String input;
            input.resize( fsize, 0 );
            file->read( (char*)input.c_str(), fsize );
            file->close();
            XeCore::Common::String errors;
            pushPath( fname );
            if( XeCore::Intuicio::PrecompilerVM::precompile( input, outStream, this, errors, &cpd ) )
            {
                outErrors += errors;
                popPath();
                if( file )
                    delete file;
                return true;
            }
            else
            {
                popPath();
                std::stringstream ss;
                ss << "Cannot precompile file: '" << fname << "'.\n" << errors;
                outErrors += ss.str();
                if( file )
                    delete file;
                return false;
            }
        }
    };

    template< typename T >
    static void XDelete( T*& p ) { if( p ) xdelete p; p = 0; };

    Runtime::Runtime()
    : m_code( 0 )
    , m_context( 0 )
    {
    }

    Runtime::~Runtime()
    {
        m_code = 0;
        m_context = 0;
    }

    int Runtime::run( const std::string& path, const std::vector< std::string >& args )
    {
        m_code = 1;
        m_types.clear();
        try
        {
            const char* kaijuStdPath = std::getenv( "KAIJU_STD_PATH" );
            const char* kaijuLibPath = std::getenv( "KAIJU_LIB_PATH" );
            ContentLoader* loader = new ContentLoader();
            if( kaijuStdPath )
                loader->paths.push_back( std::string( kaijuStdPath ) );
            std::string errors;
            Compiler::Program* kprog = loader->onContentLoad( path, errors );
            if( !kprog )
            {
                std::cout << "[Kaiju Runtime] Cannot load script: '" << path << "'!" << std::endl << errors << std::endl;
                Delete( loader );
                return m_code;
            }
            errors.clear();
            Delete( loader );
            if( !kprog->isValid )
            {
                std::cout << "[Kaiju Runtime] Program is not valid!" << std::endl << kprog->getErrors() << std::endl;
                Delete( kprog );
                return m_code;
            }
            std::stringstream content;
            if( !kprog->convertToISC( content ) )
            {
                std::cout << "[Kaiju Runtime] Cannot convert program to Intuicio assembly!" << std::endl;
                Delete( kprog );
                return m_code;
            }
            Delete( kprog );

            IntuicioContentLoader* iloader = xnew IntuicioContentLoader();
            XeCore::Common::String precompiled;
            XeCore::Common::String ierrors;
            if( !XeCore::Intuicio::PrecompilerVM::precompile( content.str(), precompiled, iloader, ierrors, &iloader->cpd ) )
            {
                std::cout << "[Kaiju Runtime] Cannot precompile Intuicio assembly!" << std::endl << ierrors << std::endl;
                XDelete( iloader );
                return m_code;
            }
            ierrors.clear();

            XeCore::Intuicio::ProgramVM* prog = XeCore::Intuicio::CompilerVM::compile( precompiled, ierrors );
            if( !prog )
            {
                std::cout << "[Kaiju Runtime] Cannot compile Intuicio assembly!" << std::endl << ierrors << std::endl;
                XDelete( prog );
                return m_code;
            }
            errors.clear();

            XeCore::Intuicio::IntuicioVM* vm = xnew XeCore::Intuicio::IntuicioVM();
            if( !prog->validate() )
            {
                std::cout << "[Kaiju Runtime] Intuicio program is not valid!" << std::endl;
                XDelete( vm );
                XDelete( prog );
                return m_code;
            }
            XeCore::Intuicio::ContextVM* cntx = vm->createContext( prog );
            if( cntx )
            {
                RuntimeInterception* runtimeListener = xnew RuntimeInterception( this, cntx );
                cntx->registerInterceptionListener( "RUNTIME", runtimeListener );
                LibraryInterception* libraryListener = xnew LibraryInterception( this, cntx );
                if( kaijuLibPath )
                    libraryListener->paths.push_back( std::string( kaijuLibPath ) );
                cntx->registerInterceptionListener( "LIBRARY", libraryListener );
                StringInterception* stringListener = xnew StringInterception( this, cntx );
                cntx->registerInterceptionListener( "STRING", stringListener );
                int32_t code = 0;
                int32_t argsCount = args.size();
                int64_t* argsArray = argsCount ? new int64_t[ argsCount ] : 0;
                int64_t argsTable = (int64_t)(void*)argsArray;
                if( argsTable )
                {
                    int i = 0;
                    for( std::vector< std::string >::const_reverse_iterator it = args.crbegin(); it != args.crend(); ++it, ++i )
                        argsArray[ i ] = (int64_t)(void*)(it->c_str());
                }
                int32_t* argsSizes = argsCount ? new int32_t[ argsCount ] : 0;
                int64_t argsTableSizes = (int64_t)(void*)argsSizes;
                if( argsTableSizes )
                {
                    int i = 0;
                    for( std::vector< std::string >::const_reverse_iterator it = args.crbegin(); it != args.crend(); ++it, ++i )
                        argsSizes[ i ] = (int32_t)it->length();
                }
                std::map< XeCore::Common::String, void* > externals;
                externals[ "___APP_ARGS_COUNT" ] = &argsCount;
                externals[ "___APP_ARGS_CSTR_PTR" ] = &argsTable;
                externals[ "___APP_ARGS_SIZE_PTR" ] = &argsTableSizes;
                externals[ "___APP_EXIT_CODE" ] = &code;
                cntx->bindExternals( externals );
                m_context = cntx;
                cntx->runProgram( true );
                m_context = 0;
                cntx->unbindExternals();
                if( argsTable )
                    delete (int64_t*)argsTable;
                if( argsTableSizes )
                    delete (int32_t*)argsTableSizes;
                cntx->unregisterAllInterceptionListeners();
                XDelete( runtimeListener );
                XDelete( libraryListener );
                XDelete( stringListener );
                m_code = code;
            }
            else
            {
                std::cout << "[Kaiju Runtime] Cannot create Intuicio program context!" << std::endl;
                XDelete( vm );
                XDelete( prog );
                return m_code;
            }

            XDelete( vm );
            XDelete( prog );
            return m_code;
        }
        catch( std::exception& ex )
        {
            std::cout << "[Kaiju Runtime] Exception thrown: " << ex.what() << std::endl;
            return m_code;
        }
        catch( ... )
        {
            std::cout << "[Kaiju Runtime] Unexpected exception thrown" << std::endl;
            return m_code;
        }
        return m_code;
    }

    bool Runtime::stackPush( int64_t caller, const void* src, uint32_t size )
    {
        if( !m_context || !caller )
            return false;
        return ((XeCore::Intuicio::ContextVM*)m_context)->stackPush( (XeCore::Intuicio::ParallelThreadVM*)caller, src, size );
    }

    bool Runtime::stackPop( int64_t caller, void* dst, uint32_t size )
    {
        if( !m_context || !caller )
            return false;
        return ((XeCore::Intuicio::ContextVM*)m_context)->stackPop( (XeCore::Intuicio::ParallelThreadVM*)caller, dst, size );
    }

    int64_t Runtime::createManagedObject()
    {
        if( !m_context )
            return 0;
        return ((XeCore::Intuicio::ContextVM*)m_context)->createManagedObject();
    }

    bool Runtime::deleteManagedObject( int64_t ptr )
    {
        if( !m_context )
            return false;
        return ((XeCore::Intuicio::ContextVM*)m_context)->deleteManagedObject( ptr );
    }

    bool Runtime::finalizeManagedObject( int64_t caller, int64_t ptr )
    {
        if( !m_context || !caller )
            return false;
        XeCore::Intuicio::ContextVM* cntx = (XeCore::Intuicio::ContextVM*)m_context;
        XeCore::Intuicio::ParallelThreadVM* pt = (XeCore::Intuicio::ParallelThreadVM*)caller;
        if( cntx->getManagedObjectRefCount( ptr ) == 1 )
        {
            uint32_t addr = cntx->getManagedObjectFinalizerAddress( ptr );
            return addr ? pt->runWithin( addr ) : true;
        }
        else
            return true;
    }

    bool Runtime::refManagedObject( int64_t ptrDst, int64_t ptrSrc )
    {
        if( !m_context )
            return false;
        return ((XeCore::Intuicio::ContextVM*)m_context)->refManagedObject( ptrDst, ptrSrc );
    }

    bool Runtime::unrefManagedObject( int64_t ptr )
    {
        if( !m_context )
            return false;
        return ((XeCore::Intuicio::ContextVM*)m_context)->unrefManagedObject( ptr );
    }

    void* Runtime::getManagedObjectDataRaw( int64_t ptr )
    {
        if( !m_context )
            return 0;
        return ((XeCore::Intuicio::ContextVM*)m_context)->getManagedObjectDataRaw( ptr );
    }

    bool Runtime::newManagedObjectRaw( int64_t caller, int64_t ptr, uint32_t size, const std::string& classId, int argsCount )
    {
        if( !caller || !m_context )
            return false;
        XeCore::Intuicio::ContextVM* cntx = (XeCore::Intuicio::ContextVM*)m_context;
        XeCore::Intuicio::ParallelThreadVM* pt = (XeCore::Intuicio::ParallelThreadVM*)caller;
        VM::___ClassMetaInfo* cmi = getType( classId );
        if( !cmi )
            return false;
        if( !cntx->newManagedObjectRaw( ptr, size, cmi->finalizerAddress ) )
            return false;
        Atom::___Atom* data = (Atom::___Atom*)cntx->getManagedObjectDataRaw( ptr );
        data->___classMetaInfo = (int64_t)cmi;
        if( !cntx->stackPush( pt, &ptr, sizeof( ptr ) ) )
            return false;
        if( !pt->runWithin( cmi->creatorAddress ) )
            return false;
        if( !cntx->stackPush( pt, &ptr, sizeof( ptr ) ) )
            return false;
        VM::___MethodMetaInfo* mmi = findManagedObjectMethod( cmi, "Constructor" );
        if( !mmi )
            return false;
        if( argsCount < 0 )
            cntx->stackPush( pt, &mmi->argsCount, sizeof( mmi->argsCount ) );
        else
            cntx->stackPush( pt, &argsCount, sizeof( argsCount ) );
        if( !pt->runWithin( mmi->address ) )
            return false;
        if( !cntx->stackPop( pt, &ptr, sizeof( ptr ) ) )
            return false;
        if( !finalizeManagedObject( caller, ptr ) )
            return false;
        if( !deleteManagedObject( ptr ) )
            return false;
        return true;
    }

    uint32_t Runtime::getManagedObjectRefCount( int64_t ptr )
    {
        if( !m_context )
            return 0;
        return ((XeCore::Intuicio::ContextVM*)m_context)->getManagedObjectRefCount( ptr );
    }

    VM::___ClassMetaInfo* Runtime::getType( const std::string& name )
    {
        for( auto t : m_types )
        {
            VM::___ClassMetaInfo* cmi = (VM::___ClassMetaInfo*)t;
            if( std::strcmp( (const char*)cmi->name, name.c_str() ) == 0 )
                return cmi;
        }
        return 0;
    }

    VM::___FieldMetaInfo* Runtime::findManagedObjectField( VM::___ClassMetaInfo* type, const std::string& name )
    {
        if( !type )
            return 0;
        VM::___FieldMetaInfo* fmi = (VM::___FieldMetaInfo*)type->fields;
        for( int i = 0; i < type->fieldsCount; ++i )
            if( std::strcmp( (const char*)fmi[ i ].name, name.c_str() ) == 0 )
                return &fmi[ i ];
        return 0;
    }

    VM::___FieldMetaInfo* Runtime::findManagedObjectField( int64_t ptr, const std::string& name )
    {
        if( !ptr )
            return 0;
        VM::___ClassMetaInfo* cmi = (VM::___ClassMetaInfo*)((Atom::___Atom*)ptr)->___classMetaInfo;
        return findManagedObjectField( cmi, name );
    }

    VM::___MethodMetaInfo* Runtime::findManagedObjectMethod( VM::___ClassMetaInfo* type, const std::string& name )
    {
        if( !type )
            return 0;
        VM::___MethodMetaInfo* mmi = (VM::___MethodMetaInfo*)type->methods;
        for( int i = 0; i < type->methodsCount; ++i )
            if( std::strcmp( (const char*)mmi[ i ].name, name.c_str() ) == 0 )
                return &mmi[ i ];
        return 0;
    }

    VM::___MethodMetaInfo* Runtime::findManagedObjectMethod( int64_t ptr, const std::string& name )
    {
        if( !ptr )
            return 0;
        VM::___ClassMetaInfo* cmi = (VM::___ClassMetaInfo*)((Atom::___Atom*)ptr)->___classMetaInfo;
        return findManagedObjectMethod( cmi, name );
    }

    bool Runtime::callManagedObjectMethod( int64_t caller, int64_t thisPtr, VM::___MethodMetaInfo* method, int argsCount )
    {
        if( !caller || !method )
            return false;
        XeCore::Intuicio::ContextVM* cntx = (XeCore::Intuicio::ContextVM*)m_context;
        XeCore::Intuicio::ParallelThreadVM* pt = (XeCore::Intuicio::ParallelThreadVM*)caller;
        if( !thisPtr )
            thisPtr = cntx->createManagedObject();
        if( !thisPtr )
            return false;
        if( !cntx->stackPush( pt, &thisPtr, sizeof( thisPtr ) ) )
            return false;
        if( argsCount < 0 )
            cntx->stackPush( pt, &method->argsCount, sizeof( method->argsCount ) );
        else
            cntx->stackPush( pt, &argsCount, sizeof( argsCount ) );
        return pt->runWithin( method->address );
    }

    void Runtime::registerType( int64_t ptr )
    {
        if( !ptr )
            return;
        if( std::find( m_types.begin(), m_types.end(), ptr ) == m_types.end() )
            m_types.push_back( ptr );
    }
}
