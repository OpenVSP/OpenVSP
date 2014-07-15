//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClippingScreen.cpp: implementation of the ClippingScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ClippingScreen.h"
#include "ClippingMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClippingScreen::ClippingScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 170, "View Clipping" )
{
    int w = m_FLTK_Window->w();
    int h = m_FLTK_Window->h();

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.SetButtonWidth( 75 );

    m_GenLayout.SetSameLineFlag( true );

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_XGTClipToggle, "Active" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_XGTClipSlider, "X >", 10, "%7.3f" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_XLTClipToggle, "Active" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_XLTClipSlider, "X <", 10, "%7.3f" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_YGTClipToggle, "Active" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_YGTClipSlider, "Y >", 10, "%7.3f" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_YLTClipToggle, "Active" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_YLTClipSlider, "Y <", 10, "%7.3f" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_ZGTClipToggle, "Active" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_ZGTClipSlider, "Z >", 10, "%7.3f" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_ZLTClipToggle, "Active" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddSlider( m_ZLTClipSlider, "Z <", 10, "%7.3f" );
    m_GenLayout.ForceNewLine();

}

ClippingScreen::~ClippingScreen()
{
}

bool ClippingScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        ClippingMgr *clip = veh->GetClippinMgrPtr();

        if( clip )
        {
            m_XGTClipSlider.Update( clip->m_XGTClip.GetID() );
            m_XLTClipSlider.Update( clip->m_XLTClip.GetID() );
            m_YGTClipSlider.Update( clip->m_YGTClip.GetID() );
            m_YLTClipSlider.Update( clip->m_YLTClip.GetID() );
            m_ZGTClipSlider.Update( clip->m_ZGTClip.GetID() );
            m_ZLTClipSlider.Update( clip->m_ZLTClip.GetID() );

            m_XGTClipToggle.Update( clip->m_XGTClipFlag.GetID() );
            m_XLTClipToggle.Update( clip->m_XLTClipFlag.GetID() );
            m_YGTClipToggle.Update( clip->m_YGTClipFlag.GetID() );
            m_YLTClipToggle.Update( clip->m_YLTClipFlag.GetID() );
            m_ZGTClipToggle.Update( clip->m_ZGTClipFlag.GetID() );
            m_ZLTClipToggle.Update( clip->m_ZLTClipFlag.GetID() );
        }
    }

    m_FLTK_Window->redraw();

    return false;
}


void ClippingScreen::Show()
{
    Update();
    m_FLTK_Window->show();

}

void ClippingScreen::Hide()
{
    m_FLTK_Window->hide();
}

void ClippingScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void ClippingScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}

void ClippingScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if( veh )
    {
        ClippingMgr *clip = veh->GetClippinMgrPtr();

        if( clip )
        {
            clip->LoadDrawObjs( draw_obj_vec );
        }
    }
}
