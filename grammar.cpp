#include <iostream>
#include <string>

#include <boost/spirit/include/qi.hpp>

#include "grammar.hpp"

namespace stream {

template <typename Iterator>
struct stream_lang_impl : qi::grammar<Iterator, qi::unused_type(), qi::blank_type> {
    template <typename Attr = qi::unused_type, typename... Inherited>
    using rule_t = qi::rule<Iterator, Attr(Inherited...), qi::blank_type>;

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
        // a quick and dirty distinct keyword parser `kw`: Should be effective,
        // at least to avoid parsing partial identifiers as keywords
        static const qi::rule<Iterator, qi::unused_type(const char*)> kw = qi::lit(qi::_r1) >> !qi::alnum;

        // EOL is redefined to be either end of line or end of input.
        // This is done to allow the input file to end without a proper line
        // ending (i.e. EOL), in violation of the UNIX definition of a line.
        eol    = +qi::eol || qi::eoi;
        id     = qi::lexeme[qi::alpha >> *qi::alnum];
        number = qi::double_;
        string = qi::lexeme['"' >> *(~qi::lit('"') | "\\\"") > '"'];

        source_unit =
            -module >> *statement;

        module =
            kw(+"module") > qualified > eol;

        statement =
            (import | function | let | expression) > eol;

        import =
            kw(+"import") > qualified > -(kw(+"as") > id);

        qualified =
            id % '.';

        function =
            kw(+"function") > id > params > "=" > expression;

        params =
            *id;

        let =
            kw(+"let") > id > "=" > expression;

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
            logical_and_expression >> *(kw(+"or") >> logical_and_expression);

        logical_and_expression =
            bit_or_expression >> *(kw(+"and") >> bit_or_expression);

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
            (kw(+"not") >> unary_expression) |
            ('+'        >> unary_expression) |
            ('-'        >> unary_expression) |
            ('~'        >> unary_expression);

        subscript_expression =
            primary_expression >> *('[' > expression > ']');

        primary_expression =
            number |
            string |
            qualified |
            ('(' > expression > ')');

        BOOST_SPIRIT_DEBUG_NODES(
            (id) (number) (string)

            (source_unit) (statement) (module) (import) (function) (params) (let) (eol) (qualified)

            (expression)
            (call_expression)
            (logical_or_expression) (logical_and_expression)
            (bit_or_expression) (bit_xor_expression) (bit_and_expression)
            (equality_expression) (relational_expression)
            (shift_expression) (addition_expression) (multiplication_expression) (power_expression)
            (range_expression) (unary_expression) (subscript_expression) (primary_expression)
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
