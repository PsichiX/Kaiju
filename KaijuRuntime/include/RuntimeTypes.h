#ifndef ___RUNTIME_TYPES___
#define ___RUNTIME_TYPES___

namespace Kaiju
{
    namespace VM
    {
        typedef int32_t Int;
        typedef float Float;
        typedef int8_t Byte;
        typedef int64_t Address;

        struct ___FieldMetaInfo
        {
            Byte name[ 128 ];
            Int namelen;
            Address uid;
            Int isstatic;
            Int offset;
            Address owner;
        };

        struct ___MethodMetaInfo
        {
            Byte name[ 128 ];
            Int namelen;
            Address uid;
            Int isstatic;
            Address owner;
        };

        struct ___ClassMetaInfo
        {
            Byte name[ 128 ];
            Int namelen;
            Address uid;
            Address inheritanceMetaInfo;
            Int fieldsCount;
            Int methodsCount;
            Address fields;
            Address methods;
        };
    }

    namespace Atom
    {
        struct ___Atom
        {
            VM::Address ___classMetaInfo;
        };

        struct Object : ___Atom {};

        struct Int : Object
        {
            VM::Int ___data;
        };

        struct Float : Object
        {
            VM::Float ___data;
        };

        struct String : Object
        {
            VM::Address ___data;
            VM::Int ___size;
        };

        struct Bool : Object
        {
            VM::Int ___data;
        };

        struct Pointer : Object
        {
            VM::Address ___data;
        };

        struct Array : Object
        {
            VM::Address ___data;
            VM::Int ___count;
        };
    }
}

#endif
