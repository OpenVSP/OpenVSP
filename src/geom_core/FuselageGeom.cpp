//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "FuselageGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"

using namespace vsp;

//==== Constructor ====//
FuselageGeom::FuselageGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "FuselageGeom";
    m_Type.m_Name = "Fuselage";
    m_Type.m_Type = FUSELAGE_GEOM_TYPE;

    m_XSecSurf.SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    m_XSecSurf.SetParentContainer( GetID() );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 17;
    m_TessW.SetMultShift( 8, 1 );

    m_Length.Init( "Length", "Design", this, 30.0, 1.0e-8, 1.0e12 );
    m_Length.SetDescript( "Length of fuselage" );

    m_OrderPolicy.Init( "OrderPolicy", "Design", this, FUSE_MONOTONIC, FUSE_MONOTONIC, NUM_FUSE_POLICY - 1 );
    m_OrderPolicy.SetDescript( "XSec ordering policy for fuselage" );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_FUSE );

    m_XSecSurf.AddXSec( XS_POINT );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_POINT );

    int j;
    FuseXSec* xs;

    j = 0;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.0;
    xs->m_TopLAngle = 45.0;
    xs->m_TopLStrength = 0.75;
    xs->m_RightLAngle = 45.0;
    xs->m_RightLStrength = 0.75;

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.25;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 2.5, 3.0 );

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.5;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 2.5, 3.0 );

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.75;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 2.5, 3.0 );

    ++j;
    xs = ( FuseXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 1.0;
    xs->m_TopLAngle = -45.0;
    xs->m_TopLStrength = 0.75;
    xs->m_RightLAngle = -45.0;
    xs->m_RightLStrength = 0.75;

}

//==== Destructor ====//
FuselageGeom::~FuselageGeom()
{

}

void FuselageGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Fuselage And Cross Section Placement ====//
void FuselageGeom::UpdateSurf()
{
    m_TessUVec.clear();

    int nxsec = m_XSecSurf.NumXSec();

    if ( m_OrderPolicy() == FUSE_LOOP )
    {
        FuseXSec* first_xs = (FuseXSec*) m_XSecSurf.FindXSec( 0 );
        FuseXSec* last_xs = (FuseXSec*) m_XSecSurf.FindXSec( nxsec - 1 );

        if ( first_xs && last_xs )
        {
            if ( last_xs->GetXSecCurve()->GetType() != first_xs->GetXSecCurve()->GetType() )
            {
                m_XSecSurf.ChangeXSecShape( nxsec - 1, first_xs->GetXSecCurve()->GetType() );
                last_xs = (FuseXSec*) m_XSecSurf.FindXSec( nxsec - 1 );
            }

            if( last_xs )
            {
                last_xs->CopyFuseXSParms( first_xs );
                last_xs->GetXSecCurve()->CopyFrom( first_xs->GetXSecCurve() );
            }
        }
    }

    //==== Cross Section Curves & joint info ====//
    vector< rib_data_type > rib_vec;
    rib_vec.resize( nxsec );

    //==== Update XSec Location/Rotation ====//
    for ( int i = 0 ; i < nxsec ; i++ )
    {
        FuseXSec* xs = ( FuseXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            //==== Reset Group Names ====//
            xs->SetGroupDisplaySuffix( i );

            //==== Set X Limits ====//
            EnforceOrder( xs, i, m_OrderPolicy() );

            xs->SetRefLength( m_Length() );

            bool first = false;
            bool last = false;

            if( i == 0 ) first = true;
            else if( i == (nxsec-1) ) last = true;

            rib_vec[i] = xs->GetRib( first, last );

            if ( i > 0 )
            {
                m_TessUVec.push_back( xs->m_SectTessU() );
            }
        }
    }

    m_MainSurfVec[0].SkinRibs( rib_vec, false );
    m_MainSurfVec[0].SetMagicVParm( false );

    for ( int i = 0 ; i < nxsec ; i++ )
    {
        FuseXSec* xs = ( FuseXSec* ) m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetUnsetParms( i, m_MainSurfVec[0] );
        }
    }

    if ( m_XSecSurf.GetFlipUD() )
    {
        m_MainSurfVec[0].FlipNormal();
    }
}

void FuselageGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen )
{
    m_SurfVec[indx].Tesselate( m_TessUVec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), degen );
}

void FuselageGeom::UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms )
{
    m_SurfVec[indx].SplitTesselate( m_TessUVec, m_TessW(), pnts, norms, m_CapUMinTess() );
}

