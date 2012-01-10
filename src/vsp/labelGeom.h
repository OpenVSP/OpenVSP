//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//  Label Class
//
//
//   J.R. Gloudemans - 8/11/04
//
//******************************************************************************

#ifndef LABEL_H
#define LABEL_H

#pragma warning(disable:4786)

#include "geom.h"
#include "fontMgr.h"

class ScreenMgr;
class Aircraft;

static int labelCnt = 0;

class LabelGeom : public GeomBase
{
public:
	enum { RULER_LABEL = 0, TEXT_LABEL = 1 };
	enum { DRAW_HIDDEN, DRAW_VISIBLE, DRAW_HIGHLIGHT };

	LabelGeom();
	virtual ~LabelGeom() {}

//	virtual void  draw() {}
	virtual void  draw(int sel) = 0;
	virtual void reset() {}
	
	virtual void  setTextSize(float size)		{ textSize = size; }
	virtual float getTextSize()					{ return textSize; }

	virtual void  setDrawMode(int mode)			{ drawMode = mode; }
	virtual int   getDrawMode()					{ return drawMode; }

	virtual void  setViewScale(double scale)	{ viewScale = scale; }
	virtual void  setCursor(vec2d pos)			{ cursor = pos; }

	virtual int   isAttached(Geom * geom)	{ return 0; }
	virtual void parm_changed(Parm*) {}

	virtual void write(Aircraft * airPtr, xmlNodePtr node) = 0;
	virtual void read(Aircraft * airPtr, xmlNodePtr node) = 0;

protected:
	virtual void writeLabelGeom(xmlNodePtr node);
	virtual void readLabelGeom(xmlNodePtr node);

	int drawMode;
	float textSize;
	double viewScale;
	vec2d cursor;

};

class TextLabel : public LabelGeom
{
public:

	TextLabel();
	virtual ~TextLabel() {}

	virtual void  draw(int sel);
	virtual void reset();
	virtual void  setVertex( VertexID vert )			{ vertex1 = vert; }

	virtual void  setTextOffset(double offset)			{ textOffset = offset; }
	virtual double getTextOffset()						{ return textOffset; }
//	virtual void  setVertexLock(int lock)				{ vertexLock = lock; }  //!@# is this necessary????
//	virtual int   getVertexLock()						{ return vertexLock; }
	virtual int   isAttached(Geom * geom)			{ return (vertex1.geomPtr == geom); }

	virtual void write(Aircraft * airPtr, xmlNodePtr node);
	virtual void read(Aircraft * airPtr, xmlNodePtr node);

	virtual bool getFixedLabelFlag()					{ return fixedLabelFlag; }
	virtual void setFixedLabelFlag( bool f )			{ fixedLabelFlag = f; }
	virtual vec2d getFixedPos()							{ return fixedPos; }				
	virtual void setFixedPosX( double x )				{ fixedPos[0] = x; }
	virtual void setFixedPosY( double y )				{ fixedPos[1] = y; }


protected:
	virtual void writeTextLabel(Aircraft * airPtr, xmlNodePtr node);
	virtual void readTextLabel(Aircraft * airPtr, xmlNodePtr node);
	vec2d drawString(GLFont * glfont, float scale, Stringc str, float x0, float y0, float xoffset, float yoffset);


	VertexID vertex1;
//	int vertexLock;

	GLFont * font;
	double textOffset;

	bool fixedLabelFlag;
	vec2d fixedPos;

};


class RulerLabel : public TextLabel
{
public:
	enum { INITIALIZED, VERTEX1_SET, VERTEX2_SET, FINALIZED};
	RulerLabel(ScreenMgr* sMgr);
	virtual ~RulerLabel() {}

	virtual void draw(int sel);
	virtual void reset();
	virtual void  setVertex2( VertexID vert2 )			{ vertex2 = vert2; }

	virtual void getVertexPoints( vec3d * start, vec3d * end );
	virtual int  getRulerState()						{ return attachState; }
	virtual void setRulerState(int num)					{ attachState = num; }

	virtual void   setOffset(float mx, float my);
	virtual double getRulerOffset()						{ return rulerOffset; }
	virtual void   setRulerOffset(double offset)		{ rulerOffset = offset; }
	virtual void   setXYZLock(int x, int y, int z)		{ xLock = x; yLock = y, zLock = z; }
	virtual int    getXLock()							{ return xLock; }
	virtual int    getYLock()							{ return yLock; }
	virtual int    getZLock()							{ return zLock; }
	virtual void   setPrecision(int p)					{ precision = p; }
	virtual int    getPrecision()						{ return precision; }
	virtual void   setUnitString(Stringc units)		{ unitString = units; }
	virtual Stringc getUnitString()					{ return unitString; }

	virtual int   isAttached(Geom * geom)			{ return (vertex1.geomPtr == geom || vertex2.geomPtr == geom); }
	virtual double calculateDistance();

	virtual void write(Aircraft * airPtr, xmlNodePtr node);
	virtual void read(Aircraft * airPtr, xmlNodePtr node);

protected:
	virtual void writeRulerLabel(Aircraft * airPtr, xmlNodePtr node);
	virtual void readRulerLabel(Aircraft * airPtr, xmlNodePtr node);
	virtual void calculateOffset();

	
	ScreenMgr* screenMgr;

	VertexID vertex2;

	double rulerOffset;
	int attachState;
	int xLock, yLock, zLock;

	double rulerDistance;

	int precision;
	Stringc unitString;
};


#endif


