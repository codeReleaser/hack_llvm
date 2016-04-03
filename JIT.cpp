//
//  JIT.cpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 13/02/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "JIT.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"


#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/Function.h"


#include <string>
#include <iostream>

namespace jit
{
   JIT::JIT() :
      execEngine_(nullptr)
   {
      std::string ErrStr;
      
      llvm::InitializeNativeTarget();
      llvm::InitializeNativeTargetAsmPrinter();
      llvm::InitializeNativeTargetAsmParser();
      
      module_ = std::make_unique<llvm::Module>("jit compiler", llvm::getGlobalContext());

      execEngine_ = llvm::EngineBuilder(std::move(module_))
      .setErrorStr(&ErrStr)
      .setMCJITMemoryManager(std::make_unique<llvm::SectionMemoryManager>())
      .create();
      
      if (!execEngine_)
      {
         std::cerr << ErrStr << std::endl;
         throw; //terminate
      }
   }
   
   JIT::~JIT()
   {
      if(module_)
      {
         module_->dump();
      }
      
      delete execEngine_;
      
   }
   
   llvm::Module* JIT::getModulePtr()
   {
      return module_.get();
   }

   void JIT::evalFunction(llvm::Function* function)
   {
      if(execEngine_)
      {
         execEngine_->finalizeObject();
         void *FPtr = execEngine_->getPointerToFunction(function);
         
         // Cast it to the right type (takes no arguments, returns a double) so we
         // can call it as a native function.
         double (*FP)() = (double (*)())(intptr_t)FPtr;
         std::cerr << "Evaluated to " << FP() << std::endl;
      }

   }
   
}

