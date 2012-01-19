#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "grammar.hpp"

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
    return qi::phrase_parse(begin, end, grammar, skipper) and begin == end;
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
