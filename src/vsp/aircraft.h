//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// aircraft.h: interface for the aircraft class.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AIRCRAFT_H__9E994B80_7FDF_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_AIRCRAFT_H__9E994B80_7FDF_11D7_AC31_0003473A025A__INCLUDED_

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>



#include "defines.h"
#include "geom.h"
#include "wingGeom.h"
#include "fuse_geom.h"
#include "FuselageGeom.h"
#include "havoc_geom.h"
#include "ext_geom.h"
#include "ms_wing.h"
#include "hwb.h"
#include "cabinLayoutGeom.h"
#include "meshGeom.h"
#include "ductGeom.h"
#include "propGeom.h"
#include "engineGeom.h"
#include "labelGeom.h"
#include "vorGeom.h"
#include "parmLinkMgr.h"
#include "quat.h"

class TriTest;

class ScreenMgr;
class VspGlWindow;

//==== Must Be in Order 0 - n - Used For Choice UI ====//
#define POD_GEOM_TYPE			0
#define WING_GEOM_TYPE			1
#define FUSE_GEOM_TYPE			2
#define HAVOC_GEOM_TYPE			3
#define EXT_GEOM_TYPE			4
#define MS_WING_GEOM_TYPE		5
#define BLANK_GEOM_TYPE			6
#define DUCT_GEOM_TYPE			7
#define PROP_GEOM_TYPE			8
#define ENGINE_GEOM_TYPE		9
#define HWB_GEOM_TYPE			10
#define FUSELAGE_GEOM_TYPE		11
#define CABIN_LAYOUT_GEOM_TYPE	12
//#define RULER_LABEL_TYPE	10
//#define TEXT_LABEL_TYPE		11

#define MESH_GEOM_TYPE			22
#define GROUP_GEOM_TYPE			23
#define VOR_GEOM_TYPE			24
#define XML_GEOM_TYPE			25
#define USER_GEOM_TYPE			26
#define XSEC_GEOM_TYPE			27


class Aircraft  
{
public:

	enum { FELISA, XSEC, STEREOLITH, RHINO3D, AWAVE, NASCART, POVRAY, CART3D, VORXSEC, XSECGEOM, GMSH, X3D };

	VSPDLL Aircraft();
	VSPDLL virtual ~Aircraft();

	VSPDLL void setScreenMgr( ScreenMgr* screenMgrIn );
	ScreenMgr * getScreenMgr()						{ return screenMgr; }
	void geomMod( Geom* gPtr );

	void newFile();
	VSPDLL int  openFile( const char* file_name );
	int  insertFile( const char* file_name );

	void writeFile( const char* file_name , bool restore_file_name = false );
	void writeFile( const char* file_name, vector< Geom * > &gVec, vector< LabelGeom * > &lVec, bool restore_file_name = false );
	int  readFile( const char* file_name );
	int  readOldRamFile ( const char* file_name );

	VSPDLL void write_stl_file(const char* file_name);
	bool check_for_stl_mesh();


	void writeX3DViewpoints( xmlNodePtr node);
	void writeViewpointsProps( xmlNodePtr node, Stringc orients, Stringc cents, Stringc posits, const char* sfov, Stringc name);
	void double4vec2str( double* vec, Stringc &str );
	void doublevec2str(double* vec, Stringc &str );
	
	void writeX3DMaterial( xmlNodePtr node, int matid );
	void floatvec2str( float* vec, Stringc &str );

	VSPDLL void write_x3d_file(const char* file_name);
	VSPDLL void write_rhino_file(const char* file_name);
	VSPDLL void write_xsec_file(const char* file_name);
	VSPDLL void write_nascart_files(const char* file_name);
	VSPDLL void write_cart3d_files(const char* file_name);
	VSPDLL void write_gmsh_files(const char* file_name);
	void write_bezier_file(const char* file_name);

	VSPDLL void write_povray_file(const char* file_name);

	//==== Felisa Stuff ====//
	VSPDLL void write_felisa_file(const char* file_name);
	VSPDLL void write_felisa_background_file(const char* file_name);
	VSPDLL void write_fps3d_bco_file(const char* file_name);
	void get_far_field(double& x_min, double& x_max, double& y_min,
                       double& y_max, double& z_min, double& z_max);
	void set_far_field(double x_min, double x_max, double y_min,
                       double y_max, double z_min, double z_max);

