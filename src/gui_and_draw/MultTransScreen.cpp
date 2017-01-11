#include "MultTransScreen.h"
#include "ScreenMgr.h"

MultTransScreen::MultTransScreen( ScreenMgr * mgr ) : TabScreen( mgr, 300, 270, "Group Modifications" )
{
    // Set the window as a geom screen window
    VSP_Window* vsp_win = dynamic_cast<VSP_Window*>(m_FLTK_Window);

    vsp_win->SetGeomScreenFlag( true );

    //==== XForm Tab ====//
    Fl_Group* xform_tab = AddTab( "XForm" );
    m_TransTabMainLayout.SetGroupAndScreen( xform_tab, this );
    m_TransTabMainLayout.AddX( 5 );
    m_TransTabMainLayout.AddSubGroupLayout( m_TransTabGenLayout, m_TransTabMainLayout.GetRemainX() - 5, m_TransTabMainLayout.GetRemainY() );

    m_TransTabGenLayout.AddDividerBox( "Transformations and Scaling" );

    m_TransTabGenLayout.AddSlider( m_XLoc, "XLoc", 10.0, "%7.3f" );
    m_TransTabGenLayout.AddSlider( m_YLoc, "YLoc", 10.0, "%7.3f" );
    m_TransTabGenLayout.AddSlider( m_ZLoc, "ZLoc", 10.0, "%7.3f" );
    m_TransTabGenLayout.AddYGap();

    m_TransTabGenLayout.AddSlider( m_XRot, "XRot", 10.0, "%7.3f" );
    m_TransTabGenLayout.AddSlider( m_YRot, "YRot", 10.0, "%7.3f" );
    m_TransTabGenLayout.AddSlider( m_ZRot, "ZRot", 10.0, "%7.3f" );
    m_TransTabGenLayout.AddYGap();

    m_TransTabGenLayout.AddButton( m_ApplyScaleToTranslations, "Scale Translations" );
    m_TransTabGenLayout.AddSlider(m_Scale, "Scale", 1, " %5.4f" );

    m_TransTabGenLayout.AddYGap();

    m_TransTabGenLayout.SetButtonWidth( m_TransTabGenLayout.GetRemainX() / 2.0 );
    m_TransTabGenLayout.SetFitWidthFlag( false );
    m_TransTabGenLayout.SetSameLineFlag( true );
    m_TransTabGenLayout.AddButton( m_AcceptButton, "Accept" );
    m_TransTabGenLayout.AddButton( m_ResetButton, "Reset" );

    //==== General Properties Tab
    Fl_Group* gen_tab = AddTab( "Gen" );
    m_MaterialTabMainLayout.SetGroupAndScreen( gen_tab, this );
    m_MaterialTabMainLayout.AddX( 5 );
    m_MaterialTabMainLayout.AddSubGroupLayout( m_MaterialTabGenLayout, m_MaterialTabMainLayout.GetRemainX() - 5, m_MaterialTabMainLayout.GetRemainY() );

    m_MaterialTabGenLayout.AddDividerBox( "Material & Color" );
    m_MaterialTabGenLayout.AddYGap();
    m_MaterialTabGenLayout.AddColorPicker( m_ColorPicker );
    m_MaterialTabGenLayout.AddYGap();

    UpdateMaterialNames();

    m_MaterialTabGenLayout.SetFitWidthFlag( true );
    m_MaterialTabGenLayout.SetSameLineFlag( true );

    m_MaterialTabGenLayout.AddChoice( m_MaterialChoice, "Material:", m_MaterialTabGenLayout.GetButtonWidth() );

    m_MaterialTabGenLayout.SetFitWidthFlag( false );
    //m_MaterialTabGenLayout.AddButton( m_CustomMaterialButton, "Custom" );
    m_MaterialTabGenLayout.ForceNewLine();

    m_MaterialTabGenLayout.SetFitWidthFlag( true );
    m_MaterialTabGenLayout.SetSameLineFlag( false );

    xform_tab->show();
}

MultTransScreen::~MultTransScreen()
{
}

void MultTransScreen::Show()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    assert( veh );

    // Reset the vehicle group transformation variables
    veh->GetGroupTransformationsPtr()->ReInitialize();

    // Update and show if applicable
    if ( Update() )
    {
        TabScreen::Show();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

bool MultTransScreen::Update()
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    assert( veh );

    // Check that more than one Geom is active
    if ( veh->GetActiveGeomVec().size() <= 1 )
    {
        Hide();
        return false;
    }

    // Get the group transformations class
    GroupTransformations* group_trans = veh->GetGroupTransformationsPtr();
    m_XLoc.Update( group_trans->m_GroupXLoc.GetID());
    m_YLoc.Update( group_trans->m_GroupYLoc.GetID());
    m_ZLoc.Update( group_trans->m_GroupZLoc.GetID());
    m_XRot.Update( group_trans->m_GroupXRot.GetID() );
    m_YRot.Update( group_trans->m_GroupYRot.GetID() );
    m_ZRot.Update( group_trans->m_GroupZRot.GetID() );
    m_Scale.Update( group_trans->m_GroupScale.GetID());
    m_ApplyScaleToTranslations.Update( group_trans->m_scaleGroupTranslations.GetID());

    // Update the wire color
    m_ColorPicker.Update( group_trans->GetColor() );

    // Update the material properties
    UpdateMaterialNames();
    m_MaterialChoice.UpdateItems();
    m_MaterialChoice.SetVal( 0 );

    Material * mat = group_trans->GetMaterial();
    std::vector< std::string > choices = m_MaterialChoice.GetItems();
    for ( int i = 0; i < (int)choices.size(); i++ )
    {
        if ( mat->m_Name == choices[i] )
        {
            m_MaterialChoice.SetVal( i );
            break;
        }
    }

    return true;
}

void MultTransScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    assert( veh );
    GroupTransformations* trans = veh->GetGroupTransformationsPtr();
    assert( trans );

    // If the device is the accept button,
    // then accept the values
    if ( device == &m_AcceptButton )
    {
        trans->Accept();
    }
    else if ( device == &m_ResetButton )
    {
        trans->Reset();
    }
    else if ( device == &m_MaterialChoice )
    {
        int index = m_MaterialChoice.GetVal() - 1;
        Material mat;
        if ( !MaterialMgr.FindMaterial( index, mat ) )
        {
            mat.SetMaterialToDefault();
        }

        trans->SetMaterial( mat );
    }
    else if ( device == &m_ColorPicker )
    {
        trans->SetColor( m_ColorPicker.GetColor() );
    }

    m_ScreenMgr->SetUpdateFlag( true );

//    if ( device == &m_LightChoice )
//    {
//        m_CurrentSelected = m_LightChoice.GetVal();
//    }
//    m_ScreenMgr->SetUpdateFlag( true );
}

// ==== Updates the list of material choices ==== //
void MultTransScreen::UpdateMaterialNames()
{
    std::vector<std::string> matNames;
    matNames = MaterialMgr.GetNames();

    m_MaterialChoice.ClearItems();
    m_MaterialChoice.AddItem( "DEFAULT" );
    for ( int i = 0; i < (int)matNames.size(); i++ )
    {
        m_MaterialChoice.AddItem( matNames[i] );
    }
}
