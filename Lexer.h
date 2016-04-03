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
      tok_def,
      tok_extern,
      
      // primary
      tok_identifier,
      tok_number,
      
      //control
      tok_if,
      tok_then,
      tok_else,
      
      //for loop
      tok_for,
      tok_in
   };
   
   
   class Lexer {
      
   public:
      
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
      
   };
   
   
}

#endif /* Lexer_h */
