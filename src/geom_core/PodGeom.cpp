//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "PodGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspCurve.h"
#include "VspSurf.h"
#include "Vehicle.h"

//==== Constructor ====//
PodGeom::PodGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "PodGeom";
    m_Type.m_Name = "Pod";
    m_Type.m_Type = POD_GEOM_TYPE;

    //==== Init Parms ====//
    m_TessU = 10;
    m_TessW = 8;

    m_Length.Init( "Length", "Design", this, 10.0, 1.0e-8, 1.0e12 );
    m_Length.SetDescript( "Length of pod" );
    m_FineRatio.Init( "FineRatio", "Design", this, 15.0, 1.0, 1000.0 );
    m_FineRatio.SetDescript( "The ratio of the length to diameter" );

    Update();
}

//==== Destructor ====//
PodGeom::~PodGeom()
{

}

void PodGeom::UpdateSurf()
{
    double len = m_Length();
    double dia = len / m_FineRatio();

    //==== Build Body/Stringer Curve ====//
    VspCurve stringer;
    curve_segment_type cs( 3 );
    curve_segment_type::control_point_type cp;

    cp << 0.0,      0.0, 0.0;
    cs.set_control_point( cp, 0 );
    cp << 0.05 * len, 0.0, dia * 0.95;
    cs.set_control_point( cp, 1 );
    cp << 0.20 * len, 0.0, dia;
    cs.set_control_point( cp, 2 );
    cp << 0.50 * len, 0.0, dia;
    cs.set_control_point( cp, 3 );
    stringer.AppendCurveSegment( cs );

    cp << 0.50 * len, 0.0, dia;
    cs.set_control_point( cp, 0 );
    cp << 0.60 * len, 0.0, dia;
    cs.set_control_point( cp, 1 );
    cp << 0.95 * len, 0.0, dia * 0.3;
    cs.set_control_point( cp, 2 );
    cp << len,      0.0, 0.0;
    cs.set_control_point( cp, 3 );
    stringer.AppendCurveSegment( cs );

    m_SurfVec[0].CreateBodyRevolution( stringer );
}

//==== Compute Rotation Center ====//
void PodGeom::ComputeCenter()
{
    m_Center.set_x( m_Length()*m_Origin() );
}

//==== Scale ====//
void PodGeom::Scale()
{
    double currentScale = m_Scale() / m_LastScale;
    m_Length *= currentScale;
    m_LastScale = m_Scale();
}
