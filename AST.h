//
//  AST.hpp
//  Kaleidoscope-LLVM
//
//  this header containts all the abstract representation of the language (aka Abstract Sintax Tree or ParseTree)
//
//  Created by Nicola Cabiddu on 07/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef ParseTree_h
#define ParseTree_h

#include <string>
#include <vector>
#include <memory>

#include "CodeGenerator.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"


namespace AST {
 
   ///
   /// @brief: base class to for all expression nodes
   ///
   class ExprAST {
      
   protected:
      using CodeGen = std::unique_ptr<code_generator::CodeGenerator>;
      static CodeGen codeGen_;
   public:
      ExprAST();
      virtual ~ExprAST() = default;
      virtual llvm::Value* codeGen() const = 0;
   };
   
   ///
   /// @brief: class to represent numeric literal expressions (aka values)
   ///
   class NumberExprAST : public ExprAST
   {
   private:
      double val_;
   public:
      NumberExprAST(double val);
      double getVal() const;
      llvm::Value* codeGen() const override;
   };
   
   ///
   /// @brief: class to represent variable expressions
   ///
   class VariableExprAST : public ExprAST
   {
   private:
      std::string name_;
   public:
      VariableExprAST(const std::string& name);
      const std::string& getName() const;
      llvm::Value* codeGen() const override;
   };
   
   ///
   /// @brief: class to represent a binary operation (e.g 1.0+2.0)
   ///
   class BinaryExprAST : public ExprAST
   {
   private:
      using operand_t = std::unique_ptr<ExprAST>;
      
      unsigned char op_;
      operand_t lhs_, rhs_;
   public:
      BinaryExprAST(unsigned char op, operand_t lhs, operand_t rhs);
      unsigned char getOperation() const;
      const ExprAST* getLeftOperand() const;
      const ExprAST* getRightOperand() const;
      llvm::Value* codeGen() const override;
      
   };
   
   ///
   /// @brief: class to represent a function calls expression
   ///
   class CallExprAST : public ExprAST
   {
   private:
      using Args = std::vector<std::unique_ptr<ExprAST>>;
      std::string callee_;
      Args args_;
   public:
      CallExprAST(const std::string& callee, Args args);
      
      const Args& getArgumentList() const;
      const std::string getCallee() const;
      llvm::Value* codeGen() const override;

   };
   
   ///
   ///@brief: class to represent a prototype of a function
   ///
   class PrototypeAST : public ExprAST
   {
   private:
      using Args = std::vector<std::string>;
      std::string name_;
      Args args_;
   public:
      PrototypeAST(const std::string& name, Args args);
      const Args& getArgumentList() const;
      const std::string& getName() const;
      llvm::Function* codeGen() const;

   };
   
   ///
   ///@brief: representation function definition itself
   ///
   class FunctionAST : public ExprAST
   {
   private:
      using prototype_t = std::unique_ptr<PrototypeAST>;
      using body_t = std::unique_ptr<ExprAST>;
      prototype_t prototype_;
      body_t body_;
   public:
      FunctionAST(prototype_t prototype, body_t body);
      const prototype_t& getPrototype() const;
      const body_t& getBody() const;
      llvm::Function* codeGen() const;
   };
   
}

#endif /* ParseTree_h */
