//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef TAG_REGION_H
#define TAG_REGION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "CharSizes.H"

// Definition of the TAG_REGION class

class TAG_REGION {

private:

    // Function data

    char GeometryName_[FILE_NAME_SIZE];

    char TagRegionName_[FILE_NAME_SIZE];

    int NumberOfTris_;

    int *TriList_;

public:

    // Constructor, Destructor, Copy

    TAG_REGION(void);
   ~TAG_REGION(void);
    TAG_REGION(const TAG_REGION &TagRegion);

    // Initialize

    void init(void);

    // Copy function

    TAG_REGION& operator=(const TAG_REGION &TagRegion);

    // Access to geometry name

    char *GeometryName(void) { return GeometryName_; };

    // Access to tag list name

    char *TagRegionName(void) { return TagRegionName_; };

    // Access to each tagged tri

    int NumberOfTris(void) { return NumberOfTris_; };

    int &TagTri(int i) { return TriList_[i]; };

    // Create a new region

    void CreateRegion(char *GeometryName, char *TagRegionName, int NumTris);

    // File I/O

    void WriteFile(char *PathName, char *GeometryName);
    void LoadFile(char *PathName, char *GeometryName, char *TagRegionName);

};

#endif



























