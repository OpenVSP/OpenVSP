//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "SubSurfaceMgr.h"
#include "GraphicEngine.h"
#include "Display.h"
#include "Viewport.h"
#include "Camera.h"
#include "MaterialEditScreen.h"
#include "Background.h"
#include "GraphicSingletons.h"
#include "StructureMgr.h"
#include "WingGeom.h"
#include "BORGeom.h"
#include "ParmMgr.h"

using namespace vsp;


using std::map;

//==== Constructor ====//
VspScreen::VspScreen( ScreenMgr* mgr )
{
    m_ScreenMgr = mgr;
    m_FLTK_Window = NULL;
}


//==== Destructor ====//
VspScreen::~VspScreen()
{

}

//==== Show Window ====//
void VspScreen::Show()
{
    assert( m_FLTK_Window );
    m_FLTK_Window->show();
}

//==== Is Window Shown ====//
bool VspScreen::IsShown()
{
    assert( m_FLTK_Window );
    return !!( m_FLTK_Window->shown() );
}

//==== Hide Window ====//
void VspScreen::Hide()
{
    assert( m_FLTK_Window );
    m_FLTK_Window->hide();
}

std::string VspScreen::getFeedbackGroupName()
{
    return "";
}

//=====================================================================//
//=====================================================================//
//=====================================================================//

ActionScreen::ActionScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
}

//==== Destructor ====//
ActionScreen::~ActionScreen()
{
}

//=====================================================================//
//=====================================================================//
//=====================================================================//


//==== Constructor ====//
BasicScreen::BasicScreen( ScreenMgr* mgr, int w, int h, const string & title  ) : VspScreen( mgr )
{
    //==== Window ====//
    m_FLTK_Window = new VSP_Window( w, h );
    m_FLTK_Window->resizable( m_FLTK_Window );
    VspScreen::SetFlWindow( m_FLTK_Window );

    //==== Title Box ====//
    m_FL_TitleBox = new Fl_Box( 2, 2, w - 4, 20 );
    m_FL_TitleBox->box( FL_ROUNDED_BOX );
    m_FL_TitleBox->labelfont( 1 );
    m_FL_TitleBox->labelsize( 16 );
    m_FL_TitleBox->labelcolor( FL_SELECTION_COLOR );

    SetTitle( title );

}

//==== Destructor ====//
BasicScreen::~BasicScreen()
{
    delete m_FLTK_Window;
}

//==== Set Title Name on Window And Box ====//
void BasicScreen::SetTitle( const string& title )
{
    if ( title != m_Title )
    {
        m_Title = title;
        m_FL_TitleBox->copy_label( m_Title.c_str() );
    }
}

string BasicScreen::GetTitle()
{
    return m_Title;
}

//=====================================================================//
//=====================================================================//
//=====================================================================//

//==== Constructor ====//
TabScreen::TabScreen( ScreenMgr* mgr, int w, int h, const string & title, int baseymargin, int basexmargin ) :
    BasicScreen( mgr, w, h, title )
{
    int topshift = 0;
    //==== Menu Tabs ====//
    if ( baseymargin < 0 )
    {
        baseymargin = -baseymargin;
        topshift = baseymargin;
    }

    m_MenuTabs = new Fl_Tabs( 0, 25 + topshift, w - basexmargin, h - 25 - baseymargin );
    m_MenuTabs->labelcolor( FL_BLUE );
}

//==== Destructor ====//
TabScreen::~TabScreen()
{
    for ( int i = 0; i < m_TabGroupVec.size(); i++ )
    {
        delete m_TabGroupVec[i];
    }
    m_TabGroupVec.clear();

    delete m_MenuTabs;
}

Fl_Group* TabScreen::MakeTab( const string& title )
{
    int rx, ry, rw, rh;
    m_MenuTabs->client_area( rx, ry, rw, rh, TAB_H );

    Fl_Group* grp = new Vsp_Group( rx, ry, rw, rh );
    grp->copy_label( title.c_str() );
    grp->selection_color( FL_GRAY );
    grp->labelfont( 1 );
    grp->labelcolor( FL_BLACK );
    grp->hide();
    m_TabGroupVec.push_back( grp );

    return grp;
}

//==== Add Tab ====//
Fl_Group* TabScreen::AddTab( const string& title )
{
    Fl_Group* grp = MakeTab( title );

    m_MenuTabs->add( grp );

    return grp;
}

//==== Add Tab ====//
Fl_Group* TabScreen::AddTab( const string& title, int indx )
{
    Fl_Group* grp = MakeTab( title );

    m_MenuTabs->insert( *grp, indx );

    return grp;
}

//==== Remove Tab ====//
void TabScreen::RemoveTab( Fl_Group* grp )
{
    m_MenuTabs->remove( grp );
}

//==== Add Tab ====//
void TabScreen::AddTab( Fl_Group* grp )
{
    m_MenuTabs->add( grp );
}

//==== Insert Tab ====//
void TabScreen::AddTab( Fl_Group* grp, int indx )
{
    m_MenuTabs->insert( *grp, indx );
}


//==== Get Tab At Index ====//
Fl_Group* TabScreen::GetTab( int index )
{
    if ( index >= 0 && index < ( int )m_TabGroupVec.size() )
    {
        return m_TabGroupVec[index];
    }

    return NULL;
}

//==== Add A Sub Group To Tab ====//
Fl_Group* TabScreen::AddSubGroup( Fl_Group* group, int border )
{
    if ( !group )
    {
        return NULL;
    }

    int rx, ry, rw, rh;
    m_MenuTabs->client_area( rx, ry, rw, rh, TAB_H );

    int x = rx + border;
    int y = ry + border;
    int w = rw - 2 * border;
    int h = rh - 2 * border;

    Fl_Group* sub_group = new Fl_Group( x, y, w, h );
    sub_group->show();

    group->add( sub_group );

    return sub_group;
}

//==== Add A Sub Scroll To Group ====//
Fl_Scroll* TabScreen::AddSubScroll( Fl_Group* group, int border, int lessh )
{
    if ( !group )
    {
        return NULL;
    }

    int rx, ry, rw, rh;
    m_MenuTabs->client_area( rx, ry, rw, rh, TAB_H );

    int x = rx + border;
    int y = ry + border;
    int w = rw - 2 * border;
    int h = rh - 2 * border - lessh;

    Fl_Scroll* sub_group = new Fl_Scroll( x, y, w, h );
    sub_group->show();

    group->add( sub_group );

    return sub_group;
}


//=====================================================================//
//=====================================================================//
//=====================================================================//
GeomScreen::GeomScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    TabScreen( mgr, w, h, title )
{
    // Set the window as a geom screen window
    VSP_Window* vsp_win = dynamic_cast<VSP_Window*>(m_FLTK_Window);

    vsp_win->SetGeomScreenFlag( true );

    Fl_Group* gen_tab = AddTab( "Gen" );
    Fl_Group* xform_tab = AddTab( "XForm" );
    Fl_Group* subsurf_tab = AddTab( "Sub" );
    m_SubSurfTab_ind = m_TabGroupVec.size() - 1;
    Fl_Group* gen_group = AddSubGroup( gen_tab, 5 );
    Fl_Group* xform_group = AddSubGroup( xform_tab, 5 );
    Fl_Group* subsurf_group = AddSubGroup( subsurf_tab, 5 );

    // Identify the Geom type name:
    m_GeomTypeName = title;

    //==== Gen Group Layout ====//
    m_GenLayout.SetGroupAndScreen( gen_group, this );
    m_GenLayout.AddDividerBox( "Name & Color" );
    m_GenLayout.AddInput( m_NameInput, "Name:" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddColorPicker( m_ColorPicker );
    m_GenLayout.AddYGap();

    UpdateMaterialNames();

    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( true );

    m_GenLayout.AddChoice( m_MaterialChoice, "Material:", m_GenLayout.GetButtonWidth() );

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.AddButton( m_CustomMaterialButton, "Custom" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Tessellation" );
    m_GenLayout.AddSlider( m_NumUSlider, "Num_U", 100, " %5.0f" );
    m_GenLayout.AddSlider( m_NumWSlider, "Num_W", 100, " %5.0f" );

    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Mass Properties" );

    //==== Two Columns ====//
    m_GenLayout.AddSubGroupLayout( m_Density, gen_group->w() / 2 - 2, 2 * m_GenLayout.GetStdHeight() );
    m_GenLayout.AddX( gen_group->w() / 2 + 2 );
    m_GenLayout.AddSubGroupLayout( m_Shell,   gen_group->w() / 2 - 2, 2 * m_GenLayout.GetStdHeight() );

    m_Density.AddInput( m_DensityInput, "Density", " %7.5f" );
    m_Density.AddCounter( m_PriorCounter, "Priority" );

    m_Shell.AddButton( m_ThinShellButton, "Thin Shell" );
    m_Shell.AddInput( m_ShellMassAreaInput, "Mass/Area", " %7.5f" );

    m_GenLayout.ForceNewLine();
    m_GenLayout.AddY( m_GenLayout.GetStdHeight() );
    m_GenLayout.AddYGap();

    //=== Negative Volumes ===//
    m_GenLayout.AddDividerBox( "CFDMesh Negative Volume" );

    m_GenLayout.AddButton( m_NegativeVolumeBtn, "Negative Volume" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Set Export/Analysis" );
    int remain_y = ( m_GenLayout.GetH() + m_GenLayout.GetStartY() ) - m_GenLayout.GetY();
    m_SetBrowser = m_GenLayout.AddCheckBrowser( remain_y );
    m_SetBrowser->callback( staticCB, this );

    gen_tab->show();

    //==== XForm Layout ====//
    m_XFormLayout.SetGroupAndScreen( xform_group, this );
    m_XFormLayout.AddDividerBox( "Transforms" );


    m_XFormLayout.SetFitWidthFlag( false );
    m_XFormLayout.SetSameLineFlag( true );
    m_XFormLayout.AddLabel( "Coord System:", 170 );
    m_XFormLayout.SetButtonWidth( m_XFormLayout.GetRemainX() / 2 );
    m_XFormLayout.AddButton( m_XFormRelativeToggle, "Rel" );
    m_XFormLayout.AddButton( m_XFormAbsoluteToggle, "Abs" );
    m_XFormLayout.ForceNewLine();

    m_XFormAbsRelToggle.Init( this );
    m_XFormAbsRelToggle.AddButton( m_XFormAbsoluteToggle.GetFlButton() );
    m_XFormAbsRelToggle.AddButton( m_XFormRelativeToggle.GetFlButton() );

    m_XFormLayout.SetFitWidthFlag( true );
    m_XFormLayout.SetSameLineFlag( false );
    m_XFormLayout.AddYGap();

    m_XFormLayout.SetButtonWidth( 50 );
    m_XFormLayout.AddSlider( m_XLocSlider, "XLoc", 10.0, "%7.3f" );
    m_XFormLayout.AddSlider( m_YLocSlider, "YLoc", 10.0, "%7.3f" );
    m_XFormLayout.AddSlider( m_ZLocSlider, "ZLoc", 10.0, "%7.3f" );
    m_XFormLayout.AddYGap();
    m_XFormLayout.AddSlider( m_XRotSlider, "XRot", 10.0, "%7.3f" );
    m_XFormLayout.AddSlider( m_YRotSlider, "YRot", 10.0, "%7.3f" );
    m_XFormLayout.AddSlider( m_ZRotSlider, "ZRot", 10.0, "%7.3f" );
    m_XFormLayout.AddYGap();
    m_XFormLayout.SetButtonWidth( 100 );
    m_XFormLayout.AddSlider( m_RotOriginSlider, "Rot Origin(X)", 1.0, "%5.3f" );
    m_XFormLayout.AddYGap();

    m_XFormLayout.AddDividerBox( "Symmetry" );

    m_XFormLayout.AddSubGroupLayout( m_SymmLayout, m_XFormLayout.GetW(), 4 * m_SymmLayout.GetStdHeight() + 3 * m_SymmLayout.GetGapHeight() );

    m_SymmLayout.SetFitWidthFlag( true );
    m_SymmLayout.SetSameLineFlag( true );

    m_SymmLayout.SetChoiceButtonWidth( 50 );
    m_SymmLayout.SetButtonWidth( 60 );

    m_SymmLayout.AddChoice( m_SymAncestorChoice, "About:", m_SymmLayout.GetButtonWidth() * 2 );
    m_SymmLayout.SetFitWidthFlag( false );
    m_SymmLayout.AddButton( m_SymAncestorOriginToggle, "Attach" );
    m_SymmLayout.AddButton( m_SymAncestorObjectToggle, "Object" );
    m_SymmLayout.ForceNewLine();
    m_SymmLayout.AddYGap();

    m_SymAncestorOriginObjectToggle.Init( this );
    m_SymAncestorOriginObjectToggle.AddButton( m_SymAncestorObjectToggle.GetFlButton() );
    m_SymAncestorOriginObjectToggle.AddButton( m_SymAncestorOriginToggle.GetFlButton() );

    m_SymmLayout.AddLabel( "Planar:", 74 );
    m_SymmLayout.SetButtonWidth( m_SymmLayout.GetRemainX() / 3 );
    m_SymmLayout.AddButton( m_XYSymToggle, "XY", vsp::SYM_XY );
    m_SymmLayout.AddButton( m_XZSymToggle, "XZ", vsp::SYM_XZ );
    m_SymmLayout.AddButton( m_YZSymToggle, "YZ", vsp::SYM_YZ );
    m_SymmLayout.ForceNewLine();
    m_SymmLayout.AddYGap();

    m_SymmLayout.AddLabel( "Axial:", 74 );
    m_SymmLayout.SetButtonWidth( m_SymmLayout.GetRemainX() / 4 );
    m_SymmLayout.AddButton( m_AxialNoneToggle, "None" );
    m_SymmLayout.AddButton( m_AxialXToggle, "X" );
    m_SymmLayout.AddButton( m_AxialYToggle, "Y" );
    m_SymmLayout.AddButton( m_AxialZToggle, "Z" );
    m_SymmLayout.ForceNewLine();

    m_AxialToggleGroup.Init( this );
    m_AxialToggleGroup.AddButton( m_AxialNoneToggle.GetFlButton() );
    m_AxialToggleGroup.AddButton( m_AxialXToggle.GetFlButton() );
    m_AxialToggleGroup.AddButton( m_AxialYToggle.GetFlButton() );
    m_AxialToggleGroup.AddButton( m_AxialZToggle.GetFlButton() );

    //==== Because SymAxFlag is Not 0-N Need To Map Vals ====//
    vector< int > axial_val_map;
    axial_val_map.push_back( 0 );
    axial_val_map.push_back( vsp::SYM_ROT_X );
    axial_val_map.push_back( vsp::SYM_ROT_Y );
    axial_val_map.push_back( vsp::SYM_ROT_Z );
    m_AxialToggleGroup.SetValMapVec( axial_val_map );

    m_SymmLayout.InitWidthHeightVals();
    m_SymmLayout.SetFitWidthFlag( true );
    m_SymmLayout.SetSameLineFlag( false );

    m_SymmLayout.AddSlider( m_AxialNSlider, "N", 100, " %5.0f" );

    m_SymmLayout.AddYGap();

    m_XFormLayout.AddY( m_SymmLayout.GetH() );

    m_XFormLayout.InitWidthHeightVals();
    m_XFormLayout.SetFitWidthFlag( true );
    m_XFormLayout.SetSameLineFlag( false );

    m_XFormLayout.AddDividerBox( "Scale Factor" );
    m_XFormLayout.SetFitWidthFlag( false );
    m_XFormLayout.SetSameLineFlag( true );
    m_XFormLayout.SetButtonWidth( 50 );
    m_XFormLayout.SetSliderWidth( 70 );

    m_XFormLayout.AddSlider( m_ScaleSlider, "Scale", 1, " %5.4f" );
    m_XFormLayout.SetButtonWidth( ( m_XFormLayout.GetRemainX() ) / 2 );
    m_XFormLayout.AddButton( m_ScaleResetButton, "Reset" );
    m_XFormLayout.AddButton( m_ScaleAcceptButton, "Accept" );
    m_XFormLayout.ForceNewLine();
    m_XFormLayout.AddYGap();

    m_XFormLayout.InitWidthHeightVals();
    m_XFormLayout.SetFitWidthFlag( true );
    m_XFormLayout.SetSameLineFlag( false );
    m_XFormLayout.AddDividerBox( "Attach To Parent" );

    m_XFormLayout.AddSubGroupLayout( m_AttachLayout, m_XFormLayout.GetW(), 4 * m_AttachLayout.GetStdHeight() + 3 * m_AttachLayout.GetGapHeight() );

    m_AttachLayout.SetFitWidthFlag( false );
    m_AttachLayout.SetSameLineFlag( true );

    m_AttachLayout.AddLabel( "Translate:", 74 );
    m_AttachLayout.SetButtonWidth( ( m_AttachLayout.GetRemainX() ) / 3 );
    m_AttachLayout.AddButton( m_TransNoneButton, "None" );
    m_AttachLayout.AddButton( m_TransCompButton, "Comp" );
    m_AttachLayout.AddButton( m_TransUVButton, "UW" );
    m_AttachLayout.ForceNewLine();
    m_AttachLayout.AddYGap();

    m_TransToggleGroup.Init( this );
    m_TransToggleGroup.AddButton( m_TransNoneButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransCompButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransUVButton.GetFlButton() );

    m_AttachLayout.AddLabel( "Rotate:", 74 );
    m_AttachLayout.AddButton( m_RotNoneButton, "None" );
    m_AttachLayout.AddButton( m_RotCompButton, "Comp" );
    m_AttachLayout.AddButton( m_RotUVButton, "UW" );
    m_AttachLayout.ForceNewLine();
    m_AttachLayout.AddYGap();

    m_RotToggleGroup.Init( this );
    m_RotToggleGroup.AddButton( m_RotNoneButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotCompButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotUVButton.GetFlButton() );

    m_AttachLayout.SetFitWidthFlag( true );
    m_AttachLayout.SetSameLineFlag( false );

    m_AttachLayout.AddSlider( m_AttachUSlider, "U", 1, " %5.4f" );
    m_AttachLayout.AddSlider( m_AttachVSlider, "W", 1, " %5.4f" );


    //=============== SubSurface Tab ===================//
    m_CurSubDispGroup = NULL;
    m_SubSurfLayout.SetGroupAndScreen( subsurf_group, this );
    m_SubSurfLayout.AddDividerBox( "Sub-Surface List" );

    // Initial column widths
    static int col_widths[] = { m_SubSurfLayout.GetW() / 2, m_SubSurfLayout.GetW() / 3, m_SubSurfLayout.GetW() / 6, 0 }; // 3 columns

    int browser_h = 100;
    m_SubSurfBrowser = m_SubSurfLayout.AddColResizeBrowser( col_widths, 3, browser_h );
    m_SubSurfBrowser->callback( staticScreenCB, this );

    m_SubSurfLayout.AddYGap();

    m_SubSurfLayout.AddButton( m_DelSubSurfButton, "Delete" );
    m_SubSurfLayout.AddYGap();

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_LINE ), vsp::SS_LINE );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_RECTANGLE ), vsp::SS_RECTANGLE );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_ELLIPSE ), vsp::SS_ELLIPSE );
// Only add control surface in WingScreen.
//    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL ), vsp::SS_CONTROL );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_FINITE_LINE ), vsp::SS_FINITE_LINE );

    m_SubSurfLayout.SetChoiceButtonWidth( m_SubSurfLayout.GetRemainX() / 3 );

    m_SubSurfLayout.AddChoice( m_SubSurfChoice, "Type" );
    m_SubSurfLayout.AddChoice( m_SubSurfSelectSurface, "Surface" );
    m_SubSurfLayout.AddButton( m_AddSubSurfButton, "Add" );

    m_SSCurrMainSurfIndx = 0;

    m_SubSurfLayout.AddYGap();

    m_SubSurfLayout.AddSubGroupLayout( m_SSCommonGroup, m_SubSurfLayout.GetW(), m_SubSurfLayout.GetRemainY() );

    m_SSCommonGroup.AddDividerBox( "Sub-Surface Parameters" );
    m_SSCommonGroup.AddInput( m_SubNameInput, "Name" );

    // Individual SubSurface Parameters

    //==== SSLine ====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSLineGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );

    int remain_x = m_SubSurfLayout.GetRemainX();

    m_SSLineGroup.SetFitWidthFlag( false );
    m_SSLineGroup.SetSameLineFlag( true );
    m_SSLineGroup.AddLabel( "Line Type", remain_x / 3 );
    m_SSLineGroup.SetButtonWidth( remain_x / 3 );
    m_SSLineGroup.AddButton( m_SSLineConstUButton, "U" );
    m_SSLineGroup.AddButton( m_SSLineConstWButton, "W" );

    m_SSLineConstToggleGroup.Init( this );
    m_SSLineConstToggleGroup.AddButton( m_SSLineConstUButton.GetFlButton() );
    m_SSLineConstToggleGroup.AddButton( m_SSLineConstWButton.GetFlButton() );

    m_SSLineGroup.ForceNewLine();
    m_SSLineGroup.AddLabel( "Test", remain_x / 3 );
    m_SSLineGroup.AddButton( m_SSLineGreaterToggle, "Greater" );
    m_SSLineGroup.AddButton( m_SSLineLessToggle, "Less" );

    m_SSLineTestToggleGroup.Init( this );
    m_SSLineTestToggleGroup.AddButton( m_SSLineGreaterToggle.GetFlButton() );
    m_SSLineTestToggleGroup.AddButton( m_SSLineLessToggle.GetFlButton() );

    m_SSLineGroup.SetFitWidthFlag( true );
    m_SSLineGroup.SetSameLineFlag( false );
    m_SSLineGroup.ForceNewLine();
    m_SSLineGroup.AddSlider( m_SSLineConstSlider, "Value", 1, "%5.4f" );

    //==== SSRectangle ====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSRecGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );
    remain_x = m_SSRecGroup.GetRemainX();

    m_SSRecGroup.SetFitWidthFlag( false );
    m_SSRecGroup.SetSameLineFlag( true );
    m_SSRecGroup.AddLabel( "Tag", remain_x / 3 );
    m_SSRecGroup.SetButtonWidth( remain_x / 3 );
    m_SSRecGroup.AddButton( m_SSRecInsideButton, "Inside" );
    m_SSRecGroup.AddButton( m_SSRecOutsideButton, "Outside" );

    m_SSRecTestToggleGroup.Init( this );
    m_SSRecTestToggleGroup.AddButton( m_SSRecInsideButton.GetFlButton() );
    m_SSRecTestToggleGroup.AddButton( m_SSRecOutsideButton.GetFlButton() );

    m_SSRecGroup.SetFitWidthFlag( true );
    m_SSRecGroup.SetSameLineFlag( false );
    m_SSRecGroup.ForceNewLine();

    m_SSRecGroup.AddSlider( m_SSRecCentUSlider, "Center U", 1, "%5.4f" );
    m_SSRecGroup.AddSlider( m_SSRecCentWSlider, "Center W", 1, "%5.4f" );
    m_SSRecGroup.AddSlider( m_SSRecULenSlider, "U Length", 1, "%5.4f" );
    m_SSRecGroup.AddSlider( m_SSRecWLenSlider, "W Length", 1, "%5.4f" );
    m_SSRecGroup.AddSlider( m_SSRecThetaSlider, "Theta", 25, "%5.4f" );

    //==== SS_Ellipse ====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSEllGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );
    remain_x = m_SSEllGroup.GetRemainX();

    m_SSEllGroup.SetFitWidthFlag( false );
    m_SSEllGroup.SetSameLineFlag( true );
    m_SSEllGroup.AddLabel( "Tag", remain_x / 3 );
    m_SSEllGroup.SetButtonWidth( remain_x / 3 );
    m_SSEllGroup.AddButton( m_SSEllInsideButton, "Inside" );
    m_SSEllGroup.AddButton( m_SSEllOutsideButton, "Outside" );

    m_SSEllTestToggleGroup.Init( this );
    m_SSEllTestToggleGroup.AddButton( m_SSEllInsideButton.GetFlButton() );
    m_SSEllTestToggleGroup.AddButton( m_SSEllOutsideButton.GetFlButton() );

    m_SSEllGroup.SetFitWidthFlag( true );
    m_SSEllGroup.SetSameLineFlag( false );
    m_SSEllGroup.ForceNewLine();

    m_SSEllGroup.AddSlider( m_SSEllTessSlider, "Num Points", 100, "%5.0f" );
    m_SSEllGroup.AddSlider( m_SSEllCentUSlider, "Center U", 1, "%5.4f" );
    m_SSEllGroup.AddSlider( m_SSEllCentWSlider, "Center W", 1, "%5.4f" );
    m_SSEllGroup.AddSlider( m_SSEllULenSlider, "U Length", 1, "%5.4f" );
    m_SSEllGroup.AddSlider( m_SSEllWLenSlider, "W Length", 1, "%5.4f" );
    m_SSEllGroup.AddSlider( m_SSEllThetaSlider, "Theta", 25, "%5.4f" );

    //===== SSControl ====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSConGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );
    remain_x = m_SSConGroup.GetRemainX();

    m_SSConGroup.SetFitWidthFlag(false);
    m_SSConGroup.SetSameLineFlag(true);
    m_SSConGroup.AddLabel("Tag", remain_x / 3);
    m_SSConGroup.SetButtonWidth(remain_x / 3);
    m_SSConGroup.AddButton(m_SSConInsideButton, "Inside");
    m_SSConGroup.AddButton(m_SSConOutsideButton, "Outside");

    m_SSConTestToggleGroup.Init(this);
    m_SSConTestToggleGroup.AddButton(m_SSConInsideButton.GetFlButton());
    m_SSConTestToggleGroup.AddButton(m_SSConOutsideButton.GetFlButton());

    m_SSConGroup.SetFitWidthFlag(true);
    m_SSConGroup.SetSameLineFlag(true);
    m_SSConGroup.ForceNewLine();
    m_SSConGroup.SetChoiceButtonWidth( m_SSConGroup.GetButtonWidth() );

    m_SSConSurfTypeChoice.AddItem("Upper");
    m_SSConSurfTypeChoice.AddItem("Lower");
    m_SSConSurfTypeChoice.AddItem("Both");
    m_SSConGroup.AddChoice(m_SSConSurfTypeChoice, "Upper/Lower", m_SSConGroup.GetButtonWidth() );

    m_SSConGroup.SetFitWidthFlag( false );
    m_SSConGroup.AddButton( m_SSConLEFlagButton, "Leading Edge" );
    m_SSConGroup.SetFitWidthFlag( true );

    m_SSConGroup.SetSameLineFlag(false);
    m_SSConGroup.ForceNewLine();

    m_SSConGroup.AddYGap();
    m_SSConGroup.AddDividerBox( "Spanwise" );

    m_SSConGroup.AddSlider(m_SSConUSSlider, "Start U", 1, "%5.4f");
    m_SSConGroup.AddSlider(m_SSConUESlider, "End U", 1, "%5.4f");

    m_SSConGroup.AddYGap();
    m_SSConGroup.AddDividerBox( "Chordwise" );


    m_SSConGroup.SetFitWidthFlag(false);
    m_SSConGroup.SetSameLineFlag(true);

    m_SSConGroup.SetButtonWidth( m_SSConGroup.GetW() / 3 );

    m_SSConGroup.AddButton( m_SSConSAbsButton, "Length" );
    m_SSConGroup.AddButton( m_SSConSRelButton, "Length/C" );
    m_SSConGroup.AddButton( m_SSConSEConstButton, "Constant" );

    m_SSConGroup.SetFitWidthFlag(true);
    m_SSConGroup.SetSameLineFlag(false);
    m_SSConGroup.ForceNewLine();

    m_SSConSAbsRelToggleGroup.Init( this );
    m_SSConSAbsRelToggleGroup.AddButton( m_SSConSAbsButton.GetFlButton() );
    m_SSConSAbsRelToggleGroup.AddButton( m_SSConSRelButton.GetFlButton() );

    m_SSConGroup.AddSlider( m_SSConSLenSlider, "Start Length", 10.0, "%5.4f" );
    m_SSConGroup.AddSlider( m_SSConSFracSlider, "Start Length/C", 1.0, "%5.4f");

    m_SSConGroup.AddSlider( m_SSConELenSlider, "End Length", 10.0, "%5.4f" );
    m_SSConGroup.AddSlider( m_SSConEFracSlider, "End Length/C", 1.0, "%5.4f" );

    m_SSConGroup.AddYGap();
    m_SSConGroup.AddDividerBox( "Surface End Angle" );

    m_SSConGroup.SetSameLineFlag( true );
    m_SSConGroup.SetFitWidthFlag( false );

    m_SSConGroup.AddButton( m_SSConSAngleButton, "Start" );
    m_SSConGroup.AddButton( m_SSConEAngleButton, "End" );
    m_SSConGroup.AddButton( m_SSConSameAngleButton, "Same Angle" );

    m_SSConGroup.SetSameLineFlag( false );
    m_SSConGroup.SetFitWidthFlag( true );
    m_SSConGroup.ForceNewLine();

    m_SSConGroup.AddSlider( m_SSConSAngleSlider, "Start Angle", 10.0, "%5.4f" );
    m_SSConGroup.AddSlider( m_SSConEAngleSlider, "End Angle", 10.0, "%5.4f" );

    m_SSConGroup.AddSlider( m_SSConTessSlider, "Num Points", 100, "%5.0f" );

    //==== SSFiniteLine ====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSFLineGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );
    remain_x = m_SSFLineGroup.GetRemainX();

    m_SSFLineGroup.SetFitWidthFlag( true );
    m_SSFLineGroup.SetSameLineFlag( false );
    m_SSFLineGroup.ForceNewLine();

    m_SSFLineGroup.AddSlider( m_SSFLineUStartSlider, "U Start", 1, "%5.4f" );
    m_SSFLineGroup.AddSlider( m_SSFLineUEndSlider, "U End", 1, "%5.4f" );
    m_SSFLineGroup.AddSlider( m_SSFLineWStartSlider, "W Start", 1, "%5.4f" );
    m_SSFLineGroup.AddSlider( m_SSFLineWEndSlider, "W End", 1, "%5.4f" );

    m_RotActive = true;
}

