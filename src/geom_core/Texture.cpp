#include "Texture.h"
#include "StringUtil.h"
#include "Vehicle.h"

Texture::Texture( std::string fileName )
{
    m_FileName = fileName;

    // Generate Display Name.
    std::string displayName = "";

    StringUtil::change_from_to( fileName, '/', ' ' );
    StringUtil::change_from_to( fileName, '\\', ' ' );

    size_t displayIndex = fileName.find_last_of( ' ' );
    if( displayIndex != std::string::npos )
    {
        displayName = fileName.substr( displayIndex + 1, fileName.size() - displayIndex - 1 );
        size_t extIndex = displayName.find_last_of( '.' );
        if( extIndex != std::string::npos )
        {
            std::string ext = displayName.substr( extIndex, displayName.size() - extIndex );
            if( ext == ".jpg" || ext == ".tga" )
            {
                displayName = displayName.substr( 0, extIndex );
            }
        }
    }
    ParmContainer::SetName( displayName );

    // Initialized Parms.
    m_U.Init( "U", "Texture_Parm", this, 0.0, -1.0, 1.0 );
    m_W.Init( "W", "Texture_Parm", this, 0.5, 0.0, 1.0 );

    m_UScale.Init( "U_Scale", "Texture_Parm", this, 1.0, 0.01, 1.0 );
    m_WScale.Init( "W_Scale", "Texture_Parm", this, 1.0, 0.01, 1.0 );

    m_Transparency.Init( "Transparency", "Texture_Parm", this, 1.0, 0.0, 1.0 );

    m_FlipU.Init( "U_Flip", "Texture_Parm", this, 0, 0, 1 );
    m_FlipU.Set( false );

    m_FlipW.Init( "W_Flip", "Texture_Parm", this, 0, 0, 1 );
    m_FlipW.Set( false );
}
Texture::~Texture()
{
}

void Texture::ParmChanged( Parm* parm_ptr, int type )
{
    VehicleMgr.GetVehicle()->ParmChanged( parm_ptr, type );
}

xmlNodePtr Texture::EncodeXml( xmlNodePtr &node )
{
    xmlNodePtr container_node = ParmContainer::EncodeXml( node );
    XmlUtil::AddStringNode( container_node, "File_Name", m_FileName );
    return container_node;
}
