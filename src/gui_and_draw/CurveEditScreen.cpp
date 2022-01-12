//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// CurveEditScreen.cpp - GUI for EditCurveXSec control point manipulation
// Justin Gravett, ESAero, 7/10/19
//////////////////////////////////////////////////////////////////////

#include "CurveEditScreen.h"
#include "BORGeom.h"
#include "WingGeom.h"

#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Camera.h"
#include "Background.h"
#include "GraphicSingletons.h"

#include "VspUtil.h"

//==== Constructor ====//
CurveEditScreen::CurveEditScreen( ScreenMgr* mgr ) : TabScreen( mgr, 750, 615+17, "Edit Curve", 180, 425 )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    Fl_Group* xsec_tab = AddTab( "Curve" );
    Fl_Group* draw_tab = AddTab( "Display" );

    xsec_tab->show();

    //==== XSec Tab ====//
    Fl_Group* xsec_group = AddSubGroup( xsec_tab, 5 );
    m_XSecLayout.SetGroupAndScreen( xsec_group, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( 7 );
    m_MainLayout.AddX( 5 );

    m_XSecLayout.AddDividerBox( "General" );
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetSameLineFlag( true );

    m_XSecLayout.SetChoiceButtonWidth( m_XSecLayout.GetRemainX() / 3 );
    m_XSecLayout.SetSliderWidth( m_XSecLayout.GetRemainX() / 3 );
    m_XSecLayout.SetButtonWidth( m_XSecLayout.GetRemainX() / 3 );

    m_ShapeChoice.AddItem( "Circle" );
    m_ShapeChoice.AddItem( "Ellipse" );
    m_ShapeChoice.AddItem( "Rectangle" );
    m_XSecLayout.AddChoice( m_ShapeChoice, "Shape" );

    m_XSecLayout.AddButton( m_InitShapeButton, "Init Shape" );
    m_XSecLayout.ForceNewLine();

    m_XSecLayout.AddButton( m_ClosedCurveToggle, "Close Curve" );
    m_XSecLayout.AddButton( m_AbsDimToggle, "View Abs" ); // Absolute dimensions flag
    m_XSecLayout.AddButton( m_SymToggle, "R-L Symmetry" );

    m_XSecLayout.ForceNewLine();
    m_XSecLayout.AddYGap();

    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.SetFitWidthFlag( false );

    m_XSecLayout.SetInputWidth( m_XSecLayout.GetRemainX() - m_XSecLayout.GetButtonWidth() );
    m_XSecLayout.AddOutput( m_CurveType, "Current Type:" );

    m_XSecLayout.ForceNewLine();

    m_ConvertChoice.AddItem( "Linear" );
    m_ConvertChoice.AddItem( "Spline (PCHIP)" );
    m_ConvertChoice.AddItem( "Cubic Bezier" );

    m_XSecLayout.AddChoice( m_ConvertChoice, "Convert to:" );

    m_XSecLayout.AddButton( m_ConvertButton, "Convert" );
    m_XSecLayout.ForceNewLine();
    m_XSecLayout.AddYGap();

    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );

    m_XSecLayout.AddDividerBox( "Scale XSec" );

    m_XSecLayout.InitWidthHeightVals();
    m_XSecLayout.SetButtonWidth( m_XSecLayout.GetRemainX() / 3 );

    vector < string > xsec_driver_labels;
    xsec_driver_labels.resize( vsp::NUM_XSEC_DRIVER );
    xsec_driver_labels[vsp::WIDTH_XSEC_DRIVER] = string( "Width" );
    xsec_driver_labels[vsp::HEIGHT_XSEC_DRIVER] = "Height";
    xsec_driver_labels[vsp::AREA_XSEC_DRIVER] = "Area";
    xsec_driver_labels[vsp::HWRATIO_XSEC_DRIVER] = "H/W Ratio";

    m_XSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
    m_XSecLayout.AddDriverGroupBank( m_XSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );

    m_XSecLayout.AddX( 2 * 17 + 1 );
    m_XSecLayout.AddSlider( m_DepthSlider, "Depth", 1, "%6.5f" );

    m_XSecLayout.AddYGap();
    m_XSecLayout.SetSameLineFlag( false );
    m_XSecLayout.SetFitWidthFlag( true );

    m_XSecLayout.AddButton( m_ReparameterizeButton, "Reparameterize by Arc Length" );

    m_XSecLayout.AddYGap();

    //==== Add Split Button ====//
    m_XSecLayout.AddDividerBox( "Split Curve" );

    m_XSecLayout.AddSlider( m_SplitPtSlider, "U Parameter", 1, "%3.2f");

    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.SetFitWidthFlag( false );

    m_XSecLayout.SetButtonWidth( m_XSecLayout.GetRemainX() / 2 );

    m_XSecLayout.AddButton( m_SplitButton, "Split U" );
    m_XSecLayout.AddButton( m_SplitPickButton, "Split Pick" );

    m_XSecLayout.ForceNewLine();
    m_XSecLayout.SetSameLineFlag( false );
    m_XSecLayout.SetFitWidthFlag( true );

    m_XSecLayout.AddYGap();

    //==== Add Delete Button and Toggle ====//
    m_XSecLayout.AddDividerBox( "Selected Point" );
    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.InitWidthHeightVals();

    m_XSecLayout.AddIndexSelector( m_PntSelector, NULL );

    m_XSecLayout.AddYGap();

    m_XSecLayout.AddDividerBox( "Delete Point" );

    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetButtonWidth( m_XSecLayout.GetRemainX() / 2 );
    m_XSecLayout.AddButton( m_DelButton, "Delete Selected" );
    m_XSecLayout.AddButton( m_DelPickButton, "Delete Pick" );
    m_XSecLayout.ForceNewLine();
    m_XSecLayout.AddYGap();

    //==== Draw Tab ====//
    Fl_Group* draw_group = AddSubGroup( draw_tab, 5 );
    m_DrawLayout.SetGroupAndScreen( draw_group, this );

    m_DrawLayout.AddDividerBox( "Line Color" );

    m_DrawLayout.AddColorPicker( m_ColorPicker );

    int button_w = m_DrawLayout.GetRemainX() / 4;

    m_DrawLayout.SetSameLineFlag( true );
    m_DrawLayout.SetFitWidthFlag( false );

    m_DrawLayout.SetButtonWidth( 20 );
    m_DrawLayout.AddButton( m_PointColorCheck, "" );
    m_DrawLayout.SetButtonWidth( button_w + 5 );
    m_DrawLayout.SetFitWidthFlag( true );
    m_DrawLayout.AddSlider( m_PointColorWheelSlider, "Point Color", 100, "%5.0f" );

    m_DrawLayout.ForceNewLine();
    m_DrawLayout.AddYGap();
    m_DrawLayout.SetSameLineFlag( false );
    m_DrawLayout.SetButtonWidth( button_w );

    m_DrawLayout.AddSlider( m_PointSizeSlider, "Point Size", 10, "%7.4f" );
    m_DrawLayout.AddSlider( m_LineThicknessSlider, "Line Thick", 10, "%7.4f" );

    m_DrawLayout.AddYGap();

    m_DrawLayout.AddDividerBox( "Background" );
    
    m_DrawLayout.SetSameLineFlag( true );
    m_DrawLayout.SetFitWidthFlag( false );
    m_DrawLayout.SetButtonWidth( m_DrawLayout.GetRemainX() / 3 );

    m_DrawLayout.AddButton( m_BorderToggle, "Border" );
    m_DrawLayout.AddButton( m_AxisToggle, "Axis" );
    m_DrawLayout.AddButton( m_GridToggle, "Grid" );

    m_BorderToggle.GetFlButton()->value( 1 );
    m_AxisToggle.GetFlButton()->value( 1 );
    m_GridToggle.GetFlButton()->value( 1 );

    m_DrawLayout.ForceNewLine();
    m_DrawLayout.AddYGap();
    m_DrawLayout.SetSameLineFlag( false );
    m_DrawLayout.SetFitWidthFlag( true );

    m_DrawLayout.AddButton( m_ImageToggle, "Image" );
    m_ImageToggle.GetFlButton()->value( 0 );

    m_DrawLayout.AddYGap();

    m_DrawLayout.AddSubGroupLayout( m_BackgroundImageLayout, m_DrawLayout.GetRemainX(), m_DrawLayout.GetRemainY() - 2 * m_DrawLayout.GetStdHeight() );

    m_BackgroundImageLayout.SetFitWidthFlag( false );
    m_BackgroundImageLayout.SetSameLineFlag( true );

    m_BackgroundImageLayout.SetInputWidth( m_BackgroundImageLayout.GetRemainX() - 80 );
    m_BackgroundImageLayout.SetButtonWidth( 50 );
    m_BackgroundImageLayout.AddOutput( m_ImageFileOutput, "File:" );
    m_BackgroundImageLayout.SetButtonWidth( 30 );
    m_BackgroundImageLayout.AddButton( m_ImageFileSelect, "..." );
    m_BackgroundImageLayout.ForceNewLine();
    m_BackgroundImageLayout.AddYGap();

    m_BackgroundImageLayout.SetFitWidthFlag( false );
    m_BackgroundImageLayout.SetSameLineFlag( true );
    m_BackgroundImageLayout.SetButtonWidth( ( m_BackgroundImageLayout.GetW() / 3 ) + 20 );

    m_BackgroundImageLayout.AddButton( m_PreserveImageAspect, "Preserve Aspect" );
    m_BackgroundImageLayout.SetButtonWidth( ( m_BackgroundImageLayout.GetW() / 3 ) - 10 );
    m_BackgroundImageLayout.AddButton( m_LockImageToggle, "Lock Image" );
    m_BackgroundImageLayout.AddButton( m_FlipImageToggle, "Flip Image" );

    m_BackgroundImageLayout.SetFitWidthFlag( true );
    m_BackgroundImageLayout.SetSameLineFlag( false );
    m_BackgroundImageLayout.ForceNewLine();
    m_BackgroundImageLayout.AddYGap();
    m_BackgroundImageLayout.SetButtonWidth( m_BackgroundImageLayout.GetRemainX() / 4 );
    m_BackgroundImageLayout.SetInputWidth( 50 );

    m_BackgroundImageLayout.AddSlider( m_ImageWScale, "W Scale", 1.0, "%7.3f" );
    m_BackgroundImageLayout.AddSlider( m_ImageHScale, "H Scale", 1.0, "%7.3f" );

    m_BackgroundImageLayout.AddYGap();

    m_BackgroundImageLayout.AddSlider( m_ImageXOffset, "X Offset", 0.500, "%7.3f" );
    m_BackgroundImageLayout.AddSlider( m_ImageYOffset, "Y Offset", 0.500, "%7.3f" );

    m_DrawLayout.AddY( m_BackgroundImageLayout.GetH() );

    m_DrawLayout.AddButton( m_CopyDrawToAllXSec, "Copy Settings to All XSecs" );

    m_DrawLayout.SetSameLineFlag( true );
    m_DrawLayout.SetFitWidthFlag( false );
    m_DrawLayout.SetButtonWidth( m_DrawLayout.GetRemainX() / 2 );

    m_DrawLayout.AddButton( m_ResetDefaultBackground, "Reset Background" );
    m_DrawLayout.AddButton( m_ResetViewButton, "Reset Zoom && Pan" );

    m_DrawLayout.ForceNewLine();

    //==== XSec Editor Window ====//

    int hxaxis = 25;
    int wyaxis = 50;
    int border = 10;
    int window_x = m_MainLayout.GetX() + m_XSecLayout.GetW() + 5 + wyaxis;
    int window_y = m_MainLayout.GetY() + m_MainLayout.GetStdHeight() + 5;

    m_GlWinWidth = m_MainLayout.GetW() - ( m_XSecLayout.GetW() + 2 * border + wyaxis ); // Same width and height

    m_XSecGlWin = new VSPGUI::EditXSecWindow( window_x, window_y, m_GlWinWidth, m_GlWinWidth, m_ScreenMgr );

    m_XSecGlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_TOP );
    m_XSecGlWin->getGraphicEngine()->getDisplay()->getViewport()->showGridOverlay( true );

    m_FLTK_Window->resizable( m_MainLayout.GetGroup() );
    m_FLTK_Window->resizable( m_XSecGlWin );

    // Placeholder Canvas pointer - required for Ca_X_Axis and Ca_Y_Axis but not shown in GUI
    // The canvas is hidden behind the VspGlWindow
    Vsp_Canvas* canvas = new Vsp_Canvas( window_x, window_y, m_GlWinWidth, m_GlWinWidth, "" );
    Vsp_Canvas::current( canvas );
    m_MainLayout.GetGroup()->add( canvas );
    m_MainLayout.GetGroup()->add( m_XSecGlWin ); // must come after canvas is added to group for callback to register correctly

    m_XAxis = new Ca_X_Axis( window_x - wyaxis, window_y + m_GlWinWidth, m_GlWinWidth, hxaxis, "" );
    m_XAxis->labelsize( 14 );
    m_XAxis->align( Fl_Align( FL_ALIGN_LEFT ) );
    m_XAxis->minimum( 0 );
    m_XAxis->maximum( 1 );
    m_XAxis->label_format( "%g" );
    m_XAxis->minor_grid_color( fl_gray_ramp( 20 ) );
    m_XAxis->major_grid_color( fl_gray_ramp( 15 ) );
    m_XAxis->label_grid_color( fl_gray_ramp( 10 ) );
    m_XAxis->grid_visible( CA_MINOR_GRID | CA_MAJOR_GRID | CA_LABEL_GRID );
    m_XAxis->major_step( 10 );
    m_XAxis->label_step( 10 );
    m_XAxis->axis_color( FL_BLACK );
    m_XAxis->axis_align( CA_BOTTOM );
    m_MainLayout.GetGroup()->add( m_XAxis );

    m_YAxis = new Ca_Y_Axis( window_x - wyaxis, window_y, wyaxis, m_GlWinWidth, "" );
    m_YAxis->labelsize( 14 );
    m_YAxis->align( Fl_Align( FL_ALIGN_TOP ) );
    m_YAxis->minimum( 0 );
    m_YAxis->maximum( 1 );
    m_YAxis->label_format( "%g" );
    m_YAxis->minor_grid_color( fl_gray_ramp( 20 ) );
    m_YAxis->major_grid_color( fl_gray_ramp( 15 ) );
    m_YAxis->label_grid_color( fl_gray_ramp( 10 ) );
    m_YAxis->grid_visible( CA_MINOR_GRID | CA_MAJOR_GRID | CA_LABEL_GRID );
    m_YAxis->major_step( 10 );
    m_YAxis->label_step( 10 );
    m_YAxis->axis_color( FL_BLACK );
    m_YAxis->axis_align( CA_LEFT );
    m_MainLayout.GetGroup()->add( m_YAxis );

    m_ViewChoice.AddItem( "Front", vsp::VIEW_FRONT );
    m_ViewChoice.AddItem( "Top", vsp::VIEW_TOP );
    m_ViewChoice.AddItem( "Left", vsp::VIEW_LEFT );

    m_MainLayout.SetX( m_XSecLayout.GetW() + 5 + 10 );
    m_MainLayout.AddChoice( m_ViewChoice, "View", m_MainLayout.GetX() + 5 );

    m_MainLayout.AddY( m_XSecLayout.GetH() + 40 );

    //==== Control Point List ====//
    m_MainLayout.AddSubGroupLayout( m_PtLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() - 5 );
    m_PtLayout.AddDividerBox( "Control Points" );

    m_PtScroll = m_PtLayout.AddFlScroll( m_PtLayout.GetRemainY() );
    m_PtScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_PtScroll->box( FL_BORDER_BOX );

    m_FLTK_Window->resizable(m_PtScroll);

    //==== Initialize Member Variables ====//
    m_FreezeAxis = false;
    m_DeleteActive = false;
    m_SplitActive = false;

    m_PrevIndex = 0;
    m_PrevCurveType = 0;
    m_InputVecVec.resize( 5 ); // LINEAR or CEDIT, most common value.

    m_ImageZoomOffset = -1;
    m_ImageXOffsetOrig = 0;
    m_ImageYOffsetOrig = 0;
    m_ImageWOrig = 1;
    m_ImageHOrig = 1;
}

