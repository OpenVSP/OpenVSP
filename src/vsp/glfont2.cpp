//*******************************************************************
//glfont2.cpp -- glFont Version 2.0 implementation
//Copyright (c) 1998-2002 Brad Fish
//See glfont.html for terms of use
//May 14, 2002
//*******************************************************************

//STL headers
#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

//glFont header
#include "glfont2.h"
using namespace glfont;

#include "basic_font.h"

//*******************************************************************
//GLFont Class Implementation
//*******************************************************************
GLFont::GLFont ()
{
	//Initialize header to safe state
	header.tex = -1;
	header.tex_width = 0;
	header.tex_height = 0;
	header.start_char = 0;
	header.end_char = 0;
	header.chars = NULL;
}
//*******************************************************************
GLFont::~GLFont ()
{
	//Destroy the font
	Destroy();
}
//*******************************************************************
bool GLFont::Create (int tex)
{
	int num_chars, num_tex_bytes;
	char *tex_bytes;

	//Destroy the old font if there was one, just to be safe
	Destroy();

	// Wash basic_font through string to ignore null characters.
	string s( basic_font, sizeof( basic_font ) );
	stringstream input( s );

	if (!input)
		return false;

	//Read the header from file
	input.read((char *)&header, sizeof(header) - (sizeof(void*) - 4));
	header.tex = tex;

	//Allocate space for character array
	num_chars = header.end_char - header.start_char + 1;
	if ((header.chars = new GLFontChar[num_chars]) == NULL)
		return false;

	//Read character array
	input.read((char *)header.chars, sizeof(GLFontChar) *
		num_chars);

	//Read texture pixel data
	num_tex_bytes = header.tex_width * header.tex_height * 2;
	tex_bytes = new char[num_tex_bytes];
	input.read(tex_bytes, num_tex_bytes);

	//Create OpenGL texture
	glBindTexture(GL_TEXTURE_2D, tex);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 2, header.tex_width,
		header.tex_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
		(void *)tex_bytes);

	//Free texture pixels memory
	delete[] tex_bytes;

//	//Uncomment to print out the data structure. Useful for finding differences between systems
//	cout << "Start: " << header.start_char << ", End: " << header.end_char
//			<< ", Chars: " << header.chars << endl;
//	cout << "Height: " << header.tex_height << ", Width: " << header.tex_width
//			<< endl;
//	//Read character array
//	for (int i = header.start_char; i < header.end_char; i++) {
//		GLFontChar c = header.chars[i - header.start_char];
//		cout << "Char: " << i << ", dx: " << c.dx << ", dy: " << c.dy << endl;
//		cout << "ty1: " << c.ty1 << ", ty2: " << c.ty2 << ", tx1: " << c.tx1
//				<< ", tx2: " << c.tx2 << endl;
//	}

	//Return successfully
	return true;
}
//*******************************************************************
void GLFont::Destroy (void)
{
	//Delete the character array if necessary
	if (header.chars)
	{
		delete[] header.chars;
		header.chars = NULL;
	}
}
//*******************************************************************
void GLFont::GetTexSize (std::pair<int, int> *size)
{
	//Retrieve texture size
	size->first = header.tex_width;
	size->second = header.tex_height;
}
//*******************************************************************
int GLFont::GetTexWidth (void)
{
	//Return texture width
	return header.tex_width;
}
//*******************************************************************
int GLFont::GetTexHeight (void)
{
	//Return texture height
	return header.tex_height;
}
//*******************************************************************
void GLFont::GetCharInterval (std::pair<int, int> *interval)
{
	//Retrieve character interval
	interval->first = header.start_char;
	interval->second = header.end_char;
}
//*******************************************************************
int GLFont::GetStartChar (void)
{
	//Return start character
	return header.start_char;
}
//*******************************************************************
int GLFont::GetEndChar (void)
{
	//Return end character
	return header.end_char;
}
//*******************************************************************
void GLFont::GetCharSize (int c, std::pair<int, int> *size)
{
	//Make sure character is in range
	if (c < header.start_char || c > header.end_char)
	{
		//Not a valid character, so it obviously has no size
		size->first = 0;
		size->second = 0;
	}
	else
	{
		GLFontChar *glfont_char;

		//Retrieve character size
		glfont_char = &header.chars[c - header.start_char];
		size->first = (int)(glfont_char->dx * header.tex_width);
		size->second = (int)(glfont_char->dy *
			header.tex_height);
	}
}
//*******************************************************************
int GLFont::GetCharWidth (int c)
{
	//Make sure in range
	if (c < header.start_char || c > header.end_char)
		return 0;
	else
	{
		GLFontChar *glfont_char;
		
		//Retrieve character width
		glfont_char = &header.chars[c - header.start_char];
		return (int)(glfont_char->dx * header.tex_width);
	}
}
//*******************************************************************
int GLFont::GetCharHeight (int c)
{
	//Make sure in range
	if (c < header.start_char || c > header.end_char)
		return 0;
	else
	{
		GLFontChar *glfont_char;

		//Retrieve character height
		glfont_char = &header.chars[c - header.start_char];
		return (int)(glfont_char->dy * header.tex_height);
	}
}
//*******************************************************************
void GLFont::Begin (void)
{
	//Bind to font texture
	glBindTexture(GL_TEXTURE_2D, header.tex);
}
//*******************************************************************

//End of file

