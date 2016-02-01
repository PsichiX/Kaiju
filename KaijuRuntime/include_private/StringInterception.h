#ifndef ___STRING_INTERCEPTION___
#define ___STRING_INTERCEPTION___

#include "../include/Runtime.h"
#include <XeCore/Intuicio/ContextVM.h>

namespace Kaiju
{
    class StringInterception : public XeCore::Intuicio::ContextVM::OnInterceptListener
    {
    public:
        enum Code
        {
            C_INT_TO_STRING,
            C_STRING_TO_INT,
            C_FLOAT_TO_STRING,
            C_STRING_TO_FLOAT,
            C_BOOL_TO_STRING,
            C_STRING_TO_BOOL,
            C_POINTER_TO_STRING,
            C_STRING_TO_POINTER
        };

        StringInterception( Runtime* owner, XeCore::Intuicio::ContextVM* context ) : m_owner( owner ), m_context( context ) {};
        virtual ~StringInterception();

        virtual bool onIntercept( XeCore::Intuicio::ParallelThreadVM* caller, uint32_t code );

    private:
        Runtime* m_owner;
        XeCore::Intuicio::ContextVM* m_context;
    };
}

#endif
