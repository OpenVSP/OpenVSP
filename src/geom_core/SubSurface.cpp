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
#include "ParmMgr.h"
#include "StructureMgr.h"

#include "eli/geom/intersect/specified_distance_curve.hpp"

SubSurface::SubSurface( string compID, int type )
{
    m_Type = type;
    m_CompID = compID;
    m_Tag = 0;
    m_LineColor = vec3d( 0, 0, 0 );
    m_PolyPntsReadyFlag = false;
    m_FirstSplit = true;
    m_PolyFlag = true;

    m_MainSurfIndx.Init( "MainSurfIndx", "SubSurface", this, -1, -1, 1e12 );
    m_MainSurfIndx.SetDescript( "Surface index for subsurface" );

    m_IncludeType.Init("IncludeFlag", "SubSurface", this, vsp::SS_INC_TREAT_AS_PARENT, vsp::SS_INC_TREAT_AS_PARENT, vsp::SS_INC_ZERO_DRAG);
    m_IncludeType.SetDescript("Indicates whether or not to include wetted area of subsurf in parasite drag calcs");

    // Parasite Drag Parms
    m_PercLam.Init("PercLam", "ParasiteDragProps", this, 0, 0, 100 );
    m_PercLam.SetDescript("Percentage Laminar" );

    m_FFBodyEqnType.Init("FFBodyEqnType", "ParasiteDragProps", this, vsp::FF_B_HOERNER_STREAMBODY, vsp::FF_B_MANUAL, vsp::FF_B_JENKINSON_AFT_FUSE_NACELLE );
    m_FFBodyEqnType.SetDescript("Equation that defines the form factor of a body type surface included this Geom");

    m_FFWingEqnType.Init("FFWingEqnType", "ParasiteDragProps", this, vsp::FF_W_HOERNER, vsp::FF_W_MANUAL, vsp::FF_W_SCHEMENSKY_SUPERCRITICAL_AF );
    m_FFWingEqnType.SetDescript("Equation that defines the form factor of a wing type surface included this Geom");

    m_FFUser.Init("FFUser", "ParasiteDragProps", this, 1, -1, 10 );
    m_FFUser.SetDescript( "User Input Form Factor Value" );

    m_Q.Init("Q", "ParasiteDragProps", this, 1, 0, 3 );
    m_Q.SetDescript( "Interference Factor" );

    m_Roughness.Init("Roughness", "ParasiteDragProps", this, -1, -1, 10 );
    m_Roughness.SetDescript( "Roughness Height" );

    m_TeTwRatio.Init("TeTwRatio", "ParasiteDragProps", this, -1, -1, 1e6 );
    m_TeTwRatio.SetDescript("Temperature Ratio of Freestream to Wall" );

    m_TawTwRatio.Init("TawTwRatio", "ParasiteDragProps", this, -1, -1, 1e6 );
    m_TawTwRatio.SetDescript("Temperature Ratio of Ambient Wall to Wall" );

    m_IncludedElements.Init( "IncludedElements", "SubSurface", this, vsp::FEA_SHELL, vsp::FEA_SHELL, vsp::FEA_SHELL_AND_BEAM );
    m_IncludedElements.SetDescript( "Indicates the FeaElements to be Included for the SubSurface" );

    m_DrawFeaPartFlag.Init( "DrawFeaPartFlag", "FeaSubSurface", this, true, false, true );
    m_DrawFeaPartFlag.SetDescript( "Flag to Draw FEA SubSurface" );

    m_FeaPropertyIndex.Init( "FeaPropertyIndex", "FeaSubSurface", this, 0, 0, 1e12 );; // Shell property default
    m_FeaPropertyIndex.SetDescript( "FeaPropertyIndex for Shell Elements" );

    m_CapFeaPropertyIndex.Init( "CapFeaPropertyIndex", "FeaSubSurface", this, 1, 0, 1e12 );; // Beam property default
    m_CapFeaPropertyIndex.SetDescript( "FeaPropertyIndex for Beam (Cap) Elements" );
}

SubSurface::~SubSurface()
{
}

void SubSurface::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

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
    m_SubSurfDO.m_LineColor = m_LineColor;

    draw_obj_vec.push_back( &m_SubSurfDO );
}

void SubSurface::LoadPartialColoredDrawObjs( const string & ss_id, int surf_num, std::vector < DrawObj* > & draw_obj_vec, vec3d color )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    m_SubSurfHighlightDO[surf_num].m_LineColor = color;
    m_SubSurfHighlightDO[surf_num].m_GeomID = (m_ID + to_string((long long)surf_num));
    draw_obj_vec.push_back(&m_SubSurfHighlightDO[surf_num]);
}

vector< TMesh* > SubSurface::CreateTMeshVec()
{
    vector<TMesh*> tmesh_vec;
    tmesh_vec.resize(1);
    tmesh_vec[0] =  new TMesh();

    for ( int ls = 0 ; ls < ( int ) m_LVec.size() ; ls++ )
    {
        m_LVec[ls].AddToTMesh( tmesh_vec[0] );
    }

    return tmesh_vec;
}

void SubSurface::UpdateDrawObjs()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }
    Geom* geom = veh->FindGeom( m_CompID );
    m_SubSurfHighlightDO.clear();

    m_SubSurfDO.m_PntVec.clear();
    m_SubSurfDO.m_GeomID = m_ID + string( "_ss_line" );
    m_SubSurfDO.m_LineWidth = 3.0;
    m_SubSurfDO.m_Type = DrawObj::VSP_LINES;

    if ( geom )
    {
        vector< VspSurf > surf_vec;
        geom->GetSurfVec( surf_vec );
        int ncopy = geom->GetNumSymmCopies();

        m_SubSurfHighlightDO.resize( m_LVec.size()*ncopy, DrawObj() );
        int ind;
        for ( int ls = 0 ; ls < ( int )m_LVec.size() ; ls++ )
        {
            ind = 0;
            int num_pnts = CompNumDrawPnts( geom );

            int isurf = m_MainSurfIndx();

            vector < int > symms = geom->GetSymmIndexs( isurf );
            assert( ncopy == symms.size() );

            for ( int s = 0 ; s < ncopy ; s++ )
            {
                vector < vec3d > pts;
                m_LVec[ls].GetDOPts( &surf_vec[symms[s]], geom, pts, num_pnts );
                m_SubSurfDO.m_PntVec.insert( m_SubSurfDO.m_PntVec.end(), pts.begin(), pts.end() );

                m_SubSurfHighlightDO[ind].m_PntVec.insert( m_SubSurfHighlightDO[ind].m_PntVec.end(), pts.begin(), pts.end());
                m_SubSurfHighlightDO[ind].m_Type = DrawObj::VSP_LINES;
                m_SubSurfHighlightDO[ind].m_LineWidth = 5.0;
                ++ind;
            }
        }
    }
    m_SubSurfDO.m_GeomChanged = true;
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
    if ( type == vsp::SS_CONTROL )
    {
        return string( "Control_Surf" );
    }
    if ( type == vsp::SS_LINE_ARRAY )
    {
        return string( "Line_Array" );
    }
    if ( type == vsp::SS_FOURVERTPOLY)
    {
        return string( "Four vertex polygon" );
    }
    if ( type == vsp::SS_POLYGON)
    {
        return string( "Polygon" );
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
    UpdatePolygonPnts(); // Update polygon vector

    if ( m_TestType() == vsp::NONE )
    {
        return false;
    }

    for ( int p = 0; p < ( int )m_PolyPntsVec.size(); p++ )
    {
        bool inPoly = PointInPolygon( vec2d( center.x(), center.y() ), m_PolyPntsVec[p] );

        if ( inPoly && m_TestType() == vsp::INSIDE )
        {
            return true;
        }
        else if ( inPoly && m_TestType() == vsp::OUTSIDE )
        {
            return false;
        }
    }

    if ( m_TestType() == vsp::OUTSIDE )
    {
        return true;
    }

    return false;
}

int SubSurface::GetFeaMaterialIndex()
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex() );

    return fea_prop->m_FeaMaterialIndex();
}

void SubSurface::SetFeaMaterialIndex( int index )
{
    FeaProperty* fea_prop = StructureMgr.GetFeaProperty( m_FeaPropertyIndex() );

    fea_prop->m_FeaMaterialIndex.Set( index );
}

//==================================//
// This method updates the polygon points that define the polygon(s) used for the
// point in polygon test used to determine which triangles are inside or outside
// of the subsurface region
//==================================//
void SubSurface::UpdatePolygonPnts()
{
    if ( m_PolyPntsReadyFlag )
    {
        return;
    }

    m_PolyPntsVec.resize( 1 );

    m_PolyPntsVec[0].clear();

    int last_ind = m_LVec.size() - 1;
    vec3d pnt;
    for ( int ls = 0; ls < last_ind + 1; ls++ )
    {
        pnt = m_LVec[ls].GetP0();
        m_PolyPntsVec[0].push_back( vec2d( pnt.x(), pnt.y() ) );
    }
    pnt = m_LVec[last_ind].GetP1();
    m_PolyPntsVec[0].push_back( vec2d( pnt.x(), pnt.y() ) );

    m_PolyPntsReadyFlag = true;
}

bool SubSurface::Subtag( TTri* tri )
{
    vec3d center = tri->ComputeCenterUW();
    return Subtag( center );
}

void SubSurface::SplitSegsU( const double & u )
{
    for ( int i = 0; i < m_SplitLVec.size(); i++ )
    {
        SplitSegsU( u, m_SplitLVec[i] );
    }
}

void SubSurface::SplitSegsW( const double & w )
{
    for ( int i = 0; i < m_SplitLVec.size(); i++ )
    {
        SplitSegsW( w, m_SplitLVec[i] );
    }
}

void SubSurface::SplitSegsU( const double & u, vector<SSLineSeg> &splitvec )
{
    double tol = 1e-10;
    int num_l_segs = splitvec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0 ; i < num_l_segs ; i++ )
    {
        SSLineSeg& seg = splitvec[i];
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
        splitvec.insert( splitvec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0], splitvec );
    }
}

void SubSurface::SplitSegsW( const double & w, vector<SSLineSeg> &splitvec )
{
    double tol = 1e-10;
    int num_l_segs = splitvec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0 ; i < num_l_segs ; i++ )
    {

        SSLineSeg& seg = splitvec[i];
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
        splitvec.insert( splitvec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0], splitvec );
    }
}

void SubSurface::ReorderSplitSegs( int ind, vector<SSLineSeg> &splitvec )
{
    if ( ind < 0 || ind > ( int )splitvec.size() - 1 )
    {
        return;
    }

    vector<SSLineSeg> ret_vec;
    ret_vec.resize( splitvec.size() );

    int cnt = 0;
    for ( int i = ind ; i < ( int )splitvec.size() ; i++ )
    {
        ret_vec[cnt] = splitvec[i];
        cnt++;
    }
    for ( int i = 0 ; i < ind ; i++ )
    {
        ret_vec[cnt] = splitvec[i];
        cnt++;
    }

    splitvec = ret_vec;
}

void SubSurface::PrepareSplitVec()
{
    m_SplitLVec.clear();
    m_FirstSplit = true;
    m_SplitLVec.push_back( m_LVec );
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

    if ( m_TestType == NO )
    {
        return false;
    }

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

void SSLineSeg::GetDOPts( VspSurf* surf, Geom* geom, vector < vec3d > &pts, int num_pnts )
{
    if ( num_pnts < 0 )
    {
        num_pnts = CompNumDrawPnts( surf, geom );
    }

    pts.resize( 2 * num_pnts );

    vec3d pprev = CompPnt( surf, m_P0 );
    vec3d p = pprev;
    for ( int i = 0 ; i < num_pnts ; i ++ )
    {
        vec3d uw = ( m_P0 + m_line * ( ( double )i / ( num_pnts - 1 ) ) );
        p = CompPnt( surf, uw );
        pts[2*i] = pprev;
        pts[2*i+1] = p;
        pprev = p;
    }
}

vec3d SSLineSeg::CompPnt( VspSurf* surf, vec3d uw_pnt ) const
{
    if ( !surf )
    {
        return vec3d();
    }

    double maxu = surf->GetUMax();
    double maxw = surf->GetWMax();

    if ( uw_pnt.x() < 0.0 )
    {
        uw_pnt.set_x( 0.0 );
    }
    else if ( uw_pnt.x() > maxu )
    {
        uw_pnt.set_x( maxu );
    }

    if ( uw_pnt.y() < 0.0 )
    {
        uw_pnt.set_y( 0.0 );
    }
    else if ( uw_pnt.y() > maxw )
    {
        uw_pnt.set_y( maxw );
    }

    return surf->CompPnt( uw_pnt.x(), uw_pnt.y() );
}

TMesh* SSLineSeg::CreateTMesh()
{
    TMesh* tmesh = new TMesh();

    AddToTMesh( tmesh );

    return tmesh;
}

void SSLineSeg::AddToTMesh( TMesh* tmesh )
{

    int num_cut_lines = 0;
    int num_z_lines = 0;

    double tol = 1.0e-6;


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

            if ( d21.mag() > tol && d01.mag() > tol && d20.mag() > tol )
            {
                norm = cross( d21, d01 );
                norm.normalize();
                tmesh->AddUWTri( v0, v1, v2, norm );
            }

            d03 = v0 - v3;
            d23 = v2 - v3;
            if ( d03.mag() > tol && d23.mag() > tol && d20.mag() > tol )
            {
                norm = cross( d03, d23 );
                norm.normalize();
                tmesh->AddUWTri( v0, v2, v3, norm );
            }
        }
    }
}