//==== Deconstructor ====//
CurveEditScreen::~CurveEditScreen()
{
}

//==== Show Curve Edit Screen ====//
void CurveEditScreen::Show()
{
    if( m_FLTK_Window )
    {
        m_FLTK_Window->show();
    }

    if ( m_XSecGlWin )
    {
        m_XSecGlWin->show();

        m_XSecGlWin->InitZoom();
    }

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    if ( !geom_ptr )
    {
        return;
    }

    geom_ptr->m_SurfDirty = true; // Ensures width/height parms are deactivated
}

//==== Get the Active XSec Curve ====//
XSecCurve* CurveEditScreen::GetXSecCurve()
{
    XSecCurve* xsc = NULL;

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    if ( !geom_ptr )
    {
        return NULL;
    }

    if ( geom_ptr->GetType().m_Type == MS_WING_GEOM_TYPE )
    {
        WingGeom* wing_ptr = dynamic_cast<WingGeom*>( geom_ptr );

        if ( !wing_ptr )
        {
            return NULL;
        }

        int aid = wing_ptr->m_ActiveXSec();
        XSec* xs = wing_ptr->GetXSec( aid );

        if ( !xs )
        {
            return NULL;
        }

        xsc = xs->GetXSecCurve();
    }
    else if ( geom_ptr->GetType().m_Type == BOR_GEOM_TYPE )
    {
        BORGeom* bor_geom = dynamic_cast <BORGeom*> ( geom_ptr );

        if ( !bor_geom )
        {
            return NULL;
        }

        xsc = bor_geom->GetXSecCurve();
    }
    else
    {
        GeomXSec* geom_xsec = dynamic_cast <GeomXSec*> ( geom_ptr );
        if ( !geom_xsec )
        {
            return NULL;
        }

        int xsid = geom_xsec->m_ActiveXSec();
        XSec* xs = geom_xsec->GetXSec( xsid );

        if ( !xs )
        {
            return NULL;
        }

        xsc = xs->GetXSecCurve();
    }

    return xsc;
}

