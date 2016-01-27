#ifndef __GRAMMAR_H__
#define __GRAMMAR_H__

#include "std_extension.h"
#include <pegtl.hh>

namespace Kaiju
{
    namespace Grammar
    {

        struct statement_inner;
        struct statements_inner;
        struct value;
        namespace Class
        {
            namespace Method
            {
                struct call;
                struct call_function;
            }
        }
        namespace Operator
        {
            struct binary_operation;
            struct unary_operation;
        }
        namespace Directive
        {
            struct statement;
        }
        namespace Comment
        {
            struct inlined : pegtl::seq< pegtl::two< '/' >, pegtl::until< pegtl::eolf > > {};
            struct block : pegtl::seq< pegtl::string< '/', '*' >, pegtl::until< pegtl::string< '*', '/' > > > {};
        }
        struct comment : pegtl::sor< Comment::block, Comment::inlined > {};
        struct whitespaces : pegtl::plus< pegtl::sor< pegtl::space, comment > > {};
        struct whitespaces_any : pegtl::star< pegtl::sor< pegtl::space, comment > > {};
        struct semicolons : pegtl::seq< pegtl::one< ';' >, pegtl::star< whitespaces_any, pegtl::one< ';' > > > {};
        struct identifier : pegtl::identifier {};
        struct object_create : pegtl::seq< pegtl::one< '+' >, whitespaces_any, Class::Method::call > {};
        struct object_destroy_statement : pegtl::seq< pegtl::one< '-' >, whitespaces_any, value, whitespaces_any, semicolons > {};
        namespace Number
        {
            struct integer_literal : pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, pegtl::plus< pegtl::digit > > {};
            struct float_literal : pegtl::seq< pegtl::opt< pegtl::one< '+', '-' > >, pegtl::plus< pegtl::digit >, pegtl::one< '.' >, pegtl::plus< pegtl::digit > > {};
            struct hex_literal : pegtl::seq< pegtl::string< '0', 'x' >, pegtl::plus< pegtl::xdigit > > {};
        }
        struct number : pegtl::sor< Number::hex_literal, Number::float_literal, Number::integer_literal > {};
        namespace String
        {
            struct unicode : pegtl::seq< pegtl::one< 'u' >, pegtl::rep< 4, pegtl::must< pegtl::xdigit > > > {};
            struct escaped_character : pegtl::one< '"', '\\', '/', 'b', 'f', 'n', 'r', 't' > {};
            struct escaped : pegtl::sor< escaped_character, unicode > {};
            struct unescaped : pegtl::utf8::range< 0x20, 0x10FFFF > {};
            struct character : pegtl::if_then_else< pegtl::one< '\\' >, pegtl::must< escaped >, unescaped > {};
        }
        struct string : pegtl::seq< pegtl::one< '"' >, pegtl::until< pegtl::one< '"' >, String::character > > {};
        struct null_value : pegtl::string< 'n', 'u', 'l', 'l'  > {};
        struct false_value : pegtl::string< 'f', 'a', 'l', 's', 'e'  > {};
        struct true_value : pegtl::string< 't', 'r', 'u', 'e' > {};
        struct access_value : pegtl::seq< whitespaces_any, pegtl::one< '.' >, whitespaces_any, value > {};
        struct field : pegtl::seq< identifier, whitespaces_any, pegtl::one< ':' >, whitespaces_any, identifier > {};
        struct value : pegtl::seq< pegtl::sor< object_create, Class::Method::call/*, Operator::binary_operation, Operator::unary_operation*/, false_value, true_value, number, string, null_value, field, identifier >, pegtl::opt< access_value > > {};
        namespace Variable
        {
            struct prefix : pegtl::one< '$' > {};
            struct prefix_static : pegtl::string< '~', '$' > {};
            struct assignment;
            struct declaration : pegtl::seq< pegtl::sor< prefix, prefix_static >, whitespaces_any, identifier > {};
            struct declaration_assignment : pegtl::seq< declaration, whitespaces_any, pegtl::one< '=' >, whitespaces_any, value > {};
            struct assignment : pegtl::seq< value, whitespaces_any, pegtl::one< '=' >, whitespaces_any, value > {};
        }
        struct variable : pegtl::sor< Variable::declaration_assignment, Variable::declaration, Variable::assignment > {};
        struct variable_statement : pegtl::seq< variable, whitespaces_any, semicolons > {};
        struct block : pegtl::seq< pegtl::one< '{' >, whitespaces_any, pegtl::opt< statements_inner >, whitespaces_any, pegtl::one< '}' > > {};
        namespace Class
        {
            namespace Method
            {
                struct prefix : pegtl::one< '@' > {};
                struct prefix_static : pegtl::string< '~', '@' > {};
                namespace Definition
                {
                    struct argument_list : pegtl::opt< identifier, pegtl::star< whitespaces_any, pegtl::one< ',' >, whitespaces_any, identifier > > {};
                    struct argument_params : pegtl::string< '.', '.', '.' > {};
                    struct arguments : pegtl::seq< pegtl::one< '(' >, whitespaces_any, pegtl::sor< argument_params, argument_list >, whitespaces_any, pegtl::one< ')' > > {};
                }
                struct definition_statement : pegtl::seq< pegtl::sor< prefix, prefix_static>, whitespaces_any, identifier, whitespaces_any, Definition::arguments, whitespaces_any, block > {};
                namespace Call
                {
                    struct argument_list : pegtl::opt< value, pegtl::star< whitespaces_any, pegtl::one< ',' >, whitespaces_any, value > > {};
                    struct arguments : pegtl::seq< pegtl::one< '(' >, whitespaces_any, argument_list, whitespaces_any, pegtl::one< ')' > > {};
                }
                struct call : pegtl::seq< pegtl::sor< field, identifier >, whitespaces_any, Call::arguments > {};
                struct call_statement : pegtl::seq< call, whitespaces_any, semicolons > {};
            }
            struct prefix : pegtl::one< '#' > {};
            struct inheritance : pegtl::seq< pegtl::one< ':' >, whitespaces_any, identifier > {};
            struct body : pegtl::seq< pegtl::one< '{' >, whitespaces_any, pegtl::star< pegtl::sor< Directive::statement, variable_statement, Method::definition_statement >, whitespaces_any >, pegtl::one< '}' > > {};
            struct definition_statement : pegtl::seq< prefix, whitespaces_any, identifier, pegtl::opt< whitespaces_any, inheritance >, whitespaces_any, body > {};
        }
        namespace Directive
        {
            struct prefix : pegtl::one< '%' > {};
            struct argument_list : pegtl::opt< value, pegtl::star< whitespaces_any, pegtl::one< ',' >, whitespaces_any, value > > {};
            struct arguments : pegtl::seq< pegtl::one< '(' >, whitespaces_any, argument_list, whitespaces_any, pegtl::one< ')' > > {};
            struct statement : pegtl::seq< prefix, whitespaces_any, identifier, pegtl::opt< whitespaces_any, arguments >, whitespaces_any, semicolons > {};
        }
        namespace ControlFlow
        {
            struct return_keyword : pegtl::string< 'r', 'e', 't', 'u', 'r', 'n' > {};
            struct return_statement : pegtl::seq< return_keyword, whitespaces_any, pegtl::opt< value, whitespaces_any >, semicolons > {};
            struct continue_keyword : pegtl::string< 'c', 'o', 'n', 't', 'i', 'n', 'u', 'e' > {};
            struct continue_statement : pegtl::seq< continue_keyword, whitespaces_any, semicolons > {};
            struct break_keyword : pegtl::string< 'b', 'r', 'e', 'a', 'k' > {};
            struct break_statement : pegtl::seq< break_keyword, whitespaces_any, semicolons > {};
            struct if_keyword : pegtl::string< 'i', 'f' > {};
            struct else_keyword : pegtl::string< 'e', 'l', 's', 'e' > {};
            struct if_statement : pegtl::seq< if_keyword, whitespaces_any, pegtl::one< '(' >, whitespaces_any, value, whitespaces_any, pegtl::one< ')' >, whitespaces_any, pegtl::sor< block, statement_inner > > {};
            struct else_if_statement : pegtl::seq< else_keyword, whitespaces_any, if_statement > {};
            struct else_statement : pegtl::seq< else_keyword, whitespaces_any, pegtl::sor< block, statement_inner > > {};
            struct condition_statement : pegtl::seq< if_statement, whitespaces_any, pegtl::star< else_if_statement, whitespaces_any >, pegtl::opt< else_statement > > {};
            struct for_keyword : pegtl::string< 'f', 'o', 'r' > {};
            struct for_stage_init : pegtl::opt< variable > {};
            struct for_stage_condition : pegtl::opt< value > {};
            struct for_stage_iteration : pegtl::opt< value > {};
            struct for_stages : pegtl::seq< pegtl::one< '(' >, whitespaces_any, for_stage_init, whitespaces_any, pegtl::one< ';' >, whitespaces_any, for_stage_condition, whitespaces_any, pegtl::one< ';' >, whitespaces_any, for_stage_iteration, whitespaces_any, pegtl::one< ')' > > {};
            struct for_statement : pegtl::seq< for_keyword, whitespaces_any, for_stages, whitespaces_any, pegtl::sor< block, statement_inner > > {};
            struct foreach_keyword : pegtl::string< 'f', 'o', 'r', 'e', 'a', 'c', 'h' > {};
            struct foreach_stage : pegtl::seq< pegtl::one< '(' >, whitespaces_any, identifier, whitespaces_any, pegtl::string< 'i', 'n' >, whitespaces_any, value, whitespaces_any, pegtl::one< ')' > > {};
            struct foreach_statement : pegtl::seq< foreach_keyword, whitespaces_any, foreach_stage, whitespaces_any, pegtl::sor< block, statement_inner > > {};
            struct while_keyword : pegtl::string< 'w', 'h', 'i', 'l', 'e' > {};
            struct while_statement : pegtl::seq< while_keyword, whitespaces_any, pegtl::one< '(' >, whitespaces_any, value, whitespaces_any, pegtl::one< ')' >, whitespaces_any, pegtl::sor< block, statement_inner > > {};
        }
        namespace Operator
        {
            struct add : pegtl::one< '+' > {};
            struct subtract : pegtl::one< '-' > {};
            struct multiply : pegtl::one< '*' > {};
            struct divide : pegtl::one< '/' > {};
            struct increment : pegtl::two< '+' > {};
            struct decrement : pegtl::two< '-' > {};
            struct logical_and : pegtl::two< '&' > {};
            struct logical_or : pegtl::two< '|' > {};
            struct logical_not : pegtl::one< '!' > {};
            struct bitwise_and : pegtl::one< '&' > {};
            struct bitwise_or : pegtl::one< '|' > {};
            struct bitwise_xor : pegtl::one< '^' > {};
            struct bitwise_not : pegtl::one< '~' > {};
            struct bitwise_lshift : pegtl::two< '<' > {};
            struct bitwise_rshift : pegtl::two< '>' > {};
            struct conditional_equal : pegtl::two< '=' > {};
            struct conditional_not_equal : pegtl::sor< pegtl::string< '!', '=' >, pegtl::string< '<', '>' > > {};
            struct conditional_less : pegtl::one< '<' > {};
            struct conditional_greater : pegtl::one< '>' > {};
            struct conditional_less_or_equal : pegtl::string< '<', '=' > {};
            struct conditional_greater_or_equal : pegtl::string< '>', '=' > {};
            struct binary_operator : pegtl::sor< add, subtract, multiply, divide, logical_and, logical_or, bitwise_and, bitwise_or, bitwise_xor, bitwise_lshift, bitwise_rshift, conditional_equal, conditional_not_equal, conditional_less, conditional_greater, conditional_less_or_equal, conditional_greater_or_equal > {};
            struct binary_operation : pegtl::seq< pegtl::one< '{' >, whitespaces_any, value, whitespaces_any, binary_operator, whitespaces_any, value, whitespaces_any, pegtl::one< '}' > > {};
            struct unary_operator : pegtl::sor< increment, decrement, logical_not, bitwise_not > {};
            struct unary_operation : pegtl::seq< unary_operator, whitespaces_any, pegtl::one< '{' >, whitespaces_any, value, whitespaces_any, pegtl::one< '}' > > {};
        }
        struct expression_statement : pegtl::seq< value, whitespaces_any, semicolons > {};
        struct statement_inner : pegtl::sor< Directive::statement, variable_statement, object_destroy_statement, ControlFlow::while_statement, ControlFlow::for_statement, ControlFlow::foreach_statement, ControlFlow::condition_statement, ControlFlow::return_statement, ControlFlow::continue_statement, ControlFlow::break_statement, Class::Method::call_statement, expression_statement > {};
        struct statement_outter : pegtl::sor< Directive::statement, Class::definition_statement > {};
        struct statements_inner : pegtl::plus< whitespaces_any, statement_inner, whitespaces_any > {};
        struct statements_outter : pegtl::plus< whitespaces_any, statement_outter, whitespaces_any > {};
        struct grammar : pegtl::must< pegtl::opt< pegtl::shebang >, pegtl::opt< statements_outter >, pegtl::eof > {};

    }
}

#endif