//////////////////////////////////////////////////////
//=================== SSLine =====================//
//////////////////////////////////////////////////////

SSLine::SSLine( string comp_id, int type ) : SubSurface( comp_id, type )
{
    m_ConstType.Init( "Const_Line_Type", "SubSurface", this, vsp::CONST_U, 0, 1 );
    m_ConstVal.Init( "Const_Line_Value", "SubSurface", this, 0.5, 0, 1 );
    m_ConstVal.SetDescript( "Either the U or V value of the line depending on what constant line type is choosen." );
    m_TestType.Init( "Test_Type", "SubSurface", this, SSLineSeg::GT, SSLineSeg::GT, SSLineSeg::NO );
    m_TestType.SetDescript( "Tag surface as being either greater than or less than const value line" );

    m_LVec.resize( 1 );
}

SSLine::~SSLine()
{
}

void SSLine::Update()
{
    // Using m_LVec[0] since SSLine should always only have one line segment
    // Update SSegLine points based on current values
    if ( m_ConstType() == vsp::CONST_U )
    {
        m_LVec[0].SetSP0( vec3d( m_ConstVal(), 1, 0 ) );
        m_LVec[0].SetSP1( vec3d( m_ConstVal(), 0, 0 ) );
    }
    else if ( m_ConstType() == vsp::CONST_W )
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
    VspSurf* surf = geom->GetSurfPtr( m_MainSurfIndx() );
    if ( !surf )
    {
        return 0;
    }

    if ( m_ConstType() == vsp::CONST_W )
    {
        return ( int )( surf->GetUMax() * ( geom->m_TessU() - 2 ) );
    }
    else if ( m_ConstType() == vsp::CONST_U )
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

/////////////////////////////////////////////////////////////////
//=================== Four vertex polygon =====================//
/////////////////////////////////////////////////////////////////

//===== Constructor =====//
SSFourVertPoly::SSFourVertPoly( string comp_id, int type ) : SubSurface( comp_id, type )
{
    m_CenterU.Init( "Center_U", "SSFourVertPoly", this, 0.5, 0.0, 1.0); //................... U component of the mid point
    m_CenterU.SetDescript( "U component of the mid point" );

    m_CenterW.Init( "Center_W", "SSFourVertPoly", this, 0.5, 0.0, 1.0); //................... W component of the mid point
    m_CenterW.SetDescript( "W component of the mid point" );

    m_UppLftU.Init( "UpperLft_U", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //................. U component of upper left corner
    m_UppLftU.SetDescript( "U component of upper left corner" );

    m_UppLftW.Init( "UpperLft_W", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //................. W component of upper left corner
    m_UppLftW.SetDescript( "W component of upper left corner" );

    m_UppRhtU.Init( "UpperRht_U", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //................. U component of upper right corner
    m_UppRhtU.SetDescript( "U component of upper right corner" );

    m_UppRhtW.Init( "UpperRht_W", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //................. W component of upper right corner
    m_UppRhtW.SetDescript( "W component of upper right corner" );

    m_LwRhtU.Init( "LowerRht_U", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //.................. U component of lower right corner
    m_LwRhtU.SetDescript( "U component of lower right corner" );

    m_LwRhtW.Init( "LowerRht_W", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //.................. W component of lower right corner
    m_LwRhtW.SetDescript( "W component of lower right corner" );

    m_LwLftU.Init( "LowerLft_U", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //.................. U component of lower left corner
    m_LwLftU.SetDescript( "U component of lower left corner" );

    m_LwLftW.Init( "LowerLft_W", "SSFourVertPoly", this, 0.1, 0.0, 1.0); //.................. W component of lower left corner
    m_LwLftW.SetDescript( "W component of lower left corner" );

    m_NrmDevUpper.Init( "NormDev_Upper", "SSFourVertPoly", this, 0.0, -1.0, 1.0); //......... Normal deviation of the mid point on the upper line
    m_NrmDevUpper.SetDescript( "Normal deviation of the mid point on the upper line" );

    m_NrmDevRight.Init( "NormDev_Right", "SSFourVertPoly", this, 0.0, -1.0, 1.0); //......... Normal deviation of the mid point on the right line
    m_NrmDevRight.SetDescript( "Normal deviation of the mid point on the right line" );

    m_NrmDevBottom.Init( "NormDev_Bottom", "SSFourVertPoly", this, 0.0, -1.0, 1.0); //....... Normal deviation of the mid point on the bottom line
    m_NrmDevBottom.SetDescript( "Normal deviation of the mid point on the bottom line" );

    m_NrmDevLeft.Init( "NormDev_Left", "SSFourVertPoly", this, 0.0, -1.0, 1.0); //........... Normal deviation of the mid point on the left line
    m_NrmDevLeft.SetDescript( "Normal deviation of the mid point on the left line" );

    m_NrmDevUpperPos.Init( "NrmDevPos_Upper", "SSFourVertPoly", this, 0.5, 0.1, 0.9 ); //..... Position of the normal deviation vector on the upper side [0.0, 1.0]
    m_NrmDevUpperPos.SetDescript( "Position of the normal deviation vector on the upper side" );

    m_NrmDevRightPos.Init( "NrmDevPos_Right", "SSFourVertPoly", this, 0.5, 0.1, 0.9 ); //..... Position of the normal deviation vector on the right side [0.0, 1.0]
    m_NrmDevRightPos.SetDescript( "Position of the normal deviation vector on the right side" );

    m_NrmDevBottomPos.Init( "NrmDevPos_Bottom", "SSFourVertPoly", this, 0.5, 0.1, 0.9 ); //... Position of the normal deviation vector on the bottom side [0.0, 1.0]
    m_NrmDevBottomPos.SetDescript( "Position of the normal deviation vector on the bottom side" );

    m_NrmDevLeftPos.Init( "NrmDevPos_Left", "SSFourVertPoly", this, 0.5, 0.1, 0.9 ); //....... Position of the normal deviation vector on the left side [0.0, 1.0]
    m_NrmDevLeftPos.SetDescript( "Position of the normal deviation vector on the left side" );

    m_NrmDevUpperAng.Init( "NrmDevAng_Upper", "SSFourVertPoly", this, 0.0, -90.0, 90.0 ); //.. Rotation angle of the normal vector on the upper edge
    m_NrmDevUpperAng.SetDescript( "Rotation angle of the normal vector on the upper edge" );

    m_NrmDevRightAng.Init( "NrmDevAng_Right", "SSFourVertPoly", this, 0.0, -90.0, 90.0 ); //.. Rotation angle of the normal vector on the right edge
    m_NrmDevRightAng.SetDescript( "Rotation angle of the normal vector on the right edge" );

    m_NrmDevBottomAng.Init( "NrmDevAng_Bottom", "SSFourVertPoly", this, 0.0, -90.0, 90.0 ); // Rotation angle of the normal vector on the bottom edge
    m_NrmDevBottomAng.SetDescript( "Rotation angle of the normal vector on the bottom edge" );

    m_NrmDevLeftAng.Init( "NrmDevAng_Left", "SSFourVertPoly", this, 0.0, -90.0, 90.0 ); //.... Rotation angle of the normal vector on the left side
    m_NrmDevLeftAng.SetDescript( "Rotation angle of the normal vector on the left side" );

    m_RadUpperL.Init( "Rad_UpperL", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //........... Radius parameter on upper left edge
    m_RadUpperL.SetDescript( "Radius parameter on upper left edge" );

    m_RadUpperR.Init( "Rad_UpperR", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //........... Radius parameter on upper right edge
    m_RadUpperR.SetDescript( "Radius parameter on upper right edge" );

    m_RadRightL.Init( "Rad_RightL", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //........... Radius parameter on right left edge
    m_RadRightL.SetDescript( "Radius parameter on right left edge" );

    m_RadRightR.Init( "Rad_RightR", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //........... Radius parameter on right right edge
    m_RadRightR.SetDescript( "Radius parameter on right right edge" );

    m_RadBottomL.Init( "Rad_BottomL", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //......... Radius parameter on bottom left edge
    m_RadBottomL.SetDescript( "Radius parameter on bottom left edge" );

    m_RadBottomR.Init( "Rad_BottomR", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //......... Radius parameter on bottom right edge
    m_RadBottomR.SetDescript( "Radius parameter on bottom right edge" );

    m_RadLeftL.Init( "Rad_LeftL", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //............. Radius parameter on left left edge
    m_RadLeftL.SetDescript( "Radius parameter on left left edge" );

    m_RadLeftR.Init( "Rad_LeftR", "SSFourVertPoly", this, 0.0, 0.0, 1.0 ); //............. Radius parameter on left right edge
    m_RadLeftR.SetDescript( "Radius parameter on left right edge" );


    m_TessPtsNum.Init( "Num_TessPts", "SSFourVertPoly", this, 1000, 24, 1000 ); //............ Number of tessellation points on the polygon
    m_TessPtsNum.SetDescript( "Number of tessellation points on the polygon" );

    m_TestType.Init( "Test_Type", "SSFourVertPoly", this, vsp::INSIDE, vsp::INSIDE, vsp::NONE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );
}

//===== Destructor =====//
SSFourVertPoly::~SSFourVertPoly()
{
}

void SSFourVertPoly::Update()
{
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom )
    {
        return;
    }

//--Scheme of the curve-------------------------------------------------------//
// 16/0  1     2     3    4                                                   //
//    +--⨉-----◌-----⨉--+      + : Corner point                               //
//    |                 |      ⨉ : Radius control point                       //
// 15 ⨉                 ⨉ 5    ◌ : Control point for edge deflection          //
//    |                 |                                                     //
// 14 ◌                 ◌ 6                                                   //
//    |                 |                                                     //
// 13 ⨉                 ⨉ 7                                                   //
//    |                 |                                                     //
//    +--⨉-----◌-----⨉--+                                                     //
// 12    11   10     9    8                                                   //
//----------------------------------------------------------------------------//

    vector< vec3d > pntVec (17); //............................................ All points of the closed curve

//--Create corner vertices------------------------------------------------------
    pntVec[0]  = vec3d(m_CenterU()-m_UppLftU(), m_CenterW()-m_UppLftW(), 0.0);
    pntVec[4]  = vec3d(m_CenterU()+m_UppRhtU(), m_CenterW()-m_UppRhtW(), 0.0);
    pntVec[8]  = vec3d(m_CenterU()+m_LwRhtU(),  m_CenterW()+m_LwRhtW(),  0.0);
    pntVec[12] = vec3d(m_CenterU()-m_LwLftU(),  m_CenterW()+m_LwLftW(),  0.0);
    pntVec[16] = pntVec[0];

//--Compute points for edge deflection -----------------------------------------
    pntVec[2]  = pntVec[0]  + m_NrmDevUpperPos()  * ( pntVec[4]  - pntVec[0]  ); //...... Top
    pntVec[6]  = pntVec[4]  + m_NrmDevRightPos()  * ( pntVec[8]  - pntVec[4]  ); //...... Right
    pntVec[10] = pntVec[8]  + m_NrmDevBottomPos() * ( pntVec[12] - pntVec[8]  ); //...... Bottom
    pntVec[14] = pntVec[12] + m_NrmDevLeftPos()   * ( pntVec[16] - pntVec[12] ); //...... Left

//--Compute normal vectors -----------------------------------------------------
    vector< vec3d > nrmVec (4);
    nrmVec[0] = vec3d( (pntVec[4] - pntVec[0] ).y(), -(pntVec[4] - pntVec[0] ).x(), 0.0); //...... Top
    nrmVec[1] = vec3d( (pntVec[8] - pntVec[4] ).y(), -(pntVec[8] - pntVec[4] ).x(), 0.0); //...... Right
    nrmVec[2] = vec3d( (pntVec[12]- pntVec[8] ).y(), -(pntVec[12]- pntVec[8] ).x(), 0.0); //...... Bottom
    nrmVec[3] = vec3d( (pntVec[16]- pntVec[12]).y(), -(pntVec[16]- pntVec[12]).x(), 0.0); //...... Left

//--Compute lenght of normal vectors--------------------------------------------
    vector<double> nrmVec_len (4);
    nrmVec_len[0] = sqrt( nrmVec[0].x()*nrmVec[0].x() + nrmVec[0].y()*nrmVec[0].y() );
    nrmVec_len[1] = sqrt( nrmVec[1].x()*nrmVec[1].x() + nrmVec[1].y()*nrmVec[1].y() );
    nrmVec_len[2] = sqrt( nrmVec[2].x()*nrmVec[2].x() + nrmVec[2].y()*nrmVec[2].y() );
    nrmVec_len[3] = sqrt( nrmVec[3].x()*nrmVec[3].x() + nrmVec[3].y()*nrmVec[3].y() );

//--Normalize normal vectors----------------------------------------------------
    nrmVec[0] = 1.0/nrmVec_len[0] * nrmVec[0];
    nrmVec[1] = 1.0/nrmVec_len[1] * nrmVec[1];
    nrmVec[2] = 1.0/nrmVec_len[2] * nrmVec[2];
    nrmVec[3] = 1.0/nrmVec_len[3] * nrmVec[3];

//--Rotate normal vector--------------------------------------------------------
    nrmVec[0].rotate_z( cos(m_NrmDevUpperAng()*PI/180.0 ), sin(m_NrmDevUpperAng()*PI/180.0)  );
    nrmVec[1].rotate_z( cos(m_NrmDevRightAng()*PI/180.0 ), sin(m_NrmDevRightAng()*PI/180.0)  );
    nrmVec[2].rotate_z( cos(m_NrmDevBottomAng()*PI/180.0), sin(m_NrmDevBottomAng()*PI/180.0) );
    nrmVec[3].rotate_z( cos(m_NrmDevLeftAng()*PI/180.0  ), sin(m_NrmDevLeftAng()*PI/180.0)   );

//--Displace control points responsible for deflection using the normal vector--
    pntVec[2]  = pntVec[2]  + nrmVec[0] * m_NrmDevUpper(); //.................. Top
    pntVec[6]  = pntVec[6]  + nrmVec[1] * m_NrmDevRight(); //.................. Right
    pntVec[10] = pntVec[10] + nrmVec[2] * m_NrmDevBottom(); //................. Bottom
    pntVec[14] = pntVec[14] + nrmVec[3] * m_NrmDevLeft(); //................... Left

//--Compute vertices for vertex radius control----------------------------------
    pntVec[1]  = pntVec[0]  + m_RadUpperL()  * (pntVec[2]  - pntVec[0]  );
    pntVec[5]  = pntVec[4]  + m_RadRightL()  * (pntVec[6]  - pntVec[4]  );
    pntVec[9]  = pntVec[8]  + m_RadBottomL() * (pntVec[10] - pntVec[8]  );
    pntVec[13] = pntVec[12] + m_RadLeftL()   * (pntVec[14] - pntVec[12] );

    pntVec[3]  = pntVec[2]  + (1.0 - m_RadUpperR())  * (pntVec[4]  - pntVec[2] );
    pntVec[7]  = pntVec[6]  + (1.0 - m_RadRightR())  * (pntVec[8]  - pntVec[6] );
    pntVec[11] = pntVec[10] + (1.0 - m_RadBottomR()) * (pntVec[12] - pntVec[10] );
    pntVec[15] = pntVec[14] + (1.0 - m_RadLeftR())   * (pntVec[16] - pntVec[14] );

//--Create list of points for 8 piecewise quadratic curves----------------------
    vector< vec3d > ptsLst_01 (3);
    ptsLst_01[0] = pntVec[15];
    ptsLst_01[1] = pntVec[0];
    ptsLst_01[2] = pntVec[1];

    vector< vec3d > ptsLst_02 (3);
    ptsLst_02[0] = pntVec[1];
    ptsLst_02[1] = pntVec[2];
    ptsLst_02[2] = pntVec[3];

    vector< vec3d > ptsLst_03 (3);
    ptsLst_03[0] = pntVec[3];
    ptsLst_03[1] = pntVec[4];
    ptsLst_03[2] = pntVec[5];

    vector< vec3d > ptsLst_04 (3);
    ptsLst_04[0] = pntVec[5];
    ptsLst_04[1] = pntVec[6];
    ptsLst_04[2] = pntVec[7];

    vector< vec3d > ptsLst_05 (3);
    ptsLst_05[0] = pntVec[7];
    ptsLst_05[1] = pntVec[8];
    ptsLst_05[2] = pntVec[9];

    vector< vec3d > ptsLst_06 (3);
    ptsLst_06[0] = pntVec[9];
    ptsLst_06[1] = pntVec[10];
    ptsLst_06[2] = pntVec[11];

    vector< vec3d > ptsLst_07 (3);
    ptsLst_07[0] = pntVec[11];
    ptsLst_07[1] = pntVec[12];
    ptsLst_07[2] = pntVec[13];

    vector< vec3d > ptsLst_08 (3);
    ptsLst_08[0] = pntVec[13];
    ptsLst_08[1] = pntVec[14];
    ptsLst_08[2] = pntVec[15];

//--Create piecewise curves-----------------------------------------------------
    VspCurve crv01;
    VspCurve crv02;
    VspCurve crv03;
    VspCurve crv04;
    VspCurve crv05;
    VspCurve crv06;
    VspCurve crv07;
    VspCurve crv08;

    crv01.SetQuadraticControlPoints( ptsLst_01, false );
    crv02.SetQuadraticControlPoints( ptsLst_02, false );
    crv03.SetQuadraticControlPoints( ptsLst_03, false );
    crv04.SetQuadraticControlPoints( ptsLst_04, false );
    crv05.SetQuadraticControlPoints( ptsLst_05, false );
    crv06.SetQuadraticControlPoints( ptsLst_06, false );
    crv07.SetQuadraticControlPoints( ptsLst_07, false );
    crv08.SetQuadraticControlPoints( ptsLst_08, false );

//--Create one common curve-----------------------------------------------------
    VspCurve crv;
    crv.Copy(crv01); //........................................................ Copy first curve
    crv.Append( crv02 );
    crv.Append( crv03 );
    crv.Append( crv04 );
    crv.Append( crv05 );
    crv.Append( crv06 );
    crv.Append( crv07 );
    crv.Append( crv08 );

//--Tessellate curve------------------------------------------------------------
    int Num_TSS = m_TessPtsNum()/8; //......................................... Number of tessellated points
    vector< double > ucache ( 8 * Num_TSS ); //................................ List for u parameters on only one edge


    for (int i=0; i < Num_TSS; i++)
    {
        ucache[i             ] = float(i)/(Num_TSS-1)      ;
        ucache[i + 1*Num_TSS ] = float(i)/(Num_TSS-1) + 1.0;
        ucache[i + 2*Num_TSS ] = float(i)/(Num_TSS-1) + 2.0;
        ucache[i + 3*Num_TSS ] = float(i)/(Num_TSS-1) + 3.0;
        ucache[i + 4*Num_TSS ] = float(i)/(Num_TSS-1) + 4.0;
        ucache[i + 5*Num_TSS ] = float(i)/(Num_TSS-1) + 5.0;
        ucache[i + 6*Num_TSS ] = float(i)/(Num_TSS-1) + 6.0;
        ucache[i + 7*Num_TSS ] = float(i)/(Num_TSS-1) + 7.0;
    }

    vector< vec3d > TessPts ( ucache.size() ); //.............................. List for tessellated points
    crv.Tesselate( ucache, TessPts); //........................................ Tessellate curve

//--Create line segments--------------------------------------------------------
    int pind = 0;
    m_LVec.resize(TessPts.size()-1);

    for ( int i = 0 ; i < TessPts.size()-1; i++ )
    {
        m_LVec[i].SetSP0( TessPts[pind] );
        pind++;
        m_LVec[i].SetSP1( TessPts[pind] );
        m_LVec[i].Update( geom );
    }

//--Update surface--------------------------------------------------------------
    SubSurface::Update();

}

//////////////////////////////////////////////////////
//=================== SSPolygon=====================//
//////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Constructor                                                                //
////////////////////////////////////////////////////////////////////////////////
SSPolygon::SSPolygon( string comp_id, int type) : SubSurface( comp_id, type )
{
// *****************************************************************************
//                            VERTEX LIST
// *****************************************************************************
    vrt_ULst.resize(0);
    vrt_WLst.resize(0);

//--Set first vertex------------------------------------------------------------
    Parm *vrtU0 = new Parm();
    Parm *vrtW0 = new Parm();

    vrtU0->Init( "", "SSPolygon", this, -0.05, -2, 2 );
    vrtW0->Init( "", "SSPolygon", this, -0.05, -2, 2 );

    vrt_ULst.push_back( vrtU0 );
    vrt_WLst.push_back( vrtW0 );

//--Set second vertex-----------------------------------------------------------
    Parm *vrtU1 = new Parm();
    Parm *vrtW1 = new Parm();

    vrtU1->Init( "", "SSPolygon", this,  0.05, -2, 2 );
    vrtW1->Init( "", "SSPolygon", this, -0.05, -2, 2 );

    vrt_ULst.push_back( vrtU1 );
    vrt_WLst.push_back( vrtW1 );

//--Set third vertex------------------------------------------------------------
    Parm *vrtU2 = new Parm();
    Parm *vrtW2 = new Parm();

    vrtU2->Init( "", "SSPolygon", this, 0.05, -2, 2 );
    vrtW2->Init( "", "SSPolygon", this, 0.05, -2, 2 );

    vrt_ULst.push_back( vrtU2 );
    vrt_WLst.push_back( vrtW2 );

//--Set fourth vertex-----------------------------------------------------------
    Parm *vrtU3 = new Parm();
    Parm *vrtW3 = new Parm();

    vrtU3->Init( "", "SSPolygon", this, -0.05, -2, 2 );
    vrtW3->Init( "", "SSPolygon", this,  0.05, -2, 2 );

    vrt_ULst.push_back( vrtU3 );
    vrt_WLst.push_back( vrtW3 );

// *****************************************************************************
//                            EDGE LIST
// *****************************************************************************
    edgNrmDeviation.resize(0);
    edgNrmDeviationPosition.resize(0);
    edgNrmDeviationRot.resize(0);

    for(int i=0; i<4; i++){
        Parm *nrmDev_i = new Parm();
        Parm *nrmDevPos_i = new Parm();
        Parm *nrmDevRot_i = new Parm();

        nrmDev_i->Init( "", "SSPolygon", this, 0.0, -1, 1 );
        nrmDevPos_i->Init( "", "SSPolygon", this, 0.5, 0.01, 0.99 );
        nrmDevRot_i->Init( "", "SSPolygon", this, 0.0, -89.9, 89.9 );

        edgNrmDeviation.push_back( nrmDev_i );
        edgNrmDeviationPosition.push_back( nrmDevPos_i );
        edgNrmDeviationRot.push_back( nrmDevRot_i );
    }

    UpdateDescription();
    UpdateVarNames();

// *****************************************************************************
//                            OTHER PARAMETERS
// *****************************************************************************
    m_TessPtsEdge.Init ( "TessPts_edge", "SSPolygon", this, 100, 3, 1000);
    m_TessPtsEdge.SetDescript( "Number of tessellated points on each side" );

    m_TestType.Init( "Test_Type", "SSPolygon", this, vsp::INSIDE, vsp::INSIDE, vsp::NONE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );

    m_CenterU.Init( "CenterU", "SSPolygon", this, 0.5, 0.0, 1.0);
    m_CenterU.SetDescript( " U component of center points" );

    m_CenterW.Init( "CenterW", "SSPolygon", this, 0.5, 0.0, 1.0);
    m_CenterW.SetDescript( " W component of center points" );

    m_ScaleU.Init( "ScaleU", "SSPolygon", this, 1.0, 0.0, 100.0);
    m_ScaleU.SetDescript( " Scale factor in U direction" );

    m_ScaleW.Init( "ScaleW", "SSPolygon", this, 1.0, 0.0, 100.0);
    m_ScaleW.SetDescript( " Scale factor in W direction" );

}

////////////////////////////////////////////////////////////////////////////////
// Destructor                                                                 //
////////////////////////////////////////////////////////////////////////////////
SSPolygon::~SSPolygon()
{
//--Clear all parameters in the lists-------------------------------------------
    for( int i=0; i<Num(); i++){

        delete vrt_ULst[i]; //................................................. U component of the vertex i-th vertex
        delete vrt_WLst[i]; //................................................. W component of the vertex i-th vertex

        delete edgNrmDeviation[i]; //.......................................... Normal deviation of the i-th edge
        delete edgNrmDeviationPosition[i]; //.................................. Position of the normal deviation of the i-th edge
        delete edgNrmDeviationRot[i]; //....................................... Rotation of the normal deviation of the i-th edge
    }
}

////////////////////////////////////////////////////////////////////////////////
// Insert a new vertex after given index placing it in the middle of the old  //
// edge                                                                       //
////////////////////////////////////////////////////////////////////////////////
bool SSPolygon::InsertAfter(int edg_idx)
{
    if(edg_idx < 0 || edg_idx > Num()-1)
        return false;

//--- New vertex is inserted on the last edge
    if ( edg_idx == Num()-1 )
    {
        Parm *u_last = vrt_ULst[edg_idx];
        Parm *w_last = vrt_WLst[edg_idx];

        Parm *u_first = vrt_ULst[0];
        Parm *w_first = vrt_WLst[0];

        vec3d lastPt, firstPt, newPt;
        lastPt  = vec3d( u_last->Get(), w_last->Get(), 0.0 );
        firstPt = vec3d( u_first->Get(), w_first->Get(), 0.0 );

        newPt = 0.5 * (lastPt + firstPt);

    //--Create new parameter and append to list-----------------------------
        Parm *newPt_u = new Parm();
        Parm *newPt_w = new Parm();
        newPt_u->Init( "", "SSPolygon", this, newPt.x(), -2, 2 );
        newPt_w->Init( "", "SSPolygon", this, newPt.y(), -2, 2 );

        vrt_ULst.push_back( newPt_u );
        vrt_WLst.push_back( newPt_w );

        Parm *newNrmDev    = new Parm();
        Parm *newNrmDevPos = new Parm();
        Parm *newNrmDevRot = new Parm();

        newNrmDev->Init( "", "SSPolygon", this, 0.0, -1, 1 );
        newNrmDevPos->Init( "", "SSPolygon", this, 0.5, 0.01, 0.99 );
        newNrmDevRot->Init( "", "SSPolygon", this, 0.0, -89.9, 89.9 );

        edgNrmDeviation.push_back( newNrmDev );
        edgNrmDeviationPosition.push_back( newNrmDevPos );
        edgNrmDeviationRot.push_back( newNrmDevRot );
    }
    else
    {
    //-- Get vertex parameters -------------------------------------------------
        Parm *u_01, *w_01, *u_02, *w_02;

        u_01 = vrt_ULst[edg_idx];
        w_01 = vrt_WLst[edg_idx];
        u_02 = vrt_ULst[edg_idx+1];
        w_02 = vrt_WLst[edg_idx+1];

        vec3d pt01, pt02, newPt; //............................................ Create 3d vector
        pt01 = vec3d( u_01->Get(), w_01->Get(), 0.0 );
        pt02 = vec3d( u_02->Get(), w_02->Get(), 0.0 );
        newPt = 0.5 * (pt01 + pt02); //........................................ Compute new point

    //--Add the new vertex------------------------------------------------------
        Parm *newPt_u = new Parm();
        Parm *newPt_w = new Parm();

        newPt_u->Init( "", "SSPolygon", this, newPt.x(), -2, 2 );
        newPt_w->Init( "", "SSPolygon", this, newPt.y(), -2, 2 );

        vrt_ULst.insert( vrt_ULst.begin()+edg_idx+1, newPt_u );
        vrt_WLst.insert( vrt_WLst.begin()+edg_idx+1, newPt_w );

    //--Add default edge parameters---------------------------------------------
        Parm *newNrmDev = new Parm();
        Parm *newNrmDevPos = new Parm();
        Parm *newNrmDevRot = new Parm();

        newNrmDev->Init( "", "SSPolygon", this, 0.0, -1, 1 );
        newNrmDevPos->Init( "", "SSPolygon", this, 0.5, 0.01, 0.99 );
        newNrmDevRot->Init( "", "SSPolygon", this, 0.0, -89.9, 89.9 );

        edgNrmDeviation.insert(edgNrmDeviation.begin()+edg_idx+1, newNrmDev );
        edgNrmDeviationPosition.insert(edgNrmDeviationPosition.begin()+edg_idx+1, newNrmDevPos );
        edgNrmDeviationRot.insert(edgNrmDeviationRot.begin()+edg_idx+1, newNrmDevRot );
    }

    UpdateDescription();
    UpdateVarNames();

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Delete vertex at a given index                                             //
////////////////////////////////////////////////////////////////////////////////
bool SSPolygon::RemoveVrtx(int vrtx_idx){
//--Check index-----------------------------------------------------------------
    if(vrtx_idx < 0 || vrtx_idx > Num()-1)
        return false;
//--If there are only three vertice, abort--------------------------------------
    if( Num() <= 3 )
        return false;

    int oldNum = Num();

    Parm *vrtU_i = vrt_ULst[ vrtx_idx ];
    Parm *vrtW_i = vrt_WLst[ vrtx_idx ];

    delete vrtU_i;
    delete vrtW_i;

    vrt_ULst.erase (vrt_ULst.begin()+vrtx_idx);
    vrt_WLst.erase (vrt_WLst.begin()+vrtx_idx);

    Parm *devMag_i = edgNrmDeviation[vrtx_idx];
    Parm *devPos_i = edgNrmDeviationPosition[vrtx_idx];
    Parm *devRot_i = edgNrmDeviationRot[vrtx_idx];

    delete devMag_i;
    delete devPos_i;
    delete devRot_i;

    edgNrmDeviation.erase (edgNrmDeviation.begin()+vrtx_idx);
    edgNrmDeviationPosition.erase (edgNrmDeviationPosition.begin()+vrtx_idx);
    edgNrmDeviationRot.erase (edgNrmDeviationRot.begin()+vrtx_idx);

    UpdateDescription();
    UpdateVarNames();

    if ( Num() != (oldNum-1) )
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Update the description of the parameters                                   //
////////////////////////////////////////////////////////////////////////////////
void SSPolygon::UpdateDescription(){
//--Update description in vrt_ULst----------------------------------------------
    for(int i=0; i<Num(); i++)
        vrt_ULst[i]->SetDescript( "Defines u component of vertex at index" + to_string(i+1) );
//--Update description in vrt_WLst----------------------------------------------
    for(int i=0; i<Num(); i++)
        vrt_WLst[i]->SetDescript( "Defines u component of vertex at index" + to_string(i+1) );
//--Update description in edgNrmDeviation---------------------------------------
    for(int i=0; i<Num(); i++)
        edgNrmDeviation[i]->SetDescript( "Normal deviation on the " + to_string(i+1) + "-th edge" );
//--Update description in edgNrmDeviationPosition-------------------------------
    for(int i=0; i<Num(); i++)
        edgNrmDeviationPosition[i]->SetDescript( "Position of the deviation on the " + to_string(i+1) + "-th edge" );
//--Update description in edgNrmDeviationRot-------------------------------
    for(int i=0; i<Num(); i++)
        edgNrmDeviationRot[i]->SetDescript( "Rotation of the deviation on the " + to_string(i+1) + "-th edge" );
}

////////////////////////////////////////////////////////////////////////////////
// Update variable names of the parameters                                    //
////////////////////////////////////////////////////////////////////////////////
void SSPolygon::UpdateVarNames(){
//--Update description in vrt_ULst----------------------------------------------
    for(int i=0; i<Num(); i++)
        vrt_ULst[i]->SetName( "Vrt_U" + to_string(i+1) );
//--Update description in vrt_WLst----------------------------------------------
    for(int i=0; i<Num(); i++)
        vrt_WLst[i]->SetName( "Vrt_W" + to_string(i+1) );
//--Update description in edgNrmDeviation---------------------------------------
    for(int i=0; i<Num(); i++)
        edgNrmDeviation[i]->SetName( "NrmDev_" + to_string(i+1) );
//--Update description in edgNrmDeviationPosition-------------------------------
    for(int i=0; i<Num(); i++)
        edgNrmDeviationPosition[i]->SetName( "NrmDevPos_" + to_string(i+1) );
//--Update description in edgNrmDeviationRot-------------------------------
    for(int i=0; i<Num(); i++)
        edgNrmDeviationRot[i]->SetName( "NrmDevRot_" + to_string(i+1) );
}

////////////////////////////////////////////////////////////////////////////////
// Update the drawing of the line segments                                    //
////////////////////////////////////////////////////////////////////////////////

void SSPolygon::Update()
{
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom )
    {
        return;
    }

    //--DESCRIPTION-----------------------------------------------------------//
    //  Every edge is created by using a second degree Bezier segment with    //
    //  three control points. First and third control point is defining the   //
    //  start and end of the curve and the middle the user defined bending:   //
    //                                                                        //
    //  +        + : Start and end point of the curve                         //
    //  | n      ◌ : Position of the normal vector                            //
    //  ◌->                                                                   //
    //  |                                                                     //
    //  |                                                                     //
    //  +                                                                     //
    //------------------------------------------------------------------------//

    vector< VspCurve > edgeCurves;

    for(int edgIdx=0; edgIdx<Num(); edgIdx++){
    //--Get start and end point of the curve------------------------------------
        Parm *start_u, *start_w;
        Parm *end_u, *end_w;

        start_u = vrt_ULst[edgIdx];
        start_w = vrt_WLst[edgIdx];

        if (edgIdx == (Num()-1) )
        {
            end_u = vrt_ULst[0];
            end_w = vrt_WLst[0];
        }
        else
        {
            end_u = vrt_ULst[edgIdx+1];
            end_w = vrt_WLst[edgIdx+1];
        }

        vec3d start_pt, end_pt;
    //--Compute start point of the edge-----------------------------------------
        start_pt = vec3d(start_u->Get()*m_ScaleU() + m_CenterU(),
                         start_w->Get()*m_ScaleW() + m_CenterW(), 0.0 );
    //--Compute end point of the edge-------------------------------------------
        end_pt   = vec3d(end_u->Get()*m_ScaleU() + m_CenterU(),
                         end_w->Get()*m_ScaleW() + m_CenterW(), 0.0 );

    //--Estimate normal vector--------------------------------------------------
        vec3d nrmVec;
        nrmVec = vec3d( (end_pt-start_pt).y(), -(end_pt-start_pt).x(), 0.0 );
        nrmVec.normalize(); //................................................. Normalize lenght

    //--Rotate normal vector----------------------------------------------------
        double rot_ang = edgNrmDeviationRot[edgIdx]->Get() * PI/180.0; //...... Rotation angle in rad
        nrmVec.rotate_z( cos(rot_ang), sin(rot_ang) ); //...................... Rotate

    //--Estimate position of the normal vector on unbended edge-----------------
        double nVecPos = edgNrmDeviationPosition[edgIdx]->Get();
        vec3d posNrmVecOnEdg;
        posNrmVecOnEdg = start_pt + nVecPos * (end_pt - start_pt);

    //--Estimate middle control point-------------------------------------------
        double normDevLen = edgNrmDeviation[edgIdx]->Get();
        vec3d midCPt;
        midCPt = posNrmVecOnEdg + normDevLen * nrmVec;

    //--Create bezier segment---------------------------------------------------
        vector< vec3d > crvCPts;
        crvCPts.push_back( start_pt );
        crvCPts.push_back( midCPt );
        crvCPts.push_back( end_pt );

        VspCurve newCrv;
        newCrv.SetQuadraticControlPoints( crvCPts, false ); //................. Create new curve segment
        edgeCurves.push_back( newCrv ); //..................................... and append to list
    }

//--TESSELLATION----------------------------------------------------------------
    int Num_TSS = m_TessPtsEdge();
    vector< vec3d > tessPts;

//--Prepare u parameter list----------------------------------------------------
    vector< double > ucache (Num_TSS);
    for( int ui=0; ui<Num_TSS; ui++)
    {
        ucache[ui] = float(ui)/(Num_TSS-1);
    }

//--Run over created curves and tesselate---------------------------------------
    for( int i=0; i<edgeCurves.size(); i++ )
    {
        VspCurve crv;
        vector< vec3d > tessPts_crv (Num_TSS); //.............................. list for tessellated points

        crv = edgeCurves[i]; //................................................ Pick curves
        crv.Tesselate( ucache, tessPts_crv ); //............................... Tesselate curve

    //--Append list of tessellated curve points to main list--------------------
        tessPts.insert(tessPts.end(), tessPts_crv.begin(), tessPts_crv.end());
    }

//--Create line segments--------------------------------------------------------
    int pind = 0;
    m_LVec.resize(tessPts.size()-1);

    for ( int i = 0 ; i < tessPts.size()-1; i++ )
    {
        m_LVec[i].SetSP0( tessPts[pind] );
        pind++;
        m_LVec[i].SetSP1( tessPts[pind] );
        m_LVec[i].Update( geom );
    }

//--Update surface--------------------------------------------------------------
    SubSurface::Update();

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

    m_TestType.Init( "Test_Type", "SS_Rectangle", this, vsp::INSIDE, vsp::INSIDE, vsp::NONE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );

    m_URadius.Init( "U_Radius", "SS_Rectangle", this, .0, 0, 1.0 );
    m_URadius.SetDescript( "Relative radius of the rectangle corners in U direction" );

    m_WRadius.Init( "W_Radius", "SS_Rectangle", this, .0, 0, 1.0 );
    m_WRadius.SetDescript( "Relative radius of the rectangle corners in W direction" );

    m_NumArcPts.Init( "Num_Radius", "SS_Rectangle", this, 5, 0, 1000 );
    m_NumArcPts.SetDescript( "Amount of arc points for tessellation of rounded rectangle corners" );

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

    // default condition is simple polygon.
    double radtol = 1e-2;
    m_PolyFlag = true;
    if ( m_URadius() > radtol && m_WRadius() > radtol )
    {
        m_PolyFlag = false;
    }

    vec3d center;
    vector< vec3d > pntVec;
    vector< vec3d > arcVec;

    int NArcPtsInt = m_NumArcPts();

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

//--Create arc points in clockwise direction of right upper corner--------------
    arcVec.resize( NArcPtsInt );
    float dphi = 0.5 * M_PI / ( NArcPtsInt + 1 );
    for( int i = 0; i < NArcPtsInt; i++ )
    {
        float phase_i = dphi * ( i + 1 );
        arcVec[i] = vec3d( sin( phase_i ) * m_URadius(), cos( phase_i ) * m_WRadius(), 0.0 )
                    + vec3d( 1.0, 1.0, 0.0 )
                    - vec3d( m_URadius(), m_WRadius(), 0.0 );
    }

//--Create points of the overall shape------------------------------------------
    int pts_idx = 0;
    pntVec.resize( 9 + 4 * NArcPtsInt );

//--Bottom line:
    pntVec[pts_idx] = vec3d( -1.0, -1.0, 0.0 ) + vec3d( m_URadius(), 0.0, 0.0 );
    pts_idx++;
    pntVec[pts_idx] = vec3d( 1.0, -1.0, 0.0 ) + vec3d( -m_URadius(), 0.0, 0.0 );
    pts_idx++;

//--Bottom right arc:
    for( int i = 0; i < NArcPtsInt; i++ )
    {
        pntVec[pts_idx] = vec3d( 0.0, 0.0, 0.0 );
        pntVec[pts_idx].set_x( arcVec[i].x() );
        pntVec[pts_idx].set_y( -arcVec[i].y() );
        pts_idx++;
    }

//--Right line:
    pntVec[pts_idx] = vec3d( 1.0, -1.0, 0.0 ) + vec3d( 0.0, m_WRadius(), 0.0 );
    pts_idx++;
    pntVec[pts_idx] = vec3d( 1.0,  1.0, 0.0 ) + vec3d( 0.0, -m_WRadius(), 0.0 );
    pts_idx++;

//--Upper right arc:
    for( int i = 0; i < NArcPtsInt; i++ )
    {
        pntVec[pts_idx] = vec3d( 0.0, 0.0, 0.0 );
        pntVec[pts_idx].set_x( arcVec[NArcPtsInt - i - 1].x() );
        pntVec[pts_idx].set_y( arcVec[NArcPtsInt - i - 1].y() );
        pts_idx++;
    }

//--Upper line:
    pntVec[pts_idx] = vec3d( 1.0,  1.0, 0.0 ) + vec3d( -m_URadius(), 0.0, 0.0 );
    pts_idx++;
    pntVec[pts_idx] = vec3d( -1.0,  1.0, 0.0 ) + vec3d( m_URadius(), 0.0, 0.0 );
    pts_idx++;

//--Upper left arc:
    for( int i = 0; i < NArcPtsInt; i++ )
    {
        pntVec[pts_idx] = vec3d( 0.0, 0.0, 0.0 );
        pntVec[pts_idx].set_x( -arcVec[i].x() );
        pntVec[pts_idx].set_y( arcVec[i].y() );
        pts_idx++;
    }

//--Left line:
    pntVec[pts_idx] = vec3d( -1.0,  1.0, 0.0 ) + vec3d( 0.0, -m_WRadius(), 0.0 );
    pts_idx++;
    pntVec[pts_idx] = vec3d( -1.0, -1.0, 0.0 ) + vec3d( 0.0, m_WRadius(), 0.0 );
    pts_idx++;

//--Bottom left arc:
    for( int i = 0; i < NArcPtsInt; i++ )
    {
        pntVec[pts_idx] = vec3d( 0.0, 0.0, 0.0 );
        pntVec[pts_idx].set_x( -arcVec[NArcPtsInt - i - 1].x() );
        pntVec[pts_idx].set_y( -arcVec[NArcPtsInt - i - 1].y() );
        pts_idx++;
    }

//--Last point to close shape:
    pntVec[pts_idx] = pntVec[0]; pts_idx++;

//--Stretch rectangle by height and weight--------------------------------------
    for ( int i = 0; i < pts_idx; i++ )
    {
        pntVec[i].scale_x( m_ULength() / 2 );
        pntVec[i].scale_y( m_WLength() / 2 );
    }

//--Align rectangle to centre---------------------------------------------------
    for ( int i = 0; i < pts_idx; i++ )
    {
        pntVec[i].set_x( pntVec[i].x() + center.x() );
        pntVec[i].set_y( pntVec[i].y() + center.y() );
    }

//--Apply transformations-------------------------------------------------------
    for ( int i = 0; i < pts_idx; i++ )
    {
        pntVec[i] = transMat2.xform( rotMat.xform( transMat1.xform( pntVec[i] ) ) );
    }

//--Create line segments--------------------------------------------------------
    int pind = 0;
    m_LVec.resize( pts_idx - 1 );

    for ( int i = 0 ; i < pts_idx - 1; i++ )
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
    m_TestType.Init( "Test_Type", "SS_Ellipse", this, vsp::INSIDE, vsp::INSIDE, vsp::NONE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );

    m_PolyFlag = false;
}

//===== Destructor =====//
SSEllipse::~SSEllipse()
{

}

// Main Update Routine
void SSEllipse::Update()
{
    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom )
    {
        return;
    }

    int num_pnts = m_Tess();
    m_LVec.resize( num_pnts );

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

//////////////////////////////////////////////////////
//=================== SSControlSurface =====================//
//////////////////////////////////////////////////////

SSControlSurf::SSControlSurf( string compID, int type ) : SubSurface( compID, type )
{
    m_Tess.Init( "Tess_Num", "SS_Control", this, 15, 1, 1000 );
    m_Tess.SetDescript( " Number of points to discretize edges" );

    m_StartLenFrac.Init( "Length_C_Start", "SS_Control", this, 0.25, 0, 1 );
    m_StartLenFrac.SetDescript( "Specifies control surface width as fraction of chord" );

    m_EndLenFrac.Init( "Length_C_End", "SS_Control", this, 0.25, 0, 1 );
    m_EndLenFrac.SetDescript( "Specifies control surface width as fraction of chord" );

    m_StartLength.Init( "Length_Start", "SS_Control", this, 1.0, 0, 1e12 );
    m_StartLength.SetDescript( "Control surface width." );

    m_EndLength.Init( "Length_End", "SS_Control", this, 1.0, 0, 1e12 );
    m_EndLength.SetDescript( "Control surface width." );

    m_AbsRelFlag.Init( "Abs_Rel_Flag", "SS_Control", this, vsp::REL, vsp::ABS, vsp::REL );
    m_AbsRelFlag.SetDescript( "Specify control surface with absolute or relative parameter." );

    m_UStart.Init( "UStart", "SS_Control", this, 0.4, 0, 1 );
    m_UStart.SetDescript( "The U starting location of the control surface" );

    m_UEnd.Init( "UEnd", "SS_Control", this, 0.6, 0, 1 );
    m_UEnd.SetDescript( "The U ending location of the control surface" );

    m_TestType.Init( "Test_Type", "SS_Control", this, vsp::INSIDE, vsp::INSIDE, vsp::NONE );
    m_TestType.SetDescript( "Determines whether or not the inside or outside of the region is tagged" );

    m_SurfType.Init( "Surf_Type", "SS_Control", this, BOTH_SURF, UPPER_SURF, BOTH_SURF );
    m_SurfType.SetDescript( "Flag to determine whether the control surface is on the upper,lower, or both surface(s) of the wing" );

    m_ConstFlag.Init( "SE_Const_Flag", "SS_Control", this, true, 0, 1 );
    m_ConstFlag.SetDescript( "Control surface start/end parameters equal." );

    m_LEFlag.Init( "LE_Flag", "SS_Control", this, false, 0, 1 );
    m_LEFlag.SetDescript( "Flag to determine whether control surface is on the leading/trailing edge." );

    m_StartAngle.Init( "StartAngle", "SS_Control", this, 90, 0, 180 );
    m_StartAngle.SetDescript( "Angle that control surface start meets leading/trailing edge." );

    m_EndAngle.Init( "EndAngle", "SS_Control", this, 90, 0, 180 );
    m_EndAngle.SetDescript( "Angle that control surface end meets leading/trailing edge." );

    m_StartAngleFlag.Init( "StartAngleFlag", "SS_Control", this, false, 0, 1 );
    m_StartAngleFlag.SetDescript( "Flag to determine whether to set control surface start angle." );

    m_EndAngleFlag.Init( "EndAngleFlag", "SS_Control", this, false, 0, 1 );
    m_EndAngleFlag.SetDescript( "Flag to determine whether to set control surface end angle." );

    m_SameAngleFlag.Init( "SameAngleFlag", "SS_Control", this, true, 0, 1 );
    m_SameAngleFlag.SetDescript( "Flag to set control surface start/end angles equal." );

    for ( int i = 0; i < 3; i++ )
    {
        m_LVec.push_back( SSLineSeg() );
    }

    m_PolyFlag = false;
}

//==== Destructor ====//
SSControlSurf::~SSControlSurf()
{

}

//==== Update Method ===//
void SSControlSurf::Update()
{
    // Build Control Surface as a rectangle with the points counter clockwise

    vec3d c_uws_upper, c_uws_lower, c_uwe_upper, c_uwe_lower;

    vec3d c_uwm_upper, c_uwm_lower;

    vec3d c_uws1_upper, c_uws1_lower, c_uwe1_upper, c_uwe1_lower;
    vec3d c_uw1_upper, c_uw1_lower;
    vec3d c_uws2_upper, c_uws2_lower, c_uwe2_upper, c_uwe2_lower;
    vec3d c_uw2_upper, c_uw2_lower;

    Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );
    if ( !geom ) { return; }

    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf ) { return; }

    m_UWStart01.clear();
    m_UWEnd01.clear();

    VspCurve startcrv;
    surf->GetU01ConstCurve( startcrv, m_UStart() );

    piecewise_curve_type c = startcrv.GetCurve();

    double vmin = c.get_parameter_min(); // Really must be 0.0
    double vmax = c.get_parameter_max(); // Really should be 4.0

    double umax = surf->GetUMax();
    double umin = 0.0;
    double ucs = m_UStart() * umax;

    double vle = ( vmin + vmax ) * 0.5;

    double vtelow = vmin + TMAGIC;
    double vteup = vmax - TMAGIC;
    double vlelow = vle - TMAGIC;
    double vleup = vle + TMAGIC;

    curve_point_type te, le;
    te = c.f( vmin );
    le = c.f( vle );

    double chord, d;
    chord = dist( le, te );

    if ( m_AbsRelFlag() == vsp::REL )
    {
        d = chord * m_StartLenFrac();
        m_StartLength.Set( d );
    }
    else
    {
        d = m_StartLength();
        m_StartLenFrac.Set( d / chord );
    }

    if ( m_ConstFlag.Get() )
    {
        if ( m_AbsRelFlag() == vsp::REL )
        {
            m_EndLenFrac.Set( d / chord );
        }
        else
        {
            m_EndLength.Set( d );
        }
    }

    if ( m_SameAngleFlag() && m_EndAngleFlag() && m_StartAngleFlag() )
    {
        m_EndAngle = m_StartAngle();
    }

    curve_point_type telow, teup;
    telow = c.f( vtelow );
    teup = c.f( vteup );

    curve_point_type lelow, leup;
    lelow = c.f( vlelow );
    leup = c.f( vleup );

    double u, v;

    if ( m_SurfType() != LOWER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( m_StartAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vteup );
                vec3d vdir;
                vdir = ( le - te ) / 2.0; // reverse direction
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_StartAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, teup, udir, d, m_StartAngle() * PI / 180.0, ucs + du, vteup - dv ); // reverse v
                c_uws_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, teup, udir, d / 3.0, m_StartAngle() * PI / 180.0, ucs + du / 3.0, vteup - dv / 3.0 ); // reverse v
                c_uws1_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, teup, udir, 2.0 * d / 3.0, m_StartAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vteup - 2.0 * dv / 3.0 ); // reverse v
                c_uws2_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, teup, d );
                c_uws_upper = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, teup, d / 3.0 );
                c_uws1_upper = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, teup, 2.0 * d / 3.0 );
                c_uws2_upper = vec3d( m_UStart(), v / vmax, 0 );
            }
        }
        else
        {
            if ( m_StartAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vleup );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_StartAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, leup, udir, d, m_StartAngle() * PI / 180.0, ucs + du, vleup + dv );
                c_uws_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, leup, udir, d / 3.0, m_StartAngle() * PI / 180.0, ucs + du / 3.0, vleup + dv / 3.0 );
                c_uws1_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, leup, udir, 2.0 * d / 3.0, m_StartAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vleup + 2.0 * dv / 3.0 );
                c_uws2_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, leup, d );
                c_uws_upper = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, leup, d / 3.0 );
                c_uws1_upper = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, leup, 2.0 * d / 3.0 );
                c_uws2_upper = vec3d( m_UStart(), v / vmax, 0 );
            }
        }
        m_UWStart01.push_back( c_uws_upper );
    }

    if ( m_SurfType() != UPPER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( m_StartAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vtelow );
                vec3d vdir;
                vdir = ( le - te ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_StartAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, telow, udir, d, m_StartAngle() * PI / 180.0, ucs + du, vtelow + dv );
                c_uws_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, telow, udir, d / 3.0, m_StartAngle() * PI / 180.0, ucs + du / 3.0, vtelow + dv / 3.0 );
                c_uws1_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, telow, udir, 2.0 * d / 3.0, m_StartAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vtelow + 2.0 * dv / 3.0 );
                c_uws2_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, telow, d );
                c_uws_lower = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, telow, d / 3.0 );
                c_uws1_lower = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, telow, 2.0 * d / 3.0);
                c_uws2_lower = vec3d( m_UStart(), v / vmax, 0 );
            }
        }
        else
        {
            if ( m_StartAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vlelow );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_StartAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, lelow, udir, d, m_StartAngle() * PI / 180.0, ucs + du, vlelow - dv );
                c_uws_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, lelow, udir, d / 3.0, m_StartAngle() * PI / 180.0, ucs + du / 3.0, vlelow - dv / 3.0 );
                c_uws1_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, lelow, udir, 2.0 * d / 3.0, m_StartAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vlelow - 2.0 * dv / 3.0 );
                c_uws2_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, lelow, d );
                c_uws_lower = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, lelow, d / 3.0 );
                c_uws1_lower = vec3d( m_UStart(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, lelow, 2.0 * d / 3.0 );
                c_uws2_lower = vec3d( m_UStart(), v / vmax, 0 );
            }
        }
        m_UWStart01.push_back( c_uws_lower );
    }

    if ( !m_StartAngleFlag() )
    {
        if ( m_LEFlag() )
        {
            vec3d udir = surf->CompTanU( ucs, vleup );
            udir.normalize();
            vec3d vdir;
            vdir = ( te - le ) / 2.0;
            vdir.normalize();
            m_StartAngle = acos( dot( udir, vdir ) ) * 180.0 / PI;
        }
        else
        {
            vec3d udir = surf->CompTanU( ucs, vtelow );
            udir.normalize();
            vec3d vdir;
            vdir = ( le - te ) / 2.0;
            vdir.normalize();
            m_StartAngle = acos( dot( udir, vdir ) ) * 180.0 / PI;
        }
    }

    VspCurve endcrv;
    surf->GetU01ConstCurve( endcrv, m_UEnd() );
    c = endcrv.GetCurve();

    te = c.f( vmin );
    le = c.f( vle );

    chord = dist( le, te );


    if ( m_AbsRelFlag() == vsp::REL )
    {
        d = chord * m_EndLenFrac();
        m_EndLength.Set( d );
    }
    else
    {
        d = m_EndLength();
        m_EndLenFrac.Set( d / chord );
    }

    telow = c.f( vtelow );
    teup = c.f( vteup );
    lelow = c.f( vlelow );
    leup = c.f( vleup );

    ucs = m_UEnd() * umax;

    if ( m_SurfType() != LOWER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( m_EndAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vteup );
                vec3d vdir;
                vdir = ( le - te ) / 2.0; // reverse direction
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_EndAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, teup, udir, d, m_EndAngle() * PI / 180.0, ucs + du, vteup - dv ); // reverse v
                c_uwe_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, teup, udir, d / 3.0, m_EndAngle() * PI / 180.0, ucs + du / 3.0, vteup - dv / 3.0 ); // reverse v
                c_uwe1_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, teup, udir, 2.0 * d / 3.0, m_EndAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vteup - 2.0 * dv / 3.0 ); // reverse v
                c_uwe2_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, teup, d );
                c_uwe_upper = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, teup, d / 3.0 );
                c_uwe1_upper = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, teup, 2.0 * d / 3.0 );
                c_uwe2_upper = vec3d( m_UEnd(), v / vmax, 0 );
            }
        }
        else
        {
            if ( m_EndAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vleup );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_EndAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, leup, udir, d, m_EndAngle() * PI / 180.0, ucs + du, vleup + dv );
                c_uwe_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, leup, udir, d / 3.0, m_EndAngle() * PI / 180.0, ucs + du / 3.0, vleup + dv / 3.0 );
                c_uwe1_upper = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, leup, udir, 2.0 * d / 3.0, m_EndAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vleup + 2.0 * dv / 3.0 );
                c_uwe2_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, leup, d );
                c_uwe_upper = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, leup, d / 3.0 );
                c_uwe1_upper = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, cup, leup, 2.0 * d / 3.0 );
                c_uwe2_upper = vec3d( m_UEnd(), v / vmax, 0 );
            }
        }
        m_UWEnd01.push_back( c_uwe_upper );
    }

    if ( m_SurfType() != UPPER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( m_EndAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vtelow );
                vec3d vdir;
                vdir = ( le - te ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_EndAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, telow, udir, d, m_EndAngle() * PI / 180.0, ucs + du, vtelow + dv );
                c_uwe_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, telow, udir, d / 3.0, m_EndAngle() * PI / 180.0, ucs + du / 3.0, vtelow + dv / 3.0 );
                c_uwe1_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, telow, udir, 2.0 * d / 3.0, m_EndAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vtelow + 2.0 * dv / 3.0 );
                c_uwe2_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, telow, d );
                c_uwe_lower = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, telow, d / 3.0 );
                c_uwe1_lower = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, telow, 2.0 * d / 3.0 );
                c_uwe2_lower = vec3d( m_UEnd(), v / vmax, 0 );
            }
        }
        else
        {
            if ( m_EndAngleFlag() )
            {
                vec3d udir = surf->CompTanU( ucs, vlelow );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, m_EndAngle() * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, lelow, udir, d, m_EndAngle() * PI / 180.0, ucs + du, vlelow - dv );
                c_uwe_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, lelow, udir, d / 3.0, m_EndAngle() * PI / 180.0, ucs + du / 3.0, vlelow - dv / 3.0 );
                c_uwe1_lower = vec3d( u / umax, v / vmax, 0 );

                surf->FindDistanceAngle( u, v, lelow, udir, 2.0 * d / 3.0, m_EndAngle() * PI / 180.0, ucs + 2.0 * du / 3.0, vlelow - 2.0 * dv / 3.0 );
                c_uwe2_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, lelow, d );
                c_uwe_lower = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, lelow, d / 3.0 );
                c_uwe1_lower = vec3d( m_UEnd(), v / vmax, 0 );

                eli::geom::intersect::specified_distance( v, clow, lelow, 2.0 * d / 3.0 );
                c_uwe2_lower = vec3d( m_UEnd(), v / vmax, 0 );
            }
        }
        m_UWEnd01.push_back( c_uwe_lower );
    }

    if ( !m_EndAngleFlag() )
    {
        if ( m_LEFlag() )
        {
            vec3d udir = surf->CompTanU( ucs, vleup );
            udir.normalize();
            vec3d vdir;
            vdir = ( te - le ) / 2.0;
            vdir.normalize();
            m_EndAngle = acos( dot( udir, vdir ) ) * 180.0 / PI;
        }
        else
        {
            vec3d udir = surf->CompTanU( ucs, vtelow );
            udir.normalize();
            vec3d vdir;
            vdir = ( le - te ) / 2.0;
            vdir.normalize();
            m_EndAngle = acos( dot( udir, vdir ) ) * 180.0 / PI;
        }
    }

    // Terrible hack to place middle points.
    c_uw1_upper = c_uws_upper + ( c_uwe_upper - c_uws_upper ) / 3.0;
    c_uw1_lower = c_uws_lower + ( c_uwe_lower - c_uws_lower ) / 3.0;

    c_uw2_upper = c_uws_upper + 2.0 * ( c_uwe_upper - c_uws_upper ) / 3.0;
    c_uw2_lower = c_uws_lower + 2.0 * ( c_uwe_lower - c_uws_lower ) / 3.0;

    // Now do a better job as needed.

    bool midangleflag = true;
    if ( !m_StartAngleFlag() && !m_EndAngleFlag() )
    {
        midangleflag = false;
    }

    double midangle;
    double umid;

    double angle1 = m_StartAngle() + ( m_EndAngle() - m_StartAngle() ) / 3.0;
    double u1 = m_UStart() + ( m_UEnd() - m_UStart() ) / 3.0;

    double angle2 = m_StartAngle() + 2.0 * ( m_EndAngle() - m_StartAngle() ) / 3.0;
    double u2 = m_UStart() + 2.0 * ( m_UEnd() - m_UStart() ) / 3.0;

    VspCurve crv1;
    surf->GetU01ConstCurve( crv1, u1 );
    VspCurve crv2;
    surf->GetU01ConstCurve( crv2, u2 );

    double d1;
    double d2;

    if ( midangleflag )
    {
        double ts = m_StartLength() * sin( m_StartAngle() * PI / 180.0 );
        double te = m_EndLength() * sin( m_EndAngle() * PI / 180.0 );
        d = 0.5 * ( ts + te ) / sin( midangle * PI / 180.0 );

        if ( angle1 < 1e-6 )
        {
            d1 = 0.0;
        }
        else
        {
            d1 = ( ts + ( te - ts ) / 3.0   ) / sin( angle1 * PI / 180.0 );
        }

        if ( angle2 < 1e-6 )
        {
            d2 = 0.0;
        }
        else
        {
            d2 = ( ts + 2.0 * ( te - ts ) / 3.0 ) / sin( angle2 * PI / 180.0 );
        }
    }
    else
    {
        d = 0.5 * ( m_StartLength() + m_EndLength() );
        d1 = m_StartLength() + ( m_EndLength() - m_StartLength() ) / 3.0;
        d2 = m_StartLength() + 2.0 * ( m_EndLength() - m_StartLength() ) / 3.0;
    }

    c = crv1.GetCurve();
    umid = u1;
    d = d1;
    midangle = angle1;

    te = c.f( vmin );
    le = c.f( vle );

    telow = c.f( vtelow );
    teup = c.f( vteup );
    lelow = c.f( vlelow );
    leup = c.f( vleup );

    ucs = umid * umax;

    if ( m_SurfType() != LOWER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vteup );
                vec3d vdir;
                vdir = ( le - te ) / 2.0; // reverse direction
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, teup, udir, d, midangle * PI / 180.0, ucs + du, vteup - dv ); // reverse v
                c_uwm_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, teup, d );
                c_uwm_upper = vec3d( umid, v / vmax, 0 );
            }
        }
        else
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vleup );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, leup, udir, d, midangle * PI / 180.0, ucs + du, vleup + dv );
                c_uwm_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, leup, d );
                c_uwm_upper = vec3d( umid, v / vmax, 0 );
            }
        }
    }

    if ( m_SurfType() != UPPER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vtelow );
                vec3d vdir;
                vdir = ( le - te ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, telow, udir, d, midangle * PI / 180.0, ucs + du, vtelow + dv );
                c_uwm_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, telow, d );
                c_uwm_lower = vec3d( umid, v / vmax, 0 );
            }
        }
        else
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vlelow );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, lelow, udir, d, midangle * PI / 180.0, ucs + du, vlelow - dv );
                c_uwm_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, lelow, d );
                c_uwm_lower = vec3d( umid, v / vmax, 0 );
            }
        }
    }

    c_uw1_upper = c_uwm_upper;
    c_uw1_lower = c_uwm_lower;

    //////////////////////////// Repeat Block

    c = crv2.GetCurve();
    umid = u2;
    d = d2;
    midangle = angle2;

    te = c.f( vmin );
    le = c.f( vle );

    telow = c.f( vtelow );
    teup = c.f( vteup );
    lelow = c.f( vlelow );
    leup = c.f( vleup );

    ucs = umid * umax;

    if ( m_SurfType() != LOWER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vteup );
                vec3d vdir;
                vdir = ( le - te ) / 2.0; // reverse direction
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, teup, udir, d, midangle * PI / 180.0, ucs + du, vteup - dv ); // reverse v
                c_uwm_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, teup, d );
                c_uwm_upper = vec3d( umid, v / vmax, 0 );
            }
        }
        else
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vleup );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, leup, udir, d, midangle * PI / 180.0, ucs + du, vleup + dv );
                c_uwm_upper = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, cup, leup, d );
                c_uwm_upper = vec3d( umid, v / vmax, 0 );
            }
        }
    }

    if ( m_SurfType() != UPPER_SURF )
    {
        if ( !m_LEFlag() )
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vtelow );
                vec3d vdir;
                vdir = ( le - te ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, telow, udir, d, midangle * PI / 180.0, ucs + du, vtelow + dv );
                c_uwm_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, telow, d );
                c_uwm_lower = vec3d( umid, v / vmax, 0 );
            }
        }
        else
        {
            if ( midangleflag )
            {
                vec3d udir = surf->CompTanU( ucs, vlelow );
                vec3d vdir;
                vdir = ( te - le ) / 2.0;
                double du, dv;
                surf->GuessDistanceAngle( du, dv, udir, vdir, d, midangle * PI / 180.0 );
                udir.normalize();
                surf->FindDistanceAngle( u, v, lelow, udir, d, midangle * PI / 180.0, ucs + du, vlelow - dv );
                c_uwm_lower = vec3d( u / umax, v / vmax, 0 );
            }
            else
            {
                piecewise_curve_type clow, cup;
                c.split( clow, cup, vle );
                eli::geom::intersect::specified_distance( v, clow, lelow, d );
                c_uwm_lower = vec3d( umid, v / vmax, 0 );
            }
        }
    }

    c_uw2_upper = c_uwm_upper;
    c_uw2_lower = c_uwm_lower;

    //////////////////////////// Repeat Block



    // Build Control Surface
    vector< vec3d > up_pnt_vec;
    vector< vec3d > low_pnt_vec;


    up_pnt_vec.reserve( 13 );
    low_pnt_vec.reserve( 13 );
    if ( !m_LEFlag() )
    {
        if ( m_SurfType() == UPPER_SURF )
        {
            up_pnt_vec.push_back( vec3d( m_UStart(), 1, 0 ) );
            up_pnt_vec.push_back( c_uws1_upper );
            up_pnt_vec.push_back( c_uws2_upper );
            up_pnt_vec.push_back( c_uws_upper );
            up_pnt_vec.push_back( c_uw1_upper );
            up_pnt_vec.push_back( c_uw2_upper );
            up_pnt_vec.push_back( c_uwe_upper );
            up_pnt_vec.push_back( c_uwe2_upper );
            up_pnt_vec.push_back( c_uwe1_upper );
            up_pnt_vec.push_back( vec3d( m_UEnd(), 1, 0 ) );
        }
        else if ( m_SurfType() == LOWER_SURF )
        {
            low_pnt_vec.push_back( vec3d( m_UStart(), 0, 0 ) );
            low_pnt_vec.push_back( c_uws1_lower );
            low_pnt_vec.push_back( c_uws2_lower );
            low_pnt_vec.push_back( c_uws_lower );
            low_pnt_vec.push_back( c_uw1_lower );
            low_pnt_vec.push_back( c_uw2_lower );
            low_pnt_vec.push_back( c_uwe_lower );
            low_pnt_vec.push_back( c_uwe2_lower );
            low_pnt_vec.push_back( c_uwe1_lower );
            low_pnt_vec.push_back( vec3d( m_UEnd(), 0, 0 ) );
        }
        else
        {
            up_pnt_vec.push_back( vec3d( m_UStart(), 1, 0 ) );
            up_pnt_vec.push_back( c_uws1_upper );
            up_pnt_vec.push_back( c_uws2_upper );
            up_pnt_vec.push_back( c_uws_upper );
            up_pnt_vec.push_back( c_uw1_upper );
            up_pnt_vec.push_back( c_uw2_upper );
            up_pnt_vec.push_back( c_uwe_upper );
            up_pnt_vec.push_back( c_uwe2_upper );
            up_pnt_vec.push_back( c_uwe1_upper );
            up_pnt_vec.push_back( vec3d( m_UEnd(), 1, 0 ) );

            low_pnt_vec.push_back( vec3d( m_UEnd(), 0, 0 ) );
            low_pnt_vec.push_back( c_uwe1_lower );
            low_pnt_vec.push_back( c_uwe2_lower );
            low_pnt_vec.push_back( c_uwe_lower );
            low_pnt_vec.push_back( c_uw2_lower );
            low_pnt_vec.push_back( c_uw1_lower );
            low_pnt_vec.push_back( c_uws_lower );
            low_pnt_vec.push_back( c_uws2_lower );
            low_pnt_vec.push_back( c_uws1_lower );
            low_pnt_vec.push_back( vec3d( m_UStart(), 0, 0 ) );
        }
        //  pnt_vec[3] = pnt_vec[0];
    }
    else
    {
        if ( m_SurfType() == UPPER_SURF )
        {
            up_pnt_vec.push_back( vec3d( m_UEnd(), 0.5, 0 ) );
            up_pnt_vec.push_back( c_uwe1_upper );
            up_pnt_vec.push_back( c_uwe2_upper );
            up_pnt_vec.push_back( c_uwe_upper );
            up_pnt_vec.push_back( c_uw2_upper );
            up_pnt_vec.push_back( c_uw1_upper );
            up_pnt_vec.push_back( c_uws_upper );
            up_pnt_vec.push_back( c_uws2_upper );
            up_pnt_vec.push_back( c_uws1_upper );
            up_pnt_vec.push_back( vec3d( m_UStart(), 0.5, 0 ) );
            up_pnt_vec.push_back( vec3d( m_UStart() + ( m_UEnd() - m_UStart() ) / 3.0, 0.5, 0 ) );
            up_pnt_vec.push_back( vec3d( m_UStart() + 2.0 * ( m_UEnd() - m_UStart() ) / 3.0, 0.5, 0 ) );
            up_pnt_vec.push_back( up_pnt_vec[0] );
        }
        else if ( m_SurfType() == LOWER_SURF )
        {
            low_pnt_vec.push_back( vec3d( m_UEnd(), 0.5, 0 ) );
            low_pnt_vec.push_back( c_uwe1_lower );
            low_pnt_vec.push_back( c_uwe2_lower );
            low_pnt_vec.push_back( c_uwe_lower );
            low_pnt_vec.push_back( c_uw2_lower );
            low_pnt_vec.push_back( c_uw1_lower );
            low_pnt_vec.push_back( c_uws_lower );
            low_pnt_vec.push_back( c_uws2_lower );
            low_pnt_vec.push_back( c_uws1_lower );
            low_pnt_vec.push_back( vec3d( m_UStart(), 0.5, 0 ) );
            low_pnt_vec.push_back( vec3d( m_UStart() + ( m_UEnd() + m_UStart() ) / 3.0, 0.5, 0 ) );
            low_pnt_vec.push_back( vec3d( m_UStart() + 2.0 * ( m_UEnd() + m_UStart() ) / 3.0, 0.5, 0 ) );
            low_pnt_vec.push_back( low_pnt_vec[0] );
        }
        else
        {
            up_pnt_vec.push_back( vec3d( m_UEnd(), 0.5, 0 ) );
            up_pnt_vec.push_back( c_uwe1_upper );
            up_pnt_vec.push_back( c_uwe2_upper );
            up_pnt_vec.push_back( c_uwe_upper );
            up_pnt_vec.push_back( c_uw2_upper );
            up_pnt_vec.push_back( c_uw1_upper );
            up_pnt_vec.push_back( c_uws_upper );
            up_pnt_vec.push_back( c_uws2_upper );
            up_pnt_vec.push_back( c_uws1_upper );
            up_pnt_vec.push_back( vec3d( m_UStart(), 0.5, 0 ) );

            low_pnt_vec.push_back( vec3d( m_UStart(), 0.5, 0 ) );
            low_pnt_vec.push_back( c_uws1_lower );
            low_pnt_vec.push_back( c_uws2_lower );
            low_pnt_vec.push_back( c_uws_lower );
            low_pnt_vec.push_back( c_uw1_lower );
            low_pnt_vec.push_back( c_uw2_lower );
            low_pnt_vec.push_back( c_uwe_lower );
            low_pnt_vec.push_back( c_uwe2_lower );
            low_pnt_vec.push_back( c_uwe1_lower );
            low_pnt_vec.push_back( vec3d( m_UEnd(), 0.5, 0 ) );
        }
        //  pnt_vec[3] = pnt_vec[0];
    }


    if ( !up_pnt_vec.empty() )
    {
        RefVec( up_pnt_vec, m_Tess() );
    }

    if ( !low_pnt_vec.empty() )
    {
        RefVec( low_pnt_vec, m_Tess() );
    }

    m_LVec.clear();

    if ( !up_pnt_vec.empty() )
    {
        for ( int i = 0; i < up_pnt_vec.size() - 1; i++ )
        {
            m_LVec.push_back( SSLineSeg() );
            m_LVec.back().SetSP0( up_pnt_vec[i] );
            m_LVec.back().SetSP1( up_pnt_vec[i+1] );
            m_LVec.back().Update( geom );
        }
    }

    m_SepIndex = m_LVec.size();

    if ( !low_pnt_vec.empty() )
    {
        for ( int i = 0; i < low_pnt_vec.size() - 1; i++ )
        {
            m_LVec.push_back( SSLineSeg() );
            m_LVec.back().SetSP0( low_pnt_vec[i] );
            m_LVec.back().SetSP1( low_pnt_vec[i+1] );
            m_LVec.back().Update( geom );
        }
    }

    m_UWStart.resize( m_UWStart01.size() );
    m_UWEnd.resize( m_UWStart01.size() );
    for ( int i = 0; i < m_UWStart01.size(); i++ )
    {
        m_UWStart[i] = vec3d( m_UWStart01[i].x() * umax, m_UWStart01[i].y() * vmax, 0.0 );
        m_UWEnd[i] = vec3d( m_UWEnd01[i].x() * umax, m_UWEnd01[i].y() * vmax, 0.0 );
    }

    SubSurface::Update();
}

