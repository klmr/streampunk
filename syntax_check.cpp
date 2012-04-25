#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "grammar.hpp"

struct printer {
    typedef boost::spirit::utf8_string string;

    void element(string const& tag, string const& value, int depth) const {
        for (int i = 0; i < (depth * 4); ++i) // indent to depth
            std::cout << ' ';

        std::cout << "tag: " << tag;
        if (value != "")
            std::cout << ", value: " << value;
        std::cout << std::endl;
    }
};

void print_info(boost::spirit::info const& what) {
    using boost::spirit::basic_info_walker;

    printer pr;
    basic_info_walker<printer> walker(pr, what.tag, 0);
    boost::apply_visitor(walker, what.value);
}

std::string read_source_code(std::istream& stream) {
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

template <typename Grammar>
bool check_syntax(std::string const& code) {
    char const* begin = &code[0];
    char const* const end = begin + code.length();
    Grammar grammar;
    typename Grammar::skipper_type skipper;

    namespace qi = boost::spirit::qi;
    try {
        return qi::phrase_parse(begin, end, grammar, skipper) and begin == end;
    }
    catch (qi::expectation_failure<char const*> const& error) {
        print_info(error.what_);
        std::cout << "Got: \"" << std::string(error.first, error.last) << "\"\n";
        return false;
    }
    catch (...) {
        std::cout << "Unexpected failure.\n";
        return false;
    }
}

bool check_syntax(std::string const& code) {
    return check_syntax<stream::stream_lang<char const*>>(code);
}

int main(int argc, char const* argv[]) {
    // Use filename if given, otherwise stdin
    bool use_stdin = argc == 1;

    std::string code;
    if (use_stdin)
        code = read_source_code(std::cin);
    else {
        std::ifstream in(argv[1]);
        code = read_source_code(in);
    }

    bool success = check_syntax(code);

    std::cout << "Code is " << (success ? "valid" : "invalid") << ".\n";
    return success ? 0 : 1;
}
