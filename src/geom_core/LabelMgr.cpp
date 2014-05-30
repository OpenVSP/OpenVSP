#include "LabelMgr.h"

LabelMgr::LabelMgr()
{
}
LabelMgr::~LabelMgr()
{
    for(int i = 0; i < (int)m_Labels.size(); i++)
    {
        delete m_Labels[i];
    }
    m_Labels.clear();
}

std::string LabelMgr::CreateAndAddRuler(const string& name)
{
    Ruler * ruler = new Ruler();

    m_Labels.push_back(ruler);
    m_Labels[m_Labels.size() - 1]->SetName(name);

    return m_Labels[m_Labels.size() - 1]->GetID();
}

Label * LabelMgr::Get(std::string ID)
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

std::vector<Label*> LabelMgr::GetVec()
{
    return m_Labels;
}

void LabelMgr::Remove(std::string ID)
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