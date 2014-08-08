//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// MaterialEditScreen.cpp Material editor screen.
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#include "MaterialEditScreen.h"

MaterialEditScreen::MaterialEditScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 400, "Material Edit" )
{

    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddDividerBox( "Ambient" );
    m_GenLayout.AddColorPicker( m_AmbientColorPicker );
    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Diffuse" );
    m_GenLayout.AddColorPicker( m_DiffuseColorPicker );
    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Specular" );
    m_GenLayout.AddColorPicker( m_SpecularColorPicker );
    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Emissive" );
    m_GenLayout.AddColorPicker( m_EmissiveColorPicker );
    m_GenLayout.AddYGap();

    m_GenLayout.AddSlider( m_AlphaSlider, "Alpha", 1.0, "%6.2f" );
    m_GenLayout.AddSlider( m_ShininessSlider, "Shininess", 128, "%3.0f" );

    m_GenLayout.AddYGap();

    m_GenLayout.AddInput( m_MaterialNameInput, "Name" );
    m_GenLayout.AddButton( m_SaveApplyButton, "Save & Apply" );
}



MaterialEditScreen::~MaterialEditScreen()
{
}

bool MaterialEditScreen::Update()
{
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

    if ( device == &m_AmbientColorPicker )
    {
        vec3d c = m_AmbientColorPicker.GetColor();
        geom_ptr->GetMaterial()->SetAmbient( c );
    }
    else if ( device == &m_DiffuseColorPicker )
    {
        vec3d c = m_DiffuseColorPicker.GetColor();
        geom_ptr->GetMaterial()->SetDiffuse( c );
    }
    else if ( device == &m_SpecularColorPicker )
    {
        vec3d c = m_SpecularColorPicker.GetColor();
        geom_ptr->GetMaterial()->SetSpecular( c );
    }
    else if ( device == &m_EmissiveColorPicker )
    {
        vec3d c = m_EmissiveColorPicker.GetColor();
        geom_ptr->GetMaterial()->SetEmissive( c );
    }
    else if ( device == &m_SaveApplyButton )
    {
        string name = geom_ptr->GetMaterial()->m_Name;
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
            Material newmat = *(geom_ptr->GetMaterial());
            newmat.m_UserMaterial = true;

            MaterialMgr.AddMaterial( newmat );
            Hide();
        }
        else
        {
            m_ScreenMgr->Alert( "Please enter a unique name for your material." );
        }
    }
    else if ( device == &m_MaterialNameInput )
    {
        geom_ptr->GetMaterial()->m_Name = m_MaterialNameInput.GetString();
    }

    geom_ptr->ForceUpdate();
}
