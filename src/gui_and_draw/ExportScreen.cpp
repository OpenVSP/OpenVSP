//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ExportScreen.h"
#include "ScreenMgr.h"
#include "STEPOptionsScreen.h"
#include "STEPStructureOptionsScreen.h"
#include "IGESOptionsScreen.h"
#include "IGESStructureOptionsScreen.h"
#include "STLOptionsScreen.h"
#include "BEMOptionsScreen.h"
#include "DXFOptionsScreen.h"
#include "SVGOptionsScreen.h"
#include "AirfoilExportScreen.h"
#include "ModeMgr.h"

using namespace vsp;

//==== Constructor ====//
ExportScreen::ExportScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 200, 25 + 4*20 + 22*20 + 2*15 + 4*6, "Export" )
{
    m_SelectedSetIndex = DEFAULT_SET;
    m_DegenSetIndex = vsp::SET_NONE;

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_MainLayout.AddX( 5 );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddSubGroupLayout( m_GenLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() );

    int bw = 100;
    m_GenLayout.SetChoiceButtonWidth( bw );
    m_GenLayout.SetButtonWidth( bw );

    m_GenLayout.AddDividerBox( "Export Set" );
    m_GenLayout.AddYGap();

    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetChoiceButtonWidth( 0 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_SetToggle, "Normal Set:" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddChoice( m_ExportSetChoice, "", bw );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetSameLineFlag( false );
    m_GenLayout.SetChoiceButtonWidth( bw );
    m_GenLayout.AddChoice( m_DegenSetChoice, "Degen Set:" );


    m_GenLayout.SetSameLineFlag( true );
    m_GenLayout.SetChoiceButtonWidth( 0 );
    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_ModeToggle, "Mode:" );
    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.AddChoice(m_ModeChoice, "", bw );
    m_GenLayout.ForceNewLine();

    m_ModeSetToggleGroup.Init( this );
    m_ModeSetToggleGroup.AddButton( m_SetToggle.GetFlButton() );
    m_ModeSetToggleGroup.AddButton( m_ModeToggle.GetFlButton() );

    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddButton( m_Subsurfs, "Subsurfs" );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "File Format" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddButton( m_XSecButton, "XSec (.hrm)" );
    m_GenLayout.AddButton( m_Plot3DButton, "PLOT3D (.p3d)" );
    m_GenLayout.AddButton( m_STLButton, "Stereolith (.stl)" );
    m_GenLayout.AddButton( m_NASCARTButton, "NASCART (.dat)" );
    m_GenLayout.AddButton( m_TRIButton, "Cart3D (.tri)" );
    m_GenLayout.AddButton( m_VSPGeomButton, "VSPGeom (.vspgeom)" );
    m_GenLayout.AddButton( m_GMSHButton, "Gmsh (.msh)" );
    m_GenLayout.AddButton( m_POVButton, "POVRAY (.pov)" );
    m_GenLayout.AddButton( m_X3DButton, "X3D (.x3d)" );
    m_GenLayout.AddButton( m_STEPButton, "Untrimmed STEP (.stp)" );
    m_GenLayout.AddButton( m_STEPStructureButton, "Untrimmed STEP Struct (.stp)" );
    m_GenLayout.AddButton( m_IGESButton, "Untrimmed IGES (.igs)" );
    m_GenLayout.AddButton( m_IGESStructureButton, "Untrimmed IGES Struct (.igs)" );
    m_GenLayout.AddButton( m_BEMButton, "Blade Element (.bem)" );
    m_GenLayout.AddButton( m_DXFButton, "AutoCAD (.dxf)" );
    m_GenLayout.AddButton( m_SVGButton, "SVG (.svg)" );
    m_GenLayout.AddButton( m_FacetButton, "Xpatch (.facet)" );
    m_GenLayout.AddButton( m_PMARCButton, "PMARC 12 (.pmin)" );
    m_GenLayout.AddButton( m_OBJButton, "OBJ (.obj)" );
    m_GenLayout.AddButton( m_SeligAirfoilButton, "Airfoil Points (.dat)" );
    m_GenLayout.AddButton( m_BezierAirfoilButton, "Airfoil Curves (.bz)" );
    m_GenLayout.AddButton( m_CustomScriptButton, "Custom Script (.vsppart)" );

    m_Subsurfs.GetFlButton()->value( 1 );
}

