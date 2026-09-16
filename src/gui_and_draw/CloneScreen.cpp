//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CloneScreen.h"
#include "ScreenMgr.h"
#include "CloneGeom.h"


//==== Constructor ====//
CloneScreen::CloneScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Clone" )
{
    Fl_Group* clone_tab = AddTab( "Clone" );
    Fl_Group* clone_group = AddSubGroup( clone_tab, 5 );

    m_CloneLayout.SetGroupAndScreen( clone_group, this );

    m_CloneLayout.AddDividerBox( "Original" );
    m_CloneLayout.AddChoice( m_OriginalChoice, "Original" );

    m_CloneLayout.AddButton( m_ReplaceButton, "Replace With Copy Of Original" );
    m_CloneLayout.AddYGap();

    m_CloneLayout.AddDividerBox( "Name" );
    m_CloneLayout.AddButton( m_AutoNameButton, "Name After Original" );

    // Appended to the name while Name After Original is on.
    m_CloneLayout.AddInput( m_NameSuffixInput, "Suffix" );
    m_CloneLayout.AddYGap();

    // Surfaces and tessellation are always copied; these are optional.
    m_CloneLayout.AddDividerBox( "Copy From Original" );
    m_CloneLayout.AddButton( m_CloneXFormButton, "Transformation" );
    m_CloneLayout.AddButton( m_CloneAttachButton, "Attachment" );
    m_CloneLayout.AddButton( m_CloneSymButton, "Symmetry and Flip" );
    m_CloneLayout.AddButton( m_CloneSetsButton, "Set Membership" );
    m_CloneLayout.AddButton( m_CloneAppearanceButton, "Color and Material" );
    m_CloneLayout.AddButton( m_CloneNegativeVolumeButton, "Negative Volume" );
    m_CloneLayout.AddButton( m_CloneMassPropsButton, "Mass Properties" );
    m_CloneLayout.AddButton( m_CloneSubSurfsButton, "Subsurfaces" );
    m_CloneLayout.AddButton( m_CloneJointButton, "Joint Deflection" );
    m_CloneLayout.AddYGap();

    // Used only when the original has a joint.  The original decides which motions are allowed;
    // Rng sets the slider range to the original's limits.
    m_CloneLayout.AddDividerBox( "Joint" );

    int bw = 110;
    int sw = 35;

    m_CloneLayout.SetSameLineFlag( true );

    m_CloneLayout.SetFitWidthFlag( false );
    m_CloneLayout.SetButtonWidth( sw );
    m_CloneLayout.AddButton( m_JointTranslateRngButton, "Rng" );
    m_CloneLayout.SetFitWidthFlag( true );
    m_CloneLayout.SetButtonWidth( bw - sw );
    m_CloneLayout.AddSlider( m_JointTranslateSlider, "Translate", 10, "%6.2f" );
    m_CloneLayout.ForceNewLine();

    m_CloneLayout.SetFitWidthFlag( false );
    m_CloneLayout.SetButtonWidth( sw );
    m_CloneLayout.AddButton( m_JointRotateRngButton, "Rng" );
    m_CloneLayout.SetFitWidthFlag( true );
    m_CloneLayout.SetButtonWidth( bw - sw );
    m_CloneLayout.AddSlider( m_JointRotateSlider, "Rotate", 100, "%6.2f" );
    m_CloneLayout.ForceNewLine();

    m_CloneLayout.SetSameLineFlag( false );
    m_CloneLayout.SetFitWidthFlag( true );
    m_CloneLayout.SetButtonWidth( bw );
    m_CloneLayout.AddYGap();
}


