//
//  Lexer.hpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 06/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef Lexer_h
#define Lexer_h

#include <string>
#include "Debug.h"

namespace lexer
{
   
   /*
    * Token enumeration.
    * Every time the lexer encounters a character that does not know it retunrs a value in the range [0-255]
    * Otherwise it returns a negative value.
    */
   enum Token
   {
      tok_eof = -1,
      
      // commands
      tok_def = -2,
      tok_extern = -3,
      
      // primary
      tok_identifier = -4,
      tok_number = -5,
      
      //control
      tok_if = -6,
      tok_then = -7,
      tok_else = -8,
      
      //for loop
      tok_for = -9,
      tok_in = -10,
      
      //user operators
      tok_unary = -11,
      tok_binary = -12,
      
      //variable definition
      tok_var = -13

   };
   
   
   class Lexer {
      
   public:
      
      Lexer(/*debug::DebugInfo& debug*/);
      
      /**
       * @brief: tokenize my input.
       *         Reading from std::input a single char and recongnise the basic tokens of the language
       */
      int gettok();
      
      double getNum() const;
      std::string getId() const;
      
      
   private:
      std::string identifierStr_;
      double numVal_;
      //debug::DebugInfo& debug_;
      
      
      int advance();
      
   };
   
   
}

#endif /* Lexer_h */
