//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "ManageCORScreen.h"
#include "ScreenMgr.h"

#include "CfdMeshScreen.h"
#include "GeometryAnalysisScreen.h"
#include "SurfaceIntersectionScreen.h"
#include "StructScreen.h"
#include "StructAssemblyScreen.h"

ManageCORScreen::ManageCORScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_CORSelectionFlag = false;
    m_VNSelectionFlag = false;
}

ManageCORScreen::~ManageCORScreen()
{
}

void ManageCORScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    VspScreen::Show();
}

bool ManageCORScreen::IsShown()
{
    return true;
}

void ManageCORScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageCORScreen::Update()
{
    VspScreen::Update();

    m_PickList.clear();

    // Update picking list if needed.

    if(!m_CORSelectionFlag && !m_VNSelectionFlag)
    {
        return true;
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    std::vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );

    std::vector< DrawObj* > geom_drawObj_vec;
    for(int i = 0; i < (int)geom_vec.size(); i++)
    {
        if ( m_CORSelectionFlag )
        {
            geom_vec[i]->LoadDrawObjs(geom_drawObj_vec);
        }
        else if ( m_VNSelectionFlag )
        {
            geom_vec[i]->LoadMainDrawObjs(geom_drawObj_vec);
        }
    }

    if ( m_CORSelectionFlag )
    {
        // Load Render Objects from CfdMeshScreen.
        CfdMeshScreen * cfdScreen = dynamic_cast< CfdMeshScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_CFD_MESH_SCREEN ) );
        if( cfdScreen )
        {
            cfdScreen->LoadDrawObjs( geom_drawObj_vec );
        }

        // Load Render Objects from SurfaceIntersectionScreen.
        SurfaceIntersectionScreen * surfScreen = dynamic_cast< SurfaceIntersectionScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_SURFACE_INTERSECTION_SCREEN ) );
        if( surfScreen )
        {
            surfScreen->LoadDrawObjs( geom_drawObj_vec );
        }

        // Load Render Objects from FeaStructScreen.
        StructScreen * structScreen = dynamic_cast< StructScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_SCREEN ) );
        if( structScreen )
        {
            structScreen->LoadDrawObjs( geom_drawObj_vec );
        }

        // Load Render Objects from FeaStructAssemblyScreen.
        StructAssemblyScreen * structAssemblyScreen = dynamic_cast< StructAssemblyScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_STRUCT_ASSEMBLY_SCREEN ) );
        if( structAssemblyScreen )
        {
            structAssemblyScreen->LoadDrawObjs( geom_drawObj_vec );
        }
    }

    // Load Render Objects for Geometry Analysis Screen
    GeometryAnalysisScreen * geometryAnalysisScreen = dynamic_cast < GeometryAnalysisScreen* >
        ( m_ScreenMgr->GetScreen( vsp::VSP_GEOMETRY_ANALYSIS_SCREEN ) );
    if ( geometryAnalysisScreen )
    {
        geometryAnalysisScreen->LoadDrawObjs( geom_drawObj_vec );
    }

    for(int j = 0; j < (int)geom_drawObj_vec.size(); j++)
    {
        if(geom_drawObj_vec[j]->m_Visible)
        {
            // Ignore bounding boxes & pick verts.
            if( geom_drawObj_vec[j]->m_GeomID.compare(0, string(BBOXHEADER).size(), BBOXHEADER) != 0 &&
                geom_drawObj_vec[j]->m_Type != DrawObj::VSP_PICK_VERTEX )
            {
                DrawObj pickDO;
                pickDO.m_Type = DrawObj::VSP_PICK_VERTEX;
                pickDO.m_GeomID = PICKVERTEXHEADER + geom_drawObj_vec[j]->m_GeomID;
                pickDO.m_PickSourceID = geom_drawObj_vec[j]->m_GeomID;
                pickDO.m_FeedbackGroup = getFeedbackGroupName();

                m_PickList.push_back(pickDO);
            }
        }
    }

    return true;
}

void ManageCORScreen::CallBack( Fl_Widget * w )
{
}

void ManageCORScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    for(int i = 0; i < (int)m_PickList.size(); i++)
    {
        draw_obj_vec.push_back(&m_PickList[i]);
    }
}

void ManageCORScreen::EnableCORSelection()
{
    m_CORSelectionFlag = true;
    m_VNSelectionFlag = false;

    m_ScreenMgr->SetUpdateFlag(true);
}

void ManageCORScreen::EnableVNSelection()
{
    m_CORSelectionFlag = false;
    m_VNSelectionFlag = true;

    m_ScreenMgr->SetUpdateFlag(true);
}

void ManageCORScreen::Set()
{
    m_CORSelectionFlag = false;
    m_VNSelectionFlag = false;

    m_ScreenMgr->SetUpdateFlag(true);
}

std::string ManageCORScreen::getFeedbackGroupName()
{
    if ( m_CORSelectionFlag )
    {
        return std::string("CORGUIGroup");
    }
    else if ( m_VNSelectionFlag )
    {
        return std::string("VNGUIGroup");
    }
    return std::string("ERROR");
}