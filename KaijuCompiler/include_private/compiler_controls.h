#ifndef __COMPILER_CONTROLS_H__
#define __COMPILER_CONTROLS_H__

#include "../include/grammar.h"
#include "ast_generator.h"

namespace Kaiju
{
    namespace Compiler
    {
        namespace Controls
        {

            template< typename Rule >
            struct ast_generator
            {
                template< typename Input, typename ... States >
                static void start( const Input & in, States && ... )
                {
                    ASTGenerator::getInstance()->pushNode( pegtl::internal::demangle< Rule >() );
                }

                template< typename Input, typename ... States >
                static void success( const Input & in, States && ... ) {}

                template< typename Input, typename ... States >
                static void failure( const Input & in, States && ... )
                {
                    ASTGenerator::getInstance()->popNode( pegtl::internal::demangle< Rule >() );
                }

                template< typename Input, typename ... States >
                static void raise( const Input & in, States && ... )
                {
                    throw pegtl::parse_error( "parse error matching " + pegtl::internal::demangle< Rule >(), in );
                }

                template< pegtl::apply_mode A, template< typename ... > class Action, template< typename ... > class Control, typename Input, typename ... States >
                static bool match( Input & in, States && ... st )
                {
                    return pegtl::internal::rule_match_one< Rule, A, Action, Control >::match( in, st ... );
                }
            };

        }
    }
}

#endif
