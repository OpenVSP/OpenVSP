//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "TagRegion.H"

/*##############################################################################
#                                                                              #
#                              TAG_REGION constructor                          #
#                                                                              #
##############################################################################*/

TAG_REGION::TAG_REGION(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                TAG_REGION init                               #
#                                                                              #
##############################################################################*/

void TAG_REGION::init(void)
{

    NumberOfTris_ = 0;

    TriList_ = NULL;

}

/*##############################################################################
#                                                                              #
#                                 TAG_REGION Copy                              #
#                                                                              #
##############################################################################*/

TAG_REGION::TAG_REGION(const TAG_REGION &TagRegion)
{

    init();

    // Just use the operator = code

    *this = TagRegion;

}

/*##############################################################################
#                                                                              #
#                               TAG_REGION destructor                          #
#                                                                              #
##############################################################################*/

TAG_REGION::~TAG_REGION(void)
{

    if ( TriList_ != NULL ) delete [] TriList_;

    NumberOfTris_ = 0;

}

/*##############################################################################
#                                                                              #
#                              TAG_REGION operator=                            #
#                                                                              #
##############################################################################*/

TAG_REGION& TAG_REGION::operator=(const TAG_REGION &TagRegion)
{

    int i;

    // Delete any old data

    if ( TriList_ != NULL ) delete [] TriList_;

    NumberOfTris_ = 0;

    // Copy over data

    NumberOfTris_ = TagRegion.NumberOfTris_;

    TriList_ = new int[NumberOfTris_ + 1];

    for ( i = 1 ; i <= NumberOfTris_ ; i++ ) {

	   TriList_[i] = TagRegion.TriList_[i];

    }

    sprintf(GeometryName_,"%s",TagRegion.GeometryName_);

    sprintf(TagRegionName_,"%s",TagRegion.TagRegionName_);

    return *this;

}

/*##############################################################################
#                                                                              #
#                              TAG_REGION CreateRegion                         #
#                                                                              #
##############################################################################*/

void TAG_REGION::CreateRegion(char *GeometryName, char *TagRegionName, int NumTris)
{

    int i;

    // Open the tag region file

    sprintf(GeometryName_,"%s",GeometryName);

    sprintf(TagRegionName_,"%s",TagRegionName);

    // Allocate space for the tri list

    NumberOfTris_ = NumTris;

    TriList_ = new int[NumberOfTris_ + 1];

    // Write out the tri list

    for ( i = 1 ; i <= NumberOfTris_; i++ ) {

       TriList_[i] = 0;

    }

}

/*##############################################################################
#                                                                              #
#                              TAG_REGION WriteFile                            #
#                                                                              #
##############################################################################*/

void TAG_REGION::WriteFile(char *PathName, char *GeometryName)
{

    int i;
    char FileNameWithExtension[FILE_NAME_SIZE];
    FILE *TagRegionFile;

    // Open the tag region file

    sprintf(FileNameWithExtension,"%s/%s.%s.tag",PathName,GeometryName,TagRegionName_);

    if ( (TagRegionFile = fopen(FileNameWithExtension,"w")) == NULL ) {

       printf("Could not open the file: %s for output! \n",FileNameWithExtension);fflush(NULL);

       exit(1);

    }

    // Write out the header ... the number of tris

    fprintf(TagRegionFile,"%d \n",NumberOfTris_);

    // Write out the tri list

    for ( i = 1 ; i <= NumberOfTris_; i++ ) {

       fprintf(TagRegionFile,"%d \n",TriList_[i]);

    }

    fclose(TagRegionFile);

}

/*##############################################################################
#                                                                              #
#                              TAG_REGION load_file                            #
#                                                                              #
##############################################################################*/

void TAG_REGION::LoadFile(char *PathName, char *GeometryName, char *TagRegionName)
{

    int i;
    char FileNameWithExtension[FILE_NAME_SIZE];
    FILE *TagRegionFile;

    // Open the tag region file

    sprintf(GeometryName_,"%s",GeometryName);

    sprintf(TagRegionName_,"%s",TagRegionName);

    sprintf(FileNameWithExtension,"%s/%s.%s.tag",PathName,GeometryName,TagRegionName_);

    if ( (TagRegionFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       printf("Could not open the file: %s for input! \n",FileNameWithExtension);fflush(NULL);

       exit(1);

    }

    // Read in the header ... the number of tris

    fscanf(TagRegionFile,"%d \n",&NumberOfTris_);

//    printf("Loading in data from tag region file: %s ... with %d tris in region. \n",FileNameWithExtension, NumberOfTris_);

    // Allocate space for the tri list

    TriList_ = new int[NumberOfTris_ + 1];

    // Write out the tri list

    for ( i = 1 ; i <= NumberOfTris_; i++ ) {

       fscanf(TagRegionFile,"%d \n",&(TriList_[i]));

    }

    fclose(TagRegionFile);

}







