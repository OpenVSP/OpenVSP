//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// screenMgr.h: interface for the screenMgr class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREENMGR_H__4D2913C0_85ED_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_SCREENMGR_H__4D2913C0_85ED_11D7_AC31_0003473A025A__INCLUDED_

#include <stdio.h>

#ifdef WIN32
#include <windows.h>		
#endif

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif


#include "mainScreen.h"
#include "exportFileScreen.h"
#include "importFileScreen.h"
#include "compGeomScreen.h"
#include "sliceScreen.h"
#include "backgroundScreen.h"
#include "lightingScreen.h"
#include "viewScreen.h"
#include "scriptScreen.h"
#include "scriptOutScreen.h"
#include "awaveScreen.h"
#include "massPropScreen.h"
#include "aeroRefScreen.h"
#include "aboutScreen.h"
#include "keyHelpScreen.h"

#include "vspScreen.h"
#include "vspGlWindow.h"
#include "geom.h"
#include "aircraft.h"
#include "sliceScreen.h"

class GeomScreen;
class GroupScreen;
class WingScreen;
class FuseScreen;
class FuselageScreen;
class HavocScreen;
class ExtScreen;
class MsWingScreen;
class HwbScreen;
class CurveEditScreen;
class BlankScreen;
class MeshScreen;
class HrmScreen;
class DuctScreen;
class PropScreen;
class EngineScreen;
class StructureScreen;
class CfdMeshScreen;
class FeaStructScreen;
class Parm;
class ParmScreen;
class ParmLinkScreen;
class ParmPickerScreen;
class TextureMgrScreen;
class CabinLayoutScreen;
class SelectFileScreen;

class LabelScreen;
class VorviewScreen;

#define POD_SCREEN		0
#define GEOM_SCREEN		1
#define WING_SCREEN		2
#define FUSE_SCREEN		3
#define HAVOC_SCREEN	4
#define EXT_SCREEN		5
#define MS_WING_SCREEN	6
#define BLANK_SCREEN	7
#define MESH_SCREEN		8
#define DUCT_SCREEN		9
#define PROP_SCREEN		10
#define ENGINE_SCREEN	11
#define HRM_SCREEN	    12
#define STRUCTURE_SCREEN 13
#define XML_SCREEN	    14
#define CFD_MESH_SCREEN 15
#define HWB_SCREEN      16
#define TEXTURE_MGR_SCREEN 17
#define CABIN_LAYOUT_SCREEN 18
#define XSEC_GEOM_SCREEN 19


class ScreenMgr  
{
public:
	VSPDLL ScreenMgr(Aircraft* airPtr);
	VSPDLL virtual ~ScreenMgr();

	VSPDLL void showGui(int argc, char** argv);
	VSPDLL void showGui();
	VSPDLL void hideGui();
	VSPDLL void hideMeshScreen();
	VSPDLL void handleEvents();
	VSPDLL void addScriptLine(const char* str);
	VSPDLL Aircraft* getAircraftPtr();

	void changeGlWindow();
	void menuCB(Fl_Widget* w);

	void update( int screenID );

	VSPDLL void hideGeomScreens();
	void updateGeomScreens();

	void updateBackgroundScreen();

//	virtual void setCursor(double cx, double cy)				{ cursorx = cx; cursory = cy; }
//	virtual vec2d getCursor()			{ return vec2d(cursorx, cursory); }


	VspGlWindow*  getDrawWin()				{ return glWin; }
	PodScreen*    getPodScreen()			{ return podScreen; }
	XsecGeomScreen*  getXsecGeomScreen()	{ return xsecGeomScreen; }
	BlankScreen*  getBlankScreen()			{ return blankScreen; }
	MeshScreen*  getMeshScreen()			{ return meshScreen; }
	HrmScreen*  getHrmScreen()				{ return hrmScreen; }
	DuctScreen*  getDuctScreen()			{ return ductScreen; }
	PropScreen*  getPropScreen()			{ return propScreen; }
	EngineScreen* getEngineScreen()			{ return engineScreen; }
	GroupScreen*  getGroupScreen()			{ return groupScreen; }
	//WingScreen*   getWingScreen()			{ return wingScreen; }
	FuseScreen*   getFuseScreen()			{ return fuseScreen; }
	HavocScreen*  getHavocScreen()			{ return havocScreen; }
	ExtScreen*    getExtScreen()			{ return extScreen; }
	MsWingScreen* getMsWingScreen()			{ return msWingScreen; }
	HwbScreen* getHwbScreen()				{ return hwbScreen; }
	CurveEditScreen* getCurveEditScreen()	{ return curveEditScreen; }
	GeomScreen* getGeomScreen()				{ return geomScreen; }
	StructureScreen* getStructureScreen()	{ return structureScreen; }
	CfdMeshScreen* getCfdMeshScreen()		{ return cfdMeshScreen; }
	ParmScreen* getParmScreen()				{ return parmScreen; }
	ParmLinkScreen* getParmLinkScreen()		{ return parmLinkScreen; }
	ParmPickerScreen* getParmPickerScreen()	{ return parmPickerScreen; }
	FeaStructScreen* getFeaStructScreen()	{ return feaStructScreen; }
	LabelScreen* getLabelScreen()			{ return labelScreen; }
	VorviewScreen* getVorviewScreen()		{ return vorviewScreen; }
	TextureMgrScreen* getTextureMgrScreen()	{ return textureMgrScreen; }
	FuselageScreen*   getFuselageScreen()	{ return fuselageScreen; }
	CabinLayoutScreen* getCabinLayoutScreen() { return cabinLayoutScreen; }
	SelectFileScreen* getSelectFileScreen()	{ return selectFileScreen; }

