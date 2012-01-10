//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// materialMgr.cpp: implementation of the materialMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "textureMgr.h"
#include "loadjpg.h"

#ifndef WIN32
#include <limits.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Texture::Texture()
{
	name = "Default";
	texID = 0;
}

bool Texture::readFile( const char* fileName, int type )
{
#ifdef WIN32
	char full[_MAX_PATH];
	_fullpath( full, fileName, _MAX_PATH );
#else
	char full[PATH_MAX];
	realpath( fileName, full );
#endif

	if ( type == TEX_TYPE_TGA )
	{
		TexData tgaimg;
		tgaimg.imageData = NULL;
		if ( loadTGA( &tgaimg, full ) == false )
		{
			if ( tgaimg.imageData )
				free ( tgaimg.imageData );
			tgaimg.imageData = NULL;

			return false;
		}
		width  = tgaimg.width;
		height = tgaimg.height;
		glGenTextures(1, &texID);			
		glBindTexture(GL_TEXTURE_2D, texID);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		GLfloat rgba[] = {1.0, 1.0, 1.0, 0.0};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgba);

		//==== Clamp For Now ====//
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );


		if ( tgaimg.type == GL_RGBA )
 		  glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tgaimg.imageData);
		else
		  glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tgaimg.imageData);
 
		if ( tgaimg.imageData )
			free ( tgaimg.imageData );
		tgaimg.imageData = NULL;

		return true;
	}
	else if ( type == TEX_TYPE_JPG )
	{
		JpgData jpgdata = loadJPEG( full );

		if ( !jpgdata.data )
			return false;

		width  = jpgdata.w;
		height = jpgdata.h;
		glGenTextures(1, &texID);			
		glBindTexture(GL_TEXTURE_2D, texID);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		GLfloat rgba[] = {1.0, 1.0, 1.0, 0.0};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgba);

		//==== Clamp For Now ====//
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, jpgdata.data);
 
		deleteJPEG( jpgdata );

		return true;
	}

	return false;
}


