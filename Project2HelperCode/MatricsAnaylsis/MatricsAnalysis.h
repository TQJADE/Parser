#ifndef METRICSANALYSIS_H
#define METRICSANALYSIS_H
/////////////////////////////////////////////////////////////////////
//  MetricsAnalysis.h - cevaluating and displaying the size and    //
//  complexity of all global functions, static member functions,  //
//  and non-static member functions in each of a set of specified //
//                                                     packages. . //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
collecting metrics for all the packages with names
that match a specified pattern in a directory tree rooted at a specified path.

Public Interface:
=================
string path = "C:/Users/Qi Tan/Downloads/MatricsAnalysis.cpp";
MatricsAnalysis* m;
m = new MatricsAnalysis(path);
m->Analysis();

Build Process:
==============
Required files
MetricsAnalysis.cpp, Tokenizer.h, Tokenizer.cpp FileSystem.h, FileSystem.cpp
Utilities.h Utilities.cpp SemiExp.h SemiExp.cpp Parser.h Parser.cpp ActionAndRule.h
ActionAndRule.cpp ConfigureParser.h ConfigureParser.cpp

Build commands (either one)
- devenv MetricsAnalysis.sln

Maintenance History:
====================
version 1.0: 3/10/2016
First created
*/
#include<vector>
#include"../Tokenizer/Tokenizer.h"
#include"../Utilities/Utilities.h"
#include"../SemiExp/SemiExp.h"
#include"../Parser/Parser.h"
#include"../Parser/ActionsAndRules.h"
#include"../Parser/ConfigureParser.h"
using namespace std;
class MatricsAnalysis {
private:
	Parser* pParser;
	string filePath;
	ConfigParseToConsole configure;
public:
	MatricsAnalysis();
	MatricsAnalysis(string path);
	void Analysis();
};
#endif
