#ifndef INTERFACEAST_H
#define INTERFACEAST_H
/////////////////////////////////////////////////////////////////////
//  interfaceAST.h - package for the AST interface                 //
//  ver 1.0                                                       //
//  Language:      Visual C++                                      //
//  Platform:      Visual Studio                                   //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse Univserity, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
/*
Module Purpose:
===============
interfaceAST is an interface that supports substituion of different types of 
display of the element we get from parser.

Maintenance History:
====================
ver 1.0 : 2016/03/11
- first release
*/
#include <string>
struct interfaceAST {
	virtual void setElement(std::string type, std::string name, size_t lineCount)=0;
	virtual void insertElement()=0;
	virtual void getParent()=0;
	virtual void display()=0;
	virtual ~interfaceAST() {};
};
#endif

