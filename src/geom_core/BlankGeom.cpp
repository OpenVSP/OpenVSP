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
    m_BlankPointMassFlag.Init( "Point_Mass_Flag", "Mass", this, false, 0, 1 );
    m_BlankPointMass.Init( "Point_Mass", "Mass", this, 0, 0, 1e12 );

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
    if ( m_BlankPointMassFlag.Get() )
    {
        m_BlankPointMass.Activate();
    }
    else
    {
        m_BlankPointMass.Deactivate();
    }
}

void BlankGeom::UpdateDrawObj()
{
    double axlen = 1.0;

    Vehicle *veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        axlen = veh->m_AxisLength();
    }

    vec3d blankOrigin = m_ModelMatrix.getTranslation();

    vector < vec3d > blankAxis;
    blankAxis.clear();
    blankAxis.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        vec3d pt = vec3d( 0.0, 0.0, 0.0 );
        pt.v[i] = axlen;
        blankAxis[i] = m_ModelMatrix.xform( pt );
    }

    m_HighlightDrawObj.m_PntVec.resize(1);
    m_HighlightDrawObj.m_PntVec[0] = blankOrigin;
    m_HighlightDrawObj.m_PointSize = 10.0;

    m_FeatureDrawObj_vec.clear();
    m_FeatureDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        m_FeatureDrawObj_vec[i].m_PntVec.push_back( blankOrigin );
        m_FeatureDrawObj_vec[i].m_PntVec.push_back( blankAxis[i] );
        vec3d c;
        c.v[i] = 1.0;
        m_FeatureDrawObj_vec[i].m_LineColor = c;
        m_FeatureDrawObj_vec[i].m_GeomChanged = true;
    }

    //=== Attach Axis ===//
    m_AxisDrawObj_vec.clear();
    m_AxisDrawObj_vec.resize( 3 );
    for ( int i = 0; i < 3; i++ )
    {
        MakeDashedLine( m_AttachOrigin,  m_AttachAxis[i], 4, m_AxisDrawObj_vec[i].m_PntVec );
        vec3d c;
        c.v[i] = 1.0;
        m_AxisDrawObj_vec[i].m_LineColor = c;
        m_AxisDrawObj_vec[i].m_GeomChanged = true;
    }
}

void BlankGeom::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    char str[256];

    if ( m_Vehicle->IsGeomActive( m_ID ) )
    {
        snprintf( str, sizeof( str ), "%d",1);
        m_HighlightDrawObj.m_GeomID = m_ID+string(str);
        m_HighlightDrawObj.m_Visible = GetSetFlag( vsp::SET_SHOWN );

        // Set Render Destination to Main VSP Window.
        m_HighlightDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
        m_HighlightDrawObj.m_Type = DrawObj::VSP_POINTS;
        draw_obj_vec.push_back( &m_HighlightDrawObj) ;


        for ( int i = 0; i < m_AxisDrawObj_vec.size(); i++ )
        {
            m_AxisDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            snprintf( str, sizeof( str ),  "_%d", i );
            m_AxisDrawObj_vec[i].m_GeomID = m_ID + "Axis_" + str;
            m_AxisDrawObj_vec[i].m_LineWidth = 2.0;
            m_AxisDrawObj_vec[i].m_Type = DrawObj::VSP_LINES;
            draw_obj_vec.push_back( &m_AxisDrawObj_vec[i] );
        }
    }

    if ( ( m_GuiDraw.GetDispFeatureFlag() && GetSetFlag( vsp::SET_SHOWN ) ) || m_Vehicle->IsGeomActive( m_ID ))
    {
        for ( int i = 0; i < m_FeatureDrawObj_vec.size(); i++ )
        {
            m_FeatureDrawObj_vec[i].m_Screen = DrawObj::VSP_MAIN_SCREEN;
            snprintf( str, sizeof( str ),  "_%d", i );
            m_FeatureDrawObj_vec[i].m_GeomID = m_ID + "Feature_" + str;
            m_FeatureDrawObj_vec[i].m_LineWidth = 2.0;
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
        m_BlankPointMassFlag = XmlUtil::FindInt( blank_node, "PointMassFlag", m_BlankPointMassFlag() );
        m_BlankPointMass = XmlUtil::FindDouble( blank_node, "PointMass", m_BlankPointMass() );
    }
}