void SSControlSurf::UpdateDrawObjs()
{
    SubSurface::UpdateDrawObjs();

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }
    Geom* geom = veh->FindGeom( m_CompID );
    if ( geom )
    {
        vector< VspSurf > surf_vec;
        geom->GetSurfVec( surf_vec );
        int ncopy = geom->GetNumSymmCopies();

        m_HingeDO.m_PntVec.clear();
        m_HingeDO.m_LineWidth = 2.0;
        m_HingeDO.m_Type = DrawObj::VSP_LINES;
        m_HingeDO.m_GeomID = m_ID + string( "_ss_hinge" );
        m_HingeDO.m_GeomChanged = true;

        m_ArrowDO.m_PntVec.clear();
        m_ArrowDO.m_Type = DrawObj::VSP_SHADED_TRIS;
        m_ArrowDO.m_GeomID = m_ID + string( "_ss_arrow" );
        m_ArrowDO.m_GeomChanged = true;

        for ( int i = 0; i < 4; i++ )
        {
            m_ArrowDO.m_MaterialInfo.Ambient[i] = 0.2f;
            m_ArrowDO.m_MaterialInfo.Diffuse[i] = 0.1f;
            m_ArrowDO.m_MaterialInfo.Specular[i] = 0.7f;
            m_ArrowDO.m_MaterialInfo.Emission[i] = 0.0f;
        }
        m_ArrowDO.m_MaterialInfo.Diffuse[3] = 0.5f;
        m_ArrowDO.m_MaterialInfo.Shininess = 5.0f;


        int isurf = m_MainSurfIndx();

        vector < int > symms = geom->GetSymmIndexs( isurf );
        assert( ncopy == symms.size() );

        int npt = m_UWStart01.size();

        for ( int s = 0 ; s < ncopy ; s++ )
        {
            VspSurf* surf = &( surf_vec[ symms[ s ] ] );

            vec3d pst, pend;
            for ( int i = 0; i < npt; i++ )
            {
                pst = pst + surf->CompPnt01( m_UWStart01[i].x(), m_UWStart01[i].y() );
                pend = pend + surf->CompPnt01( m_UWEnd01[i].x(), m_UWEnd01[i].y() );
            }
            pst = pst / ( 1.0 * npt );
            pend = pend / ( 1.0 * npt );

            vec3d pmid = ( pst + pend ) * 0.5;

            vec3d dir = pend - pst;
            double len = dir.mag();
            dir.normalize();

            m_HingeDO.m_PntVec.push_back( pst );
            m_HingeDO.m_PntVec.push_back( pend );

            MakeCircleArrow( pmid, dir, 0.25, m_HingeDO, m_ArrowDO );
        }
        m_ArrowDO.m_NormVec = vector <vec3d> ( m_ArrowDO.m_PntVec.size() );
    }
}

