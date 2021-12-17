//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SubSurface.h
// Alex Gary
//////////////////////////////////////////////////////////////////////

#ifndef SUBSURFACE_INCLUDED_
#define SUBSURFACE_INCLUDED_

#include "VspSurf.h"
#include "TMesh.h"
#include "Vec2d.h"
#include "Vec3d.h"
#include "Parm.h"
#include "ParmContainer.h"
#include "DrawObj.h"
#include "APIDefines.h"

// SubSurface Line Segment
class SSLineSeg
{
public:
    // SubSurface UW Line Type
    SSLineSeg();

    virtual void Update( Geom* geom );

    enum { GT, LT, NO };
    int m_TestType;

    virtual ~SSLineSeg();
    virtual bool Subtag( TTri* tri ) const;
    virtual bool Subtag( const vec3d & center ) const;
    virtual TMesh* CreateTMesh() const;
    virtual void AddToTMesh( TMesh* tmesh ) const;

    virtual void SetSP0( vec3d pnt )
    {
        m_SP0 = pnt;
    }
    virtual void SetSP1( vec3d pnt )
    {
        m_SP1 = pnt;
    }
    virtual void SetP0( vec3d pnt )
    {
        m_P0 = pnt;
    }
    virtual void SetP1( vec3d pnt )
    {
        m_P1 = pnt;
    }
    virtual vec3d GetP0()
    {
        return m_P0;
    }
    virtual vec3d GetP1()
    {
        return m_P1;
    }
    virtual void GetDOPts( VspSurf* surf, Geom* geom, vector < vec3d > &pts, int num_pnts );
    virtual int CompNumDrawPnts( VspSurf* surf, Geom* geom );

protected:
    vec3d m_SP0; // scaled uw points
    vec3d m_SP1;

    vec3d CompPnt( VspSurf* surf, vec3d uw_pnt ) const;

private:
    vec3d m_P0; // none scaled uw points
    vec3d m_P1;
    vec3d m_line;
};

class SubSurface : public ParmContainer
{
public:
    SubSurface( const string& compID, int type );
    virtual ~SubSurface();
    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual int GetType()
    {
        return m_Type;
    }
    virtual string GetCompID()
    {
        return m_CompID;
    }
    virtual std::vector< std::vector< SSLineSeg > >& GetSplitSegs()
    {
        return m_SplitLVec;
    }
    virtual std::vector< SSLineSeg >& GetLVec()
    {
        return m_LVec;
    }
    virtual bool GetPolyFlag()
    {
        return m_PolyFlag;
    }
    virtual std::vector< std::vector< vec2d > > & GetPolyPntsVec()
    {
        return m_PolyPntsVec;
    }
    virtual void LoadDrawObjs( std::vector< DrawObj* >& draw_obj_vec );
    virtual void LoadPartialColoredDrawObjs( const string & ss_id, int surf_num, std::vector < DrawObj * > & draw_obj_vec, vec3d color );
    virtual void SetUpdateDraw( bool flag )
    {
        m_UpdateDrawFlag = flag;
    }

    virtual void SetLineColor( vec3d color )
    {
        m_LineColor = color;
    }

    static std::string GetTypeName( int type );

    virtual bool Subtag( TTri* tri ); // Method to subtag triangles from TMesh.
    virtual bool Subtag( const vec3d & center );
    virtual void Update();
    virtual void UpdatePolygonPnts();
    virtual std::vector< TMesh* > CreateTMeshVec() const; // Method to create a TMeshVector
    virtual void UpdateDrawObjs(); // Method to create lines to draw
    virtual void SplitSegsU( const double & u ); // Split line segments that cross a constant U value
    virtual void SplitSegsW( const double & w ); // Split line segments that cross a constant W value
    virtual void SplitSegsU( const double & u, vector<SSLineSeg> &splitvec ); // Split line segments that cross a constant U value
    virtual void SplitSegsW( const double & w, vector<SSLineSeg> &splitvec ); // Split line segments that cross a constant W value
    virtual void PrepareSplitVec();
    virtual void SetDisplaySuffix( int num );
    // Save, Load
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );

    virtual int GetFeaMaterialIndex();
    virtual void SetFeaMaterialIndex( int index );

    int m_Tag;
    IntParm m_TestType;
    IntParm m_MainSurfIndx;
    IntParm m_IncludeType; // Flag indicates whether or not to include wetted area of subsurf in parasite drag calcs

    // Parasite Drag Parms
    IntParm m_FFBodyEqnType;
    IntParm m_FFWingEqnType;
    Parm m_PercLam;
    Parm m_FFUser;
    Parm m_Q;
    Parm m_Roughness;
    Parm m_TeTwRatio;
    Parm m_TawTwRatio;

    IntParm m_IncludedElements;
    BoolParm m_DrawFeaPartFlag;
    IntParm m_FeaPropertyIndex;
    IntParm m_CapFeaPropertyIndex;

