// ramGlWindow.h: interface for the ramGlWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAMGLWINDOW_H__DE0A3A02_76FB_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_RAMGLWINDOW_H__DE0A3A02_76FB_11D7_AC31_0003473A025A__INCLUDED_

#include <stdio.h>

#ifdef WIN32
#include <windows.h>		
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Image.H>
#include <FL/Enumerations.H> 

#include "loadjpg.h"


#include "defines.h"
#include "track_ball.h"
#include "geom.h"
#include "drawBase.h"

class Aircraft;

#define VSP_ONE_WIN			0
#define VSP_FOUR_WIN		1
#define VSP_TWO_HORZ_WIN	2
#define VSP_TWO_VERT_WIN	3

#define VSP_TOP_VIEW			0
#define VSP_FRONT_VIEW			1
#define VSP_LEFT_SIDE_VIEW		2
#define VSP_LEFT_ISO_VIEW		3
#define VSP_BOTTOM_VIEW			4
#define VSP_BACK_VIEW			5
#define VSP_RIGHT_SIDE_VIEW		6
#define VSP_RIGHT_ISO_VIEW		7

#define VSP_CENTER_VIEW			8

#define VSP_NUM_LIGHTS			6

#define NUM_USER_VIEWS	4

extern "C"
{ void sixseries_( const int* ser, const float* t, const float* cli, const float* a ); }

class VirtGlWindow
{
public:
//	enum { CURSOR_NONE, CURSOR_CROSSHAIR, CURSOR_TARGET };
	VirtGlWindow(int x, int y, int w, int h);
	virtual ~VirtGlWindow();

	void writeFile(xmlNodePtr node); 
	void readFile(xmlNodePtr node); 

	void predraw();
	void draw( Aircraft* airPtr );
	void postdraw();
	void resize( int x, int y, int w, int h );

	void show()				{ showFlag = 1; }
	void hide()				{ showFlag = 0; activeFlag = 0; }
	void activate()			{ activeFlag = 1; }
	void deactivate()		{ activeFlag = 0; }
	int  active()			{ return activeFlag; }
	int  getFocus()			{ return focusFlag; }
	void setFocus(int focus);
	int  mousePress( int x, int y );
	void mouseRelease( int x, int y );
	void mouseDrag( int x, int y );
	void mouseMove( int x, int y );
	void saveTrack(int i);
	void loadTrack(int i);

	void setClearColor( double r, double g, double b )	
		{ clearR = (int)r; clearG = (int)g; clearB = (int)b; }

	void setScale( double scale );
	void setCenter( double x, double y, double z )		{ cx = x; cy = y; cz = z;}

	void toggleFastDraw()						{ fastDrawFlag = !fastDrawFlag; }

	track_ball currTrack;

	void setBackColor( int r, int g, int b )	{ clearR = r; clearG = g; clearB = b; }
	int  getBackR()								{ return clearR; }
	int  getBackG()								{ return clearG; }
	int  getBackB()								{ return clearB; }
	void setBackImgFile( const char* fileName ); 
	void setDrawBackImgFlag( int f )			{ drawBackImgFlag = f; }
	int  getDrawBackImgFlag()					{ return drawBackImgFlag; }
	void setBackImgPreserveAspect( int f )		{ backImgPreserveAspect = f; }
	int  getBackImgPreserveAspect()				{ return backImgPreserveAspect; }
	
	vec2d getCursor()							{ return vec2d(cursorx, cursory); }

	void resetBackImgDefaults();
	double backImgScaleW;
	double backImgScaleH;
	double backImgOffsetX;
	double backImgOffsetY;

private:

	double orthoL, orthoR, orthoT, orthoB;
	int showFlag;
	int activeFlag;
	int focusFlag;
	int wx, wy, ww, wh;
	int mousex, mousey;
	float cursorx, cursory;

	int mouseButton1, mouseButton2, mouseButton3;

	double cx, cy, cz;					// Center of Rotation
	bool fastDrawFlag;

	int savedUserViewFlag[NUM_USER_VIEWS];
	track_ball savedTrackVec[NUM_USER_VIEWS];
	vec3d savedRotationCenters[NUM_USER_VIEWS];

	Aircraft* aircraftPtr;

	int clearR, clearG, clearB;
	int drawBackImgFlag;
	JpgData jpgImgData;

	int backImgPreserveAspect;




};

class VspGlWindow : public Fl_Gl_Window 
{
public:
	VspGlWindow(int x, int y, int w, int h);
	virtual ~VspGlWindow();

	void init();
	int handle(int event);
	void resize( int x, int y, int w, int h );

	VirtGlWindow* currVWin;

	enum{ ONE_LARGE, FOUR_UP_L, FOUR_UP_R, FOUR_LOW_L, 
		FOUR_LOW_R, TWO_L, TWO_R, TWO_UP, TWO_LOW, NUM_V_WIN };

	VirtGlWindow* vWin[NUM_V_WIN];

	void writeFile( xmlNodePtr root );
	void readFile( xmlNodePtr  root );

	int processKeyEvent();
	void setWindowLayout( int layout );
	void setView( int view );

	void centerAllViews();

	void setAircraftPtr( Aircraft* airPtr )		{ aircraftPtr = airPtr; }

	void toggleFastDraw();

	void defineLights();

	void setLightFlag( int id, int flag );
	int  getLightFlag( int id );

	void setLightAmb( double val );
	double getLightAmb()				{ return lightAmb; }
	void setLightDiff( double val );
	double getLightDiff()				{ return lightDiff; }
	void setLightSpec( double val );
	double getLightSpec()				{ return lightSpec; }

	void setMouseTracking( int track );
	double getScale()					{ return currVWin->currTrack.get_scale(); }
	vec2d getCursor()					{ return currVWin->getCursor(); }
	void setWindowFocus(int mx, int my);
	void setCursor(Fl_Cursor c);

	void grabWriteScreen( const char* filename );
	void screenGrab();

private:

	GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];

	double lightAmb;
	double lightSpec;
	double lightDiff;
	int lightFlags[VSP_NUM_LIGHTS];

	Aircraft* aircraftPtr;
	int currLayout;
	int focusFlag;
	int clearFlag;

	void draw();
	
	int mouseTrack;
	Fl_Cursor cursorIcon;

	bool triggerScreenGrabFlag;
	Stringc screenGrabFileName;



};

class XSecGlWindow : public Fl_Gl_Window 
{
public:
	XSecGlWindow(int x, int y, int w, int h);
	virtual ~XSecGlWindow();

	void init();
	int handle(int event);
	void resize( int x, int y, int w, int h );

	void setDrawBase( DrawBase* ptr );

	void setBackImg( const JpgData& d )			{ jpgImgData = d; } 
	void setDrawBackImgFlag( int f )			{ drawBackImgFlag = f; }

private:

	void draw();

	int focusFlag;
	DrawBase* drawBasePtr;
	double orthoL, orthoR, orthoT, orthoB;
	int x, y, w, h;

	int drawBackImgFlag;
	JpgData jpgImgData;

};






#endif
