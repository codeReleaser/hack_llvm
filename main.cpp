//
//  main.cpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 21/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include <iostream>
#include "Driver.h"

int main(int argc, const char * argv[]) {
   
   driver::DriverConfiguration cnf;
   driver::Driver driver{cnf};
   driver.go();
   
   return 0;
}