bool GeomScreen::Update()
{
    assert( m_ScreenMgr );
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return false;
    }

    char str[256];

    TabScreen::Update();

    //==== Name ===//
    m_NameInput.Update(  geom_ptr->GetName() );
    SetTitle( m_GeomTypeName + ": " + geom_ptr->GetName() );

    //==== Color ====//
    m_ColorPicker.Update( geom_ptr->GetColor() );

    //==== Material ====//
    Material * mat = geom_ptr->GetMaterial();

    UpdateMaterialNames();
    m_MaterialChoice.UpdateItems();

    m_MaterialChoice.SetVal( 0 );

    std::vector< std::string > choices = m_MaterialChoice.GetItems();
    for ( int i = 0; i < (int)choices.size(); i++ )
    {
        if( mat->m_Name == choices[i] )
        {
            m_MaterialChoice.SetVal(i);
            break;
        }
    }

    //==== XForms ====//
    m_ScaleSlider.Update( geom_ptr->m_Scale.GetID() );

    //===== Rel of Abs ====//
    m_XFormAbsRelToggle.Update( geom_ptr->m_AbsRelFlag.GetID() );
    m_XRotSlider.Activate();
    m_YRotSlider.Activate();
    m_ZRotSlider.Activate();
    if ( geom_ptr->m_AbsRelFlag() ==  vsp::REL )
    {
        m_XLocSlider.Update( 1, geom_ptr->m_XRelLoc.GetID(), geom_ptr->m_XLoc.GetID() );
        m_YLocSlider.Update( 1, geom_ptr->m_YRelLoc.GetID(), geom_ptr->m_YLoc.GetID() );
        m_ZLocSlider.Update( 1, geom_ptr->m_ZRelLoc.GetID(), geom_ptr->m_ZLoc.GetID() );
        m_XRotSlider.Update( 1, geom_ptr->m_XRelRot.GetID(), geom_ptr->m_XRot.GetID() );
        m_YRotSlider.Update( 1, geom_ptr->m_YRelRot.GetID(), geom_ptr->m_YRot.GetID() );
        m_ZRotSlider.Update( 1, geom_ptr->m_ZRelRot.GetID(), geom_ptr->m_ZRot.GetID() );
    }
    else
    {
        m_XLocSlider.Update( 2, geom_ptr->m_XRelLoc.GetID(), geom_ptr->m_XLoc.GetID() );
        m_YLocSlider.Update( 2, geom_ptr->m_YRelLoc.GetID(), geom_ptr->m_YLoc.GetID() );
        m_ZLocSlider.Update( 2, geom_ptr->m_ZRelLoc.GetID(), geom_ptr->m_ZLoc.GetID() );
        m_XRotSlider.Update( 2, geom_ptr->m_XRelRot.GetID(), geom_ptr->m_XRot.GetID() );
        m_YRotSlider.Update( 2, geom_ptr->m_YRelRot.GetID(), geom_ptr->m_YRot.GetID() );
        m_ZRotSlider.Update( 2, geom_ptr->m_ZRelRot.GetID(), geom_ptr->m_ZRot.GetID() );
    }
    m_RotOriginSlider.Update( geom_ptr->m_Origin.GetID() );

    if ( !m_RotActive )
    {
        m_XRotSlider.Deactivate();
        m_YRotSlider.Deactivate();
        m_ZRotSlider.Deactivate();
    }

    //==== Symmetry ====//
    std::vector<std::string> ancestorNames;
    ancestorNames.push_back( "GLOBAL ORIGIN" );
    geom_ptr->BuildAncestorList( ancestorNames );

    m_SymAncestorChoice.ClearItems();
    for( int i = 0; i < (int) ancestorNames.size(); i++ )
    {
        sprintf( str, "%3d %s", i, ancestorNames[i].c_str() );
        m_SymAncestorChoice.AddItem( str );
    }
    m_SymAncestorChoice.UpdateItems();
    m_SymAncestorChoice.SetVal( geom_ptr->m_SymAncestor() );

    m_SymAncestorOriginObjectToggle.Update( geom_ptr->m_SymAncestOriginFlag.GetID() );
    m_XYSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_XZSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_YZSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_AxialToggleGroup.Update( geom_ptr->m_SymAxFlag.GetID() );
    m_AxialNSlider.Update( geom_ptr->m_SymRotN.GetID() );

    //==== Mass Props ====//
    m_DensityInput.Update( geom_ptr->m_Density.GetID() );
    m_ShellMassAreaInput.Update( geom_ptr->m_MassArea.GetID() );
    m_ThinShellButton.Update( geom_ptr->m_ShellFlag.GetID() );
    m_PriorCounter.Update( geom_ptr->m_MassPrior.GetID() );

    //==== Negative Volume Props ====/
    m_NegativeVolumeBtn.Update(geom_ptr->m_NegativeVolumeFlag.GetID());

    // Disable Negative Volume for transparent surfaces.
    if ( geom_ptr->GetNumMainSurfs() > 0 && geom_ptr->GetMainCFDSurfType(0) == vsp::CFD_TRANSPARENT )
    {
        m_NegativeVolumeBtn.Deactivate();
    }
    else
    {
        m_NegativeVolumeBtn.Activate();
    }

    //==== Attachments ====//
    m_TransToggleGroup.Update( geom_ptr->m_TransAttachFlag.GetID() );
    m_RotToggleGroup.Update( geom_ptr->m_RotAttachFlag.GetID() );
    m_AttachUSlider.Update( geom_ptr->m_ULoc.GetID() );
    m_AttachVSlider.Update( geom_ptr->m_WLoc.GetID() );


    if ( geom_ptr->m_ShellFlag.Get() )
    {
        m_ShellMassAreaInput.Activate();
    }
    else
    {
        m_ShellMassAreaInput.Deactivate();
    }

    m_NumUSlider.Update( geom_ptr->m_TessU.GetID() );
    m_NumWSlider.Update( geom_ptr->m_TessW.GetID() );

    //==== Set Browser ====//
    vector< string > set_name_vec = m_ScreenMgr->GetVehiclePtr()->GetSetNameVec();
    vector< bool > set_flag_vec = geom_ptr->GetSetFlags();

    assert( set_name_vec.size() == set_flag_vec.size() );

    //==== Load Set Names and Values ====//
    m_SetBrowser->clear();
    for ( int i = SET_SHOWN ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_SetBrowser->add( set_name_vec[i].c_str(), static_cast<int>( set_flag_vec[i] ) );
    }

    //================= SubSurfaces Tab ===================//

    m_SubSurfSelectSurface.ClearItems();

    int nmain = geom_ptr->GetNumMainSurfs();
    for ( int i = 0; i < nmain; ++i )
    {
        sprintf( str, "Surf_%d", i );
        m_SubSurfSelectSurface.AddItem( str );
    }
    m_SubSurfSelectSurface.UpdateItems();

    if( m_SSCurrMainSurfIndx < 0 || m_SSCurrMainSurfIndx >= nmain )
    {
        m_SSCurrMainSurfIndx = 0;
    }
    m_SubSurfSelectSurface.SetVal( m_SSCurrMainSurfIndx );

    SubSurface* subsurf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );

    if ( subsurf )
    {
        m_SubNameInput.Update( subsurf->GetName() );
        if ( subsurf->GetType() == vsp::SS_LINE )
        {
            SSLine* ssline = dynamic_cast< SSLine* >( subsurf );
            assert( ssline );

            m_SSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
            m_SSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
            m_SSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
            SubSurfDispGroup( &m_SSLineGroup );

        }
        else if ( subsurf->GetType() == vsp::SS_RECTANGLE )
        {
            SSRectangle* ssrec = dynamic_cast< SSRectangle* >( subsurf );
            assert( subsurf );

            m_SSRecTestToggleGroup.Update( ssrec->m_TestType.GetID() );
            m_SSRecCentUSlider.Update( ssrec->m_CenterU.GetID() );
            m_SSRecCentWSlider.Update( ssrec->m_CenterW.GetID() );
            m_SSRecULenSlider.Update( ssrec->m_ULength.GetID() );
            m_SSRecWLenSlider.Update( ssrec->m_WLength.GetID() );
            m_SSRecThetaSlider.Update( ssrec->m_Theta.GetID() );
            SubSurfDispGroup( &m_SSRecGroup );
        }
        else if ( subsurf->GetType() == vsp::SS_ELLIPSE )
        {
            SSEllipse* ssell = dynamic_cast< SSEllipse* >( subsurf );
            assert( ssell );

            m_SSEllTestToggleGroup.Update( ssell->m_TestType.GetID() );
            m_SSEllTessSlider.Update( ssell->m_Tess.GetID() );
            m_SSEllCentUSlider.Update( ssell->m_CenterU.GetID() );
            m_SSEllCentWSlider.Update( ssell->m_CenterW.GetID() );
            m_SSEllULenSlider.Update( ssell->m_ULength.GetID() );
            m_SSEllWLenSlider.Update( ssell->m_WLength.GetID() );
            m_SSEllThetaSlider.Update( ssell->m_Theta.GetID() );
            SubSurfDispGroup( & m_SSEllGroup );
        }
        else if (subsurf->GetType() == vsp::SS_CONTROL)
        {
            SSControlSurf* sscon = dynamic_cast< SSControlSurf* >(subsurf);
            assert(sscon);

            m_SSConTestToggleGroup.Update(sscon->m_TestType.GetID());
            m_SSConUESlider.Update(sscon->m_UEnd.GetID());
            m_SSConUSSlider.Update(sscon->m_UStart.GetID());

            m_SSConSFracSlider.Update(sscon->m_StartLenFrac.GetID());
            m_SSConSLenSlider.Update(sscon->m_StartLength.GetID());

            m_SSConEFracSlider.Update(sscon->m_EndLenFrac.GetID());
            m_SSConELenSlider.Update(sscon->m_EndLength.GetID());

            m_SSConSAbsRelToggleGroup.Update(sscon->m_AbsRelFlag.GetID());
            m_SSConSEConstButton.Update(sscon->m_ConstFlag.GetID());

            m_SSConSAngleButton.Update( sscon->m_StartAngleFlag.GetID() );
            m_SSConEAngleButton.Update( sscon->m_EndAngleFlag.GetID() );

            m_SSConSAngleSlider.Update( sscon->m_StartAngle.GetID() );
            m_SSConEAngleSlider.Update( sscon->m_EndAngle.GetID() );

            m_SSConTessSlider.Update( sscon->m_Tess.GetID() );

            if ( sscon->m_StartAngleFlag() )
            {
                m_SSConSAngleSlider.Activate();
            }
            else
            {
                m_SSConSAngleSlider.Deactivate();
            }

            m_SSConSameAngleButton.Update( sscon->m_SameAngleFlag.GetID() );

            if ( sscon->m_StartAngleFlag() && sscon->m_EndAngleFlag() )
            {
                m_SSConSameAngleButton.Activate();
            }
            else
            {
                m_SSConSameAngleButton.Deactivate();
            }

            if ( sscon->m_EndAngleFlag() && ( !sscon->m_SameAngleFlag() || ( !sscon->m_StartAngleFlag() && sscon->m_SameAngleFlag() ) ) )
            {
                m_SSConEAngleSlider.Activate();
            }
            else
            {
                m_SSConEAngleSlider.Deactivate();
            }

            m_SSConLEFlagButton.Update(sscon->m_LEFlag.GetID());

            m_SSConSFracSlider.Deactivate();
            m_SSConSLenSlider.Deactivate();

            m_SSConEFracSlider.Deactivate();
            m_SSConELenSlider.Deactivate();

            if ( sscon->m_AbsRelFlag() == ABS )
            {
                m_SSConSLenSlider.Activate();

                if ( !sscon->m_ConstFlag() )
                {
                    m_SSConELenSlider.Activate();
                }
            }
            else
            {
                m_SSConSFracSlider.Activate();

                if ( !sscon->m_ConstFlag() )
                {
                    m_SSConEFracSlider.Activate();
                }
            }

            m_SSConSurfTypeChoice.Update(sscon->m_SurfType.GetID());
            SubSurfDispGroup(&m_SSConGroup);
        }
        else if ( subsurf->GetType() == vsp::SS_FINITE_LINE )
        {
            SSFiniteLine* ssfline = dynamic_cast< SSFiniteLine* >( subsurf );
            assert( ssfline );

            m_SSFLineUStartSlider.Update( ssfline->m_UStart.GetID() );
            m_SSFLineUEndSlider.Update( ssfline->m_UEnd.GetID() );
            m_SSFLineWStartSlider.Update( ssfline->m_WStart.GetID() );
            m_SSFLineWEndSlider.Update( ssfline->m_WEnd.GetID() );
            SubSurfDispGroup( &m_SSFLineGroup );
        }
    }
    else
    {
        SubSurfDispGroup( NULL );
    }

    //==== SubSurfBrowser ====//
    int h_pos = m_SubSurfBrowser->hposition();
    int v_pos = m_SubSurfBrowser->position();
    m_SubSurfBrowser->clear();

    m_SubSurfBrowser->column_char( ':' );

    sprintf( str, "@b@.NAME:@b@.TYPE:@b@.SURF" );
    m_SubSurfBrowser->add( str );

    string ss_name, ss_type;
    int ss_surf_ind;

    vector<SubSurface*> subsurf_vec = geom_ptr->GetSubSurfVec();
    for ( int i = 0; i < ( int )subsurf_vec.size() ; i++ )
    {

        ss_name = subsurf_vec[i]->GetName();
        ss_type = SubSurface::GetTypeName( subsurf_vec[i]->GetType() );

        if ( subsurf_vec[i]->GetType() == vsp::SS_LINE )
        {
            SSLine* ssline = dynamic_cast< SSLine* >( subsurf_vec[i] );
            assert( ssline );

            if ( ssline->m_ConstType() == vsp::CONST_U )
            {
                ss_type.append( " U" );
            }
            else
            {
                ss_type.append( " W" );
            }

            if ( ssline->m_TestType() == SSLineSeg::GT )
            {
                ss_type.append( ">" );
            }
            else
            {
                ss_type.append( "<" );
            }

            char buf[15];
            sprintf( buf, "%0.2f", ssline->m_ConstVal() );

            ss_type.append( buf );
        }

        ss_surf_ind = subsurf_vec[i]->m_MainSurfIndx.Get();
        sprintf( str, "%s:%s:Surf_%d", ss_name.c_str(), ss_type.c_str(), ss_surf_ind );
        m_SubSurfBrowser->add( str );
    }

    if ( geom_ptr->ValidSubSurfInd( SubSurfaceMgr.GetCurrSurfInd() ) )
    {
        m_SubSurfBrowser->select( SubSurfaceMgr.GetCurrSurfInd() + 2 );
    }

    m_SubSurfBrowser->hposition( h_pos );
    m_SubSurfBrowser->position( v_pos );

    return true;
}

