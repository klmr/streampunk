#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#include "grammar.hpp"

using namespace stream;

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

int main() {
    using std::cout;

    std::string const message =
        "module test\n"
        "\n"
        "import io\n"
        "import data.xml as xml\n"
        "\n"
        "function id x = x\n"
        "function fst a b = a\n"
        "function add a b = a + b\n"
        "\n"
        "let answer = id 42\n"
        "\n"
        "fst (id answer) \"not the answer\" |> add 23 |>  \n"
        "    io.print\n"
        "\n"
        "io.print \"Hello world\"\n"
        ;

    auto first = message.begin();
    auto last = message.end();

    cout << "Attempting to parse <<<HEREDOC\n" << message << "HEREDOC;\n";

    try {
        bool r = stream::parse(first, last);
        bool success = r and (first == last);

        cout << std::boolalpha << "r: " << r << ", success: " << success << "\n";
        cout << "Remaining: \"" << std::string(first,last) << "\"\n";
    }
    catch (qi::expectation_failure<char const*> const& error) {
        print_info(error.what_);
        cout << "Got: \"" << std::string(error.first, error.last) << "\"\n";
    }
}
