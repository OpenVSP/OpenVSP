//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// StructAssemblyScreen.cpp: implementation of the StructAssemblyScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "StructAssemblyScreen.h"
#include "FeaMeshMgr.h"
#include "StructureMgr.h"
#include "ManageViewScreen.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Camera.h"
#include "ParmMgr.h"

using namespace vsp;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StructAssemblyScreen::StructAssemblyScreen( ScreenMgr* mgr ) : TabScreen( mgr, 430, 650 + 30, "FEA Assembly", 196 )
{
    m_FLTK_Window->callback( staticCloseCB, this );

}

StructAssemblyScreen::~StructAssemblyScreen()
{
}

void StructAssemblyScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();

    m_ScreenMgr->SetUpdateFlag( true );
}

void StructAssemblyScreen::Show()
{
    m_FLTK_Window->show();

    m_ScreenMgr->SetUpdateFlag( true );
}


bool StructAssemblyScreen::Update()
{
    return true;
}

void StructAssemblyScreen::AddOutputText( const string &text )
{
    Fl::lock();
    Fl::unlock();
}

void StructAssemblyScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );


    m_ScreenMgr->SetUpdateFlag( true );
}

void StructAssemblyScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );


    m_ScreenMgr->SetUpdateFlag( true );
}

void StructAssemblyScreen::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( !veh )
    {
        return;
    }

    if ( IsShown() )
    {
        // Load Draw Objects for FeaMesh
        FeaMeshMgr.LoadDrawObjs( draw_obj_vec );
    }
}