void GeomScreen::UpdateMaterialNames()
{
    std::vector<std::string> matNames;
    matNames = MaterialMgr.GetNames();

    m_MaterialChoice.ClearItems();
    m_MaterialChoice.AddItem( "DEFAULT" );
    for( int i = 0; i < (int) matNames.size(); i++ )
    {
        m_MaterialChoice.AddItem( matNames[i] );
    }
}

void GeomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return;
    }

    if ( device == &m_ColorPicker )
    {
        vec3d c = m_ColorPicker.GetColor();
        geom_ptr->SetColor( ( int )c.x(), ( int )c.y(), ( int )c.z() );
    }
    else if ( device == &m_MaterialChoice )
    {
        int index = m_MaterialChoice.GetVal() - 1;

        Material mat;

        if( MaterialMgr.FindMaterial( index, mat ) )
        {
            geom_ptr->SetMaterial( mat.m_Name, mat.m_Ambi, mat.m_Diff, mat.m_Spec, mat.m_Emis, mat.m_Shininess );
        }
        else
        {
            geom_ptr->SetMaterialToDefault();
        }
    }
    else if ( device == &m_SymAncestorChoice )
    {
        geom_ptr->m_SymAncestor.SetFromDevice( m_SymAncestorChoice.GetVal() );
    }
    else if ( device == &m_CustomMaterialButton )
    {
        ( ( MaterialEditScreen* ) ( m_ScreenMgr->GetScreen( ScreenMgr::VSP_MATERIAL_EDIT_SCREEN ) ) )->m_OrigColor = geom_ptr->GetMaterial()->m_Name;
        geom_ptr->GetMaterial()->m_Name = "Custom";
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_MATERIAL_EDIT_SCREEN );
    }
    else if ( device == &m_ScaleAcceptButton )
    {
        geom_ptr->AcceptScale();
    }
    else if ( device == &m_ScaleResetButton )
    {
        geom_ptr->ResetScale();
    }
    else if ( device == &m_NameInput )
    {
        geom_ptr->SetName( m_NameInput.GetString() );
    }
    else if ( device == &m_AddSubSurfButton )
    {
        SubSurface* ssurf = NULL;
        ssurf = geom_ptr->AddSubSurf( m_SubSurfChoice.GetVal(), m_SSCurrMainSurfIndx );

        if ( ssurf )
        {
            ssurf->Update();
        }
        SubSurfaceMgr.SetCurrSubSurfInd( geom_ptr->NumSubSurfs() - 1 );
    }
    else if ( device == &m_DelSubSurfButton )
    {
        geom_ptr->DelSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        SubSurfaceMgr.SetCurrSubSurfInd( geom_ptr->NumSubSurfs() - 1 );
    }
    else if ( device == &m_SubNameInput )
    {
        SubSurface* sub_surf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        if ( sub_surf )
        {
            sub_surf->SetName( m_SubNameInput.GetString() );
        }
    }
    else if ( device == &m_SubSurfSelectSurface )
    {
        m_SSCurrMainSurfIndx = m_SubSurfSelectSurface.GetVal();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void GeomScreen::SubSurfDispGroup( GroupLayout* group )
{
    if ( m_CurSubDispGroup == group && group )
    {
        return;
    }

    m_SSLineGroup.Hide();
    m_SSRecGroup.Hide();
    m_SSCommonGroup.Hide();
    m_SSEllGroup.Hide();
    m_SSConGroup.Hide();
    m_SSFLineGroup.Hide();

    m_CurSubDispGroup = group;

    if ( group )
    {
        group->Show();
        m_SSCommonGroup.Show(); // Always show the Common Group if any other subsurface group is being displayed.
    }
}

void GeomScreen::CallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return;
    }

    if ( w == m_SubSurfBrowser )
    {
        SubSurfaceMgr.SetCurrSubSurfInd( m_SubSurfBrowser->value() - 2 );
        SubSurface* sub_surf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        if ( sub_surf )
        {
            m_SubSurfChoice.SetVal( sub_surf->GetType() );
            m_SSCurrMainSurfIndx = sub_surf->m_MainSurfIndx();
        }
    }
    else if ( w == m_SetBrowser )
    {
        int curr_index = m_SetBrowser->value();
        bool flag = !!m_SetBrowser->checked( curr_index );

        geom_ptr->SetSetFlag( curr_index, flag );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//=====================================================================//
//=====================================================================//
//=====================================================================//
XSecScreen::XSecScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    GeomScreen( mgr, w, h, title )
{
    m_XSecTab = AddTab( "XSec" );
    Fl_Group* xsec_group = AddSubGroup( m_XSecTab, 5 );

    m_XSecLayout.SetGroupAndScreen( xsec_group, this );

    m_XSecDivider = m_XSecLayout.AddDividerBox( "Cross Section" );

    m_XSecLayout.AddIndexSelector( m_XSecIndexSelector );
    m_XSecLayout.AddYGap();

    // Differences in XSec Geom GUIs are applied between here and AddXSecLayout()
    m_XSecDriversActive = true;
}

void XSecScreen::AddXSecLayout(bool include_point_type)
{
    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );

    m_XSecLayout.AddDividerBox( "Type" );

    if (include_point_type) {
        m_XSecTypeChoice.AddItem( "POINT", XS_POINT );
    }
    m_XSecTypeChoice.AddItem( "CIRCLE", XS_CIRCLE );
    m_XSecTypeChoice.AddItem( "ELLIPSE", XS_ELLIPSE );
    m_XSecTypeChoice.AddItem( "SUPER_ELLIPSE", XS_SUPER_ELLIPSE );
    m_XSecTypeChoice.AddItem( "ROUNDED_RECTANGLE", XS_ROUNDED_RECTANGLE );
    m_XSecTypeChoice.AddItem( "GENERAL_FUSE", XS_GENERAL_FUSE );
    m_XSecTypeChoice.AddItem( "FUSE_FILE", XS_FILE_FUSE );
    m_XSecTypeChoice.AddItem( "FOUR_SERIES", XS_FOUR_SERIES );
    m_XSecTypeChoice.AddItem( "SIX_SERIES", XS_SIX_SERIES );
    m_XSecTypeChoice.AddItem( "BICONVEX", XS_BICONVEX );
    m_XSecTypeChoice.AddItem( "WEDGE", XS_WEDGE );
    m_XSecTypeChoice.AddItem( "EDIT_CURVE", XS_EDIT_CURVE );
    m_XSecTypeChoice.AddItem( "AF_FILE", XS_FILE_AIRFOIL );
    m_XSecTypeChoice.AddItem( "CST_AIRFOIL", XS_CST_AIRFOIL );
    m_XSecTypeChoice.AddItem( "KARMAN_TREFFTZ", XS_VKT_AIRFOIL );
    m_XSecTypeChoice.AddItem( "FOUR_DIGIT_MOD", XS_FOUR_DIGIT_MOD );
    m_XSecTypeChoice.AddItem( "FIVE_DIGIT", XS_FIVE_DIGIT );
    m_XSecTypeChoice.AddItem( "FIVE_DIGIT_MOD", XS_FIVE_DIGIT_MOD );
    m_XSecTypeChoice.AddItem( "16_SERIES", XS_ONE_SIX_SERIES );

    m_XSecLayout.SetChoiceButtonWidth( 100 ); // TODO: Get appropriate width
    int show_w = 50;
    int convert_w = 100;

    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.AddChoice( m_XSecTypeChoice, "Choose Type:", (show_w + convert_w) );
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetButtonWidth( show_w );
    m_XSecLayout.AddButton( m_ShowXSecButton, "Show" );

    m_XSecLayout.AddSubGroupLayout( m_ConvertCEDITGroup, m_XSecLayout.GetW(), m_XSecLayout.GetStdHeight() );
    m_ConvertCEDITGroup.SetButtonWidth( convert_w );
    m_ConvertCEDITGroup.SetFitWidthFlag( false );
    m_ConvertCEDITGroup.AddButton( m_ConvertCEDITButton, "Convert CEDIT" );

    m_XSecLayout.AddSubGroupLayout( m_EditCEDITGroup, m_XSecLayout.GetW(), m_XSecLayout.GetStdHeight() );
    m_EditCEDITGroup.SetFitWidthFlag( false );
    m_EditCEDITGroup.SetButtonWidth( convert_w );
    m_EditCEDITGroup.AddButton( m_EditCEDITButton, "Edit Curve" );

    m_XSecLayout.ForceNewLine();

    m_XSecLayout.InitWidthHeightVals();
    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );

    m_XSecLayout.AddYGap();

    vector < string > xsec_driver_labels;
    xsec_driver_labels.resize( vsp::NUM_XSEC_DRIVER );
    xsec_driver_labels[vsp::WIDTH_XSEC_DRIVER] = "Width";
    xsec_driver_labels[vsp::AREA_XSEC_DRIVER] = "Area";
    xsec_driver_labels[vsp::HEIGHT_XSEC_DRIVER] = "Height";
    xsec_driver_labels[vsp::HWRATIO_XSEC_DRIVER] = "H/W Ratio";

    vector < string > circ_xsec_driver_labels;
    circ_xsec_driver_labels.resize( vsp::CIRCLE_NUM_XSEC_DRIVER );
    circ_xsec_driver_labels[vsp::WIDTH_XSEC_DRIVER] = "Diameter";
    circ_xsec_driver_labels[vsp::AREA_XSEC_DRIVER] = "Area";

    //==== Super XSec ====//
    m_XSecLayout.AddSubGroupLayout( m_SuperGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    if (m_XSecDriversActive)
    {
        m_SuperXSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
        m_SuperGroup.AddDriverGroupBank( m_SuperXSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_SuperGroup.AddSlider( m_SuperHeightSlider, "Height", 10, "%6.5f" );
        m_SuperGroup.AddSlider( m_SuperWidthSlider, "Width", 10, "%6.5f" );
    }

    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMaxWidthLocSlider, "MaxWLoc", 2, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddSlider( m_SuperMSlider, "M", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperNSlider, "N", 10, "%6.5f" );
    m_SuperGroup.AddYGap();
    m_SuperGroup.AddButton( m_SuperToggleSym, "T/B Symmetric Exponents" );
    m_SuperGroup.AddSlider( m_SuperM_botSlider, "M Bottom", 10, "%6.5f" );
    m_SuperGroup.AddSlider( m_SuperN_botSlider, "N Bottom", 10, "%6.5f" );

    //==== Circle XSec ====//
    m_XSecLayout.AddSubGroupLayout( m_CircleGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    if (m_XSecDriversActive)
    {
        m_CircleXSecDriverGroupBank.SetDriverGroup( &m_CircleXSecDriverGroup );
        m_CircleGroup.AddDriverGroupBank( m_CircleXSecDriverGroupBank, circ_xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_CircleGroup.AddSlider( m_DiameterSlider, "Diameter", 10, "%6.5f" );
    }

    //==== Ellipse XSec ====//
    m_XSecLayout.AddSubGroupLayout( m_EllipseGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    if (m_XSecDriversActive)
    {
        m_EllipseXSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
        m_EllipseGroup.AddDriverGroupBank( m_EllipseXSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_EllipseGroup.AddSlider( m_EllipseHeightSlider, "Height", 10, "%6.5f" );
        m_EllipseGroup.AddSlider( m_EllipseWidthSlider, "Width", 10, "%6.5f" );
    }

    //==== Rounded Rect ====//
    m_XSecLayout.AddSubGroupLayout( m_RoundedRectGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    if (m_XSecDriversActive)
    {
        m_RRXSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
        m_RoundedRectGroup.AddDriverGroupBank( m_RRXSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_RoundedRectGroup.AddSlider( m_RRHeightSlider, "Height", 10, "%6.5f" );
        m_RoundedRectGroup.AddSlider( m_RRWidthSlider, "Width", 10, "%6.5f" );
    }

    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddSlider( m_RRSkewSlider, "Skew", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRVSkewSlider, "VSkew", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRKeystoneSlider, "Keystone", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();

    m_RoundedRectGroup.SetSameLineFlag( true );
    m_RoundedRectGroup.SetFitWidthFlag( false );
    m_RoundedRectGroup.SetButtonWidth( m_RoundedRectGroup.GetRemainX() / 5 );

    m_RoundedRectGroup.AddLabel( "Symmetry:", m_RoundedRectGroup.GetRemainX() / 5 );
    m_RoundedRectGroup.AddButton( m_RRRadNoSymToggle, "None" );
    m_RoundedRectGroup.AddButton( m_RRRadRLSymToggle, "R//L" );
    m_RoundedRectGroup.AddButton( m_RRRadTBSymToggle, "T//B" );
    m_RoundedRectGroup.AddButton( m_RRRadAllSymToggle, "All" );

    m_RRRadSymRadioGroup.Init( this );
    m_RRRadSymRadioGroup.AddButton( m_RRRadNoSymToggle.GetFlButton() );
    m_RRRadSymRadioGroup.AddButton( m_RRRadRLSymToggle.GetFlButton() );
    m_RRRadSymRadioGroup.AddButton( m_RRRadTBSymToggle.GetFlButton() );
    m_RRRadSymRadioGroup.AddButton( m_RRRadAllSymToggle.GetFlButton() );

    m_RoundedRectGroup.ForceNewLine();
    m_RoundedRectGroup.SetSameLineFlag( false );
    m_RoundedRectGroup.SetFitWidthFlag( true );

    m_RoundedRectGroup.AddSlider( m_RRRadiusTRSlider, "TR Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRRadiusTLSlider, "TL Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRRadiusBLSlider, "BL Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddSlider( m_RRRadiusBRSlider, "BR Radius", 10, "%6.5f" );
    m_RoundedRectGroup.AddYGap();
    m_RoundedRectGroup.AddButton( m_RRKeyCornerButton, "Key Corner" );

    //==== General Fuse XSec ====//
    m_XSecLayout.AddSubGroupLayout( m_GenGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    if (m_XSecDriversActive)
    {
        m_GenXSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
        m_GenGroup.AddDriverGroupBank( m_GenXSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_GenGroup.AddSlider( m_GenHeightSlider, "Height", 10, "%6.5f" );
        m_GenGroup.AddSlider( m_GenWidthSlider, "Width", 10, "%6.5f" );
    }

    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenMaxWidthLocSlider, "MaxWLoc", 1, "%6.5f" );
    m_GenGroup.AddSlider( m_GenCornerRadSlider, "CornerRad", 1, "%6.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenTopTanAngleSlider, "TopTanAng", 90, "%7.5f" );
    m_GenGroup.AddSlider( m_GenBotTanAngleSlider, "BotTanAng", 90, "%7.5f" );
    m_GenGroup.AddYGap();
    m_GenGroup.AddSlider( m_GenTopStrSlider, "TopStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenBotStrSlider, "BotStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenUpStrSlider, "UpStr", 1, "%7.5f" );
    m_GenGroup.AddSlider( m_GenLowStrSlider, "LowStr", 1, "%7.5f" );

    //==== Four Series AF ====//
    m_XSecLayout.AddSubGroupLayout( m_FourSeriesGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_FourSeriesGroup.AddOutput( m_FourNameOutput, "Name" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddSlider( m_FourChordSlider, "Chord", 10, "%7.3f" );
    m_FourSeriesGroup.AddSlider( m_FourThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();

    int actionToggleButtonWidth = 15;
    int actionSliderButtonWidth = m_FourSeriesGroup.GetButtonWidth() - actionToggleButtonWidth;

    m_FourSeriesGroup.SetSameLineFlag( true );

    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourSeriesGroup.AddButton( m_FourCamberButton, "" );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddSlider( m_FourCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FourSeriesGroup.ForceNewLine();

    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourSeriesGroup.AddButton( m_FourCLiButton, "" );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddSlider( m_FourCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FourSeriesGroup.ForceNewLine();

    m_FourSeriesGroup.SetSameLineFlag( false );
    m_FourSeriesGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FourCamberGroup.Init( this );
    m_FourCamberGroup.AddButton( m_FourCamberButton.GetFlButton() );
    m_FourCamberGroup.AddButton( m_FourCLiButton.GetFlButton() );

    vector< int > camb_val_map;
    camb_val_map.push_back( vsp::MAX_CAMB );
    camb_val_map.push_back( vsp::DESIGN_CL );
    m_FourCamberGroup.SetValMapVec( camb_val_map );

    m_FourSeriesGroup.AddSlider( m_FourCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourInvertButton, "Invert Airfoil" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.AddButton( m_FourSharpTEButton, "Sharpen TE" );
    m_FourSeriesGroup.AddYGap();
    m_FourSeriesGroup.SetSameLineFlag( true );
    m_FourSeriesGroup.SetFitWidthFlag( false );
    m_FourSeriesGroup.SetButtonWidth( 125 );
    m_FourSeriesGroup.AddButton( m_FourFitCSTButton, "Fit CST" );
    m_FourSeriesGroup.InitWidthHeightVals();
    m_FourSeriesGroup.SetFitWidthFlag( true );
    m_FourSeriesGroup.AddCounter( m_FourDegreeCounter, "Degree", 125 );

    //==== Six Series AF ====//
    m_XSecLayout.AddSubGroupLayout( m_SixSeriesGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_SixSeriesGroup.AddOutput( m_SixNameOutput, "Name" );
    m_SixSeriesGroup.AddYGap();

    m_SixSeriesChoice.AddItem( "63" );
    m_SixSeriesChoice.AddItem( "64" );
    m_SixSeriesChoice.AddItem( "65" );
    m_SixSeriesChoice.AddItem( "66" );
    m_SixSeriesChoice.AddItem( "67" );
    m_SixSeriesChoice.AddItem( "63a" );
    m_SixSeriesChoice.AddItem( "64a" );
    m_SixSeriesChoice.AddItem( "65a" );
    m_SixSeriesGroup.AddChoice( m_SixSeriesChoice, "Series" );

    m_SixSeriesGroup.AddYGap();

    m_SixSeriesGroup.AddSlider( m_SixChordSlider, "Chord", 10, "%7.3f" );
    m_SixSeriesGroup.AddSlider( m_SixThickChordSlider, "T/C", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddSlider( m_SixIdealClSlider, "Ideal CL", 1, "%7.5f" );
    m_SixSeriesGroup.AddSlider( m_SixASlider, "A", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddButton( m_SixInvertButton, "Invert Airfoil" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.SetSameLineFlag( true );
    m_SixSeriesGroup.SetFitWidthFlag( false );
    m_SixSeriesGroup.SetButtonWidth( 125 );
    m_SixSeriesGroup.AddButton( m_SixFitCSTButton, "Fit CST" );
    m_SixSeriesGroup.InitWidthHeightVals();
    m_SixSeriesGroup.SetFitWidthFlag( true );
    m_SixSeriesGroup.AddCounter( m_SixDegreeCounter, "Degree", 125 );

    //==== Biconvex AF ====//
    m_XSecLayout.AddSubGroupLayout( m_BiconvexGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_BiconvexGroup.AddSlider( m_BiconvexChordSlider, "Chord", 10, "%7.3f" );
    m_BiconvexGroup.AddSlider( m_BiconvexThickChordSlider, "T/C", 1, "%7.5f" );

    //==== Wedge AF ====//
    m_XSecLayout.AddSubGroupLayout( m_WedgeGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_WedgeGroup.AddSlider( m_WedgeChordSlider, "Chord", 10, "%7.3f" );
    m_WedgeGroup.AddSlider( m_WedgeThickChordSlider, "T/C", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeThickLocSlider, "Thick X", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeFlatUpSlider, "Flat Up", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddButton( m_WedgeSymmThickButton, "Symm Thickness" );
    m_WedgeGroup.AddSlider( m_WedgeThickLocLowSlider, "Thick X Low", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeFlatLowSlider, "Flat Low", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeZCamberSlider, "Camber", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddSlider( m_WedgeUForeUpSlider, "U Fwd Up", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeUForeLowSlider, "U Fwd Low", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeDuUpSlider, "dU Flat Up", 1, "%7.5f" );
    m_WedgeGroup.AddSlider( m_WedgeDuLowSlider, "dU Flat Low", 1, "%7.5f" );
    m_WedgeGroup.AddYGap();
    m_WedgeGroup.AddButton( m_WedgeInvertButton, "Invert Airfoil" );

    //==== Fuse File ====//
    m_XSecLayout.AddSubGroupLayout( m_FuseFileGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_FuseFileGroup.AddButton( m_ReadFuseFileButton, "Read File" );
    m_FuseFileGroup.AddYGap();

    if (m_XSecDriversActive)
    {
        m_FuseFileXSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
        m_FuseFileGroup.AddDriverGroupBank( m_FuseFileXSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_FuseFileGroup.AddSlider( m_FileHeightSlider, "Height", 10, "%7.3f" );
        m_FuseFileGroup.AddSlider( m_FileWidthSlider, "Width", 10, "%7.3f" );
    }

    //==== Airfoil File ====//
    m_XSecLayout.AddSubGroupLayout( m_AfFileGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_AfFileGroup.AddButton( m_AfReadFileButton, "Read File" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddOutput( m_AfFileNameOutput, "Name" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddSlider( m_AfFileChordSlider, "Chord", 10, "%7.3f" );
    m_AfFileGroup.AddSlider( m_AfFileThickChordSlider, "T/C", 1, "%7.5f" );
    m_AfFileGroup.AddOutput( m_AfFileBaseThickChordOutput, "Base T/C", "%7.5f" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.AddButton( m_AfFileInvertButton, "Invert Airfoil" );
    m_AfFileGroup.AddYGap();
    m_AfFileGroup.SetSameLineFlag( true );
    m_AfFileGroup.SetFitWidthFlag( false );
    m_AfFileGroup.SetButtonWidth( 125 );
    m_AfFileGroup.AddButton( m_AfFileFitCSTButton, "Fit CST" );
    m_AfFileGroup.InitWidthHeightVals();
    m_AfFileGroup.SetFitWidthFlag( true );
    m_AfFileGroup.AddCounter( m_AfFileDegreeCounter, "Degree", 125 );

    //==== CST Airfoil ====//
    m_XSecLayout.AddSubGroupLayout( m_CSTAirfoilGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    m_CSTAirfoilGroup.AddOutput( m_CSTThickChordOutput, "T/C", "%7.5f" );

    m_CSTAirfoilGroup.AddButton( m_CSTContLERadButton, "Enforce Continuous LE Radius" );
    m_CSTAirfoilGroup.AddButton( m_CSTInvertButton, "Invert Airfoil" );

    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddSlider( m_CSTChordSlider, "Chord", 10, "%7.3f" ); // TODO: Check why Prop and Wing do not have CST chord

    m_CSTAirfoilGroup.AddYGap();
    m_CSTAirfoilGroup.AddDividerBox( "Upper Surface" );

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTAirfoilGroup.AddOutput( m_UpDegreeOutput, "Degree", m_CSTAirfoilGroup.GetButtonWidth() * 2 );
    m_CSTAirfoilGroup.SetFitWidthFlag( false );
    m_CSTAirfoilGroup.AddButton( m_UpDemoteButton, "Demote" );
    m_CSTAirfoilGroup.AddButton( m_UpPromoteButton, "Promote" );

    m_CSTAirfoilGroup.ForceNewLine();

    m_CSTAirfoilGroup.SetSameLineFlag( false );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTUpCoeffScroll = m_CSTAirfoilGroup.AddFlScroll( 60 );

    m_CSTUpCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CSTUpCoeffScroll->box( FL_BORDER_BOX );
    m_CSTUpCoeffLayout.SetGroupAndScreen( m_CSTUpCoeffScroll, this );

    m_CSTAirfoilGroup.AddYGap();

    m_CSTAirfoilGroup.AddDividerBox( "Lower Surface" );

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTAirfoilGroup.AddOutput( m_LowDegreeOutput, "Degree", m_CSTAirfoilGroup.GetButtonWidth() * 2 );
    m_CSTAirfoilGroup.SetFitWidthFlag( false );
    m_CSTAirfoilGroup.AddButton( m_LowDemoteButton, "Demote" );
    m_CSTAirfoilGroup.AddButton( m_LowPromoteButton, "Promote" );

    m_CSTAirfoilGroup.ForceNewLine();

    m_CSTAirfoilGroup.SetSameLineFlag( false );
    m_CSTAirfoilGroup.SetFitWidthFlag( true );

    m_CSTLowCoeffScroll = m_CSTAirfoilGroup.AddFlScroll( 60 );
    m_CSTLowCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_CSTLowCoeffScroll->box( FL_BORDER_BOX );
    m_CSTLowCoeffLayout.SetGroupAndScreen( m_CSTLowCoeffScroll, this );

    //==== VKT AF ====//
    m_XSecLayout.AddSubGroupLayout( m_VKTGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_VKTGroup.AddSlider( m_VKTChordSlider, "Chord", 10, "%7.3f" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddSlider( m_VKTEpsilonSlider, "Epsilon", 1, "%7.5f" );
    m_VKTGroup.AddSlider( m_VKTKappaSlider, "Kappa", 1, "%7.5f" );
    m_VKTGroup.AddSlider( m_VKTTauSlider, "Tau", 10, "%7.5f" );
    m_VKTGroup.AddOutput( m_VKTThickChordOutput, "T/C", "%7.5f" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.AddButton( m_VKTInvertButton, "Invert Airfoil" );
    m_VKTGroup.AddYGap();
    m_VKTGroup.SetSameLineFlag( true );
    m_VKTGroup.SetFitWidthFlag( false );
    m_VKTGroup.SetButtonWidth( 125 );
    m_VKTGroup.AddButton( m_VKTFitCSTButton, "Fit CST" );
    m_VKTGroup.InitWidthHeightVals();
    m_VKTGroup.SetFitWidthFlag( true );
    m_VKTGroup.AddCounter( m_VKTDegreeCounter, "Degree", 125 );

    //==== Four Series AF ====//
    m_XSecLayout.AddSubGroupLayout( m_FourDigitModGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_FourDigitModGroup.AddOutput( m_FourModNameOutput, "Name" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddSlider( m_FourModChordSlider, "Chord", 10, "%7.3f" );
    m_FourDigitModGroup.AddSlider( m_FourModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FourDigitModGroup.AddYGap();

    m_FourDigitModGroup.SetSameLineFlag( true );

    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourDigitModGroup.AddButton( m_FourModCamberButton, "" );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddSlider( m_FourModCamberSlider, "Camber", 0.2, "%7.5f" );

    m_FourDigitModGroup.ForceNewLine();

    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_FourDigitModGroup.AddButton( m_FourModCLiButton, "" );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddSlider( m_FourModCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_FourDigitModGroup.ForceNewLine();

    m_FourDigitModGroup.SetSameLineFlag( false );
    m_FourDigitModGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_FourModCamberGroup.Init( this );
    m_FourModCamberGroup.AddButton( m_FourModCamberButton.GetFlButton() );
    m_FourModCamberGroup.AddButton( m_FourModCLiButton.GetFlButton() );

    m_FourModCamberGroup.SetValMapVec( camb_val_map );

    m_FourDigitModGroup.AddSlider( m_FourModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddSlider( m_FourModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FourDigitModGroup.AddSlider( m_FourModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddButton( m_FourModInvertButton, "Invert Airfoil" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.AddButton( m_FourModSharpTEButton, "Sharpen TE" );
    m_FourDigitModGroup.AddYGap();
    m_FourDigitModGroup.SetSameLineFlag( true );
    m_FourDigitModGroup.SetFitWidthFlag( false );
    m_FourDigitModGroup.SetButtonWidth( 125 );
    m_FourDigitModGroup.AddButton( m_FourModFitCSTButton, "Fit CST" );
    m_FourDigitModGroup.InitWidthHeightVals();
    m_FourDigitModGroup.SetFitWidthFlag( true );
    m_FourDigitModGroup.AddCounter( m_FourModDegreeCounter, "Degree", 125 );

    //==== Five Digit AF ====//
    m_XSecLayout.AddSubGroupLayout( m_FiveDigitGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_FiveDigitGroup.AddOutput( m_FiveNameOutput, "Name" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddSlider( m_FiveChordSlider, "Chord", 10, "%7.3f" );
    m_FiveDigitGroup.AddSlider( m_FiveThickChordSlider, "T/C", 1, "%7.5f" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddSlider( m_FiveCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FiveDigitGroup.AddSlider( m_FiveCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddButton( m_FiveInvertButton, "Invert Airfoil" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.AddButton( m_FiveSharpTEButton, "Sharpen TE" );
    m_FiveDigitGroup.AddYGap();
    m_FiveDigitGroup.SetSameLineFlag( true );
    m_FiveDigitGroup.SetFitWidthFlag( false );
    m_FiveDigitGroup.SetButtonWidth( 125 );
    m_FiveDigitGroup.AddButton( m_FiveFitCSTButton, "Fit CST" );
    m_FiveDigitGroup.InitWidthHeightVals();
    m_FiveDigitGroup.SetFitWidthFlag( true );
    m_FiveDigitGroup.AddCounter( m_FiveDegreeCounter, "Degree", 125 );

    //==== Five Digit Mod AF ====//
    m_XSecLayout.AddSubGroupLayout( m_FiveDigitModGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_FiveDigitModGroup.AddOutput( m_FiveModNameOutput, "Name" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModChordSlider, "Chord", 10, "%7.3f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModThickChordSlider, "T/C", 1, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddSlider( m_FiveModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_FiveDigitModGroup.AddSlider( m_FiveModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddButton( m_FiveModInvertButton, "Invert Airfoil" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.AddButton( m_FiveModSharpTEButton, "Sharpen TE" );
    m_FiveDigitModGroup.AddYGap();
    m_FiveDigitModGroup.SetSameLineFlag( true );
    m_FiveDigitModGroup.SetFitWidthFlag( false );
    m_FiveDigitModGroup.SetButtonWidth( 125 );
    m_FiveDigitModGroup.AddButton( m_FiveModFitCSTButton, "Fit CST" );
    m_FiveDigitModGroup.InitWidthHeightVals();
    m_FiveDigitModGroup.SetFitWidthFlag( true );
    m_FiveDigitModGroup.AddCounter( m_FiveModDegreeCounter, "Degree", 125 );

    //==== 16 Series AF ====//
    m_XSecLayout.AddSubGroupLayout( m_OneSixSeriesGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );
    m_OneSixSeriesGroup.AddOutput( m_OneSixSeriesNameOutput, "Name" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesChordSlider, "Chord", 10, "%7.3f" );
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesThickChordSlider, "T/C", 1, "%7.5f" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddSlider( m_OneSixSeriesCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesInvertButton, "Invert Airfoil" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesSharpTEButton, "Sharpen TE" );
    m_OneSixSeriesGroup.AddYGap();
    m_OneSixSeriesGroup.SetSameLineFlag( true );
    m_OneSixSeriesGroup.SetFitWidthFlag( false );
    m_OneSixSeriesGroup.SetButtonWidth( 125 );
    m_OneSixSeriesGroup.AddButton( m_OneSixSeriesFitCSTButton, "Fit CST" );
    m_OneSixSeriesGroup.InitWidthHeightVals();
    m_OneSixSeriesGroup.SetFitWidthFlag( true );
    m_OneSixSeriesGroup.AddCounter( m_OneSixSeriesDegreeCounter, "Degree", 125 );

    DisplayGroup( &m_PointGroup );
}

//==== Update Pod Screen ====//
bool XSecScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if (!geom_ptr)
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    //==== XSec Index Display ===//
    GeomXSec* geomxsec_ptr = dynamic_cast<GeomXSec*>(geom_ptr);
    assert( geomxsec_ptr );
    // Note: BOR requires it's own Update because it is not a GeomXSec

    int xsid = geomxsec_ptr->m_ActiveXSec();
    m_XSecIndexSelector.Update( geomxsec_ptr->m_ActiveXSec.GetID() );

    XSec* xs = geomxsec_ptr->GetXSec( xsid );
    if (xs)
    {
        XSecCurve* xsc = xs->GetXSecCurve();
        if (xsc)
        {
            m_XSecTypeChoice.SetVal( xsc->GetType() );

            if (xsc->GetType() == XS_POINT)
            {
                DisplayGroup( NULL );
            }
            else if (xsc->GetType() == XS_SUPER_ELLIPSE)
            {
                DisplayGroup( &m_SuperGroup );

                SuperXSec* super_xs = dynamic_cast<SuperXSec*>(xsc);
                assert( super_xs );

                if (m_XSecDriversActive)
                {
                    m_SuperXSecDriverGroupBank.SetDriverGroup( super_xs->m_DriverGroup );
                    vector< string > parm_ids = super_xs->GetDriverParms();
                    m_SuperXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_SuperHeightSlider.Update( super_xs->m_Height.GetID() );
                    m_SuperWidthSlider.Update( super_xs->m_Width.GetID() );
                }

                m_SuperMSlider.Update( super_xs->m_M.GetID() );
                m_SuperNSlider.Update( super_xs->m_N.GetID() );
                m_SuperToggleSym.Update( super_xs->m_TopBotSym.GetID() );
                m_SuperM_botSlider.Update( super_xs->m_M_bot.GetID() );
                m_SuperN_botSlider.Update( super_xs->m_N_bot.GetID() );
                m_SuperMaxWidthLocSlider.Update( super_xs->m_MaxWidthLoc.GetID() );

                if (super_xs->m_TopBotSym())
                {
                    m_SuperM_botSlider.Deactivate();
                    m_SuperN_botSlider.Deactivate();
                }
                else if (!super_xs->m_TopBotSym())
                {
                    m_SuperM_botSlider.Activate();
                    m_SuperN_botSlider.Activate();
                }
            }
            else if (xsc->GetType() == XS_CIRCLE)
            {
                DisplayGroup( &m_CircleGroup );

                CircleXSec* circle_xs = dynamic_cast<CircleXSec*>(xsc);
                assert( circle_xs );
                
                if (m_XSecDriversActive)
                {
                    m_CircleXSecDriverGroupBank.SetDriverGroup( circle_xs->m_DriverGroup );
                    vector< string > parm_ids = circle_xs->GetDriverParms();
                    m_CircleXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_DiameterSlider.Update( circle_xs->m_Diameter.GetID() );
                }
            }
            else if (xsc->GetType() == XS_ELLIPSE)
            {
                DisplayGroup( &m_EllipseGroup );

                EllipseXSec* ellipse_xs = dynamic_cast<EllipseXSec*>(xsc);

                if (m_XSecDriversActive)
                {
                    m_EllipseXSecDriverGroupBank.SetDriverGroup( ellipse_xs->m_DriverGroup );
                    vector< string > parm_ids = ellipse_xs->GetDriverParms();
                    m_EllipseXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_EllipseHeightSlider.Update( ellipse_xs->m_Height.GetID() );
                    m_EllipseWidthSlider.Update( ellipse_xs->m_Width.GetID() );
                }
            }
            else if (xsc->GetType() == XS_ROUNDED_RECTANGLE)
            {
                DisplayGroup( &m_RoundedRectGroup );
                RoundedRectXSec* rect_xs = dynamic_cast<RoundedRectXSec*>(xsc);
                assert( rect_xs );
                
                if (m_XSecDriversActive)
                {
                    m_RRXSecDriverGroupBank.SetDriverGroup( rect_xs->m_DriverGroup );
                    vector< string > parm_ids = rect_xs->GetDriverParms();
                    m_RRXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_RRHeightSlider.Update( rect_xs->m_Height.GetID() );
                    m_RRWidthSlider.Update( rect_xs->m_Width.GetID() );
                }

                m_RRRadSymRadioGroup.Update( rect_xs->m_RadiusSymmetryType.GetID() );
                m_RRRadiusBRSlider.Update( rect_xs->m_RadiusBR.GetID() );
                m_RRRadiusBLSlider.Update( rect_xs->m_RadiusBL.GetID() );
                m_RRRadiusTLSlider.Update( rect_xs->m_RadiusTL.GetID() );
                m_RRRadiusTRSlider.Update( rect_xs->m_RadiusTR.GetID() );
                m_RRKeyCornerButton.Update( rect_xs->m_KeyCornerParm.GetID() );
                m_RRSkewSlider.Update( rect_xs->m_Skew.GetID() );
                m_RRKeystoneSlider.Update( rect_xs->m_Keystone.GetID() );
                m_RRVSkewSlider.Update( rect_xs->m_VSkew.GetID() );

                if (rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_NONE)
                {
                    m_RRRadiusBRSlider.Activate();
                    m_RRRadiusBLSlider.Activate();
                    m_RRRadiusTLSlider.Activate();
                }
                else if (rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_RL)
                {
                    m_RRRadiusBRSlider.Activate();
                    m_RRRadiusBLSlider.Deactivate();
                    m_RRRadiusTLSlider.Deactivate();
                }
                else if (rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_TB)
                {
                    m_RRRadiusBRSlider.Deactivate();
                    m_RRRadiusTLSlider.Activate();
                    m_RRRadiusBLSlider.Deactivate();
                }
                else if (rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_ALL)
                {
                    m_RRRadiusBRSlider.Deactivate();
                    m_RRRadiusBLSlider.Deactivate();
                    m_RRRadiusTLSlider.Deactivate();
                }
            }
            else if (xsc->GetType() == XS_GENERAL_FUSE)
            {
                DisplayGroup( &m_GenGroup );
                GeneralFuseXSec* gen_xs = dynamic_cast<GeneralFuseXSec*>(xsc);
                assert( gen_xs );
                
                if (m_XSecDriversActive)
                {
                    m_GenXSecDriverGroupBank.SetDriverGroup( gen_xs->m_DriverGroup );
                    vector< string > parm_ids = gen_xs->GetDriverParms();
                    m_GenXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_GenHeightSlider.Update( gen_xs->m_Height.GetID() );
                    m_GenWidthSlider.Update( gen_xs->m_Width.GetID() );
                }

                m_GenMaxWidthLocSlider.Update( gen_xs->m_MaxWidthLoc.GetID() );
                m_GenCornerRadSlider.Update( gen_xs->m_CornerRad.GetID() );
                m_GenTopTanAngleSlider.Update( gen_xs->m_TopTanAngle.GetID() );
                m_GenBotTanAngleSlider.Update( gen_xs->m_BotTanAngle.GetID() );
                m_GenTopStrSlider.Update( gen_xs->m_TopStr.GetID() );
                m_GenBotStrSlider.Update( gen_xs->m_BotStr.GetID() );
                m_GenUpStrSlider.Update( gen_xs->m_UpStr.GetID() );
                m_GenLowStrSlider.Update( gen_xs->m_LowStr.GetID() );
            }
            else if (xsc->GetType() == XS_FOUR_SERIES)
            {
                DisplayGroup( &m_FourSeriesGroup );
                FourSeries* fs_xs = dynamic_cast<FourSeries*>(xsc);
                assert( fs_xs );

                m_FourChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FourThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FourCamberSlider.Update( fs_xs->m_Camber.GetID() );
                m_FourCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FourCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );

                if (geom_ptr->GetType().m_Type == PROP_GEOM_TYPE)
                {
                    m_FourCamberSlider.Deactivate();
                    m_FourCLiSlider.Deactivate();
                }
                if (fs_xs->m_CamberInputFlag() == MAX_CAMB)
                {
                    m_FourCamberSlider.Activate();
                    m_FourCLiSlider.Deactivate();
                }
                else
                {
                    m_FourCamberSlider.Deactivate();
                    m_FourCLiSlider.Activate();
                }
                m_FourCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FourInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FourNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FourSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                m_FourDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_SIX_SERIES)
            {
                DisplayGroup( &m_SixSeriesGroup );
                SixSeries* ss_xs = dynamic_cast<SixSeries*>(xsc);
                assert( ss_xs );

                m_SixChordSlider.Update( ss_xs->m_Chord.GetID() );
                m_SixThickChordSlider.Update( ss_xs->m_ThickChord.GetID() );
                m_SixIdealClSlider.Update( ss_xs->m_IdealCl.GetID() );
                m_SixASlider.Update( ss_xs->m_A.GetID() );

                m_SixInvertButton.Update( ss_xs->m_Invert.GetID() );
                m_SixNameOutput.Update( ss_xs->GetAirfoilName() );
                m_SixSeriesChoice.Update( ss_xs->m_Series.GetID() );
                m_SixDegreeCounter.Update( ss_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_BICONVEX)
            {
                DisplayGroup( &m_BiconvexGroup );
                Biconvex* bi_xs = dynamic_cast<Biconvex*>(xsc);
                assert( bi_xs );

                m_BiconvexChordSlider.Update( bi_xs->m_Chord.GetID() );
                m_BiconvexThickChordSlider.Update( bi_xs->m_ThickChord.GetID() );
            }
            else if (xsc->GetType() == XS_WEDGE)
            {
                DisplayGroup( &m_WedgeGroup );
                Wedge* we_xs = dynamic_cast<Wedge*>(xsc);
                assert( we_xs );

                m_WedgeChordSlider.Update( we_xs->m_Chord.GetID() );
                m_WedgeThickChordSlider.Update( we_xs->m_ThickChord.GetID() );
                m_WedgeThickLocSlider.Update( we_xs->m_ThickLoc.GetID() );
                m_WedgeZCamberSlider.Update( we_xs->m_ZCamber.GetID() );
                m_WedgeSymmThickButton.Update( we_xs->m_SymmThick.GetID() );
                m_WedgeThickLocLowSlider.Update( we_xs->m_ThickLocLow.GetID() );
                m_WedgeFlatUpSlider.Update( we_xs->m_FlatUp.GetID() );
                m_WedgeFlatLowSlider.Update( we_xs->m_FlatLow.GetID() );
                m_WedgeUForeUpSlider.Update( we_xs->m_UForeUp.GetID() );
                m_WedgeUForeLowSlider.Update( we_xs->m_UForeLow.GetID() );
                m_WedgeDuUpSlider.Update( we_xs->m_DuUp.GetID() );
                m_WedgeDuLowSlider.Update( we_xs->m_DuLow.GetID() );
                m_WedgeInvertButton.Update( we_xs->m_Invert.GetID() );

                if (we_xs->m_SymmThick())
                {
                    m_WedgeThickLocLowSlider.Deactivate();
                    m_WedgeFlatLowSlider.Deactivate();
                }
            }
            else if (xsc->GetType() == XS_FILE_FUSE)
            {
                DisplayGroup( &m_FuseFileGroup );
                FileXSec* file_xs = dynamic_cast<FileXSec*>(xsc);
                assert( file_xs );

                if (m_XSecDriversActive)
                {
                    m_FuseFileXSecDriverGroupBank.SetDriverGroup( file_xs->m_DriverGroup );
                    vector< string > parm_ids = file_xs->GetDriverParms();
                    m_FuseFileXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_FileHeightSlider.Update( file_xs->m_Height.GetID() );
                    m_FileWidthSlider.Update( file_xs->m_Width.GetID() );
                }
            }
            else if (xsc->GetType() == XS_FILE_AIRFOIL)
            {
                DisplayGroup( &m_AfFileGroup );
                FileAirfoil* affile_xs = dynamic_cast<FileAirfoil*>(xsc);
                assert( affile_xs );

                m_AfFileChordSlider.Update( affile_xs->m_Chord.GetID() );
                m_AfFileThickChordSlider.Update( affile_xs->m_ThickChord.GetID() );
                m_AfFileBaseThickChordOutput.Update( affile_xs->m_BaseThickness.GetID() );
                m_AfFileInvertButton.Update( affile_xs->m_Invert.GetID() );
                m_AfFileNameOutput.Update( affile_xs->GetAirfoilName() );
                m_AfFileDegreeCounter.Update( affile_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_CST_AIRFOIL)
            {
                DisplayGroup( &m_CSTAirfoilGroup );
                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>(xsc);
                assert( cst_xs );

                int num_up = cst_xs->m_UpDeg() + 1;
                int num_low = cst_xs->m_LowDeg() + 1;

                char str[255];
                sprintf( str, "%d", cst_xs->m_UpDeg() );
                m_UpDegreeOutput.Update( str );
                sprintf( str, "%d", cst_xs->m_LowDeg() );
                m_LowDegreeOutput.Update( str );

                m_CSTChordSlider.Update( cst_xs->m_Chord.GetID() );
                m_CSTInvertButton.Update( cst_xs->m_Invert.GetID() );
                m_CSTContLERadButton.Update( cst_xs->m_ContLERad.GetID() );
                m_CSTThickChordOutput.Update( cst_xs->m_ThickChord.GetID() );

                if ((m_UpCoeffSliderVec.size() != num_up) || (m_LowCoeffSliderVec.size() != num_low))
                {
                    RebuildCSTGroup( cst_xs );
                }

                for (int i = 0; i < num_up; i++)
                {
                    Parm *p = cst_xs->m_UpCoeffParmVec[i];
                    if (p)
                    {
                        m_UpCoeffSliderVec[i].Update( p->GetID() );
                    }
                }

                for (int i = 0; i < num_low; i++)
                {
                    Parm *p = cst_xs->m_LowCoeffParmVec[i];
                    if (p)
                    {
                        m_LowCoeffSliderVec[i].Update( p->GetID() );
                    }
                }

                if (cst_xs->m_ContLERad() && num_low > 0)
                {
                    m_LowCoeffSliderVec[0].Deactivate();
                }
            }
            else if (xsc->GetType() == XS_VKT_AIRFOIL)
            {
                DisplayGroup( &m_VKTGroup );
                VKTAirfoil* vkt_xs = dynamic_cast<VKTAirfoil*>(xsc);
                assert( vkt_xs );

                m_VKTChordSlider.Update( vkt_xs->m_Chord.GetID() );
                m_VKTEpsilonSlider.Update( vkt_xs->m_Epsilon.GetID() );
                m_VKTKappaSlider.Update( vkt_xs->m_Kappa.GetID() );
                m_VKTTauSlider.Update( vkt_xs->m_Tau.GetID() );
                m_VKTInvertButton.Update( vkt_xs->m_Invert.GetID() );
                m_VKTDegreeCounter.Update( vkt_xs->m_FitDegree.GetID() );
                m_VKTThickChordOutput.Update( vkt_xs->m_ThickChord.GetID() );
            }
            else if (xsc->GetType() == XS_FOUR_DIGIT_MOD)
            {
                DisplayGroup( &m_FourDigitModGroup );
                FourDigMod* fs_xs = dynamic_cast<FourDigMod*>(xsc);
                assert( fs_xs );

                m_FourModChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FourModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FourModCamberSlider.Update( fs_xs->m_Camber.GetID() );
                m_FourModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FourModCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                if (fs_xs->m_CamberInputFlag() == MAX_CAMB)
                {
                    m_FourModCamberSlider.Activate();
                    m_FourModCLiSlider.Deactivate();
                }
                else
                {
                    m_FourModCamberSlider.Deactivate();
                    m_FourModCLiSlider.Activate();
                }
                m_FourModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FourModInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FourModNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FourModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                m_FourModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                m_FourModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                m_FourModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_FIVE_DIGIT)
            {
                DisplayGroup( &m_FiveDigitGroup );
                FiveDig* fs_xs = dynamic_cast<FiveDig*>(xsc);
                assert( fs_xs );

                m_FiveChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FiveThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FiveCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FiveCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FiveInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FiveNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FiveSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                m_FiveDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_FIVE_DIGIT_MOD)
            {
                DisplayGroup( &m_FiveDigitModGroup );
                FiveDigMod* fs_xs = dynamic_cast<FiveDigMod*>(xsc);
                assert( fs_xs );

                m_FiveModChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_FiveModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_FiveModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_FiveModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                m_FiveModInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_FiveModNameOutput.Update( fs_xs->GetAirfoilName() );
                m_FiveModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                m_FiveModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                m_FiveModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                m_FiveModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_ONE_SIX_SERIES)
            {
                DisplayGroup( &m_OneSixSeriesGroup );
                OneSixSeries* fs_xs = dynamic_cast<OneSixSeries*>(xsc);
                assert( fs_xs );

                m_OneSixSeriesChordSlider.Update( fs_xs->m_Chord.GetID() );
                m_OneSixSeriesThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                m_OneSixSeriesCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                m_OneSixSeriesInvertButton.Update( fs_xs->m_Invert.GetID() );
                m_OneSixSeriesNameOutput.Update( fs_xs->GetAirfoilName() );
                m_OneSixSeriesSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                m_OneSixSeriesDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
            }
            else if (xsc->GetType() == XS_EDIT_CURVE)
            {
                m_EditCEDITGroup.Show();
                m_ConvertCEDITGroup.Hide();
                DisplayGroup( NULL );
            }

            if (xsc->GetType() != XS_EDIT_CURVE)
            {
                m_EditCEDITGroup.Hide();
                m_ConvertCEDITGroup.Show();
            }
        }
    }

    GeomScreen::Update();


    return true;
}

void XSecScreen::DisplayGroup( GroupLayout* group )
{
    if (m_CurrDisplayGroup == group)
    {
        return;
    }

    m_SuperGroup.Hide();
    m_CircleGroup.Hide();
    m_EllipseGroup.Hide();
    m_RoundedRectGroup.Hide();
    m_GenGroup.Hide();
    m_FourSeriesGroup.Hide();
    m_SixSeriesGroup.Hide();
    m_BiconvexGroup.Hide();
    m_WedgeGroup.Hide();
    m_FuseFileGroup.Hide();
    m_AfFileGroup.Hide();
    m_CSTAirfoilGroup.Hide();
    m_VKTGroup.Hide();
    m_FourDigitModGroup.Hide();
    m_FiveDigitGroup.Hide();
    m_FiveDigitModGroup.Hide();
    m_OneSixSeriesGroup.Hide();

    m_CurrDisplayGroup = group;

    if (group)
    {
        group->Show();
    }
}

//==== Fltk  Callbacks ====//
void XSecScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void XSecScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if (!geom_ptr)
    {
        return;
    }
    GeomXSec* xsec_geom_ptr = dynamic_cast<GeomXSec*>(geom_ptr);
    assert( xsec_geom_ptr );
    // Note: BOR requires it's own GuiDeviceCallBack because it is not a GeomXSec

    if (gui_device == &m_XSecTypeChoice)
    {
        int t = m_XSecTypeChoice.GetVal();
        xsec_geom_ptr->SetActiveXSecType( t );

        if (t == XS_EDIT_CURVE)
        {
            m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CURVE_EDIT_SCREEN );
        }
    }
    else if (gui_device == &m_ShowXSecButton)
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_XSEC_SCREEN );
    }
    else if (gui_device == &m_ConvertCEDITButton)
    {
        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            EditCurveXSec* edit_xsec = xs->ConvertToEdit();

            if (edit_xsec)
            {
                m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CURVE_EDIT_SCREEN );
            }

            // Deactivate width and/or height parms for Prop and Wing
            if (geom_ptr->GetType().m_Type == PROP_GEOM_TYPE || geom_ptr->GetType().m_Type == MS_WING_GEOM_TYPE)
            {
                // Deactivate width Parm without updating entire surface
                string width_id = edit_xsec->GetWidthParmID();
                Parm* width_parm = ParmMgr.FindParm( width_id );

                if (width_parm)
                {
                    width_parm->Deactivate();
                }
            }
            if (geom_ptr->GetType().m_Type == PROP_GEOM_TYPE)
            {
                string height_id = edit_xsec->GetHeightParmID();
                Parm* height_parm = ParmMgr.FindParm( height_id );

                if (height_parm)
                {
                    height_parm->Deactivate();
                }
            }
        }
    }
    else if (gui_device == &m_EditCEDITButton)
    {
        m_ScreenMgr->ShowScreen( ScreenMgr::VSP_CURVE_EDIT_SCREEN );
    }
    else if (gui_device == &m_ReadFuseFileButton)
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                if (xsc->GetType() == XS_FILE_FUSE)
                {
                    FileXSec* file_xs = dynamic_cast<FileXSec*>(xsc);
                    assert( file_xs );
                    string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Fuselage Cross Section", "*.fxs" );

                    file_xs->ReadXsecFile( newfile );
                    file_xs->Update();
                    xs->Update();
                    xsec_geom_ptr->Update();
                }
            }
        }
    }
    else if (gui_device == &m_AfReadFileButton)
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                if (xsc->GetType() == XS_FILE_AIRFOIL)
                {
                    FileAirfoil* affile_xs = dynamic_cast<FileAirfoil*>(xsc);
                    assert( affile_xs );
                    string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Airfoil File", "*.{af,dat}", false );

                    affile_xs->ReadFile( newfile );
                    affile_xs->Update();
                    xs->Update();
                    xsec_geom_ptr->Update();
                }
            }
        }
    }
    else if (gui_device == &m_UpPromoteButton)
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                if (xsc->GetType() == XS_CST_AIRFOIL)
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>(xsc);
                    assert( cst_xs );

                    cst_xs->PromoteUpper();
                    cst_xs->Update();
                    xs->Update();
                    xsec_geom_ptr->Update();
                }
            }
        }
    }
    else if (gui_device == &m_LowPromoteButton)
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                if (xsc->GetType() == XS_CST_AIRFOIL)
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>(xsc);
                    assert( cst_xs );

                    cst_xs->PromoteLower();
                    cst_xs->Update();
                    xs->Update();
                    xsec_geom_ptr->Update();
                }
            }
        }

    }
    else if (gui_device == &m_UpDemoteButton)
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                if (xsc->GetType() == XS_CST_AIRFOIL)
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>(xsc);
                    assert( cst_xs );

                    cst_xs->DemoteUpper();
                    cst_xs->Update();
                    xs->Update();
                    xsec_geom_ptr->Update();
                }
            }
        }

    }
    else if (gui_device == &m_LowDemoteButton)
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                if (xsc->GetType() == XS_CST_AIRFOIL)
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>(xsc);
                    assert( cst_xs );

                    cst_xs->DemoteLower();
                    cst_xs->Update();
                    xs->Update();
                    xsec_geom_ptr->Update();
                }
            }
        }

    }
    else if ((gui_device == &m_FourFitCSTButton) ||
        (gui_device == &m_SixFitCSTButton) ||
        (gui_device == &m_AfFileFitCSTButton) ||
        (gui_device == &m_VKTFitCSTButton) ||
        (gui_device == &m_FourModFitCSTButton) ||
        (gui_device == &m_FiveFitCSTButton) ||
        (gui_device == &m_FiveModFitCSTButton) ||
        (gui_device == &m_OneSixSeriesFitCSTButton))
    {
        int xsid = xsec_geom_ptr->m_ActiveXSec();
        XSec* xs = xsec_geom_ptr->GetXSec( xsid );
        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if (xsc)
            {
                Airfoil* af_xs = dynamic_cast<Airfoil*>(xsc);

                if (af_xs)
                {
                    VspCurve c = af_xs->GetOrigCurve();
                    int deg = af_xs->m_FitDegree();

                    xsec_geom_ptr->SetActiveXSecType( XS_CST_AIRFOIL );

                    XSec* newxs = xsec_geom_ptr->GetXSec( xsid );
                    if (newxs)
                    {
                        XSecCurve* newxsc = newxs->GetXSecCurve();
                        if (newxsc)
                        {
                            if (newxsc->GetType() == XS_CST_AIRFOIL)
                            {
                                CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>(newxsc);
                                assert( cst_xs );

                                cst_xs->FitCurve( c, deg );

                                cst_xs->Update();
                                newxs->Update();
                                xsec_geom_ptr->Update();
                            }
                        }
                    }
                }
            }
        }
    }

    GeomScreen::GuiDeviceCallBack( gui_device );
}

void XSecScreen::RebuildCSTGroup( CSTAirfoil* cst_xs )
{
    if (!cst_xs)
    {
        return;
    }

    if (!m_CSTUpCoeffScroll || !m_CSTLowCoeffScroll)
    {
        return;
    }

    m_CSTUpCoeffScroll->clear();
    m_CSTUpCoeffLayout.SetGroup( m_CSTUpCoeffScroll );
    m_CSTUpCoeffLayout.InitWidthHeightVals();

    m_UpCoeffSliderVec.clear();

    unsigned int num_up = cst_xs->m_UpDeg() + 1;

    m_UpCoeffSliderVec.resize( num_up );

    for (int i = 0; i < num_up; i++)
    {
        m_CSTUpCoeffLayout.AddSlider( m_UpCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }




    m_CSTLowCoeffScroll->clear();
    m_CSTLowCoeffLayout.SetGroup( m_CSTLowCoeffScroll );
    m_CSTLowCoeffLayout.InitWidthHeightVals();

    m_LowCoeffSliderVec.clear();

    unsigned int num_low = cst_xs->m_LowDeg() + 1;

    m_LowCoeffSliderVec.resize( num_low );


    for (int i = 0; i < num_low; i++)
    {
        m_CSTLowCoeffLayout.AddSlider( m_LowCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }
}

//=====================================================================//
//=====================================================================//
//=====================================================================//
SkinScreen::SkinScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    XSecScreen( mgr, w, h, title )
{
    const char* angleFmt = "%5.2f";
    const char* strengthFmt = "%5.2f";
    const char* curveFmt = "%5.2f";

    const double angleRng = 90;
    const double strengthRng = 5;
    const double curveRng = 5;

    Fl_Group* skin_tab = AddTab( "Skinning" );
    Fl_Group* skin_group = AddSubGroup( skin_tab, 5 );

    m_SkinLayout.SetGroupAndScreen( skin_group, this );

    m_SkinLayout.AddDividerBox( "Skin Cross Section" );

    m_SkinLayout.AddIndexSelector( m_SkinIndexSelector );


    m_SkinLayout.AddYGap();

    m_SkinLayout.SetButtonWidth( 75 );

    int oldDH = m_SkinLayout.GetDividerHeight();
    m_SkinLayout.SetDividerHeight( m_SkinLayout.GetStdHeight() );

    m_SkinLayout.AddYGap();

    m_SkinLayout.SetSameLineFlag( true );
    m_SkinLayout.AddDividerBox( "Top Side", m_SkinLayout.GetButtonWidth() );
    m_SkinLayout.SetFitWidthFlag( false );
    m_SkinLayout.AddButton( m_AllSymButton, "All Sym" );
    m_SkinLayout.ForceNewLine();
    m_SkinLayout.SetFitWidthFlag( true );
    m_SkinLayout.SetSameLineFlag( false );

    m_SkinLayout.SetChoiceButtonWidth( 55 );
    m_SkinLayout.SetInputWidth( 45 );
    m_SkinLayout.SetSliderWidth( 50 );
    m_SkinLayout.AddSkinHeader( m_TopHeader );

    m_SkinLayout.AddSkinControl( m_TopAngleSkinControl, "Angle", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_TopSlewSkinControl, "Slew", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_TopStrengthSkinControl, "Strength", strengthRng, strengthFmt);
    m_SkinLayout.AddSkinControl( m_TopCurvatureSkinControl, "Curvature", curveRng, curveFmt);

    m_SkinLayout.AddYGap();
    m_SkinLayout.AddDividerBox( "Right Side" );

    m_SkinLayout.AddSkinHeader( m_RightHeader );
    m_SkinLayout.AddSkinControl( m_RightAngleSkinControl, "Angle", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_RightSlewSkinControl, "Slew", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_RightStrengthSkinControl, "Strength", strengthRng, strengthFmt);
    m_SkinLayout.AddSkinControl( m_RightCurvatureSkinControl, "Curvature", curveRng, curveFmt);

    m_SkinLayout.AddYGap();
    m_SkinLayout.SetSameLineFlag( true );
    m_SkinLayout.AddDividerBox( "Bottom Side", m_SkinLayout.GetButtonWidth() );
    m_SkinLayout.SetFitWidthFlag( false );
    m_SkinLayout.AddButton( m_TBSymButton, "T/B Sym" );
    m_SkinLayout.ForceNewLine();
    m_SkinLayout.SetFitWidthFlag( true );
    m_SkinLayout.SetSameLineFlag( false );

    m_SkinLayout.AddSkinHeader( m_BottomHeader );
    m_SkinLayout.AddSkinControl( m_BottomAngleSkinControl, "Angle", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_BottomSlewSkinControl, "Slew", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_BottomStrengthSkinControl, "Strength", strengthRng, strengthFmt);
    m_SkinLayout.AddSkinControl( m_BottomCurvatureSkinControl, "Curvature", curveRng, curveFmt);

    m_SkinLayout.AddYGap();
    m_SkinLayout.SetSameLineFlag( true );
    m_SkinLayout.AddDividerBox( "Left Side", m_SkinLayout.GetButtonWidth() );
    m_SkinLayout.SetFitWidthFlag( false );
    m_SkinLayout.AddButton( m_RLSymButton, "R/L Sym" );
    m_SkinLayout.ForceNewLine();
    m_SkinLayout.SetFitWidthFlag( true );
    m_SkinLayout.SetSameLineFlag( false );

    m_SkinLayout.AddSkinHeader( m_LeftHeader );
    m_SkinLayout.AddSkinControl( m_LeftAngleSkinControl, "Angle", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_LeftSlewSkinControl, "Slew", angleRng, angleFmt);
    m_SkinLayout.AddSkinControl( m_LeftStrengthSkinControl, "Strength", strengthRng, strengthFmt);
    m_SkinLayout.AddSkinControl( m_LeftCurvatureSkinControl, "Curvature", curveRng, curveFmt);

    m_SkinLayout.SetDividerHeight( oldDH );
}


//==== Update Pod Screen ====//
bool SkinScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return false;
    }

    XSecScreen::Update();

    GeomXSec* geomxsec_ptr = dynamic_cast< GeomXSec* >( geom_ptr );
    assert( geomxsec_ptr );


    //==== Skin & XSec Index Display ===//
    int xsid = geomxsec_ptr->m_ActiveXSec();
    m_SkinIndexSelector.Update( geomxsec_ptr->m_ActiveXSec.GetID() );

    SkinXSec* xs = ( SkinXSec* ) geomxsec_ptr->GetXSec( xsid );
    if ( xs )
    {
        //==== Skin ====//
        // Update Symmetry flags to Parms.
        m_AllSymButton.Update( xs->m_AllSymFlag.GetID() );
        m_TBSymButton.Update( xs->m_TBSymFlag.GetID() );
        m_RLSymButton.Update( xs->m_RLSymFlag.GetID() );

        // Update Controls to Parms.
        m_TopHeader.m_ContChoice->SetVal( xs->m_TopCont() );
        m_TopAngleSkinControl.Update( xs->m_TopLAngle.GetID(), xs->m_TopLAngleSet.GetID(), xs->m_TopLRAngleEq.GetID(), xs->m_TopRAngleSet.GetID(), xs->m_TopRAngle.GetID() );
        m_TopSlewSkinControl.Update( xs->m_TopLSlew.GetID(), xs->m_TopLSlewSet.GetID(), xs->m_TopLRSlewEq.GetID(), xs->m_TopRSlewSet.GetID(), xs->m_TopRSlew.GetID() );
        m_TopStrengthSkinControl.Update( xs->m_TopLStrength.GetID(), xs->m_TopLStrengthSet.GetID(), xs->m_TopLRStrengthEq.GetID(), xs->m_TopRStrengthSet.GetID(), xs->m_TopRStrength.GetID() );
        m_TopCurvatureSkinControl.Update( xs->m_TopLCurve.GetID(), xs->m_TopLCurveSet.GetID(), xs->m_TopLRCurveEq.GetID(), xs->m_TopRCurveSet.GetID(), xs->m_TopRCurve.GetID() );

        m_RightHeader.m_ContChoice->SetVal( xs->m_RightCont() );
        m_RightAngleSkinControl.Update( xs->m_RightLAngle.GetID(), xs->m_RightLAngleSet.GetID(), xs->m_RightLRAngleEq.GetID(), xs->m_RightRAngleSet.GetID(), xs->m_RightRAngle.GetID() );
        m_RightSlewSkinControl.Update( xs->m_RightLSlew.GetID(), xs->m_RightLSlewSet.GetID(), xs->m_RightLRSlewEq.GetID(), xs->m_RightRSlewSet.GetID(), xs->m_RightRSlew.GetID() );
        m_RightStrengthSkinControl.Update( xs->m_RightLStrength.GetID(), xs->m_RightLStrengthSet.GetID(), xs->m_RightLRStrengthEq.GetID(), xs->m_RightRStrengthSet.GetID(), xs->m_RightRStrength.GetID() );
        m_RightCurvatureSkinControl.Update( xs->m_RightLCurve.GetID(), xs->m_RightLCurveSet.GetID(), xs->m_RightLRCurveEq.GetID(), xs->m_RightRCurveSet.GetID(), xs->m_RightRCurve.GetID() );

        m_BottomHeader.m_ContChoice->SetVal( xs->m_BottomCont() );
        m_BottomAngleSkinControl.Update( xs->m_BottomLAngle.GetID(), xs->m_BottomLAngleSet.GetID(), xs->m_BottomLRAngleEq.GetID(), xs->m_BottomRAngleSet.GetID(), xs->m_BottomRAngle.GetID() );
        m_BottomSlewSkinControl.Update( xs->m_BottomLSlew.GetID(), xs->m_BottomLSlewSet.GetID(), xs->m_BottomLRSlewEq.GetID(), xs->m_BottomRSlewSet.GetID(), xs->m_BottomRSlew.GetID() );
        m_BottomStrengthSkinControl.Update( xs->m_BottomLStrength.GetID(), xs->m_BottomLStrengthSet.GetID(), xs->m_BottomLRStrengthEq.GetID(), xs->m_BottomRStrengthSet.GetID(), xs->m_BottomRStrength.GetID() );
        m_BottomCurvatureSkinControl.Update( xs->m_BottomLCurve.GetID(), xs->m_BottomLCurveSet.GetID(), xs->m_BottomLRCurveEq.GetID(), xs->m_BottomRCurveSet.GetID(), xs->m_BottomRCurve.GetID() );

        m_LeftHeader.m_ContChoice->SetVal( xs->m_LeftCont() );
        m_LeftAngleSkinControl.Update( xs->m_LeftLAngle.GetID(), xs->m_LeftLAngleSet.GetID(), xs->m_LeftLRAngleEq.GetID(), xs->m_LeftRAngleSet.GetID(), xs->m_LeftRAngle.GetID() );
        m_LeftSlewSkinControl.Update( xs->m_LeftLSlew.GetID(), xs->m_LeftLSlewSet.GetID(), xs->m_LeftLRSlewEq.GetID(), xs->m_LeftRSlewSet.GetID(), xs->m_LeftRSlew.GetID() );
        m_LeftStrengthSkinControl.Update( xs->m_LeftLStrength.GetID(), xs->m_LeftLStrengthSet.GetID(), xs->m_LeftLRStrengthEq.GetID(), xs->m_LeftRStrengthSet.GetID(), xs->m_LeftRStrength.GetID() );
        m_LeftCurvatureSkinControl.Update( xs->m_LeftLCurve.GetID(), xs->m_LeftLCurveSet.GetID(), xs->m_LeftLRCurveEq.GetID(), xs->m_LeftRCurveSet.GetID(), xs->m_LeftRCurve.GetID() );

        // Note that other Deactivate() calls are made in SkinControl::Update
        // to handle Set and Equal flag settings.

        // Deactivate GUI for symmetry
        if( xs->m_AllSymFlag() )
        {
            m_TBSymButton.Deactivate();
            m_RLSymButton.Deactivate();

            m_RightHeader.Deactivate();
            m_RightAngleSkinControl.Deactivate();
            m_RightSlewSkinControl.Deactivate();
            m_RightStrengthSkinControl.Deactivate();
            m_RightCurvatureSkinControl.Deactivate();

            m_BottomHeader.Deactivate();
            m_BottomAngleSkinControl.Deactivate();
            m_BottomSlewSkinControl.Deactivate();
            m_BottomStrengthSkinControl.Deactivate();
            m_BottomCurvatureSkinControl.Deactivate();

            m_LeftHeader.Deactivate();
            m_LeftAngleSkinControl.Deactivate();
            m_LeftSlewSkinControl.Deactivate();
            m_LeftStrengthSkinControl.Deactivate();
            m_LeftCurvatureSkinControl.Deactivate();
        }

        if( xs->m_TBSymFlag() )
        {
            m_BottomHeader.Deactivate();
            m_BottomAngleSkinControl.Deactivate();
            m_BottomSlewSkinControl.Deactivate();
            m_BottomStrengthSkinControl.Deactivate();
            m_BottomCurvatureSkinControl.Deactivate();
        }

        if( xs->m_RLSymFlag() )
        {
            m_LeftHeader.Deactivate();
            m_LeftAngleSkinControl.Deactivate();
            m_LeftSlewSkinControl.Deactivate();
            m_LeftStrengthSkinControl.Deactivate();
            m_LeftCurvatureSkinControl.Deactivate();
        }

        // Deactivate GUI for Set Strength -- doesn't make sense unless
        // Angle set is enabled.
        m_TopStrengthSkinControl.DeactivateSet();
        m_RightStrengthSkinControl.DeactivateSet();
        m_BottomStrengthSkinControl.DeactivateSet();
        m_LeftStrengthSkinControl.DeactivateSet();

        m_TopSlewSkinControl.DeactivateSet();
        m_RightSlewSkinControl.DeactivateSet();
        m_BottomSlewSkinControl.DeactivateSet();
        m_LeftSlewSkinControl.DeactivateSet();

        // Deactivate continuity control for first/last section.
        m_TopHeader.Activate();
        if ( xsid == 0 || xsid == (geomxsec_ptr->GetXSecSurf( 0 )->NumXSec() - 1) )
        {
            m_TopHeader.DeactiveContChoice();
        }

        // Deactivate RSet and Equality control to match continuity settings.
        // The RBL versions of these are redundant given current Code-Eli limitations
        // However, they are coded up now so fewer changes will be required later.
        if ( xs->m_TopCont() >= 1)
        {
            m_TopAngleSkinControl.DeactivateRSet();
            m_TopAngleSkinControl.DeactivateEqual();
            m_TopSlewSkinControl.DeactivateRSet();
            m_TopSlewSkinControl.DeactivateEqual();
            m_TopStrengthSkinControl.DeactivateRSet();
            m_TopStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_TopCont() >= 2)
        {
            m_TopCurvatureSkinControl.DeactivateRSet();
            m_TopCurvatureSkinControl.DeactivateEqual();
        }

        if ( xs->m_RightCont() >= 1)
        {
            m_RightAngleSkinControl.DeactivateRSet();
            m_RightAngleSkinControl.DeactivateEqual();
            m_RightSlewSkinControl.DeactivateRSet();
            m_RightSlewSkinControl.DeactivateEqual();
            m_RightStrengthSkinControl.DeactivateRSet();
            m_RightStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_RightCont() >= 2)
        {
            m_RightCurvatureSkinControl.DeactivateRSet();
            m_RightCurvatureSkinControl.DeactivateEqual();
        }

        if ( xs->m_BottomCont() >= 1)
        {
            m_BottomAngleSkinControl.DeactivateRSet();
            m_BottomAngleSkinControl.DeactivateEqual();
            m_BottomSlewSkinControl.DeactivateRSet();
            m_BottomSlewSkinControl.DeactivateEqual();
            m_BottomStrengthSkinControl.DeactivateRSet();
            m_BottomStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_BottomCont() >= 2)
        {
            m_BottomCurvatureSkinControl.DeactivateRSet();
            m_BottomCurvatureSkinControl.DeactivateEqual();
        }

        if ( xs->m_LeftCont() >= 1)
        {
            m_LeftAngleSkinControl.DeactivateRSet();
            m_LeftAngleSkinControl.DeactivateEqual();
            m_LeftSlewSkinControl.DeactivateRSet();
            m_LeftSlewSkinControl.DeactivateEqual();
            m_LeftStrengthSkinControl.DeactivateRSet();
            m_LeftStrengthSkinControl.DeactivateEqual();
        }
        if ( xs->m_LeftCont() >= 2)
        {
            m_LeftCurvatureSkinControl.DeactivateRSet();
            m_LeftCurvatureSkinControl.DeactivateEqual();
        }


        // Deactivate GUI for non-top curves.  Code-Eli right now requires
        // things to be set per cross section.  This restriction may someday
        // be lifted -- while the above Strength restriction will not.
        m_RightAngleSkinControl.DeactivateSet();
        m_RightSlewSkinControl.DeactivateSet();
        m_RightStrengthSkinControl.DeactivateSet();
        m_RightCurvatureSkinControl.DeactivateSet();

        m_BottomAngleSkinControl.DeactivateSet();
        m_BottomSlewSkinControl.DeactivateSet();
        m_BottomStrengthSkinControl.DeactivateSet();
        m_BottomCurvatureSkinControl.DeactivateSet();

        m_LeftAngleSkinControl.DeactivateSet();
        m_LeftSlewSkinControl.DeactivateSet();
        m_LeftStrengthSkinControl.DeactivateSet();
        m_LeftCurvatureSkinControl.DeactivateSet();

        // Deactivate GUI for non-top continuity control.  Code-Eli currently
        // requires continuity to be enforced on per cross section basis.
        m_RightHeader.DeactiveContChoice();
        m_BottomHeader.DeactiveContChoice();
        m_LeftHeader.DeactiveContChoice();
    }
    return true;
}

void SkinScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    //==== Find Fuselage Ptr ====//
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }
    GeomXSec* geomxsec_ptr = dynamic_cast< GeomXSec* >( geom_ptr );
    assert( geomxsec_ptr );

    if ( gui_device == m_TopHeader.m_ContChoice )
    {
        int t = m_TopHeader.m_ContChoice->GetVal();
        int xsid = geomxsec_ptr->m_ActiveXSec();
        SkinXSec* xs = (SkinXSec*) geomxsec_ptr->GetXSec( xsid );
        if ( xs )
        {
            xs->m_TopCont.Set( t );
            xs->Update();
            geomxsec_ptr->Update();
        }
    }

    XSecScreen::GuiDeviceCallBack( gui_device );
}


//==== Fltk  Callbacks ====//
void SkinScreen::CallBack( Fl_Widget *w )
{
    XSecScreen::CallBack( w );
}


//=====================================================================//
//=====================================================================//
//=====================================================================//
ChevronScreen::ChevronScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
        SkinScreen( mgr, w, h, title )
{

    //==== XSec Modifications ====//

    Fl_Group* modify_tab = AddTab( "Modify" );
    Fl_Group* modify_group = AddSubGroup( modify_tab, 5 );

    m_ModifyLayout.SetButtonWidth( 70 );

    m_ModifyLayout.SetGroupAndScreen( modify_group, this );
    m_ModifyLayout.AddDividerBox( "XSec" );

    m_ModifyLayout.AddIndexSelector( m_XsecModIndexSelector );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.InitWidthHeightVals();
    m_ModifyLayout.SetChoiceButtonWidth( m_ModifyLayout.GetButtonWidth() );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Chevron" );

    m_ChevronModeChoice.AddItem( "NONE", vsp::CHEVRON_NONE );
    m_ChevronModeChoice.AddItem( "PARTIAL", vsp::CHEVRON_PARTIAL );
    m_ChevronModeChoice.AddItem( "FULL", vsp::CHEVRON_FULL );
    m_ModifyLayout.AddChoice( m_ChevronModeChoice, "Type:" );

    m_ModifyLayout.AddSlider( m_ChevNumberSlider, "Number", 10, " %5.0f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Corner Rounding Radius" );

    m_ModifyLayout.SetButtonWidth( m_ModifyLayout.GetButtonWidth() * 0.6 );

    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddSlider( m_ChevPeakRadSlider, "Peak", 1.0, "%6.5f", m_ModifyLayout.GetW() * 0.5 + 5 );
    m_ModifyLayout.AddX( 5 );
    m_ModifyLayout.AddSlider( m_ChevValleyRadSlider, "Valley", 1.0, "%6.5f" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Waveform" );

    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddSlider( m_ChevOnDutySlider, "\% On", 1, "%6.5f", m_ModifyLayout.GetW() * 0.5 + 5 );
    m_ModifyLayout.AddX( 5 );
    m_ModifyLayout.AddSlider( m_ChevOffDutySlider, "\% Off", 1, "%6.5f" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.InitWidthHeightVals();
    m_ModifyLayout.SetChoiceButtonWidth( m_ModifyLayout.GetButtonWidth() );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Extents" );

    m_ChevronExtentModeChoice.AddItem( "Start \\/ End", vsp::CHEVRON_W01_SE );
    m_ChevronExtentModeChoice.AddItem( "Center \\/ Width", vsp::CHEVRON_W01_CW );
    m_ModifyLayout.AddChoice( m_ChevronExtentModeChoice, "Mode:" );

    m_ModifyLayout.SetChoiceButtonWidth( 0 );
    m_ModifyLayout.SetSameLineFlag( true );

    int sliderw = m_ModifyLayout.GetSliderWidth();
    m_ModifyLayout.SetSliderWidth( 75 );

    m_ModifyLayout.AddSlider( m_ChevW01StartSlider, "W Start", 1, "%6.5f", m_ModifyLayout.GetSliderWidth() );

    m_ChevW01StartChoice.AddItem( "Free", vsp::W_FREE );
    m_ChevW01StartChoice.AddItem( "Right 0", vsp::W_RIGHT_0 );
    m_ChevW01StartChoice.AddItem( "Bottom", vsp::W_BOTTOM );
    m_ChevW01StartChoice.AddItem( "Left", vsp::W_LEFT );
    m_ChevW01StartChoice.AddItem( "Top", vsp::W_TOP );
    m_ChevW01StartChoice.AddItem( "Right 1", vsp::W_RIGHT_1 );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddChoice( m_ChevW01StartChoice, "W Start:" );

    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );

    m_ModifyLayout.AddSlider( m_ChevW01EndSlider, "W End", 1, "%6.5f", m_ModifyLayout.GetSliderWidth() );

    m_ChevW01EndChoice.AddItem( "Free", vsp::W_FREE );
    m_ChevW01EndChoice.AddItem( "Right 0", vsp::W_RIGHT_0 );
    m_ChevW01EndChoice.AddItem( "Bottom", vsp::W_BOTTOM );
    m_ChevW01EndChoice.AddItem( "Left", vsp::W_LEFT );
    m_ChevW01EndChoice.AddItem( "Top", vsp::W_TOP );
    m_ChevW01EndChoice.AddItem( "Right 1", vsp::W_RIGHT_1 );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddChoice( m_ChevW01EndChoice, "W End:" );

    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );

    m_ModifyLayout.AddSlider( m_ChevW01CenterSlider, "W Center", 1, "%6.5f", m_ModifyLayout.GetSliderWidth() );

    m_ChevW01CenterChoice.AddItem( "Free", vsp::W_FREE );
    m_ChevW01CenterChoice.AddItem( "Right 0", vsp::W_RIGHT_0 );
    m_ChevW01CenterChoice.AddItem( "Bottom", vsp::W_BOTTOM );
    m_ChevW01CenterChoice.AddItem( "Left", vsp::W_LEFT );
    m_ChevW01CenterChoice.AddItem( "Top", vsp::W_TOP );
    m_ChevW01CenterChoice.AddItem( "Right 1", vsp::W_RIGHT_1 );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddChoice( m_ChevW01CenterChoice, "W Center:" );

    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetSameLineFlag( false );
    m_ModifyLayout.SetFitWidthFlag( true );

    m_ModifyLayout.AddSlider( m_ChevW01WidthSlider, "W Width", 1, "%6.5f", m_ModifyLayout.GetSliderWidth() );

    m_ModifyLayout.SetSliderWidth( sliderw );

    m_ModifyLayout.AddYGap();

    m_ModifyLayout.SetDividerHeight( m_ModifyLayout.GetStdHeight() );

    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddDividerBox( "Top Side", m_ModifyLayout.GetButtonWidth() );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_ChevAngleAllSymButton, "All Sym" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_ChevTopAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirTopAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirTopSlewSlider, "Slew", 90.0, "%5.2f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.AddDividerBox( "Right Side" );

    m_ModifyLayout.AddSlider( m_ChevRightAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirRightAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirRightSlewSlider, "Slew", 90.0, "%5.2f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddDividerBox( "Bottom Side", m_ModifyLayout.GetButtonWidth() );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_ChevAngleTBSymButton, "T/B Sym" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_ChevBottomAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirBottomAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirBottomSlewSlider, "Slew", 90.0, "%5.2f" );

    m_ModifyLayout.AddYGap();
    m_ModifyLayout.SetSameLineFlag( true );
    m_ModifyLayout.AddDividerBox( "Left Side", m_ModifyLayout.GetButtonWidth() );
    m_ModifyLayout.SetFitWidthFlag( false );
    m_ModifyLayout.AddButton( m_ChevAngleRLSymButton, "R/L Sym" );
    m_ModifyLayout.ForceNewLine();
    m_ModifyLayout.SetFitWidthFlag( true );
    m_ModifyLayout.SetSameLineFlag( false );

    m_ModifyLayout.AddSlider( m_ChevLeftAmpSlider, "Amplitude", 10, "%6.5f" );
    m_ModifyLayout.AddSlider( m_ChevDirLeftAngleSlider, "Angle", 90.0, "%5.2f" );
    m_ModifyLayout.AddSlider( m_ChevDirLeftSlewSlider, "Slew", 90.0, "%5.2f" );
}

//==== Update Pod Screen ====//
bool ChevronScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return false;
    }

    SkinScreen::Update();

    GeomXSec* geomxsec_ptr = dynamic_cast< GeomXSec* >( geom_ptr );
    assert( geomxsec_ptr );

    //==== XSec Index Display ===//
    int xsid = geomxsec_ptr->m_ActiveXSec();
    m_XsecModIndexSelector.Update( geomxsec_ptr->m_ActiveXSec.GetID() );

    StackXSec* xs = ( StackXSec* ) geomxsec_ptr->GetXSec( xsid );
    if ( xs )
    {
        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {

            m_ChevronModeChoice.Update( xsc->m_ChevronType.GetID() );

            m_ChevTopAmpSlider.Update( xsc->m_ChevTopAmplitude.GetID() );
            m_ChevBottomAmpSlider.Update( xsc->m_ChevBottomAmplitude.GetID() );
            m_ChevLeftAmpSlider.Update( xsc->m_ChevLeftAmplitude.GetID() );
            m_ChevRightAmpSlider.Update( xsc->m_ChevRightAmplitude.GetID() );

            m_ChevNumberSlider.Update( xsc->m_ChevNumber.GetID() );

            m_ChevOnDutySlider.Update( xsc->m_ChevOnDuty.GetID() );
            m_ChevOffDutySlider.Update( xsc->m_ChevOffDuty.GetID() );

            m_ChevronExtentModeChoice.Update( xsc->m_ChevronExtentMode.GetID() );

            m_ChevW01StartChoice.Update( xsc->m_ChevW01StartGuide.GetID() );
            m_ChevW01StartSlider.Update( xsc->m_ChevW01Start.GetID() );
            m_ChevW01EndChoice.Update( xsc->m_ChevW01EndGuide.GetID() );
            m_ChevW01EndSlider.Update( xsc->m_ChevW01End.GetID() );
            m_ChevW01CenterChoice.Update( xsc->m_ChevW01CenterGuide.GetID() );
            m_ChevW01CenterSlider.Update( xsc->m_ChevW01Center.GetID() );
            m_ChevW01WidthSlider.Update( xsc->m_ChevW01Width.GetID() );

            m_ChevAngleAllSymButton.Update( xsc->m_ChevDirAngleAllSymFlag.GetID() );
            m_ChevAngleTBSymButton.Update( xsc->m_ChevDirAngleTBSymFlag.GetID() );
            m_ChevAngleRLSymButton.Update( xsc->m_ChevDirAngleRLSymFlag.GetID() );

            m_ChevDirTopAngleSlider.Update( xsc->m_ChevTopAngle.GetID() );
            m_ChevDirBottomAngleSlider.Update( xsc->m_ChevBottomAngle.GetID() );
            m_ChevDirRightAngleSlider.Update( xsc->m_ChevRightAngle.GetID() );
            m_ChevDirLeftAngleSlider.Update( xsc->m_ChevLeftAngle.GetID() );

            m_ChevDirTopSlewSlider.Update( xsc->m_ChevTopSlew.GetID() );
            m_ChevDirBottomSlewSlider.Update( xsc->m_ChevBottomSlew.GetID() );
            m_ChevDirRightSlewSlider.Update( xsc->m_ChevRightSlew.GetID() );
            m_ChevDirLeftSlewSlider.Update( xsc->m_ChevLeftSlew.GetID() );

            m_ChevValleyRadSlider.Update( xsc->m_ValleyRad.GetID() );
            m_ChevPeakRadSlider.Update( xsc->m_PeakRad.GetID() );
        }
    }
    return true;
}

void ChevronScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    SkinScreen::GuiDeviceCallBack( gui_device );
}

//==== Fltk  Callbacks ====//
void ChevronScreen::CallBack( Fl_Widget *w )
{
    SkinScreen::CallBack( w );
}


//=====================================================================//
//=====================================================================//
//=====================================================================//
BlendScreen::BlendScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    XSecScreen( mgr, w, h, title )
{
    const char* angleFmt = "%5.2f";
    const char* strengthFmt = "%5.2f";

    const double angleRng = 90;
    const double strengthRng = 1;
    const double dihRng = 20;

    const int bw = 55;

    Fl_Group* blend_tab = AddTab( "Blending" );
    Fl_Group* blend_group = AddSubGroup( blend_tab, 5 );

    m_BlendLayout.SetGroupAndScreen( blend_group, this );

    m_BlendLayout.AddDividerBox( "Blend Airfoil" );

    m_BlendLayout.AddIndexSelector( m_BlendIndexSelector );

    m_BlendLayout.AddYGap();

    m_BlendLayout.AddDividerBox( "Leading Edge" );
    m_BlendLayout.AddYGap();

    m_BlendLayout.AddSubGroupLayout( m_InLELayout, m_BlendLayout.GetW()/2 - 2, m_BlendLayout.GetStdHeight() * 4 + m_BlendLayout.GetDividerHeight() );
    m_BlendLayout.AddX( m_BlendLayout.GetW()/2 + 2 );
    m_BlendLayout.AddSubGroupLayout( m_OutLELayout, m_BlendLayout.GetW()/2 - 2, m_BlendLayout.GetStdHeight() * 4 + m_BlendLayout.GetDividerHeight() );

    m_InLEChoice.AddItem( "FREE" );
    m_InLEChoice.AddItem( "ANGLES" );
    m_InLEChoice.AddItem( "IN_LE_TRAP" );
    m_InLEChoice.AddItem( "IN_TE_TRAP" );
    m_InLEChoice.AddItem( "OUT_LE_TRAP" );
    m_InLEChoice.AddItem( "OUT_TE_TRAP" );
    m_InLEChoice.AddItem( "IN_ANGLES" );
    m_InLEChoice.AddItem( "LE_ANGLES" );
    m_InLEChoice.SetFlag( BLEND_MATCH_IN_ANGLES, FL_MENU_INVISIBLE );
    m_InLEChoice.SetFlag( BLEND_MATCH_LE_ANGLES, FL_MENU_INVISIBLE );

    m_InLELayout.SetButtonWidth( bw );
    m_InLELayout.SetChoiceButtonWidth( m_InLELayout.GetButtonWidth() );
    m_InLELayout.AddDividerBox( "Inboard" );
    m_InLELayout.AddChoice( m_InLEChoice, "Match:" );
    m_InLELayout.AddSlider( m_InLESweep, "Sweep", angleRng, angleFmt );
    m_InLELayout.AddSlider( m_InLEDihedral, "Dihedral", dihRng, angleFmt );
    m_InLELayout.AddSlider( m_InLEStrength, "Strength", strengthRng, strengthFmt );

    m_OutLEChoice.AddItem( "FREE" );
    m_OutLEChoice.AddItem( "ANGLES" );
    m_OutLEChoice.AddItem( "IN_LE_TRAP" );
    m_OutLEChoice.AddItem( "IN_TE_TRAP" );
    m_OutLEChoice.AddItem( "OUT_LE_TRAP" );
    m_OutLEChoice.AddItem( "OUT_TE_TRAP" );
    m_OutLEChoice.AddItem( "IN_ANGLES" );
    m_OutLEChoice.AddItem( "LE_ANGLES" );
    m_OutLEChoice.SetFlag( BLEND_MATCH_LE_ANGLES, FL_MENU_INVISIBLE );

    m_OutLELayout.SetButtonWidth( bw );
    m_OutLELayout.SetChoiceButtonWidth( m_OutLELayout.GetButtonWidth() );
    m_OutLELayout.AddDividerBox( "Outboard" );
    m_OutLELayout.AddChoice( m_OutLEChoice, "Match:" );
    m_OutLELayout.AddSlider( m_OutLESweep, "Sweep", angleRng, angleFmt );
    m_OutLELayout.AddSlider( m_OutLEDihedral, "Dihedral", dihRng, angleFmt );
    m_OutLELayout.AddSlider( m_OutLEStrength, "Strength", strengthRng, strengthFmt );

    m_BlendLayout.ForceNewLine();
    m_BlendLayout.AddY( m_InLELayout.GetH() );

    m_BlendLayout.AddDividerBox( "Trailing Edge" );
    m_BlendLayout.AddYGap();

    m_BlendLayout.AddSubGroupLayout( m_InTELayout, m_BlendLayout.GetW()/2 - 2, m_BlendLayout.GetStdHeight() * 4 + m_BlendLayout.GetDividerHeight() );
    m_BlendLayout.AddX( m_BlendLayout.GetW()/2 + 2 );
    m_BlendLayout.AddSubGroupLayout( m_OutTELayout, m_BlendLayout.GetW()/2 - 2, m_BlendLayout.GetStdHeight() * 4 + m_BlendLayout.GetDividerHeight() );


    m_InTEChoice.AddItem( "FREE" );
    m_InTEChoice.AddItem( "ANGLES" );
    m_InTEChoice.AddItem( "IN_LE_TRAP" );
    m_InTEChoice.AddItem( "IN_TE_TRAP" );
    m_InTEChoice.AddItem( "OUT_LE_TRAP" );
    m_InTEChoice.AddItem( "OUT_TE_TRAP" );
    m_InTEChoice.AddItem( "IN_ANGLES" );
    m_InTEChoice.AddItem( "LE_ANGLES" );
    m_InTEChoice.SetFlag( BLEND_MATCH_IN_ANGLES, FL_MENU_INVISIBLE );

    m_InTELayout.SetButtonWidth( bw );
    m_InTELayout.SetChoiceButtonWidth( m_InTELayout.GetButtonWidth() );
    m_InTELayout.AddDividerBox( "Inboard" );
    m_InTELayout.AddChoice( m_InTEChoice, "Match:" );
    m_InTELayout.AddSlider( m_InTESweep, "Sweep", angleRng, angleFmt );
    m_InTELayout.AddSlider( m_InTEDihedral, "Dihedral", dihRng, angleFmt );
    m_InTELayout.AddSlider( m_InTEStrength, "Strength", strengthRng, strengthFmt );


    m_OutTEChoice.AddItem( "FREE" );
    m_OutTEChoice.AddItem( "ANGLES" );
    m_OutTEChoice.AddItem( "IN_LE_TRAP" );
    m_OutTEChoice.AddItem( "IN_TE_TRAP" );
    m_OutTEChoice.AddItem( "OUT_LE_TRAP" );
    m_OutTEChoice.AddItem( "OUT_TE_TRAP" );
    m_OutTEChoice.AddItem( "IN_ANGLES" );
    m_OutTEChoice.AddItem( "LE_ANGLES" );

    m_OutTELayout.SetButtonWidth( bw );
    m_OutTELayout.SetChoiceButtonWidth( m_OutTELayout.GetButtonWidth() );
    m_OutTELayout.AddDividerBox( "Outboard" );
    m_OutTELayout.AddChoice( m_OutTEChoice, "Match:" );
    m_OutTELayout.AddSlider( m_OutTESweep, "Sweep", angleRng, angleFmt );
    m_OutTELayout.AddSlider( m_OutTEDihedral, "Dihedral", dihRng, angleFmt );
    m_OutTELayout.AddSlider( m_OutTEStrength, "Strength", strengthRng, strengthFmt );

    m_BlendLayout.ForceNewLine();
    m_BlendLayout.AddY( m_OutLELayout.GetH() );

}


//==== Update Pod Screen ====//
bool BlendScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return false;
    }

    XSecScreen::Update();

    GeomXSec* geomxsec_ptr = dynamic_cast< GeomXSec* >( geom_ptr );
    assert( geomxsec_ptr );

    WingGeom* wing_ptr = dynamic_cast< WingGeom* >( geom_ptr );
    assert( wing_ptr );


    //==== Skin & XSec Index Display ===//
    int xsid = wing_ptr->m_ActiveXSec();
    m_BlendIndexSelector.Update( wing_ptr->m_ActiveXSec.GetID() );

    BlendWingSect* xs = ( BlendWingSect* ) geomxsec_ptr->GetXSec( xsid );
    if ( xs )
    {
        bool firstxs = xsid == 0;
        bool lastxs = xsid == ( geomxsec_ptr->GetXSecSurf( 0 )->NumXSec() - 1 );

        //==== Blend ====//

        m_InLEChoice.Update( xs->m_InLEMode.GetID() );
        m_InLESweep.Update( xs->m_InLESweep.GetID() );
        m_InLEDihedral.Update( xs->m_InLEDihedral.GetID() );
        m_InLEStrength.Update( xs->m_InLEStrength.GetID() );

        m_InTEChoice.Update( xs->m_InTEMode.GetID() );
        m_InTESweep.Update( xs->m_InTESweep.GetID() );
        m_InTEDihedral.Update( xs->m_InTEDihedral.GetID() );
        m_InTEStrength.Update( xs->m_InTEStrength.GetID() );

        m_OutLEChoice.Update( xs->m_OutLEMode.GetID() );
        m_OutLESweep.Update( xs->m_OutLESweep.GetID() );
        m_OutLEDihedral.Update( xs->m_OutLEDihedral.GetID() );
        m_OutLEStrength.Update( xs->m_OutLEStrength.GetID() );

        m_OutTEChoice.Update( xs->m_OutTEMode.GetID() );
        m_OutTESweep.Update( xs->m_OutTESweep.GetID() );
        m_OutTEDihedral.Update( xs->m_OutTEDihedral.GetID() );
        m_OutTEStrength.Update( xs->m_OutTEStrength.GetID() );


        // Set pull-down menu flags to 'normal'
        m_InTEChoice.SetFlag( BLEND_MATCH_LE_ANGLES, 0 );

        m_OutLEChoice.SetFlag( BLEND_MATCH_IN_ANGLES, 0 );

        m_OutTEChoice.SetFlag( BLEND_MATCH_IN_ANGLES, 0 );
        m_OutTEChoice.SetFlag( BLEND_MATCH_LE_ANGLES, 0 );

        m_InLEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_InLEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_InLEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_InLEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, 0 );

        m_InTEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_InTEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_InTEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_InTEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, 0 );

        m_OutLEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_OutLEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_OutLEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_OutLEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, 0 );

        m_OutTEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_OutTEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_OutTEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_OutTEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, 0 );

        if ( xs->m_InLEMode() != BLEND_ANGLES )
        {
            m_InTEChoice.SetFlag( BLEND_MATCH_LE_ANGLES, FL_MENU_INACTIVE );
            m_OutLEChoice.SetFlag( BLEND_MATCH_IN_ANGLES, FL_MENU_INACTIVE );
        }

        if ( xs->m_InTEMode() != BLEND_ANGLES && xs->m_InTEMode() != BLEND_MATCH_LE_ANGLES )
        {
            m_OutTEChoice.SetFlag( BLEND_MATCH_IN_ANGLES, FL_MENU_INACTIVE );
        }

        if ( xs->m_OutLEMode() != BLEND_ANGLES && xs->m_OutLEMode() != BLEND_MATCH_IN_ANGLES )
        {
            m_OutTEChoice.SetFlag( BLEND_MATCH_LE_ANGLES, FL_MENU_INACTIVE );
        }

        if ( firstxs )
        {
            m_InLEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_InLEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );

            m_InTEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_InTEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );

            m_OutLEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_OutLEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );

            m_OutTEChoice.SetFlag( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_OutTEChoice.SetFlag( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );
        }

        if ( lastxs )
        {
            m_InLEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_InLEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );

            m_InTEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_InTEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );

            m_OutLEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_OutLEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );

            m_OutTEChoice.SetFlag( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_OutTEChoice.SetFlag( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );
        }

        // Update menu while keeping setting.
        m_InLEChoice.UpdateItems( true );
        m_InTEChoice.UpdateItems( true );
        m_OutLEChoice.UpdateItems( true );
        m_OutTEChoice.UpdateItems( true );

        m_InLESweep.Deactivate();
        m_InLEDihedral.Deactivate();

        m_InTESweep.Deactivate();
        m_InTEDihedral.Deactivate();

        if ( firstxs )
        {
            m_InLEChoice.Deactivate();
            m_InLEStrength.Deactivate();

            m_InTEChoice.Deactivate();
            m_InTEStrength.Deactivate();
        }
        else
        {
            m_InLEChoice.Activate();
            m_InLEStrength.Activate();

            m_InTEChoice.Activate();
            m_InTEStrength.Activate();

            switch (xs->m_InLEMode())
            {
                case BLEND_ANGLES :
                {
                    m_InLESweep.Activate();
                    m_InLEDihedral.Activate();
                    break;
                }
                case BLEND_FREE :
                {
                    m_InLEStrength.Deactivate();
                    break;
                }
                case BLEND_MATCH_IN_ANGLES :
                case BLEND_MATCH_LE_ANGLES :
                case BLEND_MATCH_IN_LE_TRAP :
                case BLEND_MATCH_IN_TE_TRAP :
                case BLEND_MATCH_OUT_LE_TRAP :
                case BLEND_MATCH_OUT_TE_TRAP :
                {
                    break;
                }
            }

            switch (xs->m_InTEMode())
            {
                case BLEND_ANGLES :
                {
                    m_InTESweep.Activate();
                    m_InTEDihedral.Activate();
                    break;
                }
                case BLEND_FREE :
                {
                    m_InTEStrength.Deactivate();
                    break;
                }
                case BLEND_MATCH_IN_ANGLES :
                case BLEND_MATCH_LE_ANGLES :
                case BLEND_MATCH_IN_LE_TRAP :
                case BLEND_MATCH_IN_TE_TRAP :
                case BLEND_MATCH_OUT_LE_TRAP :
                case BLEND_MATCH_OUT_TE_TRAP :
                {
                    break;
                }
            }
        }

        m_OutLESweep.Deactivate();
        m_OutLEDihedral.Deactivate();

        m_OutTESweep.Deactivate();
        m_OutTEDihedral.Deactivate();

        if ( lastxs )
        {
            m_OutLEChoice.Deactivate();
            m_OutLEStrength.Deactivate();

            m_OutTEChoice.Deactivate();
            m_OutTEStrength.Deactivate();
        }
        else
        {
            m_OutLEChoice.Activate();
            m_OutLEStrength.Activate();

            m_OutTEChoice.Activate();
            m_OutTEStrength.Activate();

            switch (xs->m_OutLEMode())
            {
                case BLEND_ANGLES :
                {
                    m_OutLESweep.Activate();
                    m_OutLEDihedral.Activate();
                    break;
                }
                case BLEND_FREE :
                {
                    m_OutLEStrength.Deactivate();
                    break;
                }
                case BLEND_MATCH_IN_ANGLES :
                case BLEND_MATCH_LE_ANGLES :
                case BLEND_MATCH_IN_LE_TRAP :
                case BLEND_MATCH_IN_TE_TRAP :
                case BLEND_MATCH_OUT_LE_TRAP :
                case BLEND_MATCH_OUT_TE_TRAP :
                {
                    break;
                }
            }

            switch (xs->m_OutTEMode())
            {
                case BLEND_ANGLES :
                {
                    m_OutTESweep.Activate();
                    m_OutTEDihedral.Activate();
                    break;
                }
                case BLEND_FREE :
                {
                    m_OutTEStrength.Deactivate();
                    break;
                }
                case BLEND_MATCH_IN_ANGLES :
                case BLEND_MATCH_LE_ANGLES :
                case BLEND_MATCH_IN_LE_TRAP :
                case BLEND_MATCH_IN_TE_TRAP :
                case BLEND_MATCH_OUT_LE_TRAP :
                case BLEND_MATCH_OUT_TE_TRAP :
                {
                    break;
                }
            }
        }


    }
    return true;
}

//==== Fltk  Callbacks ====//
void BlendScreen::CallBack( Fl_Widget *w )
{
    XSecScreen::CallBack( w );
}


//=====================================================================//
//=====================================================================//
//=====================================================================//



XSecViewScreen::XSecViewScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 310, 600, "XSec View" )
{
    int title_h = 20;
    int border = 5;
    int window_x = m_FLTK_Window->x() + border;
    int window_y = m_FLTK_Window->y() + border + title_h;
    int window_w_h = 300;

    m_FLTK_Window->begin();
    m_GlWin = new VSPGUI::VspSubGlWindow( window_x, window_y, window_w_h, window_w_h, DrawObj::VSP_XSEC_SCREEN);
    m_FLTK_Window->end();

    m_GlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_TOP );
    m_GlWin->getGraphicEngine()->getDisplay()->getViewport()->showGridOverlay( false );

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );

    m_MainLayout.ForceNewLine();
    m_MainLayout.AddY( window_y + window_w_h - title_h );
    m_MainLayout.AddX( border );

    m_MainLayout.AddSubGroupLayout( m_BorderLayout, m_MainLayout.GetRemainX() - border,
                                    m_MainLayout.GetRemainY() - border );

    m_BorderLayout.AddSubGroupLayout( m_ColorLayout, m_BorderLayout.GetRemainX(), 3 * m_BorderLayout.GetStdHeight() + border );

    m_ColorLayout.AddY( border );
    m_ColorLayout.AddDividerBox( "Line Color" );

    m_ColorLayout.AddColorPicker( m_ColorPicker );

    m_BorderLayout.AddY( m_ColorLayout.GetH() );

    m_BorderLayout.AddDividerBox( "Background Image" );
    m_BorderLayout.AddButton( m_Image, "Image" );
    m_BorderLayout.AddYGap();

    m_BorderLayout.AddSubGroupLayout( m_ImageLayout, m_BorderLayout.GetRemainX(), m_BorderLayout.GetRemainY() - 20 );

    m_ImageLayout.SetFitWidthFlag( false );
    m_ImageLayout.SetSameLineFlag( true );

    m_ImageLayout.SetInputWidth( m_ImageLayout.GetRemainX() - 70 );
    m_ImageLayout.SetButtonWidth( 40 );
    m_ImageLayout.AddOutput( m_FileOutput, "File:" );
    m_ImageLayout.SetButtonWidth( 30 );
    m_ImageLayout.AddButton( m_FileSelect, "..." );
    m_ImageLayout.ForceNewLine();
    m_ImageLayout.AddYGap();

    m_ImageLayout.SetFitWidthFlag( false );
    m_ImageLayout.SetSameLineFlag( true );
    m_ImageLayout.SetButtonWidth( m_ImageLayout.GetRemainX() / 2 );

    m_ImageLayout.AddButton( m_PreserveAspect, "Preserve Aspect" );
    m_ImageLayout.AddButton( m_FlipImageToggle, "Flip Image" );
    m_ImageLayout.ForceNewLine();

    m_ImageLayout.SetFitWidthFlag( true );
    m_ImageLayout.SetSameLineFlag( false );
    m_ImageLayout.AddYGap();

    m_ImageLayout.SetButtonWidth( 60 );
    m_ImageLayout.SetInputWidth( 50 );
    m_ImageLayout.AddSlider( m_WScale, "W Scale", 1.0, "%7.3f" );
    m_ImageLayout.AddSlider( m_HScale, "H Scale", 1.0, "%7.3f" );
    m_ImageLayout.AddYGap();
    m_ImageLayout.AddSlider( m_XOffset, "X Offset", 0.500, "%7.3f" );
    m_ImageLayout.AddSlider( m_YOffset, "Y Offset", 0.500, "%7.3f" );

    m_BorderLayout.AddY( m_ImageLayout.GetH() );
    m_BorderLayout.AddButton( m_ResetDefaults, "Reset Defaults" );

    m_Image.GetFlButton()->value( 0 );
    m_PreserveAspect.GetFlButton()->value( 1 );
}

bool XSecViewScreen::Update()
{
    assert( m_ScreenMgr );

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    vector < Geom* > geom_vec = veh->GetActiveGeomPtrVec();
    if ( geom_vec.size() != 1 )
    {
        Hide();
        return false;
    }

    XSecCurve* xsc = NULL;

    Geom* geom = geom_vec[0];
    GeomXSec* geom_xsec = dynamic_cast<GeomXSec*>( geom );
    if ( geom_xsec )
    {
        XSec* xs = geom_xsec->GetXSec( geom_xsec->m_ActiveXSec() );

        if( !xs )
        {
            Hide();
            return false;
        }

        xsc = xs->GetXSecCurve();
    }

    BORGeom* bg = dynamic_cast< BORGeom* > ( geom );
    if ( bg )
    {
        xsc = bg->GetXSecCurve();
    }

    if( !xsc )
    {
        Hide();
        return false;
    }

    VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();

    m_ColorPicker.Update( veh->GetXSecLineColor() );

    m_Image.Update( xsc->m_XSecImageFlag.GetID() );
    m_FileOutput.Update( StringUtil::truncateFileName( xsc->GetImageFile(), 40 ).c_str() );

    // Update Scale and Offset in Background
    m_WScale.Update( xsc->m_XSecImageW.GetID() );
    m_HScale.Update( xsc->m_XSecImageH.GetID() );

    if ( xsc->m_XSecImageFlag() )
    {
        m_ImageLayout.GetGroup()->activate();

        if ( ( viewport->getBackground()->getBackgroundMode() != VSPGraphic::Common::VSP_BACKGROUND_IMAGE ||
            ( VSPGraphic::GlobalTextureRepo()->getTextureID( xsc->GetImageFile().c_str() ) != viewport->getBackground()->getTextureID() ) ) &&
            xsc->GetImageFile().size() > 0 )
        {
            viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );
            viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( xsc->GetImageFile().c_str() ) );
        }
    }
    else
    {
        m_ImageLayout.GetGroup()->deactivate();

        if ( viewport->getBackground()->getBackgroundMode() == VSPGraphic::Common::VSP_BACKGROUND_IMAGE )
        {
            viewport->getBackground()->removeImage();
            viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
        }
    }

    viewport->getBackground()->scaleW( (float)xsc->m_XSecImageW.Get() );

    m_PreserveAspect.Update( xsc->m_XSecImagePreserveAR.GetID() );
    viewport->getBackground()->preserveAR( (bool)xsc->m_XSecImagePreserveAR.Get() );

    m_FlipImageToggle.Update( xsc->m_XSecFlipImageFlag.GetID() );
    viewport->getBackground()->flipX( (bool)xsc->m_XSecFlipImageFlag.Get() );

    if ( xsc->m_XSecImagePreserveAR() )
    {
        xsc->m_XSecImageH.Set( viewport->getBackground()->getScaleH() );
        m_HScale.Deactivate();
    }
    else
    {
        viewport->getBackground()->scaleH( (float)xsc->m_XSecImageH.Get() );
        m_HScale.Activate();
    }

    m_XOffset.Update( xsc->m_XSecImageXOffset.GetID() );
    m_YOffset.Update( xsc->m_XSecImageYOffset.GetID() );

    viewport->getBackground()->offsetX( (float)xsc->m_XSecImageXOffset.Get() );
    viewport->getBackground()->offsetY( (float)xsc->m_XSecImageYOffset.Get() );

    m_GlWin->update();
    m_GlWin->redraw();

    return true;
}

void XSecViewScreen::Show()
{
    VspScreen::Show();

    int w, h;

    w = m_GlWin->pixel_w();
    h = m_GlWin->pixel_h();
    float oz = 1.5f * ( w < h ? 1.f / w : 1.f / h );
    m_GlWin->getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( oz );
}

void XSecViewScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    vector < Geom* > geom_vec = veh->GetActiveGeomPtrVec();
    if ( geom_vec.size() != 1 )
    {
        Hide();
        return;
    }

    XSecCurve* xsc = NULL;

    Geom* geom = geom_vec[0];
    GeomXSec* geom_xsec = dynamic_cast<GeomXSec*>( geom );
    if ( geom_xsec )
    {
        XSec* xs = geom_xsec->GetXSec( geom_xsec->m_ActiveXSec() );

        if( !xs )
        {
            Hide();
            return;
        }

        xsc = xs->GetXSecCurve();
    }

    BORGeom* bg = dynamic_cast< BORGeom* > ( geom );
    if ( bg )
    {
        xsc = bg->GetXSecCurve();
    }

    if( !xsc )
    {
        Hide();
        return;
    }

    VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();

    if ( device == &m_ColorPicker )
    {
        veh->SetXSecLineColor( m_ColorPicker.GetColor() );
    }
    else if ( device == &m_Image )
    {
        if ( m_Image.GetFlButton()->value() )
        {
            viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );

            if ( xsc->GetImageFile().compare( "" ) != 0 )
            {
                viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( xsc->GetImageFile().c_str() ) );
            }
        }
        else
        {
            viewport->getBackground()->removeImage();
            viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_COLOR );
        }
    }
    else if ( device == &m_FileSelect )
    {
        std::string fileName = m_ScreenMgr->GetSelectFileScreen()->FileChooser(
            "Select Image File", "*.{jpg,png,tga,bmp,gif}", false );

        if ( !fileName.empty() )
        {
            if ( m_Image.GetFlButton()->value() )
            {
                viewport->getBackground()->removeImage();
                viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( fileName.c_str() ) );
            }

            xsc->SetImageFile( fileName );
        }
    }
    else if ( device == &m_ResetDefaults )
    {
        viewport->getBackground()->reset();
        xsc->m_XSecImageFlag.Set( false );
        xsc->SetImageFile( "" );
        xsc->m_XSecImagePreserveAR.Set( true );

        veh->SetXSecLineColor( vec3d( 0, 0, 0 ) );

        // Reset Scale & Offset
        xsc->m_XSecImageW.Set( viewport->getBackground()->getScaleW() );
        xsc->m_XSecImageH.Set( viewport->getBackground()->getScaleH() );

        xsc->m_XSecImageXOffset.Set( viewport->getBackground()->getOffsetX() );
        xsc->m_XSecImageYOffset.Set( viewport->getBackground()->getOffsetY() );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

//=====================================================================//
//=====================================================================//
//=====================================================================//



FeaXSecScreen::FeaXSecScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 300, 300, "FEA XSec" )
{
    int x = m_FLTK_Window->x();
    int y = m_FLTK_Window->y();
    int w = m_FLTK_Window->w();
    int h = m_FLTK_Window->h();


    m_FLTK_Window->begin();
    m_GlWin = new VSPGUI::VspSubGlWindow( x, y, w, h, DrawObj::VSP_FEA_XSEC_SCREEN );
    m_FLTK_Window->end();

    m_GlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_TOP );
    m_GlWin->getGraphicEngine()->getDisplay()->getViewport()->showGridOverlay( false );
}

