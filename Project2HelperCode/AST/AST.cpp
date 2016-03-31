/////////////////////////////////////////////////////////////////////
//  AST.cpp - Abstract Syntax Tree Test and Implementation         //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
#include "AST.h"
#include<iostream>
#include<string>

void AST::changeElement(std::string type, std::string name, size_t lineCount)
{
	(*currItr).name = name;
	(*currItr).type = type;
	(*currItr).lineCount = lineCount;
}

//intialize the AST tree
AST::AST()
{
	currelement.type = "Namespace";
	currelement.name = "name";
	currelement.lineCount = 0;
	top = tr.begin();
	currItr = tr.insert(top, currelement);
}

//getElement from the client
void AST::setElement(std::string type, std::string name, size_t lineCount) {
	nextelement.type = type;
	nextelement.name = name;
	nextelement.lineCount = lineCount;
}
//insertChild Element
void AST::insertElement() {
	currItr = tr.append_child(currItr, nextelement);
	currelement = nextelement;
}
//getParent Element;
void AST::getParent()
{
	//currelement = tr.parent(currelement);
	currItr = tr.parent(currItr);
	currelement = *currItr;
}
//calculate node with current position complexity
int AST::calculateComplexity(tree<element>::iterator itr) {
	return (int)tr.size(itr) ;
}

//display the tree and present the type, name ,lineCount and calculateComplexity
void AST::display() {
	std::cout << "\n Requirement 6: an abstract syntax tree outlook" << std::endl;;
	tree<element>::iterator sib = tr.begin();
	tree<element>::iterator_base end = tr.end();
	while (sib != end) {
		for (int i = 0; i < tr.depth(sib); ++i)
			std::cout << " ";
		if ((*sib).type == "Namespace")
			std::cout << (*sib).type << "," << (*sib).name << std::endl;
		else
			std::cout << ((*sib).type) << " " << (*sib).name << " " << (*sib).lineCount <<"  cc:"<<calculateComplexity(sib)<< std::endl;
		++sib;
	}
}

AST::~AST()
{

}
#ifdef TEST_AST
int main()
{
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
}
#endif // TEST_AST

