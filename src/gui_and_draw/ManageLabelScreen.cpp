#include "ManageLabelScreen.h"
#include "ScreenMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "LabelMgr.h"

ManageLabelScreen::ManageLabelScreen(ScreenMgr * mgr) : VspScreen(mgr)
{
    m_LabelUI = new LabelUI();
    m_FLTK_Window = m_LabelUI->UIWindow;

    m_RedSlider.Init(this, m_LabelUI->redSlider, 256);
    m_GreenSlider.Init(this, m_LabelUI->greenSlider, 256);
    m_BlueSlider.Init(this, m_LabelUI->blueSlider, 256);

    m_SizeSlider.Init(this, m_LabelUI->textSizeSlider, 50);

    m_LabelUI->addLabelButton->callback(staticCB, this);
    m_LabelUI->removeLabelButton->callback(staticCB, this);
    m_LabelUI->selAllButton->callback(staticCB, this);
    m_LabelUI->labelBrowser->callback(staticCB, this);
    m_LabelUI->nameInput->callback(staticCB, this);
    m_LabelUI->redSlider->callback(staticCB, this);
    m_LabelUI->greenSlider->callback(staticCB, this);
    m_LabelUI->blueSlider->callback(staticCB, this);
    m_LabelUI->textSizeSlider->callback(staticCB, this);

    m_LabelUI->attachRulerButton->callback(staticCB, this);
}
ManageLabelScreen::~ManageLabelScreen()
{
    delete m_LabelUI;
}

void ManageLabelScreen::Show()
{
    if(Update())
    {
        m_FLTK_Window->show();
    }
}

void ManageLabelScreen::Hide()
{
    m_FLTK_Window->hide();
}

bool ManageLabelScreen::Update()
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();
    std::vector<Label*> labels = vPtr->getVGuiDraw()->getLabelMgr()->GetVec();

    m_LabelUI->labelBrowser->clear();

    // Add labels to browser.
    for(int i = 0; i < (int)labels.size(); i++)
    {
        m_LabelUI->labelBrowser->add(labels[i]->GetName().c_str());

        for(int j = 0; j < (int)m_Current.size(); j++)
        {
            if(labels[i]->GetID() == m_Current[j])
            {
                m_LabelUI->labelBrowser->select(i + 1);
            }
        }
    }

    // Ruler / Text Panel.
    if(m_Current.size() == 1)
    {
        Ruler * rulerType = dynamic_cast<Ruler*>(vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[0]));
        if(rulerType)
        {
            m_LabelUI->textUIGroup->hide();
            m_LabelUI->rulerUIGroup->show();
        }
    }
    else
    {
        m_LabelUI->textUIGroup->hide();
        m_LabelUI->rulerUIGroup->hide();
    }

    for(int i = 0; i < (int)m_Current.size(); i++)
    {
        Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[i]);

        m_RedSlider.Update(label->m_Red.GetID());
        m_GreenSlider.Update(label->m_Green.GetID());
        m_BlueSlider.Update(label->m_Blue.GetID());

        m_SizeSlider.Update(label->m_Size.GetID());
    }

    return true;
}

void ManageLabelScreen::CallBack(Fl_Widget * w)
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();

    if ( w == m_LabelUI->addLabelButton )
    {
        m_Current.clear();
        m_Current.push_back(GenerateRuler());

        UpdateNameInput();
    }
    else if ( w == m_LabelUI->removeLabelButton )
    {
        if(!m_Current.empty())
        {
            for(int i = 0; i < (int)m_Current.size(); i++)
            {
                RemoveRuler(m_Current[i]);
            }
            m_Current.clear();
        }
        UpdateNameInput();
    }
    else if ( w == m_LabelUI->selAllButton )
    {
        m_Current.clear();

        for(int i = 0; i < (int)m_LabelList.size(); i++)
        {
            m_Current.push_back(m_LabelList[i].m_GeomID);
        }
        UpdateNameInput();
    }
    else if ( w == m_LabelUI->labelBrowser )
    {
        std::vector<Label*> labels = vPtr->getVGuiDraw()->getLabelMgr()->GetVec();
        assert(labels.size() == m_LabelUI->labelBrowser->size());

        m_Current.clear();

        for(int i = 1; i <= (int)m_LabelUI->labelBrowser->size(); i++)
        {
            if(m_LabelUI->labelBrowser->selected(i))
            {
                m_Current.push_back(labels[i - 1]->GetID());
            }
        }
        UpdateNameInput();
    }
    else if ( w == m_LabelUI->nameInput )
    {
        for(int i = 0; i < (int)m_Current.size(); i++)
        {
            Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[i]);
            assert(label);
            label->SetName(m_LabelUI->nameInput->value());
        }
    }
    else if ( w == m_LabelUI->redSlider )
    {
        for(int i = 0; i < (int)m_Current.size(); i++)
        {
            Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[i]);
            assert(label);
            label->m_Red = m_LabelUI->redSlider->value();
        }
    }
    else if ( w == m_LabelUI->greenSlider )
    {
        for(int i = 0; i < (int)m_Current.size(); i++)
        {
            Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[i]);
            assert(label);
            label->m_Green = m_LabelUI->greenSlider->value();
        }
    }
    else if ( w == m_LabelUI->blueSlider )
    {
        for(int i = 0; i < (int)m_Current.size(); i++)
        {
            Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[i]);
            assert(label);
            label->m_Blue = m_LabelUI->blueSlider->value();
        }
    }
    else if ( w == m_LabelUI->textSizeSlider )
    {
        for(int i = 0; i < (int)m_Current.size(); i++)
        {
            Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[i]);
            assert(label);
            label->m_Size = m_LabelUI->textSizeSlider->value();
        }
    }
    else if ( w == m_LabelUI->attachRulerButton )
    {
        if(m_Current.size() == 1)
        {
            Label * label = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[0]);
            assert(label);
            label->Reset();
        }
    }

    m_ScreenMgr->SetUpdateFlag(true);
}

void ManageLabelScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    UpdateDrawObjs();

    int i;

    for(i = 0; i < (int)m_LabelList.size(); i++)
    {
        // Render completed rulers.
        if(m_LabelList[i].m_Ruler.Step == DrawObj::VSP_RULER_STEP_COMPLETE)
        {
            draw_obj_vec.push_back(&m_LabelList[i]);
        }
        // Render current selected ruler.
        else if(m_Current.size() == 1 && m_LabelList[i].m_GeomID == m_Current[0])
        {
            draw_obj_vec.push_back(&m_LabelList[i]);
        }
    }

    UpdatePickList();

    for(i = 0; i < (int)m_PickList.size(); i++)
    {
        draw_obj_vec.push_back(&m_PickList[i]);
    }
}

void ManageLabelScreen::Set(vec3d placement, std::string targetGeomId)
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if(m_Current.size() == 1)
    {
        vec2d uw;

        Label * currLabel = veh->getVGuiDraw()->getLabelMgr()->Get(m_Current[0]);

        Ruler * rulerLabel = dynamic_cast<Ruler*>(currLabel);
        if(rulerLabel)
        {
            if(rulerLabel->m_Stage == STAGE_ZERO)
            {
                rulerLabel->m_Stage = STAGE_ONE;
                rulerLabel->m_OriginGeomID = targetGeomId;

                uw = Ruler::MapToUW( targetGeomId, placement );

                rulerLabel->m_OriginU = uw.x();
                rulerLabel->m_OriginW = uw.y();
            }
            else if(rulerLabel->m_Stage == STAGE_ONE)
            {
                rulerLabel->m_Stage = STAGE_TWO;
                rulerLabel->m_RulerEndGeomID = targetGeomId;

                uw = Ruler::MapToUW( targetGeomId, placement );

                rulerLabel->m_RulerEndU = uw.x();
                rulerLabel->m_RulerEndW = uw.y();
            }
            else if(rulerLabel->m_Stage == STAGE_TWO)
            {
                rulerLabel->m_Stage = STAGE_COMPLETE;

                rulerLabel->m_XOffset = placement.x();
                rulerLabel->m_YOffset = placement.y();
                rulerLabel->m_ZOffset = placement.z();
            }
        }
    }
}

std::string ManageLabelScreen::getFeedbackGroupName()
{
    return std::string("LabelGUIGroup");
}

std::string ManageLabelScreen::GenerateRuler()
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();

    DrawObj rulerObj;
    rulerObj.m_GeomID = vPtr->getVGuiDraw()->getLabelMgr()->CreateAndAddRuler(GenerateName());
    rulerObj.m_Type = DrawObj::VSP_RULER;
    rulerObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    rulerObj.m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;
    rulerObj.m_TextColor = vec3d(0, 0, 0);
    rulerObj.m_TextSize = 0;
    rulerObj.m_Visible = false;
    m_LabelList.push_back(rulerObj);

    return rulerObj.m_GeomID;
}

void ManageLabelScreen::RemoveRuler(std::string geomId)
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();
    vPtr->getVGuiDraw()->getLabelMgr()->Remove(geomId);

    for(int i = 0; i < (int)m_LabelList.size(); i++)
    {
        if(m_LabelList[i].m_GeomID == geomId)
        {
            m_LabelList.erase(m_LabelList.begin() + i);
        }
    }
}

std::string ManageLabelScreen::GenerateName()
{
    static unsigned int uniqueIndex = 0;

    char str[256];
    sprintf(str, "Ruler_%d", uniqueIndex);
    std::string name = str;

    uniqueIndex++;

    return name;
}

DrawObj * ManageLabelScreen::Find(std::string geomID)
{
    for(int i = 0; i < (int)m_LabelList.size(); i++)
    {
        if(m_LabelList[i].m_GeomID == geomID)
        {
            return &m_LabelList[i];
        }
    }
    return NULL;
}

