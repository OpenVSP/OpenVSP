//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SubSurface.cpp
// Alex Gary
//////////////////////////////////////////////////////////////////////

#include "SubSurface.h"
#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VspSurf.h"
#include "ParmMgr.h"
#include "Util.h"
#include "Matrix.h"
#include "Defines.h"

SubSurface::SubSurface( string compID, int type )
{
    m_Type = type;
    m_CompID = compID;
    m_Tag = 0;
    m_UpdateDrawFlag = true;
    m_LineColor = vec3d( 0, 0, 0 );
    m_PolyPntsReadyFlag = false;
    m_FirstSplit = true;
    m_PolyFlag = true;
}

SubSurface::~SubSurface()
{
}

void SubSurface::ParmChanged( Parm* parm_ptr, int type )
{
    Update();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void SubSurface::SetDisplaySuffix( int num )
{
    for ( int i = 0 ; i < ( int )m_ParmVec.size() ; i++ )
    {
        Parm* p = ParmMgr.FindParm( m_ParmVec[i] );

        if ( p )
        {
            p->SetGroupDisplaySuffix( num );
        }
    }
}

void SubSurface::LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0 ; i < ( int )m_DrawObjVec.size() ; i++ )
    {
        m_DrawObjVec[i].m_LineColor = m_LineColor;
        m_DrawObjVec[i].m_GeomID = ( m_ID + to_string( ( long long )i ) );
        draw_obj_vec.push_back( &m_DrawObjVec[i] );
    }
}

vector< TMesh* > SubSurface::CreateTMeshVec()
{
    vector<TMesh*> tmesh_vec;
    tmesh_vec.resize( m_LVec.size() );
    for ( int ls = 0 ; ls < ( int ) m_LVec.size() ; ls++ )
    {
        tmesh_vec[ls] = m_LVec[ls].CreateTMesh();
    }

    return tmesh_vec;
}

void SubSurface::UpdateDrawObjs()
{
    if ( !m_UpdateDrawFlag )
    {
        return;
    }

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }
    Geom* geom = veh->FindGeom( m_CompID );
    m_DrawObjVec.clear();
    if ( geom )
    {
        vector< VspSurf > surf_vec;
        geom->GetSurfVec( surf_vec );
        m_DrawObjVec.resize( m_LVec.size()*surf_vec.size(), DrawObj() );
        int ind = 0;
        for ( int ls = 0 ; ls < ( int )m_LVec.size() ; ls++ )
        {
            int num_pnts = CompNumDrawPnts( geom );
            int *num_pnts_ptr = NULL;
            if ( num_pnts > 0 )
            {
                num_pnts_ptr = &num_pnts;
            }

            for ( int s = 0 ; s < ( int )surf_vec.size() ; s++ )
            {
                m_LVec[ls].UpdateDrawObj( &surf_vec[s], geom, m_DrawObjVec[ind], num_pnts_ptr );
                ind++;
            }
        }
    }
}

void SubSurface::Update()
{
    m_PolyPntsReadyFlag = false;
    UpdateDrawObjs();
}

std::string SubSurface::GetTypeName( int type )
{
    if ( type == vsp::SS_LINE )
    {
        return string( "Line" );
    }
    if ( type == vsp::SS_RECTANGLE )
    {
        return string( "Rectangle" );
    }
    if ( type == vsp::SS_ELLIPSE )
    {
        return string( "Ellipse" );
    }
    return string( "NONE" );
}


// Encode Data into XML file
xmlNodePtr SubSurface::EncodeXml( xmlNodePtr & node )
{
    ParmContainer::EncodeXml( node );

    xmlNodePtr ss_info = xmlNewChild( node, NULL, BAD_CAST "SubSurfaceInfo", NULL );
    XmlUtil::AddIntNode( ss_info, "Type", m_Type );

    return ss_info;
}

