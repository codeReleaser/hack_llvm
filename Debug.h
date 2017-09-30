//
//  Debug.hpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 30/04/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef Debug_h
#define Debug_h

#include "llvm/IR/DIBuilder.h"
#include <vector>
#include <memory>

namespace llvm
{
   class DICompileUnit;
   class DIType;
   class DIScope;
   class DISubroutineType;
   class DIFile;
}

namespace AST
{
   class ExprAST;
}

using namespace llvm;
using namespace AST;

namespace debug
{
   struct Info
   {
      DICompileUnit *compilationUnit_;
      DIType *debugType_;
      std::vector<DIScope *> lexicalBlocks_;
      
      void emitLocation(ExprAST *AST);
      DIType *getDoubleTy();
      
   };
   
   struct SourceLocation
   {
      int line;
      int col;
   };
   
   
   class DebugInfo
   {
   public:
      
      static SourceLocation currentLocation_;
      static SourceLocation currentLexerLocation_;
      
      ///
      /// @brief: ctor for debug info
      ///
      explicit DebugInfo();
      
      ///
      /// @brief: get type
      ///
      DIType *getDoubleTy();
      
      ///
      /// @brief: emit debug information
      ///
      void emitLocation(ExprAST *AST);
      
      ///
      /// @brief: create function
      ///
      DISubroutineType *CreateFunctionType(unsigned numArgs, DIFile *Unit);
      
      
   private:
      
      std::unique_ptr<DIBuilder> DBuilder;


   };
   
}

#endif /* Debug_h */