//==== Update Screen ====//
bool ExportScreen::Update()
{
    BasicScreen::Update();

    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_ScreenMgr->LoadSetChoice( m_ExportSetChoice, m_DegenSetChoice, m_SelectedSetIndex, m_DegenSetIndex, true );
    m_ScreenMgr->LoadModeChoice( m_ModeChoice, m_ModeIDs, m_SelectedModeChoice );

    m_ModeSetToggleGroup.Update( vehiclePtr->m_UseModeExportFlag.GetID() );

    if ( ModeMgr.GetNumModes() == 0 )
    {
        if ( vehiclePtr->m_UseModeExportFlag() )
        {
            vehiclePtr->m_UseModeExportFlag.Set( false );
            m_ScreenMgr->SetUpdateFlag( true );
        }
        m_ModeToggle.Deactivate();
    }
    else
    {
        m_ModeToggle.Activate();
    }

    if ( vehiclePtr->m_UseModeExportFlag() )
    {
        m_ModeChoice.Activate();
        m_ExportSetChoice.Deactivate();
        m_DegenSetChoice.Deactivate();

        Mode *m = ModeMgr.GetMode( m_ModeIDs[m_SelectedModeChoice] );
        if ( m )
        {
            if ( m_SelectedSetIndex != m->m_NormalSet() ||
                 m_DegenSetIndex != m->m_DegenSet() )
            {
                m_SelectedSetIndex = m->m_NormalSet();
                m_DegenSetIndex = m->m_DegenSet();
                m_ScreenMgr->SetUpdateFlag( true );
            }
        }
    }
    else
    {
        m_ModeChoice.Deactivate();
        m_ExportSetChoice.Activate();
        m_DegenSetChoice.Activate();
    }

    return true;
}

//==== Show Screen ====//
void ExportScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}

