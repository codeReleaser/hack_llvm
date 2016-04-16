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
   
   VariableExprAST::VariableExprAST(const std::string& name) :
   name_(name)
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
   /// Unary expression
   ///
   
   UnaryExprAST::UnaryExprAST(opcode_t opcode, operand_t operand) :
   opcode_(opcode),
   operand_(std::move(operand))
   {}
   
   UnaryExprAST::opcode_t UnaryExprAST::getOpcode() const
   {
      return opcode_;
   }
   const UnaryExprAST::operand_t& UnaryExprAST::getOperand() const
   {
      return operand_;
   }
   
   llvm::Value *UnaryExprAST::codeGen() const
   {
      return codeGen_->codeGenUnaryExpr(this);
   }
   
   
   ///
   /// Binary expression
   ///
   
   BinaryExprAST::BinaryExprAST(opcode_t opcode, operand_t lhs, operand_t rhs) :
   opcode_(opcode),
   lhs_(std::move(lhs)),
   rhs_(std::move(rhs))
   {}
      
   BinaryExprAST::opcode_t BinaryExprAST::getOpcode() const
   {
      return opcode_;
   }
   
   const BinaryExprAST::operand_t& BinaryExprAST::getLeftOperand() const
   {
      return lhs_;
   }
   
   const BinaryExprAST::operand_t& BinaryExprAST::getRightOperand() const
   {
      return rhs_;
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

   PrototypeAST::PrototypeAST( std::string name,
                               PrototypeAST::Args args,
                               bool is_operator,
                               unsigned precedence) :
      name_(std::move(name)),
      args_(std::move(args)),
      is_operator_(is_operator),
      precedence_(precedence)
   {}
      
   const PrototypeAST::Args& PrototypeAST::getArgumentList() const
   {
      return args_;
   }
   
   const std::string& PrototypeAST::getName() const
   {
      return name_;
   }
   
   bool PrototypeAST::isUnary() const
   {
      return is_operator_ && args_.size() == 1;
   }
   
   bool PrototypeAST::isBinary() const
   {
      return is_operator_ && args_.size() == 2;
   }
   
   unsigned PrototypeAST::getBinaryPrecedence() const
   {
      return precedence_;
   }
   
   char PrototypeAST::getOperatorName() const
   {
      assert(isUnary() || isBinary());
      return name_[name_.size()-1];
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
      return std::move(prototype_);
   } 
   
   const FunctionAST::body_t& FunctionAST::getBody() const
   {
      return body_;
   }
   
   llvm::Function* FunctionAST::codeGen() const
   {
      return codeGen_->codeGenFunctionExpr(this);
   }
   
//   void FunctionAST::eval(llvm::Function* f)
//   {
//      return codeGen_->evalFunctionExpr(f);
//   }
   
   ///
   /// IfExprAST
   ///
   
   IfExprAST::IfExprAST(condion_t c, then_branch_t t, else_branch_t e) :
   cond_(std::move(c)), then_(std::move(t)), else_(std::move(e))
   {}
                                                   
   const IfExprAST::condion_t& IfExprAST::getCondion() const
   {
      return cond_;
   }
   
   const IfExprAST::then_branch_t& IfExprAST::getThenBranch() const
   {
      return then_;
   }
   
   const IfExprAST::else_branch_t& IfExprAST::getElseBranch() const
   {
      return else_;
   }
   
   llvm::Value* IfExprAST::codeGen() const
   {
      return codeGen_->codeGenIfExpr(this);
   }
   
   ///
   /// ForExprAST
   ///
   
   ForExprAST::ForExprAST(std::string keyLoop,
                       expression_t start,
                       expression_t end,
                       expression_t step,
                       expression_t body) :
   key_(std::move(keyLoop)),
   start_(std::move(start)),
   end_(std::move(end)),
   step_(std::move(step)),
   body_(std::move(body))
   {}
   
   const std::string&  ForExprAST::getKey() const
   {
      return key_;
   }
   
   const ForExprAST::expression_t& ForExprAST::getStart() const
   {
      return start_;
   }
   
   const ForExprAST::expression_t& ForExprAST::getEnd() const
   {
      return end_;
   }
   
   const ForExprAST::expression_t& ForExprAST::getStep() const
   {
      return step_;
   }
   
   const ForExprAST::expression_t& ForExprAST::getBody() const
   {
      return body_;
   }

   llvm::Value* ForExprAST::codeGen() const
   {
      return codeGen_->codeGenForExpr(this);
   }
   
   ///
   /// VarExprAST
   ///
   
   VarExprAST::VarExprAST(variable_names_t varNames, expression_t body) :
   varNames_(std::move(varNames)),
   body_(std::move(body))
   {}
   
   const VarExprAST::variable_names_t& VarExprAST::getVarNames() const
   {
      return varNames_;
   }
   
   const VarExprAST::expression_t& VarExprAST::getBody() const
   {
      return body_;
   }
   
   llvm::Value* VarExprAST::codeGen() const
   {
      return codeGen_->codeGeneVarExpr(this);
   }


}