//==== Compute Rotation Center ====//
void FuselageGeom::ComputeCenter()
{
    m_Center.set_x( m_Length()*m_Origin() );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr FuselageGeom::EncodeXml( xmlNodePtr & node )
{
    Geom::EncodeXml( node );
    xmlNodePtr fuselage_node = xmlNewChild( node, NULL, BAD_CAST "FuselageGeom", NULL );
    if ( fuselage_node )
    {
        m_XSecSurf.EncodeXml( fuselage_node );
    }
    return fuselage_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr FuselageGeom::DecodeXml( xmlNodePtr & node )
{
    Geom::DecodeXml( node );

    xmlNodePtr fuselage_node = XmlUtil::GetNode( node, "FuselageGeom", 0 );
    if ( fuselage_node )
    {
        m_XSecSurf.DecodeXml( fuselage_node );
    }

    return fuselage_node;
}

//==== Set Active XSec Type ====//
void FuselageGeom::SetActiveXSecType( int type )
{
    XSec* xs = GetXSec( m_ActiveXSec );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetXSecCurve()->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecShape( m_ActiveXSec, type );

    Update();
}

//==== Override Geom Cut/Copy/Insert/Paste ====//
void FuselageGeom::CutXSec( int index )
{
    m_XSecSurf.CutXSec( index );
    SetActiveXSecIndex( GetActiveXSecIndex() );
    Update();
}
void FuselageGeom::CopyXSec( int index )
{
    m_XSecSurf.CopyXSec( index );
}
void FuselageGeom::PasteXSec( int index )
{
    m_XSecSurf.PasteXSec( index );
    Update();
}
void FuselageGeom::InsertXSec( int index, int type )
{
    SetActiveXSecIndex( index );
    InsertXSec( type );
}

//==== Cut Active XSec ====//
void FuselageGeom::CutActiveXSec()
{
    CutXSec( m_ActiveXSec );
}

//==== Copy Active XSec ====//
void FuselageGeom::CopyActiveXSec()
{
    CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void FuselageGeom::PasteActiveXSec()
{
    PasteXSec( m_ActiveXSec );
}

//==== Insert XSec ====//
void FuselageGeom::InsertXSec( )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec );
    if ( xs )
    {
        InsertXSec( xs->GetXSecCurve()->GetType() );
    }
}

//==== Insert XSec ====//
void FuselageGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    FuseXSec* xs = ( FuseXSec* ) GetXSec( m_ActiveXSec );
    FuseXSec* xs_1 = ( FuseXSec* ) GetXSec( m_ActiveXSec + 1 );

    double x_loc_0 = xs->m_XLocPercent();
    double x_loc_1 = xs_1->m_XLocPercent();

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    FuseXSec* inserted_xs = ( FuseXSec* ) GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->SetRefLength( m_Length() );
        inserted_xs->CopyFrom( xs );
        inserted_xs->m_XLocPercent = ( x_loc_0 + x_loc_1 ) * 0.5;
    }

    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void FuselageGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void FuselageGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
    m_Length *= currentScale;
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        if ( xs )
        {
            xs->SetScale( currentScale );
        }
    }

    m_LastScale = m_Scale();
}

void FuselageGeom::AddDefaultSources( double base_len )
{

    switch ( m_OrderPolicy() )
    {
    case FUSE_MONOTONIC:
    {
        AddDefaultSourcesXSec( base_len, m_Length(), 0 );
        AddDefaultSourcesXSec( base_len, m_Length(), m_XSecSurf.NumXSec() - 1 );

        break;
    }
    case FUSE_LOOP:
    {
        int ifront = -1;
        double xfront = 1.0;

        for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
        {
            FuseXSec* xs = ( FuseXSec* ) m_XSecSurf.FindXSec( i );
            if ( xs )
            {
                if ( xs->m_XLocPercent() < xfront )
                {
                    ifront = i;
                    xfront = xs->m_XLocPercent();
                }
            }
        }

        AddDefaultSourcesXSec( base_len, m_Length(), 0 );
        if ( ifront >= 0 )
        {
            AddDefaultSourcesXSec( base_len, m_Length(), ifront );
        }

        break;
    }
    case FUSE_FREE:
    {
        AddDefaultSourcesXSec( base_len, m_Length(), 0 );
        AddDefaultSourcesXSec( base_len, m_Length(), m_XSecSurf.NumXSec() - 1 );

        break;
    }
    }
}