bool SubSurface::Subtag( const vec3d & center )
{
    if ( !m_PolyPntsReadyFlag ) // Update polygon vector
    {
        m_PolyPnts.clear();

        int last_ind = m_LVec.size() - 1;
        vec3d pnt;
        for ( int ls = 0 ; ls < last_ind + 1 ; ls++ )
        {
            pnt = m_LVec[ls].GetP0();
            m_PolyPnts.push_back( vec2d( pnt.x(), pnt.y() ) );
        }
        pnt = m_LVec[last_ind].GetP1();
        m_PolyPnts.push_back( vec2d( pnt.x(), pnt.y() ) );

        m_PolyPntsReadyFlag = true;
    }

    bool inPoly = PointInPolygon( vec2d( center.x(), center.y() ) , m_PolyPnts );

    if ( ( inPoly && m_TestType() == INSIDE ) || ( !inPoly && m_TestType() == OUTSIDE ) )
    {
        return true;
    }

    return false;
}
bool SubSurface::Subtag( TTri* tri )
{
    vec3d center = tri->ComputeCenterUW();
    return Subtag( center );
}


void SubSurface::SplitSegs( const vector<int> & split_u, const vector<int> & split_w )
{
    // Method to Split subsurfaces for CFDMesh surfs
    CleanUpSplitVec();

    PrepareSplitVec();

    for ( int ui = 0 ; ui < ( int )split_u.size() ; ui++ )
    {
        SplitSegsU( split_u[ui] );
    }

    for ( int wi = 0 ; wi < ( int )split_w.size() ; wi++ )
    {
        SplitSegsW( split_w[wi] );
    }
}

void SubSurface::SplitSegsU( const double & u )
{
    double tol = 1e-10;
    int num_l_segs = m_SplitLVec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0 ; i < num_l_segs ; i++ )
    {
        SSLineSeg& seg = m_SplitLVec[i];
        vec3d p0 = seg.GetP0();
        vec3d p1 = seg.GetP1();

        double t = ( u - p0.x() ) / ( p1.x() - p0.x() );

        if ( t < 1 - tol && t > 0 + tol )
        {
            if ( m_FirstSplit )
            {
                m_FirstSplit = false;
                reorder = true;
            }
            // Split the segments
            vec3d int_pnt = point_on_line( p0, p1, t );
            SSLineSeg split_seg = SSLineSeg( seg );

            seg.SetP1( int_pnt );
            split_seg.SetP0( int_pnt );
            inds.push_back( i + num_splits + 1 );
            new_lsegs.push_back( split_seg );
            num_splits++;
        }
    }

    for ( int i = 0; i < ( int )inds.size() ; i++ )
    {
        m_SplitLVec.insert( m_SplitLVec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0] );
    }
}

void SubSurface::SplitSegsW( const double & w )
{
    double tol = 1e-10;
    int num_l_segs = m_SplitLVec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0 ; i < num_l_segs ; i++ )
    {

        SSLineSeg& seg = m_SplitLVec[i];
        vec3d p0 = seg.GetP0();
        vec3d p1 = seg.GetP1();

        double t = ( w - p0.y() ) / ( p1.y() - p0.y() );

        if ( t < 1 - tol && t > 0 + tol )
        {
            if ( m_FirstSplit )
            {
                m_FirstSplit = false;
                reorder = true;
            }
            // Split the segments
            vec3d int_pnt = point_on_line( p0, p1, t );
            SSLineSeg split_seg = SSLineSeg( seg );

            seg.SetP1( int_pnt );
            split_seg.SetP0( int_pnt );
            inds.push_back( i + num_splits + 1 );
            new_lsegs.push_back( split_seg );
            num_splits++;
        }
    }

    for ( int i = 0; i < ( int )inds.size() ; i++ )
    {
        m_SplitLVec.insert( m_SplitLVec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0] );
    }
}

