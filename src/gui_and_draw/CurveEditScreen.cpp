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

//==== Update Curve Edit Screen ====//
bool CurveEditScreen::Update()
{
    assert( m_ScreenMgr );


    return true;
}

void CurveEditScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{

    m_ScreenMgr->SetUpdateFlag( true );
}


void CurveEditScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Hide();
}
