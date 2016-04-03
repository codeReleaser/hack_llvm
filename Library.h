//
//  Library.hpp
//  Kaleidoscope-LLVM-Bis
//
//  Created by Nicola Cabiddu on 03/04/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef Library_hpp
#define Library_hpp

#include <stdio.h>

// fake std library for the kaledoiscope programming language


extern "C" double putchard(double X) {
   fputc((char)X, stderr);
   return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double X) {
   fprintf(stderr, "%f\n", X);
   return 0;
}

#endif /* Library_hpp */