void SubSurface::ReorderSplitSegs( int ind )
{
    if ( ind < 0 || ind > ( int )m_SplitLVec.size() - 1 )
    {
        return;
    }

    vector<SSLineSeg> ret_vec;
    ret_vec.resize( m_SplitLVec.size() );

    int cnt = 0;
    for ( int i = ind ; i < ( int )m_SplitLVec.size() ; i++ )
    {
        ret_vec[cnt] = m_SplitLVec[i];
        cnt++;
    }
    for ( int i = 0 ; i < ind ; i++ )
    {
        ret_vec[cnt] = m_SplitLVec[i];
        cnt++;
    }

    m_SplitLVec = ret_vec;
}

void SubSurface::CleanUpSplitVec()
{
    m_SplitLVec.clear();
}

void SubSurface::PrepareSplitVec()
{
    CleanUpSplitVec();
    m_FirstSplit = true;
    m_SplitLVec = m_LVec;
}

//////////////////////////////////////////////////////
//=================== SSLineSeg =====================//
//////////////////////////////////////////////////////

SSLineSeg::SSLineSeg()
{
    m_TestType = GT;
}

SSLineSeg::~SSLineSeg()
{
}

bool SSLineSeg::Subtag( const vec3d & center ) const
{
    // Compute cross product of line and first point to center
    vec3d v0c = center - m_P0;
    vec3d c_prod = cross( m_line, v0c );

    if ( m_TestType == GT && c_prod.z() > 0 )
    {
        return true;
    }
    if ( m_TestType == LT && c_prod.z() < 0 )
    {
        return true;
    }

    return false;
}

bool SSLineSeg::Subtag( TTri* tri ) const
{
    vec3d center = tri->ComputeCenterUW();

    return Subtag( center );
}

void SSLineSeg::Update( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return;
    }

    double umax = surf->GetUMax();
    double wmax = surf->GetWMax();
    // Update none scaled points
    m_P0.set_xyz( m_SP0[0]*umax, m_SP0[1]*wmax, 0 );
    m_P1.set_xyz( m_SP1[0]*umax, m_SP1[1]*wmax, 0 );

    // Update line
    m_line = m_P1 - m_P0;
}

int SSLineSeg::CompNumDrawPnts( VspSurf* surf, Geom* geom )
{
    if ( !surf || !geom )
    {
        return 0;
    }
    double avg_num_secs = ( double )( surf->GetUMax() + surf->GetWMax() ) / 2.0;
    double avg_tess = ( double )( geom->m_TessU() + geom->m_TessW() ) / 2.0;

    return ( int )( ( avg_num_secs ) * ( avg_tess - 1 ) );
}

void SSLineSeg::UpdateDrawObj( VspSurf* surf, Geom* geom, DrawObj& draw_obj, const int *num_pnts_ptr )
{
    int num_pnts;
    if ( num_pnts_ptr )
    {
        num_pnts = *num_pnts_ptr;
    }
    else
    {
        num_pnts = CompNumDrawPnts( surf, geom );
    }

    draw_obj.m_PntVec.resize( num_pnts + 1 );

    for ( int i = 0 ; i <= num_pnts ; i ++ )
    {
        vec3d uw = ( m_P0 + m_line * ( ( double )i / num_pnts ) );
        draw_obj.m_PntVec[i] = CompPnt( surf, uw );
    }

    draw_obj.m_LineWidth = 3.0;
    draw_obj.m_LineColor = vec3d( 177.0 / 255, 1, 58.0 / 255 );
    draw_obj.m_Type = DrawObj::VSP_LINE_STRIP;
    draw_obj.m_GeomChanged = true;
}

vec3d SSLineSeg::CompPnt( VspSurf* surf, vec3d uw_pnt ) const
{
    if ( !surf )
    {
        return vec3d();
    }

    int num_u = ( int )surf->GetUMax();
    int num_w = ( int )surf->GetWMax();

    if ( uw_pnt.x() < 0 )
    {
        uw_pnt.set_x( 0 );
    }
    else if ( uw_pnt.x() > num_u )
    {
        uw_pnt.set_x( num_u );
    }

    if ( uw_pnt.y() < 0 )
    {
        uw_pnt.set_y( 0 );
    }
    else if ( uw_pnt.y() > num_w )
    {
        uw_pnt.set_y( num_w );
    }

    return surf->CompPnt( uw_pnt.x(), uw_pnt.y() );
}

