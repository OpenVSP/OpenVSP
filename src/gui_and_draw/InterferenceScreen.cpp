//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "InterferenceScreen.h"
#include "InterferenceMgr.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"
#include <FL/fl_ask.H>

#include "ModeMgr.h"

//==== Constructor ====//
InterferenceScreen::InterferenceScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 600, 700, "Interference Checks" )
{
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY(7);
    m_GenLayout.AddX(5);

    m_GenLayout.AddSubGroupLayout( m_BorderLayout, m_GenLayout.GetRemainX() - 5.0,
                                   m_GenLayout.GetRemainY() - 5.0);


    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int out_col_widths[] = { 200, 100, 100, 100, 0 }; // widths for each column

    m_InterferenceCheckBrowser = m_BorderLayout.AddColResizeBrowser( out_col_widths, 4, 200 );
    m_InterferenceCheckBrowser->callback( staticScreenCB, this );
    m_InterferenceCheckBrowser->type( FL_MULTI_BROWSER );


    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2 );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_AddInterferenceCheck, "Add" );
    m_BorderLayout.AddButton( m_DelInterferenceCheck, "Del" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_EvaluateAllInterferenceChecks, "Evaluate All" );
    m_BorderLayout.AddButton( m_DelAllInterferenceChecks, "Del All" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_ShowBoth, "Show" );
    m_BorderLayout.AddButton( m_ShowOnlyBoth, "Show Only" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );


    m_BorderLayout.AddSubGroupLayout( m_ICaseLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );

    m_ICaseLayout.AddYGap();
    m_ICaseLayout.AddDividerBox( "Interference Check" );

    m_ICaseLayout.AddInput( m_ICNameInput, "Name" );

    m_ICaseLayout.AddYGap();

    m_ICaseLayout.AddChoice( m_InterferenceTypeChoice, "Type" );
    m_InterferenceTypeChoice.AddItem( "External", vsp::EXTERNAL_INTERFERENCE );
    m_InterferenceTypeChoice.AddItem( "Packaging", vsp::PACKAGING_INTERFERENCE );
    m_InterferenceTypeChoice.AddItem( "Self External", vsp::EXTERNAL_SELF_INTERFERENCE );
    m_InterferenceTypeChoice.AddItem( "Plane Distance", vsp::PLANE_STATIC_DISTANCE_INTERFERENCE );
    m_InterferenceTypeChoice.AddItem( "Plane 2pt Angle", vsp::PLANE_2PT_ANGLE_INTERFERENCE );
    m_InterferenceTypeChoice.AddItem( "Tipback", vsp::GEAR_CG_TIPBACK_ANALYSIS );
    m_InterferenceTypeChoice.UpdateItems();

    m_ICaseLayout.AddYGap();

    m_ICaseLayout.AddDividerBox( "Primary" );

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetFitWidthFlag( false );

    int bw = m_ICaseLayout.GetChoiceButtonWidth();
    m_ICaseLayout.SetButtonWidth( bw );

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetChoiceButtonWidth( 0 );
    m_ICaseLayout.SetFitWidthFlag( false );
    m_ICaseLayout.AddButton( m_PrimarySetToggle, "Set:" );
    m_ICaseLayout.SetFitWidthFlag( true );
    m_ICaseLayout.AddChoice(m_PrimarySetChoice, "", bw);
    m_ICaseLayout.ForceNewLine();

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetChoiceButtonWidth( 0 );
    m_ICaseLayout.SetFitWidthFlag( false );
    m_ICaseLayout.AddButton( m_PrimaryModeToggle, "Mode:" );
    m_ICaseLayout.SetFitWidthFlag( true );
    m_ICaseLayout.AddChoice(m_PrimaryModeChoice, "", bw );
    m_ICaseLayout.ForceNewLine();

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetChoiceButtonWidth( 0 );
    m_ICaseLayout.SetFitWidthFlag( false );
    m_ICaseLayout.AddButton( m_PrimaryGeomToggle, "Geom:" );
    m_ICaseLayout.SetFitWidthFlag( true );
    m_PrimaryGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_PrimaryGeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_PrimaryGeomPicker.AddExcludeType( HINGE_GEOM_TYPE );
    m_ICaseLayout.AddGeomPicker( m_PrimaryGeomPicker, m_ICaseLayout.GetButtonWidth(), "" );
    m_ICaseLayout.ForceNewLine();

    m_PrimaryToggleGroup.Init( this );
    m_PrimaryToggleGroup.AddButton( m_PrimarySetToggle.GetFlButton() ); // Order matters.
    m_PrimaryToggleGroup.AddButton( m_PrimaryGeomToggle.GetFlButton() );
    m_PrimaryToggleGroup.AddButton( m_PrimaryModeToggle.GetFlButton() );

    m_ICaseLayout.SetFitWidthFlag( false );

    m_ICaseLayout.SetButtonWidth( m_ICaseLayout.GetW() * 0.5 );
    m_ICaseLayout.AddButton( m_ShowPrimaryGeom, "Show" );
    m_ICaseLayout.AddButton( m_ShowOnlyPrimaryGeom, "Show Only" );
    m_ICaseLayout.ForceNewLine();

    m_ICaseLayout.SetSameLineFlag( false );
    m_ICaseLayout.SetFitWidthFlag( true );

    m_ICaseLayout.AddYGap();

    m_ICaseLayout.AddDividerBox( "Secondary" );

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetFitWidthFlag( false );

    m_ICaseLayout.SetButtonWidth( bw );

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetChoiceButtonWidth( 0 );
    m_ICaseLayout.SetFitWidthFlag( false );
    m_ICaseLayout.AddButton( m_SecondarySetToggle, "Set:" );
    m_ICaseLayout.SetFitWidthFlag( true );
    m_ICaseLayout.AddChoice(m_SecondarySetChoice, "", bw);
    m_ICaseLayout.ForceNewLine();

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetChoiceButtonWidth( 0 );
    m_ICaseLayout.SetFitWidthFlag( false );
    m_ICaseLayout.AddButton( m_SecondaryGeomToggle, "Geom:" );
    m_ICaseLayout.SetFitWidthFlag( true );
    m_SecondaryGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_SecondaryGeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_SecondaryGeomPicker.AddExcludeType( HINGE_GEOM_TYPE );
    m_ICaseLayout.AddGeomPicker( m_SecondaryGeomPicker, m_ICaseLayout.GetButtonWidth(), "" );

    m_ICaseLayout.ForceNewLine();

    m_SecondaryToggleGroup.Init( this );
    m_SecondaryToggleGroup.AddButton( m_SecondarySetToggle.GetFlButton() );
    m_SecondaryToggleGroup.AddButton( m_SecondaryGeomToggle.GetFlButton() );

    m_ICaseLayout.SetSameLineFlag( false );
    m_ICaseLayout.SetFitWidthFlag( true );

    m_ICaseLayout.AddButton( m_SecondaryUseZGroundToggle, "Use Z Plane" );
    m_ICaseLayout.AddSlider( m_SecondaryZGroundSlider, "Z", 10, "%6.4f" );

    m_ICaseLayout.AddYGap();

    m_ICaseLayout.AddButton( m_CCWToggle, "CCW" );
    m_ICaseLayout.AddButton( m_CWToggle, "CW" );
    m_ICaseLayout.ForceNewLine();

    m_CCWToggleGroup.Init( this );
    m_CCWToggleGroup.AddButton( m_CCWToggle.GetFlButton() );
    m_CCWToggleGroup.AddButton( m_CWToggle.GetFlButton() );

    m_ICaseLayout.SetSameLineFlag( true );
    m_ICaseLayout.SetFitWidthFlag( false );

    m_ICaseLayout.SetButtonWidth( m_ICaseLayout.GetW() * 0.5 );
    m_ICaseLayout.AddButton( m_ShowSecondaryGeom, "Show" );
    m_ICaseLayout.AddButton( m_ShowOnlySecondaryGeom, "Show Only" );
    m_ICaseLayout.ForceNewLine();

    m_ICaseLayout.SetSameLineFlag( false );
    m_ICaseLayout.SetFitWidthFlag( true );

    m_ICaseLayout.AddYGap();

    m_ICaseLayout.AddButton( m_Evaluate, "Evaluate" );

    m_ICaseLayout.AddOutput( m_ResultOutput, "Result", "%6.5f" );

    m_InterferenceBrowserSelect = -1;
}