//==== Show Clone Screen ====//
void CloneScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Clone Screen ====//
bool CloneScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CLONE_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    assert( clone_ptr );

    m_CloneSetsButton.Update( clone_ptr->m_CloneSets.GetID() );
    m_CloneSymButton.Update( clone_ptr->m_CloneSym.GetID() );
    m_CloneXFormButton.Update( clone_ptr->m_CloneXForm.GetID() );
    m_CloneAttachButton.Update( clone_ptr->m_CloneAttach.GetID() );
    m_CloneAppearanceButton.Update( clone_ptr->m_CloneAppearance.GetID() );
    m_CloneNegativeVolumeButton.Update( clone_ptr->m_CloneNegativeVolume.GetID() );
    m_CloneMassPropsButton.Update( clone_ptr->m_CloneMassProps.GetID() );
    m_CloneSubSurfsButton.Update( clone_ptr->m_CloneSubSurfs.GetID() );
    m_CloneJointButton.Update( clone_ptr->m_CloneJoint.GetID() );
    m_AutoNameButton.Update( clone_ptr->m_AutoName.GetID() );
    m_NameSuffixInput.Update( clone_ptr->GetNameSuffix() );

    if ( clone_ptr->GetOriginalGeom() )
    {
        m_ReplaceButton.Activate();
    }
    else
    {
        m_ReplaceButton.Deactivate();
    }

    m_JointTranslateSlider.Update( clone_ptr->m_JointTranslate.GetID() );
    m_JointRotateSlider.Update( clone_ptr->m_JointRotate.GetID() );

    if ( !clone_ptr->GetOriginalJoint() )
    {
        m_CloneJointButton.Deactivate();
    }

    // Rng needs a limit to range to.
    bool trans_min_set;
    bool trans_max_set;
    bool rot_min_set;
    bool rot_max_set;
    double min_lim;
    double max_lim;

    bool trans_on = clone_ptr->GetJointTransMotion( trans_min_set, min_lim, trans_max_set, max_lim );
    bool rot_on = clone_ptr->GetJointRotMotion( rot_min_set, min_lim, rot_max_set, max_lim );

    if ( clone_ptr->m_CloneJoint() || !trans_on || ( !trans_min_set && !trans_max_set ) )
    {
        m_JointTranslateRngButton.Deactivate();
    }
    else
    {
        m_JointTranslateRngButton.Activate();
    }

    if ( clone_ptr->m_CloneJoint() || !rot_on || ( !rot_min_set && !rot_max_set ) )
    {
        m_JointRotateRngButton.Deactivate();
    }
    else
    {
        m_JointRotateRngButton.Activate();
    }

    // While the name is automatic, Update overwrites it, so a typed name would be lost.  This
    // keys on the switch alone: a Clone with no original yet adopts its parent on the next update.
    if ( clone_ptr->NameIsAutomatic() )
    {
        m_NameInput.Deactivate();
        m_NameSuffixInput.Activate();
    }
    else
    {
        m_NameInput.Activate();
        m_NameSuffixInput.Deactivate();
    }

    // A Clone takes its size from the original.
    m_ScaleSlider.Deactivate();
    m_ScaleResetButton.Deactivate();
    m_ScaleAcceptButton.Deactivate();

    // Copied settings would be overwritten on the next update, so lock their controls.
    if ( clone_ptr->m_CloneSets() )
    {
        m_SetBrowser->deactivate();
    }
    else
    {
        m_SetBrowser->activate();
    }

    if ( clone_ptr->m_CloneAppearance() )
    {
        m_ColorPicker.Deactivate();
        m_MaterialChoice.Deactivate();
        m_CustomMaterialButton.Deactivate();
    }
    else
    {
        m_ColorPicker.Activate();
        m_MaterialChoice.Activate();
        m_CustomMaterialButton.Activate();
    }

    // Turning copying off keeps the subsurfaces as the Clone's own.
    if ( clone_ptr->m_CloneSubSurfs() )
    {
        m_AddSubSurfButton.Deactivate();
        m_DelSubSurfButton.Deactivate();
        m_SubSurfChoice.Deactivate();
        m_SubSurfSelectSurface.Deactivate();
        m_SSMoveTopButton.Deactivate();
        m_SSMoveUpButton.Deactivate();
        m_SSMoveDownButton.Deactivate();
        m_SSMoveBotButton.Deactivate();
        m_SubSurfBrowser->deactivate();
        if ( m_SSCommonGroup.GetGroup() )
        {
            m_SSCommonGroup.GetGroup()->deactivate();
        }
    }
    else
    {
        m_AddSubSurfButton.Activate();
        m_DelSubSurfButton.Activate();
        m_SubSurfChoice.Activate();
        m_SubSurfSelectSurface.Activate();
        m_SSMoveTopButton.Activate();
        m_SSMoveUpButton.Activate();
        m_SSMoveDownButton.Activate();
        m_SSMoveBotButton.Activate();
        m_SubSurfBrowser->activate();
        if ( m_SSCommonGroup.GetGroup() )
        {
            m_SSCommonGroup.GetGroup()->activate();
        }
    }

    m_OriginalChoice.ClearItems();
    m_CompVec.clear();
    map <string, int> CompIDMap;
    int icomp = 0;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector <string> geomVec = veh->GetGeomVec();

        set < string > below;
        clone_ptr->CollectDescendantIDs( below );

        for ( int i = 0; i < (int)geomVec.size(); i++ )
        {
            char str[256];
            Geom* g = veh->FindGeom( geomVec[i] );
            if ( g )
            {
                // Any Geom can be an original except this Clone, its descendants, or a Geom that
                // would form a cycle of Clones; those would be silently rejected on update.
                if ( geomVec[i] != clone_ptr->GetID() && !below.count( geomVec[i] ) &&
                     !clone_ptr->IsCloneAncestor( geomVec[i] ) )
                {
                    snprintf( str, sizeof( str ), "%d_%s", i, g->GetName().c_str() );
                    m_OriginalChoice.AddItem( str );
                    CompIDMap[ geomVec[i] ] = icomp;
                    m_CompVec.push_back( geomVec[i] );
                    icomp++;
                }
            }
        }
        m_OriginalChoice.UpdateItems();

        // find(), since [] would insert a missing ID and return 0.
        map <string, int>::iterator iorig = CompIDMap.find( clone_ptr->GetOriginalID() );
        if ( iorig != CompIDMap.end() )
        {
            m_OriginalChoice.SetVal( iorig->second );
        }
        else
        {
            m_OriginalChoice.SetVal( -1 );
        }
    }

    return true;
}


