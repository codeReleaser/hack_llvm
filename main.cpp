//
//  main.cpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 21/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include <iostream>


#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "TestParser.h"


int main(int argc, const char * argv[]) {
   
   TestParser test;
   test.interativeTest();
   
   return 0;
}
