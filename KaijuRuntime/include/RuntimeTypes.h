#ifndef ___RUNTIME_TYPES___
#define ___RUNTIME_TYPES___

#define KAIJU_STRUCT_PACKED __attribute__ ((packed))

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
        } KAIJU_STRUCT_PACKED;

        struct ___MethodMetaInfo
        {
            Byte name[ 128 ];
            Int namelen;
            Address uid;
            Int isstatic;
            Int address;
            Int argsCount;
            Address owner;
        } KAIJU_STRUCT_PACKED;

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
            Int creatorAddress;
            Int finalizerAddress;
        } KAIJU_STRUCT_PACKED;
    }

    namespace Atom
    {
        struct ___Atom
        {
            VM::Address ___classMetaInfo;
        } KAIJU_STRUCT_PACKED;

        struct Object : ___Atom {} KAIJU_STRUCT_PACKED;

        struct Int : Object
        {
            VM::Int ___data;
        } KAIJU_STRUCT_PACKED;

        struct Float : Object
        {
            VM::Float ___data;
        } KAIJU_STRUCT_PACKED;

        struct String : Object
        {
            VM::Address ___data;
            VM::Int ___size;
        } KAIJU_STRUCT_PACKED;

        struct Bool : Object
        {
            VM::Int ___data;
        } KAIJU_STRUCT_PACKED;

        struct Pointer : Object
        {
            VM::Address ___data;
        } KAIJU_STRUCT_PACKED;

        struct Array : Object
        {
            VM::Address ___data;
            VM::Int ___count;
        } KAIJU_STRUCT_PACKED;
    }
}

#endif
