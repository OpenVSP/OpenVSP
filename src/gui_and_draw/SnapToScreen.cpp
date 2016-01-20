//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// SnapToScreen.cpp: implementation of the PackingScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "SnapToScreen.h"
#include "StringUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SnapToScreen::SnapToScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 330, "Snap To" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY(7);
    m_GenLayout.AddX(5);

    m_GenLayout.AddSubGroupLayout( m_BorderLayout, m_GenLayout.GetRemainX() - 5,
                                   m_GenLayout.GetRemainY() - 5);

    ( ( Vsp_Group* ) m_BorderLayout.GetGroup() )->SetAllowDrop( true );
    m_BorderLayout.GetGroup()->callback( staticScreenCB, this );

    int gap = 4;

    m_BorderLayout.SetButtonWidth( 100 );
    m_BorderLayout.AddSlider( m_TargetMinDistSlider, "Target Min Dist", 1, "%7.3f" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetButtonWidth( 150 );
    m_BorderLayout.AddOutput( m_MinDistOutput, "Result Min Dist" );

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Collision Set And Method" );
    m_BorderLayout.AddChoice( m_SetChoice, "Set:" );
    m_BorderLayout.AddChoice( m_MethodChoice, "Method:" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddYGap();
    m_BorderLayout.AddDividerBox( "Interactive Collision Detection" );
    m_BorderLayout.AddLabel("Press Alt (Option) Key and change parm", 290 );
    m_BorderLayout.ForceNewLine();
    m_BorderLayout.AddYGap();
    m_BorderLayout.AddYGap();


    m_BorderLayout.AddDividerBox( "Manual Collision Detection" );
    m_BorderLayout.AddParmPicker( m_ParmPicker );

    m_BorderLayout.AddYGap();
    m_BorderLayout.SetButtonWidth( 100 );
    m_BorderLayout.AddSlider( m_ValSlider, "AUTO_UPDATE", 10, "%7.3f" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetRemainX()/2 - gap/2 );
    m_BorderLayout.AddButton( m_DecVal, "Decrease" );
    m_BorderLayout.AddX( gap );
    m_BorderLayout.AddButton( m_IncVal, "Increase" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );


    m_BorderLayout.AddYGap();

 }

SnapToScreen::~SnapToScreen()
{
}

bool SnapToScreen::Update()
{
    Vehicle *veh = VehicleMgr.GetVehicle();
    SnapTo *snap = veh->GetSnapToPtr();

    m_ParmPicker.Update();

    string parm_id = m_ParmPicker.GetParmChoice();
    m_ValSlider.Update( parm_id );

    m_SetChoice.ClearItems();
    vector< string > set_name_vec = veh->GetSetNameVec();
    for ( int i = 0 ; i < ( int )set_name_vec.size() ; ++i )
    {
        m_SetChoice.AddItem( set_name_vec[i].c_str() );
    }
    m_SetChoice.UpdateItems();
    m_SetChoice.SetVal( snap->m_CollisionSet );

    m_MethodChoice.ClearItems();
    m_MethodChoice.AddItem( "Mesh - Faster" );
//    m_MethodChoice.AddItem( "Surface - More Accurate" );
    m_MethodChoice.UpdateItems();

    m_TargetMinDistSlider.Update( snap->m_CollisionTargetDist.GetID() );

    string s_out = StringUtil::double_to_string( snap->m_CollisionMinDist, "  %8.7f" );
    if ( snap->m_CollisionErrorFlag ==  vsp::COLLISION_INTERSECT_NO_SOLUTION )
        s_out = string("  Touching No Soln ");
    else if ( snap->m_CollisionErrorFlag ==  vsp::COLLISION_CLEAR_NO_SOLUTION )
        s_out = string("  Not Touching No Soln ");

    m_MinDistOutput.Update( s_out );

    m_FLTK_Window->redraw();

    return false;
}


void SnapToScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void SnapToScreen::CallBack( Fl_Widget* w )
{
    if ( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
    {
        string ParmID( Fl::event_text() );
        m_ParmPicker.SetParmChoice( ParmID );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void SnapToScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle *veh = VehicleMgr.GetVehicle();
    SnapTo *snap = veh->GetSnapToPtr();

    if ( device == &m_SetChoice )
    {
        snap->m_CollisionSet = m_SetChoice.GetVal();
    }
    else if ( device == &m_MethodChoice )
    {
    }
    else if ( device == &m_IncVal )
    {
        string parm_id = m_ParmPicker.GetParmChoice();        
        snap->AdjParmToMinDist( parm_id, true );
    }
    else if ( device == &m_DecVal )
    {
        string parm_id = m_ParmPicker.GetParmChoice();        
        snap->AdjParmToMinDist( parm_id, false );
    }


    m_ScreenMgr->SetUpdateFlag( true );
}

bool SnapToScreen::ShowSnapToScreen()
{
    Show();

    while( m_FLTK_Window->shown() )
    {
        Fl::wait();
    }

    return true;
}

void SnapToScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );


    Hide();
}
