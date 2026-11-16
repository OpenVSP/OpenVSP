//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HumanGeomScreen.h"
#include "ScreenMgr.h"
#include "HumanGeom.h"


//==== Constructor ====//
HumanGeomScreen::HumanGeomScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 510, 800, "Human", "Human.html", false )
{
    Fl_Group* design_tab = AddTab( "Anthropometric" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_AnthropoLayout.SetGroupAndScreen( design_group, this );

    m_AnthropoLayout.AddDividerBox( "Representation" );

    m_RepresentationChoice.AddItem( "MESH" );
    m_RepresentationChoice.AddItem( "SURFACE" );
    m_AnthropoLayout.AddChoice( m_RepresentationChoice, "Shape" );

    m_AnthropoLayout.AddYGap();
    m_AnthropoLayout.AddDividerBox( "Units" );

    m_LenUnitChoice.AddItem( "MM" );
    m_LenUnitChoice.AddItem( "CM" );
    m_LenUnitChoice.AddItem( "M" );
    m_LenUnitChoice.AddItem( "IN" );
    m_LenUnitChoice.AddItem( "FT" );
    m_LenUnitChoice.AddItem( "YD" );
    m_AnthropoLayout.AddChoice( m_LenUnitChoice, "Length Unit" );

    m_MassUnitChoice.AddItem( "G" );
    m_MassUnitChoice.AddItem( "KG" );
    m_MassUnitChoice.AddItem( "TONNE" );
    m_MassUnitChoice.AddItem( "LBM" );
    m_MassUnitChoice.AddItem( "SLUG" );
    m_MassUnitChoice.AddItem( "LBFSEC2IN" );
    m_AnthropoLayout.AddChoice( m_MassUnitChoice, "Mass Unit" );

    m_AnthropoLayout.AddYGap();
    m_AnthropoLayout.AddDividerBox( "Anthropometric" );

    m_GenderChoice.AddItem( "MALE" );
    m_GenderChoice.AddItem( "FEMALE" );
    m_AnthropoLayout.AddChoice( m_GenderChoice, "Gender" );
    m_AnthropoLayout.AddYGap();

    m_AnthropoLayout.AddSlider( m_StatureSlider, "Stature", 100, "%7.3f" );
    m_AnthropoLayout.AddSlider( m_StaturePctSlider, "% Stature", 1, "%7.3f" );
    m_AnthropoLayout.AddYGap();

    m_AnthropoLayout.AddSlider( m_BMISlider, "BMI", 10, "%7.3f" );
    m_AnthropoLayout.AddSlider( m_BMIPctSlider, "% BMI", 1, "%7.3f" );
    m_AnthropoLayout.AddSlider( m_MassSlider, "Mass", 100, "%7.3f" );

    m_AnthropoLayout.SetSameLineFlag( true );
    m_AnthropoLayout.AddOutput( m_VolumeOutput, "Volume", "%6.2f", 100 );
    m_AnthropoLayout.SetFitWidthFlag( false );
    m_AnthropoLayout.SetButtonWidth( 100 );
    m_AnthropoLayout.AddButton( m_AutoDensityToggle, "Auto Density" );
    m_AnthropoLayout.InitWidthHeightVals();
    m_AnthropoLayout.ForceNewLine();
    m_AnthropoLayout.SetSameLineFlag( false );
    m_AnthropoLayout.SetFitWidthFlag( true );

    m_AnthropoLayout.AddYGap();

    m_AnthropoLayout.AddSlider( m_AgeSlider, "Age", 10, "%7.3f" );
    m_AnthropoLayout.AddYGap();

    m_AnthropoLayout.AddSlider( m_SitFracSlider, "Sitting H / H", 1.0, "%7.3f" );

    m_AnthropoLayout.AddYGap();

#ifdef SHOW_SHOW_SKEL_BUTTON
    m_AnthropoLayout.AddButton( m_ShowSkelButton, "Show Skeleton" );
#endif

    Fl_Group* pose_tab = AddTab( "Pose" );
    Fl_Group* pose_group = AddSubGroup( pose_tab, 5 );

    m_PoseLayout.SetGroupAndScreen( pose_group, this );
    m_PoseLayout.AddDividerBox( "Pose" );

    m_PoseLayout.SetButtonWidth( m_PoseLayout.GetButtonWidth() + 25 );

    m_PresetPoseChoice.AddItem( "STANDING" );
    m_PresetPoseChoice.AddItem( "SITTING" );

    m_PoseLayout.SetChoiceButtonWidth( m_PoseLayout.GetButtonWidth() );

    m_PoseLayout.SetSameLineFlag( true );

    m_PoseLayout.AddChoice( m_PresetPoseChoice, "Preset", m_PoseLayout.GetButtonWidth() );

    m_PoseLayout.SetFitWidthFlag( false );
    m_PoseLayout.AddButton( m_SetPose, "Set" );

    m_PoseLayout.ForceNewLine();
    m_PoseLayout.SetSameLineFlag( false );
    m_PoseLayout.SetFitWidthFlag( true );

    m_PoseLayout.AddYGap();
    m_PoseLayout.AddDividerBox( "Method" );

    m_PoseMethodChoice.AddItem( "ARAP" );
    m_PoseMethodChoice.AddItem( "FAST" );
    m_PoseLayout.AddChoice( m_PoseMethodChoice, "Method" );

    // Paired sliders share a row, each half the width, with short labels.
    int half = m_PoseLayout.GetW() / 2;
    int oldBW = m_PoseLayout.GetButtonWidth();

    m_PoseLayout.SetSameLineFlag( true );
    m_PoseLayout.SetButtonWidth( 74 );
    m_PoseLayout.AddSlider( m_ARAPMaxIterSlider, "ARAP It.", 100, "%5.0f", half );
    m_PoseLayout.AddSlider( m_ARAPTolSlider, "ARAP Tol", 1, "%7.3f" );
    m_PoseLayout.ForceNewLine();
    m_PoseLayout.AddSlider( m_FASTIterSlider, "FAST It.", 10, "%5.0f", half );
    m_PoseLayout.ForceNewLine();

    m_PoseLayout.SetSameLineFlag( false );
    m_PoseLayout.SetButtonWidth( oldBW );

    m_PoseLayout.AddYGap();
    m_PoseLayout.AddDividerBox( "Torso" );
    m_PoseLayout.AddSlider( m_BackSlider, "Back", 100, "%7.3f" );
    m_PoseLayout.AddSlider( m_WaistSlider, "Waist", 100, "%7.3f" );
    m_PoseLayout.AddSlider( m_TorsoTwistSlider, "Twist", 100, "%7.3f" );
    m_PoseLayout.AddSlider( m_TorsoBendSlider, "Side Bend", 100, "%7.3f" );

    m_PoseLayout.AddYGap();
    m_PoseLayout.AddDividerBox( "Head" );
    m_PoseLayout.AddSlider( m_NodSlider, "Nod", 100, "%7.3f" );
    m_PoseLayout.AddSlider( m_RotateHeadSlider, "Rotate", 100, "%7.3f" );
    m_PoseLayout.AddSlider( m_HeadTiltSlider, "Tilt", 100, "%7.3f" );

    // The limbs: right and left side by side, each under its own divider.
    m_PoseLayout.AddYGap();
    // The divider is as tall as the R/L Sym toggle beside it.
    int oldDH = m_PoseLayout.GetDividerHeight();
    m_PoseLayout.SetDividerHeight( m_PoseLayout.GetStdHeight() );
    m_PoseLayout.SetSameLineFlag( true );
    m_PoseLayout.AddDividerBox( "Limbs", m_PoseLayout.GetButtonWidth() );
    m_PoseLayout.SetFitWidthFlag( false );
    m_PoseLayout.AddButton( m_RLSymButton, "R/L Sym" );
    m_PoseLayout.ForceNewLine();
    m_PoseLayout.SetFitWidthFlag( true );
    m_PoseLayout.SetSameLineFlag( false );
    m_PoseLayout.SetDividerHeight( oldDH );
    m_PoseLayout.AddYGap();

    // Only the right column is labeled, so its labels can be spelled out; the columns' sliders are
    // the same width.
    int limbbw = 112;
    int colrw = ( m_PoseLayout.GetW() - 4 + limbbw ) / 2;
    int collw = m_PoseLayout.GetW() - 4 - colrw;
    int armh = m_PoseLayout.GetStdHeight() * 6 + m_PoseLayout.GetDividerHeight();
    int legh = m_PoseLayout.GetStdHeight() * 5 + m_PoseLayout.GetDividerHeight();

    m_PoseLayout.AddSubGroupLayout( m_ArmRtLayout, colrw, armh );
    m_PoseLayout.AddX( colrw + 4 );
    m_PoseLayout.AddSubGroupLayout( m_ArmLtLayout, collw, armh );
    m_PoseLayout.ForceNewLine( 0 );
    m_PoseLayout.AddY( armh );
    m_PoseLayout.AddYGap();

    m_PoseLayout.AddSubGroupLayout( m_LegRtLayout, colrw, legh );
    m_PoseLayout.AddX( colrw + 4 );
    m_PoseLayout.AddSubGroupLayout( m_LegLtLayout, collw, legh );
    m_PoseLayout.ForceNewLine( 0 );
    m_PoseLayout.AddY( legh );

    AddArmSliders( m_ArmRtLayout, "Right Arm", limbbw, m_ShoulderABADRtSlider, m_ShoulderFERtSlider, m_ShoulderIERtSlider,
                   m_ElbowRtSlider, m_ForearmRtSlider, m_WristRtSlider );
    AddArmSliders( m_ArmLtLayout, "Left Arm", 0, m_ShoulderABADLtSlider, m_ShoulderFELtSlider, m_ShoulderIELtSlider,
                   m_ElbowLtSlider, m_ForearmLtSlider, m_WristLtSlider );
    AddLegSliders( m_LegRtLayout, "Right Leg", limbbw, m_HipABADRtSlider, m_HipFERtSlider, m_HipIERtSlider, m_KneeRtSlider,
                   m_AnkleRtSlider );
    AddLegSliders( m_LegLtLayout, "Left Leg", 0, m_HipABADLtSlider, m_HipFELtSlider, m_HipIELtSlider, m_KneeLtSlider,
                   m_AnkleLtSlider );
}

// A limb slider's label, or none in a column without labels (bw 0).
static const char* LimbLabel( int bw, const char* text )
{
    if ( bw > 0 )
    {
        return text;
    }
    return "";
}

// One arm's column of joint sliders, labeled if bw is not 0.
void HumanGeomScreen::AddArmSliders( GroupLayout &layout, const string &title, int bw, SliderAdjRangeInput &shabad,
                                     SliderAdjRangeInput &shfe, SliderAdjRangeInput &shie, SliderAdjRangeInput &elbow,
                                     SliderAdjRangeInput &forearm, SliderAdjRangeInput &wrist )
{
    layout.SetButtonWidth( bw );
    layout.AddDividerBox( title );
    layout.AddSlider( shabad, LimbLabel( bw, "Shoulder AB/AD" ), 100, "%7.3f" );
    layout.AddSlider( shfe, LimbLabel( bw, "Shoulder FE" ), 100, "%7.3f" );
    layout.AddSlider( shie, LimbLabel( bw, "Shoulder IE" ), 100, "%7.3f" );
    layout.AddSlider( elbow, LimbLabel( bw, "Elbow" ), 100, "%7.3f" );
    layout.AddSlider( forearm, LimbLabel( bw, "Forearm" ), 100, "%7.3f" );
    layout.AddSlider( wrist, LimbLabel( bw, "Wrist FE" ), 100, "%7.3f" );
}

// One leg's column of joint sliders, labeled if bw is not 0.
void HumanGeomScreen::AddLegSliders( GroupLayout &layout, const string &title, int bw, SliderAdjRangeInput &hipabad,
                                     SliderAdjRangeInput &hipfe, SliderAdjRangeInput &hipie, SliderAdjRangeInput &knee,
                                     SliderAdjRangeInput &ankle )
{
    layout.SetButtonWidth( bw );
    layout.AddDividerBox( title );
    layout.AddSlider( hipabad, LimbLabel( bw, "Hip AB/AD" ), 100, "%7.3f" );
    layout.AddSlider( hipfe, LimbLabel( bw, "Hip FE" ), 100, "%7.3f" );
    layout.AddSlider( hipie, LimbLabel( bw, "Hip IE" ), 100, "%7.3f" );
    layout.AddSlider( knee, LimbLabel( bw, "Knee" ), 100, "%7.3f" );
    layout.AddSlider( ankle, LimbLabel( bw, "Ankle" ), 100, "%7.3f" );
}


//==== Show Ellipsoid Screen ====//
void HumanGeomScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Ellipsoid Screen ====//
bool HumanGeomScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != HUMAN_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== Update Human Specific Parms ====//
    HumanGeom* human_ptr = dynamic_cast< HumanGeom* >( geom_ptr );
    assert( human_ptr );

    m_PresetPoseChoice.Update( human_ptr->m_PresetPose.GetID() );

    m_PoseMethodChoice.Update( human_ptr->m_PoseMethod.GetID() );
    m_ARAPMaxIterSlider.Update( human_ptr->m_ARAPMaxIter.GetID() );
    m_ARAPTolSlider.Update( human_ptr->m_ARAPTol.GetID() );
    m_FASTIterSlider.Update( human_ptr->m_FASTIter.GetID() );
    // Only the chosen method's settings apply.
    if ( human_ptr->m_PoseMethod() == vsp::HUMAN_POSE_FAST )
    {
        m_ARAPMaxIterSlider.Deactivate();
        m_ARAPTolSlider.Deactivate();
    }
    else
    {
        m_FASTIterSlider.Deactivate();
    }

    m_RepresentationChoice.Update( human_ptr->m_Representation.GetID() );

    m_LenUnitChoice.Update( human_ptr->m_LenUnit.GetID() );
    m_MassUnitChoice.Update( human_ptr->m_MassUnit.GetID() );

    m_GenderChoice.Update( human_ptr->m_GenderFlag.GetID() );

    m_StatureSlider.SetMinBound( human_ptr->m_Stature.GetLowerLimit() );
    m_StatureSlider.SetMaxBound( human_ptr->m_Stature.GetUpperLimit() );

    m_StatureSlider.Update( human_ptr->m_Stature.GetID() );
    m_StaturePctSlider.Update( human_ptr->m_Stature_pct.GetID() );

    m_BMISlider.Update( human_ptr->m_BMI.GetID() );
    m_BMIPctSlider.Update( human_ptr->m_BMI_pct.GetID() );
    m_MassSlider.Update( human_ptr->m_Mass.GetID() );

    m_AutoDensityToggle.Update( human_ptr->m_AutoDensity.GetID() );
    m_VolumeOutput.Update( human_ptr->m_Volume.GetID() );

    m_AgeSlider.Update( human_ptr->m_Age.GetID() );
    m_SitFracSlider.Update( human_ptr->m_SitFrac.GetID() );

#ifdef SHOW_SHOW_SKEL_BUTTON
    m_ShowSkelButton.Update( human_ptr->m_ShowSkelFlag.GetID() );
#endif

    m_RLSymButton.Update( human_ptr->m_RLSymFlag.GetID() );

    m_BackSlider.Update( human_ptr->m_Back.GetID() );
    m_WaistSlider.Update( human_ptr->m_Waist.GetID() );
    m_TorsoTwistSlider.Update( human_ptr->m_TorsoTwist.GetID() );
    m_TorsoBendSlider.Update( human_ptr->m_TorsoBend.GetID() );

    m_NodSlider.Update( human_ptr->m_Nod.GetID() );
    m_RotateHeadSlider.Update( human_ptr->m_RotateHead.GetID() );
    m_HeadTiltSlider.Update( human_ptr->m_HeadTilt.GetID() );

    m_WristRtSlider.Update( human_ptr->m_WristRt.GetID() );
    m_ForearmRtSlider.Update( human_ptr->m_ForearmRt.GetID() );
    m_ElbowRtSlider.Update( human_ptr->m_ElbowRt.GetID() );
    m_ShoulderABADRtSlider.Update( human_ptr->m_ShoulderABADRt.GetID() );
    m_ShoulderFERtSlider.Update( human_ptr->m_ShoulderFERt.GetID() );
    m_ShoulderIERtSlider.Update( human_ptr->m_ShoulderIERt.GetID() );

    m_HipABADRtSlider.Update( human_ptr->m_HipABADRt.GetID() );
    m_HipFERtSlider.Update( human_ptr->m_HipFERt.GetID() );
    m_HipIERtSlider.Update( human_ptr->m_HipIERt.GetID() );
    m_KneeRtSlider.Update( human_ptr->m_KneeRt.GetID() );
    m_AnkleRtSlider.Update( human_ptr->m_AnkleRt.GetID() );

    m_WristLtSlider.Update( human_ptr->m_WristLt.GetID() );
    m_ForearmLtSlider.Update( human_ptr->m_ForearmLt.GetID() );
    m_ElbowLtSlider.Update( human_ptr->m_ElbowLt.GetID() );
    m_ShoulderABADLtSlider.Update( human_ptr->m_ShoulderABADLt.GetID() );
    m_ShoulderFELtSlider.Update( human_ptr->m_ShoulderFELt.GetID() );
    m_ShoulderIELtSlider.Update( human_ptr->m_ShoulderIELt.GetID() );

    m_HipABADLtSlider.Update( human_ptr->m_HipABADLt.GetID() );
    m_HipFELtSlider.Update( human_ptr->m_HipFELt.GetID() );
    m_HipIELtSlider.Update( human_ptr->m_HipIELt.GetID() );
    m_KneeLtSlider.Update( human_ptr->m_KneeLt.GetID() );
    m_AnkleLtSlider.Update( human_ptr->m_AnkleLt.GetID() );




    if( human_ptr->m_RLSymFlag() )
    {
        m_WristLtSlider.Deactivate();
        m_ForearmLtSlider.Deactivate();
        m_ElbowLtSlider.Deactivate();
        m_ShoulderABADLtSlider.Deactivate();
        m_ShoulderFELtSlider.Deactivate();
        m_ShoulderIELtSlider.Deactivate();

        m_HipABADLtSlider.Deactivate();
        m_HipFELtSlider.Deactivate();
        m_HipIELtSlider.Deactivate();
        m_KneeLtSlider.Deactivate();
        m_AnkleLtSlider.Deactivate();
    }

    return true;
}


//==== Non Menu Callbacks ====//
void HumanGeomScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void HumanGeomScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Human Ptr ====//
    Geom *geom_ptr = m_ScreenMgr->GetCurrGeom();
    if (!geom_ptr || geom_ptr->GetType().m_Type != HUMAN_GEOM_TYPE) {
        return;
    }

    HumanGeom *human_ptr = dynamic_cast< HumanGeom * >( geom_ptr );
    assert(human_ptr);

    if (gui_device == &m_SetPose )
    {
        human_ptr->SetPreset();
        human_ptr->Update();
    }

    GeomScreen::GuiDeviceCallBack( gui_device );
}


