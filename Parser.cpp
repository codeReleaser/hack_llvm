//
//  Parser.cpp
//  Kaleidoscope-LLVM
//
//  Created by Nicola Cabiddu on 07/01/2016.
//  Copyright © 2016 Nicola Cabiddu. All rights reserved.
//

#include "Parser.h"

#include "AST.h"
#include "Lexer.h"
#include "CodeGenerator.h"

#include <cctype>
#include <vector>
#include <string>
#include <iostream>

using namespace code_generator;
using namespace lexer;

namespace parser
{
   using ArgsExpr_t = std::vector<std::unique_ptr<AST::ExprAST>>;
   using ArgsStr_t = std::vector<std::string>;
   
   ///
   /// @brief: construct a pimpl lexer
   ///
   Parser::Parser() :
   curToken_(0),
   binaryOperationPrecedence_(),
   lexer_(std::make_unique<Lexer>())
   {}
   
   ///
   /// intenal routines
   ///
   
   int Parser::getNextToken()
   {
      curToken_ = lexer_->gettok();
      return curToken_;
   }
   
   int Parser::getTokenPrecedence()
   {
      if(!isascii(curToken_))
         return -1;
      
      int tokenPrec = binaryOperationPrecedence_[curToken_];
      if(tokenPrec <=0 )
         return -1;
      
      return tokenPrec;
   }
   
   void Parser::setTokenPrecedence(unsigned char token, int value)
   {
      binaryOperationPrecedence_[token] = value;
   }
   
   expression_t Parser::error(const char* str)
   {
      std::cerr << "Error: "<< str << "\n";
      return nullptr;
   }

   prototype_t Parser::errorP(const char *str)
   {
      error(str);
      return nullptr;
   }
   
   expression_t Parser::parseExpression()
   {
      auto lhs = parsePrimaryExpression();
      if (!lhs)
         return nullptr;
      
      return parseBinOpRHS(0, std::move(lhs));
   }
   
   expression_t Parser::parseNumberExpr()
   {
      auto res = std::make_unique<AST::NumberExprAST>(lexer_->numVal_);
      getNextToken();
      return std::move(res);
   }
   
   expression_t Parser::parseParenExpr()
   {
      getNextToken();
      
      auto v = parseExpression();
      
      if(v == nullptr)
         return nullptr;
      
      if(curToken_ != ')')
         return error("expected )");
   
      getNextToken();
      
      return v;
   }
   
   expression_t Parser::parseIdentifierExpr()
   {
      auto idName = lexer_->identifierStr_;
      
      getNextToken();
      
      if( curToken_ != '(')
         return std::make_unique<AST::VariableExprAST>(idName);
      
      getNextToken();
      ArgsExpr_t args;
      
      if( curToken_ != ')')
      {
         while(1)
         {
            auto arg = parseExpression();
            if( arg != nullptr)
            {
               args.push_back(std::move(arg));
            }
            else
            {
               return nullptr;
            }
            
            if( curToken_ == ')')
               break;
            
            if(curToken_ != ',')
               return error("Expected ) or , in argument list");
            
            getNextToken();
         }
      }
      
      getNextToken();
      return std::make_unique<AST::CallExprAST>(idName, std::move(args));
   }
   
   expression_t Parser::parsePrimaryExpression()
   {
      switch(curToken_)
      {
         default:
            return error("Unknown token where aspecting an expression");
            
         case lexer::tok_identifier :
            return parseIdentifierExpr();
            
         case lexer::tok_number:
            return parseNumberExpr();
            
         case '(':
            return parseParenExpr();
      }
   }
   
   expression_t Parser::parseBinOpRHS(int exprPrec, expression_t lhs)
   {
      while( true )
      {
         auto tokenPrec = getTokenPrecedence();
         if(tokenPrec < exprPrec)
            return lhs;
         
         int binOp = curToken_;
         getNextToken();
         
         auto rhs = parsePrimaryExpression();
         if( rhs == nullptr)
            return nullptr;
         
         int nextPrec = curToken_;
         if(tokenPrec < nextPrec)
         {
            rhs = parseBinOpRHS(tokenPrec+1, std::move(rhs));
            if(rhs == nullptr)
               return nullptr;
         }
         
         lhs = std::make_unique<AST::BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
      }
   }
   
   prototype_t Parser::parsePrototype()
   {
      if(curToken_ != lexer::tok_identifier)
      {
         return errorP("expected function name in prototype");
      }
      
      auto functionName = lexer_->identifierStr_;
      
      getNextToken();
      
      if(curToken_ != '(')
         return errorP("expected '(' in prototype");
      
      ArgsStr_t args;
      while(getNextToken() == lexer::tok_identifier)
      {
         args.push_back(lexer_->identifierStr_);
      }
      
      if(curToken_!= ')')
         return errorP("expected ')' in prototype");
      
      getNextToken();
      return std::make_unique<AST::PrototypeAST>(functionName, std::move(args));
   }
   
   function_t Parser::parseDefinition()
   {
      getNextToken();
      auto prototype = parsePrototype();
      if(prototype == nullptr)
         return nullptr;
      
      auto expression = parseExpression();
      if( expression != nullptr )
      {
         return std::make_unique<AST::FunctionAST>(std::move(prototype), std::move(expression));
      }
      
      return nullptr;
   }
   
   function_t Parser::parseTopLevelExpr()
   {
      auto expression = parseExpression();
      if( expression != nullptr)
      {
         auto prototype = std::make_unique<AST::PrototypeAST>("__anon_expr", std::vector<std::string> {});
         return std::make_unique<AST::FunctionAST>(std::move(prototype), std::move(expression));
      }
      return nullptr;
   }
   
   prototype_t Parser::parseExtern()
   {
      getNextToken();
      return parsePrototype();
   }
   
   ///
   /// Top-Level parsing
   ///
   
   void Parser::handleDefinition()
   {
      if(auto parsedDefinition = parseDefinition())
      {
         if( auto* defintionIR = parsedDefinition->codeGen())
         {
            std::cerr << "Parsed a top level expr \n";
            defintionIR->dump();
         }
      }
      else
      {
         getNextToken();
      }
   }
   
   void Parser::handleExtern()
   {
      if(auto parsedExtern = parseExtern())
      {
         if(auto* externIR = parsedExtern->codeGen())
         {
            std::cerr << "Parsed an extern \n";
            externIR->dump();
         }
      }
      else
      {
         getNextToken();
      }
   }
   
   void Parser::handleTopLevelExpression()
   {
      if(auto parsedTopLevelExpr = parseTopLevelExpr())
      {
         if( auto* topLevelExprIR = parsedTopLevelExpr->codeGen())
         {
            std::cerr << "Parsed a top level expr \n";
            topLevelExprIR->dump();
         }
      }
      else
      {
         getNextToken();
      }
   }
   
   ///
   /// main loop of parsing
   /// top ::= definition | external | expression | ';'
   ///
   
   void Parser::mainLoop()
   {
      while(true)
      {
         switch(curToken_)
         {
            case lexer::tok_eof:
               return;
            case ';':
               getNextToken();
               break;
            case lexer::tok_def:
               handleDefinition();
               break;
            case lexer::tok_extern:
               handleExtern();
               break;
            default:
               handleTopLevelExpression();
               break;
         }
         
         std::cerr << ">";

      }
   }
}
