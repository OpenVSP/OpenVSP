//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef BINARYIO_H
#define BINARYIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "utils.H"

#include "START_NAME_SPACE.H"

// Definition of the BINARYIO class

class BINARYIO {

private:

    // Flag to determine if we should byte swap the data we read or write

    int SwapOnRead_;
    int SwapOnWrite_;

    void SwapFloat(float &Word);
    void SwapDouble(double &Word);    
    void SwapInt(int &Word);
    void SwapChar(char &Word);

    void SwapBytes(char *x, int size);

public:

    // Constructor, Destructor

    BINARYIO(void);
   ~BINARYIO(void);

   // Initialization

   void init(void);

   /** Turn on byte swapping for all reads **/

   void TurnByteSwapForReadsOn(void) { SwapOnRead_ = 1; };
   
   /** Turn off byte swapping for all reads **/
   
   void TurnByteSwapForReadsOff(void) { SwapOnRead_ = 0; };

   /** Turn on byte swapping for all writes **/
   
   void TurnByteSwapForWritesOn(void) { SwapOnWrite_ = 1; };
   
   /** Turn off byte swapping for all writes **/
   
   void TurnByteSwapForWritesOff(void) { SwapOnWrite_ = 0; };

   /** Read in an integer **/

   size_t fread(int *Word, int WordSize, int NumWords , FILE *File);
   
   /** Write out an integer **/
   
   size_t fwrite(int *Word, int WordSize, int NumWords , FILE *File);

   /** Read in a float **/

   size_t fread(float *Word, int WordSize, int NumWords , FILE *File);
   
   /** Write out a float **/
   
   size_t fwrite(float *Word, int WordSize, int NumWords , FILE *File);

   /** Read in a double **/

   size_t fread(double *Word, int WordSize, int NumWords , FILE *File);
   
   /** Write out a double **/
   
   size_t fwrite(double *Word, int WordSize, int NumWords , FILE *File);

   /** Read in a char **/

   size_t fread(char *Word, int WordSize, int NumWords , FILE *File);
   
   /** Write out a char **/
   
   size_t fwrite(char *Word, int WordSize, int NumWords , FILE *File);

};

#include "END_NAME_SPACE.H"

#endif
