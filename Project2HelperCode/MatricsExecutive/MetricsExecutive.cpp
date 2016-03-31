/////////////////////////////////////////////////////////////////////
//  MatricsExecutive.cpp - collecting metrics for all the packages   //
//                        with names that match a specified pattern //
//                  in a directory tree rooted at a specified path. //
//  ver 1.1                                                        //
//  Language:      Visual C++                                      //
//  Application:   Code Parser with Abstract Syntax Tree           //
//  Author:        Qi Tan, Syracuse University, qtan100@syr.edu    //
/////////////////////////////////////////////////////////////////////
#include "MetricsExecutive.h"
//#ifdef TEST_METRICSEXECUTIVE

using namespace std;

int main(int argc, char* argv[])
{
	string path;
	path = argv[1];
	std::cout << "Requirement 9 :  Dispaly files that match a specified pattern in a directory tree rooted at a specified path." << std::endl;
	std::cout << "the path is :" << path << std::endl;
	string pattern;
	//= "*.cpp";
	pattern = argv[2];
	std::cout <<"the pattern is :"<< pattern<<std::endl;
	MetricsExecutive m = MetricsExecutive(path, pattern);
	//vector<string> test = m.getDir();
	m.Analysis();
}

//#endif // TEST_METRICSEXECUTIVE

