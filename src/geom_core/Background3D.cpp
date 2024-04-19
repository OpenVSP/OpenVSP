//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#include "Defines.h"

#include "Background3D.h"
#include "Background3DMgr.h"
#include "Vehicle.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VSP_Geom_API.h"
#include "UnitConversion.h"
#include "VspUtil.h"
#include "StlHelper.h"
#include "stb_image.h"

Background3D::Background3D() : ParmContainer()
{
    m_Visible.Init( "Visible", "Background3D", this, true, false, true );
    m_RearVisible.Init( "RearVisible", "Background3D", this, true, false, true );

    m_VisAlign.Init( "VisAlign", "Background3D", this, false, false, true );
    m_VisTol.Init( "VisTol", "Background3D", this, 1.0, 0.0, 180.0 );

    m_Direction.Init( "Direction", "Background3D", this, vsp::VIEW_LEFT, vsp::VIEW_LEFT, vsp::VIEW_NUM_TYPES - 1 );

    m_NormX.Init( "NormX", "Background3D", this, -1.0, -1.0, 1.0 );
    m_NormY.Init( "NormY", "Background3D", this, 0.0, -1.0, 1.0 );
    m_NormZ.Init( "NormZ", "Background3D", this, 0.0, -1.0, 1.0 );

    m_UpX.Init( "UpX", "Background3D", this, 0.0, -1.0, 1.0 );
    m_UpY.Init( "UpY", "Background3D", this, 0.0, -1.0, 1.0 );
    m_UpZ.Init( "UpZ", "Background3D", this, 1.0, -1.0, 1.0 );

    m_ScaleType.Init( "Scaletype", "Background3D", this, vsp::SCALE_WIDTH, vsp::SCALE_WIDTH, vsp::NUM_SCALE_TYPES - 1 );

    m_BackgroundWidth.Init( "W", "Background3D", this, 1.0, 0, 1.0e12 );
    m_BackgroundHeight.Init( "H", "Background3D", this, 1.0, 0, 1.0e12 );
    m_Resolution.Init( "Resolution", "Background3D", this, 1.0, 0, 1e12 );

    m_HAlign.Init( "HAlign", "Background3D", this, vsp::ALIGN_CENTER, vsp::ALIGN_LEFT, vsp::ALIGN_PIXEL );
    m_VAlign.Init( "VAlign", "Background3D", this, vsp::ALIGN_MIDDLE, vsp::ALIGN_PIXEL, vsp::ALIGN_BOTTOM );

    m_ImageX.Init( "ImageX", "Background3D", this, 0.0, 0, 1.0e6 );
    m_ImageY.Init( "ImageY", "Background3D", this, 0.0, 0, 1.0e6 );

    m_ImageW.Init( "ImageW", "Background3D", this, 0.0, 0, 1.0e6 );
    m_ImageH.Init( "ImageH", "Background3D", this, 0.0, 0, 1.0e6 );

    m_X.Init( "X", "Background3D", this, 0.0, -1.0e12, 1.0e12 );
    m_Y.Init( "Y", "Background3D", this, 0.0, -1.0e12, 1.0e12 );
    m_Z.Init( "Z", "Background3D", this, 0.0, -1.0e12, 1.0e12 );

    m_DepthPos.Init( "DepthPos", "Background3D", this, vsp::DEPTH_FRONT, vsp::DEPTH_FRONT, vsp::NUM_DEPTH_TYPE - 1 );

    m_ImageReady = false;
    m_ImageRev = 0;

    LinkMgr.RegisterContainer( GetID() );
}

Background3D::~Background3D()
{
    LinkMgr.UnRegisterContainer( GetID() );
}

xmlNodePtr Background3D::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "BGFile", m_BGFile );
    }

    return parmcontain_node;
}

xmlNodePtr Background3D::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_BGFile = ParmMgr.RemapID( XmlUtil::FindString( parmcontain_node, "BGFile", m_BGFile ) );
    }

    return parmcontain_node;
}

void Background3D::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_BackgroundDO );
}

