//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include "ADBSlicer.H"

/*##############################################################################
#                                                                              #
#                                 viewer main                                  #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv) {
   
    ADBSLICER Slicer;
    
    Slicer.LoadFile(argv[1]);

}

