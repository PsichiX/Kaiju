#ifndef __XE_CORE__INTUICIO__COMPILER_VM__
#define __XE_CORE__INTUICIO__COMPILER_VM__

#include <map>
#include "ProgramVM.h"
#include "../Common/Buffer.h"
#include "../Common/String.h"

namespace XeCore
{
    namespace Intuicio
    {
        using namespace XeCore::Common;

        class CompilerVM
		{
        public:
            static ProgramVM*                               compile( const String& input, String& outErrors );

            static const ProgramVM::Version                 version;
            static const char                               versionString[];

            struct Data
            {
                uint32                                      address;
                uint32                                      size;
                ProgramVM::DataType                         type;
            };

            struct Field
            {
                String                                      type;
                uint32                                      count;
                uint32                                      offset;
            };

            struct Struct
            {
                std::map< String, Field >                   fields;
                uint32                                      size;
            };

            struct ParallelGroup
            {
                uint32                                      address;
                int32                                       count;
                std::vector< uint32 >                       parallels;

                ParallelGroup( uint32 addr, int32 cnt = 0 ) : address( addr ), count( cnt ) {};
            };
        };
    }
}

#endif
