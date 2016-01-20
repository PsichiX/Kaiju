#include "../include/grammar.h"
#include "../include/compiler.h"
#include "../include_private/ast_generator.h"
#include "../include_private/compiler_actions.h"
#include "../include_private/compiler_controls.h"
#include <string>
#include <iostream>
#include <pegtl/analyze.hh>

#define ISC_STACK_SIZE      (8 * 1024)
#define ISC_REGISTERS_I     (8)
#define ISC_REGISTERS_F     (8)

namespace Kaiju
{
    namespace Compiler
    {

        bool compileToAST( const std::string& input, ASTNode& output, std::stringstream& log )
        {
            pegtl::analyze< Kaiju::Grammar::grammar >();
            output = ASTNode();
            REGISTER_TYPE( Kaiju::Grammar::identifier, "identifier", false );
            REGISTER_TYPE( Kaiju::Grammar::object_create, "object_create", true );
            REGISTER_TYPE( Kaiju::Grammar::object_destroy_statement, "object_destroy_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Number::integer_literal, "number.integer_literal", false );
            REGISTER_TYPE( Kaiju::Grammar::Number::float_literal, "number.float_literal", false );
            REGISTER_TYPE( Kaiju::Grammar::Number::hex_literal, "number.hex_literal", false );
            REGISTER_TYPE( Kaiju::Grammar::number, "number", true );
            REGISTER_TYPE( Kaiju::Grammar::string, "string", false );
            REGISTER_TYPE( Kaiju::Grammar::null_value, "null_value", false );
            REGISTER_TYPE( Kaiju::Grammar::field, "field", true );
            REGISTER_TYPE( Kaiju::Grammar::access_value, "access_value", true );
            REGISTER_TYPE( Kaiju::Grammar::value, "value", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::prefix, "variable.prefix", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::prefix_static, "variable.prefix_static", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::declaration, "variable.declaration", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::declaration_assignment, "variable.declaration_assignment", true );
            REGISTER_TYPE( Kaiju::Grammar::Variable::assignment, "variable.assignment", true );
            REGISTER_TYPE( Kaiju::Grammar::variable, "variable", true );
            REGISTER_TYPE( Kaiju::Grammar::block, "block", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::Method::prefix, "class.method.prefix", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::Method::prefix_static, "class.method.prefix_static", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::Method::Definition::argument_list, "class.method.definition.argument_list", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::Method::definition_statement, "class.method.definition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::Method::Call::argument_list, "class.method.call.argument_list", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::Method::call, "class.method.call", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::inheritance, "class.inheritance", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::body, "class.body", true );
            REGISTER_TYPE( Kaiju::Grammar::Class::definition_statement, "class.definition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Directive::argument_list, "directive.argument_list", true );
            REGISTER_TYPE( Kaiju::Grammar::Directive::statement, "directive.statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::return_statement, "control_flow.return_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::continue_statement, "control_flow.continue_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::break_statement, "control_flow.break_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::if_statement, "control_flow.if_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::else_statement, "control_flow.else_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::condition_statement, "control_flow.condition_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stage_init, "control_flow.for_stage_init", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stage_condition, "control_flow.for_stage_condition", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_stage_iteration, "control_flow.for_stage_iteration", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::for_statement, "control_flow.for_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::foreach_stage, "control_flow.foreach_stage", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::foreach_statement, "control_flow.foreach_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::ControlFlow::while_statement, "control_flow.while_statement", true );
            REGISTER_TYPE( Kaiju::Grammar::Operator::binary_operator, "operator.binary_operator", false );
            REGISTER_TYPE( Kaiju::Grammar::Operator::binary_operation, "operator.binary_operation", true );
            REGISTER_TYPE( Kaiju::Grammar::Operator::unary_operator, "operator.unary_operator", false );
            REGISTER_TYPE( Kaiju::Grammar::Operator::unary_operation, "operator.unary_operation", true );
            REGISTER_TYPE( Kaiju::Grammar::statement_inner, "statement_inner", true );
            REGISTER_TYPE( Kaiju::Grammar::statement_outter, "statement_outter", true );
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