TMesh* SSLineSeg::CreateTMesh()
{
    int num_cut_lines = 0;
    int num_z_lines = 0;

    TMesh* tmesh = new TMesh();

    vec3d dc = m_line / ( num_cut_lines + 1.0 );
    vec3d dz = vec3d( 0, 0, 2.0 ) / ( num_z_lines + 1 );
    vec3d start = m_P0 + vec3d( 0, 0, -1 );

    int c, cz;

    vector< vector< vec3d > > pnt_mesh;
    pnt_mesh.resize( num_cut_lines + 2 );
    for ( int i = 0; i < ( int )pnt_mesh.size(); i++ )
    {
        pnt_mesh[i].resize( num_z_lines + 2 );
    }

    // Build plane
    for ( c = 0 ; c < num_cut_lines + 2 ; c++ )
    {
        for ( cz = 0 ; cz < num_z_lines + 2 ; cz++ )
        {
            pnt_mesh[c][cz] = start + dc * c + dz * cz;
        }
    }

    // Build triangles on that plane

    for ( c = 0 ; c < ( int )pnt_mesh.size() - 1 ; c++ )
    {
        for ( cz = 0 ; cz < ( int )pnt_mesh[c].size() - 1 ; cz ++ )
        {
            vec3d v0, v1, v2, v3, d01, d21, d20, d03, d23, norm;

            v0 = pnt_mesh[c][cz];
            v1 = pnt_mesh[c + 1][cz];
            v2 = pnt_mesh[c + 1][cz + 1];
            v3 = pnt_mesh[c][cz + 1];

            d21 = v2 - v1;
            d01 = v0 - v1;
            d20 = v2 - v0;

            if ( d21.mag() > 0.000001 && d01.mag() > 0.000001 && d20.mag() > 0.000001 )
            {
                norm = cross( d21, d01 );
                norm.normalize();
                tmesh->AddUWTri( v0, v1, v2, norm );
            }

            d03 = v0 - v3;
            d23 = v2 - v3;
            if ( d03.mag() > 0.000001 && d23.mag() > 0.000001 && d20.mag() > 0.000001 )
            {
                norm = cross( d03, d23 );
                norm.normalize();
                tmesh->AddUWTri( v0, v2, v3, norm );
            }
        }
    }
    return tmesh;
}

//////////////////////////////////////////////////////
//=================== SSLine =====================//
//////////////////////////////////////////////////////

SSLine::SSLine( string comp_id, int type ) : SubSurface( comp_id, type )
{
    m_ConstType.Init( "Const_Line_Type", "SubSurface", this, CONST_U, 0, 1, false );
    m_ConstVal.Init( "Const_Line_Value", "SubSurface", this, 0.5, 0, 1 );
    m_ConstVal.SetDescript( "Either the U or V value of the line depending on what constant line type is choosen." );
    m_TestType.Init( "Test_Type", "SubSurface", this, SSLineSeg::GT, SSLineSeg::GT, SSLineSeg::LT, false );
    m_TestType.SetDescript( "Tag surface as being either greater than or less than const value line" );

    m_LVec.push_back( SSLineSeg() );
}

SSLine::~SSLine()
{
}

void SSLine::Update()
{
    // Using m_LVec[0] since SSLine should always only have one line segment
    // Update SSegLine points based on current values
    if ( m_ConstType() == CONST_U )
    {
        m_LVec[0].SetSP0( vec3d( m_ConstVal(), 1, 0 ) );
        m_LVec[0].SetSP1( vec3d( m_ConstVal(), 0, 0 ) );
    }
    else if ( m_ConstType() == CONST_W )
    {
        m_LVec[0].SetSP0( vec3d( 0, m_ConstVal(), 0 ) );
        m_LVec[0].SetSP1( vec3d( 1, m_ConstVal(), 0 ) );
    }

    m_LVec[0].m_TestType = m_TestType();
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom )
    {
        return;
    }
    m_LVec[0].Update( geom );

    SubSurface::Update();
}