//==== Drag Parameters ====//
void FuselageGeom::LoadDragFactors( DragFactors& drag_factors )
{
    double max_xsec_area = 0.000000000001;
    for ( int i = 0 ; i < ( int )m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        XSecCurve* xsc = xs->GetXSecCurve();
        double a = xsc->ComputeArea( );
        if ( a > max_xsec_area )
        {
            max_xsec_area = a;
        }
    }

    double dia = 2.0 * sqrt( max_xsec_area / PI );

    drag_factors.m_Length = m_Length();
    drag_factors.m_MaxXSecArea = max_xsec_area;
    drag_factors.m_LengthToDia = m_Length() / dia;
}

void FuselageGeom::EnforceOrder( FuseXSec* xs, int indx, int policy )
{
    if( policy == FUSE_MONOTONIC )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
            xs->m_XLocPercent.Set( 0.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
            xs->m_XLocPercent.Set( 1.0 );
        }
        else
        {
            FuseXSec* priorxs = ( FuseXSec* ) m_XSecSurf.FindXSec( indx - 1 );
            FuseXSec* nextxs = ( FuseXSec* ) m_XSecSurf.FindXSec( indx + 1 );
            double lower = priorxs->m_XLocPercent();
            double upper = nextxs->m_XLocPercent();
            xs->m_XLocPercent.SetLowerUpperLimits( lower , upper );
        }
    }
    else if( policy == FUSE_LOOP )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
            xs->m_XLocPercent.Set( 1.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
            xs->m_XLocPercent.Set( 1.0 );
        }
        else
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 1.0 );
        }

    }
    else if( policy == FUSE_FREE )
    {
        xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 1.0 );
    }
}

void FuselageGeom::ReadV2File( xmlNodePtr &root )
{
    xmlNodePtr node = XmlUtil::GetNode( root, "Type", 0 );

    if ( node )
    {
        string typeStr = XmlUtil::ExtractString( node );

        if ( typeStr == "Fuselage" )
        {
            ReadV2FileFuse1( root );
        }
        else if ( typeStr == "Fuselage2" )
        {
            ReadV2FileFuse2( root );
        }
    }

}

void FuselageGeom::ReadV2FileFuse2( xmlNodePtr &root )
{
    int i;
    xmlNodePtr node;

    m_XSecSurf.DeleteAllXSecs();

    //===== Read General Parameters =====//
    node = XmlUtil::GetNode( root, "General_Parms", 0 );
    if ( node )
    {
        Geom::ReadV2File( node );
    }

//    int space_type = PNT_SPACE_FIXED;

    //===== Read Fuse Parameters =====//
    node = XmlUtil::GetNode( root, "Fuse_Parms", 0 );
    if ( node )
    {
        m_Length = XmlUtil::FindDouble( node, "Fuse_Length", m_Length() );

//        space_type = XmlUtil::FindInt( node, "Space_Type", space_type );
    }

    node = XmlUtil::GetNode( root, "Cross_Section_List", 0 );
    if ( node  )
    {
        xmlNodePtr xsec_list_node = node;
        int num_xsecs = XmlUtil::GetNumNames( xsec_list_node, "Cross_Section" );

        xmlNodePtr xsec_node = node->xmlChildrenNode;
        while( xsec_node != NULL )
        {
            if ( !xmlStrcmp( xsec_node->name, ( const xmlChar * )"Cross_Section" ) )
            {
                int xstype = XmlUtil::FindInt( xsec_node, "Type", 0 );

                XSec* xsec_ptr = NULL;

                switch ( xstype )
                {
                case V2_FXS_POINT:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_POINT ) );
                    break;
                case V2_FXS_CIRCLE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_CIRCLE ) );
                    break;
                case V2_FXS_ELLIPSE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ELLIPSE ) );
                    break;
                case V2_FXS_BOX:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ROUNDED_RECTANGLE ) );
                    break;
                case V2_FXS_RND_BOX:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ROUNDED_RECTANGLE ) );
                    break;
                case V2_FXS_GENERAL:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_GENERAL_FUSE ) );
                    break;
                case V2_FXS_FROM_FILE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_FILE_FUSE ) );
                    break;
                case V2_FXS_EDIT_CRV:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ELLIPSE ) );

                    break;
                default:

                    break;
                }

                if ( xsec_ptr )
                {
                    FuseXSec* fuse_xsec_ptr = dynamic_cast < FuseXSec* > (xsec_ptr);
                    if ( fuse_xsec_ptr )
                    {
                        fuse_xsec_ptr->SetRefLength( m_Length() );
                    }
                    xsec_ptr->ReadV2FileFuse2( xsec_node );
                }
            }
            xsec_node = xsec_node->next;
        }
    }
}

