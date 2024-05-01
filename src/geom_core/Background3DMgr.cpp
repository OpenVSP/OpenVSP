#include "Background3DMgr.h"
#include "APIDefines.h"

Background3DMgrSingleton::Background3DMgrSingleton()
{
    m_CurrBackground3DIndex = 0;

    m_LightingDO.m_Type = DrawObj::VSP_SETTING;
    m_LightingDO.m_Screen = DrawObj::VSP_3DBG_PREVIEW;

    m_LightingDO.m_LightingInfos.resize( 1 );

    m_LightingDO.m_LightingInfos[0].Active = true;
    m_LightingDO.m_LightingInfos[0].X = 0;
    m_LightingDO.m_LightingInfos[0].Y = 0;
    m_LightingDO.m_LightingInfos[0].Z = 0;
    m_LightingDO.m_LightingInfos[0].Amb = 1;
    m_LightingDO.m_LightingInfos[0].Diff = 0;
    m_LightingDO.m_LightingInfos[0].Spec = 0;

    Init();
}

void Background3DMgrSingleton::Init()
{
}

void Background3DMgrSingleton::Wype()
{
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        delete m_Background3Ds[i];
    }
    m_Background3Ds.clear();

}

void Background3DMgrSingleton::Renew()
{
    Wype();
    Init();
}

Background3D * Background3DMgrSingleton::CreateAndAddBackground3D()
{
    Background3D * bg3D = new Background3D();

    m_Background3Ds.push_back( bg3D );

    m_CurrBackground3DIndex = m_Background3Ds.size() - 1;
    return bg3D;
}

Background3D * Background3DMgrSingleton::GetCurrentBackground3D()
{
    if ( m_CurrBackground3DIndex < 0 || m_CurrBackground3DIndex >= ( int )m_Background3Ds.size() )
    {
        return NULL;
    }

    return m_Background3Ds[ m_CurrBackground3DIndex ];
}

std::vector < Background3D * > Background3DMgrSingleton::GetBackground3DVec()
{
    return m_Background3Ds;
}

xmlNodePtr Background3DMgrSingleton::EncodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "Background3D", NULL );

    XmlUtil::AddIntNode( child_node, "Num_of_Background3Ds", m_Background3Ds.size() );

    for ( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Background3D_%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, NULL, BAD_CAST labelName, NULL );
        m_Background3Ds[i]->EncodeXml( label_node );
    }

    return child_node;
}

xmlNodePtr Background3DMgrSingleton::DecodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr label_root_node = XmlUtil::GetNode( node, "Background3D", 0 );

    int numofBackground3Ds = XmlUtil::FindInt( label_root_node, "Num_of_Background3Ds", 0 );
    for ( int i = 0; i < numofBackground3Ds; i++ )
    {
        snprintf( labelName, sizeof( labelName ), "Background3D_%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                Background3D *bg3D = CreateAndAddBackground3D();
                if ( bg3D )
                {
                    bg3D->DecodeXml( label_node );
                }
            }
        }
    }

    return label_root_node;
}

void Background3DMgrSingleton::Update()
{
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        m_Background3Ds[i]->Update();
    }
}

void Background3DMgrSingleton::SetCurrBackground3DIndex( int i )
{
    if ( i < 0 || i >= m_Background3Ds.size() )
    {
        m_CurrBackground3DIndex = -1;
    }
    else
    {
        m_CurrBackground3DIndex = i;
    }
}

void Background3DMgrSingleton::DelAllBackground3Ds()
{
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        delete m_Background3Ds[i];
    }
    m_Background3Ds.clear();

    m_CurrBackground3DIndex = -1;
}

void Background3DMgrSingleton::ShowAllBackground3Ds()
{
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        m_Background3Ds[i]->m_Visible = true;
    }
}

void Background3DMgrSingleton::HideAllBackground3Ds()
{
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        m_Background3Ds[i]->m_Visible = false;
    }
}

Background3D * Background3DMgrSingleton::GetBackground3D( const string &id )
{
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        if ( m_Background3Ds[i]->GetID() == id )
        {
            return m_Background3Ds[i];
        }
    }

    return NULL;
}

vector < string > Background3DMgrSingleton::GetAllBackground3Ds()
{
    vector < string > Background3DList( m_Background3Ds.size() );

    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        Background3DList[i] = m_Background3Ds[i]->GetID();
    }

    return Background3DList;
}

void Background3DMgrSingleton::DelBackground3D( const int & i )
{
    if ( i < 0 || i >= ( int )m_Background3Ds.size() )
    {
        return;
    }

    Background3D* bg3D = m_Background3Ds[i];

    m_Background3Ds.erase( m_Background3Ds.begin() +  i );

    delete bg3D;
}

void Background3DMgrSingleton::DelBackground3D( const string &id )
{
    int idel = -1;
    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        if ( m_Background3Ds[i]->GetID() == id )
        {
            idel = i;
            break;
        }
    }

    DelBackground3D( idel );
}

void Background3DMgrSingleton::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    UpdateDrawObjs();

    draw_obj_vec.push_back( & m_LightingDO );

    for( int i = 0; i < ( int )m_Background3Ds.size(); i++ )
    {
        m_Background3Ds[i]->LoadDrawObjs( draw_obj_vec );
    }

}

void Background3DMgrSingleton::UpdateDrawObjs()
{
}
