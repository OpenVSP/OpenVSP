// EditCurve.h: interface for the drawBase class.
// J.R. Gloudemans
//////////////////////////////////////////////////////////////////////

#ifndef EDIT_CURVE_H
#define EDIT_CURVE_H

#include "drawBase.h"
#include "geom.h"
#include "bezier_curve.h"
#include "herm_curve.h"
#include <vector>

class CurveEditScreen;

#define EC_NO_SYM 0
#define EC_X_SYM  1
#define EC_Y_SYM  2

#define EC_2PNT_CURVE	0
#define EC_2PNT_CIRCLE	1
#define EC_3PNT_CIRCLE	2

class EditPnt
{
public:

	EditPnt();

	vec3d pnt;
	int smoothFlag;
	vec3d  d0;
	double m0;
	vec3d  d1;
	double m1;

	int pntLockX;
	int pntLockY;

	int tan0LockX;
	int tan0LockY;

	int tan1LockX;
	int tan1LockY;

};

class EditCurve : public DrawBase , public GeomBase
{

public:
	EditCurve(GeomBase* geom_ptr_in = 0);
	virtual ~EditCurve();

    virtual void write(xmlNodePtr node);
    virtual void read(xmlNodePtr node);

	virtual void set_geom( GeomBase* geom_ptr_in )			{ geom_ptr = geom_ptr_in; }
	virtual void draw();
	virtual int processKeyEvent();
	virtual int processMoveEvent();
	virtual int processDragEvent();
	virtual int processPushEvent();

	virtual void setGlWinWidthHeight( int w, int h )	{ winWidth = w; winHeight = h; }

	virtual void setGlWinOrthoLRTB( double l, double r, double t, double b  )
									{ winLeft = l; winRight = r; winTop = t;  winBot = b; }

	virtual void parm_changed(Parm* p);
	virtual void generate();
	virtual void triggerGeom();

	Parm* getScaleX()					{ return(&scaleX); }
	Parm* getScaleY()					{ return(&scaleY); }
	Parm* getPntX()						{ return(&pntX);  }
	Parm* getPntY()						{ return(&pntY);  }
	Parm* getScaleTans()				{ return(&scaleTans); }

	Parm* getMaxWidth()					{ return(&maxWidth); } 
	Parm* getMaxHeight()				{ return(&maxHeight); }

	virtual int  getCloseFlag()					{ return closeFlag; }
	virtual void setCloseFlag( int flag );

	virtual int  getSmoothFlag();
	virtual void setSmoothFlag( int flag );

	virtual void delEditPnt();
	virtual void addEditPnt();

	virtual void updateScale();
	virtual void acceptScaleX();
	virtual void resetScaleX();
	virtual void acceptScaleY();
	virtual void resetScaleY();

	virtual int  getSym()						{ return symFlag; }
	virtual void setSym( int s )				{ symFlag = s; }

	virtual int  getLockX();
	virtual void setLockX( int f );
	virtual int  getLockY();
	virtual void setLockY( int f );

	virtual void resetScaleTans();

	virtual void initShape();
	virtual void setShapeType( int type )							{ shapeType = type; }
	virtual int  getShapeType()										{ return shapeType; }

	virtual void setCurveEditScreenPtr( CurveEditScreen* ptr )		{ screenPtr = ptr; }

	virtual double getRollerScaleFactor()							{ return rollerScaleFactor; }
	virtual void   setDrawScaleFactor( double sf );

	virtual void setEditPntVec( vector< EditPnt > & pVec );
	virtual vector< EditPnt > & getEditPntVec()						{ return pntVec; }

	virtual herm_curve getHermCurve();
	virtual Bezier_curve getBezierCurve();

	virtual void computeMaxWH();
	virtual void scaleWH( double w, double h );

	virtual void blend( double fract, EditCurve* crv0,  EditCurve* crv1 );

private:

	GeomBase* geom_ptr;

	int winWidth, winHeight;
	double winLeft, winRight, winTop, winBot;

	int nearPntID;
	int selectPntID;
	int closeFlag;
	int symFlag;

	int shapeType;

	CurveEditScreen* screenPtr;

	Bezier_curve crv;

	vector< EditPnt > pntVec;
	vector< vec3d > controlPntVec;

	virtual void movePnt( double x, double y );

	Parm scaleX;
	Parm scaleY;
	Parm pntX;						
	Parm pntY;
	Parm scaleTans;

	double lastScaleX;
	double lastScaleY;

	double rollerScaleFactor;
	double drawScaleFactor;

	Parm maxWidth;
	Parm maxHeight;

};

#endif 
