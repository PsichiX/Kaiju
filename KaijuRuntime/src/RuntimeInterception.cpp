#include "../include_private/RuntimeInterception.h"
#include "../include/RuntimeTypes.h"
#include <sstream>

namespace Kaiju
{
    RuntimeInterception::~RuntimeInterception()
    {
        m_context = 0;
        m_owner = 0;
    }

    bool RuntimeInterception::onIntercept( XeCore::Intuicio::ParallelThreadVM* caller, uint32_t code )
    {
        if( !m_owner || !m_context )
            return false;
        if( code == C_REGISTER_TYPE )
        {
            int64_t ptr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &ptr ) )
                return false;
            m_owner->registerType( ptr );
            return true;
        }
        return false;
    }
}

