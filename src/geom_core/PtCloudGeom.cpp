//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PtCloudGeom.h"
#include "PntNodeMerge.h"
#include "Vehicle.h"
#include "FitModelMgr.h"

//==== Constructor ====//
PtCloudGeom::PtCloudGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "PtCloudGeom";
    m_Type.m_Name = "PtCloud";
    m_Type.m_Type = PT_CLOUD_GEOM_TYPE;

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

    Update();
}

//==== Destructor ====//
PtCloudGeom::~PtCloudGeom()
{
}

void PtCloudGeom::UpdateSurf()
{
}

void PtCloudGeom::UpdateDrawObj()
{
    m_PickDrawObj.m_Type = DrawObj::VSP_PICK_VERTEX;
    m_PickDrawObj.m_GeomID = PICKVERTEXHEADER + m_ID;
    m_PickDrawObj.m_PickSourceID = m_ID;
    m_PickDrawObj.m_FeedbackGroup = getFeedbackGroupName();
    m_PickDrawObj.m_PointSize = 4.0;

    m_HighlightDrawObj.m_PntVec = m_BBox.GetBBoxDrawLines();

    Matrix4d transMat = GetTotalTransMat();
    m_XformPts.resize( m_Pts.size() );
    for ( int i = 0 ; i < ( int )m_Pts.size() ; i++ )
    {
        m_XformPts[i] = transMat.xform( m_Pts[i] );
    }
}

void PtCloudGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    vec3d ptColor = vec3d( m_GuiDraw.GetWireColor().x() / 255.0,
        m_GuiDraw.GetWireColor().y() / 255.0,
        m_GuiDraw.GetWireColor().z() / 255.0 );

    m_PtsDrawObj.m_Type = DrawObj::VSP_POINTS;
    m_PtsDrawObj.m_PointColor = ptColor;
    m_PtsDrawObj.m_PointSize = 4.0;
    m_PtsDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN );
    m_PtsDrawObj.m_GeomID = m_ID;

    m_SelDrawObj.m_Type = DrawObj::VSP_POINTS;
    m_SelDrawObj.m_PointColor = vec3d( 0., 0., 1.0 );
    m_SelDrawObj.m_PointSize = 4.0;
    m_SelDrawObj.m_GeomID = m_ID + "_SELPTCLD";

    m_ShownIndx.clear();
    m_PtsDrawObj.m_PntVec.clear();
    m_SelDrawObj.m_PntVec.clear();

    if ( FitModelMgr.IsGUIShown() )
    {
        for ( int j = 0; j < ( int ) m_XformPts.size(); j++ )
        {
            if ( !m_Hidden[j] && !m_Selected[j] )
            {
                m_PtsDrawObj.m_PntVec.push_back( m_XformPts[j] );
                m_ShownIndx.push_back( j );
            }

            if ( m_Selected[j] )
            {
                m_SelDrawObj.m_PntVec.push_back( m_XformPts[j] );
            }
        }
        m_SelDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN );
        m_PickDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN );
    }
    else
    {
        for ( int j = 0; j < ( int ) m_XformPts.size(); j++ )
        {
            m_PtsDrawObj.m_PntVec.push_back( m_XformPts[j] );
            m_ShownIndx.push_back( j );
        }
        m_SelDrawObj.m_Visible = false;
        m_PickDrawObj.m_Visible = false;
    }


    draw_obj_vec.push_back( &m_PtsDrawObj );
    draw_obj_vec.push_back( &m_SelDrawObj );
    draw_obj_vec.push_back( &m_PickDrawObj );

    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_GeomID = BBOXHEADER + m_ID;
        m_HighlightDrawObj.m_LineWidth = 2.0;
        m_HighlightDrawObj.m_LineColor = vec3d( 1.0, 0., 0.0 );
        m_HighlightDrawObj.m_Type = DrawObj::VSP_LINES;
        draw_obj_vec.push_back( &m_HighlightDrawObj );
    }
}

string PtCloudGeom::getFeedbackGroupName()
{
    return string("FitModelGUIGroup");
}

//==== Get Total Transformation Matrix from Original Points ====//
Matrix4d PtCloudGeom::GetTotalTransMat()
{
    Matrix4d retMat;
    retMat.initMat( m_ScaleMatrix.data() );
    retMat.postMult( m_ModelMatrix.data() );

    return retMat;
}

void PtCloudGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_ScaleFromOrig *= currentScale;
    m_ScaleMatrix.loadIdentity();
    m_ScaleMatrix.scale( m_ScaleFromOrig() );
    m_LastScale = m_Scale();
}

void PtCloudGeom::UpdateBBox()
{
    int i;
    m_BBox.Reset();
    Matrix4d transMat = GetTotalTransMat();

    if ( m_Pts.size() > 0 )
    {
        for ( i = 0 ; i < ( int )m_Pts.size() ; i++ )
        {
            m_BBox.Update( transMat.xform( m_Pts[i] ) );
        }
    }
    else
    {
        m_BBox.Update( vec3d( 0.0, 0.0, 0.0 ));
    }
}

int PtCloudGeom::ReadPTS( const char* file_name )
{
    FILE* file_id = fopen( file_name, "r" );

    if ( !file_id )
    {
        return 0;
    }
    else
    {
        m_Pts.clear();
        int stopFlag = 0;
        while ( !stopFlag )
        {
            double x, y, z;
            if ( EOF == fscanf( file_id, "%lf %lf %lf\n", &x, &y, &z ) )
            {
                break;
            }
            m_Pts.push_back( vec3d( x, y, z ) );
        }
        fclose( file_id );
    }

    if ( m_Pts.size() == 0 )
    {
        return 0;
    }

    InitPts();

    return 1;
}

void PtCloudGeom::UniquePts()
{
    //==== Build Map ====//
    PntNodeCloud pnCloud;
    pnCloud.AddPntNodes( m_Pts );

    //==== Compute Tol ====//
    double tol = m_BBox.GetLargestDist() * 1.0e-10;

    if ( tol < 1.0e-10 )
    {
        tol = 1.0e-10;
    }

    //==== Use NanoFlann to Find Close Points and Group ====//
    IndexPntNodes( pnCloud, tol );

    vector < vec3d > newpts;
    //==== Load Used Nodes ====//
    for ( int i = 0 ; i < ( int )m_Pts.size() ; i++ )
    {
        if ( pnCloud.UsedNode( i ) )
        {
            newpts.push_back( m_Pts[i] );
        }
    }
    m_Pts = newpts;
}

void PtCloudGeom::InitPts()
{
    UpdateBBox();
    UniquePts();

    m_SurfDirty = true;

    unsigned int n = m_Pts.size();
    m_Selected.assign( n, false );
    m_Hidden.assign( n, false );
}

//==== Encode XML ====//
xmlNodePtr PtCloudGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr ptcloud_node = xmlNewChild( node, NULL, BAD_CAST "PtCloudGeom", NULL );

    // Previous encoding -- all points as a large vector.
    // required too much memory to read in.
    // XmlUtil::AddVectorVec3dNode( ptcloud_node, "Points" , m_Pts );

    xmlNodePtr pt_list_node = xmlNewChild( ptcloud_node, NULL, BAD_CAST "Pt_List", NULL );
    for ( int i = 0 ; i < ( int ) m_Pts.size() ; i++ )
    {
        XmlUtil::AddVec3dNode( pt_list_node, "Pt", m_Pts[i] );
    }

    return ptcloud_node;
}

//==== Decode XML ====//
xmlNodePtr PtCloudGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr ptcloud_node = XmlUtil::GetNode( node, "PtCloudGeom", 0 );
    if ( ptcloud_node )
    {
        // Read in old encoding if it exists.
        m_Pts = XmlUtil::ExtractVectorVec3dNode( ptcloud_node, "Points" );

        // Read in new encoding if they exist.
        xmlNodePtr pt_list_node = XmlUtil::GetNode( ptcloud_node, "Pt_List", 0 );
        if ( pt_list_node )
        {
            int num_pts = XmlUtil::GetNumNames( pt_list_node, "Pt" );
            m_Pts.reserve( m_Pts.size() + num_pts );

            xmlNodePtr iter_node = pt_list_node->xmlChildrenNode;

            while( iter_node != NULL )
            {
                if ( !xmlStrcmp( iter_node->name, ( const xmlChar * )"Pt" ) )
                {
                    vec3d pt = XmlUtil::GetVec3dNode( iter_node );
                    m_Pts.push_back( pt );
                }
                iter_node = iter_node->next;
            }
        }
    }
    InitPts();

    return ptcloud_node;
}

