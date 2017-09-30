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
#include "JIT.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h"


using llvm::Value;
using llvm::Function;
using namespace parser;
using namespace AST;

//llvm::LLVMContext;

namespace code_generator
{
   
   ///
   /// return a reference to the internal map that holds all the operators
   ///
   int CodeGeneratorImpl::getOperatorPrecedence(unsigned char token) const
   {
      auto it = binaryOperationPrecedence_.find(token);
      return it != binaryOperationPrecedence_.end() ? it->second : -1;
   }
   
   ///
   /// @brief: return reference to cache of all prototype functions parsed
   ///
   const prototype_cache_t& CodeGeneratorImpl::getProtypeCache() const
   {
      return prototypeCache_;
   }
   
   ///
   /// @brief: set operator precendece
   ///
   void CodeGeneratorImpl::setOperatorPrecedence(unsigned char token, int value)
   {
      binaryOperationPrecedence_.insert(std::make_pair(token, value));
   }
   
   
   void CodeGeneratorImpl::addProtypeCache(const std::string& key, std::unique_ptr<PrototypeAST>& prototype)
   {
      //mmm... hugly
      prototypeCache_[key] = std::move(prototype);
   }
   
   void CodeGeneratorImpl::InitializeModuleAndPassManager()
   {
      module_ = std::make_unique<llvm::Module>("hacking", context_);
      optimizer_->enablePrematureOptimization(module_.get());

      module_->setDataLayout(jitCompiler_.getTargetMachine().createDataLayout());
   }

   ///
   /// ctor of the code generator
   ///
   
   //tmp hack to pass the jit compiler into the code generator2
   CodeGeneratorImpl::CodeGeneratorImpl(jit::JIT& jitCompiler) : CodeGenerator(),
      jitCompiler_(jitCompiler),
      module_(nullptr),
      builder_(context_),
      optimizer_(std::make_unique<optimizer::Optimizer>())
   {
      //InitializeModuleAndPassManager();
   }
   
   Value* CodeGeneratorImpl::errorV(const std::string& errorMsg) const
   {
      std::cerr << errorMsg << std::endl;
      return nullptr;
   }
   
   Value* CodeGeneratorImpl::codeGenNumberExpr(const NumberExprAST* numExpr)
   {
      return llvm::ConstantFP::get(context_, llvm::APFloat(numExpr->getVal()));
   }
   
   Value* CodeGeneratorImpl::codeGenVariableExpr(const VariableExprAST* variableExpr)
   {
      auto v = namedValues_.find(variableExpr->getName());
      if( v == namedValues_.end() )
      {
         return errorV( std::string("Unknown variable name : ") + variableExpr->getName());
      }
      
      return builder_.CreateLoad(v->second, variableExpr->getName());
      return v->second;
   }
   
   Value* CodeGeneratorImpl::codeGenUnaryExpr(const UnaryExprAST* unaryExpr)
   {
      auto operandValue = unaryExpr->getOperand()->codeGen();
      if (!operandValue)
         return nullptr;
      
      auto functionValue = getFunction(std::string("unary") +  unaryExpr->getOpcode());
      if (!functionValue)
         return errorV("Unknown unary operator");
      
      return builder_.CreateCall(functionValue, operandValue, "unop");
   }

   Value* CodeGeneratorImpl::codeGenBinaryExpr(const BinaryExprAST* binaryExpr)
   { auto op = binaryExpr->getOpcode();

      if( op == '=')
      {
         return manageAssignment(binaryExpr);
      }
      else
      {
         const auto& lhs = binaryExpr->getLeftOperand();
         const auto& rhs = binaryExpr->getRightOperand();
         
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
                                            llvm::Type::getDoubleTy(context_),
                                            "booltmp");
            default:
               break;
         }
         
         auto function = getFunction(std::string("binary") + (char)op);
         assert(function != nullptr && "binary function not found");
         