	//==== Aero Reference Component Stuff ====//
	VSPDLL Geom*  getRefGeom();
	VSPDLL void   setRefGeomIndex( int id );
	VSPDLL vec3d  getCGLoc()					{ return cgLoc; }
	VSPDLL void   setCGLoc( const vec3d& l)	{ cgLoc = l; }
	VSPDLL vec3d  getSMLoc();									// Static Margin
	VSPDLL void   setSMLoc( const vec3d& l);
	VSPDLL int    getCGRelAcFlag()				{ return cgRelAcFlag; }
	VSPDLL void   setCGRelAcFlag( int f )		{ cgRelAcFlag = f; }

	//==== Mass Properties Stuff ====//
	VSPDLL double get_total_mass()						{ return total_mass; }
	void   set_total_mass( double m )			{ total_mass = m; }
	VSPDLL vec3d  get_Ixx_Iyy_Ixx()					{ return Ixx_Iyy_Ixx; }
	vec3d  set_Ixx_Iyy_Ixx( const vec3d& ivec)	{ Ixx_Iyy_Ixx = ivec; }
	VSPDLL vec3d  get_Ixy_Ixz_Iyz()					{ return Ixy_Ixz_Iyz; }
	vec3d  set_Ixy_Ixz_Iyz( const vec3d& ivec)	{ Ixy_Ixz_Iyz = ivec; }
	int    getDrawCGFlag()						{ return drawCGFlag; }
	void   setDrawCGFlag( int f )				{ drawCGFlag = f; }
	int    getDrawACFlag()						{ return drawACFlag; }
	void   setDrawACFlag( int f )				{ drawACFlag = f; }

	VSPDLL Geom* comp_geom(int sliceFlag, int meshFlag = 0, int halfFlag = 0 );
	VSPDLL Geom* massprop(int numSlices);
	VSPDLL Geom* slice(int style, int numSlices, double sliceAngle, double coneSections, Stringc filename);
	Geom* read_stl_file(const char* file_name);
	Geom* read_xsec_file(const char* file_name);
	Geom* read_xsecgeom_file(const char* file_name);
	Geom* read_nascart_file(const char* file_name);
	VSPDLL Geom* addMeshGeom();

	Stringc getFileName()					{ return fileName; }

	//==== geom objects ====//
	Geom* createGeom( int type );
	Geom* copyGeom(Geom* geomPtr);
	void revertEdits();
	void addGeom( Geom* geom );
	enum { MOVE_UP, MOVE_DOWN, MOVE_TOP, MOVE_BOT, };
	void reorderGeom( int action );

	VSPDLL void delGeom( Geom* geomPtr );
	void modifyGeom( Geom* geomPtr );

	VSPDLL void setActiveGeom( Geom* gptr );
	VSPDLL Geom* getActiveGeom();
	void setActiveGeomVec( vector< Geom* > gptr );
	vector< Geom* > getActiveGeomVec();
	virtual void flagActiveGeom();

	vector< Geom* > getGeomVec()			{ return geomVec; }
	vector< Geom* > getDisplayGeomVec();
	vector< Geom* > getGeomByName(Stringc name, int children);

	int getGeomIndex(Geom * geom); //used for saving labels
	Geom* getClipboardGeom(int index); //used for saving labels

	//==== label objects ====//
	void addLabel( LabelGeom* label );
	void setActiveLabel( LabelGeom* label );
	void setActiveLabelVec( vector< LabelGeom* > lVec )	{ activeLabelVec = lVec; }
	LabelGeom * getActiveLabel();
	vector< LabelGeom* > getActiveLabelVec()	{ return activeLabelVec; }
	LabelGeom * createLabel(int type);
	vector< LabelGeom* > getLabelVec()			{ return labelVec; }
	void removeLabel(LabelGeom * label);
	void resetAttachedLabels(Geom * geom);

	vector< LabelGeom* > getLabelByName(Stringc name);


	void cutGeomVec( vector< Geom* >& gVec );
	void copyGeomVec( vector< Geom* >& gVec );
	void pasteClipBoard();
	vector< Geom* > getClipboard()			{ return clipBoard; }

