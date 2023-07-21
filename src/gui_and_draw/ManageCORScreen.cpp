#include "ManageCORScreen.h"
#include "ScreenMgr.h"

#include "CfdMeshScreen.h"
#include "SurfaceIntersectionScreen.h"
#include "StructScreen.h"
#include "StructAssemblyScreen.h"

ManageCORScreen::ManageCORScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_SelectionFlag = false;
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

    if(!m_SelectionFlag)
    {
        return true;
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    std::vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );

    std::vector< DrawObj* > geom_drawObj_vec;
    for(int i = 0; i < (int)geom_vec.size(); i++)
    {
        geom_vec[i]->LoadDrawObjs(geom_drawObj_vec);
    }

    // Load Render Objects from CfdMeshScreen.
    CfdMeshScreen * cfdScreen = dynamic_cast< CfdMeshScreen* >
    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_CFD_MESH_SCREEN ) );
    if( cfdScreen )
    {
        cfdScreen->LoadDrawObjs( geom_drawObj_vec );
    }

    // Load Render Objects from SurfaceIntersectionScreen.
    SurfaceIntersectionScreen * surfScreen = dynamic_cast< SurfaceIntersectionScreen* >
    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_SURFACE_INTERSECTION_SCREEN ) );
    if( surfScreen )
    {
        surfScreen->LoadDrawObjs( geom_drawObj_vec );
    }

    // Load Render Objects from FeaStructScreen.
    StructScreen * structScreen = dynamic_cast< StructScreen* >
    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_STRUCT_SCREEN ) );
    if( structScreen )
    {
        structScreen->LoadDrawObjs( geom_drawObj_vec );
    }

    // Load Render Objects from FeaStructAssemblyScreen.
    StructAssemblyScreen * structAssemblyScreen = dynamic_cast< StructAssemblyScreen* >
    ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_STRUCT_ASSEMBLY_SCREEN ) );
    if( structAssemblyScreen )
    {
        structAssemblyScreen->LoadDrawObjs( geom_drawObj_vec );
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

void ManageCORScreen::EnableSelection()
{
    m_SelectionFlag = true;

    m_ScreenMgr->SetUpdateFlag(true);
}

void ManageCORScreen::Set()
{
    m_SelectionFlag = false;

    m_ScreenMgr->SetUpdateFlag(true);
}

std::string ManageCORScreen::getFeedbackGroupName()
{
    return std::string("CORGUIGroup");
}