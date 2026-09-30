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

class STEP_Topology;


// The exact curve of the parameter line of its surface that crv runs along: the control points of
// a piecewise Bezier curve of degree deg, and the line's parameter at each segment end.  ucon says
// the line is one of constant u, cval is that constant, and ta and tb are where crv starts and
// ends along it.  Returns false where crv is not on a parameter line.
bool ParameterLineCurve( SCurve &crv, vector < vec3d > &cp_vec, vector < double > &break_vec, int &deg,
                         bool &ucon, double &cval, double &ta, double &tb );

// Describes a NURBS curve, formed from a Bezier surface curve (SCurve). Can be of border 
// or intersection type.
class NURBS_Curve
{
public:

    NURBS_Curve();
    virtual ~NURBS_Curve() {};

    // Initialize the curve from the SCurves of its two parent surfaces as a polyline only:
    // m_PntVec adapted to within curve_tol of the intersection, relative to a segment's
    // length, and the same points in the parameter space of both parents alongside them.
    void InitPolyline( SCurve curveA, SCurve curveB, double curve_tol );

    // Initialize the curve as it is written to trimmed CAD: the CAD curve, and m_PntVec and
    // the parameters of both parents as points along it and along its image on each.  tol
    // bounds how far the CAD curve may stray from either parent.
    void InitCAD( SCurve curveA, SCurve curveB, double tol );

    // Run the curve the other way
    void Reverse();

    // Run the CAD curve the other way
    void ReverseCAD();

    // Write the NURBS curve as a SdaiEdge_curve running between the given vertices
    SdaiEdge_curve* WriteSTEPEdge( STEPutil* step, SdaiVertex_point* start_vert, SdaiVertex_point* end_vert,
                                   const string& label = "", bool mergepnts = false ) const;

    // Defines m_IGES_Edge by as an IGES type 126 entity
    void WriteIGESEdge( IGESutil* iges, const string& label = "" );

    // Flag is true if the curve is a border curve
    bool m_BorderFlag;

    // Flag is true if the curve is an intersection curve
    bool m_InternalFlag;

    // Flag indicating the curve is a sub-surface curve
    bool m_SubSurfFlag;

    // Vector of points describing the curve, which the loops are built from
    vector < vec3d > m_PntVec;

    // The curve in the parametric space of each of its two parent surfaces, one entry
    // per m_PntVec point.  m_PntVec says where the curve runs, but not which side of it
    // a parent surface lies on; that is only answerable in the parameter space of the
    // surface in question.
    vector < vec3d > m_UWPntVec_A;
    vector < vec3d > m_UWPntVec_B;

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

    // Pointer for the IGES representation of the NURBS curve
    std::shared_ptr < DLL_IGES_ENTITY_126 > m_IGES_Edge;

    // Relative tolerance for merging points through nanoflann based on parent surface bounding box
    double m_MergeTol;

    // The curve as it is written to a CAD file: a piecewise Bezier curve of degree m_CADDeg
    // through m_CADPntVec, the segments sharing their end points, with m_CADBreakVec the
    // parameter at each segment end.  An intersection curve is written adapted: a cubic
    // through points put on both parents.
    //
    // A border curve is a parameter line of its surface, and is written as exactly that.
    vector < vec3d > m_CADPntVec;
    vector < double > m_CADBreakVec;
    int m_CADDeg;

    // The CAD curve in the parameter space of each parent: a piecewise Bezier curve of degree
    // m_CADUWDeg through the (u, w) held in x and y, with the break vector the curve's
    // parameter at each segment end.
    int m_CADUWDeg;
    vector < vec3d > m_CADUWPntVec_A;
    vector < vec3d > m_CADUWPntVec_B;
    vector < double > m_CADUWBreakVec_A;
    vector < double > m_CADUWBreakVec_B;

    // Label for the NURBS curve
    string m_Label;

    int m_CurveID;
protected:

    // Make the CAD curve the exact parameter line of the surface a border curve runs along --
    // crvA's, or crvB's where crvA is not on one -- and its pcurve on the surface across the
    // border straight between points close enough that it keeps within half tol of the
    // border.  Returns false, leaving the CAD curve alone, where neither curve is on a
    // parameter line.
    bool BuildBorderCADCurve( SCurve &crvA, SCurve &crvB, double tol );

