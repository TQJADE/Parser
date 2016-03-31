#ifndef AST_H
#define AST_H
/////////////////////////////////////////////////////////////////////
//  AST.h - declares Abstract Syntax Tree                          //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
Construct abstract syntx tree and develop functions for Metric Analysis and Metric Executive

Public Interface:
=================
std::string name = "namespace";
AST test = AST(name);
test.setElement("1","grandma",0);
test.insertElement();
test.setElement("1", "grandpa",0);
test.getParent();
test.insertElement();
test.setElement("2", "father", 0);
test.insertElement();
test.setElement("3", "I", 0);
test.insertElement();
test.display();

Build Process:
==============
Required files
AST.cpp

Build commands (either one)
- devenv AST.sln

Maintenance History:
====================
version 1.0: 3/10/2016
First created
*/

#include "../AST/tree.hh"
#include <iostream>
#include <string>
#include "../AST/interfaceAST.h"
#include <sstream>
struct element
{
	std::string type;
	std::string name;
	size_t lineCount;
	std::string show()
	{
		std::ostringstream temp;
		temp << "(";
		temp << type;
		temp << ", ";
		temp << name;
		temp << ", ";
		temp << lineCount;
		temp << ")";
		return temp.str();
	}
	std::string getType() { return type; }
	std::string getName() { return name; }
	size_t getLineCount() { return lineCount; }
};

class AST: public interfaceAST
{
public:
	AST();
	~AST();
	void setElement(std::string type, std::string name, size_t lineCount);
	void insertElement();
	void getParent();
	void display();
	int calculateComplexity(tree<element>::iterator itr);
	void changeElement(std::string type, std::string name, size_t lineCount);
private:
	tree<element> tr;
	element currelement;
	tree<element>::iterator top;
	tree<element>::iterator currItr;
	element nextelement;
};


#endif