//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef TAG_LIST_H
#define TAG_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "TagRegion.H"

// Definition of the TAG_LIST class

class TAG_LIST {

private:

    // Function data

    char GeometryName_[1000];
    char TagListName_[1000];

    int NumberOfTagRegions_;

    TAG_REGION *TagRegionList_;

public:

    // Constructor, Destructor, Copy

    TAG_LIST(void);
   ~TAG_LIST(void);
    TAG_LIST(const TAG_LIST &TagList);

    // Initialize

    void init(void);

    // Copy function

    TAG_LIST& operator=(const TAG_LIST &TagList);

    // Sizing function

    void CreateList(char *Name, int NumberOfRegions);

    // Access to tag list name

    char *GeometryName(void) { return GeometryName_; };

    // Access to tag list name

    char *TagListName(void) { return TagListName_; };

    // Access to each tag region

    int NumberOfTagRegions(void) { return NumberOfTagRegions_; };

    TAG_REGION &TagRegion(int i) { return TagRegionList_[i]; };

    // File I/O

    void WriteFile(char *PathName, char *GeometryName);
    void LoadFile(char *PathName, char *GeometryName, char *TagListName);

};

#endif
