//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "BlankGeom.h"
#include "Vehicle.h"


//==== Constructor ====//
BlankGeom::BlankGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_Name = "BlankGeom";
    m_Type.m_Name = "Blank";
    m_Type.m_Type = BLANK_GEOM_TYPE;

    // Point Mass Parms
    m_PointMassFlag.Init( "Point_Mass_Flag", "Mass", this, false, 0, 1 );
    m_PointMass.Init( "Point_Mass", "Mass", this, 0, 0, 1e12 );

    // Disable Parameters that don't make sense for BlankGeom
    m_SymPlanFlag.Deactivate();
    m_SymAxFlag.Deactivate();
    m_SymRotN.Deactivate();
    m_Density.Deactivate();
    m_ShellFlag.Deactivate();
    m_MassArea.Deactivate();
    m_MassPrior.Deactivate();

    Update();
}

//==== Destructor ====//
BlankGeom::~BlankGeom()
{
}

void BlankGeom::UpdateSurf()
{
    if ( m_PointMassFlag.Get() )
    {
        m_PointMass.Activate();
    }
    else
    {
        m_PointMass.Deactivate();
    }

    m_Origin = m_ModelMatrix.xform( vec3d( 0.0, 0.0, 0.0 ) );
}

void BlankGeom::UpdateDrawObj()
{
    m_HighlightDrawObj.m_PntVec.resize(1);
    m_HighlightDrawObj.m_PntVec[0] = vec3d(m_XLoc(), m_YLoc(), m_ZLoc());
    m_HighlightDrawObj.m_PointSize = 10.0;
}

void BlankGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    char str[256];

    if ( !m_Vehicle->IsGeomActive( m_ID ) ) return;
    sprintf(str,"%d",1);
    m_HighlightDrawObj.m_GeomID = m_ID+string(str);
    m_HighlightDrawObj.m_Visible = !m_GuiDraw.GetNoShowFlag();

    // Set Render Destination to Main VSP Window.
    m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_HighlightDrawObj.m_Type = DrawObj::VSP_POINTS;


    draw_obj_vec.push_back( &m_HighlightDrawObj) ;
}
