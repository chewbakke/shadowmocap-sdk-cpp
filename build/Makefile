#
# @file    build/Makefile
# @author  Luke Tokheim, luke@motionshadow.com
# @brief   Makefile used to build and run unit tests and any example
#          applications bundled in this repo.
#

CPPFLAGS := -O3 -Wall -Wextra -Werror -std=c++20
INCLUDE := -I../include -I/Users/luke/xcompile/osx/local/include

# Enables Clang Source-based Code Coverage
# https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
ifdef COVERAGE
CPPFLAGS += -fprofile-instr-generate -fcoverage-mapping
LDFLAGS := -fprofile-instr-generate
endif

NAME := shadowmocap

# Unit test executable
TEST_TARGET := test_$(NAME)
TEST_SRC := ../test/test.cpp
TEST_OBJ := $(patsubst %.cpp,%.o,$(TEST_SRC))
# Run the mock server and unit test executable
TEST_EXEC = python3 ../test/mock_sdk_server.py ./$(TEST_TARGET)

# Detect macOS since we need to use xcrun for the llvm-* tools.
ifeq ($(shell uname),Darwin)
XCRUN := xcrun
endif

all: $(TEST_TARGET)

test: $(TEST_TARGET)
ifdef COVERAGE
	LLVM_PROFILE_FILE=coverage.profraw $(TEST_EXEC)
else
	$(TEST_EXEC)
endif

$(TEST_TARGET): $(TEST_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(INCLUDE) -o $@ $<

# Non-standard target, any txt file. This will run the unit tests, generate
# the profiling data, index the raw profile data, and then export a line by line
# report in text format.
ifdef COVERAGE
%.profraw: $(TEST_TARGET)
	LLVM_PROFILE_FILE=$@ $(TEST_EXEC)

%.profdata: %.profraw
	$(XCRUN) llvm-profdata merge -output=$@ $<

%.txt: %.profdata
	$(XCRUN) llvm-cov show $(TEST_TARGET) -instr-profile=$< >$@

coverage: coverage.txt
endif

clean:
	rm -f $(TEST_OBJ) $(TEST_TARGET)

.phony: builddir