	VSPDLL void update_bbox();
	bbox getBndBox()						{ return bnd_box; }
	void clearBBoxColors();


	int getNumOfType( int type );

	void setName( Stringc nameIn )			{ nameStr = nameIn; }
	Stringc getName()						{ return nameStr; }

	void recenterACView();
	void triggerDraw();
	void draw();

	void fastDrawFlagOn();
	void fastDrawFlagOff();

	int get_version()						{ return oldFileVersion; }
	Geom* get_ref_geom()					{ return 0; }	//jrg fix
	Geom* get_trim_geom()					{ return 0; }	//jrg fix

	void setCursor(float cx, float cy);
	void mouseClick(float mx, float my);
	void drawHighlight();
	void draw2D();
	void drawCG();
	void drawAC();
	void drawMarker( vec3d& pos, double size, int r, int g, int b  );

	//==== Vorview Stuff ====//
	VorGeom* getVorGeom()					{ return vorGeom; }
	UserGeom* getUserGeom()					{ return userGeomPtr; }
	Geom* read_vorxsec_file(const char* file_name);

	//==== Custom Default Component Stuff ====//
	void readDefaultCompGroupFile( const char* filename );
	void setDefaultCompGroupID( int id );

	//==== Export File Names ====//
	enum{ COMP_GEOM_TXT_TYPE, COMP_GEOM_CSV_TYPE, SLICE_TXT_TYPE, MASS_PROP_TXT_TYPE, DRAG_BUILD_TSV_TYPE };
	void setExortFileName( const char* fn, int type );
	Stringc getExportFileName( int type );
	void setExportCompGeomCsvFile( bool b )		{ exportCompGeomCsvFile = b; };
	bool getExportCompGeomCsvFile()				{ return exportCompGeomCsvFile; };
	void setExportDragBuildTsvFile( bool b )	{ exportDragBuildTsvFile = b; };
	bool getExportDragBuildTsvFile()			{ return exportDragBuildTsvFile; };
	void updateExportFileNames();

	//==== Temp Dir ====//
	void setTempDir( const char* name, bool user_set );
	Stringc getTempDir();

private:

	int version;

	int oldFileVersion;

	ScreenMgr* screenMgr;
	VspGlWindow* drawWin;

//	Geom* activeGeom;
	vector< Geom* > activeGeomVec;			// Vector of Active Geoms
	vector< Geom* > geomVec;				// Vector of All Geoms
	vector< Geom* > topGeomVec;				// Top Level (ie no parents )

	vector< LabelGeom* > labelVec;				// Vector of All Label Objects
	vector< LabelGeom* > activeLabelVec;		// Vector of All Active Label Objects

	Stringc clipBoardStr;
	vector< Geom* > clipBoard;
	Geom* editGeom;
	Geom* backupGeom;

	void copyToClipBoard( vector< Geom* > & gVec );

	Stringc nameStr;
	Stringc fileName;
	bool tempDirUserOverride;
	Stringc tempDirName;

	bbox bnd_box;

	//==== Far Field for Felisa Dump ====//
	int guess_far_field;
	double far_x_min, far_x_max;
	double far_y_min, far_y_max;
	double far_z_min, far_z_max;

	//==== Reference Parms ====//
	int    drawCGFlag;
	int    drawACFlag;
	int    cgRelAcFlag;
	vec3d  cgLoc;

	//==== Mass Parms ====//
	double total_mass;
	vec3d  Ixx_Iyy_Ixx;
	vec3d  Ixy_Ixz_Iyz;

	VertexID highlightVertex;

	VorGeom* vorGeom;
	UserGeom* userGeomPtr;

	//==== Comp Geom File Names ====//
	bool exportCompGeomCsvFile;
	Stringc compGeomTxtFileName;
	Stringc compGeomCsvFileName;
	Stringc sliceFileName;
	Stringc massPropFileName;
	bool exportDragBuildTsvFile;
	Stringc dragBuildTsvFileName;


	//==== Custom Default Component Stuff ====//
	map< int, Geom* > m_DefaultCompMap;


};

#endif // !defined(AFX_AIRCRAFT_H__9E994B80_7FDF_11D7_AC31_0003473A025A__INCLUDED_)
