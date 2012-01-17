BOOST=/usr/local/include

CXXFLAGS=-std=c++0x -pedantic -isystem$(BOOST) -pedantic -Wall -Wextra -Werror -O3

test: test.o grammar.o
	$(CXX) $(CXXFLAGS) -o $@ $^
