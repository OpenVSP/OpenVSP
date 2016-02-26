//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//   FEA Part Class
//   J.R. Gloudemans - 2/14/09
//******************************************************************************

#ifndef FEA_PART_H
#define FEA_PART_H


#include "Vec2d.h"
#include "Vec3d.h"
#include "Matrix.h"
// #include "drawBase.h"
#include "BndBox.h"

#include "Surf.h"
#include "ISegChain.h"

#include <vector>
#include <algorithm>
using namespace std;


// Node Classification
enum { RIB_UPPER, RIB_LOWER, SPAR_UPPER, SPAR_LOWER, SKIN_UPPER, SKIN_LOWER, RIB_ALL, SPAR_ALL };

class FeaNodeTag
{
public:
    int m_Type;
    int m_ID;
};


class FeaNode
{
public:
    FeaNode()
    {
        m_Index = -1;
        m_Thick = 0.1;
    };
    FeaNode( vec3d & p )
    {
        m_Pnt = p;
        m_Index = -1;
        m_Thick = 0.1;
    }
    virtual ~FeaNode()              { }

    int GetIndex();

    int m_Index;
    vec3d m_Pnt;

    double m_Thick;

    void AddTag( int type, int id );
    bool HasTag( int type, int id );
    bool HasTag( int type );
    bool HasOnlyType( int type );
    vector< FeaNodeTag > m_Tags;

    void WriteNASTRAN( FILE* fp );
    void WriteCalculix( FILE* fp );

};


class FeaElement
{
public:

    FeaElement()                    {}
    virtual ~FeaElement()           {}

    virtual void DeleteAllNodes();
    virtual void LoadNodes( vector< FeaNode* > & node_vec );
    virtual int  GetType()
    {
        return m_Type;
    }
    virtual void WriteCalculix( FILE* fp, int id ) = 0;
    virtual void WriteNASTRAN( FILE* fp, int id ) = 0;
    virtual double ComputeMass( double density ) = 0;
//  virtual void DrawPoly();

    enum { FEA_TRI_6, FEA_QUAD_8, };
    vector< FeaNode* > m_Corners;
    vector< FeaNode* > m_Mids;

protected:
    int m_Type;
};

//==== 6 Point Triangle Element ====//
class FeaTri : public FeaElement
{
public:
    FeaTri()                        {}
    virtual ~FeaTri()               {}
    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2 );
    virtual void WriteCalculix( FILE* fp, int id );
    virtual void WriteNASTRAN( FILE* fp, int id );
    virtual double ComputeMass( double density );
};

//=== 8 Point Quad Element ====//
class FeaQuad : public FeaElement
{
public:
    FeaQuad()                       {}
    virtual ~FeaQuad()              {}
    virtual void Create( vec3d & p0, vec3d & p1, vec3d & p2, vec3d & p3 );
    virtual void WriteCalculix( FILE* fp, int id );
    virtual void WriteNASTRAN( FILE* fp, int id );
    virtual double ComputeMass( double density );
};

class FeaSplice
{
public:
    FeaSplice();
    virtual ~FeaSplice()            {};

    bool m_FixedFlag;
    Parm m_Pos;             // Start x/c
    Parm m_Thick;
};

class FeaSpliceLine     //  : public DrawBase
{
public:
    FeaSpliceLine( double default_thick );
    virtual ~FeaSpliceLine();

    virtual void ClearSpliceVec();
    virtual double FindMaxThick();
    virtual void SetMode( int mode );
    virtual int  GetMode()
    {
        return m_Mode;
    }
    virtual double MapMouseToChord( double x );
    virtual FeaSplice* FindClosestSplice( double x, double y );
    FeaSplice* GetEditSplice()
    {
        return m_EditSplice;
    }
    FeaSplice* GetHighlightSplice()
    {
        return m_HighlightSplice;
    }
    virtual void ChangeEditSplice( int inc );

    virtual void SetEditSpliceLoc( double xc );

    virtual void SetDefaultThick( double t )
    {
        m_DefaultThick = t;
    }

    virtual void DelEditSplice();
    virtual double ComputeThick( double per_chord );
    virtual void SetEndPoints( vec3d & e0, vec3d & e1 );

//  virtual void DrawMain();
//  virtual void draw();

    virtual void setGlWinWidthHeight( int w, int h )
    {
        winWidth = w;
        winHeight = h;
    }

    virtual void setGlWinOrthoLRTB( double l, double r, double t, double b  )
    {
        winLeft = l;
        winRight = r;
        winTop = t;
        winBot = b;
    }

    virtual int processPushEvent();
    virtual int processMoveEvent();
    virtual int processReleaseEvent();
    virtual int processDragEvent();


    int m_SectID;
    Parm m_PerSpan;
    Parm m_DefaultThick;


    vector< FeaSplice* > m_SpliceVec;

    FeaSplice* m_EditSplice;
    FeaSplice* m_HighlightSplice;

    int m_Mode;
    enum { NORMAL_MODE, ADD_MODE, DEL_MODE, EDIT_MODE, };

    int winWidth, winHeight;
    double winLeft, winRight, winTop, winBot;

    double m_WinXScale;
    double m_MouseX;
    double m_MouseY;
    double m_RefMouseX;
    double m_RefPos;

