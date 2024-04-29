#include "ManageBackground3DScreen.h"
#include "ScreenMgr.h"
#include "Background3DMgr.h"
#include "VSP_Geom_API.h"
#include "UnitConversion.h"

ManageBackground3DScreen::ManageBackground3DScreen( ScreenMgr * mgr ) : BasicScreen( mgr, 400, 800, "Background3D" )
{
    int tw = 15;
    int bw = 75;

    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                    m_MainLayout.GetRemainY() - 5.0);



    m_BorderLayout.SetButtonWidth( bw );
    m_BorderLayout.SetInputWidth( m_BorderLayout.GetW() * 0.5 - m_BorderLayout.GetButtonWidth() );


    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddInput( m_Background3DNameInput, "Name:" );

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddButton( m_AddBackground3DButton, "Add", m_BorderLayout.GetX() );

    m_BorderLayout.ForceNewLine();
    m_BorderLayout.SetSameLineFlag( false );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int width_array[] = {200, 100, 50, 0 };
    m_Background3DBrowser = m_BorderLayout.AddColResizeBrowser( width_array, 3, 75 );
    m_Background3DBrowser->callback( staticScreenCB, this );


    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2.0 );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_RemoveBackground3DButton, "Delete" );
    m_BorderLayout.AddButton( m_RemoveAllBackground3DsButton, "Delete All" );

    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_ShowAllBackground3DsButton, "Show All" );
    m_BorderLayout.AddButton( m_HideAllBackground3DsButton, "Hide All" );

    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw );
    m_BorderLayout.SetInputWidth( bw );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Image" );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2.0 );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.SetInputWidth( m_BorderLayout.GetRemainX() - 150 );
    m_BorderLayout.SetButtonWidth( bw );
    m_BorderLayout.AddOutput( m_FileOutput, "File:");
    m_BorderLayout.AddButton( m_FileSelect, "..." );
    m_BorderLayout.ForceNewLine();


    m_BorderLayout.SetInputWidth( 0.5 * ( m_BorderLayout.GetW() - 3 * m_BorderLayout.GetButtonWidth() ) );

    m_BorderLayout.AddOutput( m_ImageWOutput, "Width", "%5.0f" );
    m_BorderLayout.AddOutput( m_ImageHOutput, "Height", "%5.0f" );
    m_BorderLayout.AddButton( m_ImagePreviewButton, "Preview" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Modify Image" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.SetSliderWidth( 50 );
    m_BorderLayout.SetButtonWidth( ( m_BorderLayout.GetW() - m_BorderLayout.GetSliderWidth() ) / 4 );
    m_BorderLayout.SetChoiceButtonWidth( m_BorderLayout.GetButtonWidth() );

    m_BorderLayout.AddChoice( m_ImageRotChoice, "Rotate (CW)" );
    m_ImageRotChoice.AddItem( "0", vsp::ANG_0 );
    m_ImageRotChoice.AddItem( "90", vsp::ANG_90 );
    m_ImageRotChoice.AddItem( "180", vsp::ANG_180 );
    m_ImageRotChoice.AddItem( "270", vsp::ANG_270 );
    m_ImageRotChoice.UpdateItems();

    m_BorderLayout.AddButton( m_ImageAutoTransparentToggleButton, "Auto Trans." );

    m_BorderLayout.AddButton( m_ImageFlipLRToggleButton, "Flip LR" );
    m_BorderLayout.AddButton( m_ImageFlipUDToggleButton, "Flip UD" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.SetChoiceButtonWidth( bw );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Reference Point" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.SetSliderWidth( 0.5 * m_BorderLayout.GetW() - m_BorderLayout.GetButtonWidth() );

    m_BorderLayout.AddChoice( m_HAlignChoice, "Horizontal" );
    m_HAlignChoice.AddItem( "Left", vsp::ALIGN_LEFT );
    m_HAlignChoice.AddItem( "Center", vsp::ALIGN_CENTER );
    m_HAlignChoice.AddItem( "Right", vsp::ALIGN_RIGHT );
    m_HAlignChoice.AddItem( "Pixel", vsp::ALIGN_PIXEL );
    m_HAlignChoice.UpdateItems();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( 40 );
    m_BorderLayout.SetInputWidth( 40 );

    m_BorderLayout.AddSlider( m_ImageXSlider, "X", 1000, "%5.0f" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetInputWidth( bw );
    m_BorderLayout.SetButtonWidth( bw );

    m_BorderLayout.AddChoice( m_VAlignChoice, "Vertical" );
    m_VAlignChoice.AddItem( "Top", vsp::ALIGN_TOP );
    m_VAlignChoice.AddItem( "Middle", vsp::ALIGN_MIDDLE );
    m_VAlignChoice.AddItem( "Bottom", vsp::ALIGN_BOTTOM );
    m_VAlignChoice.AddItem( "Pixel", vsp::ALIGN_PIXEL );
    m_VAlignChoice.UpdateItems();

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( 40 );
    m_BorderLayout.SetInputWidth( 40 );

    m_BorderLayout.AddSlider( m_ImageYSlider, "Y", 1000, "%5.0f" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetInputWidth( bw );
    m_BorderLayout.SetButtonWidth( bw );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Visibility" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2.0 );
    m_BorderLayout.AddButton( m_VisibleBackground3DButton, "Visible" );
    m_BorderLayout.AddButton( m_RearVisibleBackground3DButton, "Both Sides" );
    m_BorderLayout.ForceNewLine();


    m_BorderLayout.AddButton( m_AlignVisibleBackground3DButton, "When Aligned", m_BorderLayout.GetW() * 0.5 );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( 40 );
    m_BorderLayout.SetInputWidth( 40 );

    m_BorderLayout.AddSlider( m_VisTolSlider, "Tol.", 10.0, "%4.2f" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetInputWidth( bw );
    m_BorderLayout.SetButtonWidth( bw );
    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "View" );

    m_BorderLayout.AddChoice( m_DirectionChoice, "Direction" );
    m_DirectionChoice.AddItem( "Left", vsp::VIEW_LEFT );
    m_DirectionChoice.AddItem( "Right", vsp::VIEW_RIGHT );
    m_DirectionChoice.AddItem( "Top", vsp::VIEW_TOP );
    m_DirectionChoice.AddItem( "Bottom", vsp::VIEW_BOTTOM );
    m_DirectionChoice.AddItem( "Front", vsp::VIEW_FRONT );
    m_DirectionChoice.AddItem( "Rear", vsp::VIEW_REAR );
    m_DirectionChoice.AddItem( "Other", vsp::VIEW_NONE );
    m_DirectionChoice.UpdateItems();

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_NXSlider, "X Norm", 1.0, "%5.4f" );
    m_BorderLayout.AddSlider( m_NYSlider, "Y Norm", 1.0, "%5.4f" );
    m_BorderLayout.AddSlider( m_NZSlider, "Z Norm", 1.0, "%5.4f" );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSlider( m_UpXSlider, "X Up", 1.0, "%5.4f" );
    m_BorderLayout.AddSlider( m_UpYSlider, "Y Up", 1.0, "%5.4f" );
    m_BorderLayout.AddSlider( m_UpZSlider, "Z Up", 1.0, "%5.4f" );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Scale" );

    m_BorderLayout.SetButtonWidth( tw );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_WScaleToggleButton, "" );

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw - tw );

    m_BorderLayout.AddSlider( m_WSlider, "Width", 1.0, "%5.4f" );
    m_BorderLayout.ForceNewLine();


    m_BorderLayout.SetButtonWidth( tw );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_HScaleToggleButton, "" );

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw - tw );

    m_BorderLayout.AddSlider( m_HSlider, "Height", 1.0, "%5.4f" );
    m_BorderLayout.ForceNewLine();


    m_BorderLayout.SetButtonWidth( tw );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_ResolutionScaleToggleButton, "" );

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw - tw );

    m_BorderLayout.AddSlider( m_ResolutionSlider, "Res.", 1.0, "%5.4f" );
    m_BorderLayout.ForceNewLine();

    m_ScaleGroup.Init( this );
    m_ScaleGroup.AddButton( m_WScaleToggleButton.GetFlButton() );
    m_ScaleGroup.AddButton( m_HScaleToggleButton.GetFlButton() );
    m_ScaleGroup.AddButton( m_ResolutionScaleToggleButton.GetFlButton() );

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddDividerBox( "Placement" );

    m_BorderLayout.AddChoice( m_DepthChoice, "Depth" );
    m_DepthChoice.AddItem( "Before", vsp::DEPTH_FRONT );
    m_DepthChoice.AddItem( "After", vsp::DEPTH_REAR );
    m_DepthChoice.AddItem( "Free", vsp::DEPTH_FREE );
    m_DepthChoice.UpdateItems();

    m_BorderLayout.AddSlider( m_XSlider, "X", 10, "%5.4f" );
    m_BorderLayout.AddSlider( m_YSlider, "Y", 10, "%5.4f" );
    m_BorderLayout.AddSlider( m_ZSlider, "Z", 10, "%5.4f" );
}

