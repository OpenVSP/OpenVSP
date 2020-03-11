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

//////////////////////////////////////////////////////
//================= NURBS_Loop ====================//
//////////////////////////////////////////////////////

NURBS_Loop::NURBS_Loop()
{
    m_IntersectLoopFlag = false;
    m_BorderLoopFlag = false;
    m_InternalLoopFlag = false;
    m_ClosedFlag = false;
}

void NURBS_Loop::SetPntVec( const vector < vec3d >& pnt_vec )
{
    m_PntVec = pnt_vec;

    // Check for closure
    if ( dist( m_PntVec.back(), m_PntVec.front() ) < FLT_EPSILON )
    {
        m_ClosedFlag = true;
    }
    else
    {
        m_ClosedFlag = false;
    }
};

vector < DLL_IGES_ENTITY_126* > NURBS_Loop::GetIGESEdges( IGESutil* iges )
{
    vector < DLL_IGES_ENTITY_126* > nurbs_vec( m_OrderedCurves.size() );

    for ( size_t i = 0; i < m_OrderedCurves.size(); i++ )
    {
        if ( !m_OrderedCurves[i].first.m_IGES_Edge )
        {
             // Create the curve if it is undefined
            m_OrderedCurves[i].first.WriteIGESEdge( iges );
        }

        // Note: No need to reverse vectors -> already done when building loops and IGES import
        // should automatically identify proper orientation for the type 102 composite entity
        nurbs_vec[i] = m_OrderedCurves[i].first.m_IGES_Edge;
    }

    return nurbs_vec;
}

DLL_IGES_ENTITY_144 NURBS_Loop::WriteIGESLoop( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf )
{
    if ( !m_ClosedFlag )
    {
        printf( "ERROR: Incomplete IGES Loop \n" );
    }

    vector < DLL_IGES_ENTITY_126* > nurbs_vec = GetIGESEdges( iges );

    return iges->MakeLoop( parent_surf, nurbs_vec );
}

void NURBS_Loop::WriteIGESCutout( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, DLL_IGES_ENTITY_144& trimmed_surf )
{
    if ( !m_ClosedFlag )
    {
        printf( "ERROR: Incomplete IGES Loop \n" );
        return;
    }

    vector < DLL_IGES_ENTITY_126* > nurbs_vec = GetIGESEdges( iges );

    iges->MakeCutout( parent_surf, trimmed_surf, nurbs_vec );
}

SdaiEdge_loop* NURBS_Loop::WriteSTEPLoop( STEPutil* step )
{
    if ( !m_ClosedFlag )
    {
        printf( "ERROR: Incomplete STEP Loop \n" );
        return NULL;
    }

    vector < SdaiOriented_edge* > or_edge_vec;

    for ( size_t i = 0; i < m_OrderedCurves.size(); i++ )
    {
        // Created oriented edge from NURBS_Curve edge
        SdaiOriented_edge* or_edge = (SdaiOriented_edge*)step->registry->ObjCreate( "ORIENTED_EDGE" );
        step->instance_list->Append( (SDAI_Application_instance*)or_edge, completeSE );
        or_edge->edge_element_( m_OrderedCurves[i].first.m_STEP_Edge );
        //TODO: Add edge start and end?

        Boolean orient = BTrue;
        if ( !m_OrderedCurves[i].second )
        {
            orient = BFalse;
        }

        or_edge->orientation_( orient );
        or_edge->name_( "''" );

        or_edge_vec.push_back( or_edge );
    }

    SdaiEdge_loop* loop = (SdaiEdge_loop*)step->registry->ObjCreate( "EDGE_LOOP" );
    step->instance_list->Append( (SDAI_Application_instance*)loop, completeSE );
    loop->name_( "''" );

    // Workaround for SdaiEdge_loop edge_list_ not being written out to file. 
    // https://github.com/stepcode/stepcode/issues/251
    //
    // Why doesn't SdaiEdge_loop's edge_list_() function give use
    // the edge_list from the SdaiPath??  Initialized to NULL and
    // crashes - what good is it?  Have to get at the internal
    // SdaiPath directly to build something that STEPwrite will output.
    SdaiPath* e_loop_path = (SdaiPath*)loop->GetNextMiEntity();

    std::ostringstream loop_ss;

    for ( size_t i = 0; i < or_edge_vec.size(); i++ )
    {
        loop_ss << "#" << or_edge_vec[i]->GetFileId();

        if ( i < or_edge_vec.size() - 1 )
        {
            loop_ss << ", ";
        }
    }

    e_loop_path->edge_list_()->AddNode( new GenericAggrNode( loop_ss.str().c_str() ) );

    return loop;
}

BndBox NURBS_Loop::GetBndBox()
{
    BndBox bbox;

    for ( size_t i = 0; i < m_control_pnts_xyz.size(); i++ )
    {
        bbox.Update( m_control_pnts_xyz[i] );
    }

    return bbox;
}
