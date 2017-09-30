CC=clang++
OPT_FLAGS= -g -O3 -w -Wall 
STDCPP14 = -std=c++14
CLANG_INCLUDE_CXXFLAGS = $(OPT_FLAGS) `llvm-config --cxxflags` $(STDCPP14)

CXX_FLAGS = `llvm-config --cxxflags --ldflags`
LD_FLAGS = `llvm-config --system-libs --libs core orcjit native`


all: main.cpp lexer.o parser.o ast.o codegen.o optimizer.o driver.o jit.o debug.o configurator.o
	$(CC) $(CXX_FLAGS) $(OPT_FLAGS) $(STDCPP14) $^ -o toy.out $(LD_FLAGS) 

#Components compiler
lexer.o: lexer.cpp lexer.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) 

parser.o: parser.cpp parser.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS)

ast.o: AST.cpp AST.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) 

codegen.o: CodeGenerator.cpp CodeGenerator.h 
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS)

optimizer.o: Optimizer.cpp Optimizer.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) 

driver.o: driver.cpp driver.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) 

jit.o: JIT.cpp JIT.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS)

debug.o: Debug.cpp Debug.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) 

configurator.o: CompilerConfigurator.cpp CompilerConfigurator.h
	$(CC) -c -o $@ $< $(CLANG_INCLUDE_CXXFLAGS) 

clean:
	rm *.o
	rm *.out


