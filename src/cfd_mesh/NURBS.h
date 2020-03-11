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

#endif
