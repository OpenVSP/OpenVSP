//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "MdcFinScreen.h"
#include "ScreenMgr.h"
#include "MdcFinGeom.h"
#include "ManageGeomScreen.h"

//==== Constructor ====//
MdcFinScreen::MdcFinScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "MDC_FIN", "MdcFin.html" )
{
    //==== Fin Set ====//
    Fl_Group* fin_tab = AddTab( "Fin Set" );
    Fl_Group* fin_group = AddSubGroup( fin_tab, 5 );
    m_FinSetLayout.SetGroupAndScreen( fin_group, this );

    // Choices' labels as wide as sliders', so they line up.
    m_FinSetLayout.SetChoiceButtonWidth( m_FinSetLayout.GetButtonWidth() );

    m_FinSetLayout.AddButton( m_ConvertToWingButton, "Convert to Wing" );

    m_FinSetLayout.AddYGap();
    m_FinSetLayout.AddDividerBox( "Fin Set" );
    // A preset shape, set on demand so a choice keeps the shape as it is.
    m_FinSetLayout.SetSameLineFlag( true );
    m_FinSetLayout.SetFitWidthFlag( true );
    m_FinSetLayout.AddChoice( m_PresetChoice, "Preset", m_FinSetLayout.GetButtonWidth() );
    m_FinSetLayout.SetFitWidthFlag( false );
    m_FinSetLayout.AddButton( m_SetPresetButton, "Set" );
    m_FinSetLayout.ForceNewLine();
    m_FinSetLayout.SetSameLineFlag( false );
    m_FinSetLayout.SetFitWidthFlag( true );
    m_PresetChoice.AddItem( "Default", vsp::MDC_FIN_PRESET_DEFAULT );
    m_PresetChoice.AddItem( "Generic Rocket Fins", vsp::MDC_FIN_PRESET_GENERIC_ROCKET_FINS );
    m_PresetChoice.AddItem( "Generic Rocket Strakes", vsp::MDC_FIN_PRESET_GENERIC_ROCKET_STRAKES );
    m_PresetChoice.UpdateItems();
    m_FinSetLayout.AddChoice( m_SecTypChoice, "SECTYP" );
    m_SecTypChoice.AddItem( "HEX" );
    m_SecTypChoice.AddItem( "NACA" );
    m_SecTypChoice.AddItem( "ARC" );
    m_SecTypChoice.AddItem( "USER" );
    m_SecTypChoice.UpdateItems();
    m_FinSetLayout.AddSlider( m_NPanelSlider, "NPANEL", 10, "%5.0f" );
    m_FinSetLayout.AddSlider( m_RBodySlider, "RBODY", 10, "%7.3f" );
    m_FinSetLayout.AddButton( m_BuryRootToggle, "Bury Root" );
    m_FinSetLayout.AddChoice( m_BuryRootTypeChoice, "Extension" );
    m_BuryRootTypeChoice.AddItem( "Straight", MdcFinGeom::BURY_STRAIGHT );
    m_BuryRootTypeChoice.AddItem( "Extrapolate", MdcFinGeom::BURY_EXTRAPOLATE );
    m_BuryRootTypeChoice.UpdateItems();

    m_FinSetLayout.AddYGap();
    m_FinSetLayout.AddDividerBox( "Panels" );
    m_FinSetLayout.AddButton( m_PhiFFlagToggle, "PHIF given" );
    m_FinSetLayout.AddIndexSelector( m_PanelSelector );
    m_FinSetLayout.AddSlider( m_PhiFSlider, "PHIF", 360.0, "%7.3f" );
    m_FinSetLayout.AddSlider( m_GamSlider, "GAM", 90.0, "%7.3f" );

    //==== Planform ====//
    Fl_Group* planform_tab = AddTab( "Planform" );
    Fl_Group* planform_group = AddSubGroup( planform_tab, 5 );
    m_PlanformLayout.SetGroupAndScreen( planform_group, this );
    m_PlanformLayout.SetChoiceButtonWidth( m_PlanformLayout.GetButtonWidth() );

    m_PlanformLayout.AddYGap();
    m_PlanformLayout.AddDividerBox( "Span Stations" );
    m_PlanformLayout.AddSlider( m_NSpanSlider, "NSPAN", 10, "%5.0f" );
    m_PlanformLayout.AddIndexSelector( m_StationSelector );
    m_PlanformLayout.AddSlider( m_SSpanSlider, "SSPAN", 10, "%7.3f" );
    m_PlanformLayout.AddSlider( m_ChordSlider, "CHORD", 10, "%7.3f" );
    m_PlanformLayout.AddSlider( m_XLESlider, "XLE", 10, "%7.3f" );
    m_PlanformLayout.AddButton( m_XLEFlagToggle, "XLE of next given" );
    m_PlanformLayout.AddSlider( m_SweepSlider, "SWEEP", 90.0, "%7.3f" );
    m_PlanformLayout.AddSlider( m_StaSlider, "STA", 1.0, "%7.3f" );
    m_PlanformLayout.AddSlider( m_SectTessUSlider, "Num_U", 20, "%5.0f" );
    m_PlanformLayout.AddSlider( m_LERSlider, "LER", 1.0, "%7.3f" );
    m_PlanformLayout.AddChoice( m_LERTypeChoice, "LER Type" );
    m_LERTypeChoice.AddItem( "Sharp" );
    m_LERTypeChoice.AddItem( "Keep Crest" );
    m_LERTypeChoice.AddItem( "Keep Angle" );
    m_LERTypeChoice.UpdateItems();
    m_PlanformLayout.AddSlider( m_CFOCSlider, "CFOC", 1.0, "%7.3f" );

    //==== Section ====//
    Fl_Group* section_tab = AddTab( "Section" );
    Fl_Group* section_group = AddSubGroup( section_tab, 5 );
    m_SectionLayout.SetGroupAndScreen( section_group, this );
    m_SectionLayout.SetChoiceButtonWidth( m_SectionLayout.GetButtonWidth() );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "HEX and ARC" );
    m_SectionLayout.AddIndexSelector( m_SectionStationSelector );
    m_SectionLayout.AddSlider( m_ZUpperSlider, "ZUPPER", 0.1, "%7.3f" );
    // Each in Rel, a ratio to the upper surface's, in its first input, or in Abs in its second.
    m_ZLowerSlider.Init( m_SectionLayout, this, "ZLOWER", 0.1 );
    m_SectionLayout.AddSlider( m_LMaxUSlider, "LMAXU", 1.0, "%7.3f" );
    m_LMaxLSlider.Init( m_SectionLayout, this, "LMAXL", 1.0 );
    m_SectionLayout.AddSlider( m_LFlatUSlider, "LFLATU", 1.0, "%7.3f" );
    m_LFlatLSlider.Init( m_SectionLayout, this, "LFLATL", 1.0 );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "NACA" );
    m_SectionLayout.AddChoice( m_NacaSerChoice, "NACASER" );
    m_NacaSerChoice.AddItem( "4 Digit" );
    m_NacaSerChoice.AddItem( "Modified 4 Digit" );
    m_NacaSerChoice.AddItem( "5 Digit" );
    m_NacaSerChoice.AddItem( "Modified 5 Digit" );
    m_NacaSerChoice.AddItem( "1 Series" );
    m_NacaSerChoice.AddItem( "6 Series" );
    m_NacaSerChoice.AddItem( "Supersonic Diamond" );
    m_NacaSerChoice.AddItem( "Supersonic Arc" );
    m_NacaSerChoice.AddItem( "Supersonic Hex" );
    m_NacaSerChoice.UpdateItems();
    m_SectionLayout.AddSlider( m_NacaDesSlider, "NACADES", 100.0, "%7.3f" );
    m_SectionLayout.AddSlider( m_NacaModSlider, "NACAMOD", 100.0, "%7.3f" );
    m_SectionLayout.AddButton( m_NacaSeaToggle, "NACASEA" );
    m_SectionLayout.AddSlider( m_NacaMeanSlider, "NACAMEAN", 1.0, "%7.3f" );
    m_SectionLayout.AddSlider( m_NacaXMaxSlider, "NACAXMAX", 100.0, "%7.3f" );
    m_SectionLayout.AddSlider( m_NacaFlatSlider, "NACAFLAT", 100.0, "%7.3f" );

    m_SectionLayout.AddYGap();
    m_SectionLayout.AddDividerBox( "USER" );
    m_SectionLayout.AddButton( m_ReadUserButton, "Read Airfoil..." );
}

