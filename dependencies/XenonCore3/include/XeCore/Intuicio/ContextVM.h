#ifndef __XE_CORE__INTUICIO__CONTEXT_VM__
#define __XE_CORE__INTUICIO__CONTEXT_VM__

#include "../Common/IRtti.h"
#include "../Common/MemoryManager.h"
#include "../Common/Vector.h"
#include "../Common/String.h"
#include "ProgramVM.h"
#include "ThreadVM.h"
#include <map>

namespace XeCore
{
    namespace Intuicio
    {
        class IntuicioVM;

        using namespace XeCore::Common;

        class ContextVM
			: public virtual IRtti
			, public virtual MemoryManager::Manageable
        {
            friend class IntuicioVM;
            friend class ParallelThreadVM;

            RTTI_CLASS_DECLARE( ContextVM );

        public:
            class OnInterceptListener
            {
            public:
                virtual                         ~OnInterceptListener() {};
                virtual bool                    onIntercept( ParallelThreadVM* caller, uint32 code ) = 0;
            };

        protected:
                                                ContextVM( IntuicioVM* owner, ProgramVM* prog, const char* name = 0 );

        public:
            virtual                             ~ContextVM();

            String&                             getName();
            bool                                bindExternals( std::map< String, void* >& externals );
            void                                unbindExternals();
            bool                                registerInterceptionListener( const String& name, OnInterceptListener* listener );
            bool                                unregisterInterceptionListener( const String& name );
            void                                unregisterAllInterceptionListeners();
            bool                                runProgram( bool wait = false, uint32 startAddress = 0 );
            void                                terminateProgram();
            void                                waitForTerminate();
            bool                                stackPush( ParallelThreadVM* caller, const void* src, uint32 size );
            bool                                stackPop( ParallelThreadVM* caller, void* dst, uint32 size );
            int64_t                             stackAllocOn( ParallelThreadVM* caller, uint32 size );
            bool                                stackShiftForward( ParallelThreadVM* caller, uint32 size );
            bool                                stackShiftBackward( ParallelThreadVM* caller, uint32 size );
            bool                                stackCall( ParallelThreadVM* caller, const uint32 addr );
            bool                                stackReturn( ParallelThreadVM* caller, uint32& outAddr );
            bool                                stackSave( ParallelThreadVM* caller, uint32& dst );
            bool                                stackLoad( ParallelThreadVM* caller, const uint32 src );
            int64_t                             resolveAddress( ParallelThreadVM* caller, const ProgramVM::Pointer& ptr );
            int64_t                             createManagedObject();
            bool                                deleteManagedObject( int64_t ptr );
            bool                                refManagedObject( int64_t ptrDst, int64_t ptrSrc );
            bool                                unrefManagedObject( int64_t ptr );
            void*                               getManagedObjectDataRaw( int64_t ptr );
            template< typename T >
            inline T*                           getManagedObjectData( int64_t ptr ) { return (T*)getManagedObjectDataRaw( ptr ); };
            bool                                newManagedObjectRaw( int64_t ptr, uint32 size, uint32 finaddr = 0 );
            template< typename T >
            inline bool                         newManagedObject( int64_t ptr, uint32 count, uint32 finaddr = 0 ) { return newManagedObjectRaw( ptr, count * sizeof( T ), finaddr ); };
            uint32                              getManagedObjectRefCount( int64_t ptr );
            uint32                              getManagedObjectFinalizerAddress( int64_t ptr );

        protected:
            void                                cleanup();
            void                                execute( ParallelThreadVM* caller, uint32 begin, uint32 end, uint32 start );

        private:
            class ManagedMemory
            {
            public:
                                                ManagedMemory( uint32 size, uint32 finalizerAddress );
                                                ~ManagedMemory();

                void*                           getPointer() { return (void*)data; };
                void                            setPointer( void* v ) { data = (int64_t)v; };

                int64_t                         data;
                uint32                          finalizerAddress;
            };

            IntuicioVM*                                 m_owner;
            ProgramVM*                                  m_program;
            String                                      m_name;
            std::map< String, void* >                   m_exports;
            std::map< String, void* >                   m_imports;
            ProgramVM::Info                             m_info;
            Vector< ThreadVM* >                         m_threads;
            uint8*                                      m_data;
            void**                                      m_externals;
            std::map< uint32, OnInterceptListener* >    m_intercepts;
            OnInterceptListener*                        m_intercept;
        };
    }
}

#endif