ManageBackground3DScreen::~ManageBackground3DScreen()
{
}

void ManageBackground3DScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

void ManageBackground3DScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ManageBackground3DScreen::Update()
{
    BasicScreen::Update();

    int h_pos = m_Background3DBrowser->hposition();
    int v_pos = m_Background3DBrowser->vposition();
    m_Background3DBrowser->clear();

    std::vector < Background3D * > bg3Ds = Background3DMgr.GetBackground3DVec();

    if ( bg3Ds.size() > 0 )
    {
        m_Background3DBrowser->column_char( ':' );

        char str[255];
        snprintf( str, sizeof( str ),  "@b@.NAME:@b@.DIRECTION:@b@.VISIBLE" );
        m_Background3DBrowser->add( str );

        // Add probes to browser.
        for( int i = 0; i < ( int )bg3Ds.size(); i++ )
        {
            string dir = bg3Ds[i]->GetDirectionName();
            string vis;
            if ( bg3Ds[i]->m_Visible() )
            {
                vis = "X";
            }
            snprintf( str, sizeof( str ),  "%s:%s:%s", bg3Ds[i]->GetName().c_str(), dir.c_str(), vis.c_str() );
            m_Background3DBrowser->add( str );
        }

        int index = Background3DMgr.GetCurrBackground3DIndex();
        if ( index >= 0 && index < ( int )bg3Ds.size() )
        {
            m_Background3DBrowser->select( index + 2 );
        }
        m_Background3DBrowser->hposition( h_pos );
        m_Background3DBrowser->vposition( v_pos );
    }


    Background3D * bg3D = Background3DMgr.GetCurrentBackground3D();

    if ( bg3D )
    {
        m_FileOutput.Activate();
        m_FileSelect.Activate();
        m_ImagePreviewButton.Activate();

        m_Background3DNameInput.Update( bg3D->GetName() );
        m_FileOutput.Update( StringUtil::truncateFileName( bg3D->m_BGFile, 35 ).c_str() );

        m_ImageWOutput.Update( bg3D->m_ImageW.GetID() );
        m_ImageHOutput.Update( bg3D->m_ImageH.GetID() );

        m_ImageFlipLRToggleButton.Update( bg3D->m_ImageFlipLR.GetID() );
        m_ImageFlipUDToggleButton.Update( bg3D->m_ImageFlipUD.GetID() );
        m_ImageAutoTransparentToggleButton.Update( bg3D->m_ImageAutoTransparent.GetID() );
        m_ImageRotChoice.Update( bg3D->m_ImageRot.GetID() );

        m_HAlignChoice.Update( bg3D->m_HAlign.GetID() );
        m_VAlignChoice.Update( bg3D->m_VAlign.GetID() );

        m_ImageXSlider.Update( bg3D->m_ImageX.GetID() );
        m_ImageYSlider.Update( bg3D->m_ImageY.GetID() );

        m_VisibleBackground3DButton.Update( bg3D->m_Visible.GetID() );
        m_RearVisibleBackground3DButton.Update( bg3D->m_RearVisible.GetID() );
        m_AlignVisibleBackground3DButton.Update( bg3D->m_VisAlign.GetID() );
        m_VisTolSlider.Update( bg3D->m_VisTol.GetID() );

        m_DirectionChoice.Update( bg3D->m_Direction.GetID() );

        m_NXSlider.Update( bg3D->m_NormX.GetID() );
        m_NYSlider.Update( bg3D->m_NormY.GetID() );
        m_NZSlider.Update( bg3D->m_NormZ.GetID() );

        m_UpXSlider.Update( bg3D->m_UpX.GetID() );
        m_UpYSlider.Update( bg3D->m_UpY.GetID() );
        m_UpZSlider.Update( bg3D->m_UpZ.GetID() );

        m_ScaleGroup.Update( bg3D->m_ScaleType.GetID() );
        m_WSlider.Update( bg3D->m_BackgroundWidth.GetID() );
        m_HSlider.Update( bg3D->m_BackgroundHeight.GetID() );
        m_ResolutionSlider.Update( bg3D->m_Resolution.GetID() );

        m_DepthChoice.Update( bg3D->m_DepthPos.GetID() );

        m_XSlider.Update( bg3D->m_X.GetID() );
        m_YSlider.Update( bg3D->m_Y.GetID() );
        m_ZSlider.Update( bg3D->m_Z.GetID() );



        m_NXSlider.Activate();
        m_NYSlider.Activate();
        m_NZSlider.Activate();
        m_UpXSlider.Activate();
        m_UpYSlider.Activate();
        m_UpZSlider.Activate();
        m_DepthChoice.Deactivate();
        if ( bg3D->m_Direction() != vsp::VIEW_NONE )
        {
            m_NXSlider.Deactivate();
            m_NYSlider.Deactivate();
            m_NZSlider.Deactivate();
            m_UpXSlider.Deactivate();
            m_UpYSlider.Deactivate();
            m_UpZSlider.Deactivate();

            m_DepthChoice.Activate();
        }

        m_ImageXSlider.Deactivate();
        if ( bg3D->m_HAlign() == vsp::ALIGN_PIXEL )
        {
            m_ImageXSlider.Activate();
        }

        m_ImageYSlider.Deactivate();
        if ( bg3D->m_VAlign() == vsp::ALIGN_PIXEL )
        {
            m_ImageYSlider.Activate();
        }

        m_WSlider.Activate();
        m_HSlider.Activate();
        m_ResolutionSlider.Activate();
        m_WScaleToggleButton.Activate();
        m_HScaleToggleButton.Activate();
        m_ResolutionScaleToggleButton.Activate();
        if ( bg3D->m_ScaleType() == vsp::SCALE_WIDTH )
        {
            m_HSlider.Deactivate();
            m_ResolutionSlider.Deactivate();
        }
        else if ( bg3D->m_ScaleType() == vsp::SCALE_HEIGHT )
        {
            m_WSlider.Deactivate();
            m_ResolutionSlider.Deactivate();
        }
        else // vsp::SCALE_RESOLUTION
        {
            m_WSlider.Deactivate();
            m_HSlider.Deactivate();
        }

        m_VisTolSlider.Deactivate();
        if ( bg3D->m_VisAlign() )
        {
            m_VisTolSlider.Activate();
        }


        m_XSlider.Activate();
        m_YSlider.Activate();
        m_ZSlider.Activate();
        if ( bg3D->m_Direction() != vsp::VIEW_NONE )
        {
            if ( bg3D->m_DepthPos() != vsp::DEPTH_FREE )
            {
                if ( bg3D->m_Direction() == vsp::VIEW_FRONT || bg3D->m_Direction() == vsp::VIEW_REAR )
                {
                    m_XSlider.Deactivate();
                }
                else if ( bg3D->m_Direction() == vsp::VIEW_LEFT || bg3D->m_Direction() == vsp::VIEW_RIGHT )
                {
                    m_YSlider.Deactivate();
                }
                else
                {
                    m_ZSlider.Deactivate();
                }
            }
        }

    }
    else
    {
        m_VisibleBackground3DButton.Deactivate();
        m_RearVisibleBackground3DButton.Deactivate();
        m_AlignVisibleBackground3DButton.Deactivate();
        m_VisTolSlider.Deactivate();

        m_FileOutput.Deactivate();
        m_FileSelect.Deactivate();

        m_DirectionChoice.Deactivate();

        m_NXSlider.Deactivate();
        m_NYSlider.Deactivate();
        m_NZSlider.Deactivate();

        m_UpXSlider.Deactivate();
        m_UpYSlider.Deactivate();
        m_UpZSlider.Deactivate();

        m_WScaleToggleButton.Deactivate();
        m_HScaleToggleButton.Deactivate();
        m_ResolutionScaleToggleButton.Deactivate();

        m_WSlider.Deactivate();
        m_HSlider.Deactivate();
        m_ResolutionSlider.Deactivate();

        m_HAlignChoice.Deactivate();
        m_VAlignChoice.Deactivate();
        m_ImageXSlider.Deactivate();
        m_ImageYSlider.Deactivate();

        m_ImageWOutput.Deactivate();
        m_ImageHOutput.Deactivate();
        m_ImagePreviewButton.Deactivate();

        m_ImageFlipLRToggleButton.Deactivate();
        m_ImageFlipUDToggleButton.Deactivate();
        m_ImageAutoTransparentToggleButton.Deactivate();
        m_ImageRotChoice.Deactivate();

        m_XSlider.Deactivate();
        m_YSlider.Deactivate();
        m_ZSlider.Deactivate();

        m_DepthChoice.Deactivate();
    }

    m_FLTK_Window->redraw();
    return false;
}


