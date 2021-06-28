//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "MergeSort.H"

/*##############################################################################
#                                                                              #
#                              MERGESORT constructor                           #
#                                                                              #
##############################################################################*/

MERGESORT::MERGESORT(void)
{

    // Nothing to do...

}

/*##############################################################################
#                                                                              #
#                              MERGESORT destructor                            #
#                                                                              #
##############################################################################*/

MERGESORT::~MERGESORT(void)
{

    // Nothing to do...

}

/*##############################################################################
#                                                                              #
#                               MERGESORT Copy                                 #
#                                                                              #
##############################################################################*/

MERGESORT::MERGESORT(const MERGESORT &MergeSort)
{

    // Not implemented

    PRINTF("Copy not implemented for MERGESORT class! \n");

    exit(1);

}

/*##############################################################################
#                                                                              #
#                              MERGESORT Sort                                  #
#                                                                              #
##############################################################################*/

int* MERGESORT::Sort(int NumberOfEdges, VSP_EDGE **EdgeList)
{

    int i, *perm, *iperm;

    perm = merge_sort(NumberOfEdges,EdgeList);
    
    iperm = new int[NumberOfEdges + 1];
    
    for ( i = 1 ; i <= NumberOfEdges ; i++ ) {
       
       iperm[perm[i]] = i;
       
    }
    
    delete [] perm;

    return iperm;

}

/*##############################################################################
#                                                                              #
#                              MERGESORT merge_sort                            #
#                                                                              #
# The function re-orders the grid points using Merge-Sort Algorithm.  This is  #
# a very efficient algorithm, taking N*logN (base 2) operations to finish the  #
# sorting process.  The algorithm works by merging already ordered lists into  #
# a new single list.  We start by taking N points and making up N lists of     #
# length 1 (these lists are therefore already sorted).  We then make up N/2    #
# lists of length 2 that are sorted, then N/4 lists of length 4, etc. and so   #
# on until we have a list of length N that is sorted.  If N is odd there is    #
# some extra logic (one more pass of the list sort step).                      #
#                                                                              #
# Original coding: around 1994 - in C                                          #
# Updated to C++ Feb 2000                                                      #
#                                                                              #
##############################################################################*/

int* MERGESORT::merge_sort(int NumberOfEdges, VSP_EDGE **EdgeList)
{

    int *list_1, *list_2, list_length, i;

    // Space for lists needed by merge-sort algorithm

    list_1 = new int[NumberOfEdges + 1];

    list_2 = new int[NumberOfEdges + 1];

    // intialize the lists

    for ( i = 1 ; i <= NumberOfEdges ; i++ ) {

        list_1[i] = i;

        list_2[i] = 0;

    }

    // Start with grid.number_of_nodes() lists of length 1

    list_length = 1;

    // Keep merging longer and longer lists (multiple of 2) until done

    while ( list_length <= NumberOfEdges/2 ) {

          merge_lists(list_1,list_2,list_length,NumberOfEdges,EdgeList);

          list_length = list_length * 2;

          merge_lists(list_2,list_1,list_length,NumberOfEdges,EdgeList);

          list_length = list_length * 2;

    }

    // One more pass if # of pts is odd or a multiple of 2

    if ( list_length < NumberOfEdges ) {

       merge_lists(list_1,list_2,list_length,NumberOfEdges,EdgeList);

       for ( i = 1 ; i <= NumberOfEdges ; i++ ) {

          list_1[list_2[i]] = i;

       }
       
       delete [] list_2;

       return(list_1);

    }

    else {

       for ( i = 1 ; i <= NumberOfEdges ; i++ ) {

          list_2[list_1[i]] = i;

       }
       
       delete [] list_1;

       return(list_2);

    }

}

/*##############################################################################
#                                                                              #
#                              MERGESORT merge_sort                            #
#                                                                              #
# The function is the heart of the merge-sort algorithm. Two list (perhaps of  #
# different size) are given.  Each list is ordered (in increasing x here)      #
# already.  The two lists are then merged such that the final list is also     #
# ordered.                                                                     #
#                                                                              #
# Original coding: around 1994 - in C                                          #
# Updated to C++ Feb 2000                                                      #
#                                                                              #
##############################################################################*/

void MERGESORT::merge_lists(int *list_1, int *list_2, int list_length,
                          int NumberOfEdges, VSP_EDGE **EdgeList)
{

    int list_1_front, list_1_end;
    int list_2_front, list_2_end;
    int new_list_front, i;
    int x_1, x_2;

    // front position of new permutation vector

    new_list_front = 1;

    // front position of list_1

    list_1_front = 1;

    // front position of list_2

    list_2_front = list_length + 1;

    // merge lists while there still are elements in the lists

    while ( list_1_front <= NumberOfEdges ) {

       // position for end of list 1

       list_1_end = list_1_front + list_length;

       // if list 1 goes off end, set to end + 1 to stop next pass

       if ( list_1_end > NumberOfEdges ) {

          list_1_end = NumberOfEdges + 1;

       }

       // otherwise, set end position of list 2

       else {

          list_2_end = list_2_front + list_length;

          // if list 2 goes off end, set to end + 1

          if ( list_2_end > NumberOfEdges ) {

              list_2_end = NumberOfEdges + 1;

          }

          // merge lists while both are not empty

          while ( list_1_front != list_1_end &&
                  list_2_front != list_2_end ) {

             x_1 = EdgeList[list_1[list_1_front]]->VortexEdge();
                           
             x_2 = EdgeList[list_1[list_2_front]]->VortexEdge();

             if ( x_1 <= x_2 ) {

                list_2[new_list_front] = list_1[list_1_front];

                list_1_front++;

                new_list_front++;

             }

             else {

                list_2[new_list_front] = list_1[list_2_front];

                list_2_front++;

                new_list_front++;

             }

          }

       }

       // one and only one list may have some leftover stuff

       if ( list_1_front < list_1_end ) {

          for ( i = list_1_front ; i < list_1_end ; i++ ) {

             list_2[new_list_front] = list_1[i];

             new_list_front++;

          }

          list_1_front = list_1_end;

       }

       else {

          for ( i = list_2_front ; i < list_2_end ; i++ ) {

             list_2[new_list_front] = list_1[i];

             new_list_front++;

          }

          list_2_front = list_2_end;

       }

       // set new positions for fronts of list 1 and 2

       list_1_front = list_2_front;

       list_2_front = list_2_front + list_length;

    }

}



