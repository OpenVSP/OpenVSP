/*
==============================================================================
|  GLF Library
|  Version 1.4
| 
|  Author: Roman Podobedov
|  Email: romka@ut.ee
|  WEB: http://romka.demonews.com
|  Release Date: 18 May 2001
|  
|  Copyright (C) 2000-2001, Romka Graphics
|  This library is freely distributable without any license or permissions
|  for non-commercial usage. You can use this library in any non-commercial
|  program. In each program, where You use this library You should keep 
|  this header (author name and coordinates)!
|  For commercial usage, please contact me: romka@ut.ee
==============================================================================
*/

#ifndef GLF_H
#define GLF_H

#ifdef __cplusplus
extern "C" {
#endif

/* CONSTANTS */

#define GLF_ERROR           -1
#define GLF_OK               0
#define GLF_YES              1
#define GLF_NO               2

#define GLF_CONSOLE_MESSAGES 10
#define GLF_TEXTURING        11
#define GLF_CONTOURING       12

#define GLF_LEFT_UP          20
#define GLF_LEFT_CENTER      21
#define GLF_LEFT_DOWN        22
#define GLF_CENTER_UP        23
#define GLF_CENTER_CENTER    24
#define GLF_CENTER_DOWN      25
#define GLF_RIGHT_UP         26
#define GLF_RIGHT_CENTER     27
#define GLF_RIGHT_DOWN       28
#define GLF_CENTER           24

#define GLF_LEFT			 1
#define GLF_RIGHT			 2
#define GLF_UP				 3
#define GLF_DOWN			 4

/* Console constants */
#define GLF_CONSOLE_CURSOR   30

/* Library initialization (must be called before any usage of library) */
void glfInit(); 
/* Library closing (must be called after usage of library) */
void glfClose();

/* Font loading/unloading functions */
int	glfLoadFont(char *font_name);			/* Load Vector font to memory */
int	glfLoadBMFFont(char *FName);			/* Load Bitmap font to memory */
int	glfUnloadFont();						/* Unload font from memory */
int glfUnloadBMFFont();						/* Unload current BMF font */
int	glfUnloadFontD(int font_descriptor);	/* Unload font by font_descriptor */
int	glfUnloadBMFFontD(int bmf_descriptor);	/* Unload BMF font by bmf_descriptor */

/* Text drawing functions */
/* --------- Vector Fonts ---------------- */
void glfDrawWiredSymbol(char s);    /* Draw wired symbol    */
void glfDrawWiredString(char *s);   /* Draw wired string    */
void glfDrawSolidSymbol(char s);    /* Draw solid symbol    */
void glfDrawSolidString(char *s);   /* Draw wired string    */
void glfDraw3DWiredSymbol(char s);  /* Draw 3D wired symbol */
void glfDraw3DWiredString(char *s); /* Draw 3D wired string */
void glfDraw3DSolidSymbol(char s);  /* Draw 3D solid symbol */
void glfDraw3DSolidString(char *s); /* Draw 3D solid string */

/* --------- Bitmap Fonts ---------------- */
void glfStartBitmapDrawing();       /* Must be called before bitmap text drawing */
void glfStopBitmapDrawing();        /* Must be called after bitmap text drawing */
void glfDrawBSymbol(char s);        /* Draw one symbol by bitmapped font */
void glfDrawBString(char *s);       /* Draw string by bitmapped font */
void glfDrawBMaskSymbol(char s);	/* Draw one symbol by bitmapped font (using mask) */
void glfDrawBMaskString(char *s);	/* Draw string by bitmapped font (using mask) */


/* Text drawing functions by specified font */
void glfDrawWiredSymbolF(int font_descriptor, char s);
void glfDrawWiredStringF(int font_descriptor, char *s);
void glfDrawSolidSymbolF(int font_descriptor, char s);
void glfDrawSolidStringF(int font_descriptor, char *s);
void glfDraw3DWiredSymbolF(int font_descriptor, char s);
void glfDraw3DWiredStringF(int font_descriptor, char *s);
void glfDraw3DSolidSymbolF(int font_descriptor, char s);
void glfDraw3DSolidStringF(int font_descriptor, char *s);

/* Text control functions */
void glfGetStringBoundsF(int fd, char *s, float *minx, float *miny, float *maxx, float *maxy);
void glfGetStringBounds(char *s, float *minx, float *miny, float *maxx, float *maxy);

void  glfSetSymbolSpace(float sp);   /* Set space between symbols */
float glfGetSymbolSpace();           /* Get space between symbols */
void  glfSetSpaceSize(float sp);     /* Set space size            */
float glfGetSpaceSize();             /* Get current space size    */

void  glfSetSymbolDepth(float dpth); /* Set depth of 3D symbol */
float glfGetSymbolDepth();           /* Get depth of 3D symbol */

int   glfSetCurrentFont(int Font_Descriptor);		/* Set current font */
int   glfSetCurrentBMFFont(int Font_Descriptor);	/* Set current bitmap font */
int   glfGetCurrentFont();							/* Get current font descriptor */
int   glfGetCurrentBMFFont();		                /* Get current bitmap font descriptor */

void  glfSetAnchorPoint(int anchp); /* Set symbol anchor point */
void  glfSetContourColor(float r, float g, float b, float a); /* Contour color */

/* Enable or Disable GLF features */

void glfEnable(int what); /* Enable GLF feature 'what' */
void glfDisable(int what); /* Disable GLF feature 'what' */

/* Console functions */
void glfSetConsoleParam(int width, int height);
int  glfSetConsoleFont(int Font_Descriptor);
void glfConsoleClear();
void glfPrint(char *s, int lenght);
void glfPrintString(char *s);
void glfPrintChar(char s);

void glfConsoleDraw();
void glfSetCursorBlinkRate(int Rate);

/* -------- Centering and direction ------------ */
void glfStringCentering(GLboolean center); /* Set string centering for vector fonts */
GLboolean glfGetStringCentering(); /* Get string centering for vector fonts */

void glfBitmapStringCentering(GLboolean center);  /* Set string centering for bitmap fonts */
GLboolean glfBitmapGetStringCentering();  /* Set string centering for bitmap fonts */

/* String direction for vector font (GLF_LEFT, GLF_RIGHT, GLF_UP, GLF_DOWN) */
/* GLF_LEFT by default */
void glfStringDirection(GLuint direction);
GLuint glfGetStringDirection();

/* -------- Rotating ------------ */
void glfSetRotateAngle(float angle); /* Set rotate angle for vector fonts */
void glfSetBRotateAngle(float angle); /* Set rotate angle for bitmap fonts */



#ifdef __cplusplus
};
#endif

#endif
