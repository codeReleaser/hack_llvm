//
//  Debug.cpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 30/04/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "Debug.h"
#include "AST.h"
#include "llvm/IR/DebugInfo.h"


debug::SourceLocation debug::DebugInfo::currentLocation_;
debug::SourceLocation debug::DebugInfo::currentLexerLocation_ = {0,1};
using AST::ExprAST;

namespace debug
{
   
   DebugInfo::DebugInfo()
   {}
   
   
   DIType *DebugInfo::getDoubleTy()
   {
      return nullptr;
   }
  
   void DebugInfo::emitLocation(ExprAST *AST)
   {
      
   }
   
   DISubroutineType *DebugInfo::CreateFunctionType(unsigned numArgs, DIFile *unit)
   {
      return nullptr;
   }
   
}
