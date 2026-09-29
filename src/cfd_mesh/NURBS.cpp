//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NURBS (Non-Uniform Rational B-Spline)
//
//////////////////////////////////////////////////////////////////////

#include "NURBS.h"
#include "IsectAdapt.h"


//////////////////////////////////////////////////////
//================= NURBS_Curve ====================//
//////////////////////////////////////////////////////

NURBS_Curve::NURBS_Curve()
{
    m_BorderFlag = false;
    m_InternalFlag = false;
    m_SubSurfFlag = false;
    m_SurfA_Type = vsp::CFD_NORMAL;
    m_SurfB_Type = vsp::CFD_NORMAL;
    m_InsideNegativeFlag = false;
    m_SurfA_ID = -1;
    m_SurfB_ID = -1;
    m_CurveID = -1;
    m_MergeTol = 0;
    m_CADDeg = 1;
    m_CADUWDeg = 1;
    m_BBox = BndBox();
    m_Label = string();
    m_WakeFlag = false;
}

void NURBS_Curve::InitPolyline( SCurve curveA, SCurve curveB, double curve_tol )
{
    Bezier_curve uwcrvA = curveA.GetUWCrv();
    Bezier_curve uwcrvB = curveB.GetUWCrv();

    // At the same points in the parameters of both parents.  A border runs where two patches
    // meet, often tangent to each other, so only an intersection is put on both.
    IsectAdaptCurve poly;
    poly.Adapt( uwcrvA, uwcrvB, *curveA.GetSurf(), *curveB.GetSurf(), !m_BorderFlag, false, curve_tol, 0.0 );

    m_PntVec.resize( poly.m_Pnts.size() );
    m_UWPntVec_A.resize( poly.m_Pnts.size() );
    m_UWPntVec_B.resize( poly.m_Pnts.size() );
    for ( int i = 0; i < ( int )poly.m_Pnts.size(); i++ )
    {
        m_PntVec[i] = poly.m_Pnts[i].m_Pnt;
        m_UWPntVec_A[i] = poly.m_Pnts[i].m_UW[0];
        m_UWPntVec_B[i] = poly.m_Pnts[i].m_UW[1];
    }

    SetMergeTol( curveA );
}

// A piecewise Bezier curve of degree deg through cp, the segments sharing their end points, with
// breaks the parameter at each segment end, at parameter t
static vec3d PiecewisePnt( const vector < vec3d > &cp, int deg, const vector < double > &breaks, double t )
{
    int nseg = ( int )breaks.size() - 1;

    int iseg = 0;
    while ( iseg < nseg - 1 && t > breaks[ iseg + 1 ] )
    {
        iseg++;
    }

    double dt = breaks[ iseg + 1 ] - breaks[ iseg ];
    double s = 0.0;
    if ( dt != 0.0 )
    {
        s = ( t - breaks[ iseg ] ) / dt;
    }

    // de Casteljau
    vector < vec3d > work( cp.begin() + deg * iseg, cp.begin() + deg * iseg + deg + 1 );
    for ( int r = 1; r <= deg; r++ )
    {
        for ( int j = 0; j <= deg - r; j++ )
        {
            work[j] = work[j] * ( 1.0 - s ) + work[j + 1] * s;
        }
    }
    return work[0];
}

void NURBS_Curve::InitCAD( SCurve curveA, SCurve curveB, double tol )
{
    Bezier_curve uwcrvA = curveA.GetUWCrv();
    Bezier_curve uwcrvB = curveB.GetUWCrv();

    // A border is the parameter line of its surface it runs along, and an intersection the cubic
    // adapted onto both parents
    if ( !m_BorderFlag || !BuildBorderCADCurve( curveA, curveB, tol ) )
    {
        IsectAdaptCurve adapt;
        adapt.Adapt( uwcrvA, uwcrvB, *curveA.GetSurf(), *curveB.GetSurf(), !m_BorderFlag, true, 0.0, tol );
        adapt.GetBezier( m_CADPntVec, m_CADUWPntVec_A, m_CADUWPntVec_B, m_CADBreakVec, m_CADDeg );

        if ( adapt.m_NumMiss > 0 )
        {
            printf( "WARNING: Surfaces %d and %d do not meet to within tolerance along %d segments of their intersection, apart by up to %g\n",
                    m_SurfA_ID, m_SurfB_ID, adapt.m_NumMiss, adapt.m_MaxMiss );
        }

        m_CADUWDeg = m_CADDeg;
        m_CADUWBreakVec_A = m_CADBreakVec;
        m_CADUWBreakVec_B = m_CADBreakVec;
    }

    // The polyline the loops are built from is the CAD curve itself, and its image on each parent,
    // taken at the same points along it
    const int nsamp = 8;

    vector < double > t_vec;
    for ( size_t i = 0; i + 1 < m_CADBreakVec.size(); i++ )
    {
        for ( int n = 0; n < nsamp; n++ )
        {
            t_vec.push_back( m_CADBreakVec[i] + ( m_CADBreakVec[i + 1] - m_CADBreakVec[i] ) * n / nsamp );
        }
    }
    t_vec.push_back( m_CADBreakVec.back() );

    m_PntVec.resize( t_vec.size() );
    m_UWPntVec_A.resize( t_vec.size() );
    m_UWPntVec_B.resize( t_vec.size() );
    for ( size_t i = 0; i < t_vec.size(); i++ )
    {
        m_PntVec[i] = PiecewisePnt( m_CADPntVec, m_CADDeg, m_CADBreakVec, t_vec[i] );
        m_UWPntVec_A[i] = PiecewisePnt( m_CADUWPntVec_A, m_CADUWDeg, m_CADUWBreakVec_A, t_vec[i] );
        m_UWPntVec_B[i] = PiecewisePnt( m_CADUWPntVec_B, m_CADUWDeg, m_CADUWBreakVec_B, t_vec[i] );
    }

    SetMergeTol( curveA );
}

