//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include "SubGLWindow.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "GraphicEngine.h"
#include "GraphicSingletons.h"
#include "Display.h"
#include "Scene.h"
#include "Viewport.h"
#include "Camera.h"
#include "Renderable.h"
#include "Entity.h"
#include "ManageLightingScreen.h"
#include "Lighting.h"

using namespace VSPGraphic;

namespace VSPGUI
{
VspSubGlWindow::VspSubGlWindow( int x, int y, int w , int h, DrawObj::ScreenEnum drawObjScreen )
    : Fl_Gl_Window( x, y, w, h, "VSP Sub GL Window" )
{
    mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE | FL_MULTISAMPLE );

    m_GEngine = new VSPGraphic::GraphicEngine();

    // Link this GUI to one of drawObj screen.
    m_LinkedScreen = drawObjScreen;

    m_GEngine->getDisplay()->setDisplayLayout( 1, 1 );
    m_GEngine->getDisplay()->selectViewport( 0 );
    m_GEngine->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_FRONT );

    Viewport * viewport = m_GEngine->getDisplay()->getViewport();
    assert( viewport );
    if( viewport )
    {
        // Disable border and arrows.
        viewport->showBorders( false );
        viewport->showXYZArrows( false );

        // Enable grid.
        viewport->showGridOverlay( true );
    }

    m_id = 0xFFFFFFFF;

    m_Initialized = false;

#ifdef __APPLE__
#if FL_API_VERSION >= 10304
    Fl::use_high_res_GL( true );
#endif
#endif
}
VspSubGlWindow::~VspSubGlWindow()
{
    delete m_GEngine;
}

void VspSubGlWindow::draw()
{
    // Initialize Glew when context is created.
    _initGLEW();

    if ( !valid() )
    {
        m_GEngine->getDisplay()->resize( pixel_w(), pixel_h() );
    }
    m_GEngine->draw();
}

void VspSubGlWindow::update()
{
    make_current();

    if( this->context_valid() )
    {
        vector<DrawObj*> drawObjs;

        LoadAllDrawObjs( drawObjs );

        _update( drawObjs );

        for( int i = 0; i < (int)drawObjs.size(); i++ )
        {
            if( drawObjs[i]->m_Screen == m_LinkedScreen )
            {
                drawObjs[i]->m_GeomChanged = false;
            }
        }

    }
}

void VspSubGlWindow::LoadAllDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    if ( vPtr )
    {
        vPtr->LoadDrawObjs( draw_obj_vec );
    }
}

