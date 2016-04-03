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
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

using llvm::Value;
using llvm::Function;
using namespace parser;
using namespace AST;


namespace code_generator {
      
   CodeGeneratorImpl::CodeGeneratorImpl() :
   //jit_(std::make_unique<jit::JIT>()),
   //module_(jit_->getModulePtr()),
   jit_(nullptr),
   module_(std::make_unique<llvm::Module>("hack bitcode jit", llvm::getGlobalContext())),
   builder_(llvm::getGlobalContext()),
   optimizer_(std::make_unique<optimizer::Optimizer>(module_.get()))
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
   
   Value* CodeGeneratorImpl::codeGenIfExpr(const IfExprAST* ifExpr)
   {
      if(!ifExpr)
         return nullptr;
      
      //resolve cond
      auto CondV = ifExpr->getCondion()->codeGen();
      if (!CondV)
         return nullptr;
      
      // convert to bool comparing false to 0.0 (only doubles are supported)
      CondV = builder_.CreateFCmpONE(CondV,
                                     llvm::ConstantFP::get(llvm::getGlobalContext(),
                                                           llvm::APFloat(0.0)), "ifcond");
      
      auto TheFunction = builder_.GetInsertBlock()->getParent();
      
      // Create blocks for the then and else cases.
      auto ThenBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", TheFunction);
      auto ElseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
      auto MergeBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifcont");
      builder_.CreateCondBr(CondV, ThenBB, ElseBB);
      
      // Emit then value.
      builder_.SetInsertPoint(ThenBB);
      
      //resolve 'then' branch
      auto ThenV = ifExpr->getThenBranch()->codeGen();
      if (!ThenV)
         return nullptr;
      
      builder_.CreateBr(MergeBB);
      // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
      ThenBB = builder_.GetInsertBlock();
      
      // Emit else block.
      TheFunction->getBasicBlockList().push_back(ElseBB);
      builder_.SetInsertPoint(ElseBB);
      
      auto ElseV = ifExpr->getElseBranch()->codeGen();
      if (!ElseV)
         return nullptr;
      
      builder_.CreateBr(MergeBB);
      // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
      ElseBB = builder_.GetInsertBlock();
      
      // Emit merge block.
      TheFunction->getBasicBlockList().push_back(MergeBB);
      builder_.SetInsertPoint(MergeBB);
      
      llvm::PHINode *PN = builder_.CreatePHI(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 2, "iftmp");
      PN->addIncoming(ThenV, ThenBB);
      PN->addIncoming(ElseV, ElseBB);
      return PN;
      
   }
   
   Value* CodeGeneratorImpl::codeGenForExpr(const ForExprAST* forExpr)
   {
      auto StartVal = forExpr->getStart()->codeGen();
      if (!StartVal)
         return nullptr;
      
      // Make the new basic block for the loop header, inserting after current
      // block.
      auto TheFunction = builder_.GetInsertBlock()->getParent();
      auto PreheaderBB = builder_.GetInsertBlock();
      auto LoopBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop", TheFunction);
      
      // Insert an explicit fall through from the current block to the LoopBB.
      builder_.CreateBr(LoopBB);
      
      // Start insertion in LoopBB.
      builder_.SetInsertPoint(LoopBB);
      
      // Start the PHI node with an entry for Start.
      auto Variable = builder_.CreatePHI(llvm::Type::getDoubleTy(llvm::getGlobalContext()),
                                            2, forExpr->getKey().c_str());
      
      Variable->addIncoming(StartVal, PreheaderBB);
      
      // Within the loop, the variable is defined equal to the PHI node.  If it
      // shadows an existing variable, we have to restore it, so save it now.
      const auto& varName = forExpr->getKey();
      auto OldVal = namedValues_[varName];
      namedValues_[varName] = Variable;
      
      // Emit the body of the loop.  This, like any other expr, can change the
      // current BB.  Note that we ignore the value computed by the body, but don't
      // allow an error.
      if (!forExpr->getBody()->codeGen())
         return nullptr;
      
      // Emit the step value.
      const auto& Step = forExpr->getStep();
      llvm::Value* StepVal = nullptr;
      if (Step)
      {
         StepVal = Step->codeGen();
         if (!StepVal)
            return nullptr;
      }
      else
      {
         // If not specified, use 1.0.
         StepVal = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(1.0));
      }
      
      auto NextVar = builder_.CreateFAdd(Variable, StepVal, "nextvar");
      
      // Compute the end condition.
      auto EndCond = forExpr->getEnd()->codeGen();
      if (!EndCond)
         return nullptr;
      
      // Convert condition to a bool by comparing equal to 0.0.
      EndCond = builder_.CreateFCmpONE(EndCond,
                                       llvm::ConstantFP::get(llvm::getGlobalContext(),
                                                             llvm::APFloat(0.0)), "loopcond");
      
      // Create the "after loop" block and insert it.
      auto LoopEndBB = builder_.GetInsertBlock();
      auto AfterBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "afterloop", TheFunction);
      
      // Insert the conditional branch into the end of LoopEndBB.
      builder_.CreateCondBr(EndCond, LoopBB, AfterBB);
      
      // Any new code will be inserted in AfterBB.
      builder_.SetInsertPoint(AfterBB);
      
      // Add a new entry to the PHI node for the backedge.
      Variable->addIncoming(NextVar, LoopEndBB);
      
      // Restore the unshadowed variable.
      if (OldVal)
         namedValues_[varName] = OldVal;
      else
         namedValues_.erase(varName);
      
      // for expr always returns 0.0.
      return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(llvm::getGlobalContext()));
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
         //eager optimization peephole 
         optimizer_->runLocalFunctionOptimization(f);
         return f;
      }
      
      //error reading the body
      f->eraseFromParent();
      return nullptr;
   }
   

   ///
   /// Jit compilation test
   ///
//   void CodeGeneratorImpl::evalFunctionExpr(Function* function)
//   {
//      //since I am supporting only doubles I can have only doubles as params and void or double as return value
//      //in this case I want to try a function double(*)()
//      if(jit_)
//         jit_->evalFunction(function);
//      
//   }


}