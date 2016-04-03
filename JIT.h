//
//  JIT.hpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 13/02/2016.
//  Copyright © 2016 Nicola Cabiddu.
//
//

#ifndef JIT_h
#define JIT_h

#include "llvm/IR/Module.h"
#include <memory>

///
/// this code does not work. I am keep getting "JIT has not linked in." ... I understood
/// potentially it can be a problem of including header and force the link...
/// But I haven't found any solution to this yet.
///

namespace llvm {
   
   class ExecutionEngine;
   class Module;
   class Function;
   
   namespace legacy {
      class FunctionPassManager;
   }
}

namespace jit
{
   class JIT
   {
      std::unique_ptr<llvm::Module> module_;
      llvm::ExecutionEngine* execEngine_;

   public:
      
      explicit JIT();
      ~JIT();
      
      llvm::Module* getModulePtr();
      
      void evalFunction(llvm::Function* function);
      
   };
}

#endif /* JIT_h */
