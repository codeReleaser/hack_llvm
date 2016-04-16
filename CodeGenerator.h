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
#include <map>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "Optimizer.h"

namespace llvm
{
   class Value;
   class Function;
   class AllocaInst; //to support mutable variable
}

namespace AST
{
   class NumberExprAST;
   class VariableExprAST;
   class UnaryExprAST;
   class BinaryExprAST;
   class CallExprAST;
   class PrototypeAST;
   class FunctionAST;
   class IfExprAST;
   class ForExprAST;
   class VarExprAST;
};

namespace parser
{
   class Parser;
}

using namespace AST;
using namespace parser;
using llvm::Value;
using llvm::Function;
using llvm::AllocaInst;

namespace code_generator
{
   
   ///
   /// @brief: The interface describe the hooks to evaluate throughout the JIT compiler
   ///         the constructs of the language.
   ///
   class EvalJIT
   {
   public:
      //only functions for now
      virtual ~EvalJIT() = default;
      virtual void evalFunctionExpr(Function* function) = 0;
   };
   
   ///
   /// @brief: main interface of the code generator object
   ///         that emits the IR for the AST node passed to it
   ///         This interface must be extended in order to implement
   ///         a visitor pattern
   ///
   class CodeGenerator
   {
   public:
      virtual ~CodeGenerator() = default;
      virtual Value* errorV(const std::string&) = 0;
      virtual Value* codeGenNumberExpr(const NumberExprAST*) = 0;
      virtual Value* codeGenVariableExpr(const VariableExprAST*) = 0;
      virtual Value* codeGenUnaryExpr(const UnaryExprAST*) = 0;
      virtual Value* codeGenBinaryExpr(const BinaryExprAST*) = 0;
      virtual Value* codeGenCallExpr(const CallExprAST*) = 0;
      virtual Value* codeGenIfExpr(const IfExprAST*) = 0;
      virtual Value* codeGenForExpr(const ForExprAST*) = 0;
      virtual Function* codeGenPrototypeExpr(const PrototypeAST*) = 0;
      virtual Function* codeGenFunctionExpr(const FunctionAST*) = 0;
      virtual Value* codeGeneVarExpr(const VarExprAST*) = 0;
      
   };
   
   ///
   /// @brief: concrete implementation for the code generator
   ///
   
   class CodeGeneratorImpl : public CodeGenerator
   {
      using precedence_tree_t = std::map<unsigned char, int>;
      using prototype_cache_t = std::unordered_map<std::string, std::unique_ptr<PrototypeAST>>;
      
   public:
    
      explicit CodeGeneratorImpl();

      virtual Value* errorV(const std::string&) override;
      virtual Value* codeGenNumberExpr(const NumberExprAST*) override;
      virtual Value* codeGenVariableExpr(const VariableExprAST*) override;
      virtual Value* codeGenUnaryExpr(const UnaryExprAST*) override;
      virtual Value* codeGenBinaryExpr(const BinaryExprAST*) override;
      virtual Value* codeGenCallExpr(const CallExprAST*) override;
      virtual Value* codeGenIfExpr(const IfExprAST*) override;
      virtual Value* codeGenForExpr(const ForExprAST*) override;
      virtual Function* codeGenPrototypeExpr(const PrototypeAST*) override;
      virtual Function* codeGenFunctionExpr(const FunctionAST*) override;
      virtual Value* codeGeneVarExpr(const VarExprAST*) override;

      
      //evaluate IR just generated
      //virtual void evalFunctionExpr(Function* function) override;
      
      ///
      /// @brief: bad hack to retrieve the map of operators that the language supports and
      ///         a cache of all prototypes already generated
      /// TODO -> redesign a bit the code
      ///
      static precedence_tree_t& getOperatorPrecedence();
      static prototype_cache_t& getProtypeCache();
      
   private:
      
      //std::unique_ptr<jit::JIT> jit_; //jit compiler
      std::unique_ptr<llvm::Module> module_;
      llvm::IRBuilder<> builder_;

      //std::unordered_map<std::string, Value*> namedValues_;
      std::unordered_map<std::string, llvm::AllocaInst*> namedValues_;
      std::unique_ptr<optimizer::Optimizer> optimizer_;
      
      //operators precedence map
      static precedence_tree_t binaryOperationPrecedence_;

      //prototypes whose the code generation has already run
      static prototype_cache_t prototypeCache_;
      
   private:
      
      //private interface
      
      ///
      /// @brief: create an alloca instruction at the entry of the block for the function passed
      ///         as argument. Used for mutable variables
      ///
      AllocaInst *CreateEntryBlockAlloca(Function *function, const std::string &variableName);

      
      ///
      /// @brief: retrieve a function either from the current module or run the code genetor for it
      ///
      Function* getFunction(const std::string& name) const;
      
      
      ///
      /// @brief: manage assignement 
      ///
      Value* manageAssignment(const BinaryExprAST*);
      
   };
   
}


#endif /* CodeGenerator_h */
