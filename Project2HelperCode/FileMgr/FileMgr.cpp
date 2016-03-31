/////////////////////////////////////////////////////////////////////
// FileMgr.cpp - find files matching specified patterns            //
//               on a specified path                               //
// ver 2.0                                                         //
// Jim Fawcett, Help Session Demo, Spring 2015                     //
/////////////////////////////////////////////////////////////////////

#include "FileMgr.h"
#include "../DataStore/DataStore.h"
#include <iostream>

void FileMgr::find(const std::string& path)
{

	std::string fpath = FileSystem::Path::getFullFileSpec(path);
	//---------------------------------------------------------
	// The statement above is not necessary, but helps to
	// make the processing clearer.  To show that, comment
	// it out and uncomment the following line.
	//
	// std::string fpath = path;
	//
	// Remove the std::cout statements below after you're 
	// convinced that everything works as it should.
	//---------------------------------------------------------

	std::cout << "\n  ++ " << fpath;
	for (auto patt : patterns_)  // the scope of this for loop was
	{                            // changed to improve efficiency
		std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, patt);
		for (auto f : files)
		{
			std::cout << "\n  --   " << f;
			storeF_.save(f);
			std::string fs = FileSystem::Path::fileSpec(path, f);
			store_.save(fs);
		}
	}
	std::vector<std::string> dirs = FileSystem::Directory::getDirectories(fpath);
	for (auto d : dirs)
	{
		if (d == "." || d == "..")
			continue;
		std::string dpath = fpath + "\\" + d;  // here's the fix
		find(dpath);
	}
}
#ifdef TEST_FILEMGR

int main()
{
	std::cout << "\n  Testing FileMgr";
	std::cout << "\n =================";

	DataStore ds;
	DataStore dsf;
	FileMgr fm("C:/Users/Qi Tan/Downloads/Project2/Project2HelperCode", ds, dsf);
	fm.addPattern("*.cpp");
	//fm.addPattern("*.c*");
	//fm.addPattern("*.partial");
	fm.search();

	std::cout << "\n\n  contents of DataStore";
	std::cout << "\n -----------------------";
	for (auto fs : ds)
	{
		std::cout << "\n  " << fs;
	}
	std::cout << "\n\n";
	return 0;
}
#endif
