//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NURBS (Non-Uniform Rational B-Spline)
//
//////////////////////////////////////////////////////////////////////

#include "NURBS.h"


//////////////////////////////////////////////////////
//================= NURBS_Curve ====================//
//////////////////////////////////////////////////////

NURBS_Curve::NURBS_Curve()
{
    m_BorderFlag = false;
    m_InternalFlag = false;
    m_SubSurfFlag = false;
    m_StructIntersectFlag = false;
    m_SurfA_ID = -1;
    m_SurfB_ID = -1;
    m_MergeTol = 0;
    m_Deg = 1;
    m_STEP_Start_Vert = NULL;
    m_STEP_End_Vert = NULL;
    m_STEP_Edge = NULL;
    m_IGES_Edge = NULL;
    m_SurfIntersectType = vsp::CFD_NORMAL;
    m_BBox = BndBox();
}

void NURBS_Curve::InitNURBSCurve( SCurve curve )
{
    Bezier_curve xyzcrvA = curve.GetUWCrv();
    xyzcrvA.TessAdaptXYZ( *curve.GetSurf(), m_PntVec, 5e-3, 16 );

    m_BBox = curve.GetSurf()->GetBBox();

    m_MergeTol = m_BBox.DiagDist() * 1.0e-10;

    if ( m_MergeTol < 1.0e-10 )
    {
        m_MergeTol = 1.0e-10;
    }
}

void NURBS_Curve::WriteIGESEdge( IGESutil* iges )
{
    m_IGES_Edge = new DLL_IGES_ENTITY_126( iges->MakeCurve( m_PntVec, m_Deg ) );
}

void NURBS_Curve::WriteSTEPEdge( STEPutil* step, bool mergepnts )
{
    // Identify the start and end vertex points
    m_STEP_Start_Vert = step->MakeVertex( m_PntVec[0] );

    Logical closed_curve;

    if ( dist( m_PntVec[0], m_PntVec.back() ) < ( m_BBox.DiagDist() * 1.0e-6 ) )
    {
        m_STEP_End_Vert = m_STEP_Start_Vert;
        closed_curve = LTrue;
    }
    else
    {
        m_STEP_End_Vert = step->MakeVertex( m_PntVec.back() );
        closed_curve = LFalse;
    }

    // Identify the edge
    SdaiB_spline_curve_with_knots* curve = step->MakeCurve( m_PntVec, m_Deg, closed_curve, mergepnts, m_MergeTol );

    SdaiEdge_curve* edge_crv = (SdaiEdge_curve*)step->registry->ObjCreate( "EDGE_CURVE" );
    step->instance_list->Append( (SDAI_Application_instance*)edge_crv, completeSE );
    edge_crv->edge_geometry_( curve );
    edge_crv->edge_start_( m_STEP_Start_Vert );
    edge_crv->edge_end_( m_STEP_End_Vert );
    edge_crv->same_sense_( BTrue ); // direction set by oriented edge
    edge_crv->name_( "''" );

    m_STEP_Edge = edge_crv;
}
