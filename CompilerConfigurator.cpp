//
//  CompilerConfigurator.cpp
//  llvm
//
//  Created by Nicola Cabiddu on 21/09/2017.
//  Copyright © 2017 Nicola Cabiddu. All rights reserved.
//

#include "CompilerConfigurator.h"
#include "JIT.h"
#include "CodeGenerator.h"

util::CompilerConfigurator::CompilerConfigurator(code_generator::CodeGenerator& codeGenerator, jit::JIT& jitCompiler) :
   codeGenerator_(codeGenerator),
   jitCompiler_(jitCompiler)
{}

code_generator::CodeGenerator& util::CompilerConfigurator::getCodeGenerator() const
{
   return codeGenerator_;
}

jit::JIT& util::CompilerConfigurator::getJitCompiler() const
{
   return jitCompiler_;
}
