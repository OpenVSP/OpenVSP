//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "WireGeom.h"
#include "Vehicle.h"
#include "StringUtil.h"
#include "Util.h"

//==== Constructor ====//
WireGeom::WireGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "WireGeom";
    m_Type.m_Name = "WireFrame";
    m_Type.m_Type = WIRE_FRAME_GEOM_TYPE;

    // Disable Parameters that don't make sense for PtCloudGeom
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    m_ScaleMatrix.loadIdentity();
    m_ScaleFromOrig.Init( "Scale_From_Original", "XForm", this, 1, 1.0e-5, 1.0e12 );

    m_WireType.Init( "WireType", "WireFrame", this, 0, 0, 1 );

    m_InvertFlag.Init( "InvertFlag", "Wireframe", this, false, false, true );

    m_ISkipStart.Init( "ISkipStart", "WireFrame", this, 0, 0, 1e6 );
    m_ISkipEnd.Init( "ISkipEnd", "WireFrame", this, 0, 0, 1e6 );
    m_JSkipStart.Init( "JSkipStart", "WireFrame", this, 0, 0, 1e6 );
    m_JSkipEnd.Init( "JSkipEnd", "WireFrame", this, 0, 0, 1e6 );

    Update();
}

//==== Destructor ====//
WireGeom::~WireGeom()
{
}

void WireGeom::UpdateSurf()
{
    int num_pnts, num_cross;

    num_cross = ( int ) m_WirePts.size() - m_ISkipStart() - m_ISkipEnd();

    if ( num_cross <= 0 )
    {
        m_XFormPts.resize( 0 );
        m_XFormNorm.resize( 0 );
        return;
    }

    Matrix4d transMat = GetTotalTransMat();

    m_XFormPts.resize( num_cross );
    m_XFormNorm.resize( num_cross );

    num_pnts = ( int ) m_WirePts[0].size() - m_JSkipStart() - m_JSkipEnd();

    if ( num_pnts <= 0 )
    {
        m_XFormPts.resize( 0 );
        m_XFormNorm.resize( 0 );
        return;
    }

    for ( int i = 0 ; i < num_cross ; i++ )
    {
        m_XFormPts[i].resize( num_pnts );
        m_XFormNorm[i].resize( num_pnts );
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            m_XFormPts[i][j] = transMat.xform( m_WirePts[i + m_ISkipStart()][j + m_JSkipStart()] );
        }
    }

    for ( int i = 0 ; i < num_cross ; i++ )
    {
        int inext = clamp( i + 1, 0, num_cross - 1 );
        int iprev = clamp( i - 1, 0, num_cross - 1 );
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            int jnext = clamp( j + 1, 0, num_pnts - 1 );
            int jprev = clamp( j - 1, 0, num_pnts - 1 );

            vec3d di = m_XFormPts[inext][j] - m_XFormPts[iprev][j];
            vec3d dj = m_XFormPts[i][jnext] - m_XFormPts[i][jprev];

            vec3d n = cross( di, dj );
            n.normalize();

            if ( m_InvertFlag() )
            {
                n = -1.0 * n;
            }

            m_XFormNorm[i][j] = n;
        }
    }

}

void WireGeom::UpdateDrawObj()
{
    m_WireShadeDrawObj_vec.clear();
    m_WireShadeDrawObj_vec.resize( 1 );
    m_WireShadeDrawObj_vec[0].m_FlipNormals = false;
    m_WireShadeDrawObj_vec[0].m_GeomChanged = true;

    vector < vector < double > > dum;
    dum.resize( m_XFormPts.size() );
    if ( dum.size() > 0 )
    {
        for ( int i = 0; i < m_XFormPts.size(); i++ )
        {
            dum[i].resize( m_XFormPts[0].size() );
        }
    }

    m_WireShadeDrawObj_vec[0].m_PntMesh.resize( 1 );
    m_WireShadeDrawObj_vec[0].m_PntMesh[0].insert( m_WireShadeDrawObj_vec[0].m_PntMesh[0].end(), m_XFormPts.begin(), m_XFormPts.end() );
    m_WireShadeDrawObj_vec[0].m_NormMesh.resize( 1 );
    m_WireShadeDrawObj_vec[0].m_NormMesh[0].insert( m_WireShadeDrawObj_vec[0].m_NormMesh[0].end(), m_XFormNorm.begin(), m_XFormNorm.end() );
    m_WireShadeDrawObj_vec[0].m_uTexMesh.resize( 1 );
    m_WireShadeDrawObj_vec[0].m_uTexMesh[0].insert( m_WireShadeDrawObj_vec[0].m_uTexMesh[0].end(), dum.begin(), dum.end() );
    m_WireShadeDrawObj_vec[0].m_vTexMesh.resize( 1 );
    m_WireShadeDrawObj_vec[0].m_vTexMesh[0].insert( m_WireShadeDrawObj_vec[0].m_vTexMesh[0].end(), dum.begin(), dum.end() );

    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();
}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d WireGeom::GetTotalTransMat()
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix.data() );
    retMat.postMult( m_ModelMatrix.data() );

    return retMat;
}

void WireGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}

void WireGeom::UpdateBBox()
{
    m_BBox.Reset();

    int num_pnts, num_cross;

    num_cross = ( int ) m_XFormPts.size();

    if ( num_cross == 0 )
    {
        m_BBox.Update( vec3d( 0.0, 0.0, 0.0 ) );
    }

    for ( int i = 0 ; i < num_cross ; i++ )
    {
        num_pnts = ( int ) m_XFormPts[0].size();

        if ( num_pnts == 0 )
        {
            m_BBox.Update( vec3d( 0.0, 0.0, 0.0 ) );
        }

        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            m_BBox.Update( m_XFormPts[i][j] );
        }
    }
}

void WireGeom::ReadXSec( FILE* fp )
{
    char name_str[256];
    float x, y, z;
    int group_num, type;
    int num_pnts, num_cross;

    fgets( name_str, 256, fp );
    sscanf( name_str, "%s", name_str );
    fscanf( fp, " GROUP NUMBER = %d\n", &group_num );
    fscanf( fp, " TYPE = %d\n", &type );
    fscanf( fp, " CROSS SECTIONS = %d\n", &( num_cross ) );
    fscanf( fp, " PTS/CROSS SECTION = %d\n", &( num_pnts ) );

    m_Name = string( name_str );
    StringUtil::chance_space_to_underscore( m_Name );
    m_WireType = type;

    //===== Size Cross Vec ====//
    m_WirePts.resize( num_cross );
    for ( int i = 0 ; i < num_cross ; i++ )
    {
        m_WirePts[i].resize( num_pnts );
        for ( int j = 0 ; j < num_pnts ; j++ )
        {
            fscanf( fp, "%f %f %f\n", &x, &y, &z );
            m_WirePts[i][j].set_xyz( x, y, z );
        }
    }

    m_InvertFlag = CheckInverted();

    Update();
}

//==== Encode XML ====//
xmlNodePtr WireGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );

    int num_cross = m_WirePts.size();
    if ( num_cross > 0 )
    {
        int num_pnts = m_WirePts[0].size();
        if ( num_pnts > 0 )
        {
            xmlNodePtr wire_node = xmlNewChild( node, NULL, BAD_CAST "WireGeom", NULL );

            if ( wire_node )
            {
                XmlUtil::AddIntNode( wire_node, "Num_Cross", num_cross );
                XmlUtil::AddIntNode( wire_node, "Num_Pnts", num_pnts );

                xmlNodePtr pt_list_node = xmlNewChild( wire_node, NULL, BAD_CAST "Pt_List", NULL );

                for ( int i = 0 ; i < num_cross ; i++ )
                {
                    for ( int j = 0 ; j < num_pnts ; j++ )
                    {
                        XmlUtil::AddVec3dNode( pt_list_node, "Pt", m_WirePts[i][j] );
                    }
                }
            }
        }
    }

    return node;
}

//==== Decode XML ====//
xmlNodePtr WireGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr wire_node = XmlUtil::GetNode( node, "WireGeom", 0 );
    if ( wire_node )
    {
        int num_cross = XmlUtil::FindInt( wire_node, "Num_Cross", 0 );
        int num_pnts = XmlUtil::FindInt( wire_node, "Num_Pnts", 0 );

        xmlNodePtr pt_list_node = XmlUtil::GetNode( wire_node, "Pt_List", 0 );
        if ( pt_list_node )
        {
            int num_pts = XmlUtil::GetNumNames( pt_list_node, "Pt" );

            if ( num_pts == num_cross * num_pnts )
            {
                vector<vec3d> tmppts;
                tmppts.reserve( num_pts );

                xmlNodePtr iter_node = pt_list_node->xmlChildrenNode;
                while ( iter_node != NULL )
                {
                    if ( !xmlStrcmp(iter_node->name, (const xmlChar *) "Pt") )
                    {
                        vec3d pt = XmlUtil::GetVec3dNode( iter_node );
                        tmppts.push_back( pt );
                    }
                    iter_node = iter_node->next;
                }


                // Transfer vector of points to matrix storage.
                int k = 0;
                m_WirePts.resize( num_cross );
                for ( int i = 0; i < num_cross; i++ )
                {
                    m_WirePts[i].resize( num_pnts );

                    for ( int j = 0; j < num_pnts; j++ )
                    {
                        m_WirePts[i][j] = tmppts[k];
                        k++;
                    }
                }
            }
        }
    }

    return wire_node;
}