void NURBS_Curve::SetMergeTol( SCurve &curveA )
{
    m_BBox = curveA.GetSurf()->GetBBox();

    m_MergeTol = m_BBox.DiagDist() * 1.0e-10;

    // TODO: This does not make any sense.
    // If m_BBox.DiagDist() < 1.0, then this will always happen.  That seems a bit extreme.
    // Not sure if the model is scaled at this point.  If not, 1.0 should often be a
    // significant distance.  I.e. unlikely a threshold for changing behavior.
    if ( m_MergeTol < 1.0e-10 )
    {
        m_MergeTol = 1.0e-10;
    }
}

// Where a border runs across the surface on the other side of it: points added between
// ( t0, uv0 ) and ( t1, uv1 ) until the straight line between each pair in that surface's
// parameters stays within tol of the border.  The border is the parameter line of surfA at
// cval, in u where ucon and in w otherwise, and t is the parameter along it.
static void RefineBorderPCurve( const Surf &surfA, bool ucon, double cval, const Surf &surfB, double t0, const vec3d &uv0,
                                double t1, const vec3d &uv1, double tol, int nlimit,
                                vector < double > &t_vec, vector < vec3d > &uv_vec )
{
    double tm = 0.5 * ( t0 + t1 );
    vec3d uvm = ( uv0 + uv1 ) * 0.5;

    vec3d p;
    if ( ucon )
    {
        p = surfA.CompPnt( cval, tm );
    }
    else
    {
        p = surfA.CompPnt( tm, cval );
    }

    if ( nlimit <= 0 || dist( surfB.CompPnt( uvm.x(), uvm.y() ), p ) <= tol )
    {
        return;
    }

    vec2d uvp = surfB.ClosestUW( p, uvm.x(), uvm.y() );
    vec3d uvn( uvp.x(), uvp.y(), 0.0 );

    // Where the border is not on the surface, there is nothing nearer to follow
    if ( dist( surfB.CompPnt( uvn.x(), uvn.y() ), p ) > tol )
    {
        return;
    }

    RefineBorderPCurve( surfA, ucon, cval, surfB, t0, uv0, tm, uvn, tol, nlimit - 1, t_vec, uv_vec );
    t_vec.push_back( tm );
    uv_vec.push_back( uvn );
    RefineBorderPCurve( surfA, ucon, cval, surfB, tm, uvn, t1, uv1, tol, nlimit - 1, t_vec, uv_vec );
}

bool ParameterLineCurve( SCurve &crv, vector < vec3d > &cp_vec, vector < double > &break_vec, int &deg,
                         bool &ucon, double &cval, double &ta, double &tb )
{
    Bezier_curve uwcrv = crv.GetUWCrv();

    vector < vec3d > uw_vec;
    uwcrv.GetControlPoints( uw_vec );

    if ( uw_vec.size() < 2 )
    {
        return false;
    }

    const ParmLine &line = crv.GetParmLine();
    if ( !line.IsLine() )
    {
        return false;
    }

    const piecewise_surface_type* surf = crv.GetSurf()->GetSurfCore()->GetSurf();

    piecewise_curve_type pc;

    ucon = ( line.m_Kind == ParmLine::U_CONST );
    cval = line.m_Val;
    if ( ucon )
    {
        cval = clamp( cval, surf->get_u0(), surf->get_umax() );
        surf->get_uconst_curve( pc, cval );
    }
    else
    {
        cval = clamp( cval, surf->get_v0(), surf->get_vmax() );
        surf->get_vconst_curve( pc, cval );
    }
    ta = line.Along( uw_vec.front() );
    tb = line.Along( uw_vec.back() );

    double tlo = clamp( std::min( ta, tb ), pc.get_t0(), pc.get_tmax() );
    double thi = clamp( std::max( ta, tb ), pc.get_t0(), pc.get_tmax() );

    if ( thi <= tlo )
    {
        return false;
    }

    pc.split( tlo );
    pc.split( thi );

    vector < piecewise_curve_type::curve_type > seg_vec;
    break_vec.clear();
    deg = 0;

    for ( piecewise_curve_type::index_type i = 0; i < pc.number_segments(); i++ )
    {
        piecewise_curve_type::curve_type seg;
        double t, dt;
        pc.get( seg, t, dt, i );

        double tmid = t + 0.5 * dt;
        if ( tmid > tlo && tmid < thi )
        {
            if ( break_vec.empty() )
            {
                break_vec.push_back( t );
            }
            break_vec.push_back( t + dt );
            seg_vec.push_back( seg );
            deg = std::max( deg, ( int )seg.degree() );
        }
    }

    if ( seg_vec.empty() )
    {
        return false;
    }

    cp_vec.clear();
    for ( size_t i = 0; i < seg_vec.size(); i++ )
    {
        seg_vec[i].degree_promote_to( deg );

        // Segments share their end points
        int j0 = 0;
        if ( i > 0 )
        {
            j0 = 1;
        }

        for ( int j = j0; j <= deg; j++ )
        {
            piecewise_curve_type::curve_type::control_point_type cp = seg_vec[i].get_control_point( j );
            cp_vec.push_back( vec3d( cp.x(), cp.y(), cp.z() ) );
        }
    }

    return true;
}