void VspSubGlWindow::_update( std::vector<DrawObj *> objects )
{
    // Check for changes in DrawObjs and adjust accordingly.
    _updateBuffer( objects );

    // Update Scene Objects.
    for( int i = 0; i < ( int )objects.size(); i++ )
    {
        // If this DrawObj is aimed for other screen, ignore.
        if( objects[i]->m_Screen != m_LinkedScreen )
        {
            continue;
        }

        // Load Settings.
        float red, green, blue, size;

        unsigned int id;
        ID * idPtr = _findID( objects[i]->m_GeomID );
        if( idPtr )
        {
            id = idPtr->bufferID;
        }
        else
        {
            id = 0xFFFFFFFF;
        }

        VSPGraphic::Renderable * rObj;
        VSPGraphic::Entity * eObj;
        switch( objects[i]->m_Type )
        {
        case DrawObj::VSP_SETTING:
            _setLighting( objects[i] );
            break;

        case DrawObj::VSP_POINTS:
            // Create new scene object if needed.
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }
            // Update scene object.
            red = ( float )objects[i]->m_PointColor.x();
            green = ( float )objects[i]->m_PointColor.y();
            blue = ( float )objects[i]->m_PointColor.z();

            size = ( float )objects[i]->m_PointSize;

            rObj = dynamic_cast<VSPGraphic::Renderable*>( m_GEngine->getScene()->getObject(id) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_POINTS );
                rObj->setPointColor( red, green, blue );
                rObj->setPointSize( size );

                if( objects[i]->m_GeomChanged )
                {
                    _loadPointData( rObj, objects[ i ] );
                }
            }
            break;

        case DrawObj::VSP_LINES:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_MARKER, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }

            // Update scene object.
            red = ( float )objects[i]->m_LineColor.x();
            green = ( float )objects[i]->m_LineColor.y();
            blue = ( float )objects[i]->m_LineColor.z();

            size = ( float )objects[i]->m_LineWidth;

            rObj = dynamic_cast<VSPGraphic::Renderable*>( m_GEngine->getScene()->getObject( id ) );
            if( rObj )
            {
                rObj->setVisibility( objects[i]->m_Visible );
                rObj->setPrimType( VSPGraphic::Common::VSP_LINES );
                rObj->setLineColor( red, green, blue );
                rObj->setLineWidth( size );

                if( objects[i]->m_GeomChanged )
                {
                    _loadLineData( rObj, objects[ i ] );
                }
            }
            break;

        case DrawObj::VSP_TEXTURED_MESH:
            if( id == 0xFFFFFFFF )
            {
                m_GEngine->getScene()->createObject( Common::VSP_OBJECT_ENTITY, &id );

                ID idInfo;
                idInfo.bufferID = id;
                idInfo.geomID = objects[i]->m_GeomID;
                m_ids.push_back( idInfo );
            }

            eObj = dynamic_cast<VSPGraphic::Entity*> ( m_GEngine->getScene()->getObject( id ) );
            if( eObj )
            {
                eObj->setVisibility( objects[i]->m_Visible );
                eObj->setPrimType( Common::VSP_QUADS );
                eObj->setRenderStyle( Common::VSP_DRAW_TEXTURED );

                eObj->setMaterial( objects[i]->m_MaterialInfo.Ambient, objects[i]->m_MaterialInfo.Diffuse,
                                   objects[i]->m_MaterialInfo.Specular, objects[i]->m_MaterialInfo.Emission,
                                   objects[i]->m_MaterialInfo.Shininess );


                if( objects[i]->m_GeomChanged )
                {
                    _loadXSecData( eObj, objects[i] );
                }
                _updateTextures( objects[i] );
            }
            break;

        default:
            break;
        }
    }
}

void VspSubGlWindow::_initGLEW()
{
    if( !m_Initialized )
    {
        VSPGraphic::GraphicEngine::initGlew();

        m_Initialized = true;
    }
}

void VspSubGlWindow::_loadPointData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;

    for( int i = 0; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        // Position x y z.
        vdata.push_back( ( float )drawObj->m_PntVec[i].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].z() );

        // Normal x y z.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        // Texture Coordinate u w.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof( float ) * vdata.size() );
}

void VspSubGlWindow::_loadLineData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;

    for( int i = 1; i < ( int )drawObj->m_PntVec.size(); i++ )
    {
        // Position x y z.
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i-1].z() );

        // Normal x y z.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        // Texture Coordinate u w.
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );

        vdata.push_back( ( float )drawObj->m_PntVec[i].x() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].y() );
        vdata.push_back( ( float )drawObj->m_PntVec[i].z() );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
        vdata.push_back( 0.0f );
    }
    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof( float ) * vdata.size() );
}

