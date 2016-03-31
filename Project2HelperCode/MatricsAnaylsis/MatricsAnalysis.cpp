/////////////////////////////////////////////////////////////////////
//  MetricsAnalysis.cpp - cevaluating and displaying the size and  //
//  complexity of all global functions, static member functions,  //
//  and non-static member functions in each of a set of specified //
//                                                     packages. . //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
#include "MatricsAnalysis.h"
#include <iostream>
#include <string>

MatricsAnalysis::MatricsAnalysis()
{
	pParser = configure.Build();
}

MatricsAnalysis::MatricsAnalysis(string path)
{
	pParser = configure.Build();
	filePath = path;
}

void MatricsAnalysis::Analysis()
{
	AST* tree = &configure.getTree();
	try
	{
		if (pParser)
		{
			if (!configure.Attach(filePath))
			{
				std::cout << "\n  could not open file " << filePath << std::endl;
			}
		}
		else
		{
			std::cout << "\n\n  Parser not built\n\n";
		}
		while (pParser->next())
			pParser->parse();
		std::cout << "\n";
	}
	catch (std::exception& e)
	{
			std::cout << "\n\n    " << e.what() << "\n\n";
	}
	std::cout << "Requirement 8 : evaluating and displaying the size and complexity"<<std::endl;
	tree->display();

}
#ifdef TEST_FILEMGR
   int main()
   {
	   string path = "C:/Users/Qi Tan/Downloads/MatricsAnalysis.cpp";
	   MatricsAnalysis* m;
	   m = new MatricsAnalysis(path);
	   m->Analysis();
   }
#endif

