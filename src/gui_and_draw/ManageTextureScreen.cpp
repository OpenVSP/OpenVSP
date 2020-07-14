
#include "ManageTextureScreen.h"

ManageTextureScreen::ManageTextureScreen( ScreenMgr * mgr ) : BasicScreen( mgr, 290, 660, "Texture Mgr")
{
    int yGap = 24;
    int yPadding = 7;
    int sliderButtonWidth = 90;
    int sliderInputWidth = 55;
    int smallButtonWidth = 60;
    int borderPaddingWidth = 5;
    int yGapForDisplay = 294;

    m_GeomIndex = 0;
    m_EditIndex = 0;
    m_LastActiveGeomIndex = 0;

    m_ActiveGeomChanged = false;
    m_ThisGuiDeviceWasCalledBack = false;
   
    m_FLTK_Window->callback( staticCloseCB, this );

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

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

    m_BorderLayout.SetChoiceButtonWidth( smallButtonWidth );
    m_BorderLayout.AddChoice( m_EditChoice, "Edit:" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddButton( m_DeleteButton , "Delete");
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetButtonWidth( smallButtonWidth );
    m_BorderLayout.AddInput( m_NameInput, "Name:" );
    m_BorderLayout.AddYGap();
     
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( sliderButtonWidth );
    m_BorderLayout.SetInputWidth( sliderInputWidth );

    m_BorderLayout.AddSlider( m_UPosSlider, "U Position", 1, "%6.5f" );
    m_BorderLayout.AddYGap();
    m_BorderLayout.AddSlider( m_WPosSlider, "W Position", 1, "%6.5f" );
    m_BorderLayout.AddY( yGap/2 );

    m_BorderLayout.AddSlider( m_UScaleSlider, "U Scale", 1, "%6.5f" );
    m_BorderLayout.AddYGap();
    m_BorderLayout.AddSlider( m_WScaleSlider, "W Scale", 1, "%6.5f" );
    m_BorderLayout.AddY( yGap/2 );

    m_BorderLayout.AddSlider( m_TransparencySlider, "Transparency", 1, "%6.5f" );
    m_BorderLayout.AddY( yGap/2 );

    m_BorderLayout.AddDividerBox( "Texture Coords" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetCanvasWidth()/2 +(borderPaddingWidth*2));

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.AddButton( m_FlipUToggle, "Flip U" );
    m_BorderLayout.AddX( borderPaddingWidth*2 );
    m_BorderLayout.AddButton( m_FlipWToggle, "Flip W" );

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
    m_FLTK_Window->show();
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

    //This Updates GeomChoice list based on number of existing geoms
    //It also checks if the Active Geom has changed since last update and updates an index and a bool
    vector < Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    if ( geom_vec.size() > 0 )
    {
        for ( int i = 0; i < ( int )geom_vec.size(); i++ )
        {
            sprintf( str, "%d.  %s", i + 1, geom_vec[i]->GetName().c_str() );
            m_GeomChoice.AddItem( str );
            if ( veh->IsGeomActive( geom_vec[i]->GetID() ) )
            {
                if ( i != m_LastActiveGeomIndex )
                {
                    m_ActiveGeomChanged = true;
                    m_LastActiveGeomIndex = i;
                }
            }
        }
        m_GeomChoice.UpdateItems();
    }

    //We set the m_GeomChoice index based on what changed since last Update
    if ( m_ThisGuiDeviceWasCalledBack )
    {
        m_GeomChoice.SetVal( m_GeomIndex );
        m_ThisGuiDeviceWasCalledBack = false;
    }
    else if ( m_ActiveGeomChanged )
    {
        m_GeomChoice.SetVal( m_LastActiveGeomIndex );
        m_ActiveGeomChanged = false;
    }
    else //Geom was added via Geom Browser
    {
        m_GeomChoice.SetVal( ( int )geom_vec.size()-1 );
    }

    //This section updates the textures list in m_EditChoice
    vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();
    if ( ( int )select_vec.size() > 0)
    {
        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();

        m_EditChoice.ClearItems();

        if ( ( int )textures.size() > 0)
        {
            for ( int i = 0; i < ( int )textures.size(); i++ )
            {
                string texture_file_name = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( textures[i] )->GetName();
                sprintf( str, "%d.  %s", i + 1, texture_file_name.c_str() );
                m_EditChoice.AddItem( str );
            }
        }
        else
        {
            m_EditChoice.AddItem( "" );
        }
        m_EditChoice.UpdateItems();
        m_EditChoice.SetVal( m_EditIndex );
    }
    
    //This section updates m_NameInput with name given to texture
    select_vec = veh->GetActiveGeomPtrVec();
    if ( ( int )select_vec.size() > 0 )
    {
        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();
        if ( ( int )textures.size() > 0 && m_EditIndex <= ( int )textures.size())
        {
            Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_EditIndex] );
            m_NameInput.Update( info->GetName().c_str() );
        }
        else
        {
            m_NameInput.Update( "" );
        }
    }

    //Update Preview Window.
    VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();
    assert( viewport );
    viewport->getBackground()->removeImage();
    
    // Load Textures
    select_vec = veh->GetActiveGeomPtrVec();
    if ( ( int )select_vec.size() > 0 )
    {
        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();
        if ( ( int )textures.size() > 0 )
        {
            string texture_file_name = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_EditIndex] )->m_FileName;
            viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( texture_file_name.c_str() ) );
        }
    }
    m_GlWin->redraw();

    //Update Sliders and Toggles
    select_vec = veh->GetActiveGeomPtrVec();
    if ( ( int )select_vec.size() > 0 )
    {
        m_BorderLayout.GetGroup()->activate();
        
        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();
        if ( ( int )textures.size() > 0 && m_EditIndex <= ( int )textures.size() )
        {
            Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_EditIndex] );

            m_UPosSlider.Update( info->m_U.GetID() );
            m_WPosSlider.Update( info->m_W.GetID() );

            m_UScaleSlider.Update( info->m_UScale.GetID() );
            m_WScaleSlider.Update( info->m_WScale.GetID() );

            m_TransparencySlider.Update( info->m_Transparency.GetID() );

            m_FlipUToggle.Update( info->m_FlipU.GetID() );
            m_FlipWToggle.Update( info->m_FlipW.GetID() ); 
        }
        else
        {
            m_UPosSlider.Deactivate();
            m_WPosSlider.Deactivate();

            m_UScaleSlider.Deactivate();
            m_WScaleSlider.Deactivate();

            m_TransparencySlider.Deactivate();

            m_FlipUToggle.Deactivate();
            m_FlipWToggle.Deactivate();
        }
    }
    else
    {
        m_BorderLayout.GetGroup()->deactivate();
    }

    return true;
}

void ManageTextureScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();
    assert( viewport );
    viewport->getBackground()->removeImage();

    if ( device == &m_AddButton )
    {
        vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();

        std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                "Select Image File", "*.{tga,png,jpg}", false );

        if( !fileName.empty() )
        {
            if ( ( int )select_vec.size() > 0 )
            {
                select_vec[0]->m_GuiDraw.getTextureMgr()->AttachTexture( fileName.c_str() );
            }
        }

        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();
        if ( ( int )textures.size() > 0 )
        {
            m_EditIndex = ( int )textures.size()-1;
        }

        m_GeomIndex = m_GeomChoice.GetVal();
    }
    else if ( device == &m_DeleteButton )
    {
        vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();
        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();

        if ( ( int )textures.size() > 0 )
        {
            string texture_file_id = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_EditIndex] )->GetID();
            select_vec[0]->m_GuiDraw.getTextureMgr()->RemoveTexture( texture_file_id );
            if ( ( int )textures.size() > 0 && m_EditIndex > 0)
            {
                m_EditIndex--;
            }  
        }

        m_GeomIndex = m_GeomChoice.GetVal();
    }
    else if ( device == &m_NameInput )
    {
        vector< Geom* > select_vec = veh->GetActiveGeomPtrVec();
        vector <string> textures = select_vec[0]->m_GuiDraw.getTextureMgr()->GetTextureVec();
        Texture * info = select_vec[0]->m_GuiDraw.getTextureMgr()->FindTexture( textures[m_EditIndex] );
        info->SetName( m_NameInput.GetString() );
    }
    else if ( device == &m_GeomChoice )
    {
        m_GeomIndex = m_GeomChoice.GetVal();

        vector< string > geom_id_vec = veh->GetGeomVec();
        string geom_id = geom_id_vec[m_GeomIndex];
        veh->ClearActiveGeom();
        veh->SetActiveGeom( geom_id );
    }
    else if ( device == &m_EditChoice )
    {
        m_EditIndex = m_EditChoice.GetVal();
    }

    m_ThisGuiDeviceWasCalledBack = true;
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
