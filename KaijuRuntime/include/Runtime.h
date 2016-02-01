#ifndef ___RUNTIME___
#define ___RUNTIME___

#include <string>
#include <vector>

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
        bool refManagedObject( int64_t ptrDst, int64_t ptrSrc );
        bool unrefManagedObject( int64_t ptr );
        void* getManagedObjectDataRaw( int64_t ptr );
        template< typename T >
        inline T* getManagedObjectData( int64_t ptr ) { return (T*)getManagedObjectDataRaw( ptr ); };
        bool newManagedObjectRaw( int64_t ptr, uint32_t size, const std::string& classId );
        template< typename T >
        inline bool newManagedObject( int64_t ptr, uint32_t count, const std::string& classId ) { return newManagedObjectRaw( ptr, count * sizeof( T ), classId ); };
        uint32_t getManagedObjectRefCount( int64_t ptr );
        uint32_t getManagedObjectFinalizerAddress( int64_t ptr );
        void* getTypeByNameRaw( const std::string& name );
        template< typename T >
        inline T* getTypeByName( const std::string& name ) { return (T*)getTypeByNameRaw( name ); };
        void* getTypeByUIDRaw( int64_t uid );
        template< typename T >
        inline T* getTypeByUID( int64_t uid ) { return (T*)getTypeByUIDRaw( uid ); };
        void registerType( int64_t ptr );

    private:
        int m_code;
        void* m_context;
        std::vector< int64_t > m_types;
    };
}

#endif