//==== Update Curve Edit Screen ====//
bool CurveEditScreen::Update()
{
    assert( m_ScreenMgr );

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    XSecCurve* xsc = GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE || !veh )
    {
        Hide();
        return false;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    m_ViewChoice.Update( edit_curve_xs->m_View.GetID() );

    m_ShapeChoice.Update( edit_curve_xs->m_ShapeType.GetID() );
    m_SymToggle.Update( edit_curve_xs->m_SymType.GetID() );
    m_ClosedCurveToggle.Update( edit_curve_xs->m_CloseFlag.GetID() );

    m_XSecDriverGroupBank.SetDriverGroup( edit_curve_xs->m_DriverGroup );
    vector< string > parm_ids = edit_curve_xs->GetDriverParms();
    m_XSecDriverGroupBank.Update( parm_ids );

    m_DepthSlider.Update( edit_curve_xs->m_Depth.GetID() );

    Geom* geom = m_ScreenMgr->GetCurrGeom();
    m_XSecDriverGroupBank.EnforceXSecGeomType( geom->GetType().m_Type );

    m_AbsDimToggle.Update( edit_curve_xs->m_AbsoluteFlag.GetID() );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

    if( m_XSecGlWin )
    {
        VSPGraphic::Viewport* viewport = m_XSecGlWin->getGraphicEngine()->getDisplay()->getViewport();
        assert( viewport );

        if ( edit_curve_xs->m_View() == vsp::VIEW_FRONT ) // X,Y
        {
            m_XSecGlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_TOP );
        }
        else if ( edit_curve_xs->m_View() == vsp::VIEW_TOP ) // X,Z
        {
            m_XSecGlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_LEFT );
        }
        else if ( edit_curve_xs->m_View() == vsp::VIEW_LEFT ) // Z,Y
        {
            m_XSecGlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_FRONT_YUP );
        }

        m_XSecGlWin->clear();

        edit_curve_xs->SetSelectPntID( m_PntSelector.GetIndex() );
        m_PntSelector.SetMinMaxLimits( 0, edit_curve_xs->GetNumPts() - 1 );

        UpdateDrawObj();

        if ( m_DeleteActive )
        {
            m_DelPickButton.GetFlButton()->set();
        }
        else
        {
            m_DelPickButton.GetFlButton()->clear();
        }

        if ( m_SplitActive )
        {
            m_SplitPickButton.GetFlButton()->set();
        }
        else
        {
            m_SplitPickButton.GetFlButton()->clear();
        }

        unsigned int n = edit_curve_xs->GetNumPts();

        m_SplitPtSlider.Update( edit_curve_xs->m_SplitU.GetID() );
        m_ConvertChoice.Update( edit_curve_xs->m_ConvType.GetID() );

        switch( edit_curve_xs->m_CurveType() )
        {
        case vsp::LINEAR:
            m_CurveType.Update( "Linear" );
            break;
        case vsp::PCHIP:
            m_CurveType.Update( "Spline (PCHIP)" );
            break;
        case vsp::CEDIT:
            m_CurveType.Update( "Cubic Bezier" );
            break;
        }

        if( n != m_InputVecVec[0].size() || m_PrevCurveType != edit_curve_xs->m_CurveType() )
        {
            RedrawXYSliders( n, edit_curve_xs->m_CurveType() );
        }
        else if ( m_GlWinWidth != m_XSecGlWin->pixel_w() )
        {
            // Increase slider width if window is resized
            RedrawXYSliders( n, edit_curve_xs->m_CurveType() );
            m_GlWinWidth = m_XSecGlWin->pixel_w();
        }

        for( int i = 0; i < n; i++ )
        {
            FractionParm* fp = edit_curve_xs->m_XParmVec[i];
            if( fp )
            {
                fp->SetRefVal( edit_curve_xs->GetWidth() );

                if( edit_curve_xs->m_AbsoluteFlag.Get() )
                {
                    fp->SetDisplayResultFlag( true );
                }
                else
                {
                    fp->SetDisplayResultFlag( false );
                }

                m_InputVecVec[0][i].Update( fp->GetID() );
            }

            fp = edit_curve_xs->m_YParmVec[i];
            if( fp )
            {
                fp->SetRefVal( edit_curve_xs->GetHeight() );

                if( edit_curve_xs->m_AbsoluteFlag.Get() )
                {
                    fp->SetDisplayResultFlag( true );
                }
                else
                {
                    fp->SetDisplayResultFlag( false );
                }

                m_InputVecVec[1][i].Update( fp->GetID() );
            }

            fp = edit_curve_xs->m_ZParmVec[i];
            if( fp )
            {
                fp->SetRefVal( edit_curve_xs->GetHeight() );

                if( edit_curve_xs->m_AbsoluteFlag.Get() )
                {
                    fp->SetDisplayResultFlag( true );
                }
                else
                {
                    fp->SetDisplayResultFlag( false );
                }

                m_InputVecVec[2][i].Update( fp->GetID() );
            }

            Parm* p = edit_curve_xs->m_UParmVec[i];
            if( p )
            {
                m_InputVecVec[3][i].Update( p->GetID() );
            }

            if( edit_curve_xs->m_CurveType() != vsp::PCHIP )
            {
                p = edit_curve_xs->m_RParmVec[i];
                if( p )
                {
                    m_InputVecVec[4][i].Update( p->GetID() );
                }
            }

            if( edit_curve_xs->m_CurveType() == vsp::CEDIT )
            {
                BoolParm* bp = edit_curve_xs->m_EnforceG1Vec[i];
                if( bp )
                {
                    m_EnforceG1Vec[i].Update( bp->GetID() );
                }
            }

            BoolParm* fixed_u = edit_curve_xs->m_FixedUVec[i];
            if( fixed_u )
            {
                m_FixedUCheckVec[i].Update( fixed_u->GetID() );
            }

            if( i == m_PntSelector.GetIndex() )
            {
                for ( int j = 0; j < m_InputVecVec.size(); j++ )
                {
                    m_InputVecVec[j][i].SetLabelColor( FL_YELLOW );
                }
            }
            else
            {
                for ( int j = 0; j < m_InputVecVec.size(); j++ )
                {
                    m_InputVecVec[j][i].ResetLabelColor();
                }
            }
        }

        m_PrevCurveType = edit_curve_xs->m_CurveType();
        m_PrevIndex = m_PntSelector.GetIndex();

        // Update Display Tab Settings
        m_ColorPicker.Update( veh->GetXSecLineColor() );

        m_PointSizeSlider.Update( edit_curve_xs->m_XSecPointSize.GetID() );
        m_LineThicknessSlider.Update( edit_curve_xs->m_XSecLineThickness.GetID() );

        m_PointColorCheck.Update( edit_curve_xs->m_XSecPointColorFlag.GetID() );
        m_PointColorWheelSlider.Update( edit_curve_xs->m_XSecPointColorWheel.GetID() );

        if ( !edit_curve_xs->m_XSecPointColorFlag() )
        {
            m_PointColorWheelSlider.Deactivate();
        }
        else
        {
            m_PointColorWheelSlider.Activate();
        }

        m_ImageToggle.Update( edit_curve_xs->m_XSecImageFlag.GetID() );
        m_ImageFileOutput.Update( StringUtil::truncateFileName( edit_curve_xs->GetImageFile(), 40 ).c_str() );

        // Update Scale and Offset in Background
        if( edit_curve_xs->m_XSecImageFlag() )
        {
            m_BackgroundImageLayout.GetGroup()->activate();

            if( ( viewport->getBackground()->getBackgroundMode() != VSPGraphic::Common::VSP_BACKGROUND_IMAGE || 
                ( VSPGraphic::GlobalTextureRepo()->getTextureID( edit_curve_xs->GetImageFile().c_str() ) != viewport->getBackground()->getTextureID() ) ) &&
                edit_curve_xs->GetImageFile().size() > 0 )
            {
                viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );
                viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( edit_curve_xs->GetImageFile().c_str() ) );
            }
        }
        else
        {
            m_BackgroundImageLayout.GetGroup()->deactivate();

            if( viewport->getBackground()->getBackgroundMode() == VSPGraphic::Common::VSP_BACKGROUND_IMAGE )
            {
                viewport->getBackground()->removeImage();
                viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
            }
        }

        m_PreserveImageAspect.Update( edit_curve_xs->m_XSecImagePreserveAR.GetID() );
        m_LockImageToggle.Update( edit_curve_xs->m_XSecLockImageFlag.GetID() );

        m_FlipImageToggle.Update( edit_curve_xs->m_XSecFlipImageFlag.GetID() );
        viewport->getBackground()->flipX( edit_curve_xs->m_XSecFlipImageFlag.Get() );

        if ( edit_curve_xs->m_XSecLockImageFlag() )
        {
            // Update the image offset and size to follow zoom and pan events
            glm::vec2 pan = m_XSecGlWin->getPanValues();
            double gl_w = m_XSecGlWin->pixel_w();
            double gl_h = m_XSecGlWin->pixel_h();
            double zoom = m_XSecGlWin->getRelativeZoomValue();

            if ( m_ImageZoomOffset < 0 )
            {
                m_ImageZoomOffset = zoom; // Initialization when reloading a model (can't set to zoom in constructor)
            }

            edit_curve_xs->m_XSecImageXOffset.Set( ( m_ImageXOffsetOrig * m_ImageZoomOffset / zoom ) + ( 2 * ( pan.x - m_ImagePanOffset.x ) / ( zoom * gl_w ) ) );
            edit_curve_xs->m_XSecImageYOffset.Set( ( m_ImageYOffsetOrig * m_ImageZoomOffset / zoom ) + ( 2 * ( pan.y - m_ImagePanOffset.y ) / ( zoom * gl_h ) ) );

            edit_curve_xs->m_XSecImageW.Set( m_ImageWOrig * m_ImageZoomOffset / zoom );
            edit_curve_xs->m_XSecImageH.Set( m_ImageHOrig * m_ImageZoomOffset / zoom );

            edit_curve_xs->m_XSecImageW.Deactivate();
            edit_curve_xs->m_XSecImagePreserveAR.Deactivate();
            edit_curve_xs->m_XSecImageXOffset.Deactivate();
            edit_curve_xs->m_XSecImageYOffset.Deactivate();
        }
        else
        {
            edit_curve_xs->m_XSecImageW.Activate();
            edit_curve_xs->m_XSecImagePreserveAR.Activate();
            edit_curve_xs->m_XSecImageXOffset.Activate();
            edit_curve_xs->m_XSecImageYOffset.Activate();
        }

        m_ImageXOffset.Update( edit_curve_xs->m_XSecImageXOffset.GetID() );
        m_ImageYOffset.Update( edit_curve_xs->m_XSecImageYOffset.GetID() );

        m_ImageWScale.Update( edit_curve_xs->m_XSecImageW.GetID() );

        viewport->getBackground()->scaleW( (float)edit_curve_xs->m_XSecImageW.Get() );
        viewport->getBackground()->preserveAR( (bool)edit_curve_xs->m_XSecImagePreserveAR.Get() );

        if ( edit_curve_xs->m_XSecImagePreserveAR() )
        {
            edit_curve_xs->m_XSecImageH.Set( viewport->getBackground()->getScaleH() );
        }
        else
        {
            viewport->getBackground()->scaleH( (float)edit_curve_xs->m_XSecImageH.Get() );
        }

        if ( edit_curve_xs->m_XSecImagePreserveAR() || edit_curve_xs->m_XSecLockImageFlag() )
        {
            edit_curve_xs->m_XSecImageH.Deactivate();
        }
        else
        {
            edit_curve_xs->m_XSecImageH.Activate();
        }

        m_ImageHScale.Update( edit_curve_xs->m_XSecImageH.GetID() );

        viewport->getBackground()->offsetX( (float)edit_curve_xs->m_XSecImageXOffset.Get() );
        viewport->getBackground()->offsetY( (float)edit_curve_xs->m_XSecImageYOffset.Get() );

        m_XSecGlWin->update();
        m_XSecGlWin->redraw();
    }

    m_FLTK_Window->redraw();

    return true;
}