//==== Hide Screen ====//
void ExportScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void ExportScreen::ExportFile( string &newfile, int write_set, int degen_set, int intSubsFlag, int type, bool useMode, const string &modeID )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( type == EXPORT_XSEC )
    {
        newfile = m_ScreenMgr->FileChooser( "Write XSec File?", "*.hrm", vsp::SAVE );
    }
    else if ( type == EXPORT_PLOT3D )
    {
        newfile = m_ScreenMgr->FileChooser( "Write PLOT3D File?", "*.p3d", vsp::SAVE );
    }
    else if ( type == EXPORT_STL )
    {
        if ( (( STLOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_STL_OPTIONS_SCREEN ))->ShowSTLOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write STL File?", "*.stl", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_NASCART )
    {
        newfile = m_ScreenMgr->FileChooser( "Write NASCART Files?", "*.dat", vsp::SAVE );
    }
    else if ( type == EXPORT_CART3D )
    {
        newfile = m_ScreenMgr->FileChooser( "Write Cart3D Files?", "*.tri", vsp::SAVE );
    }
    else if ( type == EXPORT_OBJ )
    {
        newfile = m_ScreenMgr->FileChooser( "Write OBJ Files?", "*.obj", vsp::SAVE );
    }
    else if ( type == EXPORT_VSPGEOM )
    {
        newfile = m_ScreenMgr->FileChooser( "Write VSPGeom Files?", "*.vspgeom", vsp::SAVE );
    }
    else if ( type == EXPORT_GMSH )
    {
        newfile = m_ScreenMgr->FileChooser( "Write GMsh Files?", "*.msh", vsp::SAVE );
    }
    else if ( type == EXPORT_POVRAY )
    {
        newfile = m_ScreenMgr->FileChooser( "Write POVRAY File?", "*.pov", vsp::SAVE );
    }
    else if ( type == EXPORT_X3D )
    {
        newfile = m_ScreenMgr->FileChooser( "Write X3D File?", "*.x3d", vsp::SAVE );
    }
    else if ( type == EXPORT_STEP )
    {
        if ( (( STEPOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_STEP_OPTIONS_SCREEN ))->ShowSTEPOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write STEP File?", "*.stp", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_STEP_STRUCTURE )
    {
        if ( (( STEPStructureOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_STEP_STRUCTURE_OPTIONS_SCREEN ))->ShowSTEPOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write STEP Structures File?", "*.stp", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_IGES )
    {
        if ( (( IGESOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_IGES_OPTIONS_SCREEN ))->ShowIGESOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write IGES File?", "*.igs", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_IGES_STRUCTURE )
    {
        if ( (( IGESStructureOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_IGES_STRUCTURE_OPTIONS_SCREEN ))->ShowIGESOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write IGES Structures File?", "*.igs", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_BEM )
    {
        if ( (( BEMOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_BEM_OPTIONS_SCREEN ))->ShowBEMOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write Blade Element File?", "*.bem", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_DXF )
    {
        if ( (( DXFOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_DXF_OPTIONS_SCREEN ))->ShowDXFOptionsScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write DXF File?", "*.dxf", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_SVG )
    {
        if ( (( SVGOptionsScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_SVG_OPTIONS_SCREEN ))->ShowSVGOptionsScreen( ) )
        {
            newfile = m_ScreenMgr->FileChooser( "Write SVG File?", "*.svg", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_FACET )
    {
        newfile = m_ScreenMgr->FileChooser( "Write Facet File?", "*.facet", vsp::SAVE );
    }

    else if ( type == EXPORT_PMARC )
    {
        newfile = m_ScreenMgr->FileChooser( "Write PMARC File?", "*.pmin", vsp::SAVE );
    }
    else if ( type == EXPORT_SELIG_AIRFOIL )
    {
        if ( ( (SeligAirfoilExportScreen*)m_ScreenMgr->GetScreen( vsp::VSP_AIRFOIL_POINTS_EXPORT_SCREEN ) )->ShowAirfoilExportScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write Airfoil Metadata File?", "*.csv", vsp::SAVE );
        }
    }
    else if ( type == EXPORT_BEZIER_AIRFOIL )
    {
        if ( ( (BezierAirfoilExportScreen*)m_ScreenMgr->GetScreen( vsp::VSP_AIRFOIL_CURVES_EXPORT_SCREEN ) )->ShowAirfoilExportScreen() )
        {
            newfile = m_ScreenMgr->FileChooser( "Write Airfoil Metadata File?", "*.csv", vsp::SAVE );
        }
    }
    else if ( type == -1 )
    {
        return;
    }

    if ( newfile.size() != 0 && newfile[ newfile.size() - 1] != '/' )
    {
        veh->ExportFile( newfile, write_set, degen_set, intSubsFlag, type, useMode, modeID );
    }

}

//==== Callbacks ====//
void ExportScreen::CallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

void ExportScreen::GuiDeviceCallBack( GuiDevice* device )
{
    string newfile;
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    int intSubsFlag = m_Subsurfs.GetFlButton()->value();

    bool useMode = veh->m_UseModeExportFlag();
    string modeID;
    if ( m_SelectedModeChoice >= 0 && m_SelectedModeChoice < m_ModeIDs.size() )
    {
        modeID = m_ModeIDs[ m_SelectedModeChoice ];
    }

    if ( device == &m_XSecButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_XSEC, useMode, modeID );
    }
    else if ( device == &m_Plot3DButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_PLOT3D, useMode, modeID );
    }
    else if (  device == &m_STLButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_STL, useMode, modeID );
    }
    else if (  device == &m_NASCARTButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_NASCART, useMode, modeID );
    }
    else if (  device == &m_TRIButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_CART3D, useMode, modeID );
    }
    else if (  device == &m_OBJButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_OBJ, useMode, modeID );
    }
    else if (  device == &m_VSPGeomButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_VSPGEOM, useMode, modeID );
    }
    else if (  device == &m_GMSHButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_GMSH, useMode, modeID );
    }
    else if (  device == &m_POVButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_POVRAY, useMode, modeID );
    }
    else if (  device == &m_X3DButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_X3D, useMode, modeID );
    }
    else if (  device == &m_STEPButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_STEP, useMode, modeID );
    }
    else if (  device == &m_STEPStructureButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_STEP_STRUCTURE, useMode, modeID );
    }
    else if ( device == &m_IGESButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_IGES, useMode, modeID );
    }
    else if ( device == &m_IGESStructureButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_IGES_STRUCTURE, useMode, modeID );
    }
    else if ( device == &m_BEMButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_BEM, useMode, modeID );
    }
    else if ( device == &m_DXFButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_DXF, useMode, modeID );
    }
    else if ( device == &m_SVGButton )
    {
        ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_SVG, useMode, modeID );
    }
    else if ( device == &m_FacetButton )
    {
        ExportFile(newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_FACET, useMode, modeID );
    }
    else if ( device == &m_PMARCButton )
    {
        ExportFile(newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_PMARC, useMode, modeID );
    }
    else if ( device == &m_SeligAirfoilButton )
    {
        if ( veh )
        {
            veh->m_AFExportType.Set( vsp::SELIG_AF_EXPORT );
            ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_SELIG_AIRFOIL, useMode, modeID );
        }
    }
    else if ( device == &m_BezierAirfoilButton )
    {
        if ( veh )
        {
            veh->m_AFExportType.Set( vsp::BEZIER_AF_EXPORT );
            ExportFile( newfile, m_SelectedSetIndex, m_DegenSetIndex, intSubsFlag, EXPORT_BEZIER_AIRFOIL, useMode, modeID );
        }
    }
    else if (  device == &m_ExportSetChoice )
    {
        m_SelectedSetIndex = m_ExportSetChoice.GetVal();

        if ( veh )
        {
            veh->m_SVGSet.Set( m_SelectedSetIndex );
        }
    }
    else if (  device == &m_DegenSetChoice )
    {
        m_DegenSetIndex = m_DegenSetChoice.GetVal();
    }
    else if ( device == &m_ModeChoice )
    {
        m_SelectedModeChoice = m_ModeChoice.GetVal();
    }
    else if ( device == &m_CustomScriptButton )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_EXPORT_CUSTOM_SCRIPT );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