//==== Show Screen ====//
void MdcFinScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

//==== Update Screen ====//
bool MdcFinScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != MDC_FIN_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    // The rows along the span are set by segment.
    m_NumUSlider.Deactivate();

    MdcFinGeom* fin = dynamic_cast< MdcFinGeom* >( geom_ptr );
    assert( fin );

    int sectyp = fin->m_SecTyp();
    int station = fin->m_Station();
    int panel = fin->m_Panel();
    int i = station - 1;

    //==== Fin Set ====//
    m_PresetChoice.Update( fin->m_Preset.GetID() );
    m_SecTypChoice.Update( fin->m_SecTyp.GetID() );
    m_NPanelSlider.Update( fin->m_NPanel.GetID() );
    m_RBodySlider.Update( fin->m_RBody.GetID() );

    // A root off the body, at SSPAN_1, is not buried.
    m_BuryRootToggle.Update( fin->m_BuryRoot.GetID() );
    m_BuryRootTypeChoice.Update( fin->m_BuryRootType.GetID() );
    if ( fin->m_SSpan[0]->Get() > 1.0e-12 )
    {
        m_BuryRootToggle.Deactivate();
    }
    if ( fin->m_SSpan[0]->Get() > 1.0e-12 || !fin->m_BuryRoot() )
    {
        m_BuryRootTypeChoice.Deactivate();
    }

    m_PhiFFlagToggle.Update( fin->m_PhiFFlag.GetID() );
    m_PanelSelector.Update( fin->m_Panel.GetID() );
    m_PhiFSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_PhiF, panel - 1 ) );
    m_GamSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_Gam, panel - 1 ) );
    if ( !fin->m_PhiFFlag() )
    {
        m_PhiFSlider.Deactivate();
    }

    //==== Planform ====//
    m_NSpanSlider.Update( fin->m_NSpan.GetID() );
    m_StationSelector.Update( fin->m_Station.GetID() );
    m_SSpanSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_SSpan, i ) );
    m_ChordSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_Chord, i ) );
    // The root's XLE is the XForm tab's X location.
    if ( i == 0 )
    {
        m_XLESlider.Update( fin->m_XRelLoc.GetID() );
    }
    else
    {
        m_XLESlider.Update( XSecSurfGeom::ParmIDAt( fin->m_XLE, i - 1 ) );
    }
    m_LERSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_LER, i ) );
    m_LERTypeChoice.Update( fin->m_LERType.GetID() );
    m_CFOCSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_CFOC, i ) );

    // The last station starts no segment, so it has no segment inputs.
    m_SweepSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_Sweep, i ) );
    m_StaSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_Sta, i ) );
    m_SectTessUSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_SectTessU, i ) );
    m_XLEFlagToggle.Update( XSecSurfGeom::ParmIDAt( fin->m_XLEFlag, i ) );

    // SWEEP where the segment gives XLE, and XLE where the segment before it gives it, shape nothing.
    if ( i < ( int )fin->m_XLEFlag.size() && fin->m_XLEFlag[i]->Get() )
    {
        m_SweepSlider.Deactivate();
    }
    if ( i > 0 && i - 1 < ( int )fin->m_XLEFlag.size() && !fin->m_XLEFlag[i - 1]->Get() )
    {
        m_XLESlider.Deactivate();
    }

    //==== Section ====//
    m_SectionStationSelector.Update( fin->m_Station.GetID() );
    m_ZUpperSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_ZUpper, i ) );
    m_ZLowerSlider.Update( fin->m_ZLowerRelFlag.GetID(), XSecSurfGeom::ParmIDAt( fin->m_ZLowerRatio, i ),
                           XSecSurfGeom::ParmIDAt( fin->m_ZLower, i ), fin->m_ZLowerRelFlag() );
    m_LMaxUSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_LMaxU, i ) );
    m_LMaxLSlider.Update( fin->m_LMaxLRelFlag.GetID(), XSecSurfGeom::ParmIDAt( fin->m_LMaxLRatio, i ),
                          XSecSurfGeom::ParmIDAt( fin->m_LMaxL, i ), fin->m_LMaxLRelFlag() );
    m_LFlatUSlider.Update( XSecSurfGeom::ParmIDAt( fin->m_LFlatU, i ) );
    m_LFlatLSlider.Update( fin->m_LFlatLRelFlag.GetID(), XSecSurfGeom::ParmIDAt( fin->m_LFlatLRatio, i ),
                           XSecSurfGeom::ParmIDAt( fin->m_LFlatL, i ), fin->m_LFlatLRelFlag() );

    m_NacaSerChoice.Update( fin->m_NacaSer.GetID() );
    m_NacaDesSlider.Update( fin->m_NacaDes.GetID() );
    m_NacaModSlider.Update( fin->m_NacaMod.GetID() );
    m_NacaSeaToggle.Update( fin->m_NacaSea.GetID() );
    m_NacaMeanSlider.Update( fin->m_NacaMean.GetID() );
    m_NacaXMaxSlider.Update( fin->m_NacaXMax.GetID() );
    m_NacaFlatSlider.Update( fin->m_NacaFlat.GetID() );
    m_ReadUserButton.Activate();

    // The section inputs of the other section types, a ZLOWER, LMAXL or LFLATL that is not its own, the lower
    // surface's and flat's inputs of an ARC, LER for a section given by its points, and the button that reads
    // the user section, unless it is the section type.
    bool section = sectyp == MdcFinGeom::HEX || sectyp == MdcFinGeom::ARC;
    if ( !section )
    {
        m_ZUpperSlider.Deactivate();
        m_ZLowerSlider.Deactivate();
        m_LMaxUSlider.Deactivate();
        m_LMaxLSlider.Deactivate();
        m_LFlatUSlider.Deactivate();
        m_LFlatLSlider.Deactivate();
        m_LERSlider.Deactivate();
        m_LERTypeChoice.Deactivate();
    }
    if ( sectyp == MdcFinGeom::ARC )
    {
        m_LMaxUSlider.Deactivate();
        m_LMaxLSlider.Deactivate();
        m_LFlatUSlider.Deactivate();
        m_LFlatLSlider.Deactivate();
    }

    if ( sectyp != MdcFinGeom::USER )
    {
        m_ReadUserButton.Deactivate();
    }

    int naca = fin->m_NacaSer();
    if ( sectyp != MdcFinGeom::NACA )
    {
        m_NacaSerChoice.Deactivate();
        m_NacaDesSlider.Deactivate();
    }
    if ( sectyp != MdcFinGeom::NACA || ( naca != MdcFinGeom::NACA4M && naca != MdcFinGeom::NACA5M ) )
    {
        m_NacaModSlider.Deactivate();
    }
    if ( sectyp != MdcFinGeom::NACA || naca != MdcFinGeom::NACA6 )
    {
        m_NacaSeaToggle.Deactivate();
        m_NacaMeanSlider.Deactivate();
    }
    if ( sectyp != MdcFinGeom::NACA || naca < MdcFinGeom::NACAS1 )
    {
        m_NacaXMaxSlider.Deactivate();
    }
    if ( sectyp != MdcFinGeom::NACA || naca != MdcFinGeom::NACAS3 )
    {
        m_NacaFlatSlider.Deactivate();
    }

    return true;
}

