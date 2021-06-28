//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include "viewerUI.h"

int ForceView = 0;
int TimeStamp = 0;
char TimeLabel[1000];
float BoxSize;

#define VER_MAJOR 6
#define VER_MINOR 2
#define VER_PATCH 0

/*##############################################################################
#                                                                              #
#                                 viewer main                                  #
#                                                                              #
##############################################################################*/

int main(int argc, char **argv) {

    int i;
    char dumchar[80];

    viewerUI *vui = new viewerUI;

    // Initial global objects.

    //  Fl::visual(FL_DOUBLE|FL_INDEX);

    // Parse the input

    char *name = argv[argc-1];

    i = 1;

    while ( i <= argc - 2 ) {

	   if ( strcmp(argv[i],"-forceview") == 0 ) {

          ForceView = 1;

          BoxSize = atof(argv[++i]);

	   }

	   else if ( strcmp(argv[i],"-timestamp") == 0 ) {

          TimeStamp = 1;

          sprintf(TimeLabel,"%s",argv[++i]);

	   }

	   else {
         printf( "VSPVIEWER v.%d.%d.%d --- %s \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__ );
         printf("Please direct questions to David Kinney, David.J.Kinney@nasa.gov \n");
	      printf("Unrecognized option! \n");
	      printf("Usage: \n");
	      printf("vspviewer filename \n");

	   }

       i++;

    }

    if ( argc <= 1 ) {

       printf( "VSPVIEWER v.%d.%d.%d --- %s \n", VER_MAJOR, VER_MINOR, VER_PATCH, __DATE__ );
       printf("Please direct questions to David Kinney, David.J.Kinney@nasa.gov \n");
       printf("Unrecognized option! \n");
       printf("Usage: \n");
       printf("vspviewer filename \n");

       exit(1);

    }

    // Pass a pointer to the vui object to the glviewer object

    vui->glviewer->PassTheVui(vui);
    
    // Pass a pointer to the geometry list box
    
    vui->glviewer->PassThePanelComGeomTagsBrowser(vui->PanelComGeomTagsBrowser);

    // Force viewing box

    if ( ForceView ) vui->glviewer->FixViewBox(BoxSize);

    // Set a time stamp label

    if ( TimeStamp ) vui->glviewer->SetTimeStampLabel(TimeLabel);

    // Load in the geometry and TPS data

    vui->LoadInitialData(name);

    // Strip off all but the first argument of the command line

    argc = 1;

    vui->show(argc, argv);

    return Fl::run();
}