void Background3D::Update()
{
    if ( !m_BGFile.empty() && !m_ImageReady )
    {
        UpdateImageInfo();
    }

    if ( m_ScaleType() == vsp::SCALE_WIDTH )
    {
        m_Resolution = ( 1.0 * m_ImageW() ) / m_BackgroundWidth();
        double h = ( 1.0 * m_ImageH() ) / m_Resolution();
        if ( isfinite( h ) )
        {
            m_BackgroundHeight = h;
        }
    }
    else if ( m_ScaleType() == vsp::SCALE_HEIGHT )
    {
        m_Resolution = ( 1.0 * m_ImageH() ) / m_BackgroundHeight();
        double w = ( 1.0 * m_ImageW() ) / m_Resolution();
        if ( isfinite( w ) )
        {
            m_BackgroundWidth = w;
        }
    }
    else // vsp::SCALE_RESOLUTION
    {
        double h = ( 1.0 * m_ImageH() ) / m_Resolution();
        if ( isfinite( h ) )
        {
            m_BackgroundHeight = h;
        }
        double w = ( 1.0 * m_ImageW() ) / m_Resolution();
        if ( isfinite( w ) )
        {
            m_BackgroundWidth = w;
        }
    }

    Matrix4d destmat;

    if ( m_Direction() == vsp::VIEW_LEFT )
    {
        destmat.setBasis( vec3d( 1, 0, 0 ), vec3d( 0, 0, 1 ), vec3d( 0, -1, 0 ) );
    }
    else if ( m_Direction() == vsp::VIEW_RIGHT )
    {
        destmat.setBasis( vec3d( -1, 0, 0 ), vec3d( 0, 0, 1 ), vec3d( 0, 1, 0 ) );
    }
    else if ( m_Direction() == vsp::VIEW_TOP )
    {
        destmat.setBasis( vec3d( 0, 1, 0 ), vec3d( -1, 0, 0 ), vec3d( 0, 0, 1 ) );
    }
    else if ( m_Direction() == vsp::VIEW_BOTTOM )
    {
        destmat.setBasis( vec3d( 0, -1, 0 ), vec3d( -1, 0, 0 ), vec3d( 0, 0, -1 ) );
    }
    else if ( m_Direction() == vsp::VIEW_FRONT )
    {
        destmat.setBasis( vec3d( 0, -1, 0 ), vec3d( 0, 0, 1 ), vec3d( -1, 0, 0 ) );
    }
    else if ( m_Direction() == vsp::VIEW_REAR )
    {
        destmat.setBasis( vec3d( 0, 1, 0 ), vec3d( 0, 0, 1 ), vec3d( 1, 0, 0 ) );
    }

    if ( m_Direction() == vsp::VIEW_NONE )
    {
        vec3d zdir( m_NormX(), m_NormY(), m_NormZ() );
        zdir.normalize();

        vec3d ydir( m_UpX(), m_UpY(), m_UpZ() );
        ydir.normalize();

        vec3d xdir = cross( zdir, ydir );
        xdir.normalize();

        ydir = cross( xdir, zdir );
        ydir.normalize();

        destmat.setBasis( xdir, ydir, zdir );

        m_DepthPos = vsp::DEPTH_FREE;
    }
    else
    {
        vec3d xdir, ydir, zdir;
        destmat.getBasis( xdir, ydir, zdir );

        m_NormX = zdir.x();
        m_NormY = zdir.y();
        m_NormZ = zdir.z();

        m_UpX = ydir.x();
        m_UpY = ydir.y();
        m_UpZ = ydir.z();

        if ( m_DepthPos() != vsp::DEPTH_FREE )
        {
            Vehicle *veh = VehicleMgr.GetVehicle();
            if ( veh )
            {
                int imain = zdir.major_comp();
                double kdir = zdir.v[ imain ]; // vsp::DEPTH_FRONT
                if ( m_DepthPos() == vsp::DEPTH_REAR )
                {
                    kdir *= -1;
                }

                double val = 0; // In case BBox is empty.

                BndBox b;
                if ( veh->GetVisibleBndBox( b ) )
                {
                    b.Expand( 0.5 * b.DiagDist() );

                    if ( kdir > 0 ) // Normal vector pointing in positive direction.
                    {
                        val = b.GetMax( imain );
                    }
                    else
                    {
                        val = b.GetMin( imain );
                    }
                }

                if ( imain == 0 )
                {
                    m_X = val;
                }
                else if ( imain == 1 )
                {
                    m_Y = val;
                }
                else
                {
                    m_Z = val;
                }
            }
        }
    }

    Matrix4d defmat;
    defmat.setBasis( vec3d( 0, -1, 0 ), vec3d( 0, 0, 1 ), vec3d( -1, 0, 0 ) );
    defmat.affineInverse();

    Matrix4d alignmat;
    if ( m_HAlign() == vsp::ALIGN_LEFT )
    {
        alignmat.translatef( 0.5 * m_BackgroundWidth(), 0, 0 );
        m_ImageX = 0;
    }
    else if ( m_HAlign() == vsp::ALIGN_RIGHT )
    {
        alignmat.translatef( -0.5 * m_BackgroundWidth(), 0, 0 );
        m_ImageX = m_ImageW();
    }
    else if ( m_HAlign() == vsp::ALIGN_PIXEL )
    {
        double x = 0.5 * m_BackgroundWidth() - m_ImageX() / m_Resolution();
        if ( !isfinite( x ) )
        {
            x = 0;
        }
        alignmat.translatef( x, 0, 0 );
    }
    else // vsp::ALIGN_CENTER
    {
        m_ImageX = 0.5 * m_ImageW();
    }

    if ( m_VAlign() == vsp::ALIGN_TOP )
    {
        alignmat.translatef( 0, -0.5 * m_BackgroundHeight(), 0 );
        m_ImageY = 0;
    }
    else if ( m_VAlign() == vsp::ALIGN_BOTTOM )
    {
        alignmat.translatef( 0, 0.5 * m_BackgroundHeight(), 0 );
        m_ImageY = m_ImageH();
    }
    else if ( m_VAlign() == vsp::ALIGN_PIXEL )
    {
        double y = -0.5 * m_BackgroundHeight() + m_ImageY() / m_Resolution();
        if ( !isfinite( y ) )
        {
            y = 0;
        }
        alignmat.translatef( 0, y, 0 );
    }
    else // vsp::ALIGN_MIDDLE
    {
        m_ImageY = 0.5 * m_ImageH();
    }


    Matrix4d transmat;
    transmat.translatef( m_X(), m_Y(), m_Z() );


    Matrix4d mat;

    mat.scaley( m_BackgroundWidth() );
    mat.scalez( m_BackgroundHeight() );

    mat.postMult( defmat );
    mat.postMult( alignmat );
    mat.postMult( destmat );
    mat.postMult( transmat );

    MakePlane( m_BackgroundDO );
    m_BackgroundDO.m_GeomID = "BG3D_" + GetID();
    m_BackgroundDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;

    mat.xformmat( m_BackgroundDO.m_PntMesh[0] );
    mat.xformnormmat( m_BackgroundDO.m_NormMesh[0] );

    if ( m_RearVisible() )
    {
        m_BackgroundDO.m_Type = DrawObj::VSP_TEXTURED_MESH;
    }
    else
    {
        m_BackgroundDO.m_Type = DrawObj::VSP_TEXTURED_MESH_TRANSPARENT_BACK;
    }

    m_BackgroundDO.m_Visible = m_Visible();

    m_BackgroundDO.m_VisibleDirFlag = m_VisAlign();
    m_BackgroundDO.m_VisTol = m_VisTol();
    if ( m_VisAlign() )
    {
        vec3d nvec( m_NormX(), m_NormY(), m_NormZ() );
        nvec.normalize();
        m_BackgroundDO.m_VisibleDir = nvec;
    }
    else
    {
        m_BackgroundDO.m_VisibleDir = vec3d();
    }

    // Reload texture infos.
    m_BackgroundDO.m_TextureInfos.resize( 1 );

    m_BackgroundDO.m_TextureInfos[0].FileName = m_BGFile;
    m_BackgroundDO.m_TextureInfos[0].UScale = 1.0;
    m_BackgroundDO.m_TextureInfos[0].WScale = 1.0;
    m_BackgroundDO.m_TextureInfos[0].U = 0.0;
    m_BackgroundDO.m_TextureInfos[0].W = 0.0;
    m_BackgroundDO.m_TextureInfos[0].Transparency = 1.0;
    m_BackgroundDO.m_TextureInfos[0].BlendTransparency = false;
    m_BackgroundDO.m_TextureInfos[0].UFlip = true;
    m_BackgroundDO.m_TextureInfos[0].WFlip = false;
    m_BackgroundDO.m_TextureInfos[0].ID = "TEX_" + to_string( m_ImageRev ) + "_" + GetID();
}

void Background3D::UpdateImageInfo()
{
    int w, h, bpp;
    unsigned char *data = stbi_load( m_BGFile.c_str(), &w, &h, &bpp, 0 );

    if ( data == NULL )
    {
        m_ImageReady = false;
        return;
    }

    m_ImageW = w;
    m_ImageH = h;
    m_ImageReady = true;
    m_ImageRev++;

    stbi_image_free(data);
}

string Background3D::GetDirectionName()
{
    string ret;
    if ( m_Direction() == vsp::VIEW_LEFT )
    {
        ret = "Left";
    }
    else if ( m_Direction() == vsp::VIEW_RIGHT )
    {
        ret = "Right";
    }
    else if ( m_Direction() == vsp::VIEW_TOP )
    {
        ret = "Top";
    }
    else if ( m_Direction() == vsp::VIEW_BOTTOM )
    {
        ret = "Bottom";
    }
    else if ( m_Direction() == vsp::VIEW_FRONT )
    {
        ret = "Front";
    }
    else if ( m_Direction() == vsp::VIEW_REAR )
    {
        ret = "Rear";
    }
    else
    {
        ret = "Other";
    }

    return ret;
}
