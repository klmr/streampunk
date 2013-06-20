#ifndef STREAM_GRAMMAR_HPP
#define STREAM_GRAMMAR_HPP

#include <memory>
#include <boost/spirit/include/qi.hpp>

namespace stream {

    namespace qi = boost::spirit::qi;

    template <typename Iterator>
    struct stream_lang_impl;

    template <typename Iterator>
    struct stream_lang : qi::grammar<Iterator, qi::unused_type(), qi::blank_type> {

        std::unique_ptr<stream_lang_impl<Iterator>> impl;
        qi::rule<Iterator, qi::unused_type(), qi::blank_type> start;

        stream_lang();
        ~stream_lang();
    private:
        stream_lang(stream_lang const&) = delete;
        stream_lang& operator =(stream_lang const&) = delete;
    };

    template <typename It>
        inline bool parse(It& first, It& last)
    {
        static const stream_lang<It> parser;
        return qi::phrase_parse(first, last, parser, qi::blank);
    }
    
} // namespace stream

#endif // ndef STREAM_GRAMMAR_HPP
