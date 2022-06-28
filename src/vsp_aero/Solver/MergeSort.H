//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef MERGESORT_H
#define MERGESORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "VSP_Edge.H"
#include "utils.H"

#include "START_NAME_SPACE.H"

// Definition of the CUTHILL_H class

class MERGESORT {

private:

    int* merge_sort(int NumberOfEdges, VSP_EDGE **EdgeList);
    void merge_lists(int *list_1, int *list_2, int list_length, int NumberOfEdges, VSP_EDGE **EdgeList);  

public:

    // Constructor, Destructor, Copy
    
    MERGESORT(void);
   ~MERGESORT(void);
    MERGESORT(const MERGESORT &MergeSort);
    
    // Sort routines
    
    int* Sort(int NumberOfEdges, VSP_EDGE **EdgeList);

};

#include "END_NAME_SPACE.H"

#endif