void SSControlSurf::LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec )
{
    SubSurface::LoadDrawObjs( draw_obj_vec );

    m_HingeDO.m_LineColor = m_LineColor;
    draw_obj_vec.push_back( &m_HingeDO );
    draw_obj_vec.push_back( &m_ArrowDO );
}

void SSControlSurf::RefVec( vector < vec3d > &pt_vec, int nref )
{
    vector < vec3d > pnt_ref;

    int nseg = ( pt_vec.size() - 1 ) / 3;
    pnt_ref.reserve( nref * nseg + 1 );

    vector < double > parm(4,0);
//    parm[1] = 0.5; parm[2] = 1.0;
    parm[1] = 1.0/3.0; parm[2] = 2.0/3.0; parm[3] = 1.0;

    VspCurve crv;
    for ( int iseg = 0; iseg < nseg; iseg++ )
    {
        int ifirst = iseg * 3;

        vector < vec3d > pts;
        pts.insert( pts.begin(), pt_vec.begin() + ifirst, pt_vec.begin() + ifirst + 4 );

        crv.InterpolatePCHIP( pts, parm, false );

        for ( int iref = 0; iref < nref; iref++ )
        {
            double p = iref * 1.0 / ( 1.0 * nref );
            pnt_ref.push_back( crv.CompPnt( p ) );
        }
    }
    pnt_ref.push_back( crv.CompPnt( 1.0 ) );

    pt_vec = pnt_ref;
}


