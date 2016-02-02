#ifndef ___RUNTIME___
#define ___RUNTIME___

#include <string>
#include <vector>
#include "RuntimeTypes.h"

namespace Kaiju
{
    class Runtime
    {
    public:
        Runtime();
        ~Runtime();

        inline int getCode() { return m_code; };

        int run( const std::string& path, const std::vector< std::string >& args );
        bool stackPush( int64_t caller, const void* src, uint32_t size );
        template< typename T >
        inline bool stackPush( int64_t caller, const T* src ) { return stackPush( caller, src, sizeof( T ) ); };
        bool stackPop( int64_t caller, void* dst, uint32_t size );
        template< typename T >
        inline bool stackPop( int64_t caller, T* dst ) { return stackPop( caller, dst, sizeof( T ) ); };
        int64_t createManagedObject();
        bool deleteManagedObject( int64_t ptr );
        bool finalizeManagedObject( int64_t caller, int64_t ptr );
        bool refManagedObject( int64_t ptrDst, int64_t ptrSrc );
        bool unrefManagedObject( int64_t ptr );
        void* getManagedObjectDataRaw( int64_t ptr );
        template< typename T >
        inline T* getManagedObjectData( int64_t ptr ) { return (T*)getManagedObjectDataRaw( ptr ); };
        bool newManagedObjectRaw( int64_t caller, int64_t ptr, uint32_t size, const std::string& classId, int argsCount = -1 );
        template< typename T >
        inline bool newManagedObject( int64_t caller, int64_t ptr, const std::string& classId, int argsCount = -1 ) { return newManagedObjectRaw( caller, ptr, sizeof( T ), classId, argsCount ); };
        uint32_t getManagedObjectRefCount( int64_t ptr );
        VM::___ClassMetaInfo* getType( const std::string& name );
        VM::___FieldMetaInfo* findManagedObjectField( VM::___ClassMetaInfo* type, const std::string& name );
        inline VM::___FieldMetaInfo* findManagedObjectField( const std::string& type, const std::string& name ) { return findManagedObjectField( getType( type ), name ); };
        VM::___FieldMetaInfo* findManagedObjectField( int64_t ptr, const std::string& name );
        VM::___MethodMetaInfo* findManagedObjectMethod( VM::___ClassMetaInfo* type, const std::string& name );
        inline VM::___MethodMetaInfo* findManagedObjectMethod( const std::string& type, const std::string& name ) { return findManagedObjectMethod( getType( type ), name ); };
        VM::___MethodMetaInfo* findManagedObjectMethod( int64_t ptr, const std::string& name );
        bool callManagedObjectMethod( int64_t caller, int64_t thisPtr, VM::___MethodMetaInfo* method, int argsCount = -1 );
        inline bool callManagedObjectMethod( int64_t caller, int64_t thisPtr, VM::___ClassMetaInfo* type, const std::string& name, int argsCount = -1 ) { return callManagedObjectMethod( caller, thisPtr, findManagedObjectMethod( type, name ), argsCount ); };
        inline bool callManagedObjectMethod( int64_t caller, int64_t thisPtr, const std::string& type, const std::string& name, int argsCount = -1 ) { return callManagedObjectMethod( caller, thisPtr, findManagedObjectMethod( type, name ), argsCount ); };
        void registerType( int64_t ptr );

    private:
        int m_code;
        void* m_context;
        std::vector< int64_t > m_types;
    };
}

#endif