InterferenceScreen::~InterferenceScreen()
{
}

//==== Update Screen ====//
bool InterferenceScreen::Update()
{
    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    InterferenceMgr.Update();

    UpdateInterferenceCheckBrowser();

    m_PrimaryGeomPicker.Update();
    m_SecondaryGeomPicker.Update();

    InterferenceCase* icase = InterferenceMgr.GetInterferenceCase( m_InterferenceBrowserSelect );

    if ( icase )
    {
        m_ICaseLayout.GetGroup()->activate();


        m_ICNameInput.Update( icase->GetName() );

        m_InterferenceTypeChoice.Update( icase->m_IntererenceCheckType.GetID() );

        m_ResultOutput.Update( icase->m_LastResultValue.GetID() );

        m_SecondaryZGroundSlider.Update( icase->m_SecondaryZGround.GetID() );
        m_SecondaryUseZGroundToggle.Update( icase->m_SecondaryUseZGround.GetID() );

        m_CCWToggleGroup.Update( icase->m_SecondaryCCWFlag.GetID() );

        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            m_ScreenMgr->LoadSetChoice( {&m_PrimarySetChoice, &m_SecondarySetChoice}, {icase->m_PrimarySet.GetID(), icase->m_SecondarySet.GetID()} );

            m_ScreenMgr->LoadModeChoice( m_PrimaryModeChoice, m_ModeIDs, icase->m_PrimaryModeID );

            if ( !veh->FindGeom( icase->m_PrimaryGeomID ) )
            {
                icase->m_PrimaryGeomID = m_PrimaryGeomPicker.GetGeomChoice();
            }
            else
            {
                m_PrimaryGeomPicker.SetGeomChoice( icase->m_PrimaryGeomID );
            }

            if ( !veh->FindGeom( icase->m_SecondaryGeomID ) )
            {
                icase->m_SecondaryGeomID = m_SecondaryGeomPicker.GetGeomChoice();
            }
            else
            {
                m_SecondaryGeomPicker.SetGeomChoice( icase->m_SecondaryGeomID );
            }


            m_PrimaryToggleGroup.Update( icase->m_PrimaryType.GetID() );
            m_SecondaryToggleGroup.Update( icase->m_SecondaryType.GetID() );

            if ( ModeMgr.GetNumModes() == 0 )
            {
                if ( icase->m_PrimaryType() == vsp::MODE_TARGET )
                {
                    icase->m_PrimaryType = vsp::SET_TARGET;
                    m_ScreenMgr->SetUpdateFlag( true );
                }
                m_PrimaryModeToggle.Deactivate();
            }
            else
            {
                m_PrimaryModeToggle.Activate();
            }

            if ( icase->m_PrimaryType() == vsp::MODE_TARGET )
            {
                m_PrimaryModeChoice.Activate();
                m_PrimarySetChoice.Deactivate();
                m_PrimaryGeomPicker.Deactivate();

                Mode *m = ModeMgr.GetMode( icase->m_PrimaryModeID );
                if ( m )
                {
                    if ( icase->m_PrimarySet() != m->m_NormalSet() )
                    {
                        icase->m_PrimarySet = m->m_NormalSet();
                        m_ScreenMgr->SetUpdateFlag( true );
                    }
                }
            }
            else if ( icase->m_PrimaryType() == vsp::SET_TARGET )
            {
                m_PrimaryModeChoice.Deactivate();
                m_PrimarySetChoice.Activate();
                m_PrimaryGeomPicker.Deactivate();

            }
            else if ( icase->m_PrimaryType() == vsp::GEOM_TARGET )
            {
                m_PrimaryModeChoice.Deactivate();
                m_PrimarySetChoice.Deactivate();
                m_PrimaryGeomPicker.Activate();
            }


            if ( icase->m_IntererenceCheckType() != vsp::EXTERNAL_SELF_INTERFERENCE )
            {
                m_SecondarySetToggle.Activate();
                m_SecondaryGeomToggle.Activate();

                m_ShowSecondaryGeom.Activate();
                m_ShowOnlySecondaryGeom.Activate();

                if ( icase->m_SecondaryType() == vsp::SET_TARGET )
                {
                    m_SecondarySetChoice.Activate();
                    m_SecondaryGeomPicker.Deactivate();
                }
                else if ( icase->m_SecondaryType() == vsp::GEOM_TARGET )
                {
                    m_SecondarySetChoice.Deactivate();
                    m_SecondaryGeomPicker.Activate();
                }
            }
            else
            {
                m_SecondarySetToggle.Deactivate();
                m_SecondaryGeomToggle.Deactivate();
                m_SecondarySetChoice.Deactivate();
                m_SecondaryGeomPicker.Deactivate();

                m_ShowSecondaryGeom.Deactivate();
                m_ShowOnlySecondaryGeom.Deactivate();
            }


        }

    }
    else
    {
        m_ICaseLayout.GetGroup()->deactivate();

        m_ICNameInput.Update( "" );

        m_ResultOutput.Update( "" );

        m_PrimaryGeomPicker.SetGeomChoice( "" );
        m_SecondaryGeomPicker.SetGeomChoice( "" );

    }


    m_FLTK_Window->redraw();
    return true;
}

