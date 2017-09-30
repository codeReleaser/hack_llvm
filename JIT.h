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
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"

#include <functional>
#include <memory>

namespace jit
{
   class JIT
   {
      
   private:
      
      std::unique_ptr<llvm::TargetMachine> targetMachine_;
      llvm::DataLayout dataLayout_;
      llvm::orc::RTDyldObjectLinkingLayer objectLayer_;
      using CompileLayer = llvm::orc::IRCompileLayer<decltype(objectLayer_), llvm::orc::SimpleCompiler>;
      CompileLayer compileLayer_;
      
      
      //embedded optimization inside the jit compiler
      using OptimizeFunction = std::function<std::shared_ptr<llvm::Module>(std::shared_ptr<llvm::Module>)>;
      llvm::orc::IRTransformLayer<decltype(compileLayer_), OptimizeFunction> optimizeLayer_;
      
   private:
      
      std::shared_ptr<llvm::Module> optimizeModule(std::shared_ptr<llvm::Module> module);
      
   public:
      
      using ModuleHandle = decltype(compileLayer_)::ModuleHandleT;
      
      explicit JIT();
      llvm::EngineBuilder engineBuilder_;
      llvm::TargetMachine &getTargetMachine() const;
      ModuleHandle addModule(std::unique_ptr<llvm::Module>& module);
      llvm::JITSymbol findSymbol(const std::string& name);
      llvm::JITTargetAddress getSymbolAddress(const std::string& name);
      void removeModule(ModuleHandle moduleHandle);      
      
   };
}

#endif /* JIT_h */