void CurveEditScreen::UpdateDrawObj()
{
    assert( m_ScreenMgr );

    VSPGraphic::Viewport* viewport = m_XSecGlWin->getGraphicEngine()->getDisplay()->getViewport();
    assert( viewport );

    Vehicle* veh = VehicleMgr.GetVehicle();

    XSecCurve* xsc = GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        Hide();
        return;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    double w = edit_curve_xs->GetWidth();
    double h = edit_curve_xs->GetHeight();

    VspCurve crv = edit_curve_xs->GetBaseEditCurve();
    crv.OffsetX( -0.5 * w );
    vec3d color = veh->GetXSecLineColor() / 255.; // normalize

    // Note, the absolute coordinates of the control points are always plotted, but the 
    // axes are adjusted according to the user's preference of absolute or nondimensionalized.
    // This allows the true 3D shape to always be shown
    vector < vec3d > control_pts = edit_curve_xs->GetCtrlPntVec( false );

    int ndata = control_pts.size();

    double point_size = m_XSecGlWin->pixels_per_unit() * edit_curve_xs->m_XSecPointSize.Get();
    double line_thick = m_XSecGlWin->pixels_per_unit() *edit_curve_xs->m_XSecLineThickness.Get();
    int point_color = 361; // DrawObj::ColorWheel( 361 ) will return black (0, 0, 0)

    if ( edit_curve_xs->m_XSecPointColorFlag.Get() )
    {
        point_color = edit_curve_xs->m_XSecPointColorWheel.Get();
    }

    if ( w == 0 && h == 0 )
    {
        vector< vec3d > pts( 1, vec3d( 0, 0, 0 ) );
        m_XSecCurveDrawObj.m_PntVec = pts;
        m_XSecCurveDrawObj.m_PointSize = 5.0;
        m_XSecCurveDrawObj.m_PointColor = color;
        m_XSecCurveDrawObj.m_Type = DrawObj::VSP_POINTS;
    }
    else
    {
        vector< vec3d > pts;
        crv.TessAdapt( pts, 1e-3, 10 );

        m_XSecCurveDrawObj.m_PntVec = pts;
        m_XSecCurveDrawObj.m_LineWidth = line_thick;
        m_XSecCurveDrawObj.m_LineColor = color;
        m_XSecCurveDrawObj.m_Type = DrawObj::VSP_LINE_STRIP;

        m_CEDITTangentLineDrawObj.clear();
        m_CEDITTangentPntDrawObj.m_PntVec.clear();

        if ( edit_curve_xs->m_CurveType.Get() == vsp::CEDIT )
        {

            m_CEDITTangentPntDrawObj.m_Type = DrawObj::VSP_POINTS;
            m_CEDITTangentPntDrawObj.m_PointSize = point_size;
            m_CEDITTangentPntDrawObj.m_Visible = true;
            m_CEDITTangentPntDrawObj.m_PointColor = DrawObj::ColorWheel( 120 );

            int nseg = ( ndata - 1 ) / 3;

            m_CEDITTangentLineDrawObj.resize( nseg + 1 );

            vector < vec3d > mid( 2 * nseg ); // Cubic segment midpoints
            int imid = 0;

            for ( int i = 0; i < ndata; i++ )
            {
                if ( ( i % 3 ) == 0 )
                {
                    int iseg = i / 3;

                    if ( i != 0 )
                    {
                        m_CEDITTangentLineDrawObj[iseg].m_PntVec.push_back( control_pts[i - 1] );
                        m_CEDITTangentLineDrawObj[iseg].m_PntVec.push_back( control_pts[i] );
                    }

                    if ( i != ndata - 1 )
                    {
                        m_CEDITTangentLineDrawObj[iseg].m_PntVec.push_back( control_pts[i] );
                        m_CEDITTangentLineDrawObj[iseg].m_PntVec.push_back( control_pts[i + 1] );
                    }

                    m_CEDITTangentLineDrawObj[iseg].m_Type = DrawObj::VSP_LINES;
                    m_CEDITTangentLineDrawObj[iseg].m_LineWidth = 0.75 * line_thick; // 3/4 as thick
                    m_CEDITTangentLineDrawObj[iseg].m_Visible = true;
                    m_CEDITTangentLineDrawObj[iseg].m_LineColor = DrawObj::ColorWheel( 240 );
                    m_CEDITTangentLineDrawObj[iseg].m_GeomChanged = true;
                    m_CEDITTangentLineDrawObj[iseg].m_Screen = DrawObj::VSP_EDIT_CURVE_SCREEN;
                    m_CEDITTangentLineDrawObj[iseg].m_GeomID = "CEDIT_" + edit_curve_xs->GetID() + "_LINE_" + std::to_string( i );
                }
                else
                {
                    mid[imid] = control_pts[i];
                    imid++;
                }
            }

            m_CEDITTangentPntDrawObj.m_PntVec = mid;
        }

        m_XSecCtrlPntsDrawObj.m_PntVec = control_pts;
        m_XSecCtrlPntsDrawObj.m_Type = DrawObj::VSP_POINTS;
        m_XSecCtrlPntsDrawObj.m_PointSize = point_size;
        m_XSecCtrlPntsDrawObj.m_PointColor = DrawObj::ColorWheel( point_color );

        if( !m_FreezeAxis && m_XSecGlWin )
        {
            UpdateAxisLimits();
        }
    }

    m_XSecCurveDrawObj.m_GeomChanged = true;
    m_XSecCurveDrawObj.m_Screen = DrawObj::VSP_EDIT_CURVE_SCREEN;
    m_XSecCurveDrawObj.m_GeomID = "XSEC_" + edit_curve_xs->GetID() + "_CURVE";

    m_XSecCtrlPntsDrawObj.m_GeomChanged = true;
    m_XSecCtrlPntsDrawObj.m_Screen = DrawObj::VSP_EDIT_CURVE_SCREEN;
    m_XSecCtrlPntsDrawObj.m_GeomID = "XSEC_" + edit_curve_xs->GetID() + "_PNTS";

    m_CEDITTangentPntDrawObj.m_GeomChanged = true;
    m_CEDITTangentPntDrawObj.m_Screen = DrawObj::VSP_EDIT_CURVE_SCREEN;
    m_CEDITTangentPntDrawObj.m_GeomID = "CEDIT_" + edit_curve_xs->GetID() + "_PNTS";

    m_CurrentPntDrawObj.m_GeomChanged = true;
    m_CurrentPntDrawObj.m_Screen = DrawObj::VSP_EDIT_CURVE_SCREEN;
    m_CurrentPntDrawObj.m_GeomID = "SELECT_" + edit_curve_xs->GetID() + "_PNT";
    m_CurrentPntDrawObj.m_PointSize = point_size;

    // Identify the selected point
    int selected_id = m_PntSelector.GetIndex();
    if ( selected_id >= 0 && selected_id < ndata )
    {
        m_CurrentPntDrawObj.m_Visible = true;
        m_CurrentPntDrawObj.m_PntVec = vector < vec3d >{ control_pts[selected_id] };

        vector < double > u_vec = edit_curve_xs->GetUVec();

        if ( edit_curve_xs->m_SymType.Get() == vsp::SYM_RL && 
             selected_id >= 0 && selected_id < u_vec.size() &&
             u_vec[selected_id] > 0.25 && u_vec[selected_id] < 0.75 )
        {
            m_CurrentPntDrawObj.m_PointColor = vec3d( 192 / 255., 192 / 255., 192 / 255. );
        }
        else
        {
            m_CurrentPntDrawObj.m_PointColor = vec3d( 1, 1, 0 );
        }
    }
    else
    {
        m_CurrentPntDrawObj.m_PntVec.clear();
        m_CurrentPntDrawObj.m_Visible = false;
    }
}

