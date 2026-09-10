//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MaterialEditScreen.cpp Material editor screen.
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "MaterialEditScreen.h"

MaterialEditScreen::MaterialEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 365, "Material Edit" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddInput( m_MaterialNameInput, "Name" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Ambient Reflection" );
    m_GenLayout.AddColorPicker( m_AmbientColorPicker );
    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Diffuse Reflection" );
    m_GenLayout.AddColorPicker( m_DiffuseColorPicker );
    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Specular Reflection" );
    m_GenLayout.AddColorPicker( m_SpecularColorPicker );
    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Emitted Light" );
    m_GenLayout.AddColorPicker( m_EmissiveColorPicker );
    m_GenLayout.AddYGap();

    m_GenLayout.AddSlider( m_AlphaSlider, "Alpha", 1.0, "%6.2f" );
    m_GenLayout.AddSlider( m_ShininessSlider, "Shininess", 128, "%3.0f" );

    m_GenLayout.AddYGap();

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetW() / 2 );
    m_GenLayout.AddButton( m_SaveApplyButton, "Apply" );
    m_GenLayout.AddButton( m_CancelButton, "Cancel" );

    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( false );
}



MaterialEditScreen::~MaterialEditScreen()
{
}

bool MaterialEditScreen::Update()
{
    BasicScreen::Update();

    assert( m_ScreenMgr );
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return false;
    }

    vec3d c;
    geom_ptr->GetMaterial()->GetAmbient( c );
    m_AmbientColorPicker.Update( c );

    geom_ptr->GetMaterial()->GetDiffuse( c );
    m_DiffuseColorPicker.Update( c );

    geom_ptr->GetMaterial()->GetSpecular( c );
    m_SpecularColorPicker.Update( c );

    geom_ptr->GetMaterial()->GetEmissive( c );
    m_EmissiveColorPicker.Update( c );

    MaterialMgr.m_ActiveGeom = geom_ptr->GetID();

    double a;
    geom_ptr->GetMaterial()->GetAlpha( a );
    MaterialMgr.m_Alpha = a;
    m_AlphaSlider.Update( MaterialMgr.m_Alpha.GetID() );

    double s;
    geom_ptr->GetMaterial()->GetShininess( s );
    MaterialMgr.m_Shininess = s;
    m_ShininessSlider.Update( MaterialMgr.m_Shininess.GetID() );

    m_MaterialNameInput.Update( geom_ptr->GetMaterial()->m_Name );

    return true;
}

void MaterialEditScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    // Edit a copy and set it back, so the Geom sees the change.
    Material mat;
    mat.SetMaterial( geom_ptr->GetMaterial() );

    if ( device == &m_AmbientColorPicker )
    {
        vec3d c = m_AmbientColorPicker.GetColor();
        mat.SetAmbient( c );
    }
    else if ( device == &m_DiffuseColorPicker )
    {
        vec3d c = m_DiffuseColorPicker.GetColor();
        mat.SetDiffuse( c );
    }
    else if ( device == &m_SpecularColorPicker )
    {
        vec3d c = m_SpecularColorPicker.GetColor();
        mat.SetSpecular( c );
    }
    else if ( device == &m_EmissiveColorPicker )
    {
        vec3d c = m_EmissiveColorPicker.GetColor();
        mat.SetEmissive( c );
    }
    else if ( device == &m_MaterialNameInput )
    {
        mat.m_Name = m_MaterialNameInput.GetString();
    }
    else if ( device == &m_SaveApplyButton )
    {
        string name = mat.m_Name;
        vector< string > names = MaterialMgr.GetNames();

        bool repeat = false;
        for( int i = 0; i < names.size(); i++ )
        {
            if ( names[i] == name )
            {
                repeat = true;
                break;
            }
        }

        if( !repeat )
        {
            Material newmat;
            newmat.SetMaterial( &mat );
            newmat.m_UserMaterial = true;

            MaterialMgr.AddMaterial( newmat );
            Hide();
        }
        else
        {
            m_ScreenMgr->Alert( "Please enter a unique name for your material." );
        }
    }
    else if ( device == &m_CancelButton )
    {
        // As in CloseCallBack, restore only a known material name.
        MaterialMgr.FindMaterial( m_OrigColor, mat );
        Hide();
    }

    geom_ptr->SetMaterial( mat );

    geom_ptr->ForceUpdate();
}

void MaterialEditScreen::CloseCallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( geom_ptr )
    {
        // Restore only a known material name.  "Custom" is not in the list, and looking it up
        // would replace the material with the default.
        Material mat;
        mat.SetMaterial( geom_ptr->GetMaterial() );

        if ( MaterialMgr.FindMaterial( m_OrigColor, mat ) )
        {
            geom_ptr->SetMaterial( mat );
            geom_ptr->ForceUpdate();
        }
    }
    Hide();
}
