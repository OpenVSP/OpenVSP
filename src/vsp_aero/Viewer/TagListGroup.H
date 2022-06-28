//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef TAG_LIST_GROUP_H
#define TAG_LIST_GROUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "TagList.H"
#include "CharSizes.H"

// Definition of the TAG_LIST_GROUP class

class TAG_LIST_GROUP {

private:

    // Function data

    char TagListGroupName_[FILE_NAME_SIZE];

    int NumberOfTagLists_;

    TAG_LIST *TagRegionListList_;

public:

    // Constructor, Destructor, Copy

    TAG_LIST_GROUP(void);
   ~TAG_LIST_GROUP(void);
    TAG_LIST_GROUP(const TAG_LIST_GROUP &TagListGroup);

    // Initialize

    void init(void);

    // Copy function

    TAG_LIST_GROUP& operator=(const TAG_LIST_GROUP &TagListGroup);

    // Sizing function

    void CreateGroup(char *Name, int NumberOfLists);

    // Access to tag list name

    char *TagListGroupName(void) { return TagListGroupName_; };

    // Access to each tag region

    int NumberOfTagLists(void) { return NumberOfTagLists_; };

    TAG_LIST &TagList(int i) { return TagRegionListList_[i]; };

    // File I/O

    void WriteFile(char *PathName, char *GeometryName);
    void LoadFile(char *PathName, char *GeometryName, char *GroupName);

};

#endif
