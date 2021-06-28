//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "TagListGroup.H"

/*##############################################################################
#                                                                              #
#                              TAG_LIST_GROUP constructor                      #
#                                                                              #
##############################################################################*/

TAG_LIST_GROUP::TAG_LIST_GROUP(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                TAG_LIST_GROUP init                           #
#                                                                              #
##############################################################################*/

void TAG_LIST_GROUP::init(void)
{

    NumberOfTagLists_ = 0;

    TagRegionListList_ = 0;

}

/*##############################################################################
#                                                                              #
#                                 TAG_LIST_GROUP Copy                          #
#                                                                              #
##############################################################################*/

TAG_LIST_GROUP::TAG_LIST_GROUP(const TAG_LIST_GROUP &TagListGroup)
{

    init();

    // Just use the operator = code

    *this = TagListGroup;

}

/*##############################################################################
#                                                                              #
#                               TAG_LIST_GROUP destructor                      #
#                                                                              #
##############################################################################*/

TAG_LIST_GROUP::~TAG_LIST_GROUP(void)
{

    if ( TagRegionListList_ != NULL ) delete [] TagRegionListList_;

    NumberOfTagLists_ = 0;

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST_GROUP operator=                        #
#                                                                              #
##############################################################################*/

TAG_LIST_GROUP& TAG_LIST_GROUP::operator=(const TAG_LIST_GROUP &TagListGroup)
{

    int i;

    // Delete any old data

    if ( TagRegionListList_ != NULL ) delete [] TagRegionListList_;

    NumberOfTagLists_ = 0;

    // Copy over data

    NumberOfTagLists_ = TagListGroup.NumberOfTagLists_;

    TagRegionListList_ = new TAG_LIST[NumberOfTagLists_ + 1];

    for ( i = 1 ; i <= NumberOfTagLists_ ; i++ ) {

	   TagRegionListList_[i] = TagListGroup.TagRegionListList_[i];

    }

    sprintf(TagListGroupName_,"%s",TagListGroup.TagListGroupName_);

    return *this;

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST_GROUP CreateGroup                      #
#                                                                              #
##############################################################################*/

void TAG_LIST_GROUP::CreateGroup(char *Name, int NumberOfLists)
{

    // Save the name

    sprintf(TagListGroupName_,"%s",Name);

    // Create space for just the size of the tag list

    NumberOfTagLists_ = NumberOfLists;

    TagRegionListList_ = new TAG_LIST[NumberOfTagLists_ + 1];

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST_GROUP WriteFile                        #
#                                                                              #
##############################################################################*/

void TAG_LIST_GROUP::WriteFile(char *PathName, char *GeometryName)
{

    int i;
    char FileNameWithExtension[FILE_NAME_SIZE];
    FILE *TagListGroupFile;

    // Open the tag list file

    sprintf(FileNameWithExtension,"%s/%s.%s.taglistgroup",PathName,GeometryName,TagListGroupName_);

    if ( (TagListGroupFile = fopen(FileNameWithExtension,"w")) == NULL ) {

       printf("Could not open the file: %s for output! \n",FileNameWithExtension);fflush(NULL);

       exit(1);

    }

    // Write out the taglist file

    for ( i = 1 ; i <= NumberOfTagLists_; i++ ) {

       fprintf(TagListGroupFile,"%s.taglist %s\n",
        GeometryName,
        TagRegionListList_[i].TagListName());

    }

    fclose(TagListGroupFile);

    // Now write out the tag files

    for ( i = 1 ; i <= NumberOfTagLists_; i++ ) {

       TagRegionListList_[i].WriteFile(PathName, GeometryName);

    }

}

/*##############################################################################
#                                                                              #
#                              TAG_LIST_GROUP load_file                        #
#                                                                              #
##############################################################################*/

void TAG_LIST_GROUP::LoadFile(char *PathName, char *GeometryName, char *TagListGroupName)
{

    int i, NumberOfLists, MaxNumberOfLists;
    char FileNameWithExtension[FILE_NAME_SIZE], Line[FILE_NAME_SIZE];
    char DumChar[FILE_NAME_SIZE];
    char *next, dot[FILE_NAME_SIZE];
    FILE *TagListGroupFile;
    TAG_LIST *TagListList;

    // Delimiters

    sprintf(dot,".");

    // Open the tag region file

    sprintf(TagListGroupName_,"%s",TagListGroupName);

    sprintf(FileNameWithExtension,"%s/%s.%s.taglistgroup",PathName,GeometryName,TagListGroupName_);

    if ( (TagListGroupFile = fopen(FileNameWithExtension,"r")) == NULL ) {

       printf("Could not open the file: %s for input! \n",FileNameWithExtension);fflush(NULL);

       exit(1);

    }

    // Allocate space for tag list... guess at number in list, we may need
    // to reallocate space later...

    MaxNumberOfLists = 10000;

    TagListList = new TAG_LIST[MaxNumberOfLists + 1];

    // Read in the the tag regions one at a time... until EOF

    NumberOfLists = 0;

    while ( fgets(Line,FILE_NAME_SIZE,TagListGroupFile) != NULL ) {

       sscanf(Line,"%s\n",DumChar);

       // Open Tag file

       NumberOfLists++;

       TagListList[NumberOfLists].LoadFile(PathName, GeometryName, DumChar);

    }

    fclose(TagListGroupFile);

    // Create space for just the size of the tag list

    NumberOfTagLists_ = NumberOfLists;

    TagRegionListList_ = new TAG_LIST[NumberOfTagLists_ + 1];

    for ( i = 1 ; i <= NumberOfTagLists_ ; i++ ) {

	   TagRegionListList_[i] = TagListList[i];

    }

    delete [] TagListList;

}







