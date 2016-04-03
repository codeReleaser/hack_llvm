//
//  Optimizer.hpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 13/02/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef Optimizer_h
#define Optimizer_h

#include <memory>
#include "llvm/IR/LegacyPassManager.h"

namespace llvm {
   class Function;
   class Module;
}

namespace  optimizer
{
   ///
   /// @brief: optimizer
   ///
   class Optimizer
   {
      std::unique_ptr<llvm::legacy::FunctionPassManager> funcPassManager_;
      
      void prematureOptimization(llvm::Module* module);
      
   public:
      explicit Optimizer(llvm::Module*);
      void runLocalFunctionOptimization(llvm::Function* f);
      
   };
}



#endif /* Optimizer_h */
