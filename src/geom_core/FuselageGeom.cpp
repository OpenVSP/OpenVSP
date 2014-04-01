//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "FuselageGeom.h"
#include "ParmMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "StlHelper.h"


//==== Constructor ====//
FuselageGeom::FuselageGeom( Vehicle* vehicle_ptr ) : GeomXSec( vehicle_ptr )
{
    m_Name = "FuselageGeom";
    m_Type.m_Name = "Fuselage";
    m_Type.m_Type = FUSELAGE_GEOM_TYPE;

    m_Closed = false;

    Matrix4d mat;
    double *pm( mat.data() );
    bool center;

    // rotation                   ; translation
    pm[0] = 0;pm[4] = 0;pm[ 8] =-1;pm[12] = 0;
    pm[1] = 1;pm[5] = 0;pm[ 9] = 0;pm[13] = 0;
    pm[2] = 0;pm[6] = 1;pm[10] = 0;pm[14] = 0;
    pm[3] = 0;pm[7] = 0;pm[11] = 0;pm[15] = 0;
    center = true;

    m_XSecSurf.SetTransformation( mat, true );
    m_XSecSurf.SetParentContainer( GetID() );

    //==== Init Parms ====//
    m_TessU = 16;
    m_TessW = 11;

    m_Length.Init( "Length", "Design", this, 30.0, 1.0e-8, 1.0e12 );
    m_Length.SetDescript( "Length of fuselage" );

    m_ActiveXSec = 0;
    m_XSecSurf.AddXSec( XSec::POINT );
    m_XSecSurf.AddXSec( XSec::ELLIPSE );
    m_XSecSurf.AddXSec( XSec::ELLIPSE );
    m_XSecSurf.AddXSec( XSec::ELLIPSE );
    m_XSecSurf.AddXSec( XSec::POINT );

    VspJointInfo joint;
    double angle, strength, curvature;
    int i, j;
    XSec* xs;

    j = 0;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.0;

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.25;
    dynamic_cast<EllipseXSec *>( xs )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.5;
    dynamic_cast<EllipseXSec *>( xs )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 0.75;
    dynamic_cast<EllipseXSec *>( xs )->SetWidthHeight( 3.0, 2.5 );

    ++j;
    xs = m_XSecSurf.FindXSec( j );
    xs->SetGroupDisplaySuffix( j );
    xs->m_XLocPercent = 1.0;

}

//==== Destructor ====//
FuselageGeom::~FuselageGeom()
{

}

void FuselageGeom::ChangeID( string id )
{
    ParmContainer::ChangeID( id );
    m_XSecSurf.SetParentContainer( GetID() );
}

//==== Update Fuselage And Cross Section Placement ====//
void FuselageGeom::UpdateSurf()
{
    double len = m_Length();

    //==== Cross Section Curves & joint info ====//
    vector< VspCurve > crv_vec;
    crv_vec.resize( m_XSecSurf.NumXSec() );

    //==== Update XSec Location/Rotation ====//
    char str[256];
    for ( int i = 0 ; i < m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );

        if ( xs )
        {
            //==== Reset Group Names ====//
            sprintf( str, "XSec_%d", i );

            //==== Set X Limits ====//
#if 0
            // NOTE: This code breaks cross section insertion and deletion because Update()
            //       gets called when a new cross section is created but before the cross section
            //       has been fully initialized. For example, trace the code through a cross section
            //       insert and see that XSecSurf::AddXSec() needs to call ParmChanged() to let
            //       geometry know that the number of u-curves changed because the cross section
            //       count changed and not the Gen tab changing the count. Also, this logic should
            //       probably be in the cross section code (perhaps XSecSurf) so that when the cross
            //       section x-location is changed this is checked.
            int policy = FUSE_MONOTONIC;
            int duct_ile = 2;  // Only needed for FUSE_DUCT
            EnforceOrder( xs, i, duct_ile, policy );
#endif

            xs->SetRefLength( m_Length() );

            crv_vec[i] =  xs->GetCurve();
        }
    }


    m_SurfVec[0].InterpolateLinear( crv_vec, false );

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

