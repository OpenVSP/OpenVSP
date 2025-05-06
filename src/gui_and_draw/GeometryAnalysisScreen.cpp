//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GeometryAnalysisScreen.h"
#include "GeometryAnalysisMgr.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"
#include <FL/fl_ask.H>

#include "ModeMgr.h"

//==== Constructor ====//
GeometryAnalysisScreen::GeometryAnalysisScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 600, 700, "Geometry Analyses" )
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

    m_GeometryAnalysisBrowser = m_BorderLayout.AddColResizeBrowser( out_col_widths, 4, 200 );
    m_GeometryAnalysisBrowser->callback( staticScreenCB, this );
    m_GeometryAnalysisBrowser->type( FL_MULTI_BROWSER );


    m_BorderLayout.SetButtonWidth( m_BorderLayout.GetW() / 2 );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_AddGeometryAnalysis, "Add" );
    m_BorderLayout.AddButton( m_DelGeometryAnalysis, "Del" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_EvaluateAllGeometryAnalyses, "Evaluate All" );
    m_BorderLayout.AddButton( m_DelAllGeometryAnalyses, "Del All" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_ShowBoth, "Show" );
    m_BorderLayout.AddButton( m_ShowOnlyBoth, "Show Only" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );


    m_BorderLayout.AddSubGroupLayout( m_GCaseLayout, m_BorderLayout.GetW(), m_BorderLayout.GetRemainY() );

    m_GCaseLayout.AddYGap();
    m_GCaseLayout.AddDividerBox( "Geometry Analysis" );

    m_GCaseLayout.AddInput( m_GANameInput, "Name" );

    m_GCaseLayout.AddYGap();

    m_GCaseLayout.AddChoice( m_GeometryAnalysisTypeChoice, "Type" );
    m_GeometryAnalysisTypeChoice.AddItem( "External", vsp::EXTERNAL_INTERFERENCE );
    m_GeometryAnalysisTypeChoice.AddItem( "Packaging", vsp::PACKAGING_INTERFERENCE );
    m_GeometryAnalysisTypeChoice.AddItem( "Self External", vsp::EXTERNAL_SELF_INTERFERENCE );
    m_GeometryAnalysisTypeChoice.AddItem( "Plane Distance", vsp::PLANE_STATIC_DISTANCE_INTERFERENCE );
    m_GeometryAnalysisTypeChoice.AddItem( "Plane 2pt Angle", vsp::PLANE_2PT_ANGLE_INTERFERENCE );
    m_GeometryAnalysisTypeChoice.AddItem( "Tipback", vsp::GEAR_CG_TIPBACK_ANALYSIS );
    m_GeometryAnalysisTypeChoice.AddItem( "Plane 1pt Angle", vsp::PLANE_1PT_ANGLE_INTERFERENCE );
    m_GeometryAnalysisTypeChoice.AddItem( "Weight Distribution", vsp::GEAR_WEIGHT_DISTRIBUTION_ANALYSIS );
    m_GeometryAnalysisTypeChoice.AddItem( "Tipover", vsp::GEAR_TIPOVER_ANALYSIS );
    m_GeometryAnalysisTypeChoice.AddItem( "Ground Maneuverability", vsp::GEAR_TURN_ANALYSIS );
    m_GeometryAnalysisTypeChoice.UpdateItems();

    m_GCaseLayout.AddYGap();

    m_GCaseLayout.AddDividerBox( "Primary" );

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetFitWidthFlag( false );

    int bw = m_GCaseLayout.GetChoiceButtonWidth();
    m_GCaseLayout.SetButtonWidth( bw );

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetChoiceButtonWidth( 0 );
    m_GCaseLayout.SetFitWidthFlag( false );
    m_GCaseLayout.AddButton( m_PrimarySetToggle, "Set:" );
    m_GCaseLayout.SetFitWidthFlag( true );
    m_GCaseLayout.AddChoice(m_PrimarySetChoice, "", bw);
    m_GCaseLayout.ForceNewLine();

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetChoiceButtonWidth( 0 );
    m_GCaseLayout.SetFitWidthFlag( false );
    m_GCaseLayout.AddButton( m_PrimaryModeToggle, "Mode:" );
    m_GCaseLayout.SetFitWidthFlag( true );
    m_GCaseLayout.AddChoice(m_PrimaryModeChoice, "", bw );
    m_GCaseLayout.ForceNewLine();

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetChoiceButtonWidth( 0 );
    m_GCaseLayout.SetFitWidthFlag( false );
    m_GCaseLayout.AddButton( m_PrimaryGeomToggle, "Geom:" );
    m_GCaseLayout.SetFitWidthFlag( true );
    m_PrimaryGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_PrimaryGeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_PrimaryGeomPicker.AddExcludeType( HINGE_GEOM_TYPE );
    m_GCaseLayout.AddGeomPicker( m_PrimaryGeomPicker, m_GCaseLayout.GetButtonWidth(), "" );
    m_GCaseLayout.ForceNewLine();

    m_PrimaryToggleGroup.Init( this );
    m_PrimaryToggleGroup.AddButton( m_PrimarySetToggle.GetFlButton() ); // Order matters.
    m_PrimaryToggleGroup.AddButton( m_PrimaryGeomToggle.GetFlButton() );
    m_PrimaryToggleGroup.AddButton( m_PrimaryModeToggle.GetFlButton() );

    m_GCaseLayout.SetFitWidthFlag( false );

    m_GCaseLayout.SetButtonWidth( m_GCaseLayout.GetW() * 0.5 );
    m_GCaseLayout.AddButton( m_ShowPrimaryGeom, "Show" );
    m_GCaseLayout.AddButton( m_ShowOnlyPrimaryGeom, "Show Only" );
    m_GCaseLayout.ForceNewLine();

    m_GCaseLayout.SetSameLineFlag( false );
    m_GCaseLayout.SetFitWidthFlag( true );

    m_GCaseLayout.AddYGap();

    m_GCaseLayout.AddDividerBox( "Secondary" );

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetFitWidthFlag( false );

    m_GCaseLayout.SetButtonWidth( bw );

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetChoiceButtonWidth( 0 );
    m_GCaseLayout.SetFitWidthFlag( false );
    m_GCaseLayout.AddButton( m_SecondarySetToggle, "Set:" );
    m_GCaseLayout.SetFitWidthFlag( true );
    m_GCaseLayout.AddChoice(m_SecondarySetChoice, "", bw);
    m_GCaseLayout.ForceNewLine();

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetChoiceButtonWidth( 0 );
    m_GCaseLayout.SetFitWidthFlag( false );
    m_GCaseLayout.AddButton( m_SecondaryGeomToggle, "Geom:" );
    m_GCaseLayout.SetFitWidthFlag( true );
    m_SecondaryGeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_SecondaryGeomPicker.AddExcludeType( BLANK_GEOM_TYPE );
    m_SecondaryGeomPicker.AddExcludeType( HINGE_GEOM_TYPE );
    m_GCaseLayout.AddGeomPicker( m_SecondaryGeomPicker, m_GCaseLayout.GetButtonWidth(), "" );

    m_GCaseLayout.ForceNewLine();

    m_SecondaryToggleGroup.Init( this );
    m_SecondaryToggleGroup.AddButton( m_SecondarySetToggle.GetFlButton() );
    m_SecondaryToggleGroup.AddButton( m_SecondaryGeomToggle.GetFlButton() );

    m_GCaseLayout.SetSameLineFlag( false );
    m_GCaseLayout.SetFitWidthFlag( true );

    m_GCaseLayout.AddButton( m_SecondaryUseZGroundToggle, "Use Z Plane" );
    m_GCaseLayout.AddSlider( m_SecondaryZGroundSlider, "Z", 10, "%6.4f" );

    m_GCaseLayout.AddYGap();

    m_GCaseLayout.AddButton( m_CCWToggle, "CCW" );
    m_GCaseLayout.AddButton( m_CWToggle, "CW" );
    m_GCaseLayout.ForceNewLine();

    m_CCWToggleGroup.Init( this );
    m_CCWToggleGroup.AddButton( m_CCWToggle.GetFlButton() );
    m_CCWToggleGroup.AddButton( m_CWToggle.GetFlButton() );

    m_GCaseLayout.SetSameLineFlag( true );
    m_GCaseLayout.SetFitWidthFlag( false );

    m_GCaseLayout.SetButtonWidth( m_GCaseLayout.GetW() * 0.5 );
    m_GCaseLayout.AddButton( m_ShowSecondaryGeom, "Show" );
    m_GCaseLayout.AddButton( m_ShowOnlySecondaryGeom, "Show Only" );
    m_GCaseLayout.ForceNewLine();

    m_GCaseLayout.SetSameLineFlag( false );
    m_GCaseLayout.SetFitWidthFlag( true );

    m_GCaseLayout.AddYGap();

    m_GCaseLayout.AddButton( m_Evaluate, "Evaluate" );

    m_GCaseLayout.AddOutput( m_ResultOutput, "Result", "%6.5f" );

    m_GeometryBrowserSelect = -1;
}

