//*******************************************************************
//glfont2.h -- Header for glfont2.cpp
//Copyright (c) 1998-2002 Brad Fish
//See glfont.html for terms of use
//May 14, 2002
//*******************************************************************

#ifndef GLFONT2_H
#define GLFONT2_H

#pragma warning(disable:4786)

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <string>

#pragma pack(push)
#pragma pack(1)

//*******************************************************************
//GLFont Interface
//*******************************************************************

//glFont namespace
namespace glfont
{
	class GLFont;	
}

//glFont class
class glfont::GLFont
{
private:

	//glFont character structure
	typedef struct
	{
		float dx, dy;
		float tx1, ty1;
		float tx2, ty2;
	} GLFontChar;

	//glFont header structure
	struct
	{
		int tex;
		int tex_width, tex_height;
		int start_char, end_char;
		GLFontChar *chars;
	} header;

public:

	//Constructor
	GLFont ();

	//Destructor
	~GLFont ();

public:

	//Creates the glFont
	bool Create (int tex);

	//Destroys the glFont
	void Destroy (void);

	//Texture size retrieval methods
	void GetTexSize (std::pair<int, int> *size);
	int GetTexWidth (void);
	int GetTexHeight (void);

	//Character interval retrieval methods
	void GetCharInterval (std::pair<int, int> *interval);
	int GetStartChar (void);
	int GetEndChar (void);

	//Character size retrieval methods
	void GetCharSize (int c, std::pair<int, int> *size);
	int GetCharWidth (int c);
	int GetCharHeight (int c);

