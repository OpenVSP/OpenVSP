#include "ManageCORScreen.h"
#include "ScreenMgr.h"

ManageCORScreen::ManageCORScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_SelectionFlag = false;
}

ManageCORScreen::~ManageCORScreen()
{
}

void ManageCORScreen::Show()
{
    Update();
}

bool ManageCORScreen::IsShown()
{
    return true;
}

void ManageCORScreen::Hide()
{
}

bool ManageCORScreen::Update()
{
    m_PickList.clear();

    // Update picking list if needed.

    if(!m_SelectionFlag)
    {
        return true;
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    std::vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec( false ) );

    for(int i = 0; i < (int)geom_vec.size(); i++)
    {
        std::vector< DrawObj* > geom_drawObj_vec;
        geom_vec[i]->LoadDrawObjs(geom_drawObj_vec);

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

    Update();

    m_ScreenMgr->SetUpdateFlag(true);
}

void ManageCORScreen::Set()
{
    m_SelectionFlag = false;

    Update();

    m_ScreenMgr->SetUpdateFlag(true);
}

std::string ManageCORScreen::getFeedbackGroupName()
{
    return std::string("CORGUIGroup");
}