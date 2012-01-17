#ifndef STREAM_GRAMMAR_HPP
#define STREAM_GRAMMAR_HPP

#include <memory>
#include <boost/spirit/include/qi.hpp>

namespace stream {

// We want a skip parser that skips whitespace but not linebreaks.

struct skipper : boost::spirit::qi::primitive_parser<skipper> {
    template <typename Context, typename Iterator>
    struct attribute {
        typedef boost::spirit::qi::unused_type type;
    };

    template <typename Iterator, typename Context, typename OtherSkipper, typename Attribute>
    bool parse(Iterator& first, Iterator const& last, Context&, OtherSkipper const& skipper, Attribute&) const {
        boost::spirit::qi::skip_over(first, last, skipper);

        if (first != last and (*first == ' ' or *first == '\t')) {
            ++first;
            return true;
        }
        return false;
    }

    template <typename Context>
    boost::spirit::qi::info what(Context&) const {
        return boost::spirit::qi::info("whitespace");
    }
};

template <typename Iterator>
struct stream_lang_impl;

template <typename Iterator>
struct stream_lang : boost::spirit::qi::grammar<Iterator, skipper> {
    typedef skipper space_type;
    typedef boost::spirit::qi::rule<Iterator, space_type> rule_type;

    std::unique_ptr<stream_lang_impl<Iterator>> impl;
    rule_type start;

    stream_lang();

    ~stream_lang();

private:

    stream_lang(stream_lang const&);
    stream_lang& operator =(stream_lang const&);
};

} // namespace stream

#endif // ndef STREAM_GRAMMAR_HPP