    // The bounding box of curveA's surface, and the point merge tolerance taken from it
    void SetMergeTol( SCurve &curveA );

    // Run breakpoints from the other end over the same range
    static void ReverseBreakVec( vector < double > &break_vec );

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

    // How the loop's curves were made: by intersection, as parameter lines of their surfaces,
    // or a mix
    CURVE_CREATION IGESCurveCreation() const;

    // The loop in the parameters of surface surf_id, whose surface is surf: each ordered curve's
    // image there, and a straight line across each gap between them, where the loop leaves out a
    // curve that collapses to a point.  A gap whose ends are not one point in space is reported.
    // Empty where a curve has no image on the surface.  flip_flag mirrors w, for a surface
    // written reversed in w.
    vector < DLL_IGES_ENTITY_126 > GetIGESUWEdges( IGESutil* iges, int surf_id, const piecewise_surface_type &surf, bool flip_flag ) const;

    // Write the NURBS loop to IGES and trim the parent 128 type entity, surface surf_id, to form
    // a type 144 entity.
    DLL_IGES_ENTITY_144 WriteIGESLoop( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, int surf_id, const piecewise_surface_type &surf,
                                       bool flip_flag, const string& label = "" );

    // Add a cutout or hole to a boundedor trimmed surface
    void WriteIGESCutout( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, DLL_IGES_ENTITY_144& trimmed_surf, int surf_id,
                          const piecewise_surface_type &surf, bool flip_flag, const string& label = "" );

    // Write the NURBS loop to STEP, taking its edges from the file's topology
    SdaiEdge_loop* WriteSTEPLoop( STEPutil* step, STEP_Topology* topo, bool mergepts = false );

    // Write the NURBS loop to STEP as a bound of a face on the given surface
    SdaiFace_bound* WriteSTEPBound( STEPutil* step, STEP_Topology* topo, int surf_id, bool cutout_flag,
                                    bool flip_flag, bool mergepts = false );

    // Based on all control points for theloop, get the bounding box
    BndBox GetBndBox();

    // Signed area of the loop in the parameter space of one of its parent surfaces: positive
    // where the loop runs counter-clockwise there
    double SignedAreaUW( int surf_id ) const;

    // The loop as a polygon in the parameter space of one of its parent surfaces
    void GetUWPolygon( int surf_id, vector < vec3d > &uw_vec ) const;

    // Whether a point of that parameter space is inside the loop
    bool ContainsUW( int surf_id, const vec3d &uw ) const;

    // Which way the loop runs as the face sees it: +1 when the face lies to its left, -1 when
    // to its right, and 0 when the loop encloses no area to tell by.  An outer boundary with
    // the face on its left runs counter-clockwise and a hole clockwise; flip_flag reverses
    // both, for a surface whose normal points into the body.
    int Sense( int surf_id, bool cutout_flag, bool flip_flag ) const;

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

    void MakeExtLoopVec( vector < NURBS_Loop > & ext_loop_vec, vector < NURBS_Loop > & cutout_vec );

    // Whether any external loop is complete, so the surface bounds a face at all
    bool HasClosedExtLoop();

    // The external loop a cutout lies in, the innermost where they nest; -1 where none holds it
    int CutoutOwner( const vector < NURBS_Loop > & ext_loop_vec, const NURBS_Loop & cutout ) const;

    // Write the NURBS loops for this NURBS surface to IGES, trimming the parent surface
    // in the process. 
    void WriteIGESLoops( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, const string& label = "" );

    // Write the NURBS surface to STEP and optionally merge points that are close together
    SdaiSurface* WriteSTEPSurf( STEPutil* step, const string& label = "", bool mergepts = false );

    // Write the NURBS loops for this NURBS surface to STEP, trimming the parent surface
    // in the process. 
    // Write the faces of this surface, and in face_ids the number topo gave each
    vector < SdaiAdvanced_face* > WriteSTEPLoops( STEPutil* step, STEP_Topology* topo, SdaiSurface* surf, vector < int > &face_ids,
                                                  const string& label = "", bool mergepts = false );

