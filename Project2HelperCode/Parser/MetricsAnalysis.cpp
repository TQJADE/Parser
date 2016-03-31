/////////////////////////////////////////////////////////////////////
//  MetricsAnalysis.cpp - MetricsAnalysis implementation           //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
#include "MetricsAnalysis.h"
#include<iostream>
#include<string>

MetricsAnalysis::MetricsAnalysis(AST& tree)
{
	ASTtree = tree;
}

void MetricsAnalysis::displayWholeTree() {
	ASTtree.display();
}

#ifdef TEST_AST
int main()
{
	AST tree;
	tree.setElement("b""a",1);
	tree.insertElement();
	MetricsAnalysis m = MetricsAnalysis(tree);
	m.display();
}
#endif