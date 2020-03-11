//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// NURBS.h
// Justin Gravett (ESAero)
//
// Data structures for defining NURBS (Non-Uniform Rational B-Spline) curves, 
// surfaces, and loops (chains) and functions to extract NURBS data from OpenVSP 
// Bezier surfaces and export to various CAD formats. 
//
//////////////////////////////////////////////////////////////////////

#if !defined(NURBS_NURBS__INCLUDED_)
#define NURBS_NURBS__INCLUDED_

#include "SCurve.h"
#include "STEPutil.h"


// Describes a NURBS curve, formed from a Bezier surface curve (SCurve). Can be of border 
// or intersection type.
class NURBS_Curve
{
public:

    NURBS_Curve();
    virtual ~NURBS_Curve() {};

    // Initialize the NURBS curve from an SCurve. Control points are extracted and 
    // saved to m_control_pnts_xyz.
    void InitNURBSCurve( SCurve curve );

    // Define the NURBS curve as a SdaiEdge_curve
    void WriteSTEPEdge( STEPutil* step, bool mergepnts );

    // Defines m_IGES_Edge by as an IGES type 126 entity
    void WriteIGESEdge( IGESutil* iges );

    // Flag is true if the curve is a border curve
    bool m_BorderFlag;

    // Flag is true if the curve is an intersection curve
    bool m_InternalFlag;

    // Flag indicating the curve is a sub-surface curve
    bool m_SubSurfFlag;

    // Flag that identifies if the intersection curve is between two FEA Parts
    bool m_StructIntersectFlag;

    // Vector of points decribing the curve
    vector < vec3d > m_PntVec;

    // Parent surface indexes
    int m_SurfA_ID;
    int m_SurfB_ID;

    // Pointers for the STEP representation of the NURBS curve
    SdaiVertex_point* m_STEP_Start_Vert;
    SdaiVertex_point* m_STEP_End_Vert;
    SdaiEdge_curve* m_STEP_Edge;

    // Pointer for the IGES representation of the NURBS curve
    DLL_IGES_ENTITY_126* m_IGES_Edge;

    // Type of intersection curve, such as transparent or structure
    int m_SurfIntersectType;

    // Relative tolerance for merging points through nanoflann based on parent surface bounding box
    double m_MergeTol;

    // Degree of the curve to define the NURBS curve
    int m_Deg;

protected:

    // Bounding box of curve. Used to identify relative tolerances
    BndBox m_BBox;

};

// A completely closed NURBS curve. Can be composed of border and/or intersection curves. 
class NURBS_Loop
{
public:

    NURBS_Loop();
    virtual ~NURBS_Loop() {};

    // Create a NURBS loop from an input point vector.
    void SetPntVec( const vector < vec3d >& pnt_vec );

    // Get the Type 126 NURBS curve pointer for each ordered curve in the loop.
    // If the Type 126 entitiy is not yet defined, WriteIGESEdge is called.
    vector < DLL_IGES_ENTITY_126* > GetIGESEdges( IGESutil* iges );

    // Write the NURBS loop to IGES and trim the parent 128 type entity to form a
    // type 144 entity. 
    DLL_IGES_ENTITY_144 WriteIGESLoop( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf );

    // Add a cutout or hole to a boundedor trimmed surface
    void WriteIGESCutout( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, DLL_IGES_ENTITY_144& trimmed_surf );

    // Write the NURBS loop to STEP
    SdaiEdge_loop* WriteSTEPLoop( STEPutil* step );

    // Based on all control points for theloop, get the bounding box
    BndBox GetBndBox();

    // Flag is true if the loop is composed of intersection curves only
    bool m_IntersectLoopFlag;

    // Flag is true if the loop is composed of border curves only
    bool m_BorderLoopFlag;

    // Flag is true if the loop bounds an internal surface
    bool m_InternalLoopFlag;

    // Flag is true if the loop is closed
    bool m_ClosedFlag;

    // vector of ordered NURBS_Curves that make up a loop. The orientation of the curve is saved
    vector < pair < NURBS_Curve, bool > > m_OrderedCurves;

protected:

    // Chain of points that defines the loop
    vector < vec3d > m_PntVec;

};

#endif