GeometryAnalysisScreen::~GeometryAnalysisScreen()
{
}

//==== Update Screen ====//
bool GeometryAnalysisScreen::Update()
{
    BasicScreen::Update();

    Vehicle *veh = VehicleMgr.GetVehicle();

    GeometryAnalysisMgr.Update();

    UpdateGeometryAnalysisBrowser();

    m_PrimaryGeomPicker.Update();
    m_SecondaryGeomPicker.Update();

    GeometryAnalysisCase* gcase = GeometryAnalysisMgr.GetGeometryAnalysis( m_GeometryBrowserSelect );

    if ( gcase )
    {
        m_GCaseLayout.GetGroup()->activate();


        m_GANameInput.Update( gcase->GetName() );

        m_GeometryAnalysisTypeChoice.Update( gcase->m_GeometryAnalysisType.GetID() );

        m_ResultOutput.Update( gcase->m_LastResultValue.GetID() );

        m_SecondaryZGroundSlider.Update( gcase->m_SecondaryZGround.GetID() );
        m_SecondaryUseZGroundToggle.Update( gcase->m_SecondaryUseZGround.GetID() );

        m_CCWToggleGroup.Update( gcase->m_SecondaryCCWFlag.GetID() );

        Vehicle *veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            m_ScreenMgr->LoadSetChoice( {&m_PrimarySetChoice, &m_SecondarySetChoice}, {gcase->m_PrimarySet.GetID(), gcase->m_SecondarySet.GetID()} );

            m_ScreenMgr->LoadModeChoice( m_PrimaryModeChoice, m_ModeIDs, gcase->m_PrimaryModeID );

            if ( !veh->FindGeom( gcase->m_PrimaryGeomID ) )
            {
                gcase->m_PrimaryGeomID = m_PrimaryGeomPicker.GetGeomChoice();
            }
            else
            {
                m_PrimaryGeomPicker.SetGeomChoice( gcase->m_PrimaryGeomID );
            }

            if ( !veh->FindGeom( gcase->m_SecondaryGeomID ) )
            {
                gcase->m_SecondaryGeomID = m_SecondaryGeomPicker.GetGeomChoice();
            }
            else
            {
                m_SecondaryGeomPicker.SetGeomChoice( gcase->m_SecondaryGeomID );
            }


            m_PrimaryToggleGroup.Update( gcase->m_PrimaryType.GetID() );
            m_SecondaryToggleGroup.Update( gcase->m_SecondaryType.GetID() );

            if ( ModeMgr.GetNumModes() == 0 )
            {
                if ( gcase->m_PrimaryType() == vsp::MODE_TARGET )
                {
                    gcase->m_PrimaryType = vsp::SET_TARGET;
                    m_ScreenMgr->SetUpdateFlag( true );
                }
                m_PrimaryModeToggle.Deactivate();
            }
            else
            {
                m_PrimaryModeToggle.Activate();
            }

            if ( gcase->m_PrimaryType() == vsp::MODE_TARGET )
            {
                m_PrimaryModeChoice.Activate();
                m_PrimarySetChoice.Deactivate();
                m_PrimaryGeomPicker.Deactivate();

                Mode *m = ModeMgr.GetMode( gcase->m_PrimaryModeID );
                if ( m )
                {
                    if ( gcase->m_PrimarySet() != m->m_NormalSet() )
                    {
                        gcase->m_PrimarySet = m->m_NormalSet();
                        m_ScreenMgr->SetUpdateFlag( true );
                    }
                }
            }
            else if ( gcase->m_PrimaryType() == vsp::SET_TARGET )
            {
                m_PrimaryModeChoice.Deactivate();
                m_PrimarySetChoice.Activate();
                m_PrimaryGeomPicker.Deactivate();

            }
            else if ( gcase->m_PrimaryType() == vsp::GEOM_TARGET )
            {
                m_PrimaryModeChoice.Deactivate();
                m_PrimarySetChoice.Deactivate();
                m_PrimaryGeomPicker.Activate();
            }


            if ( gcase->m_GeometryAnalysisType() != vsp::EXTERNAL_SELF_INTERFERENCE )
            {
                m_SecondarySetToggle.Activate();
                m_SecondaryGeomToggle.Activate();

                m_ShowSecondaryGeom.Activate();
                m_ShowOnlySecondaryGeom.Activate();

                if ( gcase->m_SecondaryType() == vsp::SET_TARGET )
                {
                    m_SecondarySetChoice.Activate();
                    m_SecondaryGeomPicker.Deactivate();
                }
                else if ( gcase->m_SecondaryType() == vsp::GEOM_TARGET )
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
        m_GCaseLayout.GetGroup()->deactivate();

        m_GANameInput.Update( "" );

        m_ResultOutput.Update( "" );

        m_PrimaryGeomPicker.SetGeomChoice( "" );
        m_SecondaryGeomPicker.SetGeomChoice( "" );

    }


    m_FLTK_Window->redraw();
    return true;
}

void GeometryAnalysisScreen::UpdateGeometryAnalysisBrowser()
{
    char str[512];

    int scroll_pos = m_GeometryAnalysisBrowser->vposition();
    int h_pos = m_GeometryAnalysisBrowser->hposition();
    m_GeometryAnalysisBrowser->clear();
    m_GeometryAnalysisBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.NAME:@b@.PRIMARY:@b@.SECONDARY:@b@.RESULT" );
    m_GeometryAnalysisBrowser->add( str );
    m_GeometryAnalysisBrowser->addCopyText( "header" );

    vector < GeometryAnalysisCase* > gcases = GeometryAnalysisMgr.GetAllGeometryAnalyses();
    for ( int i = 0 ; i < (int)gcases.size() ; i++ )
    {
        snprintf( str, sizeof( str ),  "%s:%s:%s:%f: \n", gcases[i]->GetName().c_str(), gcases[i]->GetPrimaryName().c_str(), gcases[i]->GetSecondaryName().c_str(), gcases[i]->m_LastResultValue() );
        m_GeometryAnalysisBrowser->add( str );
    }
    if ( m_GeometryBrowserSelect >= 0 && m_GeometryBrowserSelect < (int)gcases.size() )
    {
        m_GeometryAnalysisBrowser->select( m_GeometryBrowserSelect + 2 );
    }

    m_GeometryAnalysisBrowser->vposition( scroll_pos );
    m_GeometryAnalysisBrowser->hposition( h_pos );
}

void GeometryAnalysisScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    GeometryAnalysisCase* gcase = GeometryAnalysisMgr.GetGeometryAnalysis( m_GeometryBrowserSelect );

    if ( gcase )
    {
        gcase->LoadDrawObjs( draw_obj_vec );
    }
}

