//
//  JIT.cpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 13/02/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "JIT.h"

#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"


#include <string>
#include <iostream>
#include <memory>



namespace jit
{
   JIT::JIT() :
      targetMachine_(engineBuilder_.selectTarget()),
      dataLayout_(llvm::DataLayout(targetMachine_->createDataLayout())),
      objectLayer_([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
      compileLayer_(objectLayer_,llvm::orc::SimpleCompiler(*targetMachine_)),
      optimizeLayer_(compileLayer_, [this](std::shared_ptr<llvm::Module> M) {return optimizeModule(std::move(M));})
   {
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
   }
   
   llvm::TargetMachine &JIT::getTargetMachine() const
   {
      return *targetMachine_;
   }
   
   JIT::ModuleHandle JIT::addModule(std::unique_ptr<llvm::Module>& module)
   {
      //first function to invoke in order to resolve symbol
      auto firstResolver = [&](const std::string &name)
      {
         if (auto symbol = optimizeLayer_.findSymbol(name, false)) return symbol;
         return llvm::JITSymbol(nullptr);
      };
      
      //second resolver to invoke in order to resolve external symbols not found by resolver 1
      auto secondResolver = [](const std::string &name) {
         if (auto synbolAddress = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name))
            return llvm::JITSymbol(synbolAddress, llvm::JITSymbolFlags::Exported);
         return llvm::JITSymbol(nullptr);
      };
      
      
      auto resolver = llvm::orc::createLambdaResolver(firstResolver, secondResolver);
      //return cantFail(compileLayer_.addModule(std::move(module), std::move(resolver)));
      return cantFail(optimizeLayer_.addModule(std::move(module), std::move(resolver)));
   }
   
   llvm::JITSymbol JIT::findSymbol(const std::string& name) {
      std::string MangledName;
      llvm::raw_string_ostream MangledNameStream(MangledName);
      llvm::Mangler::getNameWithPrefix(MangledNameStream, name, dataLayout_);
      //return compileLayer_.findSymbol(MangledNameStream.str(), true);
      
      return optimizeLayer_.findSymbol(MangledNameStream.str(), true);
   }
   
   llvm::JITTargetAddress JIT::getSymbolAddress(const std::string& name) {
      return cantFail(findSymbol(name).getAddress());
   }
   
   void JIT::removeModule(ModuleHandle H) {
      //cantFail(compileLayer_.removeModule(H));
      cantFail(optimizeLayer_.removeModule(H));
   }
   
   std::shared_ptr<llvm::Module> JIT::optimizeModule(std::shared_ptr<llvm::Module> module)
   {
      // Create a function pass manager.
      auto functionPassManager = llvm::make_unique<llvm::legacy::FunctionPassManager>(module.get());
      
      // Add some optimizations.
      functionPassManager->add(llvm::createInstructionCombiningPass());
      functionPassManager->add(llvm::createReassociatePass());
      functionPassManager->add(llvm::createNewGVNPass());
      functionPassManager->add(llvm::createCFGSimplificationPass());
      functionPassManager->doInitialization();
      
      // Run the optimizations over all functions in the module being added to
      // the JIT.
      for (auto &function : *module)
         functionPassManager->run(function);
         
         return module;
   }
}

