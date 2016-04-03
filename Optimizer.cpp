//
//  Optimizer.cpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 13/02/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "Optimizer.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

namespace optimizer
{
   ///
   /// @brief: construct optimizer and init the function passage manager
   ///
   Optimizer::Optimizer(llvm::Module* module) :
      funcPassManager_(nullptr)
   {
      prematureOptimization(module);
   }
   
   ///
   /// @brief: run FunctionPassManager optimizer for the function passed
   ///
   void Optimizer::runLocalFunctionOptimization(llvm::Function *f)
   {
      funcPassManager_->run(*f);
   }
   
   
   ///
   /// @brief: premature optimization for function generated (mainly peephole opt)
   ///
   
   void Optimizer::prematureOptimization(llvm::Module* module)
   {
      //initializa the function passage manager
      funcPassManager_ = std::make_unique<llvm::legacy::FunctionPassManager>(module);
      
      // Do simple "peephole" optimizations plus something else.
      funcPassManager_->add(llvm::createInstructionCombiningPass());
      // Reassociate expressions.
      funcPassManager_->add(llvm::createReassociatePass());
      // Eliminate Common SubExpressions.
      funcPassManager_->add(llvm::createGVNPass());
      // Simplify the control flow graph (deleting unreachable blocks, etc).
      funcPassManager_->add(llvm::createCFGSimplificationPass());
      //do init
      funcPassManager_->doInitialization();

   }

   
}

