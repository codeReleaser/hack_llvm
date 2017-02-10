CC=clang++
CXX_FLAGS= -g -O3 -std=c++14 -w
CLANG_INCLUDE_CXXFLAGS_LINK = `llvm-config --cxxflags --ldflags --system-libs --libs core native mcjit native`
CLANG_INCLUDE_CXXFLAGS = `llvm-config --cxxflags`

all: main.cpp lexer.o parser.o ast.o codegen.o optimizer.o test_parser.o jit.o
	$(CC) $(CLANG_INCLUDE_CXXFLAGS_LINK) $(CXX_FLAGS) -o toy.out $^

#Components compiler
lexer.o: lexer.cpp lexer.h
	$(CC) -c -o $@ $< $(CXX_FLAGS)

parser.o: parser.cpp parser.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) $(CXX_FLAGS)

ast.o: AST.cpp AST.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) $(CXX_FLAGS)

codegen.o: CodeGenerator.cpp CodeGenerator.h 
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) $(CXX_FLAGS)

optimizer.o: Optimizer.cpp Optimizer.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) $(CXX_FLAGS)

#Test compiler components
test_parser.o: TestParser.cpp TestParser.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) $(CXX_FLAGS)

clean:
	rm *.o
	rm *.out