int SSLine::CompNumDrawPnts( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return 0;
    }

    if ( m_ConstType() == CONST_W )
    {
        return ( int )( surf->GetUMax() * ( geom->m_TessU() - 2 ) );
    }
    else if ( m_ConstType() == CONST_U )
    {
        return ( int )( surf->GetWMax() * ( geom->m_TessW() - 4 ) );
    }

    return -1;
}

bool SSLine::Subtag( TTri* tri )
{
    return m_LVec[0].Subtag( tri );
}

bool SSLine::Subtag( const vec3d & center )
{
    return m_LVec[0].Subtag( center );
}

//////////////////////////////////////////////////////
//=================== SSSquare =====================//
//////////////////////////////////////////////////////

//===== Constructor =====//
SSRectangle::SSRectangle( string comp_id, int type ) : SubSurface( comp_id, type )
{
    m_CenterU.Init( "Center_U", "SS_Rectangle", this, 0.5, 0, 1 );
    m_CenterU.SetDescript( "Defines the U location of the rectangle center" );
    m_CenterW.Init( "Center_W", "SS_Rectangle", this, 0.5, 0, 1 );
    m_CenterW.SetDescript( "Defines the W location of the rectangle center" );
    m_ULength.Init( "U_Length", "SS_Rectangle", this, .2, 0, 1 );
    m_ULength.SetDescript( "Defines length of rectangle in U direction before rotation" );
    m_WLength.Init( "W_Length", "SS_Rectangle", this, .2, 0, 1 );
    m_WLength.SetDescript( "Defines length of rectangle in W direction before rotation" );
    m_Theta.Init( "Theta", "SS_Rectangle", this, 0, -90, 90 );
    m_Theta.SetDescript( "Defines angle in degrees from U axis to rotate the rectangle" );
    m_TestType.Init( "Test_Type", "SS_Rectangle", this, INSIDE, INSIDE, OUTSIDE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );

    // Each Rectangle will always have 4 line segments
    for ( int i = 0; i < 4 ; i++ )
    {
        m_LVec.push_back( SSLineSeg() );
    }
}

//===== Destructor =====//
SSRectangle::~SSRectangle()
{
}

void SSRectangle::Update()
{
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom )
    {
        return;
    }

    vec3d center;
    vector< vec3d > pntVec;

    center = vec3d( m_CenterU(), m_CenterW(), 0 );

    // Rotation Matrix
    Matrix4d transMat1, transMat2;
    Matrix4d rotMat;
    rotMat.loadIdentity();
    rotMat.rotateZ( m_Theta() );
    transMat1.loadIdentity();
    transMat1.translatef( center.x() * -1, center.y() * -1, 0 );
    transMat2.loadIdentity();
    transMat2.translatef( center.x(), center.y(), 0 );

    // Make points in counter clockwise fashion;
    pntVec.resize( 5 );
    pntVec[0] = center + vec3d( m_ULength(), m_WLength(), 0 ) * -0.5;
    pntVec[1] = center + vec3d( m_ULength(), -1.0 * m_WLength(), 0 ) * 0.5;
    pntVec[2] = center + vec3d( m_ULength(), m_WLength(), 0 ) * 0.5;
    pntVec[3] = center + vec3d( -1.0 * m_ULength(), m_WLength(), 0 ) * 0.5;
    pntVec[4] = pntVec[0];

    // Apply transformations
    for ( int i = 0; i < 5 ; i++ )
    {
        pntVec[i] = transMat2.xform( rotMat.xform( transMat1.xform( pntVec[i] ) ) );
    }

    int pind = 0;

    for ( int i = 0 ; i < 4; i++ )
    {
        m_LVec[i].SetSP0( pntVec[pind] );
        pind++;
        m_LVec[i].SetSP1( pntVec[pind] );
        m_LVec[i].Update( geom );
    }

    SubSurface::Update();
}

