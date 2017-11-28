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
#include "APIDefines.h"

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

    m_SwapIJFlag.Init( "FlipIJFlag", "Wireframe", this, false, false, true );
    m_RevIFlag.Init( "RevIFlag", "Wireframe", this, false, false, true );
    m_RevJFlag.Init( "RevJFlag", "Wireframe", this, false, false, true );

    m_IStride.Init( "IStride", "WireFrame", this, 1, 1, 1e3 );
    m_JStride.Init( "JStride", "WireFrame", this, 1, 1, 1e3 );

    m_IStartPatchType.Init( "IStartPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );
    m_IEndPatchType.Init( "IStartPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );
    m_JStartPatchType.Init( "IStartPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );
    m_JEndPatchType.Init( "IStartPatchType", "WireFrame", this, vsp::PATCH_NONE, vsp::PATCH_NONE, vsp::PATCH_NUM_TYPES - 1 );

    Update();
}

//==== Destructor ====//
WireGeom::~WireGeom()
{
}

void WireGeom::UpdateSurf()
{
    int num_j, num_i;

    num_i = m_WirePts.size();

    if ( num_i <= 0 )
    {
        m_XFormPts.resize(0);
        m_XFormNorm.resize(0);
        return;
    }

    num_j = m_WirePts[0].size();

    if ( num_j <= 0 )
    {
        m_XFormPts.resize(0);
        m_XFormNorm.resize(0);
        return;
    }

    // Perform transformation on base points.
    Matrix4d transMat = GetTotalTransMat();
    m_XFormPts.resize( num_i );
    for ( int i = 0 ; i < num_i ; i++ )
    {
        m_XFormPts[i].resize( num_j );
        for ( int j = 0 ; j < num_j ; j++ )
        {
            m_XFormPts[i][j] = transMat.xform( m_WirePts[i][j] );
        }
    }

    // Handle swapping I/J.
    if ( m_SwapIJFlag() )
    {
        vector < vector < vec3d > > tmppts;

        tmppts.resize( num_j );
        for ( int j = 0 ; j < num_j ; j++ )
        {
            tmppts[j].resize( num_i );
        }

        for ( int i = 0 ; i < num_i ; i++ )
        {
            for ( int j = 0 ; j < num_j ; j++ )
            {
                tmppts[j][i] = m_XFormPts[i][j];
            }
        }

        double tmp = num_i;
        num_i = num_j;
        num_j = tmp;

        m_XFormPts = tmppts;
    }

    // Handle reversing I
    if ( m_RevIFlag() )
    {
        vector < vector < vec3d > > tmppts;

        tmppts.resize( num_i );
        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j );
        }


        for ( int i = 0 ; i < num_i ; i++ )
        {
            int k = num_i - i - 1;
            for ( int j = 0 ; j < num_j ; j++ )
            {
                tmppts[k][j] = m_XFormPts[i][j];
            }
        }

        m_XFormPts = tmppts;
    }

    // Handle reversing J
    if ( m_RevJFlag() )
    {
        vector < vector < vec3d > > tmppts;

        tmppts.resize( num_i );
        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j );
        }


        for ( int i = 0 ; i < num_i ; i++ )
        {
            for ( int j = 0 ; j < num_j ; j++ )
            {
                int k = num_j - j - 1;
                tmppts[i][k] = m_XFormPts[i][j];
            }
        }

        m_XFormPts = tmppts;
    }

    if ( m_IStartPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow = m_XFormPts[0];
        oppositerow = m_XFormPts[ m_XFormPts.size() - 1 ];

        PatchRow( oldrow, oppositerow, m_IStartPatchType(), newrow );

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i + 1 );
        tmppts[0] = newrow;
        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i+1] = m_XFormPts[i];
        }

        m_XFormPts = tmppts;
        num_i++;
    }

    if ( m_IEndPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow = m_XFormPts[ m_XFormPts.size() - 1 ];
        oppositerow = m_XFormPts[0];

        PatchRow( oldrow, oppositerow, m_IEndPatchType(), newrow );

        m_XFormPts.push_back( newrow );
        num_i++;
    }

    if ( m_JStartPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow.resize( num_i );
        oppositerow = oldrow;

        for ( int i = 0 ; i < num_i ; i++ )
        {
            oldrow[i] = m_XFormPts[i][0];
            oppositerow[i] = m_XFormPts[i][num_j - 1];
        }

        PatchRow( oldrow, oppositerow, m_JStartPatchType(), newrow );

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );

        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j + 1 );
        }

        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i][0] = newrow[i];

            for ( int j = 0 ; j < num_j ; j++ )
            {
                tmppts[i][j+1] = m_XFormPts[i][j];
            }
        }

        m_XFormPts = tmppts;
        num_j++;
    }

    if ( m_JEndPatchType() != vsp::PATCH_NONE )
    {
        vector < vec3d > oldrow;
        vector < vec3d > oppositerow;

        vector < vec3d > newrow;

        oldrow.resize( num_i );
        oppositerow = oldrow;

        for ( int i = 0 ; i < num_i ; i++ )
        {
            oldrow[i] = m_XFormPts[i][num_j - 1];
            oppositerow[i] = m_XFormPts[i][0];
        }

        PatchRow( oldrow, oppositerow, m_JEndPatchType(), newrow );

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );

        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j + 1 );
        }

        for ( int i = 0 ; i < num_i ; i++ )
        {
            for ( int j = 0 ; j < num_j ; j++ )
            {
                tmppts[i][j] = m_XFormPts[i][j];
            }
            tmppts[i][num_j] = newrow[i];
        }

        m_XFormPts = tmppts;
        num_j++;
    }

    // Handle skipping.
    if ( m_ISkipStart() != 0 || m_ISkipEnd() != 0 || m_JSkipStart() != 0 || m_JSkipEnd() != 0 )
    {
        num_i = num_i - m_ISkipStart() - m_ISkipEnd();
        num_j = num_j - m_JSkipStart() - m_JSkipEnd();

        if ( num_i <= 0 || num_j <= 0 ) // No surface left
        {
            m_XFormPts.resize(0);
            m_XFormNorm.resize(0);
            return;
        }

        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );
        for ( int i = 0 ; i < num_i ; i++ )
        {
            tmppts[i].resize( num_j );
            for ( int j = 0 ; j < num_j ; j++ )
            {
                tmppts[i][j] =  m_XFormPts[i + m_ISkipStart()][j + m_JSkipStart()];
            }
        }
        m_XFormPts = tmppts;
    }

    // Handle I Stride
    if ( m_IStride() != 1 )
    {
        vector < vector < vec3d > > tmppts;

        int ilast;
        for ( int i = 0 ; i < num_i ; i = i + m_IStride() )
        {
            tmppts.push_back( m_XFormPts[i] );
            ilast = i;
        }
        if ( ilast != num_i - 1 )
        {
            tmppts.push_back( m_XFormPts[num_i - 1] );
        }

        m_XFormPts = tmppts;
        num_i = m_XFormPts.size();
    }

    // Handle J Stride
    if ( m_JStride() != 1 )
    {
        vector < vector < vec3d > > tmppts;
        tmppts.resize( num_i );

        int jlast;
        for ( int j = 0 ; j < num_j ; j = j + m_JStride() )
        {
            for ( int i = 0 ; i < num_i ; i++ )
            {
                tmppts[i].push_back( m_XFormPts[i][j] );
            }
            jlast = j;
        }
        if ( jlast != num_j - 1 )
        {
            for ( int i = 0 ; i < num_i ; i++ )
            {
                tmppts[i].push_back( m_XFormPts[i][num_j - 1] );
            }
        }

        m_XFormPts = tmppts;
        num_j = m_XFormPts[0].size();
    }

    // Calculate normal vectors.
    m_XFormNorm.resize( num_i );
    for ( int i = 0 ; i < num_i ; i++ )
    {
        m_XFormNorm[i].resize( num_j );

        int inext = clamp( i + 1, 0, num_i - 1 );
        int iprev = clamp( i - 1, 0, num_i - 1 );
        for ( int j = 0 ; j < num_j ; j++ )
        {
            int jnext = clamp( j + 1, 0, num_j - 1 );
            int jprev = clamp( j - 1, 0, num_j - 1 );

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

void WireGeom::PatchRow( const vector < vec3d > &oldrow, const vector < vec3d > &oppositerow, int type, vector < vec3d > &newrow )
{
    if ( type == vsp::PATCH_POINT )
    {
        vec3d cen;
        for ( int i = 0; i < oldrow.size(); i++ )
        {
            cen = cen + oldrow[i];
        }
        cen = cen * 1.0 / oldrow.size();

        newrow.resize( oldrow.size(), cen );
    }
    else if ( type == vsp::PATCH_LINE )
    {
        newrow = oldrow;
        int n = ceil( newrow.size() * 0.5 );
        for ( int i = 0; i < n; i++ )
        {
            int j = newrow.size() - 1 - i;
            vec3d pt = ( newrow[i] + newrow[j] ) * 0.5;
            newrow[i] = pt;
            newrow[j] = pt;
        }
    }
    else if ( type == vsp::PATCH_COPY )
    {
        newrow = oppositerow;
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

    //=== Axis ===//
    m_AxisDrawObj_vec.clear();
    m_AxisDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }

    int num_cross = ( int ) m_XFormPts.size();
    if ( num_cross > 0 )
    {
        int ilim = floor( num_cross * 0.5 );

        int num_pnts = ( int ) m_XFormPts[0].size();

        if ( num_pnts > 0 )
        {
            int jlim = floor( num_pnts * 0.5 );

            m_FeatureDrawObj_vec.clear();
            m_FeatureDrawObj_vec.resize( 2 );

            for ( int i = 0; i < ilim - 1; i++ )
            {
                m_FeatureDrawObj_vec[0].m_PntVec.push_back( m_XFormPts[ i ][0] );
                m_FeatureDrawObj_vec[0].m_PntVec.push_back( m_XFormPts[ i + 1 ][0] );
            }
            m_FeatureDrawObj_vec[0].m_LineColor = vec3d( 1, 0, 0 );
            m_FeatureDrawObj_vec[0].m_LineWidth = 3.0;
            m_FeatureDrawObj_vec[0].m_GeomChanged = true;

            for ( int j = 0; j < jlim - 1; j++ )
            {
                m_FeatureDrawObj_vec[1].m_PntVec.push_back( m_XFormPts[0][ j ] );
                m_FeatureDrawObj_vec[1].m_PntVec.push_back( m_XFormPts[0][ j + 1 ] );
            }
            m_FeatureDrawObj_vec[1].m_LineColor = vec3d( 0, 1, 0 );
            m_FeatureDrawObj_vec[1].m_LineWidth = 3.0;
            m_FeatureDrawObj_vec[1].m_GeomChanged = true;
        }
    }
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
