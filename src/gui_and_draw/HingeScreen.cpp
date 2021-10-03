//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "HingeScreen.h"
#include "HingeGeom.h"
#include "ScreenMgr.h"


//==== Constructor ====//
HingeScreen::HingeScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 300, 525, "Hinge" )
{
    Fl_Group* orient_tab = AddTab( "Orient" );
    Fl_Group* orient_group = AddSubGroup( orient_tab, 5 );

    m_BaseOrientationLayout.SetGroupAndScreen( orient_group, this );

    m_BaseOrientationLayout.AddDividerBox( "Orient Base" );

    m_BaseOrientationLayout.AddButton( m_OrientRotToggle, "Rotations (Use XForm Tab)" );
    m_BaseOrientationLayout.AddButton( m_OrientVecToggle, "Vectors" );

    m_OrientTypeGroup.Init( this );
    m_OrientTypeGroup.AddButton( m_OrientRotToggle.GetFlButton() );
    m_OrientTypeGroup.AddButton( m_OrientVecToggle.GetFlButton() );

    m_BaseOrientationLayout.AddYGap();

    m_BaseOrientationLayout.AddDividerBox( "Vectors" );

    // Individual SubSurface Parameters
    m_PrimaryDirLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );
    m_PrimaryDirLayout.SetY( m_BaseOrientationLayout.GetY() );

    m_PrimaryDirLayout.SetButtonWidth( m_PrimaryDirLayout.GetW()/4 );

    m_PrimaryDirLayout.SetSameLineFlag( true );
    m_PrimaryDirLayout.SetFitWidthFlag( false );
    m_PrimaryDirLayout.AddLabel( "Axis", m_PrimaryDirLayout.GetButtonWidth() );
    m_PrimaryDirLayout.AddButton( m_PrimaryXToggle, "X" );
    m_PrimaryDirLayout.AddButton( m_PrimaryYToggle, "Y" );
    m_PrimaryDirLayout.AddButton( m_PrimaryZToggle, "Z" );
    m_PrimaryDirLayout.ForceNewLine();

    m_PrimaryDirGroup.Init( this );
    m_PrimaryDirGroup.AddButton( m_PrimaryXToggle.GetFlButton() );
    m_PrimaryDirGroup.AddButton( m_PrimaryYToggle.GetFlButton() );
    m_PrimaryDirGroup.AddButton( m_PrimaryZToggle.GetFlButton() );

    m_SecondaryDirLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );
    m_SecondaryDirLayout.SetY( m_PrimaryDirLayout.GetY() );

    m_SecondaryDirLayout.SetButtonWidth( m_SecondaryDirLayout.GetW()/4 );

    m_SecondaryDirLayout.SetSameLineFlag( true );
    m_SecondaryDirLayout.SetFitWidthFlag( false );
    m_SecondaryDirLayout.AddLabel( "Reference", m_SecondaryDirLayout.GetButtonWidth() );
    m_SecondaryDirLayout.AddButton( m_SecondaryXToggle, "X" );
    m_SecondaryDirLayout.AddButton( m_SecondaryYToggle, "Y" );
    m_SecondaryDirLayout.AddButton( m_SecondaryZToggle, "Z" );
    m_SecondaryDirLayout.ForceNewLine();

    m_SecondaryDirGroup.Init( this );
    m_SecondaryDirGroup.AddButton( m_SecondaryXToggle.GetFlButton() );
    m_SecondaryDirGroup.AddButton( m_SecondaryYToggle.GetFlButton() );
    m_SecondaryDirGroup.AddButton( m_SecondaryZToggle.GetFlButton() );

    m_VectorLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );

    m_VectorLayout.SetY( m_SecondaryDirLayout.GetY() );

    m_VectorLayout.AddYGap();
    m_VectorLayout.AddDividerBox( "Axis" );

    m_PrimTypeChoice.AddItem( "VECTOR3D" );
    m_PrimTypeChoice.AddItem( "POINT3D" );
    m_PrimTypeChoice.AddItem( "SURFPT" );
    m_PrimTypeChoice.AddItem( "UDIR" );
    m_PrimTypeChoice.AddItem( "WDIR" );
    m_PrimTypeChoice.AddItem( "NDIR" );
    m_VectorLayout.AddChoice( m_PrimTypeChoice, "Type" );

    m_VectorLayout.AddYGap();

    m_PrimVecLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );
    m_PrimVecLayout.SetY(  m_VectorLayout.GetY() );

    m_PrimVecLayout.SetFitWidthFlag( false );
    m_PrimVecLayout.SetSameLineFlag( true );
    m_PrimVecLayout.AddLabel( "Direction Vector:", 170 );
    m_PrimVecLayout.SetButtonWidth( m_PrimVecLayout.GetRemainX() / 2 );
    m_PrimVecLayout.AddButton( m_PrimVecRelToggle, "Rel" );
    m_PrimVecLayout.AddButton( m_PrimVecAbsToggle, "Abs" );
    m_PrimVecLayout.ForceNewLine();

    m_PrimVecAbsRelToggle.Init( this );
    m_PrimVecAbsRelToggle.AddButton( m_PrimVecAbsToggle.GetFlButton() );
    m_PrimVecAbsRelToggle.AddButton( m_PrimVecRelToggle.GetFlButton() );

    m_PrimVecLayout.SetFitWidthFlag( true );
    m_PrimVecLayout.SetSameLineFlag( false );
    m_PrimVecLayout.AddYGap();

    m_PrimVecLayout.SetButtonWidth( 50 );

    m_PrimVecLayout.AddSlider( m_PrimXVecSlider, "X", 10.0, "%7.3f" );
    m_PrimVecLayout.AddSlider( m_PrimYVecSlider, "Y", 10.0, "%7.3f" );
    m_PrimVecLayout.AddSlider( m_PrimZVecSlider, "Z", 10.0, "%7.3f" );

    m_VectorLayout.SetY( m_PrimVecLayout.GetY() );

    m_VectorLayout.AddYGap();


    m_PrimOffLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );
    m_PrimOffLayout.SetY(  m_VectorLayout.GetY() );


    m_PrimOffLayout.SetFitWidthFlag( false );
    m_PrimOffLayout.SetSameLineFlag( true );
    m_PrimOffLayout.AddLabel( "Offset:", 170 );
    m_PrimOffLayout.SetButtonWidth( m_PrimOffLayout.GetRemainX() / 2 );
    m_PrimOffLayout.AddButton( m_PrimOffRelToggle, "Rel" );
    m_PrimOffLayout.AddButton( m_PrimOffAbsToggle, "Abs" );
    m_PrimOffLayout.ForceNewLine();

    m_PrimOffAbsRelToggle.Init( this );
    m_PrimOffAbsRelToggle.AddButton( m_PrimOffAbsToggle.GetFlButton() );
    m_PrimOffAbsRelToggle.AddButton( m_PrimOffRelToggle.GetFlButton() );

    m_PrimOffLayout.SetFitWidthFlag( true );
    m_PrimOffLayout.SetSameLineFlag( false );
    m_PrimOffLayout.AddYGap();

    m_PrimOffLayout.SetButtonWidth( 50 );

    m_PrimOffLayout.AddSlider( m_PrimXOffSlider, "X", 10.0, "%7.3f" );
    m_PrimOffLayout.AddSlider( m_PrimYOffSlider, "Y", 10.0, "%7.3f" );
    m_PrimOffLayout.AddSlider( m_PrimZOffSlider, "Z", 10.0, "%7.3f" );

    m_VectorLayout.SetY( m_PrimOffLayout.GetY() );

    m_VectorLayout.AddYGap();

    m_PrimUWLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );
    m_PrimUWLayout.SetY(  m_VectorLayout.GetY() );

    m_PrimUWLayout.AddSlider( m_PrimUSlider, "U", 1, "%5.4f" );
    m_PrimUWLayout.AddSlider( m_PrimWSlider, "W", 1, "%5.4f" );

    m_VectorLayout.SetY( m_PrimUWLayout.GetY() );

    m_VectorLayout.AddYGap();

    m_SecVecLayout.SetGroupAndScreen( AddSubGroup( orient_tab, 5 ), this );
    m_SecVecLayout.SetY(  m_VectorLayout.GetY() );

    m_SecVecLayout.AddDividerBox( "Reference Vector" );

    m_SecVecLayout.SetFitWidthFlag( false );
    m_SecVecLayout.SetSameLineFlag( true );

    m_SecVecLayout.SetButtonWidth( m_SecVecLayout.GetRemainX() / 3 );
    m_SecVecLayout.AddLabel( "Reference Vec", m_SecVecLayout.GetButtonWidth() );
    m_SecVecLayout.AddButton( m_SecVecAbsToggle, "Rel" );
    m_SecVecLayout.AddButton( m_SecVecRelToggle, "Abs" );
    m_SecVecLayout.ForceNewLine();

    m_SecVecAbsRelToggle.Init( this );
    m_SecVecAbsRelToggle.AddButton( m_SecVecAbsToggle.GetFlButton() );
    m_SecVecAbsRelToggle.AddButton( m_SecVecRelToggle.GetFlButton() );

    m_SecVecLayout.AddButton( m_SecondaryVecXToggle, "X" );
    m_SecVecLayout.AddButton( m_SecondaryVecYToggle, "Y" );
    m_SecVecLayout.AddButton( m_SecondaryVecZToggle, "Z" );
    m_SecVecLayout.ForceNewLine();

    m_SecondaryVecDirGroup.Init( this );
    m_SecondaryVecDirGroup.AddButton( m_SecondaryVecXToggle.GetFlButton() );
    m_SecondaryVecDirGroup.AddButton( m_SecondaryVecYToggle.GetFlButton() );
    m_SecondaryVecDirGroup.AddButton( m_SecondaryVecZToggle.GetFlButton() );


    Fl_Group* motion_tab = AddTab( "Motion" );
    Fl_Group* motion_group = AddSubGroup( motion_tab, 5 );

    m_MotionLayout.SetGroupAndScreen( motion_group, this );

    int bw = 110;
    int tw = 15;
    int sw = 35;

    m_MotionLayout.SetSameLineFlag( false );
    m_MotionLayout.SetFitWidthFlag( true );

    m_MotionLayout.AddDividerBox( "DOF and Coordinates" );

    m_MotionLayout.AddYGap();

    m_MotionLayout.AddDividerBox( "Translate" );
    m_MotionLayout.SetSameLineFlag( true );

    m_MotionLayout.SetFitWidthFlag( false );
    m_MotionLayout.SetButtonWidth( tw );
    m_MotionLayout.AddButton( m_JointTransMinToggle, "" );
    m_MotionLayout.SetButtonWidth( sw );
    m_MotionLayout.AddButton( m_JointTransMinSetButton, "Set" );
    m_MotionLayout.SetFitWidthFlag( true );
    m_MotionLayout.SetButtonWidth( bw - tw - sw );
    m_MotionLayout.AddSlider( m_JointTransMinSlider, "Min", 10, "%6.2f" );
    m_MotionLayout.ForceNewLine();


    m_MotionLayout.SetFitWidthFlag( false );
    m_MotionLayout.SetButtonWidth( tw );
    m_MotionLayout.AddButton( m_JointTranslateToggle, "" );
    m_MotionLayout.SetButtonWidth( sw );
    m_MotionLayout.AddButton( m_JointTranslateRngButton, "Rng" );
    m_MotionLayout.SetFitWidthFlag( true );
    m_MotionLayout.SetButtonWidth( bw - tw - sw );
    m_MotionLayout.AddSlider( m_JointTranslateSlider, "Translate", 10, "%6.2f" );
    m_MotionLayout.ForceNewLine();

    m_MotionLayout.SetFitWidthFlag( false );
    m_MotionLayout.SetButtonWidth( tw );
    m_MotionLayout.AddButton( m_JointTransMaxToggle, "" );
    m_MotionLayout.SetButtonWidth( sw );
    m_MotionLayout.AddButton( m_JointTransMaxSetButton, "Set" );
    m_MotionLayout.SetFitWidthFlag( true );
    m_MotionLayout.SetButtonWidth( bw - tw - sw );
    m_MotionLayout.AddSlider( m_JointTransMaxSlider, "Max", 10, "%6.2f" );
    m_MotionLayout.ForceNewLine();


    m_MotionLayout.AddYGap();
    m_MotionLayout.SetSameLineFlag( false );
    m_MotionLayout.AddDividerBox( "Rotate" );
    m_MotionLayout.SetSameLineFlag( true );

    m_MotionLayout.SetFitWidthFlag( false );
    m_MotionLayout.SetButtonWidth( tw );
    m_MotionLayout.AddButton( m_JointRotMinToggle, "" );
    m_MotionLayout.SetButtonWidth( sw );
    m_MotionLayout.AddButton( m_JointRotMinSetButton, "Set" );
    m_MotionLayout.SetFitWidthFlag( true );
    m_MotionLayout.SetButtonWidth( bw - tw - sw );
    m_MotionLayout.AddSlider( m_JointRotMinSlider, "Min", 100, "%6.2f" );
    m_MotionLayout.ForceNewLine();


    m_MotionLayout.SetFitWidthFlag( false );
    m_MotionLayout.SetButtonWidth( tw );
    m_MotionLayout.AddButton( m_JointRotateToggle, "" );
    m_MotionLayout.SetButtonWidth( sw );
    m_MotionLayout.AddButton( m_JointRotateRngButton, "Rng" );
    m_MotionLayout.SetFitWidthFlag( true );
    m_MotionLayout.SetButtonWidth( bw - tw - sw );
    m_MotionLayout.AddSlider( m_JointRotateSlider, "Rotate", 100, "%6.2f" );
    m_MotionLayout.ForceNewLine();

    m_MotionLayout.SetFitWidthFlag( false );
    m_MotionLayout.SetButtonWidth( tw );
    m_MotionLayout.AddButton( m_JointRotMaxToggle, "" );
    m_MotionLayout.SetButtonWidth( sw );
    m_MotionLayout.AddButton( m_JointRotMaxSetButton, "Set" );
    m_MotionLayout.SetFitWidthFlag( true );
    m_MotionLayout.SetButtonWidth( bw - tw - sw );
    m_MotionLayout.AddSlider( m_JointRotMaxSlider, "Max", 100, "%6.2f" );
    m_MotionLayout.ForceNewLine();

    RemoveTab( GetTab( m_SubSurfTab_ind ) );

}


