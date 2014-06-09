#include "TextureMgr.h"
#include "StringUtil.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

Texture::Texture( std::string fileName )
{
    m_FileName = fileName;

    // Generate Display Name.
    std::string displayName = "";

    StringUtil::change_from_to( fileName, '/', ' ' );
    StringUtil::change_from_to( fileName, '\\', ' ' );

    std::string::size_type displayIndex = fileName.find_last_of( ' ' );
    if( displayIndex != std::string::npos )
    {
        displayName = fileName.substr( displayIndex + 1, fileName.size() - displayIndex - 1 );
        std::string::size_type extIndex = displayName.find_last_of( '.' );
        if( extIndex != std::string::npos )
        {
            std::string ext = displayName.substr( extIndex, displayName.size() - extIndex );
            if( ext == ".jpg" || ext == ".tga" )
            {
                displayName = displayName.substr( 0, extIndex );
            }
        }
    }
    m_DisplayName = displayName;

    // Initialized Parms.
    m_U.Init( "U", "Texture_Parm", this, 0.0, -1.0, 1.0, false );
    m_W.Init( "W", "Texture_Parm", this, 0.5, 0.0, 1.0, false );

    m_UScale.Init( "U_Scale", "Texture_Parm", this, 1.0, 0.01, 1.0, false );
    m_WScale.Init( "W_Scale", "Texture_Parm", this, 1.0, 0.01, 1.0, false );

    m_Transparency.Init( "Transparency", "Texture_Parm", this, 1.0, 0.0, 1.0, false );

    m_FlipU.Init( "U_Flip", "Texture_Parm", this, 0, 0, 1, false );
    m_FlipU.Set( false );

    m_FlipW.Init( "W_Flip", "Texture_Parm", this, 0, 0, 1, false );
    m_FlipW.Set( false );
}
Texture::~Texture()
{
}

void Texture::Rename( std::string name )
{
    m_DisplayName = name;
}

void Texture::ParmChanged( Parm* parm_ptr, int type )
{
    VehicleMgr::getInstance().GetVehicle()->ParmChanged( parm_ptr, type );
}


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