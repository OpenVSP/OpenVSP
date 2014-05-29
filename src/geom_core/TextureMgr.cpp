#include "TextureMgr.h"
#include "StringUtil.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

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

std::string TextureMgr::AttachTexture( std::string fileName )
{
    Texture * texture = new Texture( fileName );

    m_TextureList.push_back( texture );
    return texture->GetID(); 
}

void TextureMgr::RemoveTexture( std::string texture_id )
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

Texture * TextureMgr::FindTexture( std::string texture_id )
{
    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        if( m_TextureList[i]->GetID() == texture_id )
        {
            return m_TextureList[i];
        }
    }
    return NULL;
}

std::vector<std::string> TextureMgr::GetTextureVec()
{
    std::vector<std::string> ids;

    for( int i = 0; i < (int)m_TextureList.size(); i++ )
    {
        ids.push_back( m_TextureList[i]->GetID() );
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