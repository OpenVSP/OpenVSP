//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "binaryio.H"

/*##############################################################################
#                                                                              #
#                              BINARYIO constructor                            #
#                                                                              #
##############################################################################*/

BINARYIO::BINARYIO(void)
{

    // Use init routine

    init();

}

/*##############################################################################
#                                                                              #
#                                BINARYIO init                                 #
#                                                                              #
##############################################################################*/

void BINARYIO::init(void)
{

    SwapOnRead_ = SwapOnWrite_ = 0;

}


/*##############################################################################
#                                                                              #
#                               BINARYIO destructor                            #
#                                                                              #
##############################################################################*/

BINARYIO::~BINARYIO(void)
{

    SwapOnRead_ = SwapOnWrite_ = 0;


}

/*##############################################################################
#                                                                              #
#                                 BINARYIO fread (int)                         #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fread(int *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // Read the int from the file

    Code = ::fread(Word, WordSize, NumWords, File);

    // If requested, swap bytes

    if ( SwapOnRead_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapInt(*(Word+i));

	   }

    }

    return Code;

}

/*##############################################################################
#                                                                              #
#                              BINARYIO fread (float)                          #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fread(float *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // Read the int from the file

    Code = ::fread(Word, WordSize, NumWords, File);

    // If requested, swap bytes

    if ( SwapOnRead_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapFloat(*(Word+i));

	   }

    }

    return Code;

}

/*##############################################################################
#                                                                              #
#                     BINARYIO fread (VSPAERO_DOUBLE)                          #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fread(VSPAERO_DOUBLE *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // Read the int from the file

    Code = ::fread(Word, WordSize, NumWords, File);

    // If requested, swap bytes

    if ( SwapOnRead_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapDouble(*(Word+i));

	   }

    }

    return Code;

}


/*##############################################################################
#                                                                              #
#                              BINARYIO fread (char)                           #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fread(char *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // Read the char from the file

    Code = ::fread(Word, WordSize, NumWords, File);

    // If requested, swap bytes

    if ( SwapOnRead_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapChar(*(Word+i));

	   }

    }

    return Code;

}

/*##############################################################################
#                                                                              #
#                              BINARYIO fwrite (int)                           #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fwrite(int *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // If requested, swap bytes

    if ( SwapOnWrite_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapInt(*(Word+i));

	   }

    }

    // Write the int to the file

    Code = ::fwrite(Word, WordSize, NumWords, File);

    // Swap bytes back if we touched them

    if ( SwapOnWrite_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapInt(*(Word+i));

	   }

    }

    return Code;

}

/*##############################################################################
#                                                                              #
#                              BINARYIO fwrite (float)                         #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fwrite(float *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // If requested, swap bytes

    if ( SwapOnWrite_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapFloat(*(Word+i));

	   }

    }

    // Write the float to the file

    Code = ::fwrite(Word, WordSize, NumWords, File);

    // Swap bytes back if we touched them

    if ( SwapOnWrite_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapFloat(*(Word+i));

	   }

    }

    return Code;

}

/*##############################################################################
#                                                                              #
#                              BINARYIO fwrite (char)                          #
#                                                                              #
##############################################################################*/

size_t BINARYIO::fwrite(char *Word, int WordSize, int NumWords , FILE *File)
{

    int i;
    size_t Code;

    // If requested, swap bytes

    if ( SwapOnWrite_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapChar(*(Word+i));

	   }

    }

    // Write the char to the file

    Code = ::fwrite(Word, WordSize, NumWords, File);

    // Swap bytes back if we touched them

    if ( SwapOnWrite_ ) {

	   for ( i = 0 ; i < NumWords ; i++ ) {

	      SwapChar(*(Word+i));

	   }

    }

    return Code;

}

/*##############################################################################
#                                                                              #
#                             BINARYIO SwapFloat                               #
#                                                                              #
##############################################################################*/

void BINARYIO::SwapFloat(float &Word)
{

    SwapBytes((char*)(&Word),sizeof(float));

}

/*##############################################################################
#                                                                              #
#                             BINARYIO SwapDouble                              #
#                                                                              #
##############################################################################*/

void BINARYIO::SwapDouble(VSPAERO_DOUBLE &Word)
{

    SwapBytes((char*)(&Word),sizeof(double));

}

/*##############################################################################
#                                                                              #
#                             BINARYIO SwapInt                                 #
#                                                                              #
##############################################################################*/

void BINARYIO::SwapInt(int &Word)
{

    SwapBytes((char*)(&Word),sizeof(int));

}

/*##############################################################################
#                                                                              #
#                             BINARYIO SwapChar                                #
#                                                                              #
##############################################################################*/

void BINARYIO::SwapChar(char &Word)
{

    SwapBytes(&Word,sizeof(char));

}

/*##############################################################################
#                                                                              #
#                             BINARYIO SwapBytes                               #
#                                                                              #
##############################################################################*/

void BINARYIO::SwapBytes(char *x, int size)
{
  unsigned char c;
  unsigned short s;
  unsigned long l;

  switch (size)
  {

	 // Swap 2 bytes

     case 2:
        c = *x;
        *x = *(x+1);
        *(x+1) = c;
        break;

     // Swap two shorts (2-byte words)

     case 4:

        s = *(unsigned short *)x;
        *(unsigned short *)x = *((unsigned short *)x + 1);
        *((unsigned short *)x + 1) = s;
        SwapBytes((char *)x, 2);
        SwapBytes((char *)((unsigned short *)x+1), 2);
        break;

     // Swap two longs (4-bytes words)

     case 8:

        l = *(unsigned long *)x;
        *(unsigned long *)x = *((unsigned long *)x + 1);
        *((unsigned long *)x + 1) = l;
        SwapBytes((char *)x, 4);
        SwapBytes((char *)((unsigned long *)x+1), 4);
        break;

    }

}


