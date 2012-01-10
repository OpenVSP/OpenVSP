
#ifndef JPG_FILE_READ_H
#define JPG_FILE_READ_H


#include <stdlib.h>
#include <stdio.h>

//#ifdef WIN32
//#include "jpeglibWin32.h"
//#else
#include "jpeglib.h"
//#endif


#ifdef __cplusplus
extern "C"
{
#endif	


typedef struct {
	  int w;
	  int h;
	  int d;
	  unsigned char* data;
} JpgData ;


JpgData loadJPEG( const char* fileName );
void deleteJPEG( JpgData img );

int writeJPEG( char* filename, int w, int h, unsigned char* data );

void shiftImg( JpgData* img, int shift );

#ifdef __cplusplus
}
#endif		


#endif
