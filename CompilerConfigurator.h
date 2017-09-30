//
//  CompilerConfigurator.hpp
//  llvm
//
//  Created by Nicola Cabiddu on 21/09/2017.
//  Copyright © 2017 Nicola Cabiddu. All rights reserved.
//

#ifndef CompilerConfigurator_h
#define CompilerConfigurator_h

#include <memory>

//forward declaration
namespace code_generator
{
   class CodeGenerator;
}

namespace jit
{
   class JIT;
}

namespace debug
{
   class Info;
   class SourceLocation;
   class DebugInfo;
}


namespace util
{
   class CompilerConfigurator
   {
      
   private:
      
      code_generator::CodeGenerator& codeGenerator_;
      jit::JIT& jitCompiler_;
 
   public:
      explicit CompilerConfigurator(code_generator::CodeGenerator& codeGenerator, jit::JIT& jitCompiler_);

      CompilerConfigurator(const CompilerConfigurator& rhs) = delete;
      CompilerConfigurator& operator=(CompilerConfigurator& rhs) = delete;
      CompilerConfigurator(CompilerConfigurator&&) = default;
      CompilerConfigurator& operator=(CompilerConfigurator&&) = default;
      
      code_generator::CodeGenerator& getCodeGenerator() const;
      jit::JIT& getJitCompiler() const;
      
      static CompilerConfigurator instance_;
      
   };
   
}


#endif /* CompilerConfigurator_hpp */
