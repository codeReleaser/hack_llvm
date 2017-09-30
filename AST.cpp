//
//  AST.cpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 17/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "AST.h"
#include "CodeGenerator.h"
#include "llvm/Support/raw_ostream.h"

using llvm::Value;
using llvm::Function;
using llvm::raw_ostream;
using code_generator::CodeGenerator;

namespace AST {
   
   ///
   /// indent utility
   ///
   raw_ostream &indent(raw_ostream &O, int size) {
      return O << std::string(size, ' ');
   }
   
   
   ///
   /// Default ExprAST constructor (it creates a instance of the code generator)
   ///
   ExprAST::ExprAST(CodeGenerator& codeGenerator) :
      codeGenerator_(codeGenerator)
   {}
   
   int ExprAST::getLine() const
   {
      return 0; //location_.line;
   }
   
   int ExprAST::getCol() const
   {
      return 0;// location_.col;
   }
   
   raw_ostream &ExprAST::dump(raw_ostream &out, int index)
   {
     return out << ':' << getLine() << ':' << getCol() << '\n';
   }
   
   ///
   /// Numeric expression AST node
   ///
   
   NumberExprAST::NumberExprAST(CodeGenerator& codeGenerator, double val):
   ExprAST(codeGenerator),
   val_(val)
   {}
   
   double NumberExprAST::getVal() const
   {
      return val_;
   }
   
   raw_ostream& NumberExprAST::dump(raw_ostream &out, int ind)
   {
      return ExprAST::dump(out << val_, ind);
   }

   
   Value* NumberExprAST::codeGen() const
   {
      return codeGenerator_.codeGenNumberExpr(this);
   }
   
   ///
   /// Variable expression
   ///
   
   VariableExprAST::VariableExprAST(CodeGenerator& codeGenerator, const std::string& name) :
   ExprAST(codeGenerator),
   name_(name)
   {}
   
   const std::string& VariableExprAST::getName() const
   {
      return name_;
   }
   
   raw_ostream &VariableExprAST::dump(raw_ostream &out, int ind)
   {
      return ExprAST::dump(out << name_, ind);
   }

   Value* VariableExprAST::codeGen() const
   {
      return codeGenerator_.codeGenVariableExpr(this);
   }
   
   ///
   /// Unary expression
   ///
   
   UnaryExprAST::UnaryExprAST(CodeGenerator& codeGenerator, opcode_t opcode, operand_t operand) :
   ExprAST(codeGenerator),
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
   
   raw_ostream &UnaryExprAST::dump(raw_ostream &out, int ind)
   {
      ExprAST::dump(out << "unary" << opcode_, ind);
      operand_->dump(out, ind + 1);
      return out;
   }
   
   llvm::Value *UnaryExprAST::codeGen() const
   {
      return codeGenerator_.codeGenUnaryExpr(this);
   }
   
   
   ///
   /// Binary expression
   ///
   
   BinaryExprAST::BinaryExprAST(CodeGenerator& codeGenerator, opcode_t opcode, operand_t lhs, operand_t rhs) :
   ExprAST(codeGenerator),
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
   
   raw_ostream &BinaryExprAST::dump(raw_ostream &out, int ind)
   {
      ExprAST::dump(out << "binary" << opcode_, ind);
      lhs_->dump(indent(out, ind) << "LHS:", ind + 1);
      rhs_->dump(indent(out, ind) << "RHS:", ind + 1);
      return out;
   }

   llvm::Value* BinaryExprAST::codeGen() const
   {
      return codeGenerator_.codeGenBinaryExpr(this);
   }
   
   ///
   /// Call expression
   ///

   CallExprAST::CallExprAST(CodeGenerator& codeGenerator, const std::string& callee, Args args) :
   ExprAST(codeGenerator),
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
   
