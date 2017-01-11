//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "StackGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"

using namespace vsp;

//==== Constructor ====//
StackGeom::StackGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "StackGeom";
    m_Type.m_Name = "Stack";
    m_Type.m_Type = STACK_GEOM_TYPE;

    m_Closed = false;

    m_XSecSurf.SetParentContainer( GetID() );

    m_XSecSurf.SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );

    m_OrderPolicy.Init( "OrderPolicy", "Design", this, STACK_FREE, STACK_FREE, NUM_STACK_POLICY - 1 );
    m_OrderPolicy.SetDescript( "XSec ordering policy for stack" );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 17;
    m_TessW.SetMultShift( 8, 1 );

    m_ActiveXSec = 0;

    m_XSecSurf.SetXSecType( XSEC_STACK );

    m_XSecSurf.AddXSec( XS_POINT );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_ELLIPSE );
    m_XSecSurf.AddXSec( XS_POINT );

    int j;
    StackXSec* xs;

    j = 0;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 0.0;
    xs->m_TopLAngle = 45.0;
    xs->m_TopLStrength = 0.75;
    xs->m_RightLAngle = 45.0;
    xs->m_RightLStrength = 0.75;

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 1.0;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 2.0;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 1.0;
    dynamic_cast<EllipseXSec *>( xs->GetXSecCurve() )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = ( StackXSec* ) m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XDelta = 0.5;
    xs->m_TopLAngle = -45.0;
    xs->m_TopLStrength = 0.75;
    xs->m_RightLAngle = -45.0;
    xs->m_RightLStrength = 0.75;

}

//==== Destructor ====//
StackGeom::~StackGeom()
{

}

