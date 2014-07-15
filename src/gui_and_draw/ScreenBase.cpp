//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "ParmMgr.h"
#include "SubSurfaceMgr.h"
#include "SubSurface.h"
#include "APIDefines.h"
#include "MaterialRepo.h"
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

//==== Constructor ====//
BasicScreen::BasicScreen( ScreenMgr* mgr, int w, int h, const string & title  ) : VspScreen( mgr )
{
    //==== Window ====//
    m_FLTK_Window = new Fl_Double_Window( w, h );
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

//=====================================================================//
//=====================================================================//
//=====================================================================//

//==== Constructor ====//
TabScreen::TabScreen( ScreenMgr* mgr, int w, int h, const string & title, int baseymargin ) :
    BasicScreen( mgr, w, h, title )
{
    //==== Menu Tabs ====//
    m_MenuTabs = new Fl_Tabs( 0, 25, w, h - 25 - baseymargin );
    m_MenuTabs->labelcolor( FL_BLUE );
}

//==== Destructor ====//
TabScreen::~TabScreen()
{
}

//==== Add Tab ====//
Fl_Group* TabScreen::AddTab( const string& title )
{
    int rx, ry, rw, rh;
    m_MenuTabs->client_area( rx, ry, rw, rh, TAB_H );

    Fl_Group* grp = new Fl_Group( rx, ry, rw, rh );
    grp->copy_label( title.c_str() );
    grp->selection_color( FL_GRAY );
    grp->labelfont( 1 );
    grp->labelcolor( FL_BLACK );
    grp->hide();
    m_TabGroupVec.push_back( grp );

    m_MenuTabs->add( grp );

    return grp;
}

//==== Remove Tab ====//
void TabScreen::RemoveTab( Fl_Group* grp )
{
    m_MenuTabs->remove( grp );
}

//==== Remove Tab ====//
void TabScreen::AddTab( Fl_Group* grp )
{
    m_MenuTabs->add( grp );
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

//==== Add A Sub Scroll To Tab ====//
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
    Fl_Group* gen_tab = AddTab( "Gen" );
    Fl_Group* xform_tab = AddTab( "XForm" );
    Fl_Group* subsurf_tab = AddTab( "Sub" );
    m_SubSurfTab_ind = m_TabGroupVec.size() - 1;
    Fl_Group* gen_group = AddSubGroup( gen_tab, 5 );
    Fl_Group* xform_group = AddSubGroup( xform_tab, 5 );
    Fl_Group* subsurf_group = AddSubGroup( subsurf_tab, 5 );

    //==== Gen Group Layout ====//
    m_GenLayout.SetGroupAndScreen( gen_group, this );
    m_GenLayout.AddDividerBox( "Name & Color" );
    m_GenLayout.AddInput( m_NameInput, "Name:" );
    m_GenLayout.AddYGap();
    m_GenLayout.AddColorPicker( m_ColorPicker );
    m_GenLayout.AddYGap();

    std::vector<std::string> matNames;
    matNames = MaterialRepo::GetInstance()->GetNames();

    m_MaterialChoice.AddItem( "DEFAULT" );
    for( int i = 0; i < (int) matNames.size(); i++ )
    {
        m_MaterialChoice.AddItem( matNames[i] );
    }
    m_GenLayout.AddChoice( m_MaterialChoice, "Material:" );
    m_GenLayout.AddYGap();

    m_ExportNameChoice.AddItem( "NONE" );
    m_ExportNameChoice.AddItem( "WING" );
    m_ExportNameChoice.AddItem( "FUSELAGE" );
    m_ExportNameChoice.AddItem( "TAIL" );
    m_ExportNameChoice.AddItem( "CANOPY" );
    m_ExportNameChoice.AddItem( "POD" );
    m_GenLayout.AddChoice( m_ExportNameChoice, "Export Name:" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Tesselation" );
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
    m_XFormLayout.SetFitWidthFlag( false );
    m_XFormLayout.SetSameLineFlag( true );

    m_XFormLayout.AddLabel( "Planar:", 74 );
    m_XFormLayout.SetButtonWidth( m_XFormLayout.GetRemainX() / 3 );
    m_XFormLayout.AddButton( m_XYSymToggle, "XY", vsp::SYM_XY );
    m_XFormLayout.AddButton( m_XZSymToggle, "XZ", vsp::SYM_XZ );
    m_XFormLayout.AddButton( m_YZSymToggle, "YZ", vsp::SYM_YZ );
    m_XFormLayout.ForceNewLine();
    m_XFormLayout.AddYGap();

    m_XFormLayout.AddLabel( "Axial:", 74 );
    m_XFormLayout.SetButtonWidth( m_XFormLayout.GetRemainX() / 4 );
    m_XFormLayout.AddButton( m_AxialNoneToggle, "None" );
    m_XFormLayout.AddButton( m_AxialXToggle, "X" );
    m_XFormLayout.AddButton( m_AxialYToggle, "Y" );
    m_XFormLayout.AddButton( m_AxialZToggle, "Z" );
    m_XFormLayout.ForceNewLine();

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

    m_XFormLayout.InitWidthHeightVals();
    m_XFormLayout.SetFitWidthFlag( true );
    m_XFormLayout.SetSameLineFlag( false );

    m_XFormLayout.AddSlider( m_AxialNSlider, "N", 100, " %5.0f" );
    m_XFormLayout.AddYGap();

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

    m_XFormLayout.SetFitWidthFlag( false );
    m_XFormLayout.SetSameLineFlag( true );

    m_XFormLayout.AddLabel( "Translate:", 74 );
    m_XFormLayout.SetButtonWidth( ( m_XFormLayout.GetRemainX() ) / 3 );
    m_XFormLayout.AddButton( m_TransNoneButton, "None" );
    m_XFormLayout.AddButton( m_TransCompButton, "Comp" );
    m_XFormLayout.AddButton( m_TransUVButton, "UV" );
    m_XFormLayout.ForceNewLine();
    m_XFormLayout.AddYGap();

    m_TransToggleGroup.Init( this );
    m_TransToggleGroup.AddButton( m_TransNoneButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransCompButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransUVButton.GetFlButton() );

    m_XFormLayout.AddLabel( "Rotate:", 74 );
    m_XFormLayout.AddButton( m_RotNoneButton, "None" );
    m_XFormLayout.AddButton( m_RotCompButton, "Comp" );
    m_XFormLayout.AddButton( m_RotUVButton, "UV" );
    m_XFormLayout.ForceNewLine();
    m_XFormLayout.AddYGap();

    m_RotToggleGroup.Init( this );
    m_RotToggleGroup.AddButton( m_RotNoneButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotCompButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotUVButton.GetFlButton() );

    m_XFormLayout.SetFitWidthFlag( true );
    m_XFormLayout.SetSameLineFlag( false );

    m_XFormLayout.AddSlider( m_AttachUSlider, "U", 1, " %5.4f" );
    m_XFormLayout.AddSlider( m_AttachVSlider, "V", 1, " %5.4f" );


    //=============== SubSurface Tab ===================//
    m_CurSubDispGroup = NULL;
    m_SubSurfLayout.SetGroupAndScreen( subsurf_group, this );
    m_SubSurfLayout.AddDividerBox( "Sub-Surface List" );

    int browser_h = 100;
    m_SubSurfBrowser = new Fl_Browser( m_SubSurfLayout.GetX(), m_SubSurfLayout.GetY(), m_SubSurfLayout.GetW(), browser_h );
    m_SubSurfBrowser->type( 1 );
    m_SubSurfBrowser->labelfont( 13 );
    m_SubSurfBrowser->labelsize( 12 );
    m_SubSurfBrowser->textsize( 12 );
    m_SubSurfBrowser->callback( staticScreenCB, this );
    subsurf_group->add( m_SubSurfBrowser );
    m_SubSurfLayout.AddY( browser_h );
    m_SubSurfLayout.AddYGap();

    m_SubSurfLayout.AddButton( m_DelSubSurfButton, "Delete" );
    m_SubSurfLayout.AddYGap();

    m_SubSurfLayout.SetFitWidthFlag( false );
    m_SubSurfLayout.SetSameLineFlag( true );

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( SubSurface::SS_LINE ) );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( SubSurface::SS_RECTANGLE ) );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( SubSurface::SS_ELLIPSE ) );

    int b_width = m_SubSurfLayout.GetRemainX();
    m_SubSurfLayout.SetButtonWidth( (int)(b_width * 0.4) );
    m_SubSurfLayout.SetChoiceButtonWidth( b_width / 5 );
    m_SubSurfLayout.SetSliderWidth( (int)(b_width * 0.4) );
    m_SubSurfLayout.AddChoice( m_SubSurfChoice, "Type" );
    m_SubSurfLayout.AddButton( m_AddSubSurfButton, "Add" );

    m_SubSurfLayout.SetFitWidthFlag( true );
    m_SubSurfLayout.SetSameLineFlag( false );
    m_SubSurfLayout.ForceNewLine();

    m_SubSurfLayout.AddYGap();

    m_SSCommonGroup.SetGroupAndScreen( AddSubGroup( subsurf_tab, 5 ), this );
    m_SSCommonGroup.SetY( m_SubSurfLayout.GetY() );
    m_SSCommonGroup.AddDividerBox( "Sub-Surface Parameters" );
    m_SSCommonGroup.AddInput( m_SubNameInput, "Name" );

    // Indivdual SubSurface Parameters
    int start_y = m_SSCommonGroup.GetY();

    //==== SSLine ====//
    m_SSLineGroup.SetGroupAndScreen( AddSubGroup( subsurf_tab, 5 ), this );
    m_SSLineGroup.SetY( start_y );

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
    m_SSRecGroup.SetGroupAndScreen( AddSubGroup( subsurf_tab, 5 ), this );
    m_SSRecGroup.SetY( start_y );
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
    m_SSRecGroup.AddSlider( m_SSRecDelUSlider, "Delta U", 1, "%5.4f" );
    m_SSRecGroup.AddSlider( m_SSRecDelWSlider, "Delta W", 1, "%5.4f" );
    m_SSRecGroup.AddSlider( m_SSRecThetaSlider, "Theta", 25, "%5.4f" );

    //==== SS_Ellipse ====//
    m_SSEllGroup.SetGroupAndScreen( AddSubGroup( subsurf_tab, 5 ), this );
    m_SSEllGroup.SetY( start_y );
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

    //==== Color ====//
    m_ColorPicker.Update( geom_ptr->GetColor() );

    //==== Material ====//
    Material * mat = geom_ptr->GetMaterial();

    m_MaterialChoice.SetVal( 0 );

    std::vector< std::string > choices = m_MaterialChoice.GetItems();
    for ( int i = 0; i < (int)choices.size(); i++ )
    {
        if( mat->GetName() == choices[i] )
        {
            m_MaterialChoice.SetVal(i);
            break;
        }
    }

    //==== XForms ====//
    m_ScaleSlider.Update( geom_ptr->m_Scale.GetID() );

    //===== Rel of Abs ====//
    m_XFormAbsRelToggle.Update( geom_ptr->m_AbsRelFlag.GetID() );
    geom_ptr->DeactivateXForms();
    if ( geom_ptr->m_AbsRelFlag() ==  GeomXForm::RELATIVE_XFORM )
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

    //==== Symmetry ====//
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
    geom_ptr->UpdateSets();
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
    SubSurface* subsurf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );

    if ( subsurf )
    {
        m_SubNameInput.Update( subsurf->GetName() );
        if ( subsurf->GetType() == SubSurface::SS_LINE )
        {
            SSLine* ssline = dynamic_cast< SSLine* >( subsurf );
            assert( ssline );

            m_SSLineConstToggleGroup.Update( ssline->m_ConstType.GetID() );
            m_SSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
            m_SSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
            SubSurfDispGroup( &m_SSLineGroup );

        }
        else if ( subsurf->GetType() == SubSurface::SS_RECTANGLE )
        {
            SSRectangle* ssrec = dynamic_cast< SSRectangle* >( subsurf );
            assert( subsurf );

            m_SSRecTestToggleGroup.Update( ssrec->m_TestType.GetID() );
            m_SSRecCentUSlider.Update( ssrec->m_CenterU.GetID() );
            m_SSRecCentWSlider.Update( ssrec->m_CenterW.GetID() );
            m_SSRecDelUSlider.Update( ssrec->m_DeltaU.GetID() );
            m_SSRecDelWSlider.Update( ssrec->m_DeltaW.GetID() );
            m_SSRecThetaSlider.Update( ssrec->m_Theta.GetID() );
            SubSurfDispGroup( &m_SSRecGroup );
        }
        else if ( subsurf->GetType() == SubSurface::SS_ELLIPSE )
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
    }
    else
    {
        SubSurfDispGroup( NULL );
    }

    //==== SubSurfBrowser ====//
    m_SubSurfBrowser->clear();
    static int widths[] = { 75, 75 };
    m_SubSurfBrowser->column_widths( widths );
    m_SubSurfBrowser->column_char( ':' );

    sprintf( str, "@b@.NAME:@b@.TYPE" );
    m_SubSurfBrowser->add( str );

    string ss_name, ss_type;

    vector<SubSurface*> subsurf_vec = geom_ptr->GetSubSurfVec();
    for ( int i = 0; i < ( int )subsurf_vec.size() ; i++ )
    {

        ss_name = subsurf_vec[i]->GetName();
        ss_type = SubSurface::GetTypeName( subsurf_vec[i]->GetType() );
        sprintf( str, "%s:%s", ss_name.c_str(), ss_type.c_str() );
        m_SubSurfBrowser->add( str );
    }

    if ( geom_ptr->ValidSubSurfInd( SubSurfaceMgr.GetCurrSurfInd() ) )
    {
        m_SubSurfBrowser->select( SubSurfaceMgr.GetCurrSurfInd() + 2 );
    }


    return true;
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

        MaterialRepo::MaterialPref mat;

        if( MaterialRepo::GetInstance()->FindMaterial( index, mat ) )
        {
            geom_ptr->SetMaterial( mat.name, mat.ambi, mat.diff, mat.spec, mat.emis, mat.shininess );
        }
        else
        {
            geom_ptr->SetMaterialToDefault();
        }
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
        if ( m_SubSurfChoice.GetVal() == SubSurface::SS_LINE )
        {
            ssurf = geom_ptr->AddSubSurf( SubSurface::SS_LINE );
        }
        else if ( m_SubSurfChoice.GetVal() == SubSurface::SS_RECTANGLE )
        {
            ssurf = geom_ptr->AddSubSurf( SubSurface::SS_RECTANGLE );
        }
        else if ( m_SubSurfChoice.GetVal() == SubSurface::SS_ELLIPSE )
        {
            ssurf = geom_ptr->AddSubSurf( SubSurface::SS_ELLIPSE );
        }

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