   raw_ostream &CallExprAST::dump(raw_ostream &out, int ind)
   {
      ExprAST::dump(out << "Call" << callee_, ind);
      for (const auto &arg : args_)
         arg->dump( indent(out, ind+1), ind+1);
      return out;
   }

   
   llvm::Value* CallExprAST::codeGen() const
   {
      return codeGenerator_.codeGenCallExpr(this);
   }
   
   ///
   /// Prototype AST
   ///

   PrototypeAST::PrototypeAST( CodeGenerator& codeGenerator,
                               std::string name,
                               PrototypeAST::Args args,
                               bool is_operator,
                               unsigned precedence) :
      ExprAST(codeGenerator),
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
      return codeGenerator_.codeGenPrototypeExpr(this);
   }
   
   ///
   /// FunctionAST
   ///
   
   FunctionAST::FunctionAST(CodeGenerator& codeGenerator,
                            FunctionAST::prototype_t prototype,
                            FunctionAST::body_t body) :
      ExprAST(codeGenerator),
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
   
   raw_ostream &FunctionAST::dump(raw_ostream &out, int ind)
   {
      indent(out, ind) << "FunctionAST\n";
      ++ind;
      indent(out, ind) << "Body:";
      return body_ ? body_->dump(out, ind) : out << "null\n";
   }
   
   llvm::Function* FunctionAST::codeGen() const
   {
      return codeGenerator_.codeGenFunctionExpr(this);
   }
   
//   void FunctionAST::eval(llvm::Function* f)
//   {
//      return codeGen_->evalFunctionExpr(f);
//   }
   
   ///
   /// IfExprAST
   ///
   
   IfExprAST::IfExprAST(CodeGenerator& codeGenerator,
                        condion_t c,
                        then_branch_t t,
                        else_branch_t e) :
   ExprAST(codeGenerator),
   cond_(std::move(c)),
   then_(std::move(t)),
   else_(std::move(e))
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
   
   raw_ostream &IfExprAST::dump(raw_ostream &out, int ind)
   {
      ExprAST::dump(out << "if", ind);
      cond_->dump(indent(out, ind) << "Cond:", ind + 1);
      then_->dump(indent(out, ind) << "Then:", ind + 1);
      else_->dump(indent(out, ind) << "Else:", ind + 1);
      return out;
   }
   
   llvm::Value* IfExprAST::codeGen() const
   {
      return codeGenerator_.codeGenIfExpr(this);
   }
   
   ///
   /// ForExprAST
   ///
   
   ForExprAST::ForExprAST(CodeGenerator& codeGenerator,
                       std::string keyLoop,
                       expression_t start,
                       expression_t end,
                       expression_t step,
                       expression_t body) :
   ExprAST(codeGenerator),
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
   
   raw_ostream &ForExprAST::dump(raw_ostream &out, int ind)
   {
      ExprAST::dump(out << "for", ind);
      start_->dump(indent(out, ind) << "Cond:", ind + 1);
      end_->dump(indent(out, ind) << "End:", ind + 1);
      step_->dump(indent(out, ind) << "Step:", ind + 1);
      body_->dump(indent(out, ind) << "Body:", ind + 1);
      return out;
   }

   llvm::Value* ForExprAST::codeGen() const
   {
      return codeGenerator_.codeGenForExpr(this);
   }
   
   ///
   /// VarExprAST
   ///
   
   VarExprAST::VarExprAST(CodeGenerator& codeGenerator,
                          variable_names_t varNames, expression_t body) :
   ExprAST(codeGenerator),
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
   
   raw_ostream &VarExprAST::dump(raw_ostream &out, int ind)
   {
      ExprAST::dump(out << "var", ind);
      for (const auto &NamedVar : varNames_)
         NamedVar.second->dump(indent(out, ind) << NamedVar.first << ':', ind+1);
      
      body_->dump(indent(out, ind) << "Body:", ind + 1);
      return out;
   }
   
   llvm::Value* VarExprAST::codeGen() const
   {
      return codeGenerator_.codeGeneVarExpr(this);
   }


}