void VspSubGlWindow::_updateTextures( DrawObj * drawObj )
{
    VspSubGlWindow::ID * id = _findID( drawObj->m_GeomID );
    if( !id )
    {
        // Can't find Object that is linked to this drawObj
        assert( false );
        return;
    }

    Renderable * renderable = dynamic_cast<Renderable*>( m_GEngine->getScene()->getObject( id->bufferID ) );
    if( renderable )
    {
        VSPGraphic::Entity * entity = dynamic_cast<VSPGraphic::Entity *>( renderable );
        if( entity )
        {
            // Check if any textureID no long exists in drawObj.
            // Remove Deleted Textures.

            std::vector<TextureID> texToRemove;
            std::vector<TextureID> texToKeep;

            for( int i = 0; i < ( int )id->textureIDs.size(); i++ )
            {
                bool exist = false;
                for( int j = 0; j < ( int )drawObj->m_TextureInfos.size(); j++ )
                {
                    if( id->textureIDs[i].geomTexID == drawObj->m_TextureInfos[j].ID )
                    {
                        texToKeep.push_back( id->textureIDs[i] );
                        exist = true;
                        break;
                    }
                }
                if( !exist )
                {
                    texToRemove.push_back( id->textureIDs[i] );
                }
            }

            for(int i = 0; i < ( int )texToRemove.size(); i++)
            {
                entity->getTextureMgr()->remove( texToRemove[i].bufferTexID );
            }

            id->textureIDs = texToKeep;

            // Update / add textures.

            std::vector<DrawObj::TextureInfo> drawObjTexList = drawObj->m_TextureInfos;
            for( int i = 0; i < ( int )drawObjTexList.size(); i++ )
            {
                unsigned int texBufferID;

                TextureID * texID = id->find( drawObjTexList[i].ID );
                if( texID )
                {
                    texBufferID = texID->bufferTexID;
                }
                else
                {
                    // New texture.
                    TextureID newTex;
                    newTex.geomTexID = drawObjTexList[i].ID;
                    newTex.bufferTexID = entity->getTextureMgr()->add(
                            VSPGraphic::GlobalTextureRepo()->get2DTexture(
                                    drawObjTexList[i].FileName.c_str(),
                                    drawObjTexList[i].ModificationKey,
                                    drawObjTexList[i].Rot,
                                    drawObjTexList[i].FlipLR,
                                    drawObjTexList[i].FlipUD,
                                    drawObjTexList[i].AutoTrans ) );
                    id->textureIDs.push_back( newTex );

                    texBufferID = newTex.bufferTexID;
                }

                float UScale = ( float )( 1 / drawObjTexList[i].UScale );
                float WScale = ( float )( 1 / drawObjTexList[i].WScale );
                float U = ( float )( drawObjTexList[i].U * -1.f * UScale );
                float W = ( float )( drawObjTexList[i].W * WScale );

                // Update Texture Properties.
                entity->getTextureMgr()->translate( texBufferID, U, W );
                entity->getTextureMgr()->scale( texBufferID, UScale, WScale );
                entity->getTextureMgr()->transparentize( texBufferID, drawObjTexList[ i ].Transparency, drawObjTexList[ i ].BlendTransparency );
                entity->getTextureMgr()->flipCoord( texBufferID, drawObjTexList[i].UFlip, drawObjTexList[i].WFlip );
            }
        }
        else
        {
            // Render Object is not type of Textureable Entity.
            assert( false );
        }
    }
    else
    {
        // Can't find Render Object.
        assert( false );
    }
}

void VspSubGlWindow::_loadXSecData( Renderable * destObj, DrawObj * drawObj )
{
    std::vector<float> vdata;
    std::vector<unsigned int> edata;

    // Generate Texture Coordinate.
//    std::vector<std::vector<vec3d>> textureCoords = _generateTexCoordFromXSec( drawObj );

    int num_mesh = drawObj->m_PntMesh.size();

    if ( drawObj->m_NormMesh.size() != num_mesh || drawObj->m_uTexMesh.size() != num_mesh || drawObj->m_vTexMesh.size() != num_mesh )
    {
        destObj->emptyVBuffer();
        destObj->emptyEBuffer();
        destObj->enableEBuffer( false );
        return;
    }

    unsigned int vtotal = 0;
    unsigned int etotal = 0;

    for ( int k = 0; k < num_mesh; k++ )
    {
        int num_pnts = drawObj->m_PntMesh[k].size();
        int num_xsecs = 0;
        if ( num_pnts )
            num_xsecs = drawObj->m_PntMesh[k][0].size();

        vtotal += num_xsecs * num_pnts;

        etotal += (num_pnts - 1) * (num_xsecs - 1);
    }

    vdata.reserve( vtotal * 8 );
    edata.reserve( etotal * 4 );

    unsigned int offset = 0;

    for ( int k = 0; k < num_mesh; k++ )
    {
        int increment = 0;

        int num_pnts = drawObj->m_PntMesh[k].size();
        int num_xsecs = 0;
        if ( num_pnts )
            num_xsecs = drawObj->m_PntMesh[k][0].size();

        // Vertex Buffer.
        for ( int i = 0 ; i < num_pnts ; i++ )
        {
            for ( int j = 0 ; j < num_xsecs ; j++ )
            {
                vdata.push_back( (float)drawObj->m_PntMesh[k][i][j].x() );
                vdata.push_back( (float)drawObj->m_PntMesh[k][i][j].y() );
                vdata.push_back( (float)drawObj->m_PntMesh[k][i][j].z() );

                vdata.push_back( (float)drawObj->m_NormMesh[k][i][j].x() );
                vdata.push_back( (float)drawObj->m_NormMesh[k][i][j].y() );
                vdata.push_back( (float)drawObj->m_NormMesh[k][i][j].z() );

                vdata.push_back( (float)drawObj->m_uTexMesh[k][i][j] );
                vdata.push_back( (float)drawObj->m_vTexMesh[k][i][j] );
                increment++;
            }
        }


        // Element Buffer.
        for( int i = 0; i < num_pnts - 1; i++ )
        {
            for( int j = 0; j < num_xsecs - 1; j++ )
            {
                edata.push_back( offset + i * num_xsecs + j );
                edata.push_back( offset + ( i + 1 ) * num_xsecs + j );
                edata.push_back( offset + ( i + 1 ) * num_xsecs + j + 1 );
                edata.push_back( offset + i * num_xsecs + j + 1 );
            }
        }

        offset += increment;

    }

    destObj->setFacingCW( drawObj->m_FlipNormals );

    destObj->emptyVBuffer();
    destObj->appendVBuffer( vdata.data(), sizeof(float) * vdata.size() );

    destObj->emptyEBuffer();
    destObj->appendEBuffer( edata.data(), sizeof( unsigned int ) * edata.size() );
    destObj->enableEBuffer( true );

}

