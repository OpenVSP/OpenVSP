//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SVGOptionsScreen.cpp: implementation of the SVGOptionsScreen class.
//
// Justin Gravett
//////////////////////////////////////////////////////////////////////

#include "SVGOptionsScreen.h"
#include "VspUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SVGOptionsScreen::SVGOptionsScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 250, 340, "SVG Options" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_OkFlag = false;

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 30 );

    m_GenLayout.SetChoiceButtonWidth( m_GenLayout.GetW() / 2 );
    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    m_LenUnitChoice.AddItem( "YD" );
    m_LenUnitChoice.AddItem( "Dimensionless" );
    m_GenLayout.AddChoice( m_LenUnitChoice, "Length Unit" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetChoiceButtonWidth( 0 );

    int boxdim = m_GenLayout.GetW() / 2 - 20;

    m_GenLayout.AddButton( m_XSecToggle, "Force XSecs" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetFitWidthFlag( true );

    m_GenLayout.AddButton( m_ProjectionLineToggle, "Outline" );

    m_GenLayout.InitWidthHeightVals();

    m_GenLayout.AddSlider( m_TessSlider, "Refinement", 8, "%5.0f" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetChoiceButtonWidth( m_GenLayout.GetW() / 2 );
    m_2DViewType.AddItem( "One" );
    m_2DViewType.AddItem( "Two Horizontal" );
    m_2DViewType.AddItem( "Two Vertical" );
    m_2DViewType.AddItem( "Four" );
    m_GenLayout.AddChoice( m_2DViewType, "2D View Type" );

    m_GenLayout.AddYGap();
    m_GenLayout.AddX( 20 );

    m_GenLayout.AddSubGroupLayout( m_4LayoutTL, boxdim, boxdim - 40 );
    m_4LayoutTL.GetGroup()->box( Fl_Boxtype::FL_BORDER_BOX );
    m_4LayoutTL.AddX( 10 );
    m_4LayoutTL.AddY( 10 );
    m_4LayoutTL.SetChoiceButtonWidth( 0 );
    m_4LayoutTL.SetSliderWidth( 85 );
    m_4LayoutTL.SetFitWidthFlag( false );
    m_4LayoutTL.SetSameLineFlag( false );

    m_GenLayout.AddX( boxdim - 1 );

    m_GenLayout.AddSubGroupLayout( m_4LayoutTR, boxdim, boxdim - 40 );
    m_4LayoutTR.GetGroup()->box( Fl_Boxtype::FL_BORDER_BOX );
    m_4LayoutTR.AddX( 10 );
    m_4LayoutTR.AddY( 10 );
    m_4LayoutTR.SetChoiceButtonWidth( 0 );
    m_4LayoutTR.SetSliderWidth( 85 );
    m_4LayoutTR.SetFitWidthFlag( false );
    m_4LayoutTR.SetSameLineFlag( false );

    m_GenLayout.AddY( boxdim - 41 );
    m_GenLayout.SetX( 20 );

    m_GenLayout.AddSubGroupLayout( m_4LayoutBL, boxdim, boxdim - 40 );
    m_4LayoutBL.GetGroup()->box( Fl_Boxtype::FL_BORDER_BOX );
    m_4LayoutBL.AddX( 10 );
    m_4LayoutBL.AddY( 10 );
    m_4LayoutBL.SetChoiceButtonWidth( 0 );
    m_4LayoutBL.SetSliderWidth( 85 );
    m_4LayoutBL.SetFitWidthFlag( false );
    m_4LayoutBL.SetSameLineFlag( false );

    m_GenLayout.AddX( boxdim - 1 );

    m_GenLayout.AddSubGroupLayout( m_4LayoutBR, boxdim, boxdim - 40 );
    m_4LayoutBR.GetGroup()->box( Fl_Boxtype::FL_BORDER_BOX );
    m_4LayoutBR.AddX( 10 );
    m_4LayoutBR.AddY( 10 );
    m_4LayoutBR.SetChoiceButtonWidth( 0 );
    m_4LayoutBR.SetSliderWidth( 85 );
    m_4LayoutBR.SetFitWidthFlag( false );
    m_4LayoutBR.SetSameLineFlag( false );

    m_GenLayout.AddY( boxdim - 40 );

    m_4ViewChoice1.AddItem( "Left" );
    m_4ViewChoice1.AddItem( "Right" );
    m_4ViewChoice1.AddItem( "Top" );
    m_4ViewChoice1.AddItem( "Bottom" );
    m_4ViewChoice1.AddItem( "Front" );
    m_4ViewChoice1.AddItem( "Rear" );
    m_4ViewChoice1.AddItem( "None" );
    m_4LayoutTL.AddChoice( m_4ViewChoice1, "" );
    m_4LayoutTL.AddX( 10 );
    m_4RotChoice1.AddItem( "0" );
    m_4RotChoice1.AddItem( "90" );
    m_4RotChoice1.AddItem( "180" );
    m_4RotChoice1.AddItem( "270" );
    m_4LayoutTL.AddChoice( m_4RotChoice1, "" );

    m_4ViewChoice2.AddItem( "Left" );
    m_4ViewChoice2.AddItem( "Right" );
    m_4ViewChoice2.AddItem( "Top" );
    m_4ViewChoice2.AddItem( "Bottom" );
    m_4ViewChoice2.AddItem( "Front" );
    m_4ViewChoice2.AddItem( "Rear" );
    m_4ViewChoice2.AddItem( "None" );
    m_4LayoutTR.AddChoice( m_4ViewChoice2, "" );
    m_4LayoutTR.AddX( 10 );
    m_4RotChoice2.AddItem( "0" );
    m_4RotChoice2.AddItem( "90" );
    m_4RotChoice2.AddItem( "180" );
    m_4RotChoice2.AddItem( "270" );
    m_4LayoutTR.AddChoice( m_4RotChoice2, "" );

    m_4ViewChoice3.AddItem( "Left" );
    m_4ViewChoice3.AddItem( "Right" );
    m_4ViewChoice3.AddItem( "Top" );
    m_4ViewChoice3.AddItem( "Bottom" );
    m_4ViewChoice3.AddItem( "Front" );
    m_4ViewChoice3.AddItem( "Rear" );
    m_4ViewChoice3.AddItem( "None" );
    m_4LayoutBL.AddChoice( m_4ViewChoice3, "" );
    m_4LayoutBL.AddX( 10 );
    m_4RotChoice3.AddItem( "0" );
    m_4RotChoice3.AddItem( "90" );
    m_4RotChoice3.AddItem( "180" );
    m_4RotChoice3.AddItem( "270" );
    m_4LayoutBL.AddChoice( m_4RotChoice3, "" );

    m_4ViewChoice4.AddItem( "Left" );
    m_4ViewChoice4.AddItem( "Right" );
    m_4ViewChoice4.AddItem( "Top" );
    m_4ViewChoice4.AddItem( "Bottom" );
    m_4ViewChoice4.AddItem( "Front" );
    m_4ViewChoice4.AddItem( "Rear" );
    m_4ViewChoice4.AddItem( "None" );
    m_4LayoutBR.AddChoice( m_4ViewChoice4, "" );
    m_4LayoutBR.AddX( 10 );
    m_4RotChoice4.AddItem( "0" );
    m_4RotChoice4.AddItem( "90" );
    m_4RotChoice4.AddItem( "180" );
    m_4RotChoice4.AddItem( "270" );
    m_4LayoutBR.AddChoice( m_4RotChoice4, "" );

    m_GenLayout.AddYGap();
    m_GenLayout.SetX( 0 );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddX( 20 );
    m_GenLayout.AddButton( m_OkButton, "OK" );
    m_GenLayout.AddX( 10 );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );
    m_GenLayout.ForceNewLine();
}

SVGOptionsScreen::~SVGOptionsScreen()
{
}

bool SVGOptionsScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        m_LenUnitChoice.Update( veh->m_SVGLenUnit.GetID() );
        m_2DViewType.Update( veh->m_SVGView.GetID() );
        m_4ViewChoice1.Update( veh->m_SVGView1.GetID() );
        m_4ViewChoice2.Update( veh->m_SVGView2.GetID() );
        m_4ViewChoice3.Update( veh->m_SVGView3.GetID() );
        m_4ViewChoice4.Update( veh->m_SVGView4.GetID() );
        m_4RotChoice1.Update( veh->m_SVGView1_rot.GetID() );
        m_4RotChoice2.Update( veh->m_SVGView2_rot.GetID() );
        m_4RotChoice3.Update( veh->m_SVGView3_rot.GetID() );
        m_4RotChoice4.Update( veh->m_SVGView4_rot.GetID() );
        m_ProjectionLineToggle.Update( veh->m_SVGProjectionFlag.GetID() );
        m_TessSlider.Update( veh->m_SVGTessFactor.GetID() );
        m_XSecToggle.Update( veh->m_SVGAllXSecFlag.GetID() );

        if ( veh->m_SVGProjectionFlag() )
        {
            m_TessSlider.Activate();
        }
        else 
        {
            m_TessSlider.Deactivate();
        }

        if ( veh->m_SVGLenUnit() == vsp::LEN_UNITLESS )
        {
            veh->m_Scale.Set( 0 );
        }
        else 
        {
            GetScale( veh->m_SVGSet.Get() );
        }

        if ( veh->m_SVGView() == vsp::VIEW_1 )
        {
            m_4ViewChoice1.Activate();
            m_4ViewChoice2.Deactivate();
            m_4ViewChoice3.Deactivate();
            m_4ViewChoice4.Deactivate();
            m_4RotChoice1.Activate();
            m_4RotChoice2.Deactivate();
            m_4RotChoice3.Deactivate();
            m_4RotChoice4.Deactivate();
        }
        else if ( veh->m_SVGView() == vsp::VIEW_2HOR )
        {
            m_4ViewChoice1.Activate();
            m_4ViewChoice2.Activate();
            m_4ViewChoice3.Deactivate();
            m_4ViewChoice4.Deactivate();
            m_4RotChoice1.Activate();
            m_4RotChoice2.Activate();
            m_4RotChoice3.Deactivate();
            m_4RotChoice4.Deactivate();
        }
        else if ( veh->m_SVGView() == vsp::VIEW_2VER )
        {
            m_4ViewChoice1.Activate();
            m_4ViewChoice2.Deactivate();
            m_4ViewChoice3.Activate();
            m_4ViewChoice4.Deactivate();
            m_4RotChoice1.Activate();
            m_4RotChoice2.Deactivate();
            m_4RotChoice3.Activate();
            m_4RotChoice4.Deactivate();
        }
        else if ( veh->m_SVGView() == vsp::VIEW_4 )
        {
            m_4ViewChoice1.Activate();
            m_4ViewChoice2.Activate();
            m_4ViewChoice3.Activate();
            m_4ViewChoice4.Activate();
            m_4RotChoice1.Activate();
            m_4RotChoice2.Activate();
            m_4RotChoice3.Activate();
            m_4RotChoice4.Activate();
        }
    }

    m_FLTK_Window->redraw();

    return false;
}


void SVGOptionsScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void SVGOptionsScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void SVGOptionsScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_OkButton )
    {
        m_OkFlag = true;
        Hide();
    }
    else if ( device == &m_CancelButton )
    {
        Hide();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

bool SVGOptionsScreen::ShowSVGOptionsScreen( )
{
    Show();

    m_OkFlag = false;

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return m_OkFlag;
}

void SVGOptionsScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Hide();
}

void SVGOptionsScreen::GetScale( int write_set )
{
    bool foundgeom = false;

    Vehicle *veh = VehicleMgr.GetVehicle();

    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );
    BndBox svgbox;
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i]->GetSetFlag( write_set ) )
        {
            svgbox.Update( geom_vec[i]->GetBndBox() );
            foundgeom = true;
        }
    }

    if ( !foundgeom )
    {
        veh->m_Scale.Set( 0 ); // Set to 0 if no geoms are in the set
        return;
    }

    double length = svgbox.DiagDist();
    double convert_scale;

    if ( length >= 1.0 )
    {
        convert_scale = floor2scale( length, pow( 10.0, mag( length ) ) );
    }
    else
    {
        convert_scale = floor2scale( length, pow( 10.0, mag( length ) - 1 ) );
    }

    veh->m_Scale.Set( convert_scale );
}