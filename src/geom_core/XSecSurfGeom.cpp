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
    for ( int i = 0 ; i < (int)m_XSecSurfVec.size() ; i++ )
    {
        delete m_XSecSurfVec[i];
    }
    m_XSecSurfVec.clear();

    m_VspSurfType = vsp::NORMAL_SURF;
    m_VspSurfCfdType = vsp::CFD_NORMAL;
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
            m_MainSurfVec[i].SetMagicVParm( false );
        }

        vec3d axis, up;
        m_XSecSurfVec[i]->GetBasicFrame( axis, up );
        m_MainSurfVec[i].SetCapFrame( axis, up );
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
    XSecSurf* xsec_surf = new XSecSurf();
    xsec_surf->SetXSecType( xsec_type );
    xsec_surf->SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );
    xsec_surf->SetParentContainer( GetID() );
    m_XSecSurfVec.push_back( xsec_surf );

    return xsec_surf;
}