bool FeaXSecScreen::Update()
{
    assert( m_ScreenMgr );

    VSPGraphic::Viewport * viewport = m_GlWin->getGraphicEngine()->getDisplay()->getViewport();
    assert( viewport );

    Vehicle *veh = VehicleMgr.GetVehicle();

    string ExePath = veh->GetExePath();

    string fileName = ExePath + string( "/textures/" );

    char forwardSlash = '\\';
    
    for ( size_t i = 0; i < (int)fileName.size(); i++ )
    {
        if ( fileName[i] == forwardSlash )
        {
            fileName[i] = '/';
        }
    }

    int xsec_type = -1;

    if ( StructureMgr.ValidFeaPropertyInd( StructureMgr.GetCurrPropertyIndex() ) )
    {
        FeaProperty* fea_prop = StructureMgr.GetFeaPropertyVec()[StructureMgr.GetCurrPropertyIndex()];

        if ( fea_prop )
        {
            xsec_type = fea_prop->m_CrossSectType();
        }
    }

    if ( xsec_type == vsp::FEA_XSEC_CIRC )
    {
        fileName += "VSPCircXSec.png";
    }
    else if ( xsec_type == vsp::FEA_XSEC_PIPE )
    {
        fileName += "VSPPipeXSec.png";
    }
    else if ( xsec_type == vsp::FEA_XSEC_I )
    {
        fileName += "VSPIXSec.png";
    }
    else if ( xsec_type == vsp::FEA_XSEC_RECT )
    {
        fileName += "VSPRectXSec.png";
    }
    else if ( xsec_type == vsp::FEA_XSEC_BOX )
    {
        fileName += "VSPBoxXSec.png";
    }
    else
    {
        VspScreen::Hide();
    }

    viewport->getBackground()->setBackgroundMode( VSPGraphic::Common::VSP_BACKGROUND_IMAGE );

    if ( fileName.compare( "" ) != 0 )
    {
        viewport->getBackground()->removeImage();
        viewport->getBackground()->attachImage( VSPGraphic::GlobalTextureRepo()->get2DTexture( fileName.c_str() ) );
    }

    m_GlWin->update();
    m_GlWin->redraw();

    return true;
}

void FeaXSecScreen::Show()
{
    VspScreen::Show();

    int w, h;

    w = m_GlWin->pixel_w();
    h = m_GlWin->pixel_h();
    float oz = 1.5f * ( w < h ? 1.f / w : 1.f / h );
    m_GlWin->getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( oz );
}
