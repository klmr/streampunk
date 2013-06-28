#include <iostream>
#include <string>

#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>

#include "grammar.hpp"

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
struct stream_lang_impl : qi::grammar<Iterator, qi::unused_type(), qi::blank_type> {
    template <typename Attr = qi::unused_type, typename... Inherited>
    using rule_t = qi::rule<Iterator, Attr(Inherited...), qi::blank_type>;

    qi::rule<Iterator>  kw_module, kw_import, kw_as, kw_let, kw_function, kw_if,
                        kw_then, kw_else, kw_and, kw_or, kw_not;

    rule_t<std::string> id;
    rule_t<std::string> variable;
    rule_t<double>      number;
    rule_t<std::string> string;

    rule_t<>            source_unit;
    rule_t<>            statement;
    rule_t<>            module;
    rule_t<>            import;
    rule_t<>            function;
    rule_t<>            params;
    rule_t<>            let;
    rule_t<>            eol;
    rule_t<>            qualified;

    rule_t<>            sequence_expression;
    rule_t<>            expression;
    rule_t<>            call_expression;
    rule_t<>            conditional_expression;
    rule_t<>            logical_or_expression;
    rule_t<>            logical_and_expression;
    rule_t<>            bit_or_expression;
    rule_t<>            bit_xor_expression;
    rule_t<>            bit_and_expression;
    rule_t<>            equality_expression;
    rule_t<>            relational_expression;
    rule_t<>            shift_expression;
    rule_t<>            addition_expression;
    rule_t<>            multiplication_expression;
    rule_t<>            power_expression;
    rule_t<>            range_expression;
    rule_t<>            unary_expression;
    rule_t<>            subscript_expression;
    rule_t<>            primary_expression;

    // Accepts arrays instead of pointers to ensure that only string literals
    // are passed -- otherwise we would potentially save a stale pointer.
    template <std::size_t N>
    static auto kw(char const (&keyword)[N]) -> qi::rule<Iterator> {
        // qi::lit has problems with char arrays, use pointer instead.
        return qi::lit(+keyword) >> !qi::alnum;
    }

    stream_lang_impl() : stream_lang_impl::base_type{source_unit} {
        kw_module = kw("module");
        kw_import = kw("import");
        kw_as = kw("as");
        kw_let = kw("let");
        kw_function = kw("function");
        kw_if = kw("if");
        kw_then = kw("then");
        kw_else = kw("else");
        kw_and = kw("and");
        kw_or = kw("or");
        kw_not = kw("not");

        // EOL is redefined to be either end of line or end of input.
        // This is done to allow the input file to end without a proper line
        // ending (i.e. EOL), in violation of the UNIX definition of a line.
        eol    = +qi::eol || qi::eoi;
        id     = qi::lexeme[qi::alpha >> *qi::alnum];
        number = qi::double_;
        string = qi::lexeme['"' >> *(~qi::lit('"') | "\\\"") > '"'];

        // TODO Define variable to be NOT keyword
        variable = id;

        source_unit =
            -module >> *statement;

        module =
            kw_module > qualified > eol;

        statement =
            (import | function | let | sequence_expression) > eol;

        import =
            kw_import > qualified > -(kw_as > id);

        qualified =
            variable % '.';

        // TODO If functions have at least one argument, does it make sense to
        // differentiate between `function` & `let`?
        // NOTE Yes, if functions may have side-effects. How to handle this?
        function =
            kw_function > variable > params > "=" > sequence_expression;

        params =
            +variable;

        let =
            kw_let > variable > "=" > sequence_expression;

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
            kw_if   > expression >
            kw_then > expression >
            kw_else > expression;

        call_expression =
            qualified >> +expression;

        // We use the sequence (>>) instead of expect (>) in the following
        // because otherwise the lookahead may bail out in legitimate parses;
        // consider "a |> b" which the parser will first try to parse as a
        // bitwise-or (due to precedence) and then *expects* a bitwise-xor to
        // follow without considering the alternatives.

        logical_or_expression =
            logical_and_expression >> *(kw_or >> logical_and_expression);

        logical_and_expression =
            bit_or_expression >> *(kw_and >> bit_or_expression);

        bit_or_expression =
            bit_xor_expression >> *('|' >> bit_xor_expression);

        bit_xor_expression =
            bit_and_expression >> *('^' >> bit_and_expression);

        bit_and_expression =
            equality_expression >> *('&' >> equality_expression);

        equality_expression =
            relational_expression
            >> *(   ('='  >> relational_expression)
                |   ("!=" >> relational_expression)
                );

        relational_expression =
            shift_expression
            >> *(   ('<'  >> shift_expression)
                |   ('>'  >> shift_expression)
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
            >> *(   ('*'  >> power_expression)
                |   ('/'  >> power_expression)
                |   ("//" >> power_expression)
                |   ('%'  >> power_expression)
                );

        power_expression =
            unary_expression >> *("**" >> unary_expression);

        unary_expression =
            subscript_expression |
            (kw_not >> unary_expression) |
            ('~'    >> unary_expression) |
            ('('    >> ( ('+' >> unary_expression) |
                         ('-' >> unary_expression) |
                         sequence_expression ) > ')');

        subscript_expression =
            primary_expression >> *('[' > sequence_expression > ']');

        primary_expression =
            number |
            string |
            qualified;

        BOOST_SPIRIT_DEBUG_NODES(
            (id)
            (variable)
            (number)
            (string)

            (kw_module) (kw_import) (kw_as) (kw_let) (kw_function) (kw_if)
            (kw_then) (kw_else) (kw_and) (kw_or) (kw_not)

            (source_unit)
            (statement)
            (module)
            (import)
            (function)
            (params)
            (let)
            (eol)
            (qualified)

            (sequence_expression)
            (expression)
            (call_expression)
            (conditional_expression)
            (logical_or_expression)
            (logical_and_expression)
            (bit_or_expression)
            (bit_xor_expression) (bit_and_expression)
            (equality_expression)
            (relational_expression)
            (shift_expression)
            (addition_expression)
            (multiplication_expression)
            (power_expression)
            (range_expression)
            (unary_expression)
            (subscript_expression)
            (primary_expression)
        )
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

template struct stream_lang<std::string::const_iterator>;

template struct stream_lang<std::wstring::const_iterator>;

} // namespace stream
