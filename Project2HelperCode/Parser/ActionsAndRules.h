#ifndef ACTIONSANDRULES_H
#define ACTIONSANDRULES_H
/////////////////////////////////////////////////////////////////////
//  ActionsAndRules.h - declares new parsing rules and actions     //
//  ver 2.1                                                        //
//  Language:      Visual C++ 2008, SP1                            //
//  Platform:      Dell Precision T7400, Vista Ultimate SP1        //
//  Application:   Prototype for CSE687 Pr1, Sp09                  //
//  Author:        Jim Fawcett, CST 4-187, Syracuse University     //
//                 (315) 443-3948, jfawcett@twcny.rr.com           //
/////////////////////////////////////////////////////////////////////
/*
  Module Operations: 
  ==================
  This module defines several action classes.  Its classes provide 
  specialized services needed for specific applications.  The modules
  Parser, SemiExpression, and Tokenizer, are intended to be reusable
  without change.  This module provides a place to put extensions of
  these facilities and is not expected to be reusable. 

  Public Interface:
  =================
  Toker t(someFile);              // create tokenizer instance
  SemiExp se(&t);                 // create a SemiExp attached to tokenizer
  Parser parser(se);              // now we have a parser
  Rule1 r1;                       // create instance of a derived Rule class
  Action1 a1;                     // create a derived action
  r1.addAction(&a1);              // register action with the rule
  parser.addRule(&r1);            // register rule with parser
  while(se.getSemiExp())          // get semi-expression
    parser.parse();               //   and parse it

  Build Process:
  ==============
  Required files
    - Parser.h, Parser.cpp, ScopeStack.h, ScopeStack.cpp,
      ActionsAndRules.h, ActionsAndRules.cpp, ConfigureParser.cpp,
      ItokCollection.h, SemiExpression.h, SemiExpression.cpp, tokenizer.h, tokenizer.cpp
  Build commands (either one)
    - devenv CodeAnalysis.sln
    - cl /EHsc /DTEST_PARSER parser.cpp ActionsAndRules.cpp \
         semiexpression.cpp tokenizer.cpp /link setargv.obj

  Maintenance History:
  ====================
  ver 2.1 : 15 Feb 16
  - small functional change to a few of the actions changes display strategy
  - preface the (new) Toker and SemiExp with Scanner namespace
  ver 2.0 : 01 Jun 11
  - added processing on way to building strong code analyzer
  ver 1.1 : 17 Jan 09
  - changed to accept a pointer to interfaced ITokCollection instead
    of a SemiExpression
  ver 1.0 : 12 Jan 06
  - first release

*/
//
#include <queue>
#include <string>
#include <sstream>
#include "Parser.h"
#include "../SemiExp/itokcollection.h"
#include "../ScopeStack/ScopeStack.h"
#include "../Tokenizer/Tokenizer.h"
#include "../SemiExp/SemiExp.h"
#include "../AST/AST.h"
#include <iostream>

///////////////////////////////////////////////////////////////
// ScopeStack element is application specific
/* ToDo:
 * - chanage lineCount to two fields: lineCountStart and lineCountEnd
 */


///////////////////////////////////////////////////////////////
// Repository instance is used to share resources
// among all actions.
/*
 * ToDo:
 * - add AST Node class
 * - provide field to hold root of AST
 * - provide interface to access AST
 * - provide helper functions to build AST, perhaps in Tree class
 */
class Repository  // application specific
{
  ScopeStack<element> stack;
  Scanner::Toker* p_Toker;
  AST tree;
public:
  Repository(Scanner::Toker* pToker)
  {
    p_Toker = pToker;

  }
  ScopeStack<element>& scopeStack()
  {
    return stack;
  }
  Scanner::Toker* Toker()
  {
    return p_Toker;
  }
  size_t lineCount()
  {
    return (size_t)(p_Toker->currentLineCount());
  }
  AST& ast()
  {
	  return tree;
  }

};

///////////////////////////////////////////////////////////////
// rule to detect beginning of anonymous scope

class BeginningOfScope : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("{") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to handle scope stack at end of scope

class HandlePush : public IAction
{
  Repository* p_Repos;
public:
  HandlePush(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    //std::cout << "\n--BeginningOfScope rule";
    element elem;
    elem.type = "unknown";
    elem.name = "anonymous";
    elem.lineCount = p_Repos->lineCount();
    p_Repos->scopeStack().push(elem);
	p_Repos->ast().setElement(elem.type, elem.name, elem.lineCount);
	p_Repos->ast().insertElement();
  }
};

