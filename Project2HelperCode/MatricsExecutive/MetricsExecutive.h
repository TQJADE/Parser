#ifndef METRICSEXECUTIVE_H
#define METRICSEXECUTIVE_H
/////////////////////////////////////////////////////////////////////
//  MetricsExecutive.h - collecting metrics for all the packages   //
//                        with names that match a specified pattern //
//                  in a directory tree rooted at a specified path. //
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
string path;
path = argv[1];
string pattern;
std::cout <<"the pattern is :"<< pattern<<std::endl;
MetricsExecutive m = MetricsExecutive(path, pattern);
m.Analysis();

Build Process:
==============
Required files
MetricsExecutive.cpp, FileMgr.cpp, FileMgr.h, DataStore.h, DataStore.cpp

Build commands (either one)
- devenv MetricsExecutive.sln

Maintenance History:
====================
version 1.0: 3/10/2016
First created
*/

#include"../FileMgr/FileMgr.h"
#include "../FileMgr/FileSystem.h"
#include "../MatricsAnaylsis/MatricsAnalysis.h"
using namespace std;
class MetricsExecutive
{
private:
	vector<string> currFile;
	vector<string> currDir;
	FileMgr m;
	DataStore ds;
	DataStore dsf;
public:
	MetricsExecutive();
	MetricsExecutive(string path, string pattern) : m(path, ds, dsf) {
		m.addPattern(pattern);
		m.search();
		initialize();
	}
	void initialize()
	{
		for (auto fs : ds)
			currDir.push_back(fs);
		for (auto fsf : dsf)
			currFile.push_back(fsf);
	}
	void Analysis()
	{
		for (int i = 0; i < currDir.size(); i++)
		{
			MatricsAnalysis* m;
			m = new MatricsAnalysis(currDir[i]);
			m->Analysis();
		}
	}
	vector<string> getFile() { return currFile; }
	vector<string> getDir() { return currDir; }
};

#endif

