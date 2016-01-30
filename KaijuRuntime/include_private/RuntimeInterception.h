#ifndef ___RUNTIME_INTERCEPTION___
#define ___RUNTIME_INTERCEPTION___

#include "../include/Runtime.h"
#include <XeCore/Intuicio/ContextVM.h>

namespace Kaiju
{
    class RuntimeInterception : public XeCore::Intuicio::ContextVM::OnInterceptListener
    {
    public:
        enum Code
        {
            C_REGISTER_TYPE
        };

        RuntimeInterception( Runtime* owner, XeCore::Intuicio::ContextVM* context ) : m_owner( owner ), m_context( context ) {};
        virtual ~RuntimeInterception();

        virtual bool onIntercept( XeCore::Intuicio::ParallelThreadVM* caller, uint32_t code );

    private:
        Runtime* m_owner;
        XeCore::Intuicio::ContextVM* m_context;
    };
}

#endif