///////////////////////////////////////////////////////////////
// rule to detect end of scope

class EndOfScope : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("}") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to handle scope stack at end of scope

class HandlePop : public IAction
{
  Repository* p_Repos;
public:
  HandlePop(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    //std::cout << "\n--EndOfScope rule";
    if(p_Repos->scopeStack().size() == 0)
      return;
    element elem = p_Repos->scopeStack().pop();
	p_Repos->ast().changeElement(elem.type,elem.name, p_Repos->lineCount() - elem.lineCount+1);
	p_Repos->ast().getParent();
	
  }
};

///////////////////////////////////////////////////////////////
// rule to detect preprocessor statements

class PreprocStatement : public IRule
{
public:
  bool doTest(ITokCollection*& pTc)
  {
    if(pTc->find("#") < pTc->length())
    {
      doActions(pTc);
      return true;
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to print preprocessor statement to console

class PrintPreproc : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
    std::cout << "\n\n  Preproc Stmt: " << pTc->show().c_str();
  }
};

///////////////////////////////////////////////////////////////
// rule to detect non-static function definitions

class FunctionDefinition : public IRule
{
public:
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch", "static"};
    for(int i=0; i<6; ++i)
      if(tok == keys[i])
        return true;
    return false;
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& tc = *pTc;
    if(tc[tc.length()-1] == "{")
    {
      size_t len = tc.find("(");
      if(tc[0] != "static" &&len < tc.length() && !isSpecialKeyWord(tc[len-1]))
      {
        doActions(pTc);
        return true;
      }
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
//rule to detect static function definitions

class StaticFunctionDefinition : public IRule
{
public:
	bool isSpecialKeyWord(const std::string& tok)
	{
		const static std::string keys[]
			= { "for", "while", "switch", "if", "catch"};
		for (int i = 0; i<5; ++i)
			if (tok == keys[i])
				return true;
		return false;
	}

	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			size_t len = tc.find("(");
			size_t lenstatic = tc.find("static");
			if (len < tc.length() && !isSpecialKeyWord(tc[len - 1]) && lenstatic < tc.length())
			{
				doActions(pTc);
				return true;
			}
		}
		return true;
	}
};
///////////////////////////////////////////////////////////////
//rule to detect scope
class ScopeDefinition : public IRule
{
	
public:
	bool doTest(ITokCollection*& pTc)
	{
		ITokCollection& tc = *pTc;
		if (tc[tc.length() - 1] == "{")
		{
			const static std::string keys[] = { "for","while","switch","if","catch","static","namespace" };
			
			for (int i = 0; i < 7; ++i)
			{
				size_t len = tc.find(keys[i]);
				if (len < tc.length())
				{
					doActions(pTc);
					std::cout << "Requriement 4&5 IRule and Scope Detection" << std::endl;
					return true;
				}
			}
		}
		return true;
	}
};
///////////////////////////////////////////////////////////////
//action to push Scope into the Repository
class PushScope : public IAction {
	Repository* p_Repos;

public:
	PushScope(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		// next statement is now  handled in PrintFunction
		// std::cout << "\n  FunctionDef: " << pTc->show();

		// pop anonymous scope
	    p_Repos->scopeStack().pop();
		std::string name;
		std::string type;
		if ((pTc->find("struct")) < pTc->length())
		{
			type = "struct";
			name = (*pTc)[1];
		}
		else if ((pTc->find("Class")) < pTc->length())
		{
			type = "Class";
			name = (*pTc)[1];
		}
		else 
		{
			type = "scope";
			name = (*pTc)[1];
		}

		std::cout << "Requriement 4&5 IAction and Scope Action" << std::endl;
		// push scope
		element elem;
		elem.type = type;
		elem.name = name;
		elem.lineCount = p_Repos->lineCount();
		p_Repos->scopeStack().push(elem);
	}
};
///////////////////////////////////////////////////////////////
// action to push non static function name onto ScopeStack

class PushFunction : public IAction
{
  Repository* p_Repos;
public:
  PushFunction(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    // next statement is now  handled in PrintFunction
    // std::cout << "\n  FunctionDef: " << pTc->show();

    // pop anonymous scope
	  std::cout << "Requirement 3 : Find Functions" << std::endl;
    // push function scope
    std::string name = (*pTc)[pTc->find("(") - 1];
    element elem;
    elem.type = "NSfunction";
    elem.name = name;
    elem.lineCount = p_Repos->lineCount();
    p_Repos->scopeStack().push(elem);

//	p_Repos->ast().display();
  }
};
///////////////////////////////////////////////////////////////
//action to print Scope
class PrintScope :public IAction {
	Repository* p_Repos;
public:
	PrintScope(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		std::cout << "\n Scope: " << pTc->show().c_str();
	}
};
///////////////////////////////////////////////////////////////
//action to push static function name into ScopeStack

class PushStaticFunction :public IAction
{
	Repository* p_Repos;
public:
	PushStaticFunction(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
	/*	element e = p_Repos->scopeStack().pop();
		p_Repos->ast().changeElement(e.type, e.name, e.lineCount);
		p_Repos->ast().getParent();*/
		// push function scope
		std::string name = (*pTc)[pTc->find("(") - 1];
		element elem;
		elem.type = "Sfunction";
		elem.name = name;
		elem.lineCount = p_Repos->lineCount();
		p_Repos->scopeStack().push(elem);
		p_Repos->ast().setElement(elem.type, elem.name, elem.lineCount);
		p_Repos->ast().insertElement();
	}
};
///////////////////////////////////////////////////////////////
// action to send semi-expression that starts a non-static function def
// to console

class PrintFunction : public IAction
{
  Repository* p_Repos;
public:
  PrintFunction(Repository* pRepos)
  {
    p_Repos = pRepos;
  }
  void doAction(ITokCollection*& pTc)
  {
    std::cout << "\n  Non-Staic FuncDef: " << pTc->show().c_str();
  }
};
////////////////////////////////////////////////////////////////
// action to send semi-expression that starts a static function def
// to console
class PrintStaticFunction :public IAction
{
	Repository* p_Repos;
public:
	PrintStaticFunction(Repository* pRepos)
	{
		p_Repos = pRepos;
	}
	void doAction(ITokCollection*& pTc)
	{
		std::cout << "\n Static FuncDef: " << pTc->show().c_str();
	}
};
///////////////////////////////////////////////////////////////
// action to send signature of a function def to console

class PrettyPrintFunction : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
    pTc->remove("public");
    pTc->remove(":");
    pTc->trimFront();
    int len = (int)(pTc->find(")"));
    std::cout << "\n\n  Pretty Stmt:    ";
    for(int i=0; i<len+1; ++i)
      std::cout << (*pTc)[i] << " ";
    std::cout << "\n";
  }
};


