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
#include "AST.h"
#include "Debug.h"
#include "llvm/Support/raw_ostream.h"


#include <cctype>
#include <vector>
#include <string>
#include <iostream>

using namespace code_generator;
using namespace lexer;

namespace parser
{
   using ArgsExpr_t = std::vector<std::unique_ptr<ExprAST>>;
   using ArgsStr_t = std::vector<std::string>;
   
   
   debug::DebugInfo gDebugInfo;
   //code_generator::CodeGeneratorImpl gCodeGenerator;
   //jit::JIT gJitCompiler;
   
   
   ///
   /// @brief: construct a pimpl lexer
   ///
   Parser::Parser() :
   curToken_(0),
   codeGenerator_(jitCompiler_),
   configurator_(util::CompilerConfigurator(codeGenerator_, jitCompiler_)),
   lexer_(std::make_unique<Lexer>())
   {
      codeGenerator_.InitializeModuleAndPassManager();
   }
   
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
      
      //here we could throw.. what should I do? .. dunno now
      
      int tokenPrec = configurator_.getCodeGenerator().getOperatorPrecedence(curToken_);
      return tokenPrec;
   }
   
   void Parser::setTokenPrecedence(unsigned char token, int value)
   {
      //here we could throw.. what should I do? .. dunno now
      configurator_.getCodeGenerator().setOperatorPrecedence(token, value);
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
      //parseUnary();
      auto lhs = parseUnary(); //parsePrimaryExpression();
      if (!lhs)
         return nullptr;
      
      return parseBinOpRHS(0, std::move(lhs));
   }
   
   expression_t Parser::parseNumberExpr()
   {
      auto res = std::make_unique<AST::NumberExprAST>(configurator_.getCodeGenerator(), lexer_->getNum());
      getNextToken();
      return std::move(res);
   }
   
   expression_t Parser::parseParentExpr()
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
      auto idName = lexer_->getId();
      
      //SourceLocation Idlocation = debugInfo_.currentLocation_;
      
      getNextToken();
      
      if( curToken_ != '(')
         return std::make_unique<AST::VariableExprAST>(configurator_.getCodeGenerator(), idName);
      
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
      return std::make_unique<AST::CallExprAST>(configurator_.getCodeGenerator(), idName, std::move(args));
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
            return parseParentExpr();
         
         case lexer::tok_if:
            return parseIfExpr();
            
         case lexer::tok_for:
            return parseForExpr();
            
         case lexer::tok_var:
            return parseVarExpr();
      }
   }
   
   expression_t Parser::parseUnary()
   {
      if ( !isascii(curToken_) || curToken_ == '(' || curToken_ == ',')
         return parsePrimaryExpression();
      
      int opcode = curToken_;
      getNextToken();
      if (auto operand = parseUnary())
         return std::make_unique<UnaryExprAST>(configurator_.getCodeGenerator(), opcode, std::move(operand));
      
      return nullptr;
   }
   
   expression_t Parser::parseBinOpRHS(int exprPrec, expression_t lhs)
   {
      while( true )
      {
         auto tokenPrec = getTokenPrecedence();
         if(tokenPrec < exprPrec)
            return lhs;
         
         int binOp = curToken_;
         //SourceLocation binaryOpLocation = debugInfo_.currentLocation_;
         getNextToken();
         
         auto rhs = parseUnary(); 
         if( rhs == nullptr)
            return nullptr;
         
         int nextPrec = curToken_;
         if(tokenPrec < nextPrec)
         {
            rhs = parseBinOpRHS(tokenPrec+1, std::move(rhs));
            if(rhs == nullptr)
               return nullptr;
         }
         
         lhs = std::make_unique<AST::BinaryExprAST>(configurator_.getCodeGenerator(), binOp, std::move(lhs), std::move(rhs));
      }
   }
   
   prototype_t Parser::parsePrototype()
   {
      //SourceLocation fnLocation = debugInfo_.currentLocation_;

      unsigned binaryPrecedence = 30;
      // by default I assume I am going to parse a prototype definition
      unsigned kind = 0; //0 (prototype) - 1(unary) - 2(binary)
      std::string functionName = lexer_->getId();
      
      switch (curToken_)
      {
         case lexer::tok_identifier:
            getNextToken();
            break;
            
            
         case lexer::tok_unary:
            
            getNextToken();
            
            if (!isascii(curToken_))
               return errorP("Expected unary operator");
            
            functionName = ("unary") + (char)curToken_;
            kind = 1;
            getNextToken();
            
            break;
            
         case lexer::tok_binary:
            
            getNextToken();
            if (!isascii(curToken_))
               return errorP("Expected binary operator");
            
            functionName = ("binary") + (char)curToken_;
            kind = 2;
            getNextToken();
            
            
            if (curToken_ == lexer::tok_number)
            {
               if (lexer_->getNum() < 1 || lexer_->getNum() > 100)
                  return errorP("Invalid precedecnce: must be 1..100");
               
               binaryPrecedence = (unsigned)lexer_->getNum();
               getNextToken();
            }
            
            break;
            
         default:
            return errorP("expected function name in prototype");
      }
      
      if (curToken_ != '(')
         return errorP("Expected '(' in prototype");
      
      ArgsStr_t args;
      while(getNextToken() == lexer::tok_identifier)
         args.push_back(lexer_->getId());
      
      if(curToken_!= ')')
         return errorP("expected ')' in prototype");
      
      // success.
      getNextToken();
      
      if (kind && args.size() != kind)
         return errorP("Invalid number of operands for operator");
      
      return std::make_unique<AST::PrototypeAST>(configurator_.getCodeGenerator(),
                                                 functionName,
                                                 std::move(args),
                                                 kind != 0,
                                                 binaryPrecedence);
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
         return
         std::make_unique<AST::FunctionAST>(configurator_.getCodeGenerator(), std::move(prototype), std::move(expression));
      }
      
      return nullptr;
   }
   
   function_t Parser::parseTopLevelExpr()
   {
      //SourceLocation fnLocation = debugInfo_.currentLocation_;
      auto expression = parseExpression();
      if( expression != nullptr)
      {
         auto prototype = std::make_unique<AST::PrototypeAST>(configurator_.getCodeGenerator(),
                                                              "__anon_expr", std::vector<std::string> {});
         
         return std::make_unique<AST::FunctionAST>(configurator_.getCodeGenerator(),
                                                   std::move(prototype), std::move(expression));
      }
      return nullptr;
   }
   
   prototype_t Parser::parseExtern()
   {
      getNextToken();
      return parsePrototype();
   }
   
   expression_t Parser::parseIfExpr()
   {
      //SourceLocation ifLocation = debugInfo_.currentLocation_;;

      getNextToken();
      auto Cond = parseExpression();
      
      if(!Cond)
         return nullptr;
      
      if (curToken_ != tok_then)
         return errorP("expected then");
      
      getNextToken();
      
      auto Then = parseExpression();
      if (!Then)
         return nullptr;
      
      if (curToken_ != tok_else)
         return errorP("expected else");
      
      getNextToken();
      
      auto Else = parseExpression();
      if (!Else)
         return nullptr;
      
      return std::make_unique<IfExprAST>(configurator_.getCodeGenerator(),
                                         std::move(Cond), std::move(Then), std::move(Else));
   }
   
   expression_t Parser::parseForExpr()
   {
      getNextToken();
      
      if (curToken_ != tok_identifier)
         return errorP("expected identifier after for");
      
      std::string IdName = lexer_->getId();
      
      getNextToken();
      
      if (curToken_ != '=')
         return errorP("expected '=' after for");
      
      getNextToken();
      
      auto Start = parseExpression();
      if (!Start)
         return nullptr;
      
      if (curToken_ != ',')
         return errorP("expected ',' after for start value");
      
      getNextToken();
      
      auto End = parseExpression();
      if (!End)
         return nullptr;
      
      // The step value is optional.
      expression_t Step;
      if (curToken_ == ',')
      {
         getNextToken();
         Step = parseExpression();
         if (!Step)
            return nullptr;
      }
      
      if (curToken_ != tok_in)
         return errorP("expected 'in' after for");
      
      getNextToken();
      
      auto Body = parseExpression();
      if (!Body)
         return nullptr;
      
      return llvm::make_unique<ForExprAST>(configurator_.getCodeGenerator(),
                                           IdName, std::move(Start),
                                           std::move(End), std::move(Step),
                                           std::move(Body));
      
   }
   
   expression_t Parser::parseVarExpr()
   {
      getNextToken(); // eat the var.
      
      std::vector<std::pair<std::string, expression_t>> variableNames;
      
      // At least one variable name is required.
      if (curToken_ != tok_identifier)
         return errorP("expected identifier after var");
      
      while (1)
      {
         std::string name = lexer_->getId();
         getNextToken();
         
         // Read the optional initializer.
         expression_t init = nullptr;
         if (curToken_ == '=')
         {
            getNextToken(); // eat the '='.
            
            init = parseExpression();
            if (!init)
               return nullptr;
         }
         
         variableNames.push_back(std::make_pair(name, std::move(init)));
         
         // End of var list, exit loop.
         if (curToken_ != ',')
            break;
         
         getNextToken();
         
         if (curToken_ != tok_identifier)
            return errorP("expected identifier list after var");
      }
      
      // At this point, we have to have 'in'.
      if (curToken_ != tok_in)
         return errorP("expected 'in' keyword after 'var'");
      
      //eat in
      getNextToken();
      
      auto body = parseExpression();
      if (!body)
         return nullptr;
      
      return std::make_unique<VarExprAST>(configurator_.getCodeGenerator(),
                                          std::move(variableNames), std::move(body));

   }

   
   ///
   /// Top-Level parsing
   ///
   
   void Parser::handleDefinition()
   {
      if(const auto& parsedDefinition = parseDefinition())
      {
         if( const auto* defintionIR = parsedDefinition->codeGen())
         {
            defintionIR->print(llvm::errs());
            
            //TODO: remove this hack!!
            std::unique_ptr<llvm::Module> module;
            codeGenerator_.getModule(module);
            
            jitCompiler_.addModule(module);
            codeGenerator_.InitializeModuleAndPassManager();

            //jit_->addModule(std::move)
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
         if(const auto* externIR = parsedExtern->codeGen())
         {
            externIR->print(llvm::errs());
            configurator_.getCodeGenerator().addProtypeCache(parsedExtern->getName(), parsedExtern);
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
         if( const auto* topLevelExprIR = parsedTopLevelExpr->codeGen())
         {
            topLevelExprIR->print(llvm::errs());   //dump IR for the function
            
            //evaluation
            std::unique_ptr<llvm::Module> module;
            codeGenerator_.getModule(module);
            
            auto H = jitCompiler_.addModule(module);
            codeGenerator_.InitializeModuleAndPassManager();
            //InitializeModuleAndPassManager();
            
            // Search the JIT for the __anon_expr symbol.
            auto exprSymbol = jitCompiler_.findSymbol("__anon_expr");
            assert(exprSymbol && "Function not found");
            
            // Get the symbol's address and cast it to the right type (takes no
            // arguments, returns a double) so we can call it as a native function.
            double (*FP)() = (double (*)())(intptr_t)cantFail(exprSymbol.getAddress());
            fprintf(stderr, "Evaluated to %f\n", FP());
            
            // Delete the anonymous expression module from the JIT.
            jitCompiler_.removeModule(H);
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
         
         std::cout << "\n\n >>";
         
      }
   }
}
