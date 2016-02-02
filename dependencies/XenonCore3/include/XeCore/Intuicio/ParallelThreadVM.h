#ifndef __XE_CORE__INTUICIO__PARALLEL_THREAD_VM__
#define __XE_CORE__INTUICIO__PARALLEL_THREAD_VM__

#include "../Common/IRtti.h"
#include "../Common/MemoryManager.h"
#include "../Common/List.h"
#include "../Common/Vector.h"
#include "../Common/Concurrent/Thread.h"

namespace XeCore
{
    namespace Intuicio
    {
        using namespace XeCore::Common;
        using namespace XeCore::Common::Concurrent;

        class ThreadVM;
        class ProgramVM;

        class ParallelThreadVM
			: public virtual IRtti
			, public virtual MemoryManager::Manageable
			, public Thread
        {
            friend class ContextVM;

            RTTI_CLASS_DECLARE( ParallelThreadVM );

        public:
                                            ParallelThreadVM( ThreadVM* owner, ProgramVM* prog, ParallelThreadVM* parent = 0, unsigned int parallelIdx = 0, unsigned int parallelsCount = 1 );
            virtual                         ~ParallelThreadVM();

            void                            pushFragment( unsigned int begin, unsigned int end, unsigned int startAddress );
            void                            stop();
            virtual void                    run();
            bool                            runWithin( unsigned int startAddress );
            virtual void                    onComplete();
            void                            waitForChildren();
            FORCEINLINE bool                isWaiting() { SYNCHRONIZED_OBJECT( m_childrenSync ); return !m_children.empty(); };
            FORCEINLINE unsigned int        getParallelIdx() { return m_parallelIdx; };
            FORCEINLINE unsigned int        getParallelsCount() { return m_parallelsCount; };

        protected:
            void                            addChild( ParallelThreadVM* child );
            void                            removeChild( ParallelThreadVM* child );

        private:
            struct ProgramFragment
            {
                unsigned int                begin;
                unsigned int                end;
                unsigned int                start;
            };

            struct StackPosition
            {
                unsigned int                last;
                unsigned int                address;
            };

            ThreadVM*                       m_owner;
            unsigned int                    m_parallelIdx;
            unsigned int                    m_parallelsCount;
            ParallelThreadVM*               m_parent;
            ProgramVM*                      m_program;
            List< ProgramFragment >         m_fragments;
            int*                            m_regI;
            float*                          m_regF;
            unsigned int                    m_regIcount;
            unsigned int                    m_regFcount;
            byte*                           m_stack;
            unsigned int                    m_stackSize;
            unsigned int                    m_stackPos;
            List< StackPosition >           m_stackFrames;
            volatile bool                   m_running;
            Synchronized                    m_sync;
            Synchronized                    m_waitSync;
            Synchronized::Condition         m_waitCond;
            Synchronized                    m_childrenSync;
            Vector< ParallelThreadVM* >     m_children;
        };
    }
}

#endif
