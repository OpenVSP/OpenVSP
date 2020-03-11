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

    for ( size_t i = 0; i < m_PntVec.size(); i++ )
    {
        bbox.Update( m_PntVec[i] );
    }

    return bbox;
}

//////////////////////////////////////////////////////
//================ NURBS_Surface ===================//
//////////////////////////////////////////////////////

NURBS_Surface::NURBS_Surface()
{
    m_SurfID = -1;
    m_Surf = NULL;
    m_IsPlanar = false;
    m_SurfType = vsp::CFD_NORMAL;
}

void NURBS_Surface::InitNURBSSurf( Surf* surface )
{
    m_SurfID = surface->GetSurfID();
    m_BBox = surface->GetBBox();

    m_Surf = surface->GetSurfCore()->GetSurf();

    // Check if the surface is planar (simplified representation)
    double u_mid = ( m_Surf->get_u0() + m_Surf->get_umax() ) / 2;
    double w_mid = ( m_Surf->get_v0() + m_Surf->get_vmax() ) / 2;

    // Check if the surface is planar:
    double ka, kg, k1, k2;

    surface->GetSurfCore()->CompCurvature( u_mid, w_mid, ka, kg, k1, k2 );

    if ( abs( k1 ) < FLT_EPSILON && abs( k2 ) < FLT_EPSILON )
    {
        // Both principal curvatures are 0
        m_IsPlanar = true;
    }

    m_Center = surface->GetSurfCore()->CompPnt( u_mid, w_mid );
    m_Norm = surface->GetSurfCore()->CompNorm( u_mid, w_mid );
    m_Tangent = surface->GetSurfCore()->CompTanU( u_mid, w_mid );

    m_Tangent.normalize();
    m_Norm.normalize();
}

DLL_IGES_ENTITY_128 NURBS_Surface::WriteIGESSurf( IGESutil* iges, const string& label )
{
    return iges->MakeSurf( *m_Surf, label.c_str() );
}

SdaiSurface* NURBS_Surface::WriteSTEPSurf( STEPutil* step, bool mergepts )
{
    SdaiSurface* ret_surf = NULL;

    if ( m_IsPlanar )
    {
        ret_surf = step->MakePlane( m_Center, m_Norm, m_Tangent );
    }
    else
    {
        //==== Compute Tol ====//
        double merge_tol = m_BBox.DiagDist() * 1.0e-10;

        if ( merge_tol < 1.0e-10 )
        {
            merge_tol = 1.0e-10;
        }

        ret_surf = step->MakeSurf( *m_Surf, mergepts, merge_tol );
    }

    return ret_surf;
}