//==== Show Blank Screen ====//
void HingeScreen::Show()
{
    if ( Update() )
    {
        m_FLTK_Window->show();
    }
}

bool HingeScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != HINGE_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    //==== Update Blank Specific Parms ====//
    HingeGeom* hinge_geom_ptr = dynamic_cast< HingeGeom* >( geom_ptr );
    assert( hinge_geom_ptr );

    if ( hinge_geom_ptr->m_OrientType.Get() == HingeGeom::ORIENT_VEC )
    {
        m_RotActive = false;
    }
    else
    {
        m_RotActive = true;
    }

    GeomScreen::Update();

    m_OrientTypeGroup.Update( hinge_geom_ptr->m_OrientType.GetID() );

    m_PrimaryDirGroup.Update( hinge_geom_ptr->m_PrimaryDir.GetID() );
    m_SecondaryDirGroup.Update( hinge_geom_ptr->m_SecondaryDir.GetID() );

    m_SecondaryVecDirGroup.Update( hinge_geom_ptr->m_SecondaryVecDir.GetID() );
    m_SecVecAbsRelToggle.Update( hinge_geom_ptr->m_SecVecAbsRelFlag.GetID() );

    m_SecondaryXToggle.Activate();
    m_SecondaryYToggle.Activate();
    m_SecondaryZToggle.Activate();

    if ( hinge_geom_ptr->m_OrientType.Get() == HingeGeom::ORIENT_VEC )
    {

        if ( hinge_geom_ptr->m_PrimaryDir.Get() == vsp::X_DIR )
        {
            m_SecondaryXToggle.Deactivate();
        }
        if ( hinge_geom_ptr->m_PrimaryDir.Get() == vsp::Y_DIR )
        {
            m_SecondaryYToggle.Deactivate();
        }
        if ( hinge_geom_ptr->m_PrimaryDir.Get() == vsp::Z_DIR )
        {
            m_SecondaryZToggle.Deactivate();
        }

        m_PrimaryXToggle.Activate();
        m_PrimaryYToggle.Activate();
        m_PrimaryZToggle.Activate();
        if ( hinge_geom_ptr->m_SecondaryDir.Get() == vsp::X_DIR )
        {
            m_PrimaryXToggle.Deactivate();
        }
        if ( hinge_geom_ptr->m_SecondaryDir.Get() == vsp::Y_DIR )
        {
            m_PrimaryYToggle.Deactivate();
        }
        if ( hinge_geom_ptr->m_SecondaryDir.Get() == vsp::Z_DIR )
        {
            m_PrimaryZToggle.Deactivate();
        }
    }

    m_PrimVecAbsRelToggle.Update( hinge_geom_ptr->m_PrimVecAbsRelFlag.GetID() );
    if ( hinge_geom_ptr->m_PrimVecAbsRelFlag() ==  vsp::REL )
    {
        hinge_geom_ptr->m_PrimXVecRel.Activate();
        hinge_geom_ptr->m_PrimYVecRel.Activate();
        hinge_geom_ptr->m_PrimZVecRel.Activate();
        hinge_geom_ptr->m_PrimXVec.Deactivate();
        hinge_geom_ptr->m_PrimYVec.Deactivate();
        hinge_geom_ptr->m_PrimZVec.Deactivate();
        m_PrimXVecSlider.Update( 1, hinge_geom_ptr->m_PrimXVecRel.GetID(), hinge_geom_ptr->m_PrimXVec.GetID() );
        m_PrimYVecSlider.Update( 1, hinge_geom_ptr->m_PrimYVecRel.GetID(), hinge_geom_ptr->m_PrimYVec.GetID() );
        m_PrimZVecSlider.Update( 1, hinge_geom_ptr->m_PrimZVecRel.GetID(), hinge_geom_ptr->m_PrimZVec.GetID() );
    }
    else
    {
        hinge_geom_ptr->m_PrimXVecRel.Deactivate();
        hinge_geom_ptr->m_PrimYVecRel.Deactivate();
        hinge_geom_ptr->m_PrimZVecRel.Deactivate();
        hinge_geom_ptr->m_PrimXVec.Activate();
        hinge_geom_ptr->m_PrimYVec.Activate();
        hinge_geom_ptr->m_PrimZVec.Activate();
        m_PrimXVecSlider.Update( 2, hinge_geom_ptr->m_PrimXVecRel.GetID(), hinge_geom_ptr->m_PrimXVec.GetID() );
        m_PrimYVecSlider.Update( 2, hinge_geom_ptr->m_PrimYVecRel.GetID(), hinge_geom_ptr->m_PrimYVec.GetID() );
        m_PrimZVecSlider.Update( 2, hinge_geom_ptr->m_PrimZVecRel.GetID(), hinge_geom_ptr->m_PrimZVec.GetID() );
    }

    m_PrimOffAbsRelToggle.Update( hinge_geom_ptr->m_PrimOffAbsRelFlag.GetID() );
    if ( hinge_geom_ptr->m_PrimOffAbsRelFlag() ==  vsp::REL )
    {
        hinge_geom_ptr->m_PrimXOffRel.Activate();
        hinge_geom_ptr->m_PrimYOffRel.Activate();
        hinge_geom_ptr->m_PrimZOffRel.Activate();
        hinge_geom_ptr->m_PrimXOff.Deactivate();
        hinge_geom_ptr->m_PrimYOff.Deactivate();
        hinge_geom_ptr->m_PrimZOff.Deactivate();
        m_PrimXOffSlider.Update( 1, hinge_geom_ptr->m_PrimXOffRel.GetID(), hinge_geom_ptr->m_PrimXOff.GetID() );
        m_PrimYOffSlider.Update( 1, hinge_geom_ptr->m_PrimYOffRel.GetID(), hinge_geom_ptr->m_PrimYOff.GetID() );
        m_PrimZOffSlider.Update( 1, hinge_geom_ptr->m_PrimZOffRel.GetID(), hinge_geom_ptr->m_PrimZOff.GetID() );
    }
    else
    {
        hinge_geom_ptr->m_PrimXOffRel.Deactivate();
        hinge_geom_ptr->m_PrimYOffRel.Deactivate();
        hinge_geom_ptr->m_PrimZOffRel.Deactivate();
        hinge_geom_ptr->m_PrimXOff.Activate();
        hinge_geom_ptr->m_PrimYOff.Activate();
        hinge_geom_ptr->m_PrimZOff.Activate();
        m_PrimXOffSlider.Update( 2, hinge_geom_ptr->m_PrimXOffRel.GetID(), hinge_geom_ptr->m_PrimXOff.GetID() );
        m_PrimYOffSlider.Update( 2, hinge_geom_ptr->m_PrimYOffRel.GetID(), hinge_geom_ptr->m_PrimYOff.GetID() );
        m_PrimZOffSlider.Update( 2, hinge_geom_ptr->m_PrimZOffRel.GetID(), hinge_geom_ptr->m_PrimZOff.GetID() );
    }

    m_PrimUSlider.Update( hinge_geom_ptr->m_PrimULoc.GetID() );
    m_PrimWSlider.Update( hinge_geom_ptr->m_PrimWLoc.GetID() );

    m_PrimTypeChoice.Update( hinge_geom_ptr->m_PrimaryType.GetID() );

    if ( hinge_geom_ptr->m_OrientType.Get() == HingeGeom::ORIENT_VEC )
    {
        m_VectorLayout.GetGroup()->activate();
        m_SecondaryDirLayout.GetGroup()->activate();

        if ( hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::VECTOR3D )
        {
            m_PrimVecLayout.GetGroup()->activate();
            m_SecVecLayout.GetGroup()->activate();
        }
        else
        {
            m_PrimVecLayout.GetGroup()->deactivate();
            m_SecVecLayout.GetGroup()->deactivate();
        }

        if ( hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::POINT3D ||
             hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::SURFPT )
        {
            m_PrimOffLayout.GetGroup()->activate();
        }
        else
        {
            m_PrimOffLayout.GetGroup()->deactivate();
        }

        if ( hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::SURFPT ||
             hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::UDIR ||
             hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::WDIR ||
             hinge_geom_ptr->m_PrimaryType.Get() == HingeGeom::NDIR )
        {
            m_PrimUWLayout.GetGroup()->activate();
        }
        else
        {
            m_PrimUWLayout.GetGroup()->deactivate();
        }

    }
    else
    {
        m_VectorLayout.GetGroup()->deactivate();
        m_SecondaryDirLayout.GetGroup()->deactivate();
        m_PrimVecLayout.GetGroup()->deactivate();
        m_SecVecLayout.GetGroup()->deactivate();
        m_PrimOffLayout.GetGroup()->deactivate();
        m_PrimUWLayout.GetGroup()->deactivate();
    }

    m_JointTranslateToggle.Update( hinge_geom_ptr->m_JointTranslateFlag.GetID() );
    m_JointTranslateSlider.Update( hinge_geom_ptr->m_JointTranslate.GetID() );
    m_JointTransMinToggle.Update( hinge_geom_ptr->m_JointTransMinFlag.GetID() );
    m_JointTransMinSlider.Update( hinge_geom_ptr->m_JointTransMin.GetID() );
    m_JointTransMaxToggle.Update( hinge_geom_ptr->m_JointTransMaxFlag.GetID() );
    m_JointTransMaxSlider.Update( hinge_geom_ptr->m_JointTransMax.GetID() );
    m_JointRotateToggle.Update( hinge_geom_ptr->m_JointRotateFlag.GetID() );
    m_JointRotateSlider.Update( hinge_geom_ptr->m_JointRotate.GetID() );
    m_JointRotMinToggle.Update( hinge_geom_ptr->m_JointRotMinFlag.GetID() );
    m_JointRotMinSlider.Update( hinge_geom_ptr->m_JointRotMin.GetID() );
    m_JointRotMaxToggle.Update( hinge_geom_ptr->m_JointRotMaxFlag.GetID() );
    m_JointRotMaxSlider.Update( hinge_geom_ptr->m_JointRotMax.GetID() );

    m_JointTranslateSlider.Deactivate();
    m_JointTranslateRngButton.Deactivate();
    m_JointTransMinSlider.Deactivate();
    m_JointTransMaxSlider.Deactivate();
    m_JointTransMinToggle.Deactivate();
    m_JointTransMaxToggle.Deactivate();
    m_JointTransMinSetButton.Deactivate();
    m_JointTransMaxSetButton.Deactivate();
    if ( hinge_geom_ptr->m_JointTranslateFlag.Get() )
    {
        m_JointTranslateSlider.Activate();
        m_JointTransMinToggle.Activate();
        m_JointTransMaxToggle.Activate();

        if ( hinge_geom_ptr->m_JointTransMinFlag.Get() || hinge_geom_ptr->m_JointTransMaxFlag.Get() )
        {
            m_JointTranslateRngButton.Activate();
        }

        if ( hinge_geom_ptr->m_JointTransMinFlag.Get() )
        {
            m_JointTransMinSlider.Activate();
            m_JointTransMinSetButton.Activate();
        }

        if ( hinge_geom_ptr->m_JointTransMaxFlag.Get() )
        {
            m_JointTransMaxSlider.Activate();
            m_JointTransMaxSetButton.Activate();
        }
    }



    m_JointRotateSlider.Deactivate();
    m_JointRotateRngButton.Deactivate();
    m_JointRotMinSlider.Deactivate();
    m_JointRotMaxSlider.Deactivate();
    m_JointRotMinToggle.Deactivate();
    m_JointRotMaxToggle.Deactivate();
    m_JointRotMinSetButton.Deactivate();
    m_JointRotMaxSetButton.Deactivate();
    if ( hinge_geom_ptr->m_JointRotateFlag.Get() )
    {
        m_JointRotateSlider.Activate();
        m_JointRotMinToggle.Activate();
        m_JointRotMaxToggle.Activate();

        if ( hinge_geom_ptr->m_JointRotMinFlag.Get() || hinge_geom_ptr->m_JointRotMaxFlag.Get() )
        {
            m_JointRotateRngButton.Activate();
        }

        if ( hinge_geom_ptr->m_JointRotMinFlag.Get() )
        {
            m_JointRotMinSlider.Activate();
            m_JointRotMinSetButton.Activate();
        }

        if ( hinge_geom_ptr->m_JointRotMaxFlag.Get() )
        {
            m_JointRotMaxSlider.Activate();
            m_JointRotMaxSetButton.Activate();
        }
    }

    return true;
}

