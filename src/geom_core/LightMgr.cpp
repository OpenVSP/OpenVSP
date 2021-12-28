#include "LightMgr.h"


LightMgrSingleton::LightMgrSingleton()
{
}

LightMgrSingleton::~LightMgrSingleton()
{
    Wype();
}

void LightMgrSingleton::Init()
{
    for( int i = 0; i < NUMOFLIGHTS; i++ )
    {
        m_Lights.push_back( new Light() );
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

void LightMgrSingleton::Wype()
{
    for( int i = 0; i < (int)m_Lights.size(); i++ )
    {
        delete m_Lights[i];
    }
    m_Lights.clear();
}

void LightMgrSingleton::Renew()
{
    Wype();
    Init();
}

Light * LightMgrSingleton::Get(unsigned int index )
{
    if( index < NUMOFLIGHTS )
    {
        return m_Lights[index];
    }
    return NULL;
}

std::vector< Light* > LightMgrSingleton::GetVec()
{
    return m_Lights;
}

xmlNodePtr LightMgrSingleton::EncodeXml(xmlNodePtr node )
{
    char lightName[256];

    xmlNodePtr light_root_node = xmlNewChild( node, NULL, BAD_CAST "Lights", NULL );
    XmlUtil::AddIntNode( light_root_node, "Num_of_Lights", NUMOFLIGHTS );

    for ( int i = 0; i < (int)m_Lights.size(); i++ )
    {
        sprintf( lightName, "Light%d", i );
        xmlNodePtr light_node = xmlNewChild( light_root_node, NULL, BAD_CAST lightName, NULL );
        m_Lights[i]->EncodeXml( light_node );
    }
    return light_root_node;
}

xmlNodePtr LightMgrSingleton::DecodeXml(xmlNodePtr node )
{
    char lightName[256];

    xmlNodePtr light_root_node = XmlUtil::GetNode( node, "Lights", 0 );

    int numofLights = XmlUtil::FindInt( light_root_node, "Num_of_Lights", 0 );
    for ( int i = 0; i < numofLights; i++ )
    {
        sprintf( lightName, "Light%d", i );
        xmlNodePtr light_node = XmlUtil::GetNode( light_root_node, lightName, 0 );
        if( light_node )
        {
            m_Lights[i]->DecodeXml( light_node );
        }
    }
    return light_root_node;
}
