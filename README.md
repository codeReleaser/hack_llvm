# hack_llvm

Hacking LLVM toolchain in order to understand how it works.
This code follows the Kaleidoscope tutorial at this link.

http://llvm.org/docs/tutorial/index.html

The main aim of this code is to design the code in such way it can be easily reused, separating as much as possible the concepts that are the base of a compiler. The final idea is being able to compile small features of some main stream programming language changing just the front end.

Classes:

1. ***lexer*** 
token of the language
2.  ***parser***
parse the string that represents the program, generating the AST
3. ***ast***
AST representation, set of nodes that represent the entities parsed and that can be used to generate the code
4. ***code generator***
it accepts the AST nodes and generates the bit code for each node using LLVM code generation APIs