//////////////////////////////////////////////////////
//=================== SSEllipse =====================//
//////////////////////////////////////////////////////

//====== Constructor =====//
SSEllipse::SSEllipse( string comp_id, int type ) : SubSurface( comp_id, type )
{
    m_CenterU.Init( "Center_U", "SS_Ellipse", this, 0.5, 0, 1 );
    m_CenterU.SetDescript( "Defines the U location of the ellipse center" );
    m_CenterW.Init( "Center_W", "SS_Ellipse", this, 0.5, 0, 1 );
    m_CenterW.SetDescript( "Defines the W location of the ellipse center" );
    m_ULength.Init( "U_Length", "SS_Ellipse", this, 0.2, 0, 1 );
    m_ULength.SetDescript( "Length of ellipse in the u direction" );
    m_WLength.Init( "W_Length", "SS_Ellipse", this, 0.2, 0, 1 );
    m_WLength.SetDescript( "Length of ellipse in the w direction" );
    m_Theta.Init( "Theta", "SS_Ellipse", this, 0, -90, 90 );
    m_Theta.SetDescript( "Defines angle in degrees from U axis to rotate the rectangle" );
    m_Tess.Init( "Tess_Num", "SS_Ellipse", this, 15, 3, 1000 );
    m_Tess.SetDescript( " Number of points to discretize curve" );
    m_TestType.Init( "Test_Type", "SS_Ellipse", this, INSIDE, INSIDE, OUTSIDE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );

    m_PolyFlag = false;
}

//===== Destructor =====//
SSEllipse::~SSEllipse()
{

}

// Resize LVec if Tessellation has changed
void SSEllipse::UpdateLVecSize()
{
    // Do nothing if already the correct size
    if ( m_LVec.size() == m_Tess() )
    {
        return;
    }

    // If too few more line segments
    if ( ( int )m_LVec.size() < m_Tess() )
    {
        for ( int i = m_LVec.size() ; i < m_Tess() ; i++ )
        {
            m_LVec.push_back( SSLineSeg() );
        }
    }
    else if ( ( int )m_LVec.size() > m_Tess() )
    {
        // if too many line segments delete extra ones

        m_LVec.erase( m_LVec.begin() + m_Tess(), m_LVec.begin() + m_LVec.size() );
    }
}
// Main Update Routine
void SSEllipse::Update()
{
    UpdateLVecSize();

    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom )
    {
        return;
    }

    int num_pnts = m_Tess();

    vec3d center;

    center = vec3d( m_CenterU(), m_CenterW(), 0 );

    // Rotation Matrix
    Matrix4d transMat1, transMat2;
    Matrix4d rotMat;
    rotMat.loadIdentity();
    rotMat.rotateZ( m_Theta() );
    transMat1.loadIdentity();
    transMat1.translatef( center.x() * -1, center.y() * -1, 0 );
    transMat2.loadIdentity();
    transMat2.translatef( center.x(), center.y(), 0 );

    double a = m_ULength() / 2;
    double b = m_WLength() / 2;
    for ( int i = 0 ; i < num_pnts ; i++ )
    {
        double p0 = 2 * PI * ( double )i / num_pnts;
        double p1 = 2 * PI * ( double )( i + 1 ) / num_pnts;
        vec3d pnt = vec3d();
        pnt.set_xyz( a * cos( p0 ) + m_CenterU(), b * sin( p0 ) + m_CenterW(), 0 );
        pnt = transMat2.xform( rotMat.xform( transMat1.xform( pnt ) ) );
        m_LVec[i].SetSP0( pnt );
        pnt.set_xyz( a * cos( p1 ) + m_CenterU(), b * sin( p1 ) + m_CenterW(), 0 );
        pnt = transMat2.xform( rotMat.xform( transMat1.xform( pnt ) ) );
        m_LVec[i].SetSP1( pnt );
        m_LVec[i].Update( geom );
    }

    SubSurface::Update();

}
