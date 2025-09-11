//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehNotesScreen.cpp: GUI Screen for displaying Vehicle Notes attribute only- with bool parm for autoshow setting
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#include "VehNotesScreen.h"
#include "AttributeManager.h"
#include "ScreenBase.h"

//===============================================================================//
//===============================================================================//
//===============================================================================//

VehNotesScreen::VehNotesScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 600, 125, "Vehicle Notes" )
{
    m_ScreenMgr = mgr;

    int buffer = 5;

    m_NotesScreenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_FLTK_Window->resizable( m_NotesScreenLayout.GetGroup() );
    m_FLTK_Window->size_range( m_FLTK_Window->w()/2, m_FLTK_Window->h()/2 );

    m_NotesScreenLayout.ForceNewLine();
    m_NotesScreenLayout.AddYGap();

    m_NotesScreenLayout.AddSubGroupLayout( m_NotesLayout, m_NotesScreenLayout.GetW() - buffer * 2, m_NotesScreenLayout.GetRemainY() - buffer );
    m_NotesScreenLayout.GetGroup()->resizable( m_NotesLayout.GetGroup() );

    m_NotesLayout.SetX( m_NotesScreenLayout.GetStartX() + buffer );
    //==== Toggle Bool Parm for Show Notes on File Open ====//
    m_NotesLayout.AddYGap();
    m_NotesLayout.SetSameLineFlag( true );

    int toggle_label_width = 2*m_NotesLayout.GetButtonWidth();
    int toggle_btn_width = m_NotesLayout.GetStdHeight();

    // ToggleButton replaced with 3-item GUI arrangement here to better suit the large button width and add a text description to the field
    // Blank Output with Wide Label
    m_NotesLayout.SetButtonWidth( toggle_label_width );
    m_NotesLayout.AddOutput( m_ShowToggleLabel, "Show Notes on Open" );

    // Toggle Button
    m_NotesLayout.AddButton( m_ShowDataToggleIn, "" );

    // Output Field Text Description
    m_NotesLayout.SetButtonWidth( 0 );
    m_NotesLayout.AddOutput( m_ShowToggleField, "" );

    m_ShowToggleLabel.SetWidth( toggle_label_width );
    m_ShowDataToggleIn.SetWidth( toggle_btn_width );
    m_ShowToggleField.SetWidth( m_NotesLayout.GetW() - toggle_label_width - toggle_btn_width );

    m_ShowDataToggleIn.SetX( toggle_label_width + buffer );
    m_ShowToggleField.SetX( toggle_label_width + buffer + toggle_btn_width );
    m_NotesLayout.SetSameLineFlag( false );
    m_NotesLayout.ForceNewLine();

    // create text editor, and tie its widget callbacks to the staticScreenCB for updating the attribute when called
    m_NotesLayout.SetX( m_NotesScreenLayout.GetStartX() + buffer );

    bool resizable = true;
    m_DataBuffer = new Fl_Text_Buffer;
    m_DataText = m_NotesLayout.AddVspTextEditor( m_NotesLayout.GetRemainY(), m_DataBuffer, staticScreenCB, this, resizable );

    Vehicle* veh = VehicleMgr.GetVehicle();
    m_ShowBoolParmPtr = &veh->m_ShowNotesScreenParm;
}

VehNotesScreen::~VehNotesScreen()
{
    m_DataText->buffer( nullptr );
    delete m_DataBuffer;
}

void VehNotesScreen::Show()
{
    if ( Update() )
    {
        BasicScreen::Show();
    }
}

bool VehNotesScreen::Update()
{
    
    NameValData* attr = AttributeMgr.GetAttributePtr( ATTR_VEH_NOTES );

    string attrBufferText = string();

    m_ShowDataToggleIn.Update( m_ShowBoolParmPtr->GetID() );
    if ( m_ShowBoolParmPtr->Get() )
    {
        m_ShowToggleField.Update( "True" );
    }
    else
    {
        m_ShowToggleField.Update( "False" );
    }

    if ( attr )
    {
        attrBufferText = attr->GetString( 0 );
    }
    m_DataBuffer->text( attrBufferText.c_str() );

    return true;
};

void VehNotesScreen::CallBack( Fl_Widget *w )
{
    // if either text editor is called, run AttributeModify without changing the name
    if ( dynamic_cast<Fl_Text_Editor * >( w ) == m_DataText )
    {
        AttributeMgr.SetAttributeString( ATTR_VEH_NOTES, m_DataBuffer->text() );
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void VehNotesScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{

    m_ScreenMgr->SetUpdateFlag( true );
}