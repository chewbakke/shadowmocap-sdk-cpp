CPPFLAGS := \
-std=c++20 \
-I../include \
-I/Users/luke/xcompile/osx/local/include

LDFLAGS :=

test: test_shadowmocap
	./$<

test_shadowmocap: obj/test.o
	clang++ $(LDFLAGS) -o $@ $<

obj/test.o: ../test/test.cpp
	clang++ -c $(CPPFLAGS) $< -o $@