bool NURBS_Curve::BuildBorderCADCurve( SCurve &crvA, SCurve &crvB, double tol )
{
    // The side the border is a parameter line of, and the side across it
    SCurve* lcrv = &crvA;
    SCurve* ocrv = &crvB;
    vector < vec3d >* lcad = &m_CADUWPntVec_A;
    vector < vec3d >* ocad = &m_CADUWPntVec_B;
    vector < double >* lbreak = &m_CADUWBreakVec_A;
    vector < double >* obreak = &m_CADUWBreakVec_B;

    if ( !crvA.GetParmLine().IsLine() )
    {
        std::swap( lcrv, ocrv );
        std::swap( lcad, ocad );
        std::swap( lbreak, obreak );
    }

    vector < double > break_vec;
    int deg;
    bool ucon;
    double cval, ta, tb;

    if ( !ParameterLineCurve( *lcrv, m_CADPntVec, break_vec, deg, ucon, cval, ta, tb ) )
    {
        return false;
    }

    const ParmLine &line = lcrv->GetParmLine();
    const ParmLine &oline = ocrv->GetParmLine();

    m_CADDeg = deg;
    m_CADBreakVec = break_vec;

    // On its own surface the curve is the parameter line itself
    lcad->resize( break_vec.size() );
    for ( size_t i = 0; i < break_vec.size(); i++ )
    {
        if ( ucon )
        {
            ( *lcad )[i] = vec3d( cval, break_vec[i], 0.0 );
        }
        else
        {
            ( *lcad )[i] = vec3d( break_vec[i], cval, 0.0 );
        }
    }
    m_CADUWDeg = 1;
    *lbreak = break_vec;

    // On the surface across the border, its ends, which are where the curve it was matched with
    // ends, and more between them where the border bends away from the line through them there
    Bezier_curve luwcrv = lcrv->GetUWCrv();
    Bezier_curve ouwcrv = ocrv->GetUWCrv();

    vector < vec3d > ouw( 2 );
    ouw[0] = ouwcrv.FirstPnt();
    ouw[1] = ouwcrv.LastPnt();

    vector < double > t_vec( 2 );
    t_vec[0] = line.Along( luwcrv.FirstPnt() );
    t_vec[1] = line.Along( luwcrv.LastPnt() );

    ocad->clear();
    obreak->clear();
    obreak->push_back( t_vec[0] );
    ocad->push_back( ouw[0] );
    RefineBorderPCurve( *lcrv->GetSurf(), ucon, cval, *ocrv->GetSurf(), t_vec[0], ouw[0], t_vec[1], ouw[1],
                        0.5 * tol, 16, *obreak, *ocad );
    obreak->push_back( t_vec[1] );
    ocad->push_back( ouw[1] );

    // Where the border is a parameter line of that surface too, the pcurve is on it exactly
    for ( size_t i = 0; i < ocad->size(); i++ )
    {
        ( *ocad )[i] = oline.OnLine( ( *ocad )[i] );
    }

    // Run with the curve as it was built, from tlo to thi, over exactly its range
    if ( tb < ta )
    {
        reverse( ocad->begin(), ocad->end() );
        reverse( obreak->begin(), obreak->end() );
    }
    obreak->front() = break_vec.front();
    obreak->back() = break_vec.back();

    if ( tb < ta )
    {
        ReverseCAD();
    }

    return true;
}

void NURBS_Curve::Reverse()
{
    reverse( m_PntVec.begin(), m_PntVec.end() );
    reverse( m_UWPntVec_A.begin(), m_UWPntVec_A.end() );
    reverse( m_UWPntVec_B.begin(), m_UWPntVec_B.end() );

    ReverseCAD();
}

void NURBS_Curve::ReverseCAD()
{
    reverse( m_CADPntVec.begin(), m_CADPntVec.end() );
    ReverseBreakVec( m_CADBreakVec );

    reverse( m_CADUWPntVec_A.begin(), m_CADUWPntVec_A.end() );
    ReverseBreakVec( m_CADUWBreakVec_A );
    reverse( m_CADUWPntVec_B.begin(), m_CADUWPntVec_B.end() );
    ReverseBreakVec( m_CADUWBreakVec_B );
}

void NURBS_Curve::ReverseBreakVec( vector < double > &break_vec )
{
    // The same breakpoints, run from the other end over the same range
    int nbreak = ( int )break_vec.size();
    if ( nbreak > 0 )
    {
        double tsum = break_vec.front() + break_vec.back();
        vector < double > rev_vec( nbreak );
        for ( int i = 0; i < nbreak; i++ )
        {
            rev_vec[i] = tsum - break_vec[ nbreak - 1 - i ];
        }
        break_vec = rev_vec;
    }
}

void NURBS_Curve::WriteIGESEdge( IGESutil* iges, const string& label )
{
    m_IGES_Edge.reset( new DLL_IGES_ENTITY_126( iges->MakeCurve( m_CADPntVec, m_CADDeg, m_CADBreakVec, label ) ) );
}

SdaiEdge_curve* NURBS_Curve::WriteSTEPEdge( STEPutil* step, SdaiVertex_point* start_vert, SdaiVertex_point* end_vert,
                                            const string& label, bool mergepnts ) const
{
    Logical closed_curve = LFalse;
    if ( start_vert == end_vert )
    {
        closed_curve = LTrue;
    }

    SdaiB_spline_curve_with_knots* curve = step->MakeCurve( m_CADPntVec, m_CADDeg, m_CADBreakVec, label, closed_curve, mergepnts, m_MergeTol );

    SdaiEdge_curve* edge_crv = (SdaiEdge_curve*)step->registry->ObjCreate( "EDGE_CURVE" );
    step->instance_list->Append( (SDAI_Application_instance*)edge_crv, completeSE );
    edge_crv->edge_geometry_( curve );
    edge_crv->edge_start_( start_vert );
    edge_crv->edge_end_( end_vert );
    edge_crv->same_sense_( BTrue ); // direction set by oriented edge

    if ( label.size() > 0 )
    {
        edge_crv->name_( STEPString( "Edge_" + label ) );
    }
    else
    {
        edge_crv->name_( "''" );
    }

    return edge_crv;
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
    m_Label = string();
}

void NURBS_Loop::SetPntVec( const vector < vec3d >& pnt_vec )
{
    m_PntVec = pnt_vec;

    // Check for closure
    m_ClosedFlag = dist( m_PntVec.back(), m_PntVec.front() ) < FLT_EPSILON;
}

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
        nurbs_vec[i] = m_OrderedCurves[i].first.m_IGES_Edge.get();
    }

    return nurbs_vec;
}

