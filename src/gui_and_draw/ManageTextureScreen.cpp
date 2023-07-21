
#include "ManageTextureScreen.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Background.h"
#include "GraphicSingletons.h"

ManageTextureScreen::ManageTextureScreen( ScreenMgr * mgr ) : BasicScreen( mgr, 290, 630, "Texture Mgr")
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    int yPadding = 7;
    int sliderButtonWidth = 90;
    int sliderInputWidth = 55;
    int smallButtonWidth = 60;
    int borderPaddingWidth = 5;
    int yGapForDisplay = 294;

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( yPadding );
    m_MainLayout.AddX( borderPaddingWidth );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - borderPaddingWidth,
        m_MainLayout.GetRemainY() - borderPaddingWidth );

    m_BorderLayout.SetChoiceButtonWidth( smallButtonWidth );
    m_BorderLayout.AddChoice( m_GeomChoice, "Comp:" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Texture" );
    m_BorderLayout.AddY( yGapForDisplay );

    m_BorderLayout.AddButton( m_AddButton , "Add...");
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSubGroupLayout( m_ActiveTextureLayout,
                                      m_BorderLayout.GetW(),
                                      m_BorderLayout.GetRemainX() );

    m_BorderLayout.AddY( m_ActiveTextureLayout.GetH() );

    m_ActiveTextureLayout.SetChoiceButtonWidth( smallButtonWidth );
    m_ActiveTextureLayout.AddChoice( m_EditChoice, "Edit:" );
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.AddButton( m_DeleteButton , "Delete");
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.SetButtonWidth( smallButtonWidth );
    m_ActiveTextureLayout.AddInput( m_NameInput, "Name:" );
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.SetFitWidthFlag( true );
    m_ActiveTextureLayout.SetButtonWidth( sliderButtonWidth );
    m_ActiveTextureLayout.SetInputWidth( sliderInputWidth );

    m_ActiveTextureLayout.AddSlider( m_UPosSlider, "U Position", 1, "%6.5f" );
    m_ActiveTextureLayout.AddSlider( m_WPosSlider, "W Position", 1, "%6.5f" );
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.AddSlider( m_UScaleSlider, "U Scale", 1, "%6.5f" );
    m_ActiveTextureLayout.AddSlider( m_WScaleSlider, "W Scale", 1, "%6.5f" );
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.AddSlider( m_TransparencySlider, "Transparency", 1, "%6.5f" );
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.AddDividerBox( "Texture Coords" );
    m_ActiveTextureLayout.AddYGap();

    m_ActiveTextureLayout.SetButtonWidth( m_ActiveTextureLayout.GetCanvasWidth()/2 +(borderPaddingWidth*2));

    m_ActiveTextureLayout.SetFitWidthFlag( false );
    m_ActiveTextureLayout.SetSameLineFlag( true );

    m_ActiveTextureLayout.AddButton( m_FlipUToggle, "Flip U" );
    m_ActiveTextureLayout.AddX( borderPaddingWidth*2 );
    m_ActiveTextureLayout.AddButton( m_FlipWToggle, "Flip W" );

     //// Add GL 2D Window.
    texGLGroup = new Fl_Group(5, 75, 280, 280, "GL_WIN");
    texGLGroup->box(FL_BORDER_BOX);
    texGLGroup->color((Fl_Color)29);
    texGLGroup->align(Fl_Align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE));
    Fl_Widget * w = texGLGroup;
    texGLGroup->begin();
    m_GlWin = new VSPGUI::VspSubGlWindow( w->x(), w->y(), w->w(), w->h(), DrawObj::VSP_TEX_PREVIEW );
    texGLGroup->end();

}

ManageTextureScreen::~ManageTextureScreen()
{
    delete m_GlWin;
}

void ManageTextureScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void ManageTextureScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageTextureScreen::Update()
{
    char str[256];

    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return false;
    }

    //This Updates GeomChoice list based on all available Geoms
    vector < Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    m_GeomChoice.ClearItems();
    m_GeomIDVec.clear();

    map <string, int> CompIDMap; // map of Geom IDs to index in geom_vec
    int icomp = 0;

    for ( int i = 0; i < ( int )geom_vec.size(); i++ )
    {
        int geom_type = geom_vec[i]->GetType().m_Type;

        if ( geom_type != BLANK_GEOM_TYPE &&
             geom_type != HINGE_GEOM_TYPE ) // TODO: Any other types that don't support textures?
        {
            snprintf( str, sizeof( str ),  "%d.  %s", i + 1, geom_vec[i]->GetName().c_str() );
            m_GeomChoice.AddItem( str );

            m_GeomIDVec.push_back( geom_vec[i]->GetID() );
            CompIDMap[geom_vec[i]->GetID()] = icomp;
            icomp++;
        }
    }

    m_GeomChoice.UpdateItems();

    Geom* currgeom = veh->FindGeom( m_SelectedGeomID );

    if ( ( m_SelectedGeomID.length() == 0 && m_GeomIDVec.size() > 0 ) || 
        ( !currgeom && m_GeomIDVec.size() > 0 ) )
    {
        // Handle default case
        m_SelectedGeomID = m_GeomIDVec[0];
    }

    m_GeomChoice.SetVal( CompIDMap[m_SelectedGeomID] );

    // This section updates the textures list in m_EditChoice
    m_EditChoice.ClearItems();

    // Clear Preview Window.
    VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();
    assert( viewport );
    viewport->getBackground()->removeImage();

    Texture* currtex = NULL; // Pointer to the currently selected texture

    if ( currgeom )
    {
        vector < string > texture_vec = currgeom->m_GuiDraw.getTextureMgr()->GetTextureVec();

        for ( int i = 0; i < ( int )texture_vec.size(); i++ )
        {
            string texture_file_name = currgeom->m_GuiDraw.getTextureMgr()->FindTexture( texture_vec[i] )->GetName();
            snprintf( str, sizeof( str ),  "%d.  %s", i + 1, texture_file_name.c_str() );
            m_EditChoice.AddItem( str );
        }

        m_EditChoice.UpdateItems();

        if ( texture_vec.size() > 0 && ( m_SelectedTextureIndex < 0 || m_SelectedTextureIndex >= texture_vec.size() ) )
        {
            m_SelectedTextureIndex = 0;
        }

        if ( texture_vec.size() > 0 && m_SelectedTextureIndex >= 0 && m_SelectedTextureIndex < texture_vec.size() )
        {
            currtex = currgeom->m_GuiDraw.getTextureMgr()->FindTexture( texture_vec[m_SelectedTextureIndex] );

            m_EditChoice.SetVal( m_SelectedTextureIndex );
        }
    }
    
    if ( geom_vec.size() == 0 )
    {
        m_BorderLayout.GetGroup()->deactivate();
    }
    else
    {
        m_BorderLayout.GetGroup()->activate();
    }

    // Update Texture Sliders and Toggles
    if ( currtex )
    {
        m_ActiveTextureLayout.GetGroup()->activate();

        m_NameInput.Update( currtex->GetName().c_str() ); // updates m_NameInput with name given to texture

        // Load Textures
        string texture_file_name = currtex->m_FileName;
        viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( texture_file_name.c_str() ) );

        m_UPosSlider.Update( currtex->m_U.GetID() );
        m_WPosSlider.Update( currtex->m_W.GetID() );

        m_UScaleSlider.Update( currtex->m_UScale.GetID() );
        m_WScaleSlider.Update( currtex->m_WScale.GetID() );

        m_TransparencySlider.Update( currtex->m_Transparency.GetID() );

        m_FlipUToggle.Update( currtex->m_FlipU.GetID() );
        m_FlipWToggle.Update( currtex->m_FlipW.GetID() );
    }
    else
    {
        m_ActiveTextureLayout.GetGroup()->deactivate();
    }

    m_GlWin->redraw();
    m_FLTK_Window->redraw();
    return false;
}

void ManageTextureScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( device == &m_GeomChoice )
    {
        m_SelectedGeomID = m_GeomIDVec[m_GeomChoice.GetVal()];
    }
    else // Every other GuiDevice relies on a valid Geom pointer
    {
        Geom* currgeom = veh->FindGeom( m_SelectedGeomID );
        if ( !currgeom )
        {
            return;
        }

        if ( device == &m_AddButton )
        {
            std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                "Select Image File", "*.{tga,png,jpg}", false );

            if ( !fileName.empty() )
            {
                currgeom->m_GuiDraw.getTextureMgr()->AttachTexture( fileName.c_str() );

                vector < string > textures = currgeom->m_GuiDraw.getTextureMgr()->GetTextureVec();

                // Select the new texture
                m_SelectedTextureIndex = (int)textures.size() - 1;
            }
        }
        else if ( device == &m_DeleteButton )
        {
            vector < string > textures = currgeom->m_GuiDraw.getTextureMgr()->GetTextureVec();

            if ( textures.size() > 0 && m_SelectedTextureIndex >= 0 && m_SelectedTextureIndex < textures.size() )
            {
                string texture_id = currgeom->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_SelectedTextureIndex] )->GetID();
                currgeom->m_GuiDraw.getTextureMgr()->RemoveTexture( texture_id );

                m_SelectedTextureIndex--;
            }
        }
        else if ( device == &m_NameInput )
        {
            vector < string > textures = currgeom->m_GuiDraw.getTextureMgr()->GetTextureVec();

            if ( textures.size() > 0 && m_SelectedTextureIndex >= 0 && m_SelectedTextureIndex < textures.size() )
            {
                Texture * currtex = currgeom->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_SelectedTextureIndex] );
                if ( currtex )
                {
                    currtex->SetName( m_NameInput.GetString() );
                }
            }
        }
        else if ( device == &m_EditChoice )
        {
            m_SelectedTextureIndex = m_EditChoice.GetVal();
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageTextureScreen::CallBack( Fl_Widget * w )
{
    assert( m_ScreenMgr );
    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageTextureScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}
