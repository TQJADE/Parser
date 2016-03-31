#ifndef FILEMGR_H
#define FILEMGR_H
/////////////////////////////////////////////////////////////////////
// FileMgr.h - find files matching specified patterns              //
//             on a specified path                                 //
// ver 2.0                                                         //
//Author: Qi Tan                                                   //
//Source: Jim Fawcett, Help Session Demo, Spring 2015              //
/////////////////////////////////////////////////////////////////////
/*
* ver 2.0 : 12 Mar 2016
* - fixes bug in directory recursion in find(path)
* - reduces scope of for loop in find for efficiency
* ver 1.0 : 19 Feb 2016
* - first release
*/
#include "FileSystem.h"
#include "../DataStore/DataStore.h"
#include <iostream>

class FileMgr
{
public:
	using iterator = DataStore::iterator;
	using patterns = std::vector<std::string>;

	FileMgr(const std::string& path, DataStore& ds, DataStore& dsf) : path_(path), store_(ds), storeF_(dsf)
	{
		patterns_.push_back("*.*");
	}

	void addPattern(const std::string& patt)
	{
		if (patterns_.size() == 1 && patterns_[0] == "*.*")
			patterns_.pop_back();
		patterns_.push_back(patt);
	}

	void search()
	{
		find(path_);
	}

	void find(const std::string& path);
private:
	std::string path_;
	DataStore& store_;
	patterns patterns_;
	DataStore& storeF_;
};

#endif