CURVE_CREATION NURBS_Loop::IGESCurveCreation() const
{
    bool border = false;
    bool isect = false;

    for ( size_t i = 0; i < m_OrderedCurves.size(); i++ )
    {
        if ( m_OrderedCurves[i].first.m_BorderFlag )
        {
            border = true;
        }
        else
        {
            isect = true;
        }
    }

    if ( border && !isect )
    {
        return CURVE_CREATE_PARAMETRIC;
    }
    if ( isect && !border )
    {
        return CURVE_CREATE_INTERSECTION;
    }
    return CURVE_CREATE_UNSPECIFIED;
}

DLL_IGES_ENTITY_144 NURBS_Loop::WriteIGESLoop( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, const string& label )
{
    if ( !m_ClosedFlag )
    {
        printf( "ERROR: Incomplete IGES Loop \n" );
    }

    vector < DLL_IGES_ENTITY_126* > nurbs_vec = GetIGESEdges( iges );

    return iges->MakeLoop( parent_surf, nurbs_vec, IGESCurveCreation(), label );
}

void NURBS_Loop::WriteIGESCutout( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, DLL_IGES_ENTITY_144& trimmed_surf, const string& label )
{
    if ( !m_ClosedFlag )
    {
        printf( "ERROR: Incomplete IGES Loop \n" );
        return;
    }

    vector < DLL_IGES_ENTITY_126* > nurbs_vec = GetIGESEdges( iges );

    iges->MakeCutout( parent_surf, trimmed_surf, nurbs_vec, IGESCurveCreation(), label );
}