protected:
    string m_CompID; // Component ID used to match Subsurface to a specific geom
    int m_Type; // Type of SubSurface
    std::vector< DrawObj > m_SubSurfHighlightDO;
    bool m_UpdateDrawFlag;
    DrawObj m_SubSurfDO;
    vector<SSLineSeg> m_LVec; // Line Segment Vector
    vector< vector<SSLineSeg> > m_SplitLVec; // Split Line Vector
    vec3d m_LineColor; // Line Color Displayed when drawn on screen

    //std::vector< vec2d > m_PolyPnts;
    std::vector< std::vector< vec2d > > m_PolyPntsVec;
    bool m_PolyPntsReadyFlag;
    bool m_FirstSplit;
    bool m_PolyFlag; // Flag to indicate if the SubSurface is a Polygon ( this affects how it is treated in CFDMesh )

    virtual int CompNumDrawPnts( Geom* geom )
    {
        return -1;
    }
    virtual void ReorderSplitSegs( int ind, vector<SSLineSeg> &splitvec );

};

class SSLine : public SubSurface
{
    // Const U or Const W Line which really just creates an SSLineSeg
    // but with more user friendly parameters
public:

    SSLine( const string& compID, int type = vsp::SS_LINE );
    virtual ~SSLine();

    IntParm m_ConstType; // Either constant u or constant w line
    Parm m_ConstVal; // Either the const u value or const w value of line

    virtual bool Subtag( TTri* tri );
    virtual bool Subtag( const vec3d & center );

    virtual void Update();

protected:
    virtual int CompNumDrawPnts( Geom* geom );

};

class SSRectangle : public SubSurface
{
public:
    SSRectangle( const string& compID, int type = vsp::SS_RECTANGLE );
    virtual ~SSRectangle();

    Parm m_CenterU; // U location of square center
    Parm m_CenterW; // W location of square center
    Parm m_ULength; // Length of U dimension
    Parm m_WLength; // Length of W dimension
    Parm m_Theta; // Rotation Angle

    virtual void Update();
};

class SSEllipse : public SubSurface
{
public:
    SSEllipse( const string& compID, int type = vsp::SS_ELLIPSE );
    virtual ~SSEllipse();

    Parm m_CenterU; // U location of ellipse center
    Parm m_CenterW; // W location of ellipse center
    Parm m_ULength; // length of axis along u
    Parm m_WLength; // length of axis along w
    Parm m_Theta; // Rotation Angle
    IntParm m_Tess; // Number of line segments to break shape into

    virtual void Update();

};

class SSControlSurf : public SubSurface
{
public:
    SSControlSurf( const string& compID, int type = vsp::SS_CONTROL );
    virtual ~SSControlSurf();
    virtual void Update();
    virtual void UpdateDrawObjs();
    virtual void LoadDrawObjs( std::vector< DrawObj* >& draw_obj_vec );
    virtual void PrepareSplitVec();

    enum SS_CONTROL_SUBTYPE { UPPER_SURF, LOWER_SURF, BOTH_SURF };

    IntParm m_Tess; // Number of line segments to break shape into

    Parm m_UStart; // U Starting location for control surface
    Parm m_UEnd;   // U End location for control surface
    Parm m_StartLenFrac; // Percent chord that the control surface should occupy
    Parm m_EndLenFrac; // Percent chord that the control surface should occupy
    Parm m_StartLength; // Dimensional distance for control surface
    Parm m_EndLength; // Dimensional distance for control surface

    Parm m_StartAngle;
    Parm m_EndAngle;

    BoolParm m_StartAngleFlag;
    BoolParm m_EndAngleFlag;
    BoolParm m_SameAngleFlag;

    IntParm m_AbsRelFlag; // Flag to identify whether fractional or absolute should be used.
    BoolParm m_ConstFlag; // Flag to identify if start/end parameters are equal.
    BoolParm m_LEFlag; // Flag to indicate leading/trailing edge control surface.
    IntParm m_SurfType; // Defines if the control surface is on the upper/lower or both surfaces of the wing
    virtual void UpdatePolygonPnts();

    vector < vec3d > m_UWStart;
    vector < vec3d > m_UWEnd;

protected:

    vector < vec3d > m_UWStart01;
    vector < vec3d > m_UWEnd01;
    DrawObj m_HingeDO;
    DrawObj m_ArrowDO;

    int m_SepIndex;

    static void RefVec( vector < vec3d > &pt_vec, int nref );

};

class SSLineArray : public SubSurface
{
    // Array of Const U or Const W Lines used to define beam elements for FeaMesh
    public:

    SSLineArray( const string& compID, int type = vsp::SS_LINE_ARRAY );
    virtual ~SSLineArray();

    enum
    {
        CONST_U, CONST_W
    };

    virtual void Update();

    virtual void CalcNumLines();

    virtual SSLine* AddSSLine( double location, int ind );

    virtual int GetNumLines()
    {
        return m_NumLines;
    }

    IntParm m_ConstType;
    BoolParm m_PositiveDirectionFlag;
    Parm m_Spacing; // Spacing (U or W) between each constant line
    Parm m_StartLocation;
    Parm m_EndLocation;

    protected:
    virtual int CompNumDrawPnts( Geom* geom ); // Remove?

    int m_NumLines;
};

#endif