void SSControlSurf::UpdatePolygonPnts()
{
    if ( m_PolyPntsReadyFlag )
    {
        return;
    }

    if ( m_SurfType() == UPPER_SURF || m_SurfType() == LOWER_SURF )
    {
        SubSurface::UpdatePolygonPnts();
        vec3d pnt = m_LVec[0].GetP0();
        m_PolyPntsVec[0].push_back( vec2d( pnt.x(), pnt.y() ) );
        return;
    }

    m_PolyPntsVec.resize( 2 );

    vec3d pnt;
    int ipp = 0;
    m_PolyPntsVec[ipp].clear();
    for ( int i = 0; i < m_SepIndex; i++ )
    {
        pnt = m_LVec[i].GetP0();
        m_PolyPntsVec[ipp].push_back( vec2d( pnt.x(), pnt.y() ) );
    }
    pnt = m_LVec[ m_SepIndex - 1 ].GetP1();
    m_PolyPntsVec[ipp].push_back( vec2d( pnt.x(), pnt.y() ) );
    m_PolyPntsVec[ipp].push_back( m_PolyPntsVec[ipp][0] );

    ipp = 1;
    m_PolyPntsVec[ipp].clear();
    for ( int i = m_SepIndex; i < m_LVec.size(); i++ )
    {
        pnt = m_LVec[i].GetP0();
        m_PolyPntsVec[ipp].push_back( vec2d( pnt.x(), pnt.y() ) );
    }
    pnt = m_LVec[ m_LVec.size() - 1 ].GetP1();
    m_PolyPntsVec[ipp].push_back( vec2d( pnt.x(), pnt.y() ) );
    m_PolyPntsVec[ipp].push_back( m_PolyPntsVec[ipp][0] );

    m_PolyPntsReadyFlag = true;
}