SdaiEdge_loop* NURBS_Loop::WriteSTEPLoop( STEPutil* step, STEP_Topology* topo, bool mergepts )
{
    if ( !m_ClosedFlag )
    {
        printf( "ERROR: Incomplete STEP Loop \n" );
        return nullptr;
    }

    vector < SdaiOriented_edge* > or_edge_vec;

    for ( size_t i = 0; i < m_OrderedCurves.size(); i++ )
    {
        SdaiEdge_curve* edge = topo->GetEdge( step, m_OrderedCurves[i].first.m_CurveID, mergepts );

        // Created oriented edge from NURBS_Curve edge
        SdaiOriented_edge* or_edge = (SdaiOriented_edge*)step->registry->ObjCreate( "ORIENTED_EDGE" );
        step->instance_list->Append( (SDAI_Application_instance*)or_edge, completeSE );
        or_edge->edge_element_( edge );
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
    // the edge_list from the SdaiPath??  Initialized to nullptr and
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

SdaiFace_bound* NURBS_Loop::WriteSTEPBound( STEPutil* step, STEP_Topology* topo, int surf_id, bool cutout_flag,
                                            bool flip_flag, bool mergepts )
{
    SdaiEdge_loop* loop = WriteSTEPLoop( step, topo, mergepts );

    if ( !loop )
    {
        return nullptr;
    }

    SdaiFace_bound* face = nullptr;
    if ( cutout_flag )
    {
        face = (SdaiFace_bound*)step->registry->ObjCreate( "FACE_BOUND" );
    }
    else
    {
        face = (SdaiFace_bound*)step->registry->ObjCreate( "FACE_OUTER_BOUND" );
    }
    step->instance_list->Append( (SDAI_Application_instance*)face, completeSE );
    face->bound_( loop );
    face->name_( "''" );

    // The face is written facing out of the body, so its bounds run with the face on their left
    if ( Sense( surf_id, cutout_flag, flip_flag ) < 0 )
    {
        face->orientation_( BFalse );
    }
    else
    {
        face->orientation_( BTrue );
    }

    return face;
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

double NURBS_Loop::SignedAreaUW( int surf_id ) const
{
    vector < vec3d > uw_vec;

    for ( int i = 0; i < ( int )m_OrderedCurves.size(); i++ )
    {
        const NURBS_Curve &nurbs_curve = m_OrderedCurves[i].first;

        // The curve is stored in the parameter space of both its parents; take the
        // copy belonging to the surface this loop lies on.
        if ( nurbs_curve.m_SurfA_ID == surf_id )
        {
            uw_vec.insert( uw_vec.end(), nurbs_curve.m_UWPntVec_A.begin(), nurbs_curve.m_UWPntVec_A.end() );
        }
        else if ( nurbs_curve.m_SurfB_ID == surf_id )
        {
            uw_vec.insert( uw_vec.end(), nurbs_curve.m_UWPntVec_B.begin(), nurbs_curve.m_UWPntVec_B.end() );
        }
    }

    double area = 0.0;

    for ( int i = 0; i < ( int )uw_vec.size(); i++ )
    {
        const vec3d &p0 = uw_vec[i];
        const vec3d &p1 = uw_vec[( i + 1 ) % uw_vec.size()];

        area += p0.x() * p1.y() - p1.x() * p0.y();
    }

    return 0.5 * area;
}

int NURBS_Loop::Sense( int surf_id, bool cutout_flag, bool flip_flag ) const
{
    double area = SignedAreaUW( surf_id );

    if ( area == 0.0 )
    {
        return 0;
    }

    int loop_sense = 1;
    if ( area < 0.0 )
    {
        loop_sense = -1;
    }

    if ( cutout_flag )
    {
        loop_sense = -loop_sense;
    }

    if ( flip_flag )
    {
        loop_sense = -loop_sense;
    }

    return loop_sense;
}

//////////////////////////////////////////////////////
//================ NURBS_Surface ===================//
//////////////////////////////////////////////////////

NURBS_Surface::NURBS_Surface()
{
    m_SurfID = -1;
    m_Surf = nullptr;
    m_SurfType = vsp::CFD_NORMAL;
    m_Label = string();
    m_WakeFlag = false;
    m_FlipFlag = false;
}

void NURBS_Surface::InitNURBSSurf( Surf* surface )
{
    m_SurfID = surface->GetSurfID();
    m_BBox = surface->GetBBox();

    m_Surf = surface->GetSurfCore()->GetSurf();
}

DLL_IGES_ENTITY_128 NURBS_Surface::WriteIGESSurf( IGESutil* iges, const string& label )
{
    string new_label = label;
    if ( m_WakeFlag && label.size() > 0 )
    {
        new_label = "Wake_" + label;
    }

    // Facing out of the body
    if ( m_FlipFlag )
    {
        piecewise_surface_type flipped = *m_Surf;
        flipped.reverse_v();
        return iges->MakeSurf( flipped, new_label.c_str() );
    }

    return iges->MakeSurf( *m_Surf, new_label.c_str() );
}

SdaiSurface* NURBS_Surface::WriteSTEPSurf( STEPutil* step, const string& label, bool mergepts )
{
    string new_label = label;
    if ( m_WakeFlag && label.size() > 0 )
    {
        new_label = "Wake_" + label;
    }

    //==== Compute Tol ====//
    double merge_tol = m_BBox.DiagDist() * 1.0e-10;

    if ( merge_tol < 1.0e-10 )
    {
        merge_tol = 1.0e-10;
    }

    return step->MakeSurf( *m_Surf, new_label, mergepts, merge_tol );
}

unordered_map< int, vector < pair < NURBS_Curve, bool > > > NURBS_Surface::BuildOrderedChains( vector < NURBS_Curve > chain_vec )
{
    // Starting with 1 NURBS curve, march along the others to form an ordered chain.
    int map_ind = 0; // each map index represents a group of attached NURBS_Curve chains

    double tol = m_BBox.DiagDist() * 1e-2; // Note: This tolerance is kept large in order to still form loops when there are large gaps in intersection curves

    unordered_map< int, vector < pair < NURBS_Curve, bool > > > return_curve_map;

    while ( chain_vec.size() > 0 )
    {
        return_curve_map[map_ind].push_back( make_pair( chain_vec[0], true ) );
        chain_vec.erase( chain_vec.begin() );

        size_t num_int_curve = chain_vec.size();

        for ( size_t i = 0; i < num_int_curve; i++ )
        {
            vec3d curr_back_pnt = return_curve_map[map_ind].back().first.m_PntVec.back();
            vec3d curr_front_pnt = return_curve_map[map_ind].front().first.m_PntVec.front();

            double closest_dist = 1e9;
            bool orientation = false; // Identifies the orientation of the curve for this particular loop
            int chain_ind = 0;

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

            // A chain that closes nearer than the nearest curve it could take next is a loop, and
            // takes no more.  One that closes where another curve starts goes on, and is split
            // there later.  One curve is a loop only where its own ends meet.
            double closing = dist( curr_front_pnt, curr_back_pnt );
            if ( ( return_curve_map[map_ind].size() > 1 || closing <= m_BBox.DiagDist() * 1e-8 ) && closing < closest_dist )
            {
                break;
            }

            if ( closest_dist < tol )
            {
                if ( !orientation )
                {
                    chain_vec[chain_ind].Reverse();
                }

                return_curve_map[map_ind].insert( return_curve_map[map_ind].begin(), make_pair( chain_vec[chain_ind], orientation ) );

                chain_vec.erase( chain_vec.begin() + chain_ind );
            }
            else
            {
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

vector < NURBS_Loop > NURBS_Surface::MergeOrderedChains( unordered_map< int, vector < pair < NURBS_Curve, bool > > > ordered_chain_map )
{
    vector < NURBS_Loop > loop_vec( ordered_chain_map.size() );

    unordered_map< int, vector < pair < NURBS_Curve, bool > > >::iterator it;

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
        if ( ( m_NURBSCurveVec[i].m_BorderFlag && m_NURBSCurveVec[i].m_InsideNegativeFlag && m_SurfType != vsp::CFD_TRANSPARENT ) || m_NURBSCurveVec[i].m_SubSurfFlag || ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_STRUCTURE && m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_STRUCTURE ) )
        {
            continue;
        }
        else if ( m_SurfType == vsp::CFD_TRANSPARENT && m_NURBSCurveVec[i].m_InsideNegativeFlag )
        {
            // Don't trim transparent surfaces with negative components
            external_curve_vec.push_back( m_NURBSCurveVec[i] );
        }
        else if ( m_SurfType == vsp::CFD_NORMAL && m_NURBSCurveVec[i].m_WakeFlag )
        {
            continue;
        }
        else if ( m_WakeFlag && m_NURBSCurveVec[i].m_InternalFlag )
        {
            continue;
        }
        else if ( !m_NURBSCurveVec[i].m_BorderFlag )
        {
            if ( ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_TRANSPARENT || m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_TRANSPARENT ) && m_SurfType != vsp::CFD_TRANSPARENT )
            {
                // Don't trim non-transparent surfaces with intersection curves made with a transparent surface. The non-transparent
                // surface will not be broken into two surfaces at the intersection curve. 
                continue; // TODO: Break the surface at the intersection
            }
            else if ( ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_STRUCTURE || m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_STRUCTURE ) && m_SurfType != vsp::CFD_STRUCTURE )
            {
                // Don't trim non-structure surfaces with intersection curves made with a structure surface. The non-structure
                // surface will not be broken into two surfaces at the intersection curve. 
                continue; // TODO: Break the surface at the intersection
            }
            else if ( m_NURBSCurveVec[i].m_InternalFlag && ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_NEGATIVE || m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_NEGATIVE ) && m_SurfType != vsp::CFD_NEGATIVE )
            {
                // Ignore internal negative surface intersecitons when trimming non-negative surfaces
                continue;
            }
            else if ( m_SurfType == vsp::CFD_TRANSPARENT && m_NURBSCurveVec[i].m_SurfA_Type != vsp::CFD_NORMAL && m_NURBSCurveVec[i].m_SurfB_Type != vsp::CFD_NORMAL )
            {
                // Ignore transparent-transparent (i.e. disks intersecting or wakes intersecting) and transparent-negative intersections (i.e. no holes in wakes or disks)
                continue; 
            }
            else if ( m_SurfType == vsp::CFD_STRUCTURE )
            {
                internal_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
            else if ( ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_NEGATIVE && m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_NEGATIVE ) && m_NURBSCurveVec[i].m_InternalFlag )
            {
                internal_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
            else if ( !m_NURBSCurveVec[i].m_InternalFlag && m_SurfType == vsp::CFD_NEGATIVE && ( m_NURBSCurveVec[i].m_SurfA_Type != vsp::CFD_NEGATIVE || m_NURBSCurveVec[i].m_SurfB_Type != vsp::CFD_NEGATIVE ) )
            {
                internal_curve_vec.push_back( m_NURBSCurveVec[i] );
            }
            else if ( ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_NEGATIVE || m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_NEGATIVE ) && m_SurfType != vsp::CFD_NEGATIVE )
            {
                external_curve_vec.push_back( m_NURBSCurveVec[i] );
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

    unordered_map< int, vector < pair < NURBS_Curve, bool > > > ordered_internal_curve_map = BuildOrderedChains( internal_curve_vec );
    unordered_map< int, vector < pair < NURBS_Curve, bool > > > ordered_external_curve_map = BuildOrderedChains( external_curve_vec );

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

void NURBS_Surface::MakeExtLoopVec( vector < NURBS_Loop > & ext_loop_vec, vector < NURBS_Loop > & cutout_vec )
{
    // Identify if there are multiple external loops
    for ( size_t i = 0; i < m_NURBSLoopVec.size(); i++ )
    {
        if ( m_NURBSLoopVec[i].m_IntersectLoopFlag )
        {
            if ( m_SurfType == vsp::CFD_STRUCTURE || ( m_SurfType == vsp::CFD_NEGATIVE && m_NURBSLoopVec[i].m_InternalLoopFlag ) )
            {
                // Opposite trimming behavior for structures and negative surfaces when the loop is inside another surface
                // This case applies to FEA slices, which are a single surface. Domes
                // are more than 1 surface, so the bounding loop is not a single intersection
                // chain, but a combination of intersections and border curves
                ext_loop_vec.push_back( m_NURBSLoopVec[i] );
            }
            else if ( m_SurfType != vsp::CFD_NEGATIVE )
            {
                cutout_vec.push_back( m_NURBSLoopVec[i] );
            }
        }
        else if ( !m_NURBSLoopVec[i].m_InternalLoopFlag && m_SurfType != vsp::CFD_STRUCTURE && m_SurfType != vsp::CFD_NEGATIVE )
        {
            ext_loop_vec.push_back( m_NURBSLoopVec[i] );
        }
        else if ( m_NURBSLoopVec[i].m_InternalLoopFlag && ( m_SurfType == vsp::CFD_STRUCTURE || m_SurfType == vsp::CFD_NEGATIVE ) )
        {
            ext_loop_vec.push_back( m_NURBSLoopVec[i] );
        }
    }
}

void NURBS_Surface::WriteIGESLoops( IGESutil* iges, DLL_IGES_ENTITY_128& parent_surf, const string& label )
{
    // Create surface curves for sub-surfaces and FEA Part intersections (if they are inside the parent Geom)
    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        if ( m_NURBSCurveVec[i].m_SubSurfFlag || 
             ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_STRUCTURE && m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_STRUCTURE && m_NURBSCurveVec[i].m_InternalFlag ) ||
             ( m_SurfType == vsp::CFD_TRANSPARENT && m_NURBSCurveVec[i].m_SurfA_Type != vsp::CFD_NORMAL && m_NURBSCurveVec[i].m_SurfB_Type != vsp::CFD_NORMAL && !m_NURBSCurveVec[i].m_BorderFlag && !m_NURBSCurveVec[i].m_InternalFlag ) )
        {
            iges->MakeCurve( m_NURBSCurveVec[i].m_CADPntVec, m_NURBSCurveVec[i].m_CADDeg, m_NURBSCurveVec[i].m_CADBreakVec, label );
        }
    }

    // Identify if there are multiple external loops
    vector < NURBS_Loop > ext_loop_vec, cutout_vec;

    MakeExtLoopVec( ext_loop_vec,  cutout_vec );

    if ( ext_loop_vec.size() == 1 )
    {
        DLL_IGES_ENTITY_144 trimmed_surf = ext_loop_vec[0].WriteIGESLoop( iges, parent_surf, label );

        for ( size_t i = 0; i < cutout_vec.size(); i++ )
        {
            cutout_vec[i].WriteIGESCutout( iges, parent_surf, trimmed_surf, label );
        }
    }
    else if ( ext_loop_vec.size() > 1 )
    {
        // If more than 1 external loop, create a new trimmed surface for each
        // with separate loop bounds.
        for ( size_t i = 0; i < ext_loop_vec.size(); i++ )
        {
            DLL_IGES_ENTITY_144 trimmed_surf = ext_loop_vec[i].WriteIGESLoop( iges, parent_surf, label );

            // Check if for any cutouts on the trimmed surface
            for ( size_t j = 0; j < cutout_vec.size(); j++ )
            {
                if ( Compare( ext_loop_vec[i].GetBndBox(), cutout_vec[j].GetBndBox() ) ) // TODO: Improve this comparison
                {
                    cutout_vec[j].WriteIGESCutout( iges, parent_surf, trimmed_surf, label );
                }
            }
        }
    }
}

vector < SdaiAdvanced_face* > NURBS_Surface::WriteSTEPLoops( STEPutil* step, STEP_Topology* topo, SdaiSurface* surf, vector < int > &face_ids,
                                                             const string& label, bool mergepts )
{
    // Create surface curves for sub-surfaces and FEA Part intersections (if they are inside the parent Geom)
    for ( size_t i = 0; i < m_NURBSCurveVec.size(); i++ )
    {
        if ( m_NURBSCurveVec[i].m_SubSurfFlag || 
             ( m_NURBSCurveVec[i].m_SurfA_Type == vsp::CFD_STRUCTURE && m_NURBSCurveVec[i].m_SurfB_Type == vsp::CFD_STRUCTURE && m_NURBSCurveVec[i].m_InternalFlag ) ||
             ( m_SurfType == vsp::CFD_TRANSPARENT && m_NURBSCurveVec[i].m_SurfA_Type != vsp::CFD_NORMAL && m_NURBSCurveVec[i].m_SurfB_Type != vsp::CFD_NORMAL && !m_NURBSCurveVec[i].m_BorderFlag && !m_NURBSCurveVec[i].m_InternalFlag ) )
        {
            step->MakeSurfaceCurve( m_NURBSCurveVec[i].m_CADPntVec, m_NURBSCurveVec[i].m_CADDeg, m_NURBSCurveVec[i].m_CADBreakVec, label,
                                    mergepts, m_NURBSCurveVec[i].m_MergeTol );
        }
    }

    // The loops that bound a face, and the holes cut in one
    vector < NURBS_Loop > ext_loop_vec, cutout_vec;

    MakeExtLoopVec( ext_loop_vec, cutout_vec );

    // One face per external loop, all on the same parent surface.  Holes go on the first.
    vector < vector < SdaiFace_bound* > > face_bound_vec;
    int first_face = topo->NewFaces( ( int )ext_loop_vec.size() );

    for ( size_t i = 0; i < ext_loop_vec.size(); i++ )
    {
        topo->SetFace( first_face + ( int )i );
        SdaiFace_bound* bound = ext_loop_vec[i].WriteSTEPBound( step, topo, m_SurfID, false, m_FlipFlag, mergepts );

        if ( bound )
        {
            face_bound_vec.push_back( vector < SdaiFace_bound* > ( 1, bound ) );
        }
    }

    if ( face_bound_vec.empty() )
    {
        face_bound_vec.resize( 1 );
    }

    for ( size_t i = 0; i < cutout_vec.size(); i++ )
    {
        topo->SetFace( first_face );
        SdaiFace_bound* bound = cutout_vec[i].WriteSTEPBound( step, topo, m_SurfID, true, m_FlipFlag, mergepts );

        if ( bound )
        {
            face_bound_vec[0].push_back( bound );
        }
    }

    vector < SdaiAdvanced_face* > adv_vec;
    face_ids.clear();

    for ( size_t i = 0; i < face_bound_vec.size(); i++ )
    {
        if ( face_bound_vec[i].empty() )
        {
            continue;
        }

        SdaiAdvanced_face* adv_face = (SdaiAdvanced_face*)step->registry->ObjCreate( "ADVANCED_FACE" );
        step->instance_list->Append( (SDAI_Application_instance*)adv_face, completeSE );
        adv_face->face_geometry_( surf );
        adv_face->name_( "''" );

        // Face out of the body
        if ( m_FlipFlag )
        {
            adv_face->same_sense_( BFalse );
        }
        else
        {
            adv_face->same_sense_( BTrue );
        }

        std::ostringstream face_ss;

        for ( size_t j = 0; j < face_bound_vec[i].size(); j++ )
        {
            face_ss << "#" << face_bound_vec[i][j]->GetFileId();

            if ( j < face_bound_vec[i].size() - 1 )
            {
                face_ss << ", ";
            }
        }

        adv_face->bounds_()->AddNode( new GenericAggrNode( face_ss.str().c_str() ) );

        adv_vec.push_back( adv_face );
        face_ids.push_back( first_face + ( int )i );
    }

    return adv_vec;
}

vector < NURBS_Curve > NURBS_Surface::MatchNURBSCurves( const vector < NURBS_Curve > &all_curve_vec )
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

//////////////////////////////////////////////////////
//================ STEP_Topology ===================//
//////////////////////////////////////////////////////

STEP_Topology::STEP_Topology( const vector < NURBS_Curve > &curve_vec, const vector < NURBS_Surface > &surf_vec ) :
    m_CurveVec( curve_vec )
{
    m_MaxEndGap = 0.0;

    int nend = 2 * ( int )m_CurveVec.size();

    // Union-find over the curve ends; m_EndVertVec holds each end's parent until the end
    m_EndVertVec.resize( nend );
    for ( int i = 0; i < nend; i++ )
    {
        m_EndVertVec[i] = i;
    }

    for ( size_t si = 0; si < surf_vec.size(); si++ )
    {
        const vector < NURBS_Loop > &loop_vec = surf_vec[si].m_NURBSLoopVec;

        for ( size_t li = 0; li < loop_vec.size(); li++ )
        {
            if ( !loop_vec[li].m_ClosedFlag )
            {
                continue;
            }

            const vector < pair < NURBS_Curve, bool > > &oc = loop_vec[li].m_OrderedCurves;
            int n = ( int )oc.size();

            for ( int k = 0; k < n; k++ )
            {
                const pair < NURBS_Curve, bool > &next = oc[ ( k + 1 ) % n ];

                // A curve walked backwards leaves from its end and arrives at its start
                int arrive = EndIndex( oc[k].first.m_CurveID, oc[k].second );
                int leave = EndIndex( next.first.m_CurveID, !next.second );

                int ra = FindRoot( arrive );
                int rl = FindRoot( leave );
                if ( ra != rl )
                {
                    m_EndVertVec[ rl ] = ra;
                }
            }
        }
    }

    vector < int > root_vert( nend, -1 );
    vector < int > nmember;

    for ( int i = 0; i < nend; i++ )
    {
        int r = FindRoot( i );

        if ( root_vert[r] < 0 )
        {
            root_vert[r] = ( int )m_VertPntVec.size();
            m_VertPntVec.push_back( vec3d() );
            nmember.push_back( 0 );
        }

        const vector < vec3d > &pnt_vec = m_CurveVec[ i / 2 ].m_CADPntVec;
        if ( i % 2 == 0 )
        {
            m_VertPntVec[ root_vert[r] ] = m_VertPntVec[ root_vert[r] ] + pnt_vec.front();
        }
        else
        {
            m_VertPntVec[ root_vert[r] ] = m_VertPntVec[ root_vert[r] ] + pnt_vec.back();
        }
        nmember[ root_vert[r] ]++;
    }

    vector < int > end_vert( nend );
    for ( int i = 0; i < nend; i++ )
    {
        end_vert[i] = root_vert[ FindRoot( i ) ];
    }
    m_EndVertVec = end_vert;

    for ( size_t v = 0; v < m_VertPntVec.size(); v++ )
    {
        m_VertPntVec[v] = m_VertPntVec[v] / ( double )nmember[v];
    }

    m_VertVec.resize( m_VertPntVec.size(), nullptr );
    m_EdgeVec.resize( m_CurveVec.size(), nullptr );
    m_NumFace = 0;
    m_Face = -1;
}

void STEP_Topology::Shells( const vector < int > &face_vec, vector < vector < int > > &shell_vec, vector < bool > &closed_vec ) const
{
    shell_vec.clear();
    closed_vec.clear();

    // Faces joined through a shared curve, as a union-find over positions in face_vec
    int nface = ( int )face_vec.size();
    vector < int > parent( nface );
    for ( int i = 0; i < nface; i++ )
    {
        parent[i] = i;
    }

    unordered_map < int, int > first_user;
    for ( int i = 0; i < nface; i++ )
    {
        unordered_map < int, vector < int > >::const_iterator it = m_FaceCurveMap.find( face_vec[i] );
        if ( it == m_FaceCurveMap.end() )
        {
            continue;
        }
        for ( size_t k = 0; k < it->second.size(); k++ )
        {
            int curve_id = it->second[k];
            unordered_map < int, int >::iterator f = first_user.find( curve_id );
            if ( f == first_user.end() )
            {
                first_user[ curve_id ] = i;
                continue;
            }

            int a = i;
            while ( parent[a] != a )
            {
                a = parent[a];
            }
            int b = f->second;
            while ( parent[b] != b )
            {
                b = parent[b];
            }
            if ( a < b )
            {
                parent[b] = a;
            }
            else if ( b < a )
            {
                parent[a] = b;
            }
        }
    }

    // Each root is its shell's first face, so going through the faces in order makes the shells
    // in the order of their first faces
    unordered_map < int, int > shell_of_root;
    for ( int i = 0; i < nface; i++ )
    {
        int r = i;
        while ( parent[r] != r )
        {
            r = parent[r];
        }
        unordered_map < int, int >::iterator s = shell_of_root.find( r );
        if ( s == shell_of_root.end() )
        {
            shell_of_root[ r ] = ( int )shell_vec.size();
            shell_vec.push_back( vector < int > ( 1, i ) );
        }
        else
        {
            shell_vec[ s->second ].push_back( i );
        }
    }

    for ( size_t s = 0; s < shell_vec.size(); s++ )
    {
        unordered_map < int, int > use;
        for ( size_t k = 0; k < shell_vec[s].size(); k++ )
        {
            unordered_map < int, vector < int > >::const_iterator it = m_FaceCurveMap.find( face_vec[ shell_vec[s][k] ] );
            if ( it == m_FaceCurveMap.end() )
            {
                continue;
            }
            for ( size_t c = 0; c < it->second.size(); c++ )
            {
                use[ it->second[c] ]++;
            }
        }

        bool closed = true;
        for ( unordered_map < int, int >::const_iterator u = use.begin(); u != use.end(); ++u )
        {
            if ( u->second != 2 )
            {
                closed = false;
            }
        }
        closed_vec.push_back( closed );
    }
}

int STEP_Topology::FindRoot( int i )
{
    while ( m_EndVertVec[i] != i )
    {
        m_EndVertVec[i] = m_EndVertVec[ m_EndVertVec[i] ];
        i = m_EndVertVec[i];
    }
    return i;
}

SdaiVertex_point* STEP_Topology::GetVertex( STEPutil* step, int vert )
{
    if ( !m_VertVec[vert] )
    {
        m_VertVec[vert] = step->MakeVertex( m_VertPntVec[vert] );
    }
    return m_VertVec[vert];
}

SdaiEdge_curve* STEP_Topology::GetEdge( STEPutil* step, int curve_id, bool mergepts )
{
    m_FaceCurveMap[ m_Face ].push_back( curve_id );

    if ( !m_EdgeVec[curve_id] )
    {
        const NURBS_Curve &crv = m_CurveVec[curve_id];

        int vstart = m_EndVertVec[ EndIndex( curve_id, false ) ];
        int vend = m_EndVertVec[ EndIndex( curve_id, true ) ];

        m_MaxEndGap = std::max( m_MaxEndGap, dist( crv.m_CADPntVec.front(), m_VertPntVec[vstart] ) );
        m_MaxEndGap = std::max( m_MaxEndGap, dist( crv.m_CADPntVec.back(), m_VertPntVec[vend] ) );

        m_EdgeVec[curve_id] = crv.WriteSTEPEdge( step, GetVertex( step, vstart ), GetVertex( step, vend ),
                                                 to_string( curve_id ), mergepts );

        // Place the curve on each surface written that it lies on
        vector < SdaiPcurve* > pcurve_vec;

        unordered_map < int, SdaiSurface* >::const_iterator it = m_SurfMap.find( crv.m_SurfA_ID );
        if ( it != m_SurfMap.end() )
        {
            pcurve_vec.push_back( step->MakePCurve( it->second, crv.m_CADUWPntVec_A, crv.m_CADUWDeg, crv.m_CADUWBreakVec_A ) );
        }

        // A border a surface meets itself along gets one on each side, at its two places there
        it = m_SurfMap.find( crv.m_SurfB_ID );
        if ( it != m_SurfMap.end() )
        {
            pcurve_vec.push_back( step->MakePCurve( it->second, crv.m_CADUWPntVec_B, crv.m_CADUWDeg, crv.m_CADUWBreakVec_B ) );
        }

        if ( !pcurve_vec.empty() )
        {
            SdaiEdge_curve* edge = m_EdgeVec[curve_id];
            edge->edge_geometry_( step->MakeCurveOnSurfaces( edge->edge_geometry_(), pcurve_vec, to_string( curve_id ) ) );
        }
    }
    return m_EdgeVec[curve_id];
}
