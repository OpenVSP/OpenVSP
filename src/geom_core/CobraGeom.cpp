//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CobraGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspCurve.h"
#include "VspSurf.h"
#include "Vehicle.h"

//==== Constructor ====//
CobraGeom::CobraGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "CobraGeom";
    m_Type.m_Name = "Cobra";
    m_Type.m_Type = COBRA_GEOM_TYPE;

    //==== Init Parms ====//
    m_TessU = 10;
    m_TessW = 8;

    m_XradN.Init( "XradN", "Design", this, 1.0, 0.0, 1.0e12 );
    m_YradN.Init( "YradN", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowX.Init( "PowX", "Design", this, 2.0, 1.0, 10.0 );

    m_ZradNL.Init( "ZradNL", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowNL.Init( "PowNL", "Design", this, 2.0, 1.0, 10.0 );
    m_NoseULSymm.Init( "NoseULSymm", "Design", this, true, false, true );
    m_ZradNU.Init( "ZradNU", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowNU.Init( "PowNU", "Design", this, 2.0, 1.0, 10.0 );

    m_XlenA.Init( "XlenA", "Design", this, 1.0, 0.0, 1.0e12 );
    m_YradA.Init( "YradA", "Design", this, 1.0, 0.0, 1.0e12 );
    m_ZradAL.Init( "ZradAL", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowAL.Init( "PowAL", "Design", this, 2.0, 1.0, 10.0 );
    m_AftULSymm.Init( "AftULSymm", "Design", this, true, false, true );
    m_ZradAU.Init( "ZradAU", "Design", this, 1.0, 0.0, 1.0e12 );
    m_PowAU.Init( "PowAU", "Design", this, 2.0, 1.0, 10.0 );

    m_Xoff = 0.0;
}

//==== Destructor ====//
CobraGeom::~CobraGeom()
{

}

void CobraGeom::UpdateSurf()
{
    double xo = m_Xoff;

}

//==== Compute Rotation Center ====//
void CobraGeom::ComputeCenter()
{
    // m_Center.set_x( m_Length()*m_Origin() );
}

//==== Scale ====//
void CobraGeom::Scale()
{
    // double currentScale = m_Scale() / m_LastScale();
    // m_Length *= currentScale;
    m_LastScale = m_Scale();
}

void CobraGeom::AddDefaultSources( double base_len )
{
    // double len = m_Length();
    // double rad = len / m_FineRatio();
    //
    // PointSource* psource;
    // LineSource* lsource;
    //
    // psource = new PointSource();
    // psource->SetName( "Def_Fwd_PS" );
    // psource->m_Len = 0.05 * rad;
    // psource->m_Rad = 1.0 * rad;
    // psource->m_ULoc = 0.0;
    // psource->m_WLoc = 0.0;
    // psource->m_MainSurfIndx = 0;
    // AddCfdMeshSource( psource );
    //
    // psource = new PointSource();
    // psource->SetName( "Def_Aft_PS" );
    // psource->m_Len = 0.05 * rad;
    // psource->m_Rad = 1.0 * rad;
    // psource->m_ULoc = 1.0;
    // psource->m_WLoc = 0.0;
    // psource->m_MainSurfIndx = 0;
    // AddCfdMeshSource( psource );
    //
    // lsource = new LineSource();
    // lsource->SetName( "Def_Fwd_Aft_LS" );
    // lsource->m_Len = 0.25 * rad;
    // lsource->m_Len2 = 0.25 * rad;
    // lsource->m_Rad = 2.0 * rad;
    // lsource->m_Rad2 = 2.0 * rad;
    // lsource->m_ULoc1 = 0.0;
    // lsource->m_WLoc1 = 0.0;
    // lsource->m_ULoc2 = 1.0;
    // lsource->m_WLoc2 = 0.0;
    // lsource->m_MainSurfIndx = 0;
    // AddCfdMeshSource( lsource );
}

void CobraGeom::OffsetXSecs( double off )
{
    // double len = m_Length();
    // double rad = len / m_FineRatio();
    //
    // // Calculate offset parameters
    // double lenoff = len - 2.0 * off;
    // double radoff = rad - off;
    //
    // // Protect against negative diameter
    // if ( radoff < 0.0 )
    // {
    //     radoff = 0.0;
    // }
    //
    // // Protect against negative length -- and divide by zero
    // if ( lenoff <= 0.0 )
    // {
    //     lenoff = 0.0;
    //     // Don't adjust fineness ratio.
    // }
    // else // Nominal case.
    // {
    //     m_FineRatio = lenoff / radoff;
    // }
    //
    // m_Length = lenoff;
    m_Xoff = off;
}