//==== Set Index For Active XSec ====//
void FuselageGeom::SetActiveXSecIndex( int index )
{
    index = Clamp<int>( index, 0, m_XSecSurf.NumXSec() - 1 );

    m_ActiveXSec = index;
}

//==== Get XSec ====//
XSec* FuselageGeom::GetXSec( int index )
{
    return m_XSecSurf.FindXSec( index );
}

//==== Set Active XSec Type ====//
void FuselageGeom::SetActiveXSecType( int type )
{
    XSec* xs = GetXSec( m_ActiveXSec );

    if ( !xs )
    {
        return;
    }

    if ( type == xs->GetType() )
    {
        return;
    }

    m_XSecSurf.ChangeXSecType( m_ActiveXSec, type );

    Update();
}

//==== Cut Active XSec ====//
void FuselageGeom::CutActiveXSec()
{
    m_XSecSurf.CutXSec( m_ActiveXSec );
    Update();
}

//==== Copy Active XSec ====//
void FuselageGeom::CopyActiveXSec()
{
    m_XSecSurf.CopyXSec( m_ActiveXSec );
}

//==== Paste Cut/Copied XSec To Active XSec ====//
void FuselageGeom::PasteActiveXSec()
{
	printf("FuselageGeom::PasteActiveSXec()\n");
    m_XSecSurf.PasteXSec( m_ActiveXSec );
	printf("  Done PasteSXec(), calling ::Update()\n");

    Update();
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
        InsertXSec( xs->GetType() );
    }
}

//==== Insert XSec ====//
void FuselageGeom::InsertXSec( int type )
{
    if ( m_ActiveXSec >= NumXSec() - 1 )
    {
        return;
    }

    XSec* xs = GetXSec( m_ActiveXSec );

    double x_loc_0 = xs->m_XLocPercent();
    double x_loc_1 = GetXSec( m_ActiveXSec + 1 )->m_XLocPercent();

    m_XSecSurf.InsertXSec( type, m_ActiveXSec );
    m_ActiveXSec++;

    XSec* inserted_xs = GetXSec( m_ActiveXSec );

    if ( inserted_xs )
    {
        inserted_xs->CopyFrom( xs );
        GetXSec( m_ActiveXSec )->m_XLocPercent = ( x_loc_0 + x_loc_1 ) * 0.5;
    }

    Update();
}


//==== Look Though All Parms and Load Linkable Ones ===//
void FuselageGeom::AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id )
{
    ParmContainer::AddLinkableParms( linkable_parm_vec );

    m_XSecSurf.AddLinkableParms( linkable_parm_vec, m_ID  );
}

//==== Scale ====//
void FuselageGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale;
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

//==== Drag Parameters ====//
void FuselageGeom::LoadDragFactors( DragFactors& drag_factors )
{
    double max_xsec_area = 0.000000000001;
    for ( int i = 0 ; i < ( int )m_XSecSurf.NumXSec() ; i++ )
    {
        XSec* xs = m_XSecSurf.FindXSec( i );
        double a = xs->ComputeArea( 100 );
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

void FuselageGeom::GetJointParams( int joint, VspJointInfo &jointInfo ) const
{
    m_SurfVec[0].CompJointParams( joint, jointInfo );
}

bool FuselageGeom::IsClosed() const
{
    return m_Closed;
}

void FuselageGeom::EnforceOrder( XSec* xs, int indx, int ile, int policy )
{
    if( policy == FUSE_MONOTONIC )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else
        {
            double lower = m_XSecSurf.FindXSec( indx - 1 )->m_XLocPercent();
            double upper = m_XSecSurf.FindXSec( indx + 1 )->m_XLocPercent();
            xs->m_XLocPercent.SetLowerUpperLimits( lower , upper );
        }
    }
    else if( policy == FUSE_DUCT )
    {
        if ( indx == 0 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else if ( indx ==  m_XSecSurf.NumXSec() - 1 )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 1.0, 1.0 );
        }
        else if ( indx == ile )
        {
            xs->m_XLocPercent.SetLowerUpperLimits( 0.0, 0.0 );
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
