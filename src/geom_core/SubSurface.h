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

    enum { GT, LT };
    int m_TestType;

    virtual ~SSLineSeg();
    virtual bool Subtag( TTri* tri ) const;
    virtual bool Subtag( const vec3d & center ) const;
    virtual TMesh* CreateTMesh();
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
    virtual void UpdateDrawObj( VspSurf* surf, Geom* geom, DrawObj& draw_obj, const int *num_pnts_ptr );
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
    SubSurface( string compID, int type );
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
    virtual std::vector< SSLineSeg >& GetSplitSegs()
    {
        return m_SplitLVec;
    }
    virtual bool GetPolyFlag()
    {
        return m_PolyFlag;
    }
    virtual void LoadDrawObjs( std::vector< DrawObj* >& draw_obj_vec );
    virtual void SetUpdateDraw( bool flag )
    {
        m_UpdateDrawFlag = flag;
    }

    virtual void SetLineColor( vec3d color )
    {
        m_LineColor = color;
    }

    enum { INSIDE, OUTSIDE };

    static std::string GetTypeName( int type );

    virtual bool Subtag( TTri* tri ); // Method to subtag triangles from TMesh.
    virtual bool Subtag( const vec3d & center );
    virtual void Update();
    virtual std::vector< TMesh* > CreateTMeshVec(); // Method to create a TMeshVector
    virtual void UpdateDrawObjs(); // Method to create lines to draw
    virtual void SplitSegs( const vector<int> & split_u, const vector<int> & split_w ); // Split line segments for CfdMesh surfaces
    virtual void SplitSegsU( const double & u ); // Split line segments that cross a constant U value
    virtual void SplitSegsW( const double & w ); // Split line segments that cross a constant W value
    virtual void CleanUpSplitVec();
    virtual void PrepareSplitVec();
    virtual void SetDisplaySuffix( int num );
    // Save, Load
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );

    int m_Tag;
    IntParm m_TestType;

protected:
    string m_CompID; // Component ID used to match Subsurface to a specific geom
    int m_Type; // Type of SubSurface
    std::vector< DrawObj > m_DrawObjVec;
    bool m_UpdateDrawFlag;
    vector<SSLineSeg> m_LVec; // Line Segment Vector
    vector<SSLineSeg> m_SplitLVec; // Split Line Vector
    vec3d m_LineColor; // Line Color Displayed when drawn on screen

    std::vector< vec2d > m_PolyPnts;
    bool m_PolyPntsReadyFlag;
    bool m_FirstSplit;
    bool m_PolyFlag; // Flag to indicate if the SubSurface is a Polygon ( this affects how it is treated in CFDMesh )

    virtual int CompNumDrawPnts( Geom* geom )
    {
        return -1;
    }
    virtual void ReorderSplitSegs( int ind );

};

class SSLine : public SubSurface
{
    // Const U or Const W Line which really just creates an SSLineSeg
    // but with more user friendly parameters
public:

    SSLine( string compID, int type = vsp::SS_LINE );
    virtual ~SSLine();

    enum { CONST_U, CONST_W };
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
    SSRectangle( string compID, int type = vsp::SS_RECTANGLE );
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
    SSEllipse( string compID, int type = vsp::SS_ELLIPSE );
    virtual ~SSEllipse();

    Parm m_CenterU; // U location of ellipse center
    Parm m_CenterW; // W location of ellipse center
    Parm m_ULength; // length of axis along u
    Parm m_WLength; // length of axis along w
    Parm m_Theta; // Rotation Angle
    IntParm m_Tess; // Number of line segments to break shape into

    virtual void Update();
    virtual void UpdateLVecSize();

};

#endif
