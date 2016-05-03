/////////////////////////////////////////////////////////////////////////
// MsgServer.cpp - Store results of file search                        //
//                                                                     //
// Source: Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016   //
// Author: Qi Tan, qtan100@syr.edu                                     //
// Application: OOD Project #4                                         //
/////////////////////////////////////////////////////////////////////////
#include "DataStore.h"
#include <iostream>

#ifdef TEST_DATASTORE

int main()
{
  std::cout << "\n  Testing DataStore";

  DataStore ds;
  ds.save("one");
  ds.save("two");
  ds.save("three");
  DataStore::iterator iter = ds.begin();
  std::cout << "\n  " << (*iter).c_str();

  for (auto item : ds)
  {
    std::cout << "\n  " << item.c_str();
  }
  std::cout << "\n\n";
}

#endif