         Value* ops[] = {leftValue, rightValue};
         return builder_.CreateCall(function, ops, "binop");
      }
      
     
   }
   
   Value* CodeGeneratorImpl::codeGenCallExpr(const CallExprAST* callExpr)
   {
      Function* function = getFunction(callExpr->getCallee());
      if( function == nullptr ) {
         errorV("Unknown function referenced");
         return nullptr;
      }
      
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
                                     llvm::ConstantFP::get(context_,
                                                           llvm::APFloat(0.0)), "ifcond");
      
      auto TheFunction = builder_.GetInsertBlock()->getParent();
      
      // Create blocks for the then and else cases.
      auto ThenBB = llvm::BasicBlock::Create(context_, "then", TheFunction);
      auto ElseBB = llvm::BasicBlock::Create(context_, "else");
      auto MergeBB = llvm::BasicBlock::Create(context_, "ifcont");
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
      
      llvm::PHINode *PN = builder_.CreatePHI(llvm::Type::getDoubleTy(context_), 2, "iftmp");
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
      auto LoopBB = llvm::BasicBlock::Create(context_, "loop", TheFunction);
      
      // Insert an explicit fall through from the current block to the LoopBB.
      builder_.CreateBr(LoopBB);
      
      // Start insertion in LoopBB.
      builder_.SetInsertPoint(LoopBB);
      
      // Start the PHI node with an entry for Start.
      auto Variable = builder_.CreatePHI(llvm::Type::getDoubleTy(context_),
                                            2, forExpr->getKey().c_str());
      
      Variable->addIncoming(StartVal, PreheaderBB);
      
      // Within the loop, the variable is defined equal to the PHI node.  If it
      // shadows an existing variable, we have to restore it, so save it now.
      const auto& varName = forExpr->getKey();
      auto OldVal = namedValues_[varName];
      namedValues_[varName] = CreateEntryBlockAlloca(TheFunction, varName);
      
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
         StepVal = llvm::ConstantFP::get(context_, llvm::APFloat(1.0));
      }
      
      auto NextVar = builder_.CreateFAdd(Variable, StepVal, "nextvar");
      
      // Compute the end condition.
      auto EndCond = forExpr->getEnd()->codeGen();
      if (!EndCond)
         return nullptr;
      
      // Convert condition to a bool by comparing equal to 0.0.
      EndCond = builder_.CreateFCmpONE(EndCond,
                                       llvm::ConstantFP::get(context_,
                                                             llvm::APFloat(0.0)), "loopcond");
      
      // Create the "after loop" block and insert it.
      auto LoopEndBB = builder_.GetInsertBlock();
      auto AfterBB = llvm::BasicBlock::Create(context_, "afterloop", TheFunction);
      
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
      return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(context_));
   }


   Function* CodeGeneratorImpl::codeGenPrototypeExpr(const PrototypeAST* protoExpr)
   {
      auto argList = protoExpr->getArgumentList();
      
      std::vector<llvm::Type*> args { argList.size(),
         llvm::Type::getDoubleTy(context_)};
      
      llvm::FunctionType* functionType = llvm::FunctionType::get(llvm::Type::getDoubleTy(context_),
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
      auto name = prototype->getName();
      llvm::Function* f = getFunction(prototype->getName());
      
      if( f == nullptr )
         f = prototype->codeGen();
      
      if(prototype->isBinary())
         binaryOperationPrecedence_[prototype->getOperatorName()] = prototype->getBinaryPrecedence();
      
      llvm::BasicBlock* bb = llvm::BasicBlock::Create(context_, "entry", f);
      builder_.SetInsertPoint(bb);
      namedValues_.clear();
      for( auto& arg : f->args())
      {
         AllocaInst *alloca = CreateEntryBlockAlloca(f, arg.getName());
         builder_.CreateStore(&arg, alloca);
         namedValues_[arg.getName()] = alloca;
      }

      auto returnValue = body->codeGen();
      
      //incredible hack to move in the ptr!!! work this out in some way that's better
      auto& p = const_cast<std::unique_ptr<PrototypeAST>&>(prototype);
      std::unique_ptr<PrototypeAST> ptr;
      ptr.reset(p.release());
      prototypeCache_[ptr->getName()] = std::move(ptr);
      
      if(returnValue != nullptr)
      {
         builder_.CreateRet(returnValue);
         if(!llvm::verifyFunction(*f)) {
            //eager optimization peephole
            optimizer_->runLocalFunctionOptimization(f);
         }
         return f;
      }
      
      //error reading the body
      f->eraseFromParent();
      return nullptr;
   }
   
   Value* CodeGeneratorImpl::codeGeneVarExpr(const VarExprAST* variableExpr)
   {
      std::vector<AllocaInst *> oldBindings;
      Function *function = builder_.GetInsertBlock()->getParent();
      
      const auto& variableNames = variableExpr->getVarNames();
      
      for (unsigned i = 0, e = variableNames.size(); i != e; ++i)
      {
         const auto& varName = variableNames[i].first;
         const auto& init = variableNames[i].second;
         
         //emit init
         Value *initVal;
         if (init)
         {
            initVal = init->codeGen();
            if (!initVal)
               return nullptr;
         }
         else
         {
            initVal = llvm::ConstantFP::get(context_, llvm::APFloat(0.0));
         }
         
         auto alloca = CreateEntryBlockAlloca(function, varName);
         builder_.CreateStore(initVal, alloca);
         
         //memorize bind
         oldBindings.push_back(namedValues_[varName]);
         
         // Remember this binding.
         namedValues_[varName] = alloca;
      }
      
      // Codegen the body, now that all vars are in scope.
      auto bodyVal = variableExpr->getBody()->codeGen();
      if (!bodyVal)
         return nullptr;
      
      // Pop all our variables from scope.
      for (unsigned i = 0, e = variableNames.size(); i != e; ++i)
         namedValues_[variableNames[i].first] = oldBindings[i];
      
      // Return the body computation.
      return bodyVal;

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
   
   ///
   /// private interface
   ///
   
   AllocaInst* CodeGeneratorImpl::CreateEntryBlockAlloca(Function *function, const std::string &variableName)
   {
      llvm::IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
      return TmpB.CreateAlloca(llvm::Type::getDoubleTy(context_), 0, variableName.c_str());
   }
   
   ///
   ///
   ///
   Function* CodeGeneratorImpl::getFunction(const std::string& name) const
   {
      if( auto f = module_->getFunction(name) )
         return f;
      
      auto fi = prototypeCache_.find(name);
      if ( fi != prototypeCache_.end())
         return fi->second->codeGen();
      
      return nullptr;
   }

   
   ///
   /// @brief: manage assigment
   ///
   llvm::Value* CodeGeneratorImpl::manageAssignment(const BinaryExprAST* binaryExpression)
   {
      using variable_expression_t = const std::unique_ptr<VariableExprAST>&;
      //bit hacky here...
      const auto& lhs = reinterpret_cast<variable_expression_t>(binaryExpression->getLeftOperand());
      const auto& rhs = binaryExpression->getRightOperand();
      
      if (!lhs)
         return errorV("destination of '=' must be a variable");
      
      if(!rhs)
         return errorV("expression to evaluate to the right of '=' must be valid");
      
      auto value = rhs->codeGen();
      if (!value)
         return nullptr;
      
      // Look-up the name.
      auto variable = namedValues_[lhs->getName()];
      if (!variable)
         return errorV("Unknown variable name");
      
      builder_.CreateStore(value, variable);
      return value;
   }
}
