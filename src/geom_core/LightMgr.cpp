#include "LightMgr.h"

LightMgr::LightMgr()
{
    for( int i = 0; i < NUMOFLIGHTS; i++ )
    {
        m_Lights.push_back(new Light());
    }

    // Setup default lights.
    m_Lights[0]->m_Active = true;
    m_Lights[0]->m_X = 10.0;
    m_Lights[0]->m_Y = -50.0;
    m_Lights[0]->m_Z = 20.0;
    m_Lights[0]->m_Amb = 0.5;
    m_Lights[0]->m_Diff = 0.35;
    m_Lights[0]->m_Spec = 1.0;

    m_Lights[1]->m_Active = true;
    m_Lights[1]->m_X = 10.0;
    m_Lights[1]->m_Y = 15.0;
    m_Lights[1]->m_Z = 30.0;
    m_Lights[1]->m_Amb = 0.5;
    m_Lights[1]->m_Diff = 0.5;
    m_Lights[1]->m_Spec = 1.0;

    m_Lights[2]->m_Active = true;
    m_Lights[2]->m_X = -50.0;
    m_Lights[2]->m_Y = 30.0;
    m_Lights[2]->m_Z = 10.0;
    m_Lights[2]->m_Amb = 0.0;
    m_Lights[2]->m_Diff = 0.5;
    m_Lights[2]->m_Spec = 0.5;
}
LightMgr::~LightMgr()
{
    for( int i = 0; i < m_Lights.size(); i++ )
    {
        delete m_Lights[i];
    }
}

Light * LightMgr::Get( unsigned int index )
{
    if( index >= 0 && index < NUMOFLIGHTS )
    {
        return m_Lights[index];
    }
    return NULL;
}

std::vector< Light* > LightMgr::GetVec()
{
    return m_Lights;
}