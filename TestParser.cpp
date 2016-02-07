//
//  TestParser.cpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 11/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "TestParser.h"
#include "Parser.h"
#include <iostream>


void TestParser::interativeTest()
{
   parser_.setTokenPrecedence('<', 10);
   parser_.setTokenPrecedence('+', 20);
   parser_.setTokenPrecedence('-', 30);
   parser_.setTokenPrecedence('*', 40);
   
   // Prime the first token.
   std::cerr<<"> ";
   parser_.getNextToken();
   
   // Run the main "interpreter loop" now.
   parser_.mainLoop();

}