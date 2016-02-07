//
//  TestParser.hpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 11/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef TestParser_h
#define TestParser_h

#include <memory>
#include "Parser.h"

class TestParser
{
   parser::Parser parser_;
   
public:
   ///
   /// @brief: run the main loop and ask the user to enter valid code statements
   ///
   void interativeTest();
};

#endif /* TestParser_h */
