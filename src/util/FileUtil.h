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
#include <filesystem>
namespace fs = std::filesystem;

vector< string > ScanFolder( const char* dir_path );
int ScanFolder();

string PathToExe();
string PathToHome();
string PathToCWD();

bool CheckForFile( const string & path, const string &file );
bool FileExist( const string & file );
int WaitForFile( const string &filename );
int WaitForFiles( const vector < string > & fnames );
string GetFilename( const string &pathfile );
string GetBasename( const string &fname );
void GetPathFile( const string &pathfile, string &path, string &file );

void AppendFile_BtoA( FILE* fpa, FILE* fpb );

void EnforceFilter( string &in, const string & filter );

std::vector< fs::path > get_files_matching_pattern( const std::string& full_pattern );

std::vector< fs::path > get_files_matching_pattern( const fs::path& directory, const std::string& pattern );

void remove_files( const std::vector< fs::path > &file_list );


#endif

