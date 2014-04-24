#include "Labels.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

Label::Label() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_OriginX.Init("OriginX", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_OriginY.Init("OriginY", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_OriginZ.Init("OriginZ", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);

    m_Red.Init("Red", "Label_Parm", this, 100, 0, 255, false);
    m_Green.Init("Green", "Label_Parm", this, 100, 0, 255, false);
    m_Blue.Init("Blue", "Label_Parm", this, 100, 0, 255, false);

    m_Size.Init("Size", "Label_Parm", this, 2.0, 1.0, 10.0, false);

    m_Visible = true;
}
Label::~Label()
{
}

void Label::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();
    if ( vPtr )
        vPtr->ParmChanged( parm_ptr, Parm::SET );
}

void Label::Reset()
{
    m_Stage = STAGE_ZERO;

    m_OriginX = 0.0;
    m_OriginY = 0.0;
    m_OriginZ = 0.0;

    m_Red = 100.0;
    m_Green = 100.0;
    m_Blue = 100.0;

    m_Size = 2.0;
}

Ruler::Ruler() : Label()
{
    m_EndX.Init("RulerEndX", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_EndY.Init("RulerEndY", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_EndZ.Init("RulerEndZ", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);

    m_XOffset.Init("X_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_YOffset.Init("Y_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_ZOffset.Init("Z_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);

    m_Precision.Init("Precision", "Label_Parm", this, 0, 0, 10, false);
}
Ruler::~Ruler()
{
}

void Ruler::Reset()
{
    Label::Reset();

    m_EndX = 0.0;
    m_EndY = 0.0;
    m_EndZ = 0.0;

    m_XOffset = 0.0;
    m_YOffset = 0.0;
    m_ZOffset = 0.0;

    m_Precision = 0.0;
}

Labels::Labels()
{
}
Labels::~Labels()
{
    for(int i = 0; i < (int)m_Labels.size(); i++)
    {
        delete m_Labels[i];
    }
    m_Labels.clear();
}

std::string Labels::CreateAndAddRuler(const string& name)
{
    Ruler * ruler = new Ruler();

    m_Labels.push_back(ruler);
    m_Labels[m_Labels.size() - 1]->SetName(name);

    return m_Labels[m_Labels.size() - 1]->GetID();
}

Label * Labels::Get(std::string ID)
{
    for(int i = 0; i < (int)m_Labels.size(); i++)
    {
        if(m_Labels[i]->GetID() == ID)
        {
            return m_Labels[i];
        }
    }
    return NULL;
}

std::vector<Label*> Labels::GetVec()
{
    return m_Labels;
}

void Labels::Remove(std::string ID)
{
    for(int i = 0; i < (int)m_Labels.size(); i++)
    {
        if(m_Labels[i]->GetID() == ID)
        {
            delete m_Labels[i];

            m_Labels.erase(m_Labels.begin() + i);
            return;
        }
    }
}