void VspSubGlWindow::_setLighting( DrawObj * drawObj )
{
    if( drawObj->m_Type != DrawObj::VSP_SETTING )
        return;

    // Currently only support up to eight light sources.
    assert( drawObj->m_LightingInfos.size() <= 8 );
    if( drawObj->m_LightingInfos.size() > 8 )
        return;

    DrawObj::LightSourceInfo lInfo;

    LightSource * lSource;

    Lighting * lights = m_GEngine->getScene()->getLights();

    for( unsigned int i = 0; i < (int)drawObj->m_LightingInfos.size(); i++ )
    {
        lInfo = drawObj->m_LightingInfos[i];

        lSource = lights->getLightSource( i );

        lSource->position( lInfo.X, lInfo.Y, lInfo.Z, 0.0f );
        lSource->ambient( lInfo.Amb, lInfo.Amb, lInfo.Amb, 1.0f );
        lSource->diffuse( lInfo.Diff, lInfo.Diff, lInfo.Diff, 1.0f );
        lSource->specular( lInfo.Spec, lInfo.Spec, lInfo.Spec, 1.0f );

        if( lInfo.Active )
            lSource->enable();
        else
            lSource->disable();
    }
}

VspSubGlWindow::ID * VspSubGlWindow::_findID( std::string geomID )
{
    for( int i = 0; i < ( int )m_ids.size(); i++ )
    {
        if( m_ids[i].geomID == geomID )
        {
            return &m_ids[i];
        }
    }
    return NULL;
}

VspSubGlWindow::ID * VspSubGlWindow::_findID( unsigned int bufferID )
{
    for( int i = 0; i < ( int )m_ids.size(); i++ )
    {
        if( m_ids[i].bufferID == bufferID )
        {
            return &m_ids[i];
        }
    }
    return NULL;
}

void VspSubGlWindow::_updateBuffer( std::vector<DrawObj *> objects )
{
    std::vector<ID> idsToRemove;
    std::vector<ID> idsToKeep;

    // Figure out if any buffer object no longer exists in DrawObjs.
    for( int i = 0; i < ( int )m_ids.size(); i++ )
    {
        bool exist = false;
        for( int j = 0; j < ( int )objects.size(); j++ )
        {
            if( m_ids[i].geomID == objects[j]->m_GeomID && m_ids[i].geomID != std::string( "Default" ) )
            {
                idsToKeep.push_back( m_ids[i] );
                exist = true;
                break;
            }
        }
        if( !exist )
        {
            idsToRemove.push_back( m_ids[i] );
        }
    }

    // Remove unnecessary buffers.
    for( int i = 0; i < ( int )idsToRemove.size(); i++ )
    {
        m_GEngine->getScene()->removeObject( idsToRemove[i].bufferID );
    }

    // Update m_ids.
    m_ids.clear();
    for( int i = 0; i < ( int )idsToKeep.size(); i++ )
    {
        m_ids.push_back( idsToKeep[i] );
    }
}

}