void StackGeom::ChangeID( string id )
{
    Geom::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Fuselage And Cross Section Placement ====//
void StackGeom::UpdateSurf()
{
    m_TessUVec.clear();

    int nxsec = m_XSecSurf.NumXSec();

    if ( m_OrderPolicy() == STACK_LOOP )
    {
        StackXSec* first_xs = (StackXSec*) m_XSecSurf.FindXSec( 0 );
        StackXSec* last_xs = (StackXSec*) m_XSecSurf.FindXSec( nxsec - 1 );

        if ( first_xs && last_xs )
        {
            if ( last_xs->GetXSecCurve()->GetType() != first_xs->GetXSecCurve()->GetType() )
            {
                m_XSecSurf.ChangeXSecShape( nxsec - 1, first_xs->GetXSecCurve()->GetType() );
                last_xs = (StackXSec*) m_XSecSurf.FindXSec( nxsec - 1 );
            }

            if( last_xs )
            {
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
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            EnforceOrder( xs, i, m_OrderPolicy() );

            bool first = false;
            bool last = false;

            if( i == 0 ) first = true;
            else if( i == (nxsec-1) ) last = true;

            //==== Reset Group Names ====//
            xs->SetGroupDisplaySuffix( i );

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
        StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );
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


void StackGeom::UpdateTesselate( int indx, vector< vector< vec3d > > &pnts, vector< vector< vec3d > > &norms, vector< vector< vec3d > > &uw_pnts, bool degen  )
{
    m_SurfVec[indx].Tesselate( m_TessUVec, m_TessW(), pnts, norms, uw_pnts, m_CapUMinTess(), degen );
}

void StackGeom::UpdateSplitTesselate( int indx, vector< vector< vector< vec3d > > > &pnts, vector< vector< vector< vec3d > > > &norms )
{
    m_SurfVec[indx].SplitTesselate( m_TessUVec, m_TessW(), pnts, norms, m_CapUMinTess() );
}

//==== Compute Rotation Center ====//
void StackGeom::ComputeCenter()
{
    m_Center.set_x( 0.0 );
    m_Center.set_y( 0.0 );
    m_Center.set_z( 0.0 );
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr StackGeom::EncodeXml( xmlNodePtr & node )
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
xmlNodePtr StackGeom::DecodeXml( xmlNodePtr & node )
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
void StackGeom::SetActiveXSecType( int type )
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

//==== Override Geom Cut/Copy/Paste/Insert ====//
void StackGeom::CutXSec( int index )
{
    m_XSecSurf.CutXSec( index );
    SetActiveXSecIndex( GetActiveXSecIndex() );
    m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    Update();
}
void StackGeom::CopyXSec( int index )
{
    m_XSecSurf.CopyXSec( index );
}
void StackGeom::PasteXSec( int index )
{
    m_XSecSurf.PasteXSec( index );
    m_XSecSurf.FindXSec( index )->SetLateUpdateFlag( true );
    Update();
}
void StackGeom::InsertXSec( int index, int type )
{
    SetActiveXSecIndex( index );
    InsertXSec( type );
}


//==== Cut Active XSec ====//
void StackGeom::CutActiveXSec()
{
    CutXSec( m_ActiveXSec );
    //m_XSecSurf.CutXSec( m_ActiveXSec );
    //SetActiveXSecIndex( GetActiveXSecIndex() );
    //m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    //Update();
}

//==== Copy Active XSec ====//
void StackGeom::CopyActiveXSec()
{
    CopyXSec( m_ActiveXSec );
//    m_XSecSurf.CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void StackGeom::PasteActiveXSec()
{
    PasteXSec( m_ActiveXSec );
    //m_XSecSurf.PasteXSec( m_ActiveXSec );
    //m_XSecSurf.FindXSec( m_ActiveXSec )->SetLateUpdateFlag( true );
    //Update();
}

//==== Insert XSec ====//
void StackGeom::InsertXSec( )
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
void StackGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    StackXSec* xs = ( StackXSec* ) GetXSec( m_ActiveXSec );

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    StackXSec* inserted_xs = ( StackXSec* ) GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->CopyFrom( xs );

        if ( fabs(inserted_xs->m_XDelta()) < 0.0000001 )
            inserted_xs->m_XDelta = 1.0;

        inserted_xs->SetLateUpdateFlag( true );
    }
    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void StackGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    Geom::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void StackGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale();
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

void StackGeom::AddDefaultSources( double base_len )
{
    switch ( m_OrderPolicy() )
    {
    case STACK_FREE:
    {

        StackXSec* lastxs = (StackXSec*) m_XSecSurf.FindXSec( m_XSecSurf.NumXSec() - 1);
        if( lastxs )
        {
            Matrix4d prevxform;
            prevxform.loadIdentity();

            prevxform.matMult( lastxs->GetTransform()->data() );

            prevxform.affineInverse();
            vec3d offset = prevxform.xform( vec3d( 0.0, 0.0, 0.0 ) );

            double len = offset.mag();

            AddDefaultSourcesXSec( base_len, len, 0 );
            AddDefaultSourcesXSec( base_len, len, m_XSecSurf.NumXSec() - 1 );
        }


        break;
    }
    case STACK_LOOP:
    {
        int iback = -1;
        double dfront = -1.0;

        for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
        {
            StackXSec* xs = ( StackXSec* ) m_XSecSurf.FindXSec( i );
            if ( xs )
            {
                Matrix4d prevxform;
                prevxform.loadIdentity();

                prevxform.matMult( xs->GetTransform()->data() );

                prevxform.affineInverse();
                vec3d offset = prevxform.xform( vec3d( 0.0, 0.0, 0.0 ) );

                double len = offset.mag();

                if ( len > dfront )
                {
                    dfront = len;
                    iback = i;
                }

            }
        }

        AddDefaultSourcesXSec( base_len, dfront, 0 );
        if ( iback >= 0 )
        {
            AddDefaultSourcesXSec( base_len, dfront, iback );
        }

        break;
    }
    }
}

//==== Drag Parameters ====//
void StackGeom::LoadDragFactors( DragFactors& drag_factors )
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

//    drag_factors.m_Length = m_Length();
    drag_factors.m_MaxXSecArea = max_xsec_area;
//    drag_factors.m_LengthToDia = m_Length() / dia;
}

bool StackGeom::IsClosed() const
{
    return m_Closed;
}


void StackGeom::EnforceOrder( StackXSec* xs, int indx, int policy )
{
    int nxsec = m_XSecSurf.NumXSec();

    bool first = false;
    bool last = false;
    bool nextlast = false;

    if( indx == 0 ) first = true;
    else if( indx == (nxsec-1) ) last = true;
    else if( indx == (nxsec-2) ) nextlast = true;

    // STACK_FREE implicit.
    if ( first )
    {
        xs->m_XDelta.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_YDelta.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_ZDelta.SetLowerUpperLimits( 0.0, 0.0 );

        xs->m_XRotate.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_YRotate.SetLowerUpperLimits( 0.0, 0.0 );
        xs->m_ZRotate.SetLowerUpperLimits( 0.0, 0.0 );
    }
    else
    {
        xs->m_XDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );
        xs->m_YDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );
        xs->m_ZDelta.SetLowerUpperLimits( -1.0e12, 1.0e12 );

        xs->m_XRotate.SetLowerUpperLimits( -180.0, 180.0 );
        xs->m_YRotate.SetLowerUpperLimits( -180.0, 180.0 );
        xs->m_ZRotate.SetLowerUpperLimits( -180.0, 180.0 );
    }

    if( policy == STACK_LOOP )
    {
        if ( last )
        {
            StackXSec* prevxs = (StackXSec*) m_XSecSurf.FindXSec( indx - 1);
            if( prevxs )
            {
                Matrix4d prevxform;
                prevxform.loadIdentity();

                prevxform.matMult( prevxs->GetTransform()->data() );

                prevxform.affineInverse();
                vec3d offset = prevxform.xform( vec3d( 0.0, 0.0, 0.0 ) );

                xs->m_XDelta.SetLowerUpperLimits( offset.x(), offset.x() );
                xs->m_YDelta.SetLowerUpperLimits( offset.y(), offset.y() );
                xs->m_ZDelta.SetLowerUpperLimits( offset.z(), offset.z() );

                xs->m_XDelta.Set( offset.x() );
                xs->m_YDelta.Set( offset.y() );
                xs->m_ZDelta.Set( offset.z() );

                vec3d angle = prevxform.getAngles();
                xs->m_XRotate.SetLowerUpperLimits( angle.x(), angle.x() );
                xs->m_YRotate.SetLowerUpperLimits( angle.y(), angle.y() );
                xs->m_ZRotate.SetLowerUpperLimits( angle.z(), angle.z() );

                xs->m_XRotate.Set( angle.x() );
                xs->m_YRotate.Set( angle.y() );
                xs->m_ZRotate.Set( angle.z() );
            }
        }
    }
}
