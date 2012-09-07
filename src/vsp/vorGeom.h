//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "geom.h"

#ifdef WIN32

#else
#include <unistd.h>
#endif

//******************************************************************************
//    
//   Vorview Geometry Class
//  
// 
//   J.R. Gloudemans - 11/11/04
//
//    
//******************************************************************************

#ifndef VORGEOM_H
#define VORGEOM_H

#include "vorSlice.h"

class VorviewScreen;

class HrmComp
{
public:
	   
	Stringc name_str;
	int groupNum;
	int type;
	int numCross;
	int numPnts;

	vector< vector< vec3d > > crossVec;
};

#define MAX_NUM_VEC 100

class CaseData
{
public:

	CaseData();
	virtual ~CaseData();

	virtual int read_file( const char* file_name );
	virtual int write_file( const char* file_name );
	virtual double findDouble( const char* name, vector< Stringc > & wVec, double defVal );
	virtual int findInt( const char* name, vector< Stringc > & wVec, int defVal );
	virtual vector<double> findDoubleVec( const char* name, vector< Stringc > & wVec, vector<double> defVal );


 	int lax;				// Card2
	double hag;
	int isweep;
	int itrmax;
	int idetail;
	double spc;
	double floatx;
	double floaty;

	int nmach;				// Card3
	double xmach[MAX_NUM_VEC];

	int nalpha;				// Card4
	double alpha[MAX_NUM_VEC];
	
	int latral;				// Card5
	double psi;
	double pitchq;
	double rollq;
	double yawq;
	double vinf;

	double sref;			// Card 6
	double ar;
	double taper;
	double cbar;
	double xbar;
	double zbar;
	double wspan;

	int nsbpan;			// Card 7

	double ltail;			// Card 8
	double ztail;
	double swpwng;
	int istab;	
	int iaero;
	double cdmach;
	double cdo;

	int iburst;				// Card 9
	double xref;
	double angcrt;
	double ck1;
	double ck2;

	int iac;				// Card 10
	double aceps;

	int itrim;				// Card 11
	int ntrim;
	double alpha_trim[MAX_NUM_VEC];
	double cltrim[MAX_NUM_VEC];
	double cmtrim[MAX_NUM_VEC];
	double cmeps;

	double reinf;			// Card 12
	double reflen;
	int idrag;				
	int numren;
	int nummch;

//jrg add 2d airfoil data???

};



enum{ DRAW_XSEC, DRAW_SLICE, DRAW_SUBPOLY, DRAW_CP, DRAW_CP_SURF, DRAW_CAMBER, DRAW_CSF, NUM_DRAW_FLAGS };

enum{ EXECUTE_SUBDIVIDE, EXECUTE_VORLAX, EXECUTE_SURFACE_CP, };

enum{ CP_MIN, CP_MAX, CP_MIN_CLAMP, CP_MAX_CLAMP, CP_SURF_MIN, CP_SURF_MAX, CP_MIN_SURF_CLAMP, CP_MAX_SURF_CLAMP, };




class VorGeom : public Geom
{
public:
	VorGeom(Aircraft* aptr);
	virtual ~VorGeom();

	virtual void write(xmlNodePtr node);
	virtual void read(xmlNodePtr node);

	virtual int  read_xsec( const char* file_name );
	virtual void convertToHerm();

	virtual void copy( Geom* fromGeom );
	virtual void parm_changed(Parm*);
	virtual void draw();
	virtual void drawAlpha();
	virtual void update_bbox();

	virtual void acceptScaleFactor();
	virtual void resetScaleFactor();

	virtual int getNumXSecSurfs()									{ return 0; }

    virtual vec3d getVertex3d(int surf, double x, double p, int r)	{ return vec3d(); }
	virtual void  getVertexVec(vector< VertexID > *vertVec)			{ }

	CaseData caseData;

	virtual void setBaseName( const char* name)						{ baseName = name; }
	virtual Stringc getBaseName()									{ return baseName; }

	virtual void setDrawFlag( int id, int flag )					{ drawFlags[id] = flag; }
	virtual int  getDrawFlag( int id )								{ return drawFlags[id]; }

	virtual int  runVorlax(int type);
    virtual int  forkVorlax(int type, int isurf = 0, int icalc = 1, double delta = 0.0, int stab = 0);

	virtual void waitVorlax();										// Wait till Vorlax Fork finishes

	virtual int  checkVorlaxRun( double* percentDone );

	virtual void readVorlaxOut( int* isurf, int* icalc, double* delta, int* stab );
	
	virtual void readVorlaxFiles();

	virtual void readInputVorlaxFiles();

	virtual float getCpClamps( int id );
	virtual void  incCpClamps( int minInc, int maxInc );

	virtual vec3d getCpColor( double fract );

	virtual void setCamberScale( double s );

	virtual int getNumSubPanels()									{ return caseData.nsbpan; }
	virtual void setNumSubPanels(int n)								{ caseData.nsbpan = n; }

	virtual void autoFind();
	virtual void autoEliminate();
	virtual void autoMerge();
	virtual void autoSlice();
	virtual void autoAll();
	virtual void autoReset();

	virtual void setFlatFlag(int f);
	virtual int  getFlagFlag();
	
	virtual void setKeySliceSymFlag(int f);
	virtual int  getKeySliceSymFlag();

	virtual int  getNumKeySlice();
	virtual void setCurrKeySlice(int ind);
	virtual int  getCurrKeySlice();
	virtual void delCurrKeySlice();

	virtual void setCurrNumRegionSlices( int n );

	virtual float getCurrKeySliceY();
	virtual float getCurrKeySliceYMin();
	virtual float getCurrKeySliceYMax();
	virtual float getCurrKeySliceYFract();

	virtual void  setKeySliceY( float y );
	virtual void  adjustKeySliceY( float dir );
	virtual float getTempKeySliceY();

	virtual void setEditKeySliceYFract( float yf, int sym_flag );

	virtual void  setKeySliceTempFlag( int flag );
	virtual int   getKeySliceTempFlag();

	virtual void  addKeySlice();

	VorSlice* getVorSlicePtr()			{ return vorSlicePtr; }

	virtual void displayControl( int flag );

	virtual void writeCsf();

	virtual void setVorviewScreenPtr( VorviewScreen* ptr )		{ vorviewScreenPtr = ptr; }
	
	virtual void setCsfName( const char* val );
	virtual void setCsfAngle( double val );
	virtual void setCsfChord( double val );
	virtual void setCsfHinge( double val );
	virtual void setCsfType( int val );
	virtual void setCsfRefl( int val );
	virtual void setCsfSym( int val );

	virtual void saveWriteSlices();

	virtual bool vorlaxExeExists();

private:

	Stringc  baseName;
	vector< HrmComp > compVec;
	VorSlice* vorSlicePtr;

	int drawFlags[NUM_DRAW_FLAGS];
	int saveDrawFlags[NUM_DRAW_FLAGS];

	int validGeomFlag;

	VorviewScreen* vorviewScreenPtr;


	//===== Windows Process Structs ====//
#ifdef WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#else
	pid_t childPid;
#endif



};


#endif

