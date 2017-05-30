//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FeaPartEditScreen.cpp FeaPart editor screen.
// Justin Gravett
//
//////////////////////////////////////////////////////////////////////

#include "FeaPartEditScreen.h"

FeaPartEditScreen::FeaPartEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 365, "FEA Part Edit" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
}



FeaPartEditScreen::~FeaPartEditScreen()
{
}

bool FeaPartEditScreen::Update()
{
    assert( m_ScreenMgr );

    return true;
}

void FeaPartEditScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

}

void FeaPartEditScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    Hide();
}
