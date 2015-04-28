//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "TagList.H"

/*##############################################################################
#                                                                              #
#                              TAG_LIST constructor                            #
#                                                                              #
##############################################################################*/

TAG_LIST::TAG_LIST(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                TAG_LIST init                                 #
#                                                                              #
##############################################################################*/

void TAG_LIST::init(void)
{

    NumberOfTagRegions_ = 0;

    TagRegionList_ = NULL;

}

/*##############################################################################
#                                                                              #
#                                 TAG_LIST Copy                                #
#                                                                              #
##############################################################################*/

TAG_LIST::TAG_LIST(const TAG_LIST &TagList)
{

    init();

    // Just use the operator = code

    *this = TagList;

}

/*##############################################################################
#                                                                              #
#                               TAG_LIST destructor                            #
#                                                                              #
##############################################################################*/

TAG_LIST::~TAG_LIST(void)
{

    if ( TagRegionList_ != NULL ) delete [] TagRegionList_;

    NumberOfTagRegions_ = 0;

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST operator=                              #
#                                                                              #
##############################################################################*/

TAG_LIST& TAG_LIST::operator=(const TAG_LIST &TagList)
{

    int i;

    // Delete any old data

    if ( TagRegionList_ != NULL ) delete [] TagRegionList_;

    NumberOfTagRegions_ = 0;

    // Copy over data

    NumberOfTagRegions_ = TagList.NumberOfTagRegions_;

    TagRegionList_ = new TAG_REGION[NumberOfTagRegions_ + 1];

    for ( i = 1 ; i <= NumberOfTagRegions_ ; i++ ) {

	   TagRegionList_[i] = TagList.TagRegionList_[i];

    }

    sprintf(TagListName_,"%s",TagList.TagListName_);

    return *this;

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST CreateList                             #
#                                                                              #
##############################################################################*/

void TAG_LIST::CreateList(char *Name, int NumberOfRegions)
{

    // Save the name

    sprintf(TagListName_,"%s",Name);

    // Create space for just the size of the tag list

    NumberOfTagRegions_ = NumberOfRegions;

    TagRegionList_ = new TAG_REGION[NumberOfTagRegions_ + 1];

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST write_file                             #
#                                                                              #
##############################################################################*/

void TAG_LIST::WriteFile(char *PathName, char *GeometryName)
{

    int i;
    char FileNameWithExtension[FILE_NAME_SIZE];
    FILE *TagListFile;

    // Open the tag list file

    sprintf(FileNameWithExtension,"%s/%s.%s.taglist",PathName,GeometryName,TagListName_);

    if ( (TagListFile = fopen(FileNameWithExtension,"w")) == NULL ) {

       printf("Could not open the file: %s for output! \n",FileNameWithExtension);fflush(NULL);

       exit(1);

    }

    // Write out the taglist file

    for ( i = 1 ; i <= NumberOfTagRegions_; i++ ) {

       fprintf(TagListFile,"%d %s.%s\n",
        TagRegionList_[i].NumberOfTris(),
        GeometryName,
        TagRegionList_[i].TagRegionName());

    }

    fclose(TagListFile);

    // Now write out the tag files

    for ( i = 1 ; i <= NumberOfTagRegions_; i++ ) {

       TagRegionList_[i].WriteFile(PathName, GeometryName);

    }

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST load_file                              #
#                                                                              #
##############################################################################*/

void TAG_LIST::LoadFile(char *PathName, char *GeometryName, char *TagListName)
{

    int i, NumberOfRegions, MaxNumberOfRegions, NumTris;
    int GeometryNameLength, TagRegionNameLength;
    char FileNameWithExtension[FILE_NAME_SIZE], Line[FILE_NAME_SIZE];
    char DumChar[FILE_NAME_SIZE];
    char *next, dot[FILE_NAME_SIZE], *LastDot;
    char TagGeometryName[FILE_NAME_SIZE], TagRegionName[FILE_NAME_SIZE];
    FILE *TagListFile;
    TAG_REGION *TagList;

    // Delimiters

    sprintf(dot,".");

    // Save the geometry name

    sprintf(GeometryName_,"%s",GeometryName);

    // Open the tag region file

    sprintf(TagListName_,"%s",TagListName);

    sprintf(FileNameWithExtension,"%s/%s.%s.taglist",PathName,GeometryName,TagListName_);

    if ( (TagListFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       printf("Could not open the file: %s for input! \n",FileNameWithExtension);fflush(NULL);

       exit(1);

    }

    // Allocate space for tag list... guess at number in list, we may need
    // to reallocate space later...

    MaxNumberOfRegions = 10000;

    TagList = new TAG_REGION[MaxNumberOfRegions + 1];

    // Read in the the tag regions one at a time... until EOF

    NumberOfRegions = 0;

    while ( fgets(Line,FILE_NAME_SIZE,TagListFile) != NULL ) {

       sscanf(Line,"%d %s\n",&NumTris,DumChar);

       // The text before the last '.' is the tag region name

       LastDot = strrchr(DumChar,'.');

       GeometryNameLength = LastDot - DumChar;

       TagRegionNameLength = strlen(DumChar) - GeometryNameLength - 1;

       for ( i = 0 ; i < GeometryNameLength ; i++ ) {

	      TagGeometryName[i] = DumChar[i];

	   }

	   TagGeometryName[GeometryNameLength] = '\0';

       for ( i = 0 ; i < TagRegionNameLength ; i++ ) {

	      TagRegionName[i] = DumChar[GeometryNameLength+i+1];

	   }

	   TagRegionName[TagRegionNameLength] = '\0';

       // Open Tag file

       NumberOfRegions++;

       TagList[NumberOfRegions].LoadFile(PathName, TagGeometryName, TagRegionName);


    }

    fclose(TagListFile);

    // Create space for just the size of the tag list

    NumberOfTagRegions_ = NumberOfRegions;

    TagRegionList_ = new TAG_REGION[NumberOfTagRegions_ + 1];

    for ( i = 1 ; i <= NumberOfTagRegions_ ; i++ ) {

	   TagRegionList_[i] = TagList[i];

    }

    delete [] TagList;

}







