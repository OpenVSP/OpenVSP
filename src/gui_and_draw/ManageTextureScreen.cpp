#include <assert.h>

#include "ScreenMgr.h"

#include "ManageTextureScreen.h"
#include "textureMgrFlScreen.h"

#include "SubGLWindow.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "GraphicSingletons.h"

#include "Geom.h"
#include "Textures.h"

#include "FL/Fl_File_Chooser.H"

ManageTextureScreen::ManageTextureScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_TextureMgrUI = new TextureMgrUI();
    m_FLTK_Window = m_TextureMgrUI->UIWindow;

    m_TextureMgrUI->UIWindow->position( 775, 50 );

    m_UPosSlider.Init( this, m_TextureMgrUI->uPosSlider, m_TextureMgrUI->uPosInput, 1, "%6.5f", m_TextureMgrUI->uPosButton );
    m_WPosSlider.Init( this, m_TextureMgrUI->wPosSlider, m_TextureMgrUI->wPosInput, 1, "%6.5f", m_TextureMgrUI->wPosButton );

    m_UScaleSlider.Init( this, m_TextureMgrUI->uScaleSlider, m_TextureMgrUI->uScaleInput, 1, "%6.5f", m_TextureMgrUI->uScaleButton );
    m_WScaleSlider.Init( this, m_TextureMgrUI->wScaleSlider, m_TextureMgrUI->wScaleInput, 1, "%6.5f", m_TextureMgrUI->wScaleButton );

    m_TransparencySlider.Init( this, m_TextureMgrUI->alphaSlider, m_TextureMgrUI->alphaInput, 1, "%6.5f", m_TextureMgrUI->alphaButton );

    m_FlipUButton.Init( this, m_TextureMgrUI->flipUButton );
    m_FlipWButton.Init( this, m_TextureMgrUI->flipWButton );

    m_TextureMgrUI->compChoice->callback( staticCB, this );
    m_TextureMgrUI->textureChoice->callback( staticCB, this );

    m_TextureMgrUI->textureNameInput->callback( staticCB, this );

    m_TextureMgrUI->addTextureButton->callback( staticCB, this );
    m_TextureMgrUI->delTextureButton->callback( staticCB, this );

    // Add GL 2D Window.
    Fl_Widget * w = m_TextureMgrUI->texGLGroup;
    m_TextureMgrUI->texGLGroup->begin();
    m_GlWin = new VSPGUI::VspSubGlWindow( w->x(), w->y(), w->w(), w->h(), DrawObj::VSP_TEX_PREVIEW );
    m_TextureMgrUI->texGLGroup->end();

    ResetCurrentSelected();
}

ManageTextureScreen::~ManageTextureScreen()
{
    delete m_GlWin;
    delete m_TextureMgrUI;
}

void ManageTextureScreen::Show()
{
    if( Update() )
    {
        m_FLTK_Window->show();
    }
}

void ManageTextureScreen::Hide()
{
    m_FLTK_Window->hide();
}

