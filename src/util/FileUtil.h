//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#if !defined(FILE_UTIL__INCLUDED_)
#define FILE_UTIL__INCLUDED_

#include <vector>
#include <string>
using std::vector;
using std::string;

vector< string > ScanFolder( const char* dir_path );
int ScanFolder();

string PathToExe();
string PathToHome();

bool CheckForFile( const string & path, string &file );
#endif