map< int, vector < pair < NURBS_Curve, bool > > > NURBS_Surface::BuildOrderedChains( vector < NURBS_Curve > chain_vec )
{
    // Starting with 1 NURBS curve, march along the others to form an ordered chain.
    int map_ind = 0; // each map index represents a group of attached NURBS_Curve chains

    double tol = m_BBox.DiagDist() * 1e-2; // Note: This tolerance is kept large in order to still form loops when there are large gaps in intersection curves

    map< int, vector < pair < NURBS_Curve, bool > > > return_curve_map;

    while ( chain_vec.size() > 0 )
    {
        bool transparent_chain = false;

        // Identify any component and transparent surface (disk) intersection segments
        for ( size_t i = 0; i < chain_vec.size(); i++ )
        {
            if ( chain_vec[i].m_SurfIntersectType == vsp::CFD_TRANSPARENT && !chain_vec[i].m_BorderFlag && !chain_vec[i].m_InternalFlag )
            {
                return_curve_map[map_ind].push_back( make_pair( chain_vec[i], true ) );
                chain_vec.erase( chain_vec.begin() + i );
                transparent_chain = true;
                break;
            }
        }

        if ( !transparent_chain )
        {
            return_curve_map[map_ind].push_back( make_pair( chain_vec[0], true ) );
            chain_vec.erase( chain_vec.begin() );
        }

        size_t num_int_curve = chain_vec.size();

        for ( size_t i = 0; i < num_int_curve; i++ )
        {
            vec3d curr_back_pnt = return_curve_map[map_ind].back().first.m_PntVec.back();
            vec3d curr_front_pnt = return_curve_map[map_ind].front().first.m_PntVec.front();

            double closest_dist = 1e9;
            bool orientation; // Identifies the orientation of the curve for this particular loop
            int chain_ind;

            for ( size_t j = 0; j < chain_vec.size(); j++ )
            {
                vec3d next_front_pnt = chain_vec[j].m_PntVec.front();
                vec3d next_back_pnt = chain_vec[j].m_PntVec.back();

                double dist_front_front = dist( curr_front_pnt, next_front_pnt );
                double dist_front_back = dist( curr_front_pnt, next_back_pnt );

                if ( dist_front_front < closest_dist )
                {
                    orientation = false;
                    chain_ind = (int)j;
                    closest_dist = dist_front_front;
                }

                if ( dist_front_back < closest_dist )
                {
                    orientation = true;
                    chain_ind = (int)j;
                    closest_dist = dist_front_back;
                }
            }

            vector < vec3d > chain_cp_vec = chain_vec[chain_ind].m_PntVec;

            if ( closest_dist < tol )
            {
                if ( !orientation )
                {
                    reverse( chain_vec[chain_ind].m_PntVec.begin(), chain_vec[chain_ind].m_PntVec.end() );
                }

                return_curve_map[map_ind].insert( return_curve_map[map_ind].begin(), make_pair( chain_vec[chain_ind], orientation ) );

                chain_vec.erase( chain_vec.begin() + chain_ind );
            }
            else
            {
                break;
            }

            if ( transparent_chain && ( dist( curr_back_pnt, chain_cp_vec.front() ) < tol || dist( curr_back_pnt, chain_cp_vec.back() ) < tol ) )
            {
                if ( chain_vec.size() > 0 )
                {
                    chain_vec.push_back( return_curve_map[map_ind].back().first );
                }
                break;
            }
        }

        vec3d curr_back_pnt = return_curve_map[map_ind].back().first.m_PntVec.back();
        vec3d curr_front_pnt = return_curve_map[map_ind].front().first.m_PntVec.front();

        if ( dist( curr_back_pnt, curr_front_pnt ) < tol && dist( curr_back_pnt, curr_front_pnt ) > FLT_EPSILON )
        {
            vec3d midpnt = ( curr_back_pnt + curr_front_pnt ) / 2;

            return_curve_map[map_ind].back().first.m_PntVec.back() = midpnt;
            return_curve_map[map_ind].front().first.m_PntVec.front() = midpnt;
            // Also do next/previous control point?
        }

        map_ind++;
    }

    return return_curve_map;
}

vector < NURBS_Loop > NURBS_Surface::MergeOrderedChains( map< int, vector < pair < NURBS_Curve, bool > > > ordered_chain_map )
{
    vector < NURBS_Loop > loop_vec( ordered_chain_map.size() );

    map< int, vector < pair < NURBS_Curve, bool > > >::iterator it;

    for ( it = ordered_chain_map.begin(); it != ordered_chain_map.end(); ++it )
    {
        vector < vec3d > complete_pnt_vec;
        bool border_loop_flag = true, intersect_loop_flag = true;

        for ( size_t i = 0; i < ( it )->second.size(); i++ )
        {
            vector < vec3d > current_pnt_vec = ( it )->second[i].first.m_PntVec;

            if ( complete_pnt_vec.size() > 0 && dist( complete_pnt_vec.back(), current_pnt_vec.front() ) > FLT_EPSILON )
            {
                complete_pnt_vec.back() = current_pnt_vec.front();
                complete_pnt_vec[complete_pnt_vec.size() - 2] = current_pnt_vec.front();
            }

            complete_pnt_vec.insert( complete_pnt_vec.end(), current_pnt_vec.begin(), current_pnt_vec.end() );

            if ( ( it )->second[i].first.m_BorderFlag )
            {
                intersect_loop_flag = false;
            }
            else
            {
                border_loop_flag = false;
            }
        }

        loop_vec[( it )->first].SetPntVec( complete_pnt_vec );
        loop_vec[( it )->first].m_BorderLoopFlag = border_loop_flag;
        loop_vec[( it )->first].m_IntersectLoopFlag = intersect_loop_flag;
        loop_vec[( it )->first].m_OrderedCurves = ( it )->second;
    }

    return loop_vec;
}

