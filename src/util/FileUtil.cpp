//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "FileUtil.h"
#include "tinydir.h"
#include <stdio.h>


vector< string > ScanFolder( const char* dir_path )
{
    vector< string > file_vec;
    tinydir_dir dir;

    //==== Open Path ====//
    if ( tinydir_open( &dir, dir_path ) == -1 )
    {
        tinydir_close( &dir );
        return file_vec;
    }

    //==== Load Files ====//
    while ( dir.has_next )
    {
        tinydir_file file;
        if ( tinydir_readfile( &dir, &file ) == -1 )
        {
        }
        else if ( file.is_dir )
        {
        }
        else
        {
            file_vec.push_back( file.name );
        }
        tinydir_next( &dir );
    }
    return file_vec;

}

int ScanFolder()
{
    tinydir_dir dir;
    if ( tinydir_open( &dir, "." ) == -1 )
    {
        perror( "Error opening file" );
        goto bail;
    }

    while ( dir.has_next )
    {
        tinydir_file file;
        if ( tinydir_readfile( &dir, &file ) == -1 )
        {
            perror( "Error getting file" );
            goto bail;
        }

        printf( "%s", file.name );
        if ( file.is_dir )
        {
            printf( "/" );
        }
        printf( "\n" );

        tinydir_next( &dir );
    }

bail:
    tinydir_close( &dir );
    return 0;
}


//DIR *dir;
//struct dirent *ent;
//if ((dir = opendir ("c:\\")) != NULL)
//{
//  /* print all the files and directories within directory */
//  while ((ent = readdir (dir)) != NULL)
//  {
//      printf ("%s\n", ent->d_name);
//  }
//  closedir (dir);
//}
//else
//{
//  /* could not open directory */
//  perror ("");
//  return EXIT_FAILURE;
//}
//return 0;


