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

std::string LabelMgr::CreateAndAddRuler()
{
    return CreateAndAddRuler( "" );
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

xmlNodePtr LabelMgr::EncodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr child_node = xmlNewChild( node, NULL, BAD_CAST "Labels", NULL );
    XmlUtil::AddIntNode( child_node, "Num_of_Labels", m_Labels.size() );

    for ( int i = 0; i < (int)m_Labels.size(); i++ )
    {
        sprintf( labelName, "Label%d", i );
        xmlNodePtr label_node = xmlNewChild( child_node, NULL, BAD_CAST labelName, NULL );
        m_Labels[i]->EncodeXml( label_node );
    }
    return child_node;
}

xmlNodePtr LabelMgr::DecodeXml( xmlNodePtr & node )
{
    char labelName[256];

    xmlNodePtr label_root_node = XmlUtil::GetNode( node, "Labels", 0 );

    int numofLabels = XmlUtil::FindInt( label_root_node, "Num_of_Labels", 0 );
    for ( int i = 0; i < numofLabels; i++ )
    {
        sprintf( labelName, "Label%d", i );
        xmlNodePtr label_node = XmlUtil::GetNode( label_root_node, labelName, 0 );
        if( label_node )
        {
            xmlNodePtr parmcontain_node = XmlUtil::GetNode( label_node, "ParmContainer", 0 );
            if ( parmcontain_node )
            {
                // Find type of label.
                std::string type = XmlUtil::FindString( parmcontain_node, "Type", TYPE_LABEL );

                // Ruler
                if ( type == TYPE_RULER )
                {
                    std::string id = CreateAndAddRuler();
                    Get( id )->DecodeXml( label_node );
                }
            }
        }
    }
    return label_root_node;
}