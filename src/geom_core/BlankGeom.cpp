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
    m_AxisLength.Init( "Axis_Length", "Axis", this, 1.0, 0.0, 1e12 );

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

    m_Axis.clear();
    m_Axis.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        vec3d pt = vec3d( 0.0, 0.0, 0.0 );
        pt.v[i] = m_AxisLength.Get();
        m_Axis[i] = m_ModelMatrix.xform( pt );
    }
}

void BlankGeom::UpdateDrawObj()
{
    m_HighlightDrawObj.m_PntVec.resize(1);
    m_HighlightDrawObj.m_PntVec[0] = m_Origin;
    m_HighlightDrawObj.m_PointSize = 10.0;

    m_FeatureDrawObj_vec.clear();
    m_FeatureDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        m_FeatureDrawObj_vec[i].m_PntVec.push_back( m_Origin );
        m_FeatureDrawObj_vec[i].m_PntVec.push_back( m_Axis[i] );
        m_FeatureDrawObj_vec[i].m_GeomChanged = true;
    }
}

void BlankGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    char str[256];

    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        sprintf(str,"%d",1);
        m_HighlightDrawObj.m_GeomID = m_ID+string(str);
        m_HighlightDrawObj.m_Visible = !m_GuiDraw.GetNoShowFlag();

        // Set Render Destination to Main VSP Window.
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_Type = DrawObj::VSP_POINTS;
        draw_obj_vec.push_back( &m_HighlightDrawObj) ;
    }

    if ( ( m_GuiDraw.GetDispFeatureFlag() && !m_GuiDraw.GetNoShowFlag() ) || m_Vehicle->IsGeomActive( m_ID ))
    {
        for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
        {
            m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            sprintf( str, "_%d", i );
            m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;
            m_FeatureDrawObj_vec[i].m_LineWidth = 2.0;
            m_FeatureDrawObj_vec[i].m_LineColor = vec3d( 0.0, 0.0, 0.0 );
            m_FeatureDrawObj_vec[i].m_Type = DrawObj::VSP_LINE_STRIP;
            draw_obj_vec.push_back( &m_FeatureDrawObj_vec[i] );
        }
    }
}

void BlankGeom::ReadV2File( xmlNodePtr &root )
{
    xmlNodePtr node;

    //===== Read General Parameters =====//
    node = XmlUtil::GetNode( root, "General_Parms", 0 );
    if ( node )
    {
        Geom::ReadV2File( node );
    }

    //===== Read Blank Parameters =====//
    xmlNodePtr blank_node = XmlUtil::GetNode( root, "Blank_Parms", 0 );
    if ( blank_node )
    {
        m_PointMassFlag = XmlUtil::FindInt( blank_node, "PointMassFlag", m_PointMassFlag() );
        m_PointMass = XmlUtil::FindDouble( blank_node, "PointMass", m_PointMass() );
    }
}