//==== Non Menu Callbacks ====//
void MdcFinScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

//==== Gui Device Callbacks ====//
void MdcFinScreen::GuiDeviceCallBack( GuiDevice* device )
{
    MdcFinGeom* preset_geom = dynamic_cast< MdcFinGeom* >( m_ScreenMgr->GetCurrGeom() );
    if ( preset_geom && device == &m_SetPresetButton )
    {
        preset_geom->SetPreset();
        preset_geom->Update();
    }

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( device == &m_ConvertToWingButton && geom_ptr && geom_ptr->GetType().m_Type == MDC_FIN_GEOM_TYPE )
    {
        int choice = fl_choice( "Convert MDC_FIN to Wing Geom.\nReplace the MDC_FIN (can not be un-done), or add a Wing and Clones beside it?",
                                "Cancel", "Replace", "Add" );
        if ( choice == 1 || choice == 2 )
        {
            bool replace_flag = ( choice == 1 );

            // Replacing deletes this Geom, so nothing may touch it afterwards.
            Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
            vector< string > wing_ids = veh->ConvertMdcFinToWing( geom_ptr->GetID(), replace_flag );
            if ( wing_ids.empty() )
            {
                fl_alert( "The MDC_FIN could not be converted." );
                return;
            }

            if ( replace_flag )
            {
                veh->SetActiveGeom( wing_ids[0] );
            }

            ManageGeomScreen* mgs = dynamic_cast< ManageGeomScreen* >( m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) );
            if ( mgs )
            {
                mgs->SetNeedsShowHideGeoms();
            }

            m_ScreenMgr->SetUpdateFlag( true );
            return;
        }
        return;
    }

    GeomScreen::GuiDeviceCallBack( device );

    if ( device != &m_ReadUserButton )
    {
        return;
    }

    MdcFinGeom* fin = dynamic_cast< MdcFinGeom* >( m_ScreenMgr->GetCurrGeom() );
    if ( !fin )
    {
        return;
    }

    //==== Read an Airfoil File Into the USER Section ====//
    FileAirfoil* af = fin->GetUserAirfoil();
    if ( !af )
    {
        return;
    }

    string newfile = m_ScreenMgr->FileChooser( "Airfoil File", "*.{af,dat}" );
    if ( newfile.empty() || !af->ReadFile( newfile ) )
    {
        return;
    }

    fin->ForceUpdate();
    m_ScreenMgr->SetUpdateFlag( true );
}