    // Identifies the internal and external NURBS curves on the surface, organizes
    // them into connected chains, and forms loops.  
    void BuildNURBSLoopMap();

    // Find all NURBS surves associated with this NURBS surface
    vector < NURBS_Curve > MatchNURBSCurves( const vector < NURBS_Curve > &all_curve_vec );

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

    // Set when the surface's parametric normal points into the body rather than out of it
    bool m_FlipFlag;

    // All NURBS curves associated with the surface
    vector < NURBS_Curve > m_NURBSCurveVec;

    // Nurbs loops for the surface. Will always contain at least one for the surface border
    vector < NURBS_Loop > m_NURBSLoopVec;

    // Label for the NURBS surface
    string m_Label;

protected:

    // Organize a vector of NURBS curves into a map of ordered chains, where all NURBS curves 
    // for a map index are connected.
    unordered_map< int, vector < pair < NURBS_Curve, bool > > > BuildOrderedChains( vector < NURBS_Curve > chain_vec );

    // Split each chain where it comes back to a point it already passed, so every loop is simple
    unordered_map< int, vector < pair < NURBS_Curve, bool > > > SplitPinchedChains( const unordered_map< int, vector < pair < NURBS_Curve, bool > > > &chain_map ) const;

    // Transform vectors of connected NURBS curves into individual NURBS loops.
    vector < NURBS_Loop > MergeOrderedChains( unordered_map< int, vector < pair < NURBS_Curve, bool > > > ordered_chain_map );

    // NURBS Surface definition
    piecewise_surface_type* m_Surf;

    // Bounding box of the surface, used to scale tolerances appropriately
    BndBox m_BBox;

};

// The vertices and edges of a trimmed STEP file.  Each curve is written once, as one edge,
// however many faces it bounds.  Curve ends that some loop walks straight between share a
// vertex, so the faces meet in the file as they do in the model, whatever small gap the
// intersection left between the curves.
class STEP_Topology
{
public:

    STEP_Topology( const vector < NURBS_Curve > &curve_vec, const vector < NURBS_Surface > &surf_vec );

    // The edge for a curve, written the first time it is asked for
    SdaiEdge_curve* GetEdge( STEPutil* step, int curve_id, bool mergepts );

    // The surface written for a NURBS surface, which a curve on it gets a pcurve on
    void SetSurf( int surf_id, SdaiSurface* surf )
    {
        m_SurfMap[ surf_id ] = surf;
    }

    // Largest distance from a curve end to the vertex it was given, over every edge written
    double GetMaxEndGap() const
    {
        return m_MaxEndGap;
    }

    // Numbers for n new faces, the first of them returned
    int NewFaces( int n )
    {
        int first = m_NumFace;
        m_NumFace += n;
        return first;
    }

    // Count the edges the bounds written from here on use against a face
    void SetFace( int face )
    {
        m_Face = face;
    }

    // The faces given, in shells of faces joined through the edges they share, each in face order
    // and the shells in the order of their first faces; a shell is closed where every edge its
    // faces use is used by exactly two of them
    void Shells( const vector < int > &face_vec, vector < vector < int > > &shell_vec, vector < bool > &closed_vec ) const;

protected:

    // Index of one end of a curve in m_EndVertVec
    static int EndIndex( int curve_id, bool end_flag )
    {
        if ( end_flag )
        {
            return 2 * curve_id + 1;
        }
        return 2 * curve_id;
    }

    int FindRoot( int i );

    SdaiVertex_point* GetVertex( STEPutil* step, int vert );

    const vector < NURBS_Curve > &m_CurveVec;

    // Vertex number for each end of each curve, start then end
    vector < int > m_EndVertVec;

    // Position of each vertex: the mean of the curve ends that share it
    vector < vec3d > m_VertPntVec;

    vector < SdaiVertex_point* > m_VertVec;
    vector < SdaiEdge_curve* > m_EdgeVec;

    // The curves each face's bounds use, once for each use
    int m_NumFace;
    int m_Face;
    unordered_map < int, vector < int > > m_FaceCurveMap;

    unordered_map < int, SdaiSurface* > m_SurfMap;

    double m_MaxEndGap;
};

#endif