//==== Non Menu Callbacks ====//
void CloneScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void CloneScreen::GuiDeviceCallBack( GuiDevice *device )
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CLONE_GEOM_TYPE )
    {
        return;
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    assert( clone_ptr );

    if ( device == &m_NameSuffixInput )
    {
        // The suffix is not a Parm, so update here to rebuild the name.
        clone_ptr->SetNameSuffix( m_NameSuffixInput.GetString() );
        clone_ptr->Update();
    }
    else if ( device == &m_ReplaceButton )
    {
        // Deletes this Geom; clone_ptr is invalid afterwards.
        m_ScreenMgr->GetVehiclePtr()->ReplaceCloneGeom( clone_ptr->GetID() );

        m_ScreenMgr->SetUpdateFlag( true );
        return;
    }
    else if ( device == &m_JointTranslateRngButton )
    {
        bool min_set;
        bool max_set;
        double min_val;
        double max_val;

        if ( clone_ptr->GetJointTransMotion( min_set, min_val, max_set, max_val ) )
        {
            if ( min_set )
            {
                m_JointTranslateSlider.SetMinBound( min_val );
            }
            if ( max_set )
            {
                m_JointTranslateSlider.SetMaxBound( max_val );
            }
        }
    }
    else if ( device == &m_JointRotateRngButton )
    {
        bool min_set;
        bool max_set;
        double min_val;
        double max_val;

        if ( clone_ptr->GetJointRotMotion( min_set, min_val, max_set, max_val ) )
        {
            if ( min_set )
            {
                m_JointRotateSlider.SetMinBound( min_val );
            }
            if ( max_set )
            {
                m_JointRotateSlider.SetMaxBound( max_val );
            }
        }
    }
    else if ( device == &m_OriginalChoice )
    {
        int id = m_OriginalChoice.GetVal();
        if ( id >= 0 && id < ( int )m_CompVec.size() )
        {
            clone_ptr->SetOriginalID( m_CompVec[id] );
        }
    }

    GeomScreen::GuiDeviceCallBack( device );
}