void ManageLabelScreen::UpdateDrawObjs()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    std::vector<Label*> labelList = veh->getVGuiDraw()->getLabelMgr()->GetVec();
    for(int i = 0; i < (int)labelList.size(); i++)
    {
        // Find out label type.
        Ruler * rulerType = dynamic_cast<Ruler*>(labelList[i]);

        // Find draw object of this Label.  If no match, create one.
        DrawObj * match = Find(labelList[i]->GetID());
        if(!match)
        {
            DrawObj newDO;
            newDO.m_GeomID = labelList[i]->GetID();
            newDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;

            m_LabelList.push_back(newDO);

            // Get matching draw object.
            match = Find(labelList[i]->GetID());
            assert(match);
        }

        match->m_Visible = labelList[i]->m_Visible.Get();

        match->m_TextColor = vec3d(labelList[i]->m_Red.Get() / 255,
            labelList[i]->m_Green.Get() / 255,
            labelList[i]->m_Blue.Get() / 255);

        match->m_TextSize = labelList[i]->m_Size.Get();

        // Set label base on type.
        if(rulerType)
        {
            match->m_Type = DrawObj::VSP_RULER;

            // Set label stage.  Load stage data.
            match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_ZERO;

            if(rulerType->m_Stage == STAGE_ONE)
            {
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_ONE;

                // Load ruler's starting info to DrawObj.
                UpdateRulerStartDO(match, rulerType);
            }
            else if(rulerType->m_Stage == STAGE_TWO)
            {
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_TWO;

                // Load ruler's starting and ending info to DrawObj. 
                UpdateRulerStartDO(match, rulerType);
                UpdateRulerEndDO(match, rulerType);
            }
            else if(rulerType->m_Stage == STAGE_COMPLETE)
            {
                match->m_Ruler.Step = DrawObj::VSP_RULER_STEP_COMPLETE;

                // Load ruler's starting and ending info to DrawObj. 
                UpdateRulerStartDO(match, rulerType);
                UpdateRulerEndDO(match, rulerType);

                // Load placement info to DrawObj.
                match->m_Ruler.Offset = vec3d(rulerType->m_XOffset.Get(), 
                    rulerType->m_YOffset.Get(), 
                    rulerType->m_ZOffset.Get());
            }
        }
    }
}

void ManageLabelScreen::UpdatePickList()
{
    // Load all geom.
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec( false ) );

    m_PickList.clear();
    if(m_Current.size() == 1)
    {
        DrawObj * currDrawObj = Find(m_Current[0]);
        if(currDrawObj && currDrawObj->m_Ruler.Step == DrawObj::VSP_RULER_STEP_COMPLETE)
        {
            // Do nothing.
        }
        else if(currDrawObj && currDrawObj->m_Ruler.Step == DrawObj::VSP_RULER_STEP_TWO)
        {
            DrawObj pickDO;
            pickDO.m_Type = DrawObj::VSP_PICK_LOCATION;
            pickDO.m_GeomID = PICKLOCHEADER + currDrawObj->m_GeomID;
            pickDO.m_PickSourceID = "";
            pickDO.m_FeedbackGroup = getFeedbackGroupName();

            m_PickList.push_back(pickDO);
        }
        else if(currDrawObj)
        {
            for(int i = 0; i < (int)geom_vec.size(); i++)
            {
                vector<DrawObj*> geom_drawObj_vec;
                geom_vec[i]->LoadDrawObjs(geom_drawObj_vec);

                for(int j = 0; j < (int)geom_drawObj_vec.size(); j++)
                {
                    if(geom_drawObj_vec[j]->m_Visible)
                    {
                        // Ignore bounding boxes.
                        if(geom_drawObj_vec[j]->m_GeomID.compare(0, string(BBOXHEADER).size(), BBOXHEADER) != 0)
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
        }
    }
}

void ManageLabelScreen::UpdateNameInput()
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();

    if(m_Current.size() > 1)
    {
        m_LabelUI->nameInput->value("<multiple>");
    }
    else if(m_Current.size() == 1)
    {
        Label * currLabel = vPtr->getVGuiDraw()->getLabelMgr()->Get(m_Current[0]);
        assert(currLabel);
        m_LabelUI->nameInput->value(currLabel->GetName().c_str());
    }
    else
    {
        m_LabelUI->nameInput->value("");
    }
}

void ManageLabelScreen::UpdateRulerStartDO(DrawObj * targetDO, Ruler * ruler)
{
    targetDO->m_Ruler.Start = Ruler::MapToXYZ(
        ruler->m_OriginGeomID, 
        vec2d(ruler->m_OriginU.Get(), ruler->m_OriginW.Get()));
}

void ManageLabelScreen::UpdateRulerEndDO(DrawObj * targetDO, Ruler * ruler)
{
    targetDO->m_Ruler.End = Ruler::MapToXYZ(
        ruler->m_RulerEndGeomID, 
        vec2d(ruler->m_RulerEndU.Get(), ruler->m_RulerEndW.Get()));
}