void NURBS_Surface::BuildNURBSLoopMap()
{
    // Ientify vectors of NURBS available as internal or external loops
    vector < NURBS_Curve > internal_curve_vec, external_curve_vec;

    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        if ( m_NURBSCurveVec[i].m_SubSurfFlag || m_NURBSCurveVec[i].m_StructIntersectFlag )
        {
            continue;
        }
        else if ( !m_NURBSCurveVec[i].m_BorderFlag )
        {
            if ( m_NURBSCurveVec[i].m_SurfIntersectType == vsp::CFD_TRANSPARENT && m_SurfType != vsp::CFD_TRANSPARENT )
            {
                // Don't trim non-transparent surfaces with intersection curves made with a transparent surface. The non-transparent
                // surface will not be broken into two surfaces at the intersection curve. 
                continue;
            }
            else if ( m_NURBSCurveVec[i].m_SurfIntersectType == vsp::CFD_STRUCTURE && m_SurfType != vsp::CFD_STRUCTURE )
            {
                // Don't trim non-structure surfaces with intersection curves made with a structure surface. The non-structure
                // surface will not be broken into two surfaces at the intersection curve. 
                continue;
            }
            else if ( m_NURBSCurveVec[i].m_SurfIntersectType == vsp::CFD_STRUCTURE && m_SurfType == vsp::CFD_STRUCTURE )
            {
                internal_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
            else if ( !m_NURBSCurveVec[i].m_InternalFlag )
            {
                external_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
        }
        else
        {
            if ( m_NURBSCurveVec[i].m_InternalFlag )
            {
                internal_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
            else
            {
                external_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
        }
    }

    map< int, vector < pair < NURBS_Curve, bool > > > ordered_internal_curve_map = BuildOrderedChains( internal_curve_vec );
    map< int, vector < pair < NURBS_Curve, bool > > > ordered_external_curve_map = BuildOrderedChains( external_curve_vec );

    vector < NURBS_Loop > internal_loop_vec = MergeOrderedChains( ordered_internal_curve_map );
    for ( size_t i = 0; i < internal_loop_vec.size(); i++ )
    {
        internal_loop_vec[i].m_InternalLoopFlag = true;
    }

    vector < NURBS_Loop > external_loop_vec = MergeOrderedChains( ordered_external_curve_map );
    for ( size_t i = 0; i < external_loop_vec.size(); i++ )
    {
        external_loop_vec[i].m_InternalLoopFlag = false;
    }

    m_NURBSLoopVec = internal_loop_vec;
    m_NURBSLoopVec.insert( m_NURBSLoopVec.end(), external_loop_vec.begin(), external_loop_vec.end() );
}

void NURBS_Surface::WriteIGESLoops( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf )
{
    // Create surface curves for sub-surfaces and FEA Part intersections (if they are inside the parent Geom)
    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        if ( m_NURBSCurveVec[i].m_SubSurfFlag || ( m_NURBSCurveVec[i].m_StructIntersectFlag && m_NURBSCurveVec[i].m_InternalFlag ) )
        {
            iges->MakeCurve( m_NURBSCurveVec[i].m_PntVec, m_NURBSCurveVec[i].m_Deg );
        }
    }

    // Identify if there are multiple external loops
    vector < NURBS_Loop > ext_loop_vec, cutout_vec;

    for ( size_t i = 0; i < m_NURBSLoopVec.size(); i++ )
    {
        if ( m_NURBSLoopVec[i].m_IntersectLoopFlag )
        {
            if ( m_SurfType == vsp::CFD_STRUCTURE )
            {
                // Opposite trimming behavior for structures
                // This case applies to FEA slices, which are a single surface. Domes
                // are more than 1 surface, so the bounding loop is not a single intersection
                // chain, but a combination of intersections and border curves
                ext_loop_vec.push_back( m_NURBSLoopVec[i] );
            }
            else
            {
                cutout_vec.push_back( m_NURBSLoopVec[i] );
            }
        }
        else if ( !m_NURBSLoopVec[i].m_InternalLoopFlag && m_SurfType != vsp::CFD_STRUCTURE )
        {
            ext_loop_vec.push_back( m_NURBSLoopVec[i] );
        }
        else if ( m_NURBSLoopVec[i].m_InternalLoopFlag && m_SurfType == vsp::CFD_STRUCTURE )
        {
            ext_loop_vec.push_back( m_NURBSLoopVec[i] );
        }
    }

    if ( ext_loop_vec.size() == 1 )
    {
        DLL_IGES_ENTITY_144 trimmed_surf = ext_loop_vec[0].WriteIGESLoop( iges, parent_surf );

        for ( size_t i = 0; i < cutout_vec.size(); i++ )
        {
            cutout_vec[i].WriteIGESCutout( iges, parent_surf, trimmed_surf );
        }
    }
    else if ( ext_loop_vec.size() > 1 )
    {
        // If more than 1 external loop, create a 128 parent NURBS surface for each
        // and trim with separate bounds. This is necessary because an IGES 128 entity
        // can only have a single outer bound. 
        for ( size_t i = 0; i < ext_loop_vec.size(); i++ )
        {
            if ( i == 0 )
            {
                DLL_IGES_ENTITY_144 trimmed_surf = ext_loop_vec[i].WriteIGESLoop( iges, parent_surf );

                // Check if for any cutouts on the trimmed surface
                for ( size_t i = 0; i < cutout_vec.size(); i++ )
                {
                    if ( Compare( ext_loop_vec[i].GetBndBox(), cutout_vec[i].GetBndBox() ) ) // TODO: Improve this comparison
                    {
                        cutout_vec[i].WriteIGESCutout( iges, parent_surf, trimmed_surf );
                    }
                }
            }
            else
            {
                // Create new NURBS surface to trim
                string label = "";

                if ( m_SurfID > 0 ) // TODO: Get parent surf label
                {
                    label = "Surf_";

                    label.append( to_string( m_SurfID ) );
                    label.append( "_" );
                    label.append( to_string( i ) ); // add loop index
                }

                DLL_IGES_ENTITY_128 isurf = WriteIGESSurf( iges, label );

                DLL_IGES_ENTITY_144 trimmed_surf = ext_loop_vec[i].WriteIGESLoop( iges, isurf );

                // Check if for any cutouts on the trimmed surface
                for ( size_t i = 0; i < cutout_vec.size(); i++ )
                {
                    if ( Compare( ext_loop_vec[i].GetBndBox(), cutout_vec[i].GetBndBox() ) ) // TODO: Improve this comparison
                    {
                        cutout_vec[i].WriteIGESCutout( iges, parent_surf, trimmed_surf );
                    }
                }
            }
        }
    }
}

vector < SdaiAdvanced_face* > NURBS_Surface::WriteSTEPLoops( STEPutil* step, SdaiSurface* surf, bool mergepts )
{
    // Create surface curves for sub-surfaces and FEA Part intersections (if they are inside the parent Geom)
    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        if ( m_NURBSCurveVec[i].m_SubSurfFlag || ( m_NURBSCurveVec[i].m_StructIntersectFlag && m_NURBSCurveVec[i].m_InternalFlag ) )
        {
            step->MakeSurfaceCurve( m_NURBSCurveVec[i].m_PntVec, m_NURBSCurveVec[i].m_Deg, mergepts, m_NURBSCurveVec[i].m_MergeTol );
        }
    }

    vector < SdaiAdvanced_face* > adv_vec;
    vector < SdaiFace_bound* > face_vec;
    vector < SdaiFace_outer_bound* > outer_face_vec;

    // Identify if there are multiple external loops
    vector < NURBS_Loop > ext_loop_vec;

    for ( size_t i = 0; i < m_NURBSLoopVec.size(); i++ )
    {
        if ( m_NURBSLoopVec[i].m_IntersectLoopFlag )
        {
            if ( m_SurfType == vsp::CFD_STRUCTURE )
            {
                // Opposite trimming behavior for structures
                // This case applies to FEA slices, which are a single surface. Domes
                // are more than 1 surface, so the bounding loop is not a single intersection
                // chain, but a combination of intersections and border curves
                ext_loop_vec.push_back( m_NURBSLoopVec[i] );
            }
            else
            {
                SdaiEdge_loop* loop = m_NURBSLoopVec[i].WriteSTEPLoop( step );

                if ( loop ) // TODO: Identify if the interior loop is on a surface that will be split
                {
                    SdaiFace_bound* face = (SdaiFace_bound*)step->registry->ObjCreate( "FACE_BOUND" );
                    step->instance_list->Append( (SDAI_Application_instance*)face, completeSE );
                    face->bound_( loop );
                    face->name_( "''" );
                    face->orientation_( BTrue );

                    face_vec.push_back( face );
                }
            }
        }
        else if ( !m_NURBSLoopVec[i].m_InternalLoopFlag && m_SurfType != vsp::CFD_STRUCTURE )
        {
            ext_loop_vec.push_back( m_NURBSLoopVec[i] );
        }
        else if ( m_NURBSLoopVec[i].m_InternalLoopFlag && m_SurfType == vsp::CFD_STRUCTURE )
        {
            ext_loop_vec.push_back( m_NURBSLoopVec[i] );
        }
    }

    if ( ext_loop_vec.size() == 1 )
    {
        SdaiEdge_loop* loop = ext_loop_vec[0].WriteSTEPLoop( step );

        if ( loop )
        {
            SdaiFace_outer_bound* face = (SdaiFace_outer_bound*)step->registry->ObjCreate( "FACE_OUTER_BOUND" );
            step->instance_list->Append( (SDAI_Application_instance*)face, completeSE );
            face->bound_( loop );
            face->name_( "''" );
            face->orientation_( BTrue );

            outer_face_vec.push_back( face );
        }
    }
    else if ( ext_loop_vec.size() > 1 )
    {
        // If more than 1 external loop, create a parent NURBS surface for each
        // and trim with separate bounds. This is necessary because an NURBS surface
        // can only have a single outer bound. 
        for ( size_t i = 0; i < ext_loop_vec.size(); i++ )
        {
            if ( i == 0 )
            {
                SdaiEdge_loop* loop = ext_loop_vec[i].WriteSTEPLoop( step );

                if ( loop )
                {
                    SdaiFace_outer_bound* face = (SdaiFace_outer_bound*)step->registry->ObjCreate( "FACE_OUTER_BOUND" );
                    step->instance_list->Append( (SDAI_Application_instance*)face, completeSE );
                    face->bound_( loop );
                    face->name_( "''" );
                    face->orientation_( BTrue );

                    outer_face_vec.push_back( face );
                }
            }
            else
            {
                SdaiSurface* new_surf = WriteSTEPSurf( step, mergepts );

                SdaiEdge_loop* loop = ext_loop_vec[i].WriteSTEPLoop( step );

                if ( loop && new_surf )
                {
                    SdaiFace_outer_bound* face = (SdaiFace_outer_bound*)step->registry->ObjCreate( "FACE_OUTER_BOUND" );
                    step->instance_list->Append( (SDAI_Application_instance*)face, completeSE );
                    face->bound_( loop );
                    face->name_( "''" );
                    face->orientation_( BTrue );

                    SdaiAdvanced_face* adv_face = (SdaiAdvanced_face*)step->registry->ObjCreate( "ADVANCED_FACE" );
                    step->instance_list->Append( (SDAI_Application_instance*)adv_face, completeSE );
                    adv_face->face_geometry_( new_surf );
                    adv_face->name_( "''" );
                    adv_face->same_sense_( BTrue );

                    std::ostringstream face_ss;
                    face_ss << '#' << face->GetFileId();

                    adv_face->bounds_()->AddNode( new GenericAggrNode( face_ss.str().c_str() ) );

                    adv_vec.push_back( adv_face );
                }
            }
        }
    }

    SdaiAdvanced_face* adv_face = (SdaiAdvanced_face*)step->registry->ObjCreate( "ADVANCED_FACE" );
    step->instance_list->Append( (SDAI_Application_instance*)adv_face, completeSE );
    adv_face->face_geometry_( surf );
    adv_face->name_( "''" );
    adv_face->same_sense_( BTrue );

    std::ostringstream face_ss;

    for ( size_t i = 0; i < outer_face_vec.size(); i++ )
    {
        face_ss << "#" << outer_face_vec[i]->GetFileId();

        if ( i < outer_face_vec.size() - 1 )
        {
            face_ss << ", ";
        }
    }

    if ( face_vec.size() > 0 && outer_face_vec.size() > 0 )
    {
        face_ss << ", ";
    }

    for ( size_t i = 0; i < face_vec.size(); i++ )
    {
        face_ss << "#" << face_vec[i]->GetFileId();

        if ( i < face_vec.size() - 1 )
        {
            face_ss << ", ";
        }
    }

    adv_face->bounds_()->AddNode( new GenericAggrNode( face_ss.str().c_str() ) );

    adv_vec.push_back( adv_face );

    return adv_vec;
}

vector < NURBS_Curve > NURBS_Surface::MatchNURBSCurves( vector < NURBS_Curve > all_curve_vec )
{
    vector < NURBS_Curve > return_vec;

    for ( size_t i = 0; i < all_curve_vec.size(); i++ )
    {
        if ( ( all_curve_vec[i].m_SurfA_ID == m_SurfID ) || ( all_curve_vec[i].m_SurfB_ID == m_SurfID ) )
        {
            return_vec.push_back( all_curve_vec[i] );
        }
    }

    return return_vec;
}