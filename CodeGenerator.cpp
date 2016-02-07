//
//  CodeGenerator.cpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 14/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "CodeGenerator.h"

#include <memory>
#include <map>
#include <string>
#include <iostream>

#include "Parser.h"
#include "AST.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

using llvm::Value;
using llvm::Function;
using namespace parser;
using namespace AST;

static llvm::IRBuilder<> builder_ { llvm::getGlobalContext() };


namespace code_generator {
      
   CodeGeneratorImpl::CodeGeneratorImpl() :
   module_(std::make_unique<llvm::Module>("hack bitcode jit", llvm::getGlobalContext()))
   {}
   
   Value* CodeGeneratorImpl::errorV(const char* errorMsg)
   {
      std::cerr << errorMsg << std::endl;
      return nullptr;
   }
   
   Value* CodeGeneratorImpl::codeGenNumberExpr(const NumberExprAST* numExpr)
   {
      return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(numExpr->getVal()));
   }
   
   Value* CodeGeneratorImpl::codeGenVariableExpr(const VariableExprAST* variableExpr)
   {
      auto v = namedValues_.find(variableExpr->getName());
      if( v == namedValues_.end() )
      {
         std::string err = "Unknown variable name : ";
         err +=  variableExpr->getName();
         return errorV( err.c_str() );
      }
      
      return v->second;
   }
   
   Value* CodeGeneratorImpl::codeGenBinaryExpr(const BinaryExprAST* binaryExpr)
   {
      auto lhs = binaryExpr->getLeftOperand();
      auto rhs = binaryExpr->getRightOperand();
      auto op = binaryExpr->getOperation();
      
      if( lhs == nullptr || rhs == nullptr )
         return nullptr;
      
      //evaluete operands
      auto leftValue  = lhs->codeGen();
      auto rightValue = rhs->codeGen();
      
      if(leftValue == nullptr || rightValue == nullptr)
         return nullptr;
      
      switch (op)
      {
         case '+':
            return builder_.CreateFAdd(leftValue, rightValue, "addtmp");
         case '-':
            return builder_.CreateFSub(leftValue, rightValue, "subtmp");
         case '*':
            return builder_.CreateFMul(leftValue, rightValue, "multmp");
         case '<':
            leftValue = builder_.CreateFCmpULT(leftValue, rightValue, "cmptmp");
            // Convert bool to double
            return builder_.CreateUIToFP(leftValue,
                                        llvm::Type::getDoubleTy(llvm::getGlobalContext()),
                                        "booltmp");
         default:
            return errorV("invalid binary operator");
      }
      return nullptr;
   }
   
   Value* CodeGeneratorImpl::codeGenCallExpr(const CallExprAST* callExpr)
   {
      Function* function = module_->getFunction(callExpr->getCallee());
      if( function == nullptr )
         errorV("Unknown function referenced");
      
      const auto& args = callExpr->getArgumentList();
      if( function->arg_size() != args.size())
         errorV("Incorrect number of parameters passed");
      
      std::vector<Value*> argsV; //list of arguments evalueted
      for( const auto& arg : args ) {
         argsV.push_back(arg->codeGen());
         if( args.back() == nullptr )
            return nullptr;
      }
      
      return builder_.CreateCall(function, argsV, "calltmp");
   }
   
   Function* CodeGeneratorImpl::codeGenPrototypeExpr(const PrototypeAST* protoExpr)
   {
      auto argList = protoExpr->getArgumentList();
      
      std::vector<llvm::Type*> args { argList.size(),
         llvm::Type::getDoubleTy(llvm::getGlobalContext())};
      
      llvm::FunctionType* functionType = llvm::FunctionType::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()),
                                                                 args,
                                                                 false);
      llvm::Function* f = llvm::Function::Create(functionType,
                                                 llvm::Function::ExternalLinkage,
                                                 protoExpr->getName(),
                                                 module_.get());
      unsigned i = 0;
      for(auto& arg: f->args())
         arg.setName(argList[i++]);
      
      return f;
   }
   
   
   Function* CodeGeneratorImpl::codeGenFunctionExpr(const FunctionAST* functExpr)
   {
      const auto& prototype = functExpr->getPrototype();
      const auto& body = functExpr->getBody();
      
      //search for function declared by previous 'extern'
      llvm::Function* f = module_->getFunction(prototype->getName());
      
      if( f == nullptr )
         f = prototype->codeGen();
      
      if( f == nullptr )
         return nullptr;
      
      llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", f);
      builder_.SetInsertPoint(bb);
      namedValues_.clear();
      for( auto& arg : f->args())
         namedValues_[arg.getName()] = &arg;
      
      auto returnValue = body->codeGen();
      
      if(returnValue != nullptr)
      {
         builder_.CreateRet(returnValue);
         llvm::verifyFunction(*f);
         return f;
      }
      
      //error reading the body
      f->eraseFromParent();
      return nullptr;
   }

}