//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include "ADBSlicer.H"

int GnuPlot = 0;

int main(int argc, char **argv);
void ParseInput(int argc, char *argv[]);

/*##############################################################################
#                                                                              #
#                                 viewer main                                  #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv) {
   
    ADBSLICER Slicer;
    
    ParseInput(argc, argv);
    
    if ( GnuPlot ) {
       
       Slicer.GnuPlot() = 1;
    
       Slicer.LoadFile(argv[2]);
       
    }
    
    else {
    
       Slicer.LoadFile(argv[1]);
   
    }
    
    return 0;

}

/*##############################################################################
#                                                                              #
#                              ParseInput                                      #
#                                                                              #
##############################################################################*/

void ParseInput(int argc, char *argv[])
{

    int i;

    // Parse the input

    i = 1;

    while ( i <= argc - 2 ) {

       if ( strcmp(argv[i],"-gnu") == 0 ) {
        
          GnuPlot = 1;
          
       }  
       
       i++;    
       
    }
    
}
