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

using namespace vsp;


//==== Constructor ====//
CurveEditScreen::CurveEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 500, 760, "Edit Curve" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( 7 );
    m_MainLayout.AddX( 5 );

    m_FLTK_Window->resizable( 0 ); // Don't resize so plot AR is fixed

    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - 5,
        m_MainLayout.GetRemainY() - 5 );

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );

    m_ShapeChoice.AddItem( "Circle" );
    m_ShapeChoice.AddItem( "Ellipse" );
    m_ShapeChoice.AddItem( "Rectangle" );
    m_GenLayout.AddChoice( m_ShapeChoice, "Shape" );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetRemainX() / 2 );

    m_GenLayout.AddButton( m_InitShapeButton, "Init" );
    m_GenLayout.AddButton( m_ClosedCurveToggle, "Closed Curve" );

    m_GenLayout.ForceNewLine();

    m_SymChoice.AddItem( "NONE" );
    m_SymChoice.AddItem( "SYM R-L" );
    //m_SymChoice.AddItem( "SYM T-B" );
    m_GenLayout.AddChoice( m_SymChoice, "Sym" );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetRemainX() / 2 );

    m_GenLayout.AddButton( m_PreserveARToggle, "Preserve AR" );
    m_GenLayout.AddButton( m_AbsDimToggle, "View Abs" ); // Absolute dimensions flag

    m_GenLayout.ForceNewLine();

    m_GenLayout.AddYGap();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );

    int button_w = 75;
    int gap_w = 4;
    
    m_GenLayout.SetButtonWidth( button_w );

    int slider_w = ( m_GenLayout.GetRemainX() / 2 ) - button_w - ( gap_w / 2 ) - m_GenLayout.GetInputWidth() - 2 * m_GenLayout.GetRangeButtonWidth();
    m_GenLayout.SetSliderWidth( slider_w );

    m_GenLayout.AddSlider( m_WidthSlider, "Width", 10, "%5.3f" );
    m_GenLayout.AddX( gap_w );
    m_GenLayout.AddSlider( m_HeightSlider, "Height", 10, "%5.3f" );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddYGap();

    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( false );

    //==== Add Curve Point Editor ====//
    m_GenLayout.InitWidthHeightVals();

    m_GenLayout.SetCanvasHeight( 300 );

    m_GenLayout.AddXSecCurveEditor( m_CurveEditor );
}

//==== Deconstructor ====//
CurveEditScreen::~CurveEditScreen()
{
}

//==== Show Curve Edit Screen ====//
void CurveEditScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
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

        int aid = wing_ptr->GetActiveAirfoilIndex();
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

        int xsid = geom_xsec->GetActiveXSecIndex();
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

    XSecCurve* xsc = GetXSecCurve();

    if ( !xsc || xsc->GetType() != XS_EDIT_CURVE )
    {
        Hide();
        return false;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );
 
    m_ShapeChoice.Update( edit_curve_xs->m_ShapeType.GetID() );
    m_SymChoice.Update( edit_curve_xs->m_SymType.GetID() );
    m_ClosedCurveToggle.Update( edit_curve_xs->m_CloseFlag.GetID() );

    m_WidthSlider.Update( edit_curve_xs->m_Width.GetID() );
    m_HeightSlider.Update( edit_curve_xs->m_Height.GetID() );

    m_AbsDimToggle.Update( edit_curve_xs->m_AbsoluteFlag.GetID() );
    m_PreserveARToggle.Update( edit_curve_xs->m_PreserveARFlag.GetID() );

    if ( edit_curve_xs->m_PreserveARFlag() )
    {
        m_HeightSlider.Deactivate();
    }
    else
    {
        m_HeightSlider.Activate();
    }

    m_CurveEditor.Update( edit_curve_xs );

    return true;
}

void CurveEditScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find EditCurveXSec Ptr ====//
    XSecCurve* xsc = GetXSecCurve();

    if ( !xsc || xsc->GetType() != XS_EDIT_CURVE )
    {
        return;
    }

    EditCurveXSec* edit_curve_xs = dynamic_cast<EditCurveXSec*>( xsc );
    assert( edit_curve_xs );

    if ( gui_device == &m_InitShapeButton )
    {
        edit_curve_xs->InitShape( );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}


void CurveEditScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Hide();
}