///////////////////////////////////////////////////////////////
// rule to detect declaration

class Declaration : public IRule          // declar ends in semicolon
{                                         // has type, name, modifiers &
public:                                   // initializers.  So eliminate
  bool isModifier(const std::string& tok) // modifiers and initializers.
  {                                       // If you have two things left
    const size_t numKeys = 22;            // its declar else executable.
    const static std::string keys[numKeys] = {
      "const", "extern", "friend", "mutable", "signed", "static",
      "typedef", "typename", "unsigned", "volatile", "&", "*", "std", "::",
      "public", "protected", "private", ":", "typename", "typedef", "++", "--"
    };
    for (int i = 0; i<numKeys; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void condenseTemplateTypes(ITokCollection& tc)
  {
    size_t start = tc.find("<");
    size_t end = tc.find(">");
    if (start >= end || start == 0)
      return;
    else
    {
      if (end == tc.length())
        end = tc.find(">::");
      if (end == tc.length())
        return;
    }
    std::string save = tc[end];
    std::string tok = tc[start - 1];
    for (size_t i = start; i < end + 1; ++i)
      tok += tc[i];
    for (size_t i = start; i < end + 1; ++i)
      tc.remove(start);
    if (save == ">::")
    {
      tok += tc[start + 1];
      tc.remove(start);
    }
    tc[start - 1] = tok;
    //std::cout << "\n  -- " << tc.show();
  }
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch" };
    for (int i = 0; i<5; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void removeInvocationParens(ITokCollection& tc)
  {
    size_t start = tc.find("(");
    size_t end = tc.find(")");
    if (start >= end || end == tc.length() || start == 0)
      return;
    if (isSpecialKeyWord(tc[start - 1]))
      return;
    for (size_t i = start; i < end + 1; ++i)
      tc.remove(start);
    //std::cout << "\n  -- " << tc.show();
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& in = *pTc;
    Scanner::SemiExp tc;
    for (size_t i = 0; i<in.length(); ++i)
      tc.push_back(in[i]);

    if (tc[tc.length() - 1] == ";" && tc.length() > 2)
    {
      //std::cout << "\n  ++ " << tc.show();
      removeInvocationParens(tc);
      condenseTemplateTypes(tc);

      // remove modifiers, comments, newlines, returns, and initializers

      Scanner::SemiExp se;
      for (size_t i = 0; i < tc.length(); ++i)
      {
        if (isModifier(tc[i]))
          continue;
        if (se.isComment(tc[i]) || tc[i] == "\n" || tc[i] == "return")
          continue;
        if (tc[i] == "=" || tc[i] == ";")
          break;
        else
          se.push_back(tc[i]);
      }
      //std::cout << "\n  ** " << se.show();
      if (se.length() == 2)  // type & name, so declaration
      {
        doActions(pTc);
        return true;
      }
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to show declaration

class ShowDeclaration : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
    ITokCollection& tc = *pTc;
    // remove comments
    Scanner::SemiExp se;
    for (size_t i = 0; i<tc.length(); ++i)
      if (!se.isComment(tc[i]))
        se.push_back(tc[i]);
    // show cleaned semiExp
    std::cout << "\n  Declaration: " << se.show();
  }
};

///////////////////////////////////////////////////////////////
// rule to detect executable

class Executable : public IRule           // declar ends in semicolon
{                                         // has type, name, modifiers &
public:                                   // initializers.  So eliminate
  bool isModifier(const std::string& tok) // modifiers and initializers.
  {                                       // If you have two things left
    const size_t numKeys = 22;            // its declar else executable.
    const static std::string keys[numKeys] = {
      "const", "extern", "friend", "mutable", "signed", "static",
      "typedef", "typename", "unsigned", "volatile", "&", "*", "std", "::",
      "public", "protected", "private", ":", "typename", "typedef", "++", "--"
    };
    for (int i = 0; i<numKeys; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void condenseTemplateTypes(ITokCollection& tc)
  {
    size_t start = tc.find("<");
    size_t end = tc.find(">");
    if (start >= end || start == 0)
      return;
    else
    {
      if (end == tc.length())
        end = tc.find(">::");
      if (end == tc.length())
        return;
    }
    std::string save = tc[end];
    std::string tok = tc[start - 1];
    for (size_t i = start; i < end+1; ++i)
      tok += tc[i];
    for (size_t i = start; i < end+1; ++i)
      tc.remove(start);
    if (save == ">::")
    {
      tok += tc[start + 1];
      tc.remove(start);
    }
    tc[start - 1] = tok;
    //std::cout << "\n  -- " << tc.show();
  }
  
  bool isSpecialKeyWord(const std::string& tok)
  {
    const static std::string keys[]
      = { "for", "while", "switch", "if", "catch" };
    for (int i = 0; i<5; ++i)
    if (tok == keys[i])
      return true;
    return false;
  }
  void removeInvocationParens(ITokCollection& tc)
  {
    size_t start = tc.find("(");
    size_t end = tc.find(")");
    if (start >= end || end == tc.length() || start == 0)
      return;
    if (isSpecialKeyWord(tc[start - 1]))
      return;
    for (size_t i = start; i < end + 1; ++i)
      tc.remove(start);
    //std::cout << "\n  -- " << tc.show();
  }
  bool doTest(ITokCollection*& pTc)
  {
    ITokCollection& in = *pTc;
    Scanner::SemiExp tc;
    for (size_t i = 0; i<in.length(); ++i)
      tc.push_back(in[i]);

    if (tc[tc.length() - 1] == ";" && tc.length() > 2)
    {
      //std::cout << "\n  ++ " << tc.show();
      removeInvocationParens(tc);
      condenseTemplateTypes(tc);

      // remove modifiers, comments, newlines, returns, and initializers

      Scanner::SemiExp se;
      for (size_t i = 0; i < tc.length(); ++i)
      {
        if (isModifier(tc[i]))
          continue;
        if (se.isComment(tc[i]) || tc[i] == "\n" || tc[i] == "return")
          continue;
        if (tc[i] == "=" || tc[i] == ";")
          break;
        else
          se.push_back(tc[i]);
      }
      //std::cout << "\n  ** " << se.show();
      if (se.length() != 2)  // not a declaration
      {
        doActions(pTc);
        return true;
      }
    }
    return true;
  }
};

///////////////////////////////////////////////////////////////
// action to show executable

class ShowExecutable : public IAction
{
public:
  void doAction(ITokCollection*& pTc)
  {
    ITokCollection& tc = *pTc;
    // remove comments
    Scanner::SemiExp se;
    for (size_t i = 0; i < tc.length(); ++i)
    {
      if (!se.isComment(tc[i]))
        se.push_back(tc[i]);
    }
    // show cleaned semiExp
    std::cout << "\n  Executable: " << se.show();
  }
};

#endif