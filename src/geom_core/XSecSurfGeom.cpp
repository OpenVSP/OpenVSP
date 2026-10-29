//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "XSecSurfGeom.h"
#include "APIDefines.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"

using namespace vsp;

//==== Constructor ====//
XSecSurfGeom::XSecSurfGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_VspSurfType = vsp::NORMAL_SURF;
    m_VspSurfCfdType = vsp::CFD_NORMAL;
    m_ConformalFlag = false;
    m_ConformalOffset = 0;
    m_TessMult = 1;
    m_NextSurf = 0;
}

//==== Destructor ====//
XSecSurfGeom::~XSecSurfGeom()
{
    ClearXSecSurfs();
}

//==== Remove All XSec Surfs =====//
void XSecSurfGeom::ClearXSecSurfs()
{
    //==== Clear XSec Surfs ====//
    DeleteXSecSurfsFrom( 0 );
    m_NextSurf = 0;

    m_VspSurfType = vsp::NORMAL_SURF;
    m_VspSurfCfdType = vsp::CFD_NORMAL;
}

void XSecSurfGeom::DeleteXSecSurfsFrom( int index )
{
    index = std::max( index, 0 );
    for ( int i = index; i < ( int )m_XSecSurfVec.size(); i++ )
    {
        m_NextXSec.erase( m_XSecSurfVec[i] );
        delete m_XSecSurfVec[i];
    }
    if ( index < ( int )m_XSecSurfVec.size() )
    {
        m_XSecSurfVec.resize( index );
    }
}

void XSecSurfGeom::BeginXSecSurfs()
{
    m_NextSurf = 0;
    m_NextXSec.clear();

    m_VspSurfType = vsp::NORMAL_SURF;
    m_VspSurfCfdType = vsp::CFD_NORMAL;
}

void XSecSurfGeom::EndXSecSurfs()
{
    for ( int i = 0; i < m_NextSurf && i < ( int )m_XSecSurfVec.size(); i++ )
    {
        m_XSecSurfVec[i]->DeleteXSecsFrom( m_NextXSec[ m_XSecSurfVec[i] ] );
    }
    DeleteXSecSurfsFrom( m_NextSurf );
}

//==== Get XSecSurf At Index =====//
XSecSurf* XSecSurfGeom::GetXSecSurf( int index )
{
    if ( index < 0 || index >= ( int )m_XSecSurfVec.size() )
    {
        return nullptr;
    }
    return m_XSecSurfVec[index];
}

//==== Skin XSec Surfs ====//
void XSecSurfGeom::SkinXSecSurf( bool closed_flag )
{
    if ( m_ConformalFlag )
    {
        ApplyConformalOffset( m_ConformalOffset );
        m_ConformalFlag = false;
    }


    m_MainSurfVec.resize( m_XSecSurfVec.size() );
    assert( m_XSecSurfVec.size() == m_MainSurfVec.size() );

    for ( int i = 0 ; i < ( int )m_XSecSurfVec.size() ; i++ )
    {
        // A surface typed a wing before it is skinned is a wing's: its sections are closed and split at their
        // trailing and leading edges as a Wing's are, and its parameter across them is a wing's.
        bool wing = GetVspSurfType( i ) == vsp::WING_SURF;
        for ( int j = 0 ; j < m_XSecSurfVec[i]->NumXSec() ; j++ )
        {
            XSec* xs = m_XSecSurfVec[i]->FindXSec( j );
            if ( xs && xs->GetXSecCurve() )
            {
                xs->GetXSecCurve()->SetForceWingType( wing );
                xs->GetXSecCurve()->SetLateUpdateFlag( true );
                xs->SetLateUpdateFlag( true );
            }
        }

        vector< BareXSec* > xsec_vec;
        for ( int j = 0 ; j < m_XSecSurfVec[i]->NumXSec() ; j++ )
        {
            BareXSec* xs = dynamic_cast<BareXSec*>( m_XSecSurfVec[i]->FindXSec( j ) );
            if ( xs )
            {
                xsec_vec.push_back( xs );
            }
        }
        //===== Make Sure Last XS is Exact ====//
        if ( xsec_vec.size() > 2 && closed_flag )
        {
            xsec_vec[ xsec_vec.size()-1] = xsec_vec[0];
        }

        //==== Cross Section Curves & joint info ====//
        vector< rib_data_type > rib_vec;

        //==== Update XSec Location/Rotation ====//
       for ( int j = 0 ; j < (int)xsec_vec.size() ; j++ )
       {
            BareXSec* xs = xsec_vec[j];
            xs->SetLateUpdateFlag( true );

            VspCurve crv = xs->GetCurve();

            //==== Load Ribs ====//
            // GetRib answers with the Top station's conditions and applies them right around
            // the section.  A BareXSec is a SkinXSec, so it carries spines and per side
            // conditions like any other, but this path does not build the per group rib sets
            // that Fuselage and Stack blend -- so a CustomGeom whose sides disagree, or which
            // has a spine, silently gets Top's skinning everywhere.  Wiring it to
            // PrepSkinRibs and SkinRibsBlended is the fix; until then this is the limit, and
            // it is a limit rather than a bug in GetRib.
            if ( j == 0 )
                rib_vec.push_back( xs->GetRib( true, false ) );
            else if ( j == m_XSecSurfVec[i]->NumXSec() -1 )
                rib_vec.push_back( xs->GetRib( false, true ) );
            else
                rib_vec.push_back( xs->GetRib( false, false ) );
       }

        if ( xsec_vec.size() >= 2 )
        {
            m_MainSurfVec[i].SkinRibsUniform( rib_vec, false );
            m_MainSurfVec[i].SetMagicVParm( wing );
        }

        vec3d axis, up;
        m_XSecSurfVec[i]->GetBasicFrame( axis, up );
        m_MainSurfVec[i].SetCapFrame( axis, up );

        m_MainSurfVec[i].ClearCamber();
    }
}

//==== Make A Copy Of MainSurf at Index and Apply XForm ====//
void XSecSurfGeom::CloneSurf( int index, Matrix4d & mat )
{
    if ( index >= 0 && index < (int)m_MainSurfVec.size() )
    {
        VspSurf clone = m_MainSurfVec[index];
        clone.Transform( mat );
        clone.SetClone( index, mat );
        m_MainSurfVec.push_back( clone );
    }
}

//==== Apply Transformation for a Main Surface ====//
void XSecSurfGeom::TransformSurf( int index, Matrix4d & mat )
{
    if ( index >= 0 && index < (int)m_MainSurfVec.size() )
    {
        m_MainSurfVec[index].Transform( mat );
    }
}

void XSecSurfGeom::UpdateFlags()
{
    for ( int i = 0 ; i < (int)m_MainSurfVec.size() ; i++ )
    {
        m_MainSurfVec[i].SetSurfType( GetVspSurfType( i ) );
        m_MainSurfVec[i].SetSurfCfdType( m_VspSurfCfdType );

        if ( m_NegativeVolumeFlag.Get() && m_VspSurfCfdType != vsp::CFD_TRANSPARENT )
        {
            m_MainSurfVec[i].SetSurfCfdType( vsp::CFD_NEGATIVE );
        }
    }
}

//==== Trigger Conformal XSec Offset =====//
void XSecSurfGeom::OffsetXSecs( double off )
{
    m_ConformalFlag = true;
    m_ConformalOffset = off;
    m_SurfDirty = true;
}

//==== Apply Conformal Offset ====//
void XSecSurfGeom::ApplyConformalOffset( double off )
{

    for ( int i = 0 ; i < ( int )m_XSecSurfVec.size() ; i++ )
    {

        int nxsec = m_XSecSurfVec[i]->NumXSec();
        for ( int j = 0 ; j < nxsec ; j++ )
        {
            XSec* xs = m_XSecSurfVec[i]->FindXSec( j );
            if ( xs )
            {
                XSecCurve* xsc = xs->GetXSecCurve();
                if ( xsc )
                {
                    xsc->OffsetCurve( off );
                }
            }
        }
    }
}

//==== Add an XSecSurf, Answering the Surface ====//
XSecSurf* XSecSurfGeom::NewXSecSurf( int xsec_type )
{
    XSecSurf* xsec_surf = nullptr;
    if ( m_NextSurf < ( int )m_XSecSurfVec.size() && m_XSecSurfVec[m_NextSurf]->GetXSecType() == xsec_type )
    {
        xsec_surf = m_XSecSurfVec[m_NextSurf];
        m_NextSurf++;
    }
    else
    {
        DeleteXSecSurfsFrom( m_NextSurf );

        xsec_surf = new XSecSurf();
        xsec_surf->SetXSecType( xsec_type );
        xsec_surf->SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );
        xsec_surf->SetParentContainer( GetID() );
        m_XSecSurfVec.push_back( xsec_surf );
        m_NextSurf = ( int )m_XSecSurfVec.size();
    }
    m_NextXSec[xsec_surf] = 0;

    return xsec_surf;
}

//==== Append a Cross Section at loc ====//
BareXSec* XSecSurfGeom::AppendXSec( XSecSurf* xsec_surf, int type, const vec3d & loc )
{
    int index = m_NextXSec[xsec_surf];
    m_NextXSec[xsec_surf] = index + 1;

    BareXSec* xs = dynamic_cast< BareXSec* >( xsec_surf->FindXSec( index ) );
    if ( xs && xs->GetXSecCurve() && xs->GetXSecCurve()->GetType() == type )
    {
        xs->Reset();
    }
    else
    {
        xsec_surf->DeleteXSecsFrom( index );
        xs = dynamic_cast< BareXSec* >( xsec_surf->FindXSec( xsec_surf->AddXSec( type ) ) );
    }
    if ( xs )
    {
        xs->SetLoc( loc );
        xs->SetContinuity( 0 );
    }
    return xs;
}

void XSecSurfGeom::NeedTessW( int n )
{
    int a = m_TessMult;
    int b = n;
    while ( b != 0 )
    {
        int t = a % b;
        a = b;
        b = t;
    }
    m_TessMult = m_TessMult / a * n;
}

// A linear section at x through the points ( ys[i], zs[i] ), closed back to the first, its points spaced
// evenly in its parameter.
void XSecSurfGeom::AddPolySection( XSecSurf* surf, double x, const vector< double > & ys, const vector< double > & zs )
{
    int n = ( int )ys.size();
    vector< double > u;
    vector< double > rad;
    vector< vec3d > pts;
    for ( int i = 0; i <= n; i++ )
    {
        int j = i % n;
        u.push_back( double( i ) / double( n ) );
        rad.push_back( 0.0 );
        pts.push_back( vec3d( ys[j], zs[j], 0.0 ) );
    }

    BareXSec* xs = AppendXSec( surf, XS_EDIT_CURVE, vec3d( x, 0.0, 0.0 ) );

    EditCurveXSec* edit = dynamic_cast< EditCurveXSec* >( xs->GetXSecCurve() );
    edit->m_SymType.Set( SYM_NONE );
    edit->m_CurveType.Set( LINEAR );
    edit->m_Width.Set( 1.0 );
    edit->m_Height.Set( 1.0 );
    edit->SetPntVecs( u, pts, rad );
}

BareXSec* XSecSurfGeom::AppendPointXSec( XSecSurf* xsec_surf, const vec3d & loc )
{
    return AppendXSec( xsec_surf, XS_POINT, loc );
}

BareXSec* XSecSurfGeom::AppendEllipseXSec( XSecSurf* xsec_surf, const vec3d & loc, double w, double h )
{
    BareXSec* xs = AppendXSec( xsec_surf, XS_ELLIPSE, loc );
    EllipseXSec* ellipse = dynamic_cast< EllipseXSec* >( xs->GetXSecCurve() );
    ellipse->m_Width.Set( w );
    ellipse->m_Height.Set( h );
    return xs;
}

BareXSec* XSecSurfGeom::AppendCircleXSec( XSecSurf* xsec_surf, const vec3d & loc, double d )
{
    BareXSec* xs = AppendXSec( xsec_surf, XS_CIRCLE, loc );
    dynamic_cast< CircleXSec* >( xs->GetXSecCurve() )->m_Diameter.Set( d );
    return xs;
}

BareXSec* XSecSurfGeom::AppendRectXSec( XSecSurf* xsec_surf, const vec3d & loc, double w, double h )
{
    BareXSec* xs = AppendXSec( xsec_surf, XS_ROUNDED_RECTANGLE, loc );
    RoundedRectXSec* rect = dynamic_cast< RoundedRectXSec* >( xs->GetXSecCurve() );
    rect->m_Width.Set( w );
    rect->m_Height.Set( h );
    rect->m_RadiusTR.Set( 0.0 );
    return xs;
}

double XSecSurfGeom::DesignValue( xmlNodePtr & node, const char* name, double def, const char* group )
{
    xmlNodePtr container_node = XmlUtil::GetNode( node, "ParmContainer", 0 );
    if ( container_node )
    {
        xmlNodePtr group_node = XmlUtil::GetNode( container_node, group, 0 );
        if ( group_node )
        {
            xmlNodePtr n = XmlUtil::GetNode( group_node, name, 0 );
            if ( n )
            {
                return XmlUtil::FindDoubleProp( n, "Value", def );
            }
        }
    }
    return def;
}
