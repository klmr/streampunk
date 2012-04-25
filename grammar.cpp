#include <iostream>
#include <string>

#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lambda/lambda.hpp>

#include "grammar.hpp"

namespace qi = boost::spirit::qi;

namespace stream {

// TODO Change grammar of function calls to require parentheses, otherwise the
// following CANNOT be disambiguated:
//
//  function f = 23
//  function g = 42
//  let a = [f, g]
//  let x = a[1]
//
// Now, is x == g or did we intend to invoke the function, getting x == [1..5]?
// DOESN'T MATTER! Except (maybe) to deduce static type of expression?
//
// f :: -> num
// g :: -> num
// a :: [(-> num)]
// x :: (-> num) | num

template <typename Iterator>
struct stream_lang_impl : qi::grammar<Iterator, skipper> {
    typedef skipper space_type;
    // TODO Replace by template alias, once GCC supports it.
#define RULE(type) qi::rule<Iterator, space_type, type>
    typedef qi::rule<Iterator, space_type> rule_t;

    RULE(std::string()) id;
    RULE(std::string()) variable;
    RULE(double()) number;
    RULE(std::string()) string;
    rule_t source_unit;
    rule_t statement;
    rule_t module;
    rule_t import;
    rule_t function;
    rule_t params;
    rule_t let;
    rule_t eol;
    rule_t qualified;

    rule_t sequence_expression;
    rule_t expression;
    rule_t call_expression;
    rule_t conditional_expression;
    rule_t logical_or_expression;
    rule_t logical_and_expression;
    rule_t bit_or_expression;
    rule_t bit_xor_expression;
    rule_t bit_and_expression;
    rule_t equality_expression;
    rule_t relational_expression;
    rule_t shift_expression;
    rule_t addition_expression;
    rule_t multiplication_expression;
    rule_t power_expression;
    rule_t range_expression;
    rule_t unary_expression;
    rule_t subscript_expression;
    rule_t primary_expression;

#undef RULE