void FuselageGeom::ReadV2FileFuse1( xmlNodePtr &root )
{
    xmlNodePtr node;

    m_XSecSurf.DeleteAllXSecs();

    //===== Read General Parameters =====//
    node = XmlUtil::GetNode( root, "General_Parms", 0 );
    if ( node )
    {
        Geom::ReadV2File( node );
    }

    double nose_rho = 1.0;
    double aft_rho = 1.0;

    int nose_super_flag = 0;
    int aft_super_flag = 0;

    //===== Read Fuse Parameters =====//
    node = XmlUtil::GetNode( root, "Fuse_Parms", 0 );
    if ( node )
    {
        m_Length = XmlUtil::FindDouble( node, "Fuse_Length", m_Length() );
        nose_rho = XmlUtil::FindDouble( node, "Nose_Rho", nose_rho );
        aft_rho = XmlUtil::FindDouble( node, "Aft_Rho", aft_rho );
        nose_super_flag = XmlUtil::FindInt( node, "Nose_Super_Flag", nose_super_flag );
        aft_super_flag = XmlUtil::FindInt( node, "Aft_Super_Flag", aft_super_flag );

//        camber = XmlUtil::FindDouble( node, "Camber", camber() );
//        camber_loc = XmlUtil::FindDouble( node, "Camber_Location", camber_loc() );
//        aft_offset = XmlUtil::FindDouble( node, "Aft_Offset", aft_offset() );
//        nose_angle = XmlUtil::FindDouble( node, "Nose_Angle", nose_angle());
//        nose_str = XmlUtil::FindDouble( node, "Nose_Strength", nose_str() );
//
//        imlFlag = XmlUtil::FindInt( node, "IML_Flag", imlFlag );
//        space_type = XmlUtil::FindInt( node, "Space_Type", space_type );
//
    }

    node = XmlUtil::GetNode( root, "Cross_Section_List", 0 );
    if ( node  )
    {
        xmlNodePtr xsec_list_node = node;
        int num_xsecs = XmlUtil::GetNumNames( xsec_list_node, "Cross_Section" );

        xmlNodePtr xsec_node = node->xmlChildrenNode;
        while( xsec_node != NULL )
        {
            if ( !xmlStrcmp( xsec_node->name, ( const xmlChar * )"Cross_Section" ) )
            {
                xmlNodePtr omlNode = XmlUtil::GetNode( xsec_node, "OML_Parms", 0 );
                int xstype = XmlUtil::FindInt( omlNode, "Type", -1 );

                XSec* xsec_ptr = NULL;

                switch ( xstype )
                {
                case V2_XSEC_POINT:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_POINT ) );
                    break;
                case V2_CIRCLE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_CIRCLE ) );
                    break;
                case V2_ELLIPSE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ELLIPSE ) );
                    break;
                case V2_RND_BOX:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ROUNDED_RECTANGLE ) );
                    break;
                case V2_GENERAL:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_GENERAL_FUSE ) );
                    break;
                case V2_FROM_FILE:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_FILE_FUSE ) );
                    break;
                case V2_EDIT_CRV:
                    xsec_ptr = m_XSecSurf.FindXSec( m_XSecSurf.AddXSec( XS_ELLIPSE ) );
                    break;
                default:

                    break;
                }

                if ( xsec_ptr )
                {
                    FuseXSec* fuse_xsec_ptr = dynamic_cast < FuseXSec* > (xsec_ptr);
                    if ( fuse_xsec_ptr )
                    {
                        fuse_xsec_ptr->SetRefLength( m_Length() );
                    }
                    xsec_ptr->ReadV2FileFuse1( xsec_node );
                }

            }
            xsec_node = xsec_node->next;
        }
    }

    // Nose and aft rho control the exponent of a superellipse for v2 Fuse1.
    // Duplicating that shape exactly is not practical, so this heuristic will
    // get something somewht close.
    if ( nose_super_flag )
    {
        FuseXSec* fuse_xs = dynamic_cast < FuseXSec* > ( GetXSec( 0 ) );
        if ( fuse_xs )
        {
            double ang;
            if ( nose_rho < 2.0 )
            {
                ang = nose_rho * 45.0;
            }
            else
            {
                ang = 90.0;
            }
            fuse_xs->m_TopLAngle = ang;
        }
    }

    if ( aft_super_flag )
    {
        FuseXSec* fuse_xs = dynamic_cast < FuseXSec* > ( GetXSec( GetXSecSurf( 0 )->NumXSec() - 1 ) );
        if ( fuse_xs )
        {
            double ang;
            if ( aft_rho < 2.0 )
            {
                ang = -aft_rho * 45.0;
            }
            else
            {
                ang = -90.0;
            }
            fuse_xs->m_TopLAngle = ang;
        }
    }

}