	//Calculates the size in pixels of a character array
	template<class T> void GetStringSize (const T *text,
		std::pair<int, int> *size)
	{
		const T *i;
		GLFontChar *glfont_char;
		float width;
		
		//Height is the same for now...might change in future
		size->second = (int)(header.chars[header.start_char].dy *
			header.tex_height);

		//Calculate width of string
		width = 0.0F;
		for (i = text; *i != (T)'\0'; i++)
		{
			//Make sure character is in range
			if (*i < header.start_char || *i > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[*i - header.start_char];

			//Get width and height
			width += glfont_char->dx * header.tex_width;		
		}

		//Save width
		size->first = (int)width;
	}
	
	//Template function to calculate size of a std::basic_string
	template<class T> void GetStringSize (
		const std::basic_string<T> &text, std::pair<int, int> *size)
	{
		unsigned int i;
		T *c;
		GLFontChar *glfont_char;
		float width;
		
		//Height is the same for now...might change in future
		size->second = (int)(header.chars[header.start_char].dy *
			header.tex_height);

		//Calculate width of string
		width = 0.0F;
		for (i = 0; i < text.size(); i++)
		{
			//Make sure character is in range
			c = text[i];
			if (c < header.start_char || c > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[c - header.start_char];

			//Get width and height
			width += glfont_char->dx * header.tex_width;		
		}

		//Save width
		size->first = (int)width;
	}

	//Begins text output with this font
	void Begin (void);

	//Template function to output a character array
	template<class T> void DrawString (const T *text, float x,
		float y)
	{
		const T *i;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = text; *i != (T)'\0'; i++)
		{
			//Make sure character is in range
			if (*i < header.start_char || *i > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[*i - header.start_char];

			//Get width and height
			width = glfont_char->dx * header.tex_width;
			height = glfont_char->dy * header.tex_height;
			
			//Specify vertices and texture coordinates
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to draw a std::basic_string
	template<class T> void DrawString (
		const std::basic_string<T> &text, float x, float y)
	{
		unsigned int i;
		T c;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = 0; i < text.size(); i++)
		{
			//Make sure character is in range
			c = text[i];
			if (c < header.start_char || c > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[c - header.start_char];

			//Get width and height
			width = glfont_char->dx * header.tex_width;
			height = glfont_char->dy * header.tex_height;
			
			//Specify vertices and texture coordinates
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to output a scaled character array
	template<class T> void DrawString (const T *text, float scalar,
		float x, float y)
	{
		const T *i;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = text; *i != (T)'\0'; i++)
		{
			//Make sure character is in range
			if (*i < header.start_char || *i > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[*i - header.start_char];

			//Get width and height
			width = (glfont_char->dx * header.tex_width) * scalar;
			height = (glfont_char->dy * header.tex_height) * scalar;
			
			//Specify vertices and texture coordinates
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to output a scaled std::basic_string
	template<class T> void DrawString (
		const std::basic_string<T> &text, float scalar, float x,
		float y)
	{
		unsigned int i;
		T c;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = 0; i < text.size(); i++)
		{
			//Make sure character is in range
			c = text[i];
			if (c < header.start_char || c > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[c - header.start_char];

			//Get width and height
			width = (glfont_char->dx * header.tex_width) * scalar;
			height = (glfont_char->dy * header.tex_height) * scalar;
			
			//Specify vertices and texture coordinates
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to output a colored character array
	template<class T> void DrawString (const T *text, float x,
		float y, const float *top_color,
		const float *bottom_color)
	{
		const T *i;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = text; *i != '\0'; i++)
		{
			//Make sure character is in range
			if (*i < header.start_char || *i > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[*i - header.start_char];

			//Get width and height
			width = glfont_char->dx * header.tex_width;
			height = glfont_char->dy * header.tex_height;
			
			//Specify colors, vertices, and texture coordinates
			glColor3fv(top_color);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
			glColor3fv(bottom_color);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);		
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to output a colored std::basic_string
	template<class T> void DrawString (
		const std::basic_string<T> &text, float x, float y,
		const float *top_color, const float *bottom_color)
	{
		unsigned int i;
		T c;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = 0; i < text.size(); i++)
		{
			//Make sure character is in range
			c = text[i];
			if (c < header.start_char || c > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[c - header.start_char];

			//Get width and height
			width = glfont_char->dx * header.tex_width;
			height = glfont_char->dy * header.tex_height;
			
			//Specify colors, vertices, and texture coordinates
			glColor3fv(top_color);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
			glColor3fv(bottom_color);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);		
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to output a scaled, colored character array
	template<class T> void DrawString (const T *text, float scalar,
		float x, float y, const float *top_color,
		const float *bottom_color)
	{
		const T *i;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = text; *i != '\0'; i++)
		{
			//Make sure character is in range
			if (*i < header.start_char || *i > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[*i - header.start_char];

			//Get width and height
			width = (glfont_char->dx * header.tex_width) * scalar;
			height = (glfont_char->dy * header.tex_height) * scalar;
			
			//Specify colors, vertices, and texture coordinates
			glColor3fv(top_color);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
			glColor3fv(bottom_color);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);		
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}

	//Template function to output a scaled, colored std::basic_string
	template<class T> void DrawString (
		const std::basic_string<T> &text, float scalar, float x,
		float y, const float *top_color, const float *bottom_color)
	{
		unsigned int i;
		T c;
		GLFontChar *glfont_char;
		float width, height;
		
		//Begin rendering quads
		glBegin(GL_QUADS);
		
		//Loop through characters
		for (i = 0; i < text.size(); i++)
		{
			//Make sure character is in range
			c = text[i];
			if (c < header.start_char || c > header.end_char)
				continue;

			//Get pointer to glFont character
			glfont_char = &header.chars[c - header.start_char];

			//Get width and height
			width = (glfont_char->dx * header.tex_width) * scalar;
			height = (glfont_char->dy * header.tex_height) * scalar;
			
			//Specify colors, vertices, and texture coordinates
			glColor3fv(top_color);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty1);
			glVertex3f(x, y, 0.0F);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty1);
			glVertex3f(x + width, y, 0.0F);
			glColor3fv(bottom_color);
			glTexCoord2f(glfont_char->tx2, glfont_char->ty2);
			glVertex3f(x + width, y - height, 0.0F);
			glTexCoord2f(glfont_char->tx1, glfont_char->ty2);
			glVertex3f(x, y - height, 0.0F);		
		
			//Move to next character
			x += width;
		}

		//Stop rendering quads
		glEnd();
	}
};

//*******************************************************************
#pragma pack(pop)

#endif

//End of file