void GeometryAnalysisScreen::MarkDOChanged()
{
    vector< DrawObj* > draw_obj_vec;
    LoadDrawObjs( draw_obj_vec );

    for ( int i = 0; i < draw_obj_vec.size(); i++ )
    {
        draw_obj_vec[i]->m_GeomChanged = true;
    }
}

bool GeometryAnalysisScreen::GetVisBndBox( BndBox &bbox )
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
void GeometryAnalysisScreen::Show()
{
    MarkDOChanged();
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}


//==== Hide Screen ====//
void GeometryAnalysisScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

//==== Callbacks ====//
void GeometryAnalysisScreen::CallBack( Fl_Widget *w )
{
    Vehicle *veh = VehicleMgr.GetVehicle();

    if ( w == m_GeometryAnalysisBrowser )
    {
        int sel = m_GeometryAnalysisBrowser->value();
        m_GeometryBrowserSelect = sel - 2;

        GeometryAnalysisCase* gcase = GeometryAnalysisMgr.GetGeometryAnalysis( m_GeometryBrowserSelect );

        if ( gcase )
        {
            m_PrimaryGeomPicker.SetGeomChoice( gcase->m_PrimaryGeomID );
            m_SecondaryGeomPicker.SetGeomChoice( gcase->m_SecondaryGeomID );
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
void GeometryAnalysisScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    GeometryAnalysisCase* gcase = GeometryAnalysisMgr.GetGeometryAnalysis( m_GeometryBrowserSelect );

    if ( gui_device == &m_AddGeometryAnalysis )
    {
        string id = GeometryAnalysisMgr.AddGeometryAnalysis();
        m_GeometryBrowserSelect = GeometryAnalysisMgr.GetGeometryAnalysisIndex( id );
    }
    else if ( gui_device == &m_DelGeometryAnalysis )
    {
        GeometryAnalysisMgr.DeleteGeometryAnalysis( m_GeometryBrowserSelect );
    }
    else if ( gui_device == &m_DelAllGeometryAnalyses )
    {
        GeometryAnalysisMgr.DeleteAllGeometryAnalyses();
    }
    else if ( gui_device == & m_GANameInput )
    {
        if ( gcase )
        {
            gcase->SetName( m_GANameInput.GetString() );
        }
    }
    else if ( gui_device == &m_PrimaryModeChoice || gui_device == &m_PrimaryToggleGroup )
    {
        if ( gcase )
        {
            if ( gcase->m_PrimaryType() == vsp::MODE_TARGET )
            {
                int indx = m_PrimaryModeChoice.GetVal();
                if ( indx >= 0  && indx < m_ModeIDs.size() )
                {
                    gcase->m_PrimaryModeID = m_ModeIDs[ indx ];
                }
                else
                {
                    gcase->m_PrimaryModeID = "";
                }
            }

            // Include this code if we want to actively visualize the mode in real time.
            /*
            Mode *m = ModeMgr.GetMode( gcase->m_BaseModeID );
            if ( m )
            {
                m->ApplySettings();
            }
            */
        }
    }
    else if ( gui_device == & m_PrimaryGeomPicker )
    {
        if ( gcase )
        {
            gcase->m_PrimaryGeomID = m_PrimaryGeomPicker.GetGeomChoice();
        }
    }
    else if ( gui_device == & m_SecondaryGeomPicker )
    {
        if ( gcase )
        {
            gcase->m_SecondaryGeomID = m_SecondaryGeomPicker.GetGeomChoice();
        }
    }
    else if ( gui_device == & m_ShowBoth )
    {
        if ( gcase )
        {
            gcase->ShowBoth();
        }
    }
    else if ( gui_device == & m_ShowOnlyBoth )
    {
        if ( gcase )
        {
            gcase->ShowOnlyBoth();
        }
    }
    else if ( gui_device == & m_ShowPrimaryGeom )
    {
        if ( gcase )
        {
            gcase->ShowPrimary();
        }
    }
    else if ( gui_device == & m_ShowOnlyPrimaryGeom )
    {
        if ( gcase )
        {
            gcase->ShowOnlyPrimary();
        }
    }
    else if ( gui_device == & m_ShowSecondaryGeom )
    {
        if ( gcase )
        {
            gcase->ShowSecondary();
        }
    }
    else if ( gui_device == & m_ShowOnlySecondaryGeom )
    {
        if ( gcase )
        {
            gcase->ShowOnlySecondary();
        }
    }
    else if ( gui_device == &m_EvaluateAllGeometryAnalyses )
    {
        GeometryAnalysisMgr.EvaluateAll();
    }
    else if ( gui_device == &m_Evaluate )
    {
        if ( gcase )
        {
            gcase->Evaluate();
        }
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