//==== Non Menu Callbacks ====//
void HingeScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void HingeScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != HINGE_GEOM_TYPE )
    {
        return;
    }

    HingeGeom* hinge_geom_ptr = dynamic_cast< HingeGeom* >( geom_ptr );
    assert( hinge_geom_ptr );

    if ( device == &m_JointTransMinSetButton )
    {
        hinge_geom_ptr->m_JointTransMin = hinge_geom_ptr->m_JointTranslate();
    }
    else if ( device == &m_JointTransMaxSetButton )
    {
        hinge_geom_ptr->m_JointTransMax = hinge_geom_ptr->m_JointTranslate();
    }
    else if ( device == &m_JointTranslateRngButton )
    {
        if ( hinge_geom_ptr->m_JointTransMinFlag.Get() )
        {
            m_JointTranslateSlider.SetMinBound( hinge_geom_ptr->m_JointTransMin() );
        }
        if ( hinge_geom_ptr->m_JointTransMaxFlag.Get() )
        {
            m_JointTranslateSlider.SetMaxBound( hinge_geom_ptr->m_JointTransMax() );
        }
    }
    else if ( device == &m_JointRotMinSetButton )
    {
        hinge_geom_ptr->m_JointRotMin = hinge_geom_ptr->m_JointRotate();
    }
    else if ( device == &m_JointRotMaxSetButton )
    {
        hinge_geom_ptr->m_JointRotMax = hinge_geom_ptr->m_JointRotate();
    }
    else if ( device == &m_JointRotateRngButton )
    {
        if ( hinge_geom_ptr->m_JointRotMinFlag.Get() )
        {
            m_JointRotateSlider.SetMinBound( hinge_geom_ptr->m_JointRotMin() );
        }
        if ( hinge_geom_ptr->m_JointRotMaxFlag.Get() )
        {
            m_JointRotateSlider.SetMaxBound( hinge_geom_ptr->m_JointRotMax() );
        }
    }

    GeomScreen::GuiDeviceCallBack( device );
}
