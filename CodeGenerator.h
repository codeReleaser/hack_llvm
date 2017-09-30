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
#include "llvm/IR/LLVMContext.h"
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

namespace jit {
   class JIT;
}

using namespace AST;
using namespace parser;
using llvm::Value;
using llvm::Function;
using llvm::AllocaInst;

namespace code_generator
{
   using precedence_tree_t = std::map<unsigned char, int>;
   using prototype_cache_t = std::unordered_map<std::string, std::unique_ptr<PrototypeAST>>;
   
   ///
   /// @brief: custom exception thrown by code generator
   ///
   class CodeGeneratorOperatorNotFound : public std::exception
   {
      std::string message_;
   public:
      explicit CodeGeneratorOperatorNotFound(char const* const message) noexcept : message_(message)
      {}
      
      virtual const char* what() const noexcept
      {
         return message_.c_str();
      }
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
      
      //IR generation
      
      virtual ~CodeGenerator() = default;
      virtual Value* errorV(const std::string&) const = 0;
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
      
   public:
      
      //adding new symbols and/or process new prototypes
      
      virtual int getOperatorPrecedence(unsigned char token) const = 0;
      virtual const prototype_cache_t& getProtypeCache() const = 0;
      
      virtual void setOperatorPrecedence(unsigned char token, int value) = 0;
      virtual void addProtypeCache(const std::string& key, std::unique_ptr<PrototypeAST>& prototype) = 0;
      
      //virtual hack to get the module
      virtual void getModule(std::unique_ptr<llvm::Module>& module) = 0;
      //hack to initialize the module and pass manager
      virtual void InitializeModuleAndPassManager() = 0;
      
   };
   
   ///
   /// @brief: concrete implementation for the code generator
   ///
   
   class CodeGeneratorImpl : public CodeGenerator
   {
   public:
    
      explicit CodeGeneratorImpl(jit::JIT& jitCompiler);

      //concrete impleentation for generatring IR
      
      virtual Value* errorV(const std::string&) const override;
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

   public:
      
      //concrete implementation for adding new prototypes/symbols
      virtual int getOperatorPrecedence(unsigned char token) const override;
      virtual const prototype_cache_t& getProtypeCache() const override;
      virtual void setOperatorPrecedence(unsigned char token, int value) override;
      virtual void addProtypeCache(const std::string& key, std::unique_ptr<PrototypeAST>& prototype) override;

      //hack to retrieve the module
      virtual void getModule( std::unique_ptr<llvm::Module>& module) override { module = std::move(module_); }
      virtual void InitializeModuleAndPassManager() override;

      
   private:
      
      llvm::LLVMContext context_;
      llvm::IRBuilder<> builder_;
      std::unique_ptr<llvm::Module> module_;
      std::unique_ptr<optimizer::Optimizer> optimizer_;
      std::unordered_map<std::string, llvm::AllocaInst*> namedValues_;
      precedence_tree_t binaryOperationPrecedence_;
      prototype_cache_t prototypeCache_;
      
      jit::JIT& jitCompiler_;
      
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
