#include "../include/grammar.h"
#include "../include/compiler.h"
#include "../include_private/ast_generator.h"
#include "../include_private/compiler_actions.h"
#include "../include_private/compiler_controls.h"
#include <string>
#include <iostream>
#include <pegtl/analyze.hh>

namespace Kaiju
{
    namespace Compiler
    {

        bool compileToAST( const std::string& input, ASTNode& output, std::stringstream& log )
        {
            pegtl::analyze< Kaiju::Grammar::grammar >();
            output = ASTNode();
            /*REGISTER_TYPE( Kaiju::Grammar::static_keyword, "static_keyword", false );
            REGISTER_TYPE( Kaiju::Grammar::namespace_block, "namespace_path", true );
            REGISTER_TYPE( Kaiju::Grammar::namespace_block, "namespace_block", true );
            REGISTER_TYPE( Kaiju::Grammar::using_namespace_statement, "using_namespace_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::public_keyword, "public_keyword", false );
            REGISTER_TYPE( Kaiju::Grammar::private_keyword, "private_keyword", false );
            REGISTER_TYPE( Kaiju::Grammar::protected_keyword, "protected_keyword", false );
            REGISTER_TYPE( Kaiju::Grammar::internal_keyword, "internal_keyword", false );
            REGISTER_TYPE( Kaiju::Grammar::access_mode, "access_mode", true );
            REGISTER_TYPE( Kaiju::Grammar::identifier, "identifier", false );
            REGISTER_TYPE( Kaiju::Grammar::array_specifier, "array_specifier", true );
            REGISTER_TYPE( Kaiju::Grammar::alias_statement, "alias_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::default_value, "default_value", true );
            REGISTER_TYPE( Kaiju::Grammar::object_create, "object_create", true );
            REGISTER_TYPE( Kaiju::Grammar::object_destroy_statement, "object_destroy_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Number::integer_literal, "number.integer_literal", false );
            REGISTER_TYPE( Kaiju::Grammar::Number::float_literal, "number.float_literal", false );
            REGISTER_TYPE( Kaiju::Grammar::Number::hex_literal, "number.hex_literal", false );
            REGISTER_TYPE( Kaiju::Grammar::number, "number", true );
            REGISTER_TYPE( Kaiju::Grammar::string, "string", false );
            REGISTER_TYPE( Kaiju::Grammar::field, "field", true );
            REGISTER_TYPE( Kaiju::Grammar::value, "value", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::add, "operators.add", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::subtract, "operators.subtract", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::multiply, "operators.multiply", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::divide, "operators.divide", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::assign, "operators.assign", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::add_assign, "operators.add_assign", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::subtract_assign, "operators.subtract_assign", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::multiply_assign, "operators.multiply_assign", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::divide_assign, "operators.divide_assign", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::negate_assign, "operators.negate_assign", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::increment, "operators.increment", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::decrement, "operators.decrement", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::logical_and, "operators.logical_and", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::logical_or, "operators.logical_or", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::logical_not, "operators.logical_not", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_and, "operators.bitwise_and", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_or, "operators.bitwise_or", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_xor, "operators.bitwise_xor", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_not, "operators.bitwise_not", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_lshift, "operators.bitwise_lshift", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_rshift, "operators.bitwise_rshift", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_equal, "operators.conditional_equal", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_not_equal, "operators.conditional_not_equal", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_less, "operators.conditional_less", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_greater, "operators.conditional_greater", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_less_or_equal, "operators.conditional_less_or_equal", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_greater_or_equal, "operators.conditional_greater_or_equal", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::array_access_operator, "operators.array_access_operator", false );
            REGISTER_TYPE( Kaiju::Grammar::Operators::operation_prefix, "operators.operation_prefix", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::operation_suffix, "operators.operation_suffix", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::binary_operation, "operators.binary_operation", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::logical_operation, "operators.logical_operation", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::bitwise_operation, "operators.bitwise_operation", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::conditional_operation, "operators.conditional_operation", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::operation, "operators.operation", true );
            REGISTER_TYPE( Kaiju::Grammar::Operators::assignment, "operators.assignment", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::return_statement, "control_flow.return_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::continue_statement, "control_flow.continue_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::break_statement, "control_flow.break_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::if_statement, "control_flow.if_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::else_if_statement, "control_flow.else_if_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::else_statement, "control_flow.else_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::condition_statement, "control_flow.condition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::inlined_condition_test, "control_flow.inlined_condition_test", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::inlined_condition_true, "control_flow.inlined_condition_true", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::inlined_condition_false, "control_flow.inlined_condition_false", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::inlined_condition, "control_flow.inlined_condition", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stage_init, "control_flow.for_stage_init", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stage_condition, "control_flow.for_stage_condition", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stage_iteration, "control_flow.for_stage_iteration", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stages, "control_flow.for_stages", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_statement, "control_flow.for_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::foreach_stage, "control_flow.foreach_stage", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::foreach_statement, "control_flow.foreach_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::while_statement, "control_flow.while_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::declaration, "variable.declaration", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::assignment_expression, "variable.assignment_expression", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::declaration_assignment, "variable.declaration_assignment", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::declaration_instantiation, "variable.declaration_instantiation", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::assignment, "variable.assignment", true );
            REGISTER_TYPE( Kaiju::Grammar::variable, "variable", true );
            REGISTER_TYPE( Kaiju::Grammar::variable_statement, "variable_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::block, "block", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::Definition::argument_list, "function.definition.argument_list", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::Definition::arguments, "function.definition.arguments", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::definition_statement, "function.definition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::definition_anonymous, "function.definition_anonymous", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::definition_operator_statement, "function.definition_operator_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::Call::argument_list, "function.call.argument_list", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::Call::arguments, "function.call.arguments", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::call, "function.call", true );
            REGISTER_TYPE( Kaiju::Grammar::Function::call_statement, "function.call_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::type, "class.type", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::template_parameters, "class.template_parameters", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::inheritance, "class.inheritance", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::constructor, "class.constructor", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::destructor, "class.destructor", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::property_getter, "class.property_getter", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::property_setter, "class.property_setter", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::property, "class.property", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::body, "class.body", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::definition_statement, "class.definition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::method_call, "class.method_call", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::method_call_statement, "class.method_call_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::type_data, "type_data", true );
            REGISTER_TYPE( Kaiju::Grammar::type_object, "type_object", true );
            REGISTER_TYPE( Kaiju::Grammar::type_pointer, "type_pointer", true );
            REGISTER_TYPE( Kaiju::Grammar::type_specifier, "type_specifier", true );
            REGISTER_TYPE( Kaiju::Grammar::cast_suffix, "cast_suffix", true );
            REGISTER_TYPE( Kaiju::Grammar::Enumeration::field, "enum.field", true );
            REGISTER_TYPE( Kaiju::Grammar::Enumeration::body, "enum.body", true );
            REGISTER_TYPE( Kaiju::Grammar::Enumeration::definition_statement, "enum.definition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::MetaAttribute::meta_instance, "meta_attribute.meta_instance", true );
            REGISTER_TYPE( Kaiju::Grammar::MetaAttribute::definition, "meta_attribute.definition", true );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::local_mode, "parallel.local_mode", false );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::parent_mode, "parallel.parent_mode", false );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::global_mode, "parallel.global_mode", false );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::mode, "parallel.mode", true );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::hardware_repeats, "parallel.hardware_repeats", false );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::repeats, "parallel.repeats", true );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::block_statement, "parallel.block_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Parallel::group_statement, "parallel.group_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::parallel, "parallel", true );
            REGISTER_TYPE( Kaiju::Grammar::expression_bracket, "expression_bracket", true );
            REGISTER_TYPE( Kaiju::Grammar::expression_atomic, "expression_atomic", true );
            REGISTER_TYPE( Kaiju::Grammar::expression, "expression", true );
            REGISTER_TYPE( Kaiju::Grammar::isc_inlined_statement, "isc_inlined_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::inject_inlined_statement, "inject_inlined_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::statement, "statement", true );*/
            ASTGenerator::getInstance()->acquire( &output, input.data() );
            try
            {
                bool status = pegtl::parse< Kaiju::Grammar::grammar, Actions::ast_generator, Controls::ast_generator >( input, "input" );
                if( status )
                    ASTGenerator::getInstance()->assignUIDs();
                ASTGenerator::getInstance()->release();
                ASTGenerator::destroyInstance();
                return status;
            }
            catch( const pegtl::parse_error& ex )
            {
                log << ex.what() << std::endl;
                ASTGenerator::getInstance()->release();
                ASTGenerator::destroyInstance();
                return false;
            }
        }

