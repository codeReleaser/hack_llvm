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
            return tok_def;
         if (identifierStr_ == "extern")
            return tok_extern;
         if (identifierStr_ == "if")
            return tok_if;
         if (identifierStr_ == "then")
            return tok_then;
         if (identifierStr_ == "else")
            return tok_else;
         if(identifierStr_ == "for")
            return tok_for;
         if(identifierStr_ == "in")
            return tok_in;
         
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
   
   double Lexer::getNum() const
   {
      return numVal_;
   }
   
   std::string Lexer::getId() const
   {
      return identifierStr_;
   }
   
}