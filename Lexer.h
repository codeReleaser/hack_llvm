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

namespace parser {
   class Parser;
}

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
      tok_number = -5
   };
   
   
   class Lexer {
      
      friend class parser::Parser;
      
   private:
      
      std::string identifierStr_;
      double numVal_;
      
   public:
      
      int gettok();
      double getNum() const;
      std::string getId() const;
      
   };
   
   
}

#endif /* Lexer_h */