void SSControlSurf::PrepareSplitVec()
{
    m_SplitLVec.clear();
    m_FirstSplit = true;

    vector<SSLineSeg> grp;
    grp.reserve( m_Tess() );

    int cnt = 0;
    for ( int i = 0; i < m_LVec.size(); i++ )
    {
        grp.push_back( m_LVec[i] );
        cnt++;

        if ( cnt >= m_Tess() )
        {
            m_SplitLVec.push_back( grp );
            grp.clear();
            cnt = 0;
        }
    }
}

//////////////////////////////////////////////////////
//================== SSLineArray ===================//
//////////////////////////////////////////////////////

SSLineArray::SSLineArray( string comp_id, int type ) : SubSurface( comp_id, type )
{
    m_ConstType.Init( "ConstLineType", "SS_LineArray", this, CONST_U, CONST_U, CONST_W );
    m_ConstType.SetDescript( "Either Constant U or Constant W SSLines" );

    m_PositiveDirectionFlag.Init( "PositiveDirectionFlag", "SS_LineArray", this, true, false, true );
    m_PositiveDirectionFlag.SetDescript( "Flag to Increment SSLines in Positive or Negative Direction" );

    m_Spacing.Init( "Spacing", "SS_LineArray", this, 0.2, 1e-6, 1.0 );
    m_Spacing.SetDescript( "Spacing Between SSLines in Array" );

    m_StartLocation.Init( "StartLocation", "SS_LineArray", this, 0.0, 0.0, 1.0 );
    m_StartLocation.SetDescript( "Location of First SSLine in Array" );

    m_EndLocation.Init( "EndLocation", "SS_LineArray", this, 1.0, 0.0, 1.0 );
    m_EndLocation.SetDescript( "Location for Final SSLine in Array" );

    // Set to only Beam elements (cap) with no tags (tris)
    m_TestType = SSLineSeg::NO;
    m_IncludedElements.Set( vsp::FEA_BEAM );

    m_NumLines = 0;
}

