#ifndef ___LIBRARY_INTERCEPTION___
#define ___LIBRARY_INTERCEPTION___

#include "../include/Runtime.h"
#include <XeCore/Intuicio/ContextVM.h>
#include <map>

namespace Kaiju
{
    struct Library
    {
    public:
        Library() : handle( 0 ), onLoad( 0 ), onUnload( 0 ), onCall( 0 ) {};

        void* handle;
        int32_t (*onLoad)();
        int32_t (*onUnload)();
        int32_t (*onCall)( Runtime*, int64_t, int64_t );
    };

    class LibraryInterception : public XeCore::Intuicio::ContextVM::OnInterceptListener
    {
    public:
        enum Code
        {
            C_LOAD,
            C_UNLOAD,
            C_CALL
        };

        LibraryInterception( Runtime* owner, XeCore::Intuicio::ContextVM* context ) : m_owner( owner ), m_context( context ) {};
        virtual ~LibraryInterception();

        virtual bool onIntercept( XeCore::Intuicio::ParallelThreadVM* caller, uint32_t code );

        std::vector< std::string > paths;

    private:
        Runtime* m_owner;
        XeCore::Intuicio::ContextVM* m_context;
        std::map< uint32_t, Library > m_libraries;
    };
}

#endif