    stream_lang_impl() : stream_lang_impl::base_type(source_unit) {
        // EOL is redefined to be either end of line or end of input.
        // This is done to allow the input file to end without a proper line
        // ending (i.e. EOL), in violation of the UNIX definition of a line.
        eol = +qi::eol || qi::eoi;
        id = qi::lexeme[qi::char_("a-zA-Z") >> *qi::char_("a-zA-Z0-9")];
        number = qi::double_;
        string = qi::lexeme['"' >> *(~qi::lit('"') | "\\\"") >> '"'];

        // TODO Define variable to be NOT keyword
        variable = id;

        source_unit =
            -module >> *statement;

        module =
            "module" > qualified > eol;

        statement =
            (import | function | let | sequence_expression) > eol;

        import =
            "import" > qualified > -("as" > id);

        qualified =
            variable % '.';

        // TODO If functions have at least one argument, does it make sense to
        // differentiate between `function` & `let`?
        // NOTE Yes, if functions may have side-effects. How to handle this?
        function =
            "function" > variable > params > "=" > sequence_expression;

        params =
            +variable;

        let =
            "let" > variable > "=" > sequence_expression;

        // The following nesting hierarchy of the rules reflects the operator
        // precedence of the expression types. The sequence operator (pipe)
        // has the lowest precedence.

        sequence_expression =
            expression % ("|>" >> -qi::eol);

        expression =
            conditional_expression |
            call_expression |
            logical_or_expression;

        conditional_expression =
            "if" > expression >
            "then" > expression >
            "else" > expression;

        call_expression =
            qualified >> +expression;

        // We use the sequence (>>) instead of expect (>) in the following
        // because otherwise the lookahead may bail out in legitimate parses;
        // consider "a |> b" which the parser will first try to parse as a
        // bitwise-or (due to precedence) and then *expects* a bitwise-xor to
        // follow without considering the alternatives.

        logical_or_expression =
            logical_and_expression >> *("or" >> logical_and_expression);

        logical_and_expression =
            bit_or_expression >> *("and" >> bit_or_expression);

        bit_or_expression =
            bit_xor_expression >> *('|' >> bit_xor_expression);

        bit_xor_expression =
            bit_and_expression >> *('^' >> bit_and_expression);

        bit_and_expression =
            equality_expression >> *('&' >> equality_expression);

        equality_expression =
            relational_expression
            >> *(   ('=' >> relational_expression)
                |   ("!=" >> relational_expression)
                );

        relational_expression =
            shift_expression
            >> *(   ('<' >> shift_expression)
                |   ('>' >> shift_expression)
                |   ("<=" >> shift_expression)
                |   (">=" >> shift_expression)
                );

        shift_expression =
            addition_expression
            >> *(   ("<<" >> addition_expression)
                |   (">>" >> addition_expression)
                );

        addition_expression =
            multiplication_expression
            >> *(   ('+' >> multiplication_expression)
                |   ('-' >> multiplication_expression)
                );

        multiplication_expression =
            power_expression
            >> *(   ('*' >> power_expression)
                |   ('/' >> power_expression)
                |   ("//" >> power_expression)
                |   ('%' >> power_expression)
                );

        power_expression =
            unary_expression >> *("**" >> unary_expression);

        unary_expression =
            subscript_expression |
            ("not" >> unary_expression) |
            ('~' >> unary_expression) |
            ('(' >> ( ('+' >> unary_expression) |
                      ('-' >> unary_expression) |
                      sequence_expression ) > ')');

        subscript_expression =
            primary_expression >> *('[' > sequence_expression > ']');

        primary_expression =
            number |
            string |
            qualified;

        // Tell me this can't be automated?!
        id.name("id");
        variable.name("variable");
        number.name("number");
        string.name("string");
        source_unit.name("source_unit");
        statement.name("statement");
        module.name("module");
        import.name("import");
        function.name("function");
        params.name("params");
        let.name("let");
        eol.name("eol");
        qualified.name("qualified");
        sequence_expression.name("sequence_expression");
        expression.name("expression");
        call_expression.name("call_expression");
        conditional_expression.name("conditional_expression");
        logical_or_expression.name("logical_or_expression");
        logical_and_expression.name("logical_and_expression");
        bit_or_expression.name("bit_or_expression");
        bit_xor_expression.name("bit_xor_expression");
        bit_and_expression.name("bit_and_expression");
        equality_expression.name("equality_expression");
        relational_expression.name("relational_expression");
        shift_expression.name("shift_expression");
        addition_expression.name("addition_expression");
        multiplication_expression.name("multiplication_expression");
        power_expression.name("power_expression");
        range_expression.name("range_expression");
        unary_expression.name("unary_expression");
        subscript_expression.name("subscript_expression");
        primary_expression.name("primary_expression");

        BOOST_SPIRIT_DEBUG_NODE(id);
        BOOST_SPIRIT_DEBUG_NODE(variable);
        BOOST_SPIRIT_DEBUG_NODE(number);
        BOOST_SPIRIT_DEBUG_NODE(string);
        BOOST_SPIRIT_DEBUG_NODE(source_unit);
        BOOST_SPIRIT_DEBUG_NODE(statement);
        BOOST_SPIRIT_DEBUG_NODE(module);
        BOOST_SPIRIT_DEBUG_NODE(import);
        BOOST_SPIRIT_DEBUG_NODE(function);
        BOOST_SPIRIT_DEBUG_NODE(params);
        BOOST_SPIRIT_DEBUG_NODE(let);
        BOOST_SPIRIT_DEBUG_NODE(eol);
        BOOST_SPIRIT_DEBUG_NODE(qualified);
        BOOST_SPIRIT_DEBUG_NODE(sequence_expression);
        BOOST_SPIRIT_DEBUG_NODE(expression);
        BOOST_SPIRIT_DEBUG_NODE(call_expression);
        BOOST_SPIRIT_DEBUG_NODE(conditional_expression);
        BOOST_SPIRIT_DEBUG_NODE(logical_or_expression);
        BOOST_SPIRIT_DEBUG_NODE(logical_and_expression);
        BOOST_SPIRIT_DEBUG_NODE(bit_or_expression);
        BOOST_SPIRIT_DEBUG_NODE(bit_xor_expression);
        BOOST_SPIRIT_DEBUG_NODE(bit_and_expression);
        BOOST_SPIRIT_DEBUG_NODE(equality_expression);
        BOOST_SPIRIT_DEBUG_NODE(relational_expression);
        BOOST_SPIRIT_DEBUG_NODE(shift_expression);
        BOOST_SPIRIT_DEBUG_NODE(addition_expression);
        BOOST_SPIRIT_DEBUG_NODE(multiplication_expression);
        BOOST_SPIRIT_DEBUG_NODE(power_expression);
        BOOST_SPIRIT_DEBUG_NODE(range_expression);
        BOOST_SPIRIT_DEBUG_NODE(unary_expression);
        BOOST_SPIRIT_DEBUG_NODE(subscript_expression);
        BOOST_SPIRIT_DEBUG_NODE(primary_expression);
    }
};

template <typename Iterator>
stream_lang<Iterator>::stream_lang() :
    stream_lang<Iterator>::base_type(start),
    impl(new stream_lang_impl<Iterator>)
{
    start = impl->source_unit;
}

template <typename Iterator>
stream_lang<Iterator>::~stream_lang()
{ }

template struct stream_lang<char const*>;

template struct stream_lang<wchar_t const*>;

} // namespace stream
