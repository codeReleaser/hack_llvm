//
//  Lexer.cpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 06/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "Lexer.h"

#include <cctype>
#include <cstdlib>


namespace lexer
{
   /**
    * @brief: tokenize my input.
    *         Reading from std::input a single char and recongnise the basic tokens of the language
    */
   int Lexer::gettok() {
      
      static int LastChar = ' ';
      
      // Skip any whitespace.
      while (isspace(LastChar)) {
         LastChar = getchar();
      }
      
      if (isalpha(LastChar)) {
         // identifier: [a-zA-Z][a-zA-Z0-9]*
         identifierStr_ = LastChar;
         while (isalnum((LastChar = getchar())))
         {
            identifierStr_ += LastChar;
         }
         
         if (identifierStr_ == "def")
         {
            return tok_def;
         }
         else if (identifierStr_ == "extern")
         {
            return tok_extern;
         }
         
         return tok_identifier;
      }
      
      if (isdigit(LastChar) || LastChar == '.') {
         
         // Number: [0-9.]+
         std::string NumStr;
         do
         {
            NumStr += LastChar;
            LastChar = getchar();
         } while (isdigit(LastChar) || LastChar == '.');
         
         numVal_ = strtod(NumStr.c_str(), nullptr);
         return tok_number;
      }
      
      
      if (LastChar == '#') {
         // Comment until end of line.
         do
         {
            LastChar = getchar();
         }while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
         
         if (LastChar != EOF)
            return gettok();
      }
      
      // Check for end of file.  Don't eat the EOF.
      if (LastChar == EOF)
         return tok_eof;
      
      // Otherwise, just return the character as its ascii value.
      int ThisChar = LastChar;
      LastChar = getchar();
      return ThisChar;
     
   }
   
}