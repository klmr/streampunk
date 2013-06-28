BOOST=/usr/local/include

LD=$(CXX)
CC=$(CXX)
CXXFLAGS+=-std=c++0x -isystem$(BOOST) \
		  -pedantic -Wall -Wextra -Werror \
		  -fvisibility=hidden -fvisibility-inlines-hidden \
		  -O0

%.o: %.cpp *.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: test.o grammar.o
	$(CXX) $(CXXFLAGS) -o $@ $^

syntax_check: syntax_check.o grammar.o

cleanall:
	@$(RM) *.o
	@$(RM) test syntax_check
