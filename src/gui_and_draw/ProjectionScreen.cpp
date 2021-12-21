//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ProjectionScreen.h"
#include "ProjectionMgr.h"

ProjectionScreen::ProjectionScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 397, "Projected Area Analysis" )
{
    m_FLTK_Window->callback( staticCloseCB, this );
    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY(7);
    m_MainLayout.AddX(5);

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - 5.0,
                                       m_MainLayout.GetRemainY() - 5.0);

    m_BorderLayout.SetChoiceButtonWidth( 0 );

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddDividerBox("Target");

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.AddButton( m_TargetTypeSet, "Set" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddChoice( m_TargetSet, "", m_BorderLayout.GetButtonWidth() );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_TargetTypeGeom, "Geom" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_TargetGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_TargetGeom.AddExcludeType( WIRE_FRAME_GEOM_TYPE );
    m_TargetGeom.AddExcludeType( BLANK_GEOM_TYPE );
    m_TargetGeom.AddExcludeType( HINGE_GEOM_TYPE );
    m_BorderLayout.AddGeomPicker( m_TargetGeom, m_BorderLayout.GetButtonWidth() );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.ForceNewLine();

    m_TargetTypeGroup.Init( this );
    m_TargetTypeGroup.AddButton( m_TargetTypeSet.GetFlButton() );
    m_TargetTypeGroup.AddButton( m_TargetTypeGeom.GetFlButton() );

    vector< int > target_type_map;
    target_type_map.push_back( vsp::SET_TARGET );
    target_type_map.push_back( vsp::GEOM_TARGET );
    m_TargetTypeGroup.SetValMapVec( target_type_map );

    m_BorderLayout.AddYGap();

    m_BorderLayout.SetSameLineFlag( false );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddDividerBox("Boundary");

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddButton( m_BoundaryTypeNone, "None" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.AddButton( m_BoundaryTypeSet, "Set" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.AddChoice( m_BoundarySet, "", m_BorderLayout.GetButtonWidth() );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddButton( m_BoundaryTypeGeom, "Geom" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BoundaryGeom.AddExcludeType( MESH_GEOM_TYPE );
    m_BoundaryGeom.AddExcludeType( HUMAN_GEOM_TYPE );   // QUESTION THESE
    m_BoundaryGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_BoundaryGeom.AddExcludeType( WIRE_FRAME_GEOM_TYPE );
    m_BoundaryGeom.AddExcludeType( BLANK_GEOM_TYPE );
    m_BoundaryGeom.AddExcludeType( HINGE_GEOM_TYPE );
    m_BorderLayout.AddGeomPicker( m_BoundaryGeom, m_BorderLayout.GetButtonWidth() );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.ForceNewLine();

    m_BoundaryTypeGroup.Init( this );
    m_BoundaryTypeGroup.AddButton( m_BoundaryTypeNone.GetFlButton() );
    m_BoundaryTypeGroup.AddButton( m_BoundaryTypeSet.GetFlButton() );
    m_BoundaryTypeGroup.AddButton( m_BoundaryTypeGeom.GetFlButton() );

    vector< int > bndy_type_map;
    bndy_type_map.push_back( vsp::NO_BOUNDARY );
    bndy_type_map.push_back( vsp::SET_BOUNDARY );
    bndy_type_map.push_back( vsp::GEOM_BOUNDARY );
    m_BoundaryTypeGroup.SetValMapVec( bndy_type_map );

    m_BorderLayout.AddYGap();


    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddDividerBox("Direction");

    int tw = 15;
    int bw = m_BorderLayout.GetButtonWidth();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.AddButton( m_DirectionTypeVector, "Vector" );

    m_BorderLayout.SetSameLineFlag( true );

    m_BorderLayout.SetButtonWidth( tw );
    m_BorderLayout.AddButton( m_DirectionTypeX, "" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw - tw );
    m_BorderLayout.AddSlider( m_XSlider, "X", 1, "%5.3f" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetButtonWidth( tw );
    m_BorderLayout.AddButton( m_DirectionTypeY, "" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw - tw );
    m_BorderLayout.AddSlider( m_YSlider, "Y", 1, "%5.3f" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetButtonWidth( tw );
    m_BorderLayout.AddButton( m_DirectionTypeZ, "" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetButtonWidth( bw - tw );
    m_BorderLayout.AddSlider( m_ZSlider, "Z", 1, "%5.3f" );
    m_BorderLayout.ForceNewLine();

    m_BorderLayout.SetButtonWidth( bw );
    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( false );

    m_BorderLayout.AddButton( m_DirectionTypeGeom, "Geom" );
    m_BorderLayout.SetFitWidthFlag( true );
    m_DirectionGeom.AddExcludeType( MESH_GEOM_TYPE );
    m_DirectionGeom.AddExcludeType( HUMAN_GEOM_TYPE );
    m_DirectionGeom.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_DirectionGeom.AddExcludeType( WIRE_FRAME_GEOM_TYPE );
    m_DirectionGeom.AddExcludeType( BLANK_GEOM_TYPE );
    m_DirectionGeom.AddExcludeType( HINGE_GEOM_TYPE );
    m_BorderLayout.AddGeomPicker( m_DirectionGeom, m_BorderLayout.GetButtonWidth() );
    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.ForceNewLine();

    m_DirectionTypeGroup.Init( this );
    m_DirectionTypeGroup.AddButton( m_DirectionTypeX.GetFlButton() );
    m_DirectionTypeGroup.AddButton( m_DirectionTypeY.GetFlButton() );
    m_DirectionTypeGroup.AddButton( m_DirectionTypeZ.GetFlButton() );
    m_DirectionTypeGroup.AddButton( m_DirectionTypeGeom.GetFlButton() );
    m_DirectionTypeGroup.AddButton( m_DirectionTypeVector.GetFlButton() );

    vector< int > dir_type_map;
    dir_type_map.push_back( vsp::X_PROJ );
    dir_type_map.push_back( vsp::Y_PROJ );
    dir_type_map.push_back( vsp::Z_PROJ );
    dir_type_map.push_back( vsp::GEOM_PROJ );
    dir_type_map.push_back( vsp::VEC_PROJ );
    m_DirectionTypeGroup.SetValMapVec( dir_type_map );

    m_BorderLayout.AddYGap();

    // Output
    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddDividerBox( "Output File" );

    m_BorderLayout.SetSameLineFlag( true );
    m_BorderLayout.SetFitWidthFlag( true );

    m_BorderLayout.AddOutput( m_FileSelect, "File", 25 );

    m_BorderLayout.SetFitWidthFlag( false );
    m_BorderLayout.SetButtonWidth( 25 );
    m_BorderLayout.AddButton( m_FileTrigger, "..." );

    m_BorderLayout.ForceNewLine();

    m_BorderLayout.AddYGap();

    m_BorderLayout.SetButtonWidth( bw );

    m_BorderLayout.SetFitWidthFlag( true );
    m_BorderLayout.SetSameLineFlag( false );

    m_BorderLayout.AddDividerBox("Execute Projected Area");

    m_BorderLayout.AddButton(m_Execute, "START");

    m_BorderLayout.AddYGap();

    m_BorderLayout.AddOutput( m_AreaOutput, "Area" );
}

ProjectionScreen::~ProjectionScreen()
{
}

void ProjectionScreen::LoadSetChoice( Choice & c, int & index )
{
    c.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; ++i )
    {
        c.AddItem( set_name_vec[i].c_str() );
    }

    c.UpdateItems();
    c.SetVal( index );
}

void ProjectionScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void ProjectionScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

bool ProjectionScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_BoundaryTypeGroup.Update( vehiclePtr->m_BoundaryType.GetID() );
    m_TargetTypeGroup.Update( vehiclePtr->m_TargetType.GetID() );
    m_DirectionTypeGroup.Update( vehiclePtr->m_DirectionType.GetID() );

    m_BoundaryGeom.Update();
    m_TargetGeom.Update();
    m_DirectionGeom.Update();

    ProjectionMgr.SetGeomIDs( m_TargetGeom.GetGeomChoice(), m_BoundaryGeom.GetGeomChoice(), m_DirectionGeom.GetGeomChoice() );

    ProjectionMgr.UpdateDirection();


    LoadSetChoice( m_TargetSet, ProjectionMgr.m_TargetSetIndex );
    LoadSetChoice( m_BoundarySet, ProjectionMgr.m_BoundarySetIndex );

    m_XSlider.Update( vehiclePtr->m_XComp.GetID() );
    m_YSlider.Update( vehiclePtr->m_YComp.GetID() );
    m_ZSlider.Update( vehiclePtr->m_ZComp.GetID() );


    switch ( vehiclePtr->m_BoundaryType() )
    {
        case vsp::NO_BOUNDARY:
            m_BoundaryGeom.Deactivate();
            m_BoundarySet.Deactivate();
            break;
        case vsp::SET_BOUNDARY:
            m_BoundaryGeom.Deactivate();
            m_BoundarySet.Activate();
            break;
        case vsp::GEOM_BOUNDARY:
            m_BoundaryGeom.Activate();
            m_BoundarySet.Deactivate();
            break;
    }

    if ( vehiclePtr->m_TargetType() == vsp::SET_TARGET )
    {
        m_TargetGeom.Deactivate();
        m_TargetSet.Activate();
    }
    else
    {
        m_TargetGeom.Activate();
        m_TargetSet.Deactivate();
    }

    switch ( vehiclePtr->m_DirectionType() )
    {
        case vsp::X_PROJ:
        case vsp::Y_PROJ:
        case vsp::Z_PROJ:
            m_XSlider.Deactivate();
            m_YSlider.Deactivate();
            m_ZSlider.Deactivate();
            m_DirectionGeom.Deactivate();
            break;
        case vsp::GEOM_PROJ:
            m_XSlider.Deactivate();
            m_YSlider.Deactivate();
            m_ZSlider.Deactivate();
            m_DirectionGeom.Activate();
            break;
        case vsp::VEC_PROJ:
            m_XSlider.Activate();
            m_YSlider.Activate();
            m_ZSlider.Activate();
            m_DirectionGeom.Deactivate();
            break;
    }


    m_FileSelect.Update( vehiclePtr->getExportFileName( vsp::PROJ_AREA_CSV_TYPE ).c_str() );

    m_FLTK_Window->redraw();
    return false;
}

// Callback for Link Browser
void ProjectionScreen::CallBack( Fl_Widget* w )
{
    assert( m_ScreenMgr );

    m_ScreenMgr->SetUpdateFlag( true );

}

void ProjectionScreen::CloseCallBack( Fl_Widget *w )
{
    Hide();
}

// Callback for all other GUI Devices
void ProjectionScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    if ( device == &m_TargetSet )
    {
        ProjectionMgr.m_TargetSetIndex = m_TargetSet.GetVal();
    }
    else if ( device == &m_BoundarySet )
    {
        ProjectionMgr.m_BoundarySetIndex = m_BoundarySet.GetVal();
    }
    else if ( device == &m_FileTrigger  )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Choose Projected Area Output File", "*.csv" );
        vehiclePtr->setExportFileName( vsp::PROJ_AREA_CSV_TYPE, newfile );
    }
    else if ( device == &m_Execute )
    {
        Results* res = ProjectionMgr.Project();

        if ( res )
        {
            NameValData *nvd = res->FindPtr( "Area", 0 );

            if ( nvd )
            {
                double area = nvd->GetDouble( 0 );

                char str[255];
                sprintf( str, "%g", area );

                m_AreaOutput.Update( str );
            }

            string csvfn = vehiclePtr->getExportFileName( vsp::PROJ_AREA_CSV_TYPE );
            res->WriteCSVFile( csvfn );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
