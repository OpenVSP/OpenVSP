#include "TextureMgr.h"


TextureMgr::TextureMgr()
{
}
TextureMgr::~TextureMgr()
{
    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        delete m_TextureList[i];
    }
}

std::string TextureMgr::AttachTexture( const std::string &fileName )
{
    Texture * texture = new Texture( fileName );

    m_TextureList.push_back( texture );
    return texture->GetID(); 
}

void TextureMgr::RemoveTexture( const std::string &texture_id )
{
    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        if( m_TextureList[i]->GetID() == texture_id )
        {
            delete m_TextureList[i];
            m_TextureList.erase( m_TextureList.begin() + i );
            break;
        }
    }
}

Texture * TextureMgr::FindTexture( const std::string &texture_id )
{
    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        if( m_TextureList[i]->GetID() == texture_id )
        {
            return m_TextureList[i];
        }
    }
    return nullptr;
}

std::vector<std::string> TextureMgr::GetTextureVec()
{
    std::vector<std::string> ids( m_TextureList.size() );

    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        ids[i] = m_TextureList[i]->GetID();
    }
    return ids;
}

std::vector<Texture*> TextureMgr::FindTextureVec( std::vector<std::string> texture_id_vec )
{
    std::vector<Texture*> textureVec;
    for( int i = 0; i < (int)texture_id_vec.size(); i++ )
    {
        Texture * matchptr = FindTexture(texture_id_vec[i]);
        if( matchptr )
        {
            textureVec.push_back( matchptr );
        }
    }
    return textureVec;
}

xmlNodePtr TextureMgr::EncodeXml( xmlNodePtr node )
{
    char texName[256];

    xmlNodePtr child_node = xmlNewChild( node, nullptr, BAD_CAST "Textures", nullptr );
    XmlUtil::AddIntNode( child_node, "Num_of_Tex", m_TextureList.size() );

    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        snprintf( texName, sizeof( texName ), "Texture%d", i );
        xmlNodePtr tex_node = xmlNewChild( child_node, nullptr, BAD_CAST texName, nullptr );
        m_TextureList[i]->EncodeXml( tex_node );
    }
    return child_node;
}

xmlNodePtr TextureMgr::DecodeXml( xmlNodePtr node )
{
    char texName[256];

    xmlNodePtr child_node = XmlUtil::GetNode( node, "Textures", 0 );

    int numofTex = XmlUtil::FindInt( child_node, "Num_of_Tex", 0 );
    for( int i = 0; i < numofTex; i++ )
    {
        snprintf( texName, sizeof( texName ), "Texture%d", i );
        xmlNodePtr tex_node = XmlUtil::GetNode( child_node, texName, 0 );
        xmlNodePtr parm_node = XmlUtil::GetNode( tex_node, "ParmContainer", 0 );
        if( tex_node && parm_node )
        {
            std::string file = XmlUtil::FindString( parm_node, "File_Name", "" );
            std::string id = AttachTexture( file );

            FindTexture( id )->DecodeXml( tex_node );
        }
    }
    return child_node;
}
