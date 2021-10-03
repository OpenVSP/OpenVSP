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
#include "CADutil.h"


// Describes a NURBS curve, formed from a Bezier surface curve (SCurve). Can be of border 
// or intersection type.
class NURBS_Curve
{
public:

    NURBS_Curve();
    virtual ~NURBS_Curve() {};

    // Initialize the NURBS curve from an SCurve. Control points are extracted and 
    // saved to m_control_pnts_xyz.
    void InitNURBSCurve( SCurve curve, double curve_tol );

    // Define the NURBS curve as a SdaiEdge_curve
    void WriteSTEPEdge( STEPutil* step, const string& label = "", bool mergepnts = false );

    // Defines m_IGES_Edge by as an IGES type 126 entity
    void WriteIGESEdge( IGESutil* iges, const string& label = "" );

    // Flag is true if the curve is a border curve
    bool m_BorderFlag;

    // Flag is true if the curve is an intersection curve
    bool m_InternalFlag;

    // Flag indicating the curve is a sub-surface curve
    bool m_SubSurfFlag;

    // Vector of points describing the curve
    vector < vec3d > m_PntVec;

    // Parent surface indexes
    int m_SurfA_ID;
    int m_SurfB_ID;

    // Parent surface CFD types
    int m_SurfA_Type;
    int m_SurfB_Type;

    // Flag that indicates if a curve is inside a negative surface
    bool m_InsideNegativeFlag;

    // Flag that indicates one of the parent surfaces of the curve is a wake
    bool m_WakeFlag;

    // Pointers for the STEP representation of the NURBS curve
    SdaiVertex_point* m_STEP_Start_Vert;
    SdaiVertex_point* m_STEP_End_Vert;
    SdaiEdge_curve* m_STEP_Edge;

    // Pointer for the IGES representation of the NURBS curve
    DLL_IGES_ENTITY_126* m_IGES_Edge;

    // Relative tolerance for merging points through nanoflann based on parent surface bounding box
    double m_MergeTol;

    // Degree of the curve to define the NURBS curve
    int m_Deg;

    // Label for the NURBS curve
    string m_Label;

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
    // If the Type 126 entity is not yet defined, WriteIGESEdge is called.
    vector < DLL_IGES_ENTITY_126* > GetIGESEdges( IGESutil* iges );

    // Write the NURBS loop to IGES and trim the parent 128 type entity to form a
    // type 144 entity. 
    DLL_IGES_ENTITY_144 WriteIGESLoop( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, const string& label = "" );

    // Add a cutout or hole to a boundedor trimmed surface
    void WriteIGESCutout( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, DLL_IGES_ENTITY_144& trimmed_surf, const string& label = "" );

    // Write the NURBS loop to STEP
    SdaiEdge_loop* WriteSTEPLoop( STEPutil* step, const string& label = "", bool mergepts = false );

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

    // Label for the NURBS loop
    string m_Label;

protected:

    // Chain of points that defines the loop
    vector < vec3d > m_PntVec;

};

// Defines a NURBS surface, created from a piecewise Bezier surface. Every NURBS surface 
// contains NURBS curves that define intersections and/or borders. Each surface will 
// at least one NURBS loop associated with it to describe the outer surface boundary. 
// NURBS loops define how a NURBS surface will be trimmed. 
class NURBS_Surface
{
public:

    NURBS_Surface();
    virtual ~NURBS_Surface() {};

    // Create a NURBS surface from a piecewise Bezier surface. The knot vectors,
    // number of points and number of patches in the U and V directions are 
    // saved as member variable.
    void InitNURBSSurf( Surf* surface );

    // Write the NURBS surface to IGES
    DLL_IGES_ENTITY_128 WriteIGESSurf( IGESutil* iges, const string& label = "" );

    // Write the NURBS loops for this NURBS surface to IGES, trimming the parent surface
    // in the process. 
    void WriteIGESLoops( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, const string& label = "" );

    // Write the NURBS surface to STEP and optionally merge points that are close together
    SdaiSurface* WriteSTEPSurf( STEPutil* step, const string& label = "", bool mergepts = false );

    // Write the NURBS loops for this NURBS surface to STEP, trimming the parent surface
    // in the process. 
    vector < SdaiAdvanced_face* > WriteSTEPLoops( STEPutil* step, SdaiSurface* surf, const string& label = "", bool mergepts = false );

    // Identifies the internal and external NURBS curves on the surface, organizes
    // them into connected chains, and forms loops.  
    void BuildNURBSLoopMap();

    // Find all NURBS surves associated with this NURBS surface
    vector < NURBS_Curve > MatchNURBSCurves( vector < NURBS_Curve > all_curve_vec );

    void SetNURBSCurveVec( const vector < NURBS_Curve >& curve_vec )
    {
        m_NURBSCurveVec = curve_vec;
    }
    vector < NURBS_Curve > GetNURBSCurveVec()
    {
        return m_NURBSCurveVec;
    }

    // Parent surface index
    int m_SurfID;

    // Surface type to identify how to perform trimming (i.e. disk or structure surface)
    int m_SurfType;

    // Identifies if the surface is a wake or not
    bool m_WakeFlag;

    // All NURBS curves associated with the surface
    vector < NURBS_Curve > m_NURBSCurveVec;

    // Nurbs loops for the surface. Will always contain at least one for the surface border
    vector < NURBS_Loop > m_NURBSLoopVec;

    // Label for the NURBS surface
    string m_Label;

protected:

    // Organize a vector of NURBS curves into a map of ordered chains, where all NURBS curves 
    // for a map index are connected.
    map< int, vector < pair < NURBS_Curve, bool > > > BuildOrderedChains( vector < NURBS_Curve > chain_vec );

    // Transform vectors of connected NURBS curves into individual NURBS loops.  
    vector < NURBS_Loop > MergeOrderedChains( map< int, vector < pair < NURBS_Curve, bool > > > ordered_chain_map );

    // NURBS Surface definition
    piecewise_surface_type* m_Surf;

    // Variables for representing the NURBS surface as a simplified planar surface if possible 
    bool m_IsPlanar;
    vec3d m_Tangent;
    vec3d m_Norm;
    vec3d m_Center;

    // Bounding box of the surface, used to scale tolerances appropriately
    BndBox m_BBox;

};

#endif