bool Texture::loadTGA(TexData * texture, const char * filename)				
{
	TGAHeader tgaheader;											// TGA header
	TGA tga;														// TGA image info

	FILE * fTGA;													// File pointer to texture file
	fTGA = fopen(filename, "rb");									// Open file for reading
	GLuint pixelcount;
	GLuint currentpixel;											
	GLuint currentbyte;												
	
	if(fTGA == NULL)												// If it didn't open....
		return false;														

	if(fread(&tgaheader, sizeof(TGAHeader), 1, fTGA) == 0)					// Attempt to read 12 byte header from file
	{
		if(fTGA != NULL)													// Check to seeiffile is still open
		{
			fclose(fTGA);													// If it is, close it
		}
		return false;														
	}

	switch( tgaheader.datatypecode )
	{
	case 1:	case 2:	case 9:	case 10:
		break;
	default:
		fclose(fTGA);
		return false;																
	}

	texture->width  = tgaheader.width;
	texture->height = tgaheader.height;
	texture->bpp	= tgaheader.bitsperpixel;
	
	if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32) && (texture->bpp !=8)))
	{
		if(fTGA != NULL)													// Check if file is still open
		{
			fclose(fTGA);													// If so, close it
		}
		return false;														// Return failed
	}

	if(texture->bpp == 24)													//If the BPP of the image is 24...
		texture->type	= GL_RGB;											// Set Image type to GL_RGB
	else if(texture->bpp == 32)																	// Else if its 32 BPP
		texture->type	= GL_RGBA;											// Set image type to GL_RGBA
	else if(texture->bpp == 8)																	// Else if its 32 BPP
		texture->type	= GL_LUMINANCE;										// Set image type to GL_LUMINANCE

	tga.bytesPerPixel	= (tgaheader.bitsperpixel / 8);						// Compute the number of BYTES per pixel
	tga.imageSize		= (tga.bytesPerPixel * tgaheader.width * tgaheader.height);	// Compute the total amout ofmemory needed to store data
	texture->imageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory
		
	if(texture->imageData == NULL)											// If no space was allocated
	{
		fclose(fTGA);														// Close the file
		return false;														// Return failed
	}

	if( tgaheader.idlength > 0 )
	{
		if(fread(texture->imageData, 1, tgaheader.idlength, fTGA) != (unsigned)tgaheader.idlength)	// Attempt to read id tag
		{
			if(texture->imageData != NULL)										// If imagedata has data in it
			{
				free(texture->imageData);										// Delete data from memory
				texture->imageData = NULL;
			}
			fclose(fTGA);														// Close file
			return false;														// Return failed
		}
	}
	
	tga.temp = (GLuint) tgaheader.colourmapdepth / 8
			 * (GLuint)(tgaheader.hi_colourmaplength * 256 + tgaheader.lo_colourmaplength);
			 
	if( tga.temp )
	{
		if(fread(texture->imageData, 1, tga.temp, fTGA) != tga.temp)	// Attempt to read id tag
		{
			if(texture->imageData != NULL)										// If imagedata has data in it
			{
				free(texture->imageData);										// Delete data from memory
				texture->imageData = NULL;
			}
			fclose(fTGA);														// Close file
			return false;														// Return failed
		}
	}

	if( !(tgaheader.datatypecode & 8) )		// uncompressed
	{
		if(fread(texture->imageData, 1, tga.imageSize, fTGA) != tga.imageSize)	// Attempt to read image data
		{
			if(texture->imageData != NULL)										// If imagedata has data in it
			{
				free(texture->imageData);										// Delete data from memory
				texture->imageData = NULL;
			}
			fclose(fTGA);														// Close file
			return false;														// Return failed
		}
	} else
	{								// compressed

		pixelcount	= tgaheader.height * tgaheader.width;							// Nuber of pixels in the image
		currentpixel= 0;												// Current pixel being read
		currentbyte	= 0;												// Current byte 
		
		do
		{
			GLubyte chunkheader = 0;											// Storage for "chunk" header
	
			if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)				// Read in the 1 byte header
			{
				if(fTGA != NULL)												// If file is open
				{
					fclose(fTGA);												// Close file
				}
				if(texture->imageData != NULL)									// If there is stored image data
				{
					free(texture->imageData);									// Delete image data
				}
				return false;													// Return failed
			}
	
			if(chunkheader < 128)												// If the ehader is < 128, it means the that is the number of RAW color packets minus 1
			{																	// that follow the header
				chunkheader++;													// add 1 to get number of following color values
				for(short counter = 0; counter < chunkheader; counter++)		// Read RAW color values
				{
					currentpixel++;															// Increase current pixel by 1
					if(currentpixel > pixelcount)											// Make sure we havent read too many pixels
					{	
						if(fTGA != NULL)													// If there is a file open
						{
							fclose(fTGA);													// Close file
						}	
	
						if(texture->imageData != NULL)										// If there is Image data
						{
							free(texture->imageData);										// delete it
							texture->imageData = NULL;
						}
	
						return false;														// Return failed
					}

					if(fread(&texture->imageData[currentbyte], 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel) // Try to read 1 pixel
					{	
						if(fTGA != NULL)													// See if file is open
						{
							fclose(fTGA);													// If so, close file
						}
	
						if(texture->imageData != NULL)										// See if there is stored Image data
						{
							free(texture->imageData);										// If so, delete it too
							texture->imageData = NULL;
						}
	
						return false;														// Return failed
					}
											
					currentbyte += tga.bytesPerPixel;										// Increase thecurrent byte by the number of bytes per pixel
				}
			}
			else																			// chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
			{
				chunkheader -= 127;															// Subteact 127 to get rid of the ID bit
				if(fread(&texture->imageData[currentbyte], 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel)		// Attempt to read following color values
				{		
					if(fTGA != NULL)														// If thereis a file open
					{
						fclose(fTGA);														// Close it
					}
	
					if(texture->imageData != NULL)											// If thereis image data
					{
						free(texture->imageData);											// delete it
						texture->imageData = NULL;
					}
	
					return false;															// return failed
				}

				GLuint bookmark=currentbyte;
				for(short counter = 0; counter < chunkheader; counter++)					// copy the color into the image data as many times as dictated 
				{																			// by the header
					currentpixel++;															// Increase pixel count by 1
	
					if(currentpixel > pixelcount)											// Make sure we havent written too many pixels
					{
	
						if(fTGA != NULL)													// If there is a file open
						{
							fclose(fTGA);													// Close file
						}	
	
						if(texture->imageData != NULL)										// If there is Image data
						{
							free(texture->imageData);										// delete it
							texture->imageData = NULL;
						}
	
						return false;														// Return failed
					}

					for(unsigned x=0; x<tga.bytesPerPixel; ++x)									// safe to write to memory
						texture->imageData[currentbyte + x] = texture->imageData[bookmark + x];	// Flip R and B vcolor values around in the process 
						
					currentbyte += tga.bytesPerPixel;										// Increase thecurrent byte by the number of bytes per pixel
				}
			}
		}
		while(currentpixel < pixelcount);													// Loop while there are still pixels left
	}

	if( !tgaheader.colourmaptype )
	{
		// Byte Swapping Optimized By Steve Thomas
		for(GLuint cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel)
		{
			texture->imageData[cswap] ^= texture->imageData[cswap+2] ^=
			texture->imageData[cswap] ^= texture->imageData[cswap+2];
		}
	}
	
	fclose(fTGA);															// Close file
	return true;															// Return success
}