void CurveEditScreen::LoadDrawObjs( vector< DrawObj* >& draw_obj_vec )
{
    //::LoadDrawObjs( draw_obj_vec );
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return;
    }

    draw_obj_vec.push_back( &m_XSecCurveDrawObj );
    draw_obj_vec.push_back( &m_XSecCtrlPntsDrawObj );
    for ( size_t i = 0; i < m_CEDITTangentLineDrawObj.size(); i++ )
    {
        draw_obj_vec.push_back( &m_CEDITTangentLineDrawObj[i] );
    }
    draw_obj_vec.push_back( &m_CEDITTangentPntDrawObj );
    draw_obj_vec.push_back( &m_CurrentPntDrawObj );
}

void CurveEditScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    //==== Find EditCurveXSec Ptr ====//
    XSecCurve* xsc = GetXSecCurve();

    if ( !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE || !veh )
    {
        return;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    if ( gui_device == &m_InitShapeButton )
    {
        edit_curve_xs->InitShape();

        Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();

        if ( !geom_ptr )
        {
            return;
        }

        // Set tess dirty flag and update. This fixes an issue where the XSec is drawn detached 
        // from the parent Geom surface since the surface was not updated
        geom_ptr->m_SurfDirty = true;
        geom_ptr->Update();
    }
    else if ( gui_device == &m_ReparameterizeButton )
    {
        edit_curve_xs->ReparameterizeEqualArcLength();
        // Force update the parent Geom after reparameterization
        edit_curve_xs->ParmChanged( NULL, Parm::SET_FROM_DEVICE );
    }
    else if ( gui_device == &m_SplitButton )
    {
        int new_pnt = edit_curve_xs->Split01();
        m_PntSelector.SetMinMaxLimits( 0, edit_curve_xs->GetNumPts() - 1 );
        m_PntSelector.SetIndex( new_pnt );
    }
    else if ( gui_device == &m_DelButton )
    {
        edit_curve_xs->DeletePt(); // Try to delete currently selected point
    }
    else if ( gui_device == &m_ConvertButton )
    {
        edit_curve_xs->ConvertTo( edit_curve_xs->m_ConvType() );
    }
    else if ( gui_device == &m_PntSelector )
    {
        UpdateIndexSelector( m_PntSelector.GetIndex() );
    }
    else if ( gui_device == &m_DelPickButton )
    {
        m_DeleteActive = !m_DeleteActive;
        if( m_DeleteActive )
        {
            m_SplitActive = false;
        }
    }
    else if ( gui_device == &m_SplitPickButton )
    {
        m_SplitActive = !m_SplitActive;
        if( m_SplitActive )
        {
            m_DeleteActive = false;
        }
    }
    else if ( gui_device == &m_LockImageToggle )
    {
        m_ImageZoomOffset = m_XSecGlWin->getRelativeZoomValue();
        m_ImagePanOffset = m_XSecGlWin->getPanValues();

        m_ImageWOrig = edit_curve_xs->m_XSecImageW.Get();
        m_ImageHOrig = edit_curve_xs->m_XSecImageH.Get();
        m_ImageXOffsetOrig = edit_curve_xs->m_XSecImageXOffset.Get();
        m_ImageYOffsetOrig = edit_curve_xs->m_XSecImageYOffset.Get();
    }
    else if( gui_device == &m_CopyDrawToAllXSec )
    {
        Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
        if( !geom_ptr )
        {
            return;
        }

        XSecSurf* xsec_surf = geom_ptr->GetXSecSurf( 0 );
        if( !xsec_surf )
        {
            return;
        }

        int num_xsec = xsec_surf->NumXSec();

        for( int i = 0; i < num_xsec; i++ )
        {
            XSec* other_xsec = xsec_surf->FindXSec( i );
            XSecCurve* other_xsc = other_xsec->GetXSecCurve();

            other_xsc->CopyBackgroundSettings( xsc );
        }
    }
    else
    {
        // Identify event in PntLayout. Set current index to row that 
        // corresponds to the event.
        string parm_id = gui_device->GetParmID();

        for( size_t j = 0; j < edit_curve_xs->m_XParmVec.size(); j++ )
        {
            if( !strcmp( parm_id.c_str(), edit_curve_xs->m_XParmVec[j]->GetID().c_str() ) || 
                !strcmp( parm_id.c_str(), edit_curve_xs->m_YParmVec[j]->GetID().c_str() ) ||
                !strcmp( parm_id.c_str(), edit_curve_xs->m_ZParmVec[j]->GetID().c_str() ) ||
                !strcmp( parm_id.c_str(), edit_curve_xs->m_RParmVec[j]->GetID().c_str() ) ||
                !strcmp( parm_id.c_str(), edit_curve_xs->m_UParmVec[j]->GetID().c_str() ) ||
                !strcmp( parm_id.c_str(), edit_curve_xs->m_EnforceG1Vec[j]->GetID().c_str() ) || 
                !strcmp( parm_id.c_str(), edit_curve_xs->m_FixedUVec[j]->GetID().c_str() ) )
            {
                m_PntSelector.SetIndex( j );
                break;
            }
        }
    }

    if( m_XSecGlWin )
    {
        VSPGraphic::Viewport* viewport = m_XSecGlWin->getGraphicEngine()->getDisplay()->getViewport();

        if( gui_device == &m_ResetViewButton )
        {
            m_XSecGlWin->relativePan( 0, 0 );
            m_XSecGlWin->InitZoom();
        }
        else if( gui_device == &m_ColorPicker )
        {
            veh->SetXSecLineColor( m_ColorPicker.GetColor() );
        }
        else if( gui_device == &m_BorderToggle )
        {
            viewport->showBorders( (bool)m_BorderToggle.GetFlButton()->value() );
        }
        else if( gui_device == &m_AxisToggle )
        {
            viewport->showXYZArrows( (bool)m_AxisToggle.GetFlButton()->value() );
        }
        else if( gui_device == &m_GridToggle )
        {
            viewport->showGridOverlay( (bool)m_GridToggle.GetFlButton()->value() );
        }
        else if( gui_device == &m_ImageToggle )
        {
            if( m_ImageToggle.GetFlButton()->value() )
            {
                viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );

                if( edit_curve_xs->GetImageFile().compare( "" ) != 0 )
                {
                    viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( edit_curve_xs->GetImageFile().c_str() ) );
                }
            }
            else
            {
                viewport->getBackground()->removeImage();
                viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
            }
        }
        else if( gui_device == &m_ImageFileSelect )
        {
            std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
                "Select Image File", "*.{jpg,png,tga,bmp,gif}", false );

            if( !fileName.empty() )
            {
                if( m_ImageToggle.GetFlButton()->value() )
                {
                    viewport->getBackground()->removeImage();
                    viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( fileName.c_str() ) );
                }

                edit_curve_xs->SetImageFile( fileName );
            }
        }
        else if( gui_device == &m_ResetDefaultBackground )
        {
            viewport->getBackground()->reset();
            edit_curve_xs->m_XSecImageFlag.Set( false );
            edit_curve_xs->SetImageFile( "" );
            edit_curve_xs->m_XSecImagePreserveAR.Set( true );

            veh->SetXSecLineColor( vec3d( 0, 0, 0 ) );

            // Reset Scale & Offset
            edit_curve_xs->m_XSecImageW.Set( viewport->getBackground()->getScaleW() );
            edit_curve_xs->m_XSecImageH.Set( viewport->getBackground()->getScaleH() );

            edit_curve_xs->m_XSecImageXOffset.Set( viewport->getBackground()->getOffsetX() );
            edit_curve_xs->m_XSecImageYOffset.Set( viewport->getBackground()->getOffsetY() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void CurveEditScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Hide();
}

void CurveEditScreen::UpdateAxisLimits()
{
    //==== Find EditCurveXSec Ptr ====//
    XSecCurve* xsc = GetXSecCurve();

    if( !m_XSecGlWin ||!xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    float zoom = m_XSecGlWin->getRelativeZoomValue();
    glm::vec2 pan = m_XSecGlWin->getPanValues();
    double gl_w = m_XSecGlWin->pixel_w();
    double gl_h = m_XSecGlWin->pixel_h();

    double scale_w = 1;
    double scale_h = 1;

    if( !edit_curve_xs->m_AbsoluteFlag() )
    {
        scale_w = edit_curve_xs->GetWidth();
        scale_h = edit_curve_xs->GetHeight();
    }

    m_XAxis->minimum( ( -0.5 * gl_w * zoom - pan.x ) / scale_w );
    m_XAxis->maximum( ( 0.5 * gl_w * zoom - pan.x ) / scale_w );

    m_YAxis->minimum( ( -0.5 * gl_h * zoom - pan.y ) / scale_h );
    m_YAxis->maximum( ( 0.5 * gl_h * zoom - pan.y ) / scale_h );

    // Identify tick values to align grid
    double x_tick_value, y_tick_value;
    int x_tick_order, y_tick_order;
    double x_interval = 0;
    double y_interval = 0;
    int x_tick_index = -1;
    int y_tick_index = -1;

    vector < double > x_tick_vec, y_tick_vec;

    while( m_XAxis->next_tick( x_tick_index, x_tick_value, x_tick_order, x_interval ) )
    {
        x_tick_vec.push_back( x_tick_value * scale_w );
    }
    while( m_YAxis->next_tick( y_tick_index, y_tick_value, y_tick_order, y_interval ) )
    {
        y_tick_vec.push_back( y_tick_value * scale_h );
    }

    m_XSecGlWin->getGraphicEngine()->getDisplay()->getViewport()->setGridVals( x_tick_vec, y_tick_vec );
}

void CurveEditScreen::RedrawXYSliders( int num_pts, int curve_type )
{
    int num_inputs;
    if( curve_type == vsp::PCHIP )
    {
        num_inputs = 4; // X, Y, Z, U
    }
    else
    {
        num_inputs = 5; // X, Y, Z, U, & R
    }

    if ( num_inputs != m_InputVecVec.size() )
    {
        m_InputVecVec.clear();
        m_InputVecVec.resize( num_inputs );
    }

    for( int i = 0; i < num_inputs; i++ )
    {
        m_InputVecVec[i].clear();
        m_InputVecVec[i].resize( num_pts );
    }

    m_PtScroll->clear();
    m_PtLayout.SetGroup( m_PtScroll );
    m_PtLayout.InitWidthHeightVals();

    m_EnforceG1Vec.clear();
    m_EnforceG1Vec.resize( num_pts );

    m_FixedUCheckVec.clear();
    m_FixedUCheckVec.resize( num_pts );

    int gap_w = 5;
    int button_w = 45;
    int scroll_w = 15;
    int fix_u_w = 55;

    m_PtLayout.SetSameLineFlag( true );
    m_PtLayout.SetFitWidthFlag( false );

    if( curve_type == vsp::CEDIT )
    {
        int g1_w = 45;
        int input_w = ( m_PtLayout.GetRemainX() - (fix_u_w + scroll_w + g1_w + (num_inputs + 2 ) * gap_w + num_inputs * button_w ) ) / num_inputs;
        m_PtLayout.SetInputWidth( input_w );

        for( int n = 0; n < num_pts; n++ )
        {
            m_PtLayout.SetButtonWidth( button_w );
            m_PtLayout.AddX( gap_w );

            for(int i = 0; i < num_inputs; i++ )
            {
                m_PtLayout.AddInput( m_InputVecVec[i][n], "AUTO_UPDATE", "%9.4f" );

                m_PtLayout.AddX( gap_w );
            }

            m_PtLayout.SetButtonWidth(g1_w );
            m_PtLayout.AddButton( m_EnforceG1Vec[n], "G1" );
            m_PtLayout.SetButtonWidth( fix_u_w );
            m_PtLayout.AddButton( m_FixedUCheckVec[n], "Fix U" );
            m_PtLayout.ForceNewLine();
        }
    }
    else
    {
        int input_w = ( m_PtLayout.GetRemainX() - (fix_u_w + scroll_w + (num_inputs + 2 ) * gap_w + num_inputs * button_w ) ) / num_inputs;
        m_PtLayout.SetInputWidth( input_w );

        for( int n = 0; n < num_pts; n++ )
        {
            m_PtLayout.SetButtonWidth( button_w );
            m_PtLayout.AddX( gap_w );

            for(int i = 0; i < num_inputs; i++ )
            {
                m_PtLayout.AddInput( m_InputVecVec[i][n], "AUTO_UPDATE", "%9.4f" );

                m_PtLayout.AddX( gap_w );
            }

            m_PtLayout.SetButtonWidth( fix_u_w );
            m_PtLayout.AddButton( m_FixedUCheckVec[n], "Fix U" );
            m_PtLayout.ForceNewLine();
        }
    }
}

void CurveEditScreen::UpdateIndexSelector( int index, bool skip_intermediate )
{
    //==== Find EditCurveXSec Ptr ====//
    XSecCurve* xsc = GetXSecCurve();

    if ( !m_XSecGlWin || !xsc || xsc->GetType() != vsp::XS_EDIT_CURVE )
    {
        return;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    // Don't cycle through intermediate Cubic Bezier points
    if( edit_curve_xs->m_CurveType.Get() == vsp::CEDIT && index % 3 != 0 && skip_intermediate )
    {
        if( index == m_PrevIndex + 1 )
        {
            if( ( index + 1 ) % 3 == 0 )
            {
                index = index + 1;
            }
            else
            {
                index = index + 2;
            }
        }
        else if( index == m_PrevIndex - 1 )
        {
            if( ( index - 1 ) % 3 == 0 )
            {
                index = index - 1;
            }
            else
            {
                index = index - 2;
            }
        }
    }

    m_PntSelector.SetIndex( index );
    m_PntSelector.SetMinMaxLimits( 0, edit_curve_xs->GetNumPts() - 1 );

    if( ( edit_curve_xs->m_CurveType.Get() == vsp::CEDIT && index % 3 != 0 ) ||
        ( index == m_PntSelector.GetMinIndex() ) ||
        ( index == m_PntSelector.GetMaxIndex() ) )
    {
        m_DelButton.Deactivate();
    }
    else
    {
        m_DelButton.Activate();
    }
}