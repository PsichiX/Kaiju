#ifndef __XE_CORE__COMMON__CONCURRENT__SYNCHRONIZED__
#define __XE_CORE__COMMON__CONCURRENT__SYNCHRONIZED__

#include "TinyThreads/tinythread.h"
#include "../MemoryManager.h"
#ifndef NDEBUG
#include "../Logger.h"
#endif

#define __SYNCHRONIZED_OBJECT(name, obj)        XeCore::Common::Concurrent::Synchronized::ScopeGuard \
                                                __XeCore_Common_Concurrent_Synchronized_ScopeGuard_##name( ( obj ) )
#define ___SYNCHRONIZED_OBJECT(name, obj)       __SYNCHRONIZED_OBJECT(name, obj)

#define __SYNCHRONIZED_SCOPE_BY_NAME(name)      static XeCore::Common::Concurrent::Synchronized \
                                                __XeCore_Common_Concurrent_Synchronized_##name; \
                                                __SYNCHRONIZED_OBJECT( name, __XeCore_Common_Concurrent_Synchronized_##name )
#define ___SYNCHRONIZED_SCOPE_BY_NAME(name)     __SYNCHRONIZED_SCOPE_BY_NAME(name)

#if defined( NDEBUG ) || !defined( LOG_SYNCHRONIZATION_POINTS )
#define SYNCHRONIZED_HERE                      ___SYNCHRONIZED_SCOPE_BY_NAME( __COUNTER__ )
#define SYNCHRONIZED_OBJECT(obj)               ___SYNCHRONIZED_OBJECT( __COUNTER__, obj )
#else
#define __QUOTTED(name)                         #name
#define _SYNCHRONIZED_HERE                      ___SYNCHRONIZED_SCOPE_BY_NAME( __COUNTER__ )
#define _SYNCHRONIZED_OBJECT(obj)               ___SYNCHRONIZED_OBJECT( __COUNTER__, obj )
#define _SYNCHRONIZED_HERE_(file, line)         LOGNL( "SYNCHRONIZED_HERE: " __QUOTTED( file ) ":" __QUOTTED( line ) ); _SYNCHRONIZED_HERE;
#define _SYNCHRONIZED_OBJECT_(obj, file, line)  LOGNL( "SYNCHRONIZED_OBJECT: " __QUOTTED( obj ) " -> " __QUOTTED( file ) ":" __QUOTTED( line ) ); _SYNCHRONIZED_OBJECT( obj );
#define SYNCHRONIZED_HERE                       _SYNCHRONIZED_HERE_( __FILE__, __LINE__ )
#define SYNCHRONIZED_OBJECT(obj)                _SYNCHRONIZED_OBJECT_( obj, __FILE__, __LINE__ )
#endif

namespace XeCore
{
    namespace Common
    {
        namespace Concurrent
        {
            class Synchronized
                : public virtual IRtti
                , public virtual MemoryManager::Manageable
            {
                friend class Condition;

                RTTI_CLASS_DECLARE( Synchronized );

            public:
                                                Synchronized();
                                                ~Synchronized();

                void					        lock();
                bool                            tryLock();
                void					        unlock();
                bool                            isLocked();

            private:
                tthread::mutex*			        m_mutex;

            public:
                class ScopeGuard
                    : public virtual IRtti
                    , public virtual MemoryManager::Manageable
                {
                    RTTI_CLASS_DECLARE( ScopeGuard );

                public:
                                                ScopeGuard();
                                                explicit ScopeGuard( Synchronized& sync );
                                                ~ScopeGuard();

                private:
                    Synchronized*               m_sync;
                };

                class Condition
                    : public virtual IRtti
                    , public virtual MemoryManager::Manageable
                {
                    RTTI_CLASS_DECLARE( Condition );

                public:
                                                Condition();

                    void                        wait( Synchronized& sync );
                    void                        notifyOne();
                    void                        notifyAll();

                private:
                    tthread::condition_variable m_cond;
                };
            };
        }
    }
}

#endif