	VspScreen * getScreen(Geom * geom);

	void showParmScreen(Parm* p, int x, int y);


//	static void idleCB( App* appPtr )	{ appPtr->idle(); }
	static void staticMenuCB( Fl_Widget *w, void* data )	{ ((ScreenMgr*)data)->menuCB(w);}

	void closeCB( Fl_Widget* w );
	static void staticCloseCB( Fl_Widget *w, void* data ) { ((ScreenMgr*)data)->closeCB( w ); }

	void hideCB( Fl_Widget* w);
	static void staticHideCB( Fl_Widget* w, void* data) { ((ScreenMgr*)data)->hideCB(w); }

	// scripting support
	void s_new(int src);
	VSPDLL void s_open(int src, const char * file);
	void s_save(int src);
	void s_saveas(int src, const char * newfile);
	void s_savesel(int src, const char * newfile);
	void s_insert(int src, const char * newfile);
	void s_import(int src, const char * newfile, int type);
	void s_export(int src, const char * newfile, int type);
	void s_compgeom(int src);
	void s_meshgeom(int src);
	void s_cfdmeshgeom(int src);
	void s_texturemgr(int scr);
	void s_parmlinkmgr(int src);
	void s_parmpickmgr(int src);
	void s_massprop(int src, int num);
	void s_oldslice(int src, int num);
	void s_awave(int src, int style, int num, double angle, int section, Stringc filename);
	void s_screenshot(int src);

	char* FileChooser( const char* title, const char* filter );

	void MessageBox( const char* msg );


	int antialias;

private:

	Aircraft* aircraftPtr;

	MainWinUI* mainWinUI;
	VspGlWindow* glWin;
	ExportFileUI* exportFileUI;
	ImportFileUI* importFileUI;
	CompGeomUI*	compGeomUI;
	SliceUI* sliceUI;
	AwaveUI* awaveUI;
	BackgroundUI* backgroundUI;
	LightingUI* lightingUI;
	MassPropUI* massPropUI;
	AeroRefUI* aeroRefUI;
	ViewUI* viewUI;
	ScriptUI* scriptUI;
	Fl_Text_Buffer *scriptBuffer;
	ScriptOutUI* scriptOutUI;
	Fl_Text_Buffer *scriptErrorBuffer;
	Fl_Text_Buffer *compGeomTextBuffer;
	Fl_Text_Buffer *awaveTextBuffer;
	Fl_Text_Buffer *aboutScreenTextBuffer;

	PodScreen* podScreen;
	MeshScreen* meshScreen;
	HrmScreen* hrmScreen;
	DuctScreen* ductScreen;
	PropScreen* propScreen;
	EngineScreen* engineScreen;
	BlankScreen* blankScreen;
	//WingScreen* wingScreen;
	GeomScreen* geomScreen;
	StructureScreen* structureScreen;
	CfdMeshScreen* cfdMeshScreen;
	ParmScreen* parmScreen;
	ParmLinkScreen* parmLinkScreen;
	ParmPickerScreen* parmPickerScreen;
	FeaStructScreen* feaStructScreen;
	GroupScreen* groupScreen;
	FuseScreen* fuseScreen;
	FuselageScreen* fuselageScreen;
	HavocScreen* havocScreen;
	ExtScreen* extScreen;
	MsWingScreen* msWingScreen;
	HwbScreen* hwbScreen;
	TextureMgrScreen* textureMgrScreen;
	CabinLayoutScreen* cabinLayoutScreen;
	SelectFileScreen* selectFileScreen;
	XsecGeomScreen* xsecGeomScreen;

	CurveEditScreen* curveEditScreen;

	LabelScreen* labelScreen;
	VorviewScreen* vorviewScreen;
	AboutScreen* aboutScreen;
	KeyHelpScreen* keyHelpScreen;

	char labelStr[256];

	SliderInputCombo* backgroundWScaleSlider;
	SliderInputCombo* backgroundHScaleSlider;
	SliderInputCombo* backgroundXOffSlider;
	SliderInputCombo* backgroundYOffSlider;

	void createGui();

//	double cursorx, cursory;

};

#endif // !defined(AFX_SCREENMGR_H__4D2913C0_85ED_11D7_AC31_0003473A025A__INCLUDED_)
