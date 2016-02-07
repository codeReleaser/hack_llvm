//
//  AST.cpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 17/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "AST.h"

using llvm::Value;
using llvm::Function;

namespace AST {
   
   ExprAST::CodeGen ExprAST::codeGen_ =
      std::make_unique<code_generator::CodeGeneratorImpl>();

   ///
   /// Default ExprAST constructor (it creates a instance of the code generator)
   ///
   ExprAST::ExprAST()
   {}
   
   ///
   /// Numeric expression AST node
   ///
   
   NumberExprAST::NumberExprAST(double val) : val_(val)
   {}
   
   double NumberExprAST::getVal() const
   {
      return val_;
   }
   
   Value* NumberExprAST::codeGen() const
   {
      return codeGen_->codeGenNumberExpr(this);
   }
   
   ///
   /// Variable expression
   ///
   
   VariableExprAST::VariableExprAST(const std::string& name) : name_(name)
   {}
   
   const std::string& VariableExprAST::getName() const
   {
      return name_;
   }
   
   Value* VariableExprAST::codeGen() const
   {
      return codeGen_->codeGenVariableExpr(this);
   }
   
   ///
   /// Binary expression
   ///
   
   BinaryExprAST::BinaryExprAST(unsigned char op, operand_t lhs, operand_t rhs) :
   op_(op),
   lhs_(std::move(lhs)),
   rhs_(std::move(rhs))
   {}
      
   unsigned char BinaryExprAST::getOperation() const
   {
      return op_;
   }
   
   const ExprAST* BinaryExprAST::getLeftOperand() const
   {
      return lhs_.get();
   }
   
   const ExprAST* BinaryExprAST::getRightOperand() const
   {
      return rhs_.get();
   }
   
   llvm::Value* BinaryExprAST::codeGen() const
   {
      return codeGen_->codeGenBinaryExpr(this);
   }
   
   ///
   /// Call expression
   ///

   CallExprAST::CallExprAST(const std::string& callee, Args args) :
   callee_(callee),
   args_(std::move(args))
   {}
      
   const CallExprAST::Args& CallExprAST::getArgumentList() const
   {
      return args_;
   }
   
   const std::string CallExprAST::getCallee() const
   {
      return callee_;
   }
   
   llvm::Value* CallExprAST::codeGen() const
   {
      return codeGen_->codeGenCallExpr(this);
   }
   
   ///
   /// Prototype AST
   ///

   PrototypeAST::PrototypeAST(const std::string& name, PrototypeAST::Args args) :
      name_(std::move(name)),
      args_(std::move(args))
   {}
      
   const PrototypeAST::Args& PrototypeAST::getArgumentList() const
   {
      return args_;
   }
   
   const std::string& PrototypeAST::getName() const
   {
      return name_;
   }
   
   llvm::Function* PrototypeAST::codeGen() const
   {
      return codeGen_->codeGenPrototypeExpr(this);
   }
   
   ///
   /// FunctionAST
   ///
   
   FunctionAST::FunctionAST(FunctionAST::prototype_t prototype, FunctionAST::body_t body) :
      prototype_(std::move(prototype)),
      body_(std::move(body))
   {}
      
   const FunctionAST::prototype_t& FunctionAST::getPrototype() const
   {
      return prototype_;
   } 
   
   const FunctionAST::body_t& FunctionAST::getBody() const
   {
      return body_;
   }
   
   llvm::Function* FunctionAST::codeGen() const
   {
      return codeGen_->codeGenFunctionExpr(this);
   }


}