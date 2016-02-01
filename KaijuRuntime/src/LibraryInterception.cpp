#include "../include_private/LibraryInterception.h"
#include <dlfcn.h>

namespace Kaiju
{
    void* loadLibrary( const std::string& path )
    {
        void* h = dlopen( path.c_str(), RTLD_LAZY );
        if( !h )
            h = dlopen( (path + ".so").c_str(), RTLD_LAZY );
        return h;
    }

    void* getFunction( void* handle, const std::string& name )
    {
        return dlsym( handle, name.c_str() );
    }

    int closeLibrary( void* handle )
    {
        return dlclose( handle );
    }

    LibraryInterception::~LibraryInterception()
    {
        for( auto& kv : m_libraries )
            closeLibrary( kv.second.handle );
        m_libraries.clear();
        m_context = 0;
        m_owner = 0;
    }

    bool LibraryInterception::onIntercept( XeCore::Intuicio::ParallelThreadVM* caller, uint32 code )
    {
        if( !m_context || !m_owner )
            return false;
        if( code == C_LOAD )
        {
            Library lib;
            int64_t id = 0;
            int64_t ptrPath = 0;
            std::string path;
            if( !m_context->stackPop( caller, &id, sizeof( id ) ) )
                return false;
            if( !m_context->stackPop( caller, &ptrPath, sizeof( ptrPath ) ) )
                return false;
            if( m_libraries.count( id ) )
                return false;
            if( !id || !ptrPath )
                return false;
            path = (char*)ptrPath;
            lib.handle = loadLibrary( path );
            if( !lib.handle )
            {
                for( auto p : paths )
                {
                    lib.handle = loadLibrary( (p + "/" + path).c_str() );
                    if( lib.handle )
                        break;
                }
            }
            if( !lib.handle )
            {
                int32_t v = 0;
                m_context->stackPush( caller, &v, sizeof( v ) );
                return true;
            }
            lib.onLoad = (int32_t (*)())getFunction( lib.handle, "onLoad" );
            lib.onUnload = (int32_t (*)())getFunction( lib.handle, "onUnload" );
            lib.onCall = (int32_t (*)( Runtime*, int64_t, int64_t ))getFunction( lib.handle, "onCall" );
            if( !lib.onLoad || !lib.onUnload || !lib.onCall )
            {
                closeLibrary( lib.handle );
                int32_t v = 0;
                m_context->stackPush( caller, &v, sizeof( v ) );
                return true;
            }
            if( !lib.onLoad() )
            {
                closeLibrary( lib.handle );
                int32_t v = 0;
                m_context->stackPush( caller, &v, sizeof( v ) );
                return true;
            }
            m_libraries[ id ] = lib;
            int32_t v = 1;
            m_context->stackPush( caller, &v, sizeof( v ) );
            return true;
        }
        else if( code == C_UNLOAD )
        {
            int64_t id = 0;
            if( !m_context->stackPop( caller, &id, sizeof( id ) ) )
                return false;
            if( !m_libraries.count( id ) )
                return false;
            m_libraries[ id ].onUnload();
            closeLibrary( m_libraries[ id ].handle );
            m_libraries.erase( id );
            int32_t v = 1;
            m_context->stackPush( caller, &v, sizeof( v ) );
            return true;
        }
        else if( code == C_CALL )
        {
            int64_t id = 0;
            int64_t func = 0;
            if( !m_context->stackPop( caller, &id, sizeof( id ) ) )
                return false;
            if( !m_context->stackPop( caller, &func, sizeof( func ) ) )
                return false;
            if( !m_libraries.count( id ) )
                return false;
            int32_t status = m_libraries[ id ].onCall( m_owner, (int64_t)caller, func );
            return status;
        }
        return false;
    }
}