bool ManageTextureScreen::Update()
{
    assert( m_ScreenMgr );

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();

    if ( select_vec.size() != 1 )
    {
        Hide();
        return false;
    }

    // Redo list on each update.
    m_TextureMgrUI->compChoice->clear();
    m_CompDropDownList.clear();

    std::vector<Geom *> geomVec = veh->FindGeomVec( veh->GetGeomVec() );
    for( int i = 0; i < ( int )geomVec.size(); i++ )
    {
        CompDropDownItem item;
        item.GeomName = geomVec[i]->GetName();
        item.GeomID = geomVec[i]->GetID();

        // Hack to add duplicate names
        char str[256];
        sprintf( str, "%d", i );
        item.GUIIndex = m_TextureMgrUI->compChoice->add( str );

        m_CompDropDownList.push_back( item );
    }
    // Fill Hacked char array with correct names.
    for( int i = 0; i < ( int )m_CompDropDownList.size(); i++ )
    {
        m_TextureMgrUI->compChoice->replace( m_CompDropDownList[i].GUIIndex, m_CompDropDownList[i].GeomName.c_str() );
    }

    // Set compChoice to current selected and update texture dropdown list.
    for( int i = 0; i < ( int )m_CompDropDownList.size(); i++ )
    {
        if( m_CompDropDownList[i].GeomID == select_vec[0]->GetID() )
        {
            m_TextureMgrUI->compChoice->value( m_CompDropDownList[i].GUIIndex );

            // Update Texture Dropdown List. //

            // Redo texture list on each update.
            m_TextureMgrUI->textureChoice->clear();
            m_TexDropDownList.clear();

            // Clear preview window.
            VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();
            assert( viewport );
            viewport->getBackground()->removeImage();

            // Load Textures...
            TextureMgr * texMgr = select_vec[0]->m_GuiDraw.getTextureMgr();
            std::vector<Texture*> texInfos = texMgr->FindTextureVec( texMgr->GetTextureVec() );
            for( int j = 0; j < ( int )texInfos.size(); j++ )
            {
                TexDropDownItem item;
                item.TexInfo = texInfos[j];

                // Hack to add duplicate names
                char str[256];
                sprintf( str, "%d", j );
                item.GUIIndex = m_TextureMgrUI->textureChoice->add( str );

                m_TexDropDownList.push_back( item );
            }
            // Fill Hacked char array with correct names.
            for( int j = 0; j < ( int )m_TexDropDownList.size(); j++ )
            {
                m_TextureMgrUI->textureChoice->replace( m_TexDropDownList[j].GUIIndex, m_TexDropDownList[j].TexInfo->m_DisplayName.c_str() );
            }
            if( !m_TexDropDownList.empty() )
            {
                if( m_SelectedTexItem )
                {
                    m_TextureMgrUI->textureChoice->value( m_SelectedTexItem->GUIIndex );
                }
                else
                {
                    // On refresh list, if nothing is selected, pick last item on list.
                    m_TextureMgrUI->textureChoice->value( m_TexDropDownList[m_TexDropDownList.size() - 1].GUIIndex );
                }
                UpdateCurrentSelected();

                assert( m_SelectedTexItem );

                viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->
                    get2DTexture( m_SelectedTexItem->TexInfo->m_FileName.c_str() ) );
            }
            else
            {
                ResetCurrentSelected();

                // Force redraw empty list.
                m_TextureMgrUI->textureChoice->redraw();
            }

            // Update Sliders and Buttons.
            if( m_SelectedTexItem )
            {
                Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( m_SelectedTexItem->TexInfo->GetID() );

                m_TextureMgrUI->textureNameInput->value( info->m_DisplayName.c_str() );

                m_UScaleSlider.Update( info->m_UScale.GetID() );
                m_WScaleSlider.Update( info->m_WScale.GetID() );

                m_UPosSlider.Update( info->m_U.GetID() );
                m_WPosSlider.Update( info->m_W.GetID() );

                m_TransparencySlider.Update( info->m_Transparency.GetID() );

                m_FlipUButton.Update( info->m_FlipU.GetID() );
                m_FlipWButton.Update( info->m_FlipW.GetID() );
            }
            break;
        }
    }
    m_GlWin->redraw();
    return true;
}

void ManageTextureScreen::CallBack( Fl_Widget * w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if( w == m_TextureMgrUI->compChoice )
    {
        int selectedIndex = m_TextureMgrUI->compChoice->value();
        for( int i = 0; i < ( int )m_CompDropDownList.size(); i++ )
        {
            if( m_CompDropDownList[i].GUIIndex == selectedIndex )
            {
                veh->SetActiveGeom( m_CompDropDownList[i].GeomID );
                ResetCurrentSelected();
                break;
            }
        }
    }
    else if( w == m_TextureMgrUI->textureChoice )
    {
        UpdateCurrentSelected();
    }
    else if( w == m_TextureMgrUI->textureNameInput )
    {
        vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();
        Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( m_SelectedTexItem->TexInfo->GetID() );
        info->m_DisplayName = m_TextureMgrUI->textureNameInput->value();
    }
    else if( w == m_TextureMgrUI->addTextureButton )
    {
        vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();

        Fl_File_Chooser fc( ".", "TGA, JPG Files (*.{tga,jpg})", Fl_File_Chooser::SINGLE, "Read Texture?" );
        fc.show();

        while( fc.shown() )
        {
            Fl::wait();
        }

        if( fc.value() == NULL )
        {
            return;
        }
        select_vec[0]->m_GuiDraw.getTextureMgr()->AttachTexture( fc.value() );

        ResetCurrentSelected();
    }
    else if( w == m_TextureMgrUI->delTextureButton )
    {
        if( m_SelectedTexItem )
        {
            vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();
            select_vec[0]->m_GuiDraw.getTextureMgr()->RemoveTexture( m_SelectedTexItem->TexInfo->GetID() );

            ResetCurrentSelected();
        }
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageTextureScreen::UpdateCurrentSelected()
{
    m_SelectedTexItem = NULL;

    for( int i = 0; i < ( int )m_TexDropDownList.size(); i++ )
    {
        if( m_TexDropDownList[i].GUIIndex == m_TextureMgrUI->textureChoice->value() )
        {
            m_SelectedTexItem = &m_TexDropDownList[i];
            break;
        }
    }
}

void ManageTextureScreen::ResetCurrentSelected()
{
    m_SelectedTexItem = NULL;
}