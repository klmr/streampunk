#include <iostream>
#include <string>

#include <boost/spirit/include/qi.hpp>

#include "grammar.hpp"

namespace stream {

template <typename Iterator>
struct stream_lang_impl : qi::grammar<Iterator, qi::unused_type(), qi::blank_type> {
    //
    template <typename Attr=qi::unused_type, typename... Inherited>
    using rule_t = qi::rule<Iterator, Attr(Inherited...), qi::blank_type>;
    //

    rule_t<std::string> id;
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
    
    rule_t<>            expression;
    rule_t<>            call_expression;
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

    stream_lang_impl() : stream_lang_impl::base_type(source_unit) {
        // EOL is redefined to be either end of line or end of input.
        // This is done to allow the input file to end without a proper line
        // ending (i.e. EOL), in violation of the UNIX definition of a line.
        eol = +qi::eol || qi::eoi;
        id = qi::lexeme[qi::char_("a-zA-Z") >> *qi::char_("a-zA-Z0-9")];
        number = qi::double_;
        string = qi::lexeme['"' >> *(~qi::lit('"') | "\\\"") >> '"'];

        source_unit =
            -module >> *statement;

        module =
            "module" > qualified > eol;

        statement =
            (import | function | let | expression) > eol;

        import =
            "import" > qualified > -("as" > id);

        qualified =
            id % '.';

        function =
            "function" > id > params > "=" > expression;

        params =
            *id;

        let =
            "let" > id > "=" > expression;

        // The following nesting hierarchy of the rules reflects the operator
        // precedence of the expression types. The sequence operator (pipe)
        // has the lowest precedence.

        expression =
            call_expression % ("|>" >> -qi::eol);

        call_expression =
            qualified >> *logical_or_expression;

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
            ('+' >> unary_expression) |
            ('-' >> unary_expression) |
            ('~' >> unary_expression);

        subscript_expression =
            primary_expression >> *('[' > expression > ']');

        primary_expression =
            number |
            string |
            qualified |
            ('(' > expression > ')');

        id.name("id");
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
        expression.name("expression");
        call_expression.name("call_expression");
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
