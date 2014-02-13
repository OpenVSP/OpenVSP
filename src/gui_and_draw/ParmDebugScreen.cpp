//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParmDebugScreen.cpp: implementation of the ParmDebugScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ParmDebugScreen.h"
#include "ParmMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ParmDebugScreen::ParmDebugScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 500, "Parameter Debug" )
{
    int w = m_FLTK_Window->w();
    int h = m_FLTK_Window->h();

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddYGap();

    m_GenLayout.AddParmPicker( m_ParmPicker );

    m_GenLayout.AddYGap();

    m_GenLayout.SetButtonWidth( 100 );

    m_GenLayout.AddOutput( m_ContainerOutput, "Container" );
    m_GenLayout.AddOutput( m_GroupOutput, "Group" );
    m_GenLayout.AddOutput( m_ParmOutput, "Parm" );
    m_GenLayout.AddOutput( m_DescOutput, "Description" );
    m_GenLayout.AddOutput( m_MinOutput, "Minimum" );
    m_GenLayout.AddOutput( m_MaxOutput, "Maximum" );
    m_GenLayout.AddOutput( m_ValOutput, "Value" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetButtonWidth( 74 );

    m_GenLayout.AddSlider( m_ParmSlider, "AUTO_UPDATE", 10, "%7.3f" );
}

ParmDebugScreen::~ParmDebugScreen()
{
}

bool ParmDebugScreen::Update()
{
    int i;
    char str[256];

    m_ParmPicker.Update();

    Parm *p = ParmMgr.FindParm( m_ParmPicker.GetParmChoice() );
    if( p )
    {
        string c_name, g_name, p_name;
        ParmMgr.GetNames( p->GetID(), c_name, g_name, p_name );

        m_ContainerOutput.Update( c_name );
        m_GroupOutput.Update( g_name );
        m_ParmOutput.Update( p_name );
        m_DescOutput.Update( p->GetDescript() );
        m_MinOutput.Update( std::to_string( ( long double )p->GetLowerLimit() ) );
        m_MaxOutput.Update( std::to_string( ( long double )p->GetUpperLimit() ) );
        m_ValOutput.Update( std::to_string( ( long double )p->Get() ) );
    }

    m_ParmSlider.Update( m_ParmPicker.GetParmChoice() );

    m_FLTK_Window->redraw();

    return false;
}


void ParmDebugScreen::Show()
{
    Update();
    m_FLTK_Window->show();

}

void ParmDebugScreen::Hide()
{
    m_FLTK_Window->hide();
}

void ParmDebugScreen::CallBack( Fl_Widget* w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void ParmDebugScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );
}
