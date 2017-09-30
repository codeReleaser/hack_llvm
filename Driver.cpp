//
//  Driver.cpp
//  llvm
//
//  Created by Nicola Cabiddu on 21/09/2017.
//  Copyright © 2017 Nicola Cabiddu. All rights reserved.
//

#include "Driver.h"
#include "Parser.h"
#include <iostream>
#include "llvm/Support/TargetSelect.h"


driver::DriverConfiguration::DriverConfiguration(bool enableJit,
                                                 bool enableOpt,
                                                 bool enableDebug,
                                                 bool saveAsObjectFile,
                                                 bool saveAsAsmFile,
                                                 bool saveAsIRFile,
                                                 bool dumpOnScreen) : enableJit_(enableJit), enableOpt_(enableOpt), enableDebug_(enableDebug), saveAsObjectFile_(saveAsObjectFile), saveAsAsmFile_(saveAsAsmFile),saveAsIRFile_(saveAsIRFile), dumpOnScreen_(dumpOnScreen)
{}

driver::Driver::Driver(driver::DriverConfiguration cnf) :
   cnf_(std::move(cnf))
{}

void driver::Driver::go()
{
   
   InitializeNativeTarget();
   InitializeNativeTargetAsmPrinter();
   InitializeNativeTargetAsmParser();
   
   parser::Parser parser_;
   parser_.setTokenPrecedence('=', 2);
   parser_.setTokenPrecedence('<', 10);
   parser_.setTokenPrecedence('+', 20);
   parser_.setTokenPrecedence('-', 30);
   parser_.setTokenPrecedence('*', 40);
   
   std::cout<<"\n >>";
   parser_.getNextToken();
   parser_.mainLoop();
   
}