void InterferenceScreen::UpdateInterferenceCheckBrowser()
{
    char str[512];

    int scroll_pos = m_InterferenceCheckBrowser->vposition();
    int h_pos = m_InterferenceCheckBrowser->hposition();
    m_InterferenceCheckBrowser->clear();
    m_InterferenceCheckBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.NAME:@b@.PRIMARY:@b@.SECONDARY:@b@.RESULT" );
    m_InterferenceCheckBrowser->add( str );
    m_InterferenceCheckBrowser->addCopyText( "header" );

    vector < InterferenceCase* > icases = InterferenceMgr.GetAllInterferenceCases();
    for ( int i = 0 ; i < (int)icases.size() ; i++ )
    {
        snprintf( str, sizeof( str ),  "%s:%s:%s:%f: \n", icases[i]->GetName().c_str(), icases[i]->GetPrimaryName().c_str(), icases[i]->GetSecondaryName().c_str(), icases[i]->m_LastResultValue() );
        m_InterferenceCheckBrowser->add( str );
    }
    if ( m_InterferenceBrowserSelect >= 0 && m_InterferenceBrowserSelect < (int)icases.size() )
    {
        m_InterferenceCheckBrowser->select( m_InterferenceBrowserSelect + 2 );
    }

    m_InterferenceCheckBrowser->vposition( scroll_pos );
    m_InterferenceCheckBrowser->hposition( h_pos );
}

void InterferenceScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    InterferenceCase* icase = InterferenceMgr.GetInterferenceCase( m_InterferenceBrowserSelect );

    if ( icase )
    {
        icase->LoadDrawObjs( draw_obj_vec );
    }
}

void InterferenceScreen::MarkDOChanged()
{
    vector< DrawObj* > draw_obj_vec;
    LoadDrawObjs( draw_obj_vec );

    for ( int i = 0; i < draw_obj_vec.size(); i++ )
    {
        draw_obj_vec[i]->m_GeomChanged = true;
    }
}

bool InterferenceScreen::GetVisBndBox( BndBox &bbox )
{
    bool anyvisible = false;

    vector< DrawObj* > draw_obj_vec;
    LoadDrawObjs( draw_obj_vec );

    for(int j = 0; j < (int)draw_obj_vec.size(); j++)
    {
        if(draw_obj_vec[j]->m_Visible)
        {
            bbox.Update( draw_obj_vec[j]->m_PntVec );
            bbox.Update( draw_obj_vec[j]->m_PntMesh );
            anyvisible = true;
        }
    }

    return anyvisible;
}

//==== Show Screen ====//
void InterferenceScreen::Show()
{
    MarkDOChanged();
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}


//==== Hide Screen ====//
void InterferenceScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void InterferenceScreen::CallBack( Fl_Widget *w )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( w == m_InterferenceCheckBrowser )
    {
        int sel = m_InterferenceCheckBrowser->value();
        m_InterferenceBrowserSelect = sel - 2;

        InterferenceCase* icase = InterferenceMgr.GetInterferenceCase( m_InterferenceBrowserSelect );

        if ( icase )
        {
            m_PrimaryGeomPicker.SetGeomChoice( icase->m_PrimaryGeomID );
            m_SecondaryGeomPicker.SetGeomChoice( icase->m_SecondaryGeomID );
        }

        MarkDOChanged();
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Gui Device CallBacks ====//
void InterferenceScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    InterferenceCase* icase = InterferenceMgr.GetInterferenceCase( m_InterferenceBrowserSelect );

    if ( gui_device == &m_AddInterferenceCheck )
    {
        string id = InterferenceMgr.AddInterferenceCase();
        m_InterferenceBrowserSelect = InterferenceMgr.GetInterferenceCaseIndex( id );
    }
    else if ( gui_device == &m_DelInterferenceCheck )
    {
        InterferenceMgr.DeleteInterferenceCase( m_InterferenceBrowserSelect );
    }
    else if ( gui_device == &m_DelAllInterferenceChecks )
    {
        InterferenceMgr.DeleteAllInterferenceCases();
    }
    else if ( gui_device == & m_ICNameInput )
    {
        if ( icase )
        {
            icase->SetName( m_ICNameInput.GetString() );
        }
    }
    else if ( gui_device == &m_PrimaryModeChoice || gui_device == &m_PrimaryToggleGroup )
    {
        if ( icase )
        {
            if ( icase->m_PrimaryType() == vsp::MODE_TARGET )
            {
                int indx = m_PrimaryModeChoice.GetVal();
                if ( indx >= 0  && indx < m_ModeIDs.size() )
                {
                    icase->m_PrimaryModeID = m_ModeIDs[ indx ];
                }
                else
                {
                    icase->m_PrimaryModeID = "";
                }
            }

            // Include this code if we want to actively visualize the mode in real time.
            /*
            Mode *m = ModeMgr.GetMode( icase->m_BaseModeID );
            if ( m )
            {
                m->ApplySettings();
            }
            */
        }
    }
    else if ( gui_device == & m_PrimaryGeomPicker )
    {
        if ( icase )
        {
            icase->m_PrimaryGeomID = m_PrimaryGeomPicker.GetGeomChoice();
        }
    }
    else if ( gui_device == & m_SecondaryGeomPicker )
    {
        if ( icase )
        {
            icase->m_SecondaryGeomID = m_SecondaryGeomPicker.GetGeomChoice();
        }
    }
    else if ( gui_device == & m_ShowBoth )
    {
        if ( icase )
        {
            icase->ShowBoth();
        }
    }
    else if ( gui_device == & m_ShowOnlyBoth )
    {
        if ( icase )
        {
            icase->ShowOnlyBoth();
        }
    }
    else if ( gui_device == & m_ShowPrimaryGeom )
    {
        if ( icase )
        {
            icase->ShowPrimary();
        }
    }
    else if ( gui_device == & m_ShowOnlyPrimaryGeom )
    {
        if ( icase )
        {
            icase->ShowOnlyPrimary();
        }
    }
    else if ( gui_device == & m_ShowSecondaryGeom )
    {
        if ( icase )
        {
            icase->ShowSecondary();
        }
    }
    else if ( gui_device == & m_ShowOnlySecondaryGeom )
    {
        if ( icase )
        {
            icase->ShowOnlySecondary();
        }
    }
    else if ( gui_device == &m_EvaluateAllInterferenceChecks )
    {
        InterferenceMgr.EvaluateAll();
    }
    else if ( gui_device == &m_Evaluate )
    {
        if ( icase )
        {
            icase->Evaluate();
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
