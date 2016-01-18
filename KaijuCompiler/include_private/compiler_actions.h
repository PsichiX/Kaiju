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

            /*AST_GENERATOR_ACTION( Kaiju::Grammar::static_keyword );
            AST_GENERATOR_ACTION( Kaiju::Grammar::namespace_path );
            AST_GENERATOR_ACTION( Kaiju::Grammar::namespace_block );
            AST_GENERATOR_ACTION( Kaiju::Grammar::using_namespace_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::public_keyword );
            AST_GENERATOR_ACTION( Kaiju::Grammar::private_keyword );
            AST_GENERATOR_ACTION( Kaiju::Grammar::protected_keyword );
            AST_GENERATOR_ACTION( Kaiju::Grammar::internal_keyword );
            AST_GENERATOR_ACTION( Kaiju::Grammar::access_mode );
            AST_GENERATOR_ACTION( Kaiju::Grammar::identifier );
            AST_GENERATOR_ACTION( Kaiju::Grammar::array_specifier );
            AST_GENERATOR_ACTION( Kaiju::Grammar::alias_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::default_value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::object_create );
            AST_GENERATOR_ACTION( Kaiju::Grammar::object_destroy_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Number::integer_literal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Number::float_literal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Number::hex_literal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::number );
            AST_GENERATOR_ACTION( Kaiju::Grammar::string );
            AST_GENERATOR_ACTION( Kaiju::Grammar::field );
            AST_GENERATOR_ACTION( Kaiju::Grammar::value );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::add );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::subtract );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::multiply );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::divide );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::assign );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::add_assign );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::subtract_assign );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::multiply_assign );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::divide_assign );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::negate_assign );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::increment );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::decrement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::logical_and );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::logical_or );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::logical_not );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_and );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_or );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_xor );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_not );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_lshift );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_rshift );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_equal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_not_equal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_less );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_greater );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_less_or_equal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_greater_or_equal );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::array_access_operator );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::operation_prefix );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::operation_suffix );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::binary_operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::logical_operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::bitwise_operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::conditional_operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::operation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Operators::assignment );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::return_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::continue_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::break_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::if_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::else_if_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::else_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::condition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::inlined_condition_test );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::inlined_condition_true );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::inlined_condition_false );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::inlined_condition );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stage_init );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stage_condition );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stage_iteration );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_stages );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::for_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::foreach_stage );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::foreach_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::ControlFlow::while_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::declaration );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::assignment_expression );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::declaration_assignment );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::declaration_instantiation );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Variable::assignment );
            AST_GENERATOR_ACTION( Kaiju::Grammar::variable );
            AST_GENERATOR_ACTION( Kaiju::Grammar::variable_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::block );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::Definition::argument_list );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::Definition::arguments );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::definition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::definition_anonymous );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::definition_operator_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::Call::argument_list );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::Call::arguments );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::call );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Function::call_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::type );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::type_path );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::template_parameters );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::inheritance );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::constructor );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::destructor );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::property_getter );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::property_setter );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::property );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::body );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::definition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::method_call );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Class::method_call_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::type_data );
            AST_GENERATOR_ACTION( Kaiju::Grammar::type_object );
            AST_GENERATOR_ACTION( Kaiju::Grammar::type_pointer );
            AST_GENERATOR_ACTION( Kaiju::Grammar::type_specifier );
            AST_GENERATOR_ACTION( Kaiju::Grammar::cast_suffix );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Enumeration::field );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Enumeration::body );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Enumeration::definition_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::MetaAttribute::meta_instance );
            AST_GENERATOR_ACTION( Kaiju::Grammar::MetaAttribute::definition );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::local_mode );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::parent_mode );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::global_mode );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::mode );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::hardware_repeats );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::repeats );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::block_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::Parallel::group_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::parallel );
            AST_GENERATOR_ACTION( Kaiju::Grammar::expression_bracket );
            AST_GENERATOR_ACTION( Kaiju::Grammar::expression_atomic );
            AST_GENERATOR_ACTION( Kaiju::Grammar::expression );
            AST_GENERATOR_ACTION( Kaiju::Grammar::isc_inlined_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::inject_inlined_statement );
            AST_GENERATOR_ACTION( Kaiju::Grammar::statement );*/

        }
    }
}

#endif
