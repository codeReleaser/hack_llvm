//
//  CodeGenerator.hpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 14/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef CodeGenerator_h
#define CodeGenerator_h

#include <string>
#include <unordered_map>
#include <memory>

#include "llvm/IR/Module.h"

namespace llvm {
   class Value;
   class Function;
}

namespace AST {
   class NumberExprAST;
   class VariableExprAST;
   class BinaryExprAST;
   class CallExprAST;
   class PrototypeAST;
   class FunctionAST;
};

namespace parser {
   class Parser;
}

using namespace AST;
using namespace parser;
using llvm::Value;
using llvm::Function;

namespace code_generator
{
   ///
   /// @brief: main interface of the code generator object
   ///         that emits the IR for the AST node passed to it
   ///         This interface must be extended in order to implement
   ///         a visitor pattern
   ///
   class CodeGenerator {

   public:
      virtual ~CodeGenerator() = default;
      virtual Value* errorV(const char*) = 0;
      virtual Value* codeGenNumberExpr(const NumberExprAST*) = 0;
      virtual Value* codeGenVariableExpr(const VariableExprAST*) = 0;
      virtual Value* codeGenBinaryExpr(const BinaryExprAST*) = 0;
      virtual Value* codeGenCallExpr(const CallExprAST*) = 0;
      virtual Function* codeGenPrototypeExpr(const PrototypeAST*) = 0;
      virtual Function* codeGenFunctionExpr(const FunctionAST*) = 0;
      
      
   };
   
   class CodeGeneratorImpl : public CodeGenerator {
      std::unique_ptr<llvm::Module> module_;
      std::unordered_map<std::string, Value*> namedValues_;
   
   public:
      CodeGeneratorImpl();
      virtual Value* errorV(const char*) override;
      virtual Value* codeGenNumberExpr(const NumberExprAST*) override;
      virtual Value* codeGenVariableExpr(const VariableExprAST*) override;
      virtual Value* codeGenBinaryExpr(const BinaryExprAST*) override;
      virtual Value* codeGenCallExpr(const CallExprAST*) override;
      virtual Function* codeGenPrototypeExpr(const PrototypeAST*) override;
      virtual Function* codeGenFunctionExpr(const FunctionAST*) override;
   };
   
}


#endif /* CodeGenerator_h */