//=======================================================================//
//=======================================================================//
//=======================================================================//	
AppliedTex::AppliedTex()
{
	allSurfFlag = false;		
	surfID = 0;				

	texID = 0;

	u = 0.5;
	w = 0.5;
	scaleu = 1.0;
	scalew = 1.0;

	wrapUFlag = false;
	wrapWFlag = true;

	repeatFlag = false;
	bright = 0.6;
	alpha  = 1.0;

	flipUFlag = false;
	flipWFlag = false;
	reflFlipUFlag = true;
	reflFlipWFlag = false;

}

void AppliedTex::extractName( const char* filename )
{
	Stringc texname = filename;
	texname.change_from_to('/', ' ');
	texname.change_from_to('\\', ' ');
	int nword = texname.count_words();

	Stringc basename = texname.get_word( nword-1 );

	basename.remove_substring(".jpg", 1);
	basename.remove_substring(".jpeg", 1);
	basename.remove_substring(".tga", 1);
	basename.remove_substring(".png", 1);

	nameStr = basename;
}




//=======================================================================//
//=======================================================================//
//=======================================================================//

TextureMgr::TextureMgr()
{

}

TextureMgr::~TextureMgr()
{

}

int TextureMgr::loadTex( const char* name) 
{
	if (name == NULL) return 0;

	int index = strlen(name);
	char c = name[index];

	while (c != '.' && index > 0) {
		c = name[--index];
	}

	int type = Texture::TEX_TYPE_INVALID;
	if (strcmp(name+index, TGA_EXT) == 0) 
		type = Texture::TEX_TYPE_TGA;
	else if ( strcmp(name+index, JPG_EXT) == 0) 
		type = Texture::TEX_TYPE_JPG;
	else if ( strcmp(name+index, JPEG_EXT) == 0) 
		type = Texture::TEX_TYPE_JPG;

	if ( type == Texture::TEX_TYPE_INVALID )	return 0;

	texMapIter = texMap.find( name );

	//==== Check it its already loaded ====//
	if ( texMapIter != texMap.end() )
	{
		Texture* tex = (*texMapIter).second;
		return tex->getTexID();
	}
	else	// Create and load it
	{
		Texture* tex = new Texture;
		tex->readFile( name, type );
		if ( tex->getTexID() > 0 )
			texMap[name] = tex;
		return tex->getTexID();
	}

	return 0;

}

//=======================================================================//
//=======================================================================//
//=======================================================================//

SingleTexMgr::SingleTexMgr()
{
	static TextureMgr* ptr = 0; 

    if (!ptr) 
		ptr = new TextureMgr();

	texMgr = ptr;     
}
