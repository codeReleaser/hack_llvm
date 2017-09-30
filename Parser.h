//
//  Parser.hpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 07/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#ifndef Parser_h
#define Parser_h

#include <map>
#include <memory>

#include "Lexer.h"
#include "AST.h"
#include "CompilerConfigurator.h"
#include "CodeGenerator.h"
#include "JIT.h"

//namespace AST {
//   class ExprAST;
//   class PrototypeAST;
//   class FunctionAST;
//
//}


using namespace AST;

namespace parser {
   
   using expression_t = std::unique_ptr<ExprAST>;
   using prototype_t = std::unique_ptr<PrototypeAST>;
   using function_t = std::unique_ptr<FunctionAST>;
   
   class Parser
   {
   public:
      
      ///
      /// default constructor
      ///
      explicit Parser();
      
      ///
      /// delete copy ctor and copy assignment
      ///
      Parser(const Parser&) = delete;
      Parser& operator=(const Parser&) = delete;
      
      ///
      /// Deal with the language constructs
      ///
      
      int getNextToken();
      void setTokenPrecedence(unsigned char, int);
      int getTokenPrecedence();

      expression_t error(const char* str);
      prototype_t errorP(const char* str);
            
      ///numbexpr := number
      expression_t parseNumberExpr();
      
      ///parenexpr := '(' expression ')'
      expression_t parseParentExpr();
      
      ///identifierexpr
      ///   := identifier
      //    := identifier '('expression')'
      expression_t parseIdentifierExpr();
      
      /// primary
      ///   ::= identifierexpr
      ///   ::= numberexpr
      ///   ::= parenexpr
      expression_t parsePrimaryExpression();
      
      /// unary operation
      ///  ::= primary
      ///  ::= '!' unary
      expression_t parseUnary();
      
      /// binary operation
      ///   ::= ('+' primary)*
      expression_t parseBinOpRHS(int exprPrec, expression_t lhs);
      
      /// expression
      ///   ::= primary binoprhs
      ///
      expression_t parseExpression();
      
      /// prototype
      ///   ::= id '(' id* ')'
      ///   ::= unary  LETTER number? (id)
      ///   ::= binary LETTER number? (id, id)
      prototype_t parsePrototype();
      
      /// definition ::= 'def' prototype expression
      function_t parseDefinition();

      /// toplevelexpr ::= expression
      function_t parseTopLevelExpr();
      
      /// external ::= 'extern' prototype
      prototype_t parseExtern();
      
      /// ifexpr ::= 'if' expression 'then' expression 'else' expression
      expression_t parseIfExpr();
      
      /// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
      expression_t parseForExpr();
      
      /// varexpr ::= 'var' identifier ('=' expression)?
      //  (',' identifier ('=' expression)?)* 'in' expression
      expression_t parseVarExpr();
      
      ///
      /// Top Level parsing
      ///
      void handleDefinition();
      void handleExtern();
      void handleTopLevelExpression();
      
      void mainLoop();
      
   private:
      
      code_generator::CodeGeneratorImpl codeGenerator_;
      jit::JIT jitCompiler_;

      
      int curToken_;
      util::CompilerConfigurator configurator_;
      std::unique_ptr<lexer::Lexer> lexer_;
      
   };
   
   
}

#endif /* Parser_h */