        void _convertASTNodeToIRVT( ASTNode& input, std::stringstream& output, int level )
        {
            output << "[" << input.uid << "]";
            for( int i = 0; i < level; ++i )
                output << "-";
            if( input.value.empty() )
                output << "(" << input.type << "){" << input.line << "|" << input.column << "|" << input.position << "|" << input.size << "}" << std::endl;
            else
            {
                std::string r = std::string_replace( input.value, "\\", "\\\\" );
                r = std::string_replace( r, "\r", "\\r" );
                r = std::string_replace( r, "\n", "\\n" );
                r = std::string_replace( r, "\f", "\\f" );
                r = std::string_replace( r, "\v", "\\v" );
                r = std::string_replace( r, "\b", "\\b" );
                r = std::string_replace( r, "\a", "\\a" );
                output << "(" << input.type << "){" << input.line << "|" << input.column << "|" << input.position << "|" << input.size << "}" << r << std::endl;
            }
            level++;
            for( std::vector< ASTNode >::iterator it = input.nodes.begin(); it != input.nodes.end(); ++it )
                _convertASTNodeToIRVT( *it, output, level );
        }

        void convertASTNodeToIRVT( ASTNode& input, std::stringstream& output )
        {
            _convertASTNodeToIRVT( input, output, 0 );
        }

    }
}
