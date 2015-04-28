//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VSPAEROMgr.cpp: VSPAERO Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "VSPAEROMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "StlHelper.h"
#include "APIDefines.h"
#include "WingGeom.h"

//==== Constructor ====//
VSPAEROMgrSingleton::VSPAEROMgrSingleton()
{
    m_DegenGeomSet.Init( "GeomSet", "VSPAERO", this, 0, 0, 12 );
    m_DegenGeomSet.SetDescript( "Geometry set" );

    m_Sref.Init( "Sref", "VSPAERO", this, 100.0, 0.0, 1e12, false );
    m_Sref.SetDescript( "Reference area" );

    m_bref.Init( "bref", "VSPAERO", this, 1.0, 0.0, 1e6, false );
    m_bref.SetDescript( "Reference span" );

    m_cref.Init( "cref", "VSPAERO", this, 1.0, 0.0, 1e6, false );
    m_cref.SetDescript( "Reference chord" );

    m_RefFlag.Init( "RefFlag", "VSPAERO", this, MANUAL_REF, MANUAL_REF, COMPONENT_REF, false );
    m_RefFlag.SetDescript( "Reference quantity flag" );

    m_CGGeomSet.Init( "MassSet", "VSPAERO", this, 0, 0, 12 );
    m_CGGeomSet.SetDescript( "Mass property set" );

    m_NumMassSlice.Init( "NumMassSlice", "VSPAERO", this, 10, 10, 200 );
    m_NumMassSlice.SetDescript( "Number of mass property slices" );

    m_Xcg.Init( "Xcg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Xcg.SetDescript( "X Center of Gravity" );

    m_Ycg.Init( "Ycg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Ycg.SetDescript( "Y Center of Gravity" );

    m_Zcg.Init( "Zcg", "VSPAERO", this, 0.0, -1.0e12, 1.0e12 );
    m_Zcg.SetDescript( "Z Center of Gravity" );

    m_Alpha.Init( "Alpha", "VSPAERO", this, 5.0, -180, 180 );
    m_Alpha.SetDescript( "Angle of attack" );

    m_Beta.Init( "Beta", "VSPAERO", this, 0.0, -180, 180 );
    m_Beta.SetDescript( "Angle of sideslip" );

    m_Mach.Init( "Mach", "VSPAERO", this, 0.3, 0.0, 5.0 );
    m_Mach.SetDescript( "Freestream Mach number" );


    m_NCPU.Init( "NCPU", "VSPAERO", this, 2, 1, 255 );
    m_NCPU.SetDescript( "Number of processors to use" );
}

void VSPAEROMgrSingleton::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        veh->ParmChanged( parm_ptr, Parm::SET );
    }
}


xmlNodePtr VSPAEROMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = xmlNewChild( node, NULL, BAD_CAST"VSPAEROSettings", NULL );

    ParmContainer::EncodeXml( VSPAEROsetnode );

    XmlUtil::AddStringNode( VSPAEROsetnode, "ReferenceGeomID", m_RefGeomID );

    return VSPAEROsetnode;
}

xmlNodePtr VSPAEROMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr VSPAEROsetnode = XmlUtil::GetNode( node, "VSPAEROSettings", 0 );
    if ( VSPAEROsetnode )
    {
        ParmContainer::DecodeXml( VSPAEROsetnode );
        m_RefGeomID   = XmlUtil::FindString( VSPAEROsetnode, "ReferenceGeomID", m_RefGeomID );
    }

    return VSPAEROsetnode;
}


void VSPAEROMgrSingleton::Update()
{
    if( m_RefFlag() == MANUAL_REF )
    {
        m_Sref.Activate();
        m_bref.Activate();
        m_cref.Activate();
    }
    else
    {
        Geom* refgeom = VehicleMgr.GetVehicle()->FindGeom( m_RefGeomID );

        if( refgeom )
        {
            if( refgeom->GetType().m_Type == MS_WING_GEOM_TYPE )
            {
                WingGeom* refwing = (WingGeom*) refgeom;
                m_Sref.Set( refwing->m_TotalArea() );
                m_bref.Set( refwing->m_TotalSpan() );
                m_cref.Set( refwing->m_TotalChord() );

                m_Sref.Deactivate();
                m_bref.Deactivate();
                m_cref.Deactivate();
            }
        }
    }


}
