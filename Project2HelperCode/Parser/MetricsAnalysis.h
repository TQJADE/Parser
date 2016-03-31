#ifndef METRICSANALYSIS_H
#define METRICSANALYSIS_H
/////////////////////////////////////////////////////////////////////
//  MetricsAnalysis.h - Evaluate and display the size and          //
//                      complexity of all functions                //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
Evaluate and display the size and complexity of all functions

Public Interface:
=================

Build Process:
==============
Required files
MetricsAnalysis.cpp

Build commands 
- devenv MetricsAnalysis.sln

Maintenance History:
====================
version 1.0: 3/10/2016
First created
*/
#include "../AST/AST.h"

class MetricsAnalysis
{
private:
	AST ASTtree;
public:
	MetricsAnalysis();
	MetricsAnalysis(AST& tree);
	void displayWholeTree();
};
#endif 