    vec3d m_EndPnts[2];

};


class FeaPart
{
public:

    FeaPart();
    virtual ~FeaPart();

    Surf* GetSurf()
    {
        return m_Surf;
    }

    void LoadNodes( vector< FeaNode* > & node_vec );

    double ComputeMass();


    enum { COMP_ID = -9999 };

    Surf* m_Surf;
    vector< FeaElement* > m_Elements;

    Parm m_Density;

};

class WingSection;

class FeaSkin : public FeaPart
{
public:
    FeaSkin();
    virtual ~FeaSkin();

    void Clean();

    void WriteData( xmlNodePtr root );
    void ReadData(  xmlNodePtr root );

    void SetSurf( Surf* s )
    {
        m_Surf = s;    // Upper or Lower Wing Surface
    }
    void BuildMesh();
    void SetNodeThick();
    void SetExportFlag( bool f )
    {
        m_ExportFlag = f;
    }
    bool GetExportFlag()
    {
        return m_ExportFlag;
    }
    void SetWingSection( WingSection* wp )
    {
        m_WingSection = wp;
    }

    void SetDefaultThick( double t );
//  virtual void Draw( bool highlight );
//    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, bool highlight );

    void AddSpliceLine();
    void DelCurrSpliceLine();
    void SetCurrSpliceLineID( int id );
    int  GetCurrSpliceLineID()
    {
        return m_CurrSpliceLineID;
    }
    FeaSpliceLine* GetCurrSpliceLine();

    void ComputeSpliceLineEndPoints();

    bool m_ExportFlag;

    WingSection* m_WingSection;

    Parm m_DefaultThick;

    int m_CurrSpliceLineID;
    vector< FeaSpliceLine* > m_SpliceLineVec;


};

class FeaSlice : public FeaPart
{
public:
    FeaSlice();
    virtual ~FeaSlice();

    virtual void Clean();
    virtual void ComputeEndPoints()             { }

    virtual void SetEndPoints( vec2d & uwA, vec2d uwB );
    virtual void SetNormal( vec3d & norm )
    {
        m_Normal = norm;
    }

    virtual void FindUpperLowerPoints();
    virtual int  ComputeNumDivisions();
    virtual void SetNumDivisions( int n )
    {
        m_NumDivisions = n;
    }

    virtual void BuildMesh();
    virtual void MergeChains( list< ISegChain* > & chain_list, vec3d & start_pnt, vector< vec3d > & pnt_vec );

    virtual void FindLowerPnts(  vector< vec3d > & upper_pnts, list< ISegChain* > & chain_list,
                                 vector< vec3d > & lower_pnts );
    virtual void MatchUpperLowerPnts( vector< vec3d > & upper_pnts, vector< vec3d > & lower_pnts );

    virtual int FindClosestPnt( vec3d & pnt, vector< vec3d > & target_vec );
    virtual int FindClosestPnt( vec3d & pnt, vec3d & dir, vector< vec3d > & target_vec );

    virtual bool IsCap()
    {
        return false;
    }
    virtual void SetUpperCapSurfs( Surf* s0, Surf* s1 );
    virtual void SetLowerCapSurfs( Surf* s0, Surf* s1 );

//  virtual void Draw();
//  virtual void DrawSlicePlane();
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    virtual void SnapUpperLowerToSkin( vector < FeaNode* > & skinNodes );

    Parm m_Thick;

    vec3d m_UpperEndPnts[2];
    vec3d m_UpperStartChainPnt;
    vec3d m_LowerStartChainPnt;
    vec3d m_UpperEndChainPnt;
    vec3d m_LowerEndChainPnt;
    vec3d m_Normal;

    int m_NumDivisions;
    vector < vec3d > m_UpperPnts;
    vector < vec3d > m_LowerPnts;

    //==== Surfaces That Border Cap ====//
    bool m_CapInFlag;
    Surf* m_CapUpperSurf[2];
    Surf* m_CapLowerSurf[2];

    DrawObj m_SliceDO;

};


class FeaSpar : public FeaSlice
{
public:
    FeaSpar();
    virtual ~FeaSpar();

    virtual void ComputeEndPoints();
//  virtual void Draw( bool highlight );
//  virtual void Draw() {FeaSlice::Draw();}
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, bool highlight );


    int m_SectID;               // Section
    Parm m_PerChord;            // Percent Chord
    bool m_AbsSweepFlag;        // Use Absolute Sweep
    Parm m_Sweep;

    bool m_TrimFlag;
};

class FeaRib : public FeaSlice
{
public:
    FeaRib();
    virtual ~FeaRib();

    virtual void ComputeEndPoints();
    virtual bool IsCap();

//  virtual void Draw( bool highlight );
//  virtual void Draw() {FeaSlice::Draw();}
    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec, bool highlight );

    int m_SectID;
    Parm m_PerSpan;
    bool m_AbsSweepFlag;
    Parm m_Sweep;

    bool m_TrimFlag;
};

class FeaPointMass
{
public:
    FeaPointMass();
    virtual ~FeaPointMass();

    Parm m_PosX;
    Parm m_PosY;
    Parm m_PosZ;

    vec3d m_AttachPos;

//  virtual void Draw( bool highlight );

};

#endif