SSLineArray::~SSLineArray()
{
}

void SSLineArray::Update()
{
    CalcNumLines();

    m_LVec.resize( m_NumLines );

    for ( size_t i = 0; i < m_NumLines; i++ )
    {
        double dir = 1;
        if ( !m_PositiveDirectionFlag() )
        {
            dir = -1;
        }

        double const_val = m_StartLocation() + dir * i * m_Spacing();

        if ( m_ConstType() == CONST_U )
        {
            m_LVec[i].SetSP0( vec3d( const_val, 1, 0 ) );
            m_LVec[i].SetSP1( vec3d( const_val, 0, 0 ) );
        }
        else if ( m_ConstType() == CONST_W )
        {
            m_LVec[i].SetSP0( vec3d( 0, const_val, 0 ) );
            m_LVec[i].SetSP1( vec3d( 1, const_val, 0 ) );
        }

        m_LVec[i].m_TestType = m_TestType();

        Geom* geom = VehicleMgr.GetVehicle()->FindGeom( m_CompID );

        if ( !geom )
        {
            return;
        }

        m_LVec[i].Update( geom );
    }

    SubSurface::Update();
}

void SSLineArray::CalcNumLines()
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        Geom* current_geom = veh->FindGeom( m_CompID );

        if ( !current_geom )
        {
            return;
        }

        vector< VspSurf > surf_vec;
        current_geom->GetSurfVec( surf_vec );
        VspSurf current_surf = surf_vec[m_MainSurfIndx()];

        if ( m_PositiveDirectionFlag() )
        {
            if ( m_EndLocation() < m_StartLocation() )
            {
                double temp_end = m_EndLocation();
                m_EndLocation.Set( m_StartLocation() );
                m_StartLocation.Set( temp_end );
            }

            m_EndLocation.SetLowerUpperLimits( m_StartLocation(), 1.0 );
            m_StartLocation.SetLowerUpperLimits( 0.0, m_EndLocation() );

            m_Spacing.SetLowerUpperLimits( ( m_EndLocation() - m_StartLocation() ) / 100, ( m_EndLocation() - m_StartLocation() ) ); // Limit to 100 lines
            m_NumLines = 1 + (int)floor( ( m_EndLocation() - m_StartLocation() ) / m_Spacing() );
        }
        else
        {
            if ( m_StartLocation() < m_EndLocation() )
            {
                double temp_start = m_StartLocation();
                m_StartLocation.Set( m_EndLocation() );
                m_EndLocation.Set( temp_start );
            }

            m_StartLocation.SetLowerUpperLimits( m_EndLocation(), 1.0 );
            m_EndLocation.SetLowerUpperLimits( 0.0, m_StartLocation() );

            m_Spacing.SetLowerUpperLimits( ( m_StartLocation() - m_EndLocation() ) / 100, ( m_StartLocation() - m_EndLocation() ) ); // Limit to 100 lines
            m_NumLines = 1 + (int)floor( ( m_StartLocation() - m_EndLocation() ) / m_Spacing() );
        }

        if ( m_NumLines < 1 || m_NumLines > 101 )
        {
            m_NumLines = 1;
        }
    }
}

SSLine* SSLineArray::AddSSLine( double location, int ind )
{
    SSLine* ssline = new SSLine( m_CompID );

    if ( ssline )
    {
        ssline->m_IncludedElements.Set( m_IncludedElements() );
        ssline->m_ConstType.Set( m_ConstType() );
        ssline->m_ConstVal.Set( location );
        ssline->m_TestType.Set( m_TestType() );
        ssline->m_FeaPropertyIndex.Set( m_FeaPropertyIndex() );
        ssline->m_CapFeaPropertyIndex.Set( m_CapFeaPropertyIndex() );

        ssline->SetName( string( m_Name + "_SSLine_" + std::to_string( ind ) ) );

        ssline->Update();
    }

    return ssline;
}

int SSLineArray::CompNumDrawPnts( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return 0;
    }

    if ( m_ConstType() == CONST_W )
    {
        return (int)( surf->GetUMax() * ( geom->m_TessU() - 2 ) );
    }
    else if ( m_ConstType() == CONST_U )
    {
        return (int)( surf->GetWMax() * ( geom->m_TessW() - 4 ) );
    }

    return -1;
}
