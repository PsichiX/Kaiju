#ifndef __GRAMMAR_H__
#define __GRAMMAR_H__

#include "std_extension.h"
#include <pegtl.hh>

namespace Kaiju
{
    namespace Grammar
    {

        struct statements;
        struct expression;
        namespace Class
        {
            namespace Method
            {
                struct call;
            }
        }
        namespace Comment
        {
            struct inlined : pegtl::seq< pegtl::two< '/' >, pegtl::until< pegtl::eolf > > {};
            struct block : pegtl::seq< pegtl::string< '/', '*' >, pegtl::until< pegtl::string< '*', '/' > > > {};
        }
        struct comment : pegtl::sor< Comment::inlined, Comment::block > {};
        struct whitespaces : pegtl::plus< pegtl::sor< pegtl::space, comment > > {};
        struct whitespaces_any : pegtl::star< pegtl::sor< pegtl::space, comment > > {};
        struct semicolons : pegtl::seq< pegtl::one< ';' >, pegtl::star< whitespaces_any, pegtl::one< ';' > > > {};
        struct identifier : pegtl::identifier {};
        struct new_keyword : pegtl::string< 'n', 'e', 'w' > {};
        struct delete_keyword : pegtl::string< 'd', 'e', 'l', 'e', 't', 'e' > {};
        struct object_create : pegtl::seq< new_keyword, whitespaces, Class::Method::call > {};
        struct object_destroy_statement : pegtl::seq< delete_keyword, whitespaces, expression, whitespaces_any, semicolons > {};
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
        struct access_field : pegtl::seq< whitespaces_any, pegtl::one< '.' >, whitespaces_any, identifier > {};
        struct field : pegtl::seq< identifier, pegtl::star< access_field > > {};
        struct value : pegtl::sor< object_create, Class::Method::call, field, number, string > {};
        namespace Variable
        {
            struct prefix : pegtl::one< '$' > {};
            struct prefix_static : pegtl::string< '~', '$' > {};
            struct assignment;
            struct declaration : pegtl::seq< pegtl::sor< prefix, prefix_static >, whitespaces_any, identifier > {};
            struct assignment_expression : pegtl::sor< assignment, expression > {};
            struct declaration_assignment : pegtl::seq< declaration, whitespaces_any, pegtl::one< '=' >, whitespaces_any, assignment_expression > {};
            struct assignment : pegtl::seq< field, whitespaces_any, pegtl::one< '=' >, whitespaces_any, assignment_expression > {};
        }
        struct variable : pegtl::sor< Variable::declaration_assignment, Variable::declaration, Variable::assignment > {};
        struct variable_statement : pegtl::seq< variable, whitespaces_any, semicolons > {};
        struct block : pegtl::seq< pegtl::one< '{' >, whitespaces_any, pegtl::opt< statements >, whitespaces_any, pegtl::one< '}' > > {};
        namespace Class
        {
            namespace Method
            {
                struct prefix : pegtl::one< '@' > {};
                struct prefix_static : pegtl::string< '~', '@' > {};
                namespace Definition
                {
                    struct argument_list : pegtl::opt< identifier, pegtl::star< whitespaces_any, pegtl::one< ',' >, whitespaces_any, identifier > > {};
                    struct arguments : pegtl::seq< pegtl::one< '(' >, whitespaces_any, argument_list, whitespaces_any, pegtl::one< ')' > > {};
                }
                struct definition_statement : pegtl::seq< pegtl::sor< prefix, prefix_static>, whitespaces_any, identifier, whitespaces_any, Definition::arguments, whitespaces_any, block > {};
                namespace Call
                {
                    struct argument_list : pegtl::opt< expression, pegtl::star< whitespaces_any, pegtl::one< ',' >, whitespaces_any, expression > > {};
                    struct arguments : pegtl::seq< pegtl::one< '(' >, whitespaces_any, argument_list, whitespaces_any, pegtl::one< ')' > > {};
                }
                struct call : pegtl::seq< field, whitespaces_any, Call::arguments > {};
                struct call_statement : pegtl::seq< call, whitespaces_any, semicolons > {};
            }
            struct prefix : pegtl::one< '#' > {};
            struct inheritance : pegtl::seq< whitespaces_any, pegtl::one< ':' >, whitespaces_any, field, whitespaces_any > {};
            struct body : pegtl::seq< pegtl::one< '{' >, whitespaces_any, pegtl::star< pegtl::sor< variable_statement, Method::definition_statement >, whitespaces_any >, pegtl::one< '}' > > {};
            struct definition_statement : pegtl::seq< prefix, whitespaces, identifier, whitespaces_any, inheritance, whitespaces_any, body > {};
        }
        struct expression_bracket : pegtl::seq< pegtl::one< '(' >, whitespaces_any, expression, whitespaces_any, pegtl::one< ')' > > {};
        struct expression_atomic : pegtl::sor< expression_bracket, value > {};
        struct expression : pegtl::seq< expression_atomic > {};
        struct expression_statement : pegtl::seq< expression, whitespaces_any, semicolons > {};
        struct statement : pegtl::sor< comment, block, variable_statement, Class::Method::call_statement, Class::definition_statement, expression_statement > {};
        struct statements : pegtl::plus< pegtl::seq< whitespaces_any, statement, whitespaces_any > > {};
        struct grammar : pegtl::must< pegtl::opt< statements >, pegtl::eof > {};

    }
}

#endif