void ManageBackground3DScreen::GuiDeviceCallBack( GuiDevice* device )
{
    if ( device == &m_AddBackground3DButton )
    {
        Background3DMgr.CreateAndAddBackground3D();
    }
    else if ( device == &m_RemoveBackground3DButton )
    {
        Background3DMgr.DelBackground3D( Background3DMgr.GetCurrBackground3DIndex() );
    }
    else if ( device == &m_RemoveAllBackground3DsButton )
    {
        Background3DMgr.DelAllBackground3Ds();
    }
    else if ( device == &m_ShowAllBackground3DsButton )
    {
        Background3DMgr.ShowAllBackground3Ds();
    }
    else if ( device == &m_HideAllBackground3DsButton )
    {
        Background3DMgr.HideAllBackground3Ds();
    }
    else if ( device == &m_Background3DNameInput )
    {
        Background3D * bg3D = Background3DMgr.GetCurrentBackground3D();
        if ( bg3D )
        {
            bg3D->SetName( m_Background3DNameInput.GetString() );
        }
    }
    else if ( device == &m_FileSelect )
    {
        std::string fileName = m_ScreenMgr->FileChooser( "Select Image File", "*.{jpg,png,tga,bmp,gif}" );

        if( !fileName.empty() )
        {

            Background3D * bg3D = Background3DMgr.GetCurrentBackground3D();
            if ( bg3D )
            {
                bg3D->m_BGFile = fileName;
                bg3D->UpdateImageInfo();
            }

        }
    }
    else if ( device == &m_ImagePreviewButton )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_BACKGROUND3D_PREVIEW_SCREEN );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageBackground3DScreen::CallBack( Fl_Widget * w )
{
    if ( w == m_Background3DBrowser )
    {
        int sel = m_Background3DBrowser->value();
        Background3DMgr.SetCurrBackground3DIndex( sel - 2 );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageBackground3DScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

void ManageBackground3DScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    Background3DMgr.LoadDrawObjs( draw_obj_vec );
}

