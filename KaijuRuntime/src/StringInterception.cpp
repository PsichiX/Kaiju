#include "../include_private/StringInterception.h"
#include "../include/RuntimeTypes.h"
#include <sstream>

namespace Kaiju
{
    StringInterception::~StringInterception()
    {
        m_context = 0;
        m_owner = 0;
    }

    bool StringInterception::onIntercept( XeCore::Intuicio::ParallelThreadVM* caller, uint32_t code )
    {
        if( !m_context || !m_owner )
            return false;
        if( code == C_INT_TO_STRING )
        {
            int64_t intPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &intPtr ) )
                return false;
            Atom::Int* intData = m_owner->getManagedObjectData< Atom::Int >( intPtr );
            std::stringstream ss;
            ss << intData->___data;
            char* cstr = xnew char[ ss.str().length() + 1 ];
            memcpy( cstr, ss.str().c_str(), ss.str().length() + 1 );
            int64_t strPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::String >( (int64_t)caller, strPtr, "String" ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            if( strData->___data )
                XeCore::Common::MemoryManager::instance()->_delete( (void*)strData->___data );
            strData->___size = ss.str().length();
            strData->___data = (int64_t)cstr;
            return m_owner->stackPush( (int64_t)caller, &strPtr );
        }
        else if( code == C_STRING_TO_INT )
        {
            int64_t strPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &strPtr ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            std::stringstream ss( (char*)strData->___data );
            int32_t cint = 0;
            ss >> cint;
            int64_t intPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::Int >( (int64_t)caller, intPtr, "Int" ) )
                return false;
            Atom::Int* intData = m_owner->getManagedObjectData< Atom::Int >( intPtr );
            intData->___data = cint;
            return m_owner->stackPush( (int64_t)caller, &intPtr );
        }
        else if( code == C_FLOAT_TO_STRING )
        {
            int64_t floatPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &floatPtr ) )
                return false;
            Atom::Float* floatData = m_owner->getManagedObjectData< Atom::Float >( floatPtr );
            std::stringstream ss;
            ss << floatData->___data;
            char* cstr = xnew char[ ss.str().length() + 1 ];
            memcpy( cstr, ss.str().c_str(), ss.str().length() + 1 );
            int64_t strPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::String >( (int64_t)caller, strPtr, "String" ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            if( strData->___data )
                XeCore::Common::MemoryManager::instance()->_delete( (void*)strData->___data );
            strData->___size = ss.str().length();
            strData->___data = (int64_t)cstr;
            return m_owner->stackPush( (int64_t)caller, &strPtr );
        }
        else if( code == C_STRING_TO_FLOAT )
        {
            int64_t strPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &strPtr ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            std::stringstream ss( (char*)strData->___data );
            float cfloat = 0.0f;
            ss >> cfloat;
            int64_t floatPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::Float >( (int64_t)caller, floatPtr, "Float" ) )
                return false;
            Atom::Float* floatData = m_owner->getManagedObjectData< Atom::Float >( floatPtr );
            floatData->___data = cfloat;
            return m_owner->stackPush( (int64_t)caller, &floatPtr );
        }
        else if( code == C_BOOL_TO_STRING )
        {
            int64_t boolPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &boolPtr ) )
                return false;
            Atom::Bool* boolData = m_owner->getManagedObjectData< Atom::Bool >( boolPtr );
            std::string ss = boolData->___data ? "true" : "false";
            char* cstr = xnew char[ ss.length() + 1 ];
            memcpy( cstr, ss.c_str(), ss.length() + 1 );
            int64_t strPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::String >( (int64_t)caller, strPtr, "String" ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            if( strData->___data )
                XeCore::Common::MemoryManager::instance()->_delete( (void*)strData->___data );
            strData->___size = ss.length();
            strData->___data = (int64_t)cstr;
            return m_owner->stackPush( (int64_t)caller, &strPtr );
        }
        else if( code == C_STRING_TO_BOOL )
        {
            int64_t strPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &strPtr ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            std::string ss( (char*)strData->___data );
            int64_t boolPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::Bool >( (int64_t)caller, boolPtr, "Bool" ) )
                return false;
            Atom::Bool* boolData = m_owner->getManagedObjectData< Atom::Bool >( boolPtr );
            boolData->___data = ss == "true" ? 1 : 0;
            return m_owner->stackPush( (int64_t)caller, &boolPtr );
        }
        else if( code == C_POINTER_TO_STRING )
        {
            int64_t ptrPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &ptrPtr ) )
                return false;
            Atom::Pointer* ptrData = m_owner->getManagedObjectData< Atom::Pointer >( ptrPtr );
            std::stringstream ss;
            ss << ptrData->___data;
            char* cstr = xnew char[ ss.str().length() + 1 ];
            memcpy( cstr, ss.str().c_str(), ss.str().length() + 1 );
            int64_t strPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::String >( (int64_t)caller, strPtr, "String" ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            if( strData->___data )
                XeCore::Common::MemoryManager::instance()->_delete( (void*)strData->___data );
            strData->___size = ss.str().length();
            strData->___data = (int64_t)cstr;
            return m_owner->stackPush( (int64_t)caller, &strPtr );
        }
        else if( code == C_STRING_TO_POINTER )
        {
            int64_t strPtr = 0;
            if( !m_owner->stackPop( (int64_t)caller, &strPtr ) )
                return false;
            Atom::String* strData = m_owner->getManagedObjectData< Atom::String >( strPtr );
            std::stringstream ss( (char*)strData->___data );
            int64_t cptr = 0;
            ss >> cptr;
            int64_t ptrPtr = m_owner->createManagedObject();
            if( !m_owner->newManagedObject< Atom::Pointer >( (int64_t)caller, ptrPtr, "Pointer" ) )
                return false;
            Atom::Pointer* ptrData = m_owner->getManagedObjectData< Atom::Pointer >( ptrPtr );
            ptrData->___data = cptr;
            return m_owner->stackPush( (int64_t)caller, &ptrPtr );
        }
        return false;
    }
}
