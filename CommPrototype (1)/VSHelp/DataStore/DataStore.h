#ifndef DATASTORE_H
#define DATASTORE_H
/////////////////////////////////////////////////////////////////////////
// MsgServer.cpp - Store results of file search                        //
//                                                                     //
// Source: Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016   //
// Author: Qi Tan, qtan100@syr.edu                                     //
// Application: OOD Project #4                                         //
/////////////////////////////////////////////////////////////////////////
/*
* This package is to store files in DS
*/
/*
* Required Files:
*  DataStore.cpp
*
* Build Command:
* ==============
* devenv DataStore.sln /rebuild debug
*
* Maintenance History:
* ====================
* version 1.0 first release 2016/05/01
*/

#include <vector>

class DataStore
{
public:
  using iterator = std::vector<std::string>::iterator;
  void save(const std::string& filespec) { store.push_back(filespec);  }
  void save(const std::string& filename, const std::string& path) {}
  iterator begin() { return store.begin(); }
  iterator end() { return store.end(); }
private:
  std::vector<std::string> store;
};
#endif
