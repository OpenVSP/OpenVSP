//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkScreen.h"
#include "ScreenMgr.h"
#include "CustomGeom.h"
#include "LinkMgr.h"
#include "ParmMgr.h"

#include <assert.h>

//==== Constructor ====//
AdvLinkScreen::AdvLinkScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    AdvLinkUI* ui = m_AdvLinkUI = new AdvLinkUI();
    VspScreen::SetFlWindow( ui->UIWindow );

    m_BigGroup.SetGroupAndScreen( ui->windowGroup, this );
    m_BigGroup.AddDividerBox( "Adv Links" );
    m_BigGroup.AddInput( m_NameInput, "Link Name:" );


    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );
    m_LinkBrowser = m_LinkBrowserGroup.AddFlBrowser( 70 );

    m_BigGroup.AddButton( m_AddLink, "Add" );
    int gap = 12;
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_DelLink, "Del" );
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_DelAllLink, "Del_All" );
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_SaveLink, "Save" );
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_LoadLink, "Load" );
    m_BigGroup.ForceNewLine();


    m_BigGroup.SetFitWidthFlag( true );
    m_BigGroup.SetSameLineFlag( false );

    m_BigGroup.AddYGap();
    m_BigGroup.AddDividerBox( "Parm Picker" );

    m_BigGroup.AddParmPicker( m_ParmPicker );
    m_BigGroup.SetButtonWidth(100);

    m_BigGroup.AddInput( m_VarNameInput, "Var Name:" );
    m_BigGroup.AddYGap();

    gap = 4;
    m_BigGroup.SetFitWidthFlag( false );
    m_BigGroup.SetSameLineFlag( true );
    m_BigGroup.SetButtonWidth( m_BigGroup.GetRemainX()/2 - gap/2 );
    m_BigGroup.AddButton( m_PickInput, "Add_Input" );
    m_BigGroup.AddX( gap );
    m_BigGroup.AddButton( m_PickOutput, "Add_Output" );
    m_BigGroup.SetFitWidthFlag( true );
    m_BigGroup.SetSameLineFlag( false );
    m_BigGroup.ForceNewLine();
    m_BigGroup.AddYGap();

    m_BigGroup.AddSubGroupLayout( m_InputGroup, m_BigGroup.GetW() / 2 - 2, 150 );
    m_BigGroup.AddX( m_BigGroup.GetW() / 2 + 2 );
    m_BigGroup.AddSubGroupLayout( m_OutputGroup, m_BigGroup.GetW() / 2 - 2, 150 );

    m_InputGroup.AddDividerBox("Input Parms");
    m_InputBrowser = m_InputGroup.AddFlBrowser( 100 );
    m_InputGroup.SetFitWidthFlag( false );
    m_InputGroup.SetSameLineFlag( true );
    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelInput, "Del" );
    m_InputGroup.AddX( 20 );
    m_InputGroup.AddButton( m_DelAllInput, "Del All" );

    m_OutputGroup.AddDividerBox("Output Parms");
    m_OutputBrowser = m_OutputGroup.AddFlBrowser( 100 );
    m_OutputGroup.SetFitWidthFlag( false );
    m_OutputGroup.SetSameLineFlag( true );
    m_OutputGroup.AddX( 20 );
    m_OutputGroup.AddButton( m_DelOutput, "Del" );
    m_OutputGroup.AddX( 20 );
    m_OutputGroup.AddButton( m_DelAllOutput, "Del All" );

    m_BigGroup.ForceNewLine();
    m_BigGroup.AddY( 120 );
    m_BigGroup.AddDividerBox("Code");

}

//==== Update Screen ====//
bool AdvLinkScreen::Update()
{


    return true;
}

//==== Show Screen ====//
void AdvLinkScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void AdvLinkScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Callbacks ====//
void AdvLinkScreen::CallBack( Fl_Widget *w )
{

}


void AdvLinkScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
 
}
