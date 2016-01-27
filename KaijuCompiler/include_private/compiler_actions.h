#ifndef __COMPILER_ACTIONS_H__
#define __COMPILER_ACTIONS_H__

#include "../include/grammar.h"
#include "ast_generator.h"

#define AST_GENERATOR_ACTION( type ) \
    template<> struct ast_generator< type > \
    { \
        static void apply( const pegtl::input & in ) \
        { \
            ASTGenerator::getInstance()->acceptNode( pegtl::internal::demangle< type >(), in.string(), in.line(), in.column(), in.begin(), in.end() ); \
        } \
    }

namespace Kaiju
{
    namespace Compiler
    {
        namespace Actions
        {

            template< typename Rule >
            struct ast_generator : pegtl::nothing< Rule > {};

            AST_GENERATOR_ACTION( Kaiju::Grammar::identifier );
            AST_GENERATOR_ACTION( Kaiju::Grammar::object_create );
            AST_GENERATOR_ACTION( Kaiju::Grammar::object_destroy_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Number::integer_literal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Number::float_literal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Number::hex_literal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::number );
            AST_GENERATOR_ACTION( Kaiju::Grammar::string );
            AST_GENERATOR_ACTION( Kaiju::Grammar::null_value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::false_value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::true_value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::access_value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::field );
            AST_GENERATOR_ACTION( Kaiju::Grammar::value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::prefix );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::prefix_static );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::declaration );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::declaration_assignment );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::assignment );
            AST_GENERATOR_ACTION( Kaiju::Grammar::variable );
            AST_GENERATOR_ACTION( Kaiju::Grammar::block );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::prefix );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::prefix_static );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::Definition::argument_list );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::Definition::argument_params );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::definition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::Call::argument_list );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::Method::call );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::inheritance );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::body );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::definition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Directive::argument_list );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Directive::statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::return_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::continue_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::break_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::if_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::else_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::condition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stage_init );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stage_condition );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stage_iteration );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::foreach_stage );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::foreach_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::while_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operator::binary_operator );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operator::binary_operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operator::unary_operator );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operator::unary_operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::statement_inner );
            AST_GENERATOR_ACTION( Kaiju::Grammar::statement_outter );

        }
    }
}

#endif