bool WireGeom::CheckInverted()
{
    int num_pnts, num_cross;

    num_cross = ( int ) m_WirePts.size();

    if ( num_cross == 0 )
    {
        return false;
    }

    num_pnts = ( int ) m_WirePts[0].size();

    if ( num_pnts == 0 )
    {
        return false;
    }

    // Find approximate center point to improve volume calculation.
    vec3d cen;
    for ( int i = 0 ; i < num_cross; i++ )
    {
        for ( int j = 0 ; j < num_pnts; j++ )
        {
            cen = cen + m_WirePts[i][j];
        }
    }
    cen = cen / ( num_cross * num_pnts );

    // Find approximate volume.  Since bodies are not required to be
    // watertight, this may not be perfectly accurate.  However, we're
    // only interested in the sign of the result.
    double vol = 0;
    for ( int i = 0 ; i < num_cross - 1; i++ )
    {
        for ( int j = 0 ; j < num_pnts - 1; j++ )
        {
            vec3d a = m_WirePts[i][j] - cen;
            vec3d b = m_WirePts[i+1][j] - cen;
            vec3d c = m_WirePts[i+1][j+1] - cen;
            vec3d d = m_WirePts[i][j+1] - cen;

            vol = vol + tetra_volume( a, b, c )
                      + tetra_volume( a, c, d );
        }
    }

    // Check volume sign as indication of orientation.
    if ( vol < 0 )
    {
        return true;
    }
    return false;
}

//==== Create TMesh Vector ====//
vector< TMesh* > WireGeom::CreateTMeshVec()
{
    vector < TMesh* > tmeshvec;

    int num_pnts, num_cross;

    num_cross = ( int ) m_XFormPts.size();

    if ( num_cross == 0 )
    {
        return tmeshvec;
    }

    num_pnts = ( int ) m_XFormPts[0].size();

    if ( num_pnts == 0 )
    {
        return tmeshvec;
    }

    TMesh*  tMesh = new TMesh();
    //==== Convert CrossSections to Triangles ====//
    for ( int i = 1 ; i < num_cross ; i++ )
    {
        for ( int j = 1; j < num_pnts; j++ )
        {
            if ( m_InvertFlag() )
            {
                tMesh->AddTri( m_XFormPts[i - 1][j - 1], m_XFormPts[i][j], m_XFormPts[i][j - 1] );
                tMesh->AddTri( m_XFormPts[i - 1][j - 1], m_XFormPts[i - 1][j], m_XFormPts[i][j] );
            }
            else
            {
                tMesh->AddTri( m_XFormPts[i - 1][j - 1], m_XFormPts[i][j - 1], m_XFormPts[i][j] );
                tMesh->AddTri( m_XFormPts[i - 1][j - 1], m_XFormPts[i][j], m_XFormPts[i - 1][j] );
            }
        }
    }

    tmeshvec.push_back( tMesh );
    return tmeshvec;
}

//==== Create Degenerate Geometry ====//
// When preview = true, this simplifies to generate only the
// required degen plate,surface, and subsurface for updating the preview DrawObj vectors
void WireGeom::CreateDegenGeom( vector<DegenGeom> &dgs, bool preview )
{
    int num_pnts, num_cross;

    num_cross = ( int ) m_XFormPts.size();

    if ( num_cross == 0 )
    {
        return;
    }

    num_pnts = ( int ) m_XFormPts[0].size();

    if ( num_pnts == 0 )
    {
        return;
    }

    vector< vector< vec3d > > uwpnts;
    uwpnts.resize( num_cross );

    for ( int i = 0; i < num_cross; i++ )
    {
        uwpnts[i].resize( num_pnts );
        for ( int j = 0; j < num_pnts; j++ )
        {
            uwpnts[i][j] = vec3d( i * 1.0 / num_cross, j * 1.0 / num_pnts, 0.0 );
        }
    }

    int surftype = DegenGeom::SURFACE_TYPE;
    if ( m_WireType() == 1 )
    {
        surftype = DegenGeom::BODY_TYPE;
    }

    Geom::CreateDegenGeom( dgs, m_XFormPts, m_XFormNorm, uwpnts, false, 0, preview, m_InvertFlag(), surftype, NULL );

}