void PtCloudGeom::SelectPoint( int index )
{
    m_Selected[ m_ShownIndx[ index ] ] = true;
    m_LastSelected = m_ShownIndx[ index ];
    m_NumSelected++;
}

void PtCloudGeom::UnSelectLastSel()
{
    if ( m_LastSelected >= 0 && m_LastSelected < m_Selected.size() )
    {
        if ( m_Selected[ m_LastSelected ] )
        {
            m_NumSelected--;
        }
        m_Selected[ m_LastSelected ] = false;
    }
}

void PtCloudGeom::SelectLastSel()
{
    if ( m_LastSelected >= 0 && m_LastSelected < m_Selected.size() )
    {
        if ( !m_Selected[ m_LastSelected ] )
        {
            m_NumSelected++;
        }
        m_Selected[ m_LastSelected ] = true;
    }
}

void PtCloudGeom::SelectAllShown()
{
    m_NumSelected = 0;

    for ( int j = 0; j < ( int ) m_Pts.size(); j++ )
    {
        if ( !m_Hidden[j] )
        {
            m_Selected[j] = true;
            m_NumSelected++;
        }
    }
}

void PtCloudGeom::SelectNone()
{
    unsigned int n = m_Pts.size();
    m_Selected.assign( n, false );
    m_NumSelected = 0;
}

void PtCloudGeom::SelectInv()
{
    m_NumSelected = 0;

    for ( int j = 0; j < ( int ) m_Pts.size(); j++ )
    {
        if ( !m_Hidden[j] )
        {
            if ( !m_Selected[j] )
            {
                m_Selected[j] = true;
                m_NumSelected++;
            }
            else
            {
                m_Selected[j] = false;
            }
        }
    }
}

void PtCloudGeom::HideSelection()
{
    for ( int j = 0; j < ( int ) m_Pts.size(); j++ )
    {
        if ( m_Selected[j] )
        {
            m_Hidden[j] = true;
            m_Selected[j] = false;
        }
    }
    SelectNone();
}

void PtCloudGeom::HideUnselected()
{
    for ( int j = 0; j < ( int ) m_Pts.size(); j++ )
    {
        if ( !m_Selected[j] )
        {
            m_Hidden[j] = true;
        }
    }
}

void PtCloudGeom::HideAll()
{
    unsigned int n = m_Pts.size();
    m_Hidden.assign( n, true );
    SelectNone();
}

void PtCloudGeom::HideInv()
{
    for ( int j = 0; j < ( int ) m_Pts.size(); j++ )
    {
        if ( !m_Hidden[j] )
        {
            m_Hidden[j] = true;
        }
        else
        {
            m_Hidden[j] = false;
        }
        m_Selected[j] = false;
    }
    m_NumSelected = 0;
}

void PtCloudGeom::ShowAll()
{
    unsigned int n = m_Pts.size();
    m_Hidden.assign( n, false );
}

void PtCloudGeom::GetSelectedPoints( vector < vec3d > &selpts )
{
    Matrix4d transMat = GetTotalTransMat();
    for ( int i = 0 ; i < ( int )m_Pts.size() ; i++ )
    {
        if ( m_Selected[i] )
        {
            selpts.push_back(transMat.xform( m_Pts[i] ) );
        }
    }
}

void PtCloudGeom::ProjectPts( string geomid, int surfid, int idir )
{
    Matrix4d transMat = GetTotalTransMat();
    Matrix4d invMat = transMat;
    invMat.affineInverse();

    Geom* g = m_Vehicle->FindGeom( geomid );

    if ( g )
    {
        VspSurf *surf = g->GetSurfPtr( surfid );

        for ( int i = 0 ; i < ( int )m_Pts.size() ; i++ )
        {
            vec3d pin = transMat.xform( m_Pts[i] );
            vec3d pout;

            double u, w;
            surf->ProjectPt( pin, idir, u, w, pout );

            m_Pts[i] = invMat.xform( pout );
        }
    }

    m_SurfDirty = true;
    Update();
}
