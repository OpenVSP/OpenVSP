//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.cpp: implementation of the Vehicle Class and Vehicle Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#undef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

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
#include "SuperConeGeom.h"
#include "ParmMgr.h"
#include "Background3DMgr.h"
#include "RoutingGeom.h"

using namespace vsp;


using std::map;

#define MAX_WINDOW_PX_HEIGHT 800

//==== Constructor ====//
VspScreen::VspScreen( ScreenMgr* mgr )
{
    m_ScreenMgr = mgr;
    m_FLTK_Window = nullptr;
    m_ScreenType = -1;
}


//==== Destructor ====//
VspScreen::~VspScreen()
{

}

//==== Show Window ====//
void VspScreen::Show()
{
    if ( m_ScreenType >= 0 && m_ScreenType != VSP_COR_SCREEN )
    {
        if ( m_ScreenMgr->IsGUIScreenDisabled( m_ScreenType ) )
        {
            return;
        }
    }
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

bool VspScreen::Update()
{
    if ( m_ScreenType > 0 && m_ScreenType != VSP_COR_SCREEN )
    {
        if ( m_ScreenMgr->IsGUIScreenDisabled( m_ScreenType ) )
        {
            Hide();
        }
    }
    return false;
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
BasicScreen::BasicScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile  ) : VspScreen( mgr )
{
    if ( h > MAX_WINDOW_PX_HEIGHT )
    {
        printf( "Screen %s is too tall %d.\n", title.c_str(), h );
    }

    //==== Window ====//
    m_FLTK_Window = new VSP_Window( w, h );
    m_FLTK_Window->resizable( m_FLTK_Window );
    VspScreen::SetFlWindow( m_FLTK_Window );

    int reserve = 0;
    if ( !helpfile.empty() )
    {
        reserve = 20 + 4;
    }

    //==== Title Box ====//
    m_FL_TitleBox = new Fl_Box( 2, 2, w - 4 - reserve, 20 );
    m_FL_TitleBox->box( FL_ROUNDED_BOX );
    m_FL_TitleBox->labelfont( 1 );
    m_FL_TitleBox->labelsize( 16 );
    m_FL_TitleBox->labelcolor( FL_SELECTION_COLOR );

    if ( !helpfile.empty() )
    {
        m_MasterHelpButton = new Fl_Button( 2 + (w - 4 - reserve) + 4, 2, 20, 20, "?" );
        m_MasterHelpButton->labelfont( FL_HELVETICA_BOLD );
        m_MasterHelpButton->labelsize( 12 );
        m_MasterHelpButton->labelcolor( FL_DARK_BLUE );
        m_MasterHelpButton->align( FL_ALIGN_NOWRAP );
        m_MasterHelpButton->callback( staticHelpCB, this );

        m_HelpFile = helpfile;
    }
    else
    {
        m_MasterHelpButton = nullptr;
        m_HelpFile = "";
    }

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

void BasicScreen::HelpCallBack( Fl_Widget *w )
{
    if ( w == m_MasterHelpButton )
    {
        if ( m_ScreenMgr )
        {
            m_ScreenMgr->HelpSystemDefaultBrowser( m_HelpFile );
        }
    }
}

//=====================================================================//
//=====================================================================//
//=====================================================================//

//==== Constructor ====//
TabScreen::TabScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile, int baseymargin, int basexmargin ) :
    BasicScreen( mgr, w, h, title, helpfile )
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

    return nullptr;
}

//==== Add A Sub Group To Tab ====//
Fl_Group* TabScreen::AddSubGroup( Fl_Group* group, int border )
{
    if ( !group )
    {
        return nullptr;
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
Fl_Scroll* TabScreen::AddSubScroll( Fl_Group* group, int border, int lessh, int starty )
{
    if ( !group )
    {
        return nullptr;
    }

    int rx, ry, rw, rh;
    m_MenuTabs->client_area( rx, ry, rw, rh, TAB_H );

    int x = rx + border;
    int y = ry + border + starty;
    int w = rw - 2 * border;
    int h = rh - 2 * border - lessh - starty;

    Fl_Scroll* sub_group = new Fl_Scroll( x, y, w, h );
    sub_group->show();

    group->add( sub_group );

    return sub_group;
}

//=====================================================================//
//=====================================================================//
//=====================================================================//
VehScreen::VehScreen( ScreenMgr* mgr, int w, int h, const string & title ) :
    TabScreen( mgr, w, h, title )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    // Set the window as a geom screen window
    VSP_Window* vsp_win = dynamic_cast<VSP_Window*>(m_FLTK_Window);
    vsp_win->SetGeomScreenFlag( true );

    Fl_Group* attribute_tab = AddTab( "Attributes" );
    Fl_Group* attribute_group = AddSubGroup( attribute_tab, 5 );
    m_AttributeLayout.SetGroupAndScreen( attribute_group , this );
    m_AttributeEditor.Init( &m_AttributeLayout , attribute_group , this, staticScreenCB, false, 0, 250);
};

void VehScreen::Show( )
{
    if ( Update() )
    {
        VspScreen::Show( );
    }
};


bool VehScreen::Update()
{
    assert( m_ScreenMgr );

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        SetTitle( veh->GetName() );
    }

    TabScreen::Update();

    //==== Attributes ====//
    if ( veh )
    {
        m_AttributeEditor.SetEditorCollID( veh->m_AttrCollection.GetID() );
        m_AttributeEditor.Update();
    }
    return true;
};

void VehScreen::CallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );
    m_AttributeEditor.DeviceCB( w );
    m_ScreenMgr->SetUpdateFlag( true );
};

void VehScreen::CloseCallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
    Hide();
}

void VehScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );
    m_AttributeEditor.GuiDeviceCallBack( device );
    m_ScreenMgr->SetUpdateFlag( true );
};

void VehScreen::GetCollIDs( vector < string > &collIDVec )
{
    m_AttributeEditor.GetCollIDs( collIDVec );
}

//=====================================================================//
//=====================================================================//
//=====================================================================//
GeomScreen::GeomScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile ) :
    TabScreen( mgr, w, h, title, helpfile )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    // Set the window as a geom screen window
    VSP_Window* vsp_win = dynamic_cast<VSP_Window*>(m_FLTK_Window);

    vsp_win->SetGeomScreenFlag( true );

    Fl_Group* gen_tab = AddTab( "Gen" );
    Fl_Group* xform_tab = AddTab( "XForm" );
    Fl_Group* massprop_tab = AddTab( "Mass" );
    Fl_Group* subsurf_tab = AddTab( "Sub" );
    m_MassPropTab_ind = m_TabGroupVec.size() - 2;
    m_SubSurfTab_ind = m_TabGroupVec.size() - 1;
    Fl_Group* gen_group = AddSubGroup( gen_tab, 5 );
    Fl_Group* xform_group = AddSubGroup( xform_tab, 5 );
    Fl_Group* massprop_group = AddSubGroup( massprop_tab, 5 );
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

    //=== Negative Volumes ===//
    m_GenLayout.AddDividerBox( "CFDMesh Negative Volume" );

    m_GenLayout.AddButton( m_NegativeVolumeBtn, "Negative Volume" );
    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Set Export/Analysis" );
    int attrColHt = 130;
    int buffer = 20;
    int remain_y = ( m_GenLayout.GetH() + m_GenLayout.GetStartY() ) - m_GenLayout.GetY() - attrColHt - buffer ;
    m_SetBrowser = m_GenLayout.AddCheckBrowser( remain_y );
    m_SetBrowser->callback( staticCB, this );
    m_GenLayout.AddY( m_SetBrowser->h() - 21 );

    // //===== Attributes ====//
    m_GenLayout.AddYGap();
    m_AttributeEditor.Init( &m_GenLayout, gen_group, this, staticScreenCB, true, m_GenLayout.GetY(), attrColHt);

    gen_tab->show();

    //==== XForm Layout ====//
    m_XFormLayout.SetGroupAndScreen( xform_group, this );
    m_XFormLayout.AddDividerBox( "Transforms" );


    m_XFormLayout.SetFitWidthFlag( false );
    m_XFormLayout.SetSameLineFlag( true );
    m_XFormLayout.AddLabel( "Coord System:", m_XFormLayout.GetW() - 2 * m_XFormLayout.GetInputWidth() );
    m_XFormLayout.SetButtonWidth( m_XFormLayout.GetInputWidth() );
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

    m_XFormLayout.AddSubGroupLayout( m_AttachLayout, m_XFormLayout.GetW(), 11 * m_AttachLayout.GetStdHeight() + 5 * m_AttachLayout.GetGapHeight() );

    m_AttachLayout.SetFitWidthFlag( false );
    m_AttachLayout.SetSameLineFlag( true );

    char etaMN[7];
    int indx = 0;
    indx += fl_utf8encode( 951, &etaMN[ indx ] ); // Greek character eta
    etaMN[ indx ] = 'M';
    etaMN[ indx + 1 ] = 'N';
    etaMN[ indx + 2 ] = 0;

    m_AttachLayout.AddLabel( "Translate:", 74 );
    m_AttachLayout.SetButtonWidth( ( m_AttachLayout.GetRemainX() ) / 6 );
    m_AttachLayout.AddButton( m_TransNoneButton, "None" );
    m_AttachLayout.AddButton( m_TransCompButton, "Comp" );
    m_AttachLayout.AddButton( m_TransUVButton, "UW" );
    m_AttachLayout.AddButton( m_TransRSTButton, "RST" );
    m_AttachLayout.AddButton( m_TransLMNButton, "LMN" );
    m_AttachLayout.AddButton( m_TransEtaMNButton, etaMN );
    m_AttachLayout.ForceNewLine();
    m_AttachLayout.AddYGap();

    m_TransToggleGroup.Init( this );
    m_TransToggleGroup.AddButton( m_TransNoneButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransCompButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransUVButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransRSTButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransLMNButton.GetFlButton() );
    m_TransToggleGroup.AddButton( m_TransEtaMNButton.GetFlButton() );

    m_AttachLayout.AddLabel( "Rotate:", 74 );
    m_AttachLayout.AddButton( m_RotNoneButton, "None" );
    m_AttachLayout.AddButton( m_RotCompButton, "Comp" );
    m_AttachLayout.AddButton( m_RotUVButton, "UW" );
    m_AttachLayout.AddButton( m_RotRSTButton, "RST" );
    m_AttachLayout.AddButton( m_RotLMNButton, "LMN" );
    m_AttachLayout.AddButton( m_RotEtaMNButton, etaMN );
    m_AttachLayout.ForceNewLine();
    m_AttachLayout.AddYGap();

    m_RotToggleGroup.Init( this );
    m_RotToggleGroup.AddButton( m_RotNoneButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotCompButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotUVButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotRSTButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotLMNButton.GetFlButton() );
    m_RotToggleGroup.AddButton( m_RotEtaMNButton.GetFlButton() );

    m_AttachLayout.SetFitWidthFlag( true );
    m_AttachLayout.SetSameLineFlag( false );

    int actionToggleButtonWidth = 35;
    int normalButtonWidth = 90;

    m_AttachLayout.SetFitWidthFlag( false );
    m_AttachLayout.SetSameLineFlag( true );

    m_AttachLayout.SetButtonWidth( actionToggleButtonWidth );
    m_AttachLayout.AddButton( m_U01Toggle, "01" );
    m_AttachLayout.AddButton( m_U0NToggle, "0N" );

    m_AttachLayout.SetFitWidthFlag( true );

    m_AttachLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_AttachLayout.AddSlider( m_AttachUSlider, "U", 1, " %7.6f" );

    m_UScaleToggleGroup.Init( this );
    m_UScaleToggleGroup.AddButton( m_U0NToggle.GetFlButton() ); // 0 false added first
    m_UScaleToggleGroup.AddButton( m_U01Toggle.GetFlButton() ); // 1 true added second

    m_AttachLayout.ForceNewLine();
    m_AttachLayout.SetFitWidthFlag( true );
    m_AttachLayout.SetSameLineFlag( false );

    m_AttachLayout.SetButtonWidth( normalButtonWidth );

    m_AttachLayout.AddSlider( m_AttachVSlider, "W", 1, " %7.6f" );
    m_AttachLayout.AddYGap();

    m_AttachLayout.SetFitWidthFlag( false );
    m_AttachLayout.SetSameLineFlag( true );

    m_AttachLayout.SetButtonWidth( actionToggleButtonWidth );
    m_AttachLayout.AddButton( m_R01Toggle, "01" );
    m_AttachLayout.AddButton( m_R0NToggle, "0N" );

    m_AttachLayout.SetFitWidthFlag( true );

    m_AttachLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_AttachLayout.AddSlider( m_AttachRSlider, "R", 1, " %7.6f" );

    m_RScaleToggleGroup.Init( this );
    m_RScaleToggleGroup.AddButton( m_R0NToggle.GetFlButton() ); // 0 false added first
    m_RScaleToggleGroup.AddButton( m_R01Toggle.GetFlButton() ); // 1 true added second

    m_AttachLayout.ForceNewLine();
    m_AttachLayout.SetFitWidthFlag( true );
    m_AttachLayout.SetSameLineFlag( false );

    m_AttachLayout.SetButtonWidth( normalButtonWidth );

    m_AttachLayout.AddSlider( m_AttachSSlider, "S", 1, " %7.6f" );
    m_AttachLayout.AddSlider( m_AttachTSlider, "T", 1, " %7.6f" );
    m_AttachLayout.AddYGap();

    m_AttachLayout.SetFitWidthFlag( false );
    m_AttachLayout.SetSameLineFlag( true );

    m_AttachLayout.SetButtonWidth( actionToggleButtonWidth );
    m_AttachLayout.AddButton( m_L01Toggle, "01" );
    m_AttachLayout.AddButton( m_L0LenToggle, "0D" );

    m_AttachLayout.SetFitWidthFlag( true );

    m_AttachLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_AttachLayout.AddSlider( m_AttachLSlider, "L", 1, " %7.6f" );

    m_LScaleToggleGroup.Init( this );
    m_LScaleToggleGroup.AddButton( m_L0LenToggle.GetFlButton() ); // 0 false added first
    m_LScaleToggleGroup.AddButton( m_L01Toggle.GetFlButton() ); // 1 true added second

    m_AttachLayout.ForceNewLine();
    m_AttachLayout.SetFitWidthFlag( true );
    m_AttachLayout.SetSameLineFlag( false );

    m_AttachLayout.SetButtonWidth( normalButtonWidth );

    char eta[5];
    indx = 0;
    indx += fl_utf8encode( 951, &eta[ indx ] ); // Greek character eta
    eta[ indx ] = 0;

    m_AttachLayout.AddSlider( m_AttachEtaSlider, eta, 1, " %7.6f" );

    m_AttachLayout.AddSlider( m_AttachMSlider, "M", 1, " %7.6f" );
    m_AttachLayout.AddSlider( m_AttachNSlider, "N", 1, " %7.6f" );


    //=============== SubSurface Tab ===================//
    m_CurSubDispGroup = nullptr;
    m_SubSurfLayout.SetGroupAndScreen( subsurf_group, this );
    m_SubSurfLayout.AddDividerBox( "Sub-Surface List" );

    // Initial column widths
    static int col_widths[] = { m_SubSurfLayout.GetW() / 2, m_SubSurfLayout.GetW() / 3, m_SubSurfLayout.GetW() / 6, 0 }; // 3 columns

    int browser_h = 100;
    int attr_h = 130;

    m_SubSurfBrowser = m_SubSurfLayout.AddColResizeBrowser( col_widths, 3, browser_h );
    m_SubSurfBrowser->callback( staticScreenCB, this );

    m_SubSurfLayout.SetSameLineFlag( true );
    m_SubSurfLayout.AddButton( m_AddSubSurfButton, "Add", m_SubSurfLayout.GetW() * 0.5 );
    m_SubSurfLayout.AddButton( m_DelSubSurfButton, "Delete", m_SubSurfLayout.GetW() * 0.5 );
    m_SubSurfLayout.ForceNewLine();
    m_SubSurfLayout.SetSameLineFlag( false );
    m_SubSurfLayout.AddYGap();

    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_LINE ), vsp::SS_LINE );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_RECTANGLE ), vsp::SS_RECTANGLE );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_ELLIPSE ), vsp::SS_ELLIPSE );
// Only add control surface in WingScreen.
//    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_CONTROL ), vsp::SS_CONTROL );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_FINITE_LINE ), vsp::SS_FINITE_LINE );
    m_SubSurfChoice.AddItem( SubSurface::GetTypeName( vsp::SS_XSEC_CURVE ), vsp::SS_XSEC_CURVE );

    m_SubSurfLayout.SetChoiceButtonWidth( m_SubSurfLayout.GetRemainX() / 3 );

    m_SubSurfLayout.AddChoice( m_SubSurfChoice, "Type" );
    m_SubSurfLayout.AddChoice( m_SubSurfSelectSurface, "Surface" );

    m_SSCurrMainSurfIndx = -1;

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

    m_SSLineGroup.AddLabel( "Scale", remain_x / 3 );
    m_SSLineGroup.AddButton( m_SSLine01Toggle, "[0, 1]" );
    m_SSLineGroup.AddButton( m_SSLine0NToggle, "[0, N]" );

    m_SSLineScaleToggleGroup.Init( this );
    m_SSLineScaleToggleGroup.AddButton( m_SSLine0NToggle.GetFlButton() ); // 0 false added first
    m_SSLineScaleToggleGroup.AddButton( m_SSLine01Toggle.GetFlButton() ); // 1 true added second

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
    m_SSLineGroup.AddSlider( m_SSLineConstSlider, "Value01", 1, "%7.6f" );
    m_SSLineGroup.AddSlider( m_SSLineConstSlider0N, "Value0N", 1, "%7.6f" );

    m_SSLineGroup.AddYGap();
    m_SSLineAttrEditor.Init( &m_SSLineGroup , subsurf_group , this, staticScreenCB, true , m_SSLineGroup.GetY() , attr_h);

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

    m_SSRecGroup.AddSlider( m_SSRecCentUSlider, "Center U", 1, "%7.6f" );
    m_SSRecGroup.AddSlider( m_SSRecCentWSlider, "Center W", 1, "%7.6f" );
    m_SSRecGroup.AddSlider( m_SSRecULenSlider, "U Length", 1, "%7.6f" );
    m_SSRecGroup.AddSlider( m_SSRecWLenSlider, "W Length", 1, "%7.6f" );
    m_SSRecGroup.AddSlider( m_SSRecThetaSlider, "Theta", 25, "%7.6f" );

    m_SSRecGroup.AddYGap();
    m_SSRecAttrEditor.Init( &m_SSRecGroup , subsurf_group , this , staticScreenCB, true , m_SSRecGroup.GetY() , attr_h);

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
    m_SSEllGroup.AddSlider( m_SSEllCentUSlider, "Center U", 1, "%7.6f" );
    m_SSEllGroup.AddSlider( m_SSEllCentWSlider, "Center W", 1, "%7.6f" );
    m_SSEllGroup.AddSlider( m_SSEllULenSlider, "U Length", 1, "%7.6f" );
    m_SSEllGroup.AddSlider( m_SSEllWLenSlider, "W Length", 1, "%7.6f" );
    m_SSEllGroup.AddSlider( m_SSEllThetaSlider, "Theta", 25, "%7.6f" );

    m_SSEllGroup.AddYGap();
    m_SSEllAttrEditor.Init( &m_SSEllGroup , subsurf_group , this , staticScreenCB, true , m_SSEllGroup.GetY() , attr_h);

    //===== SSXsecCurve =====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSXSCGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );

    remain_x = m_SSEllGroup.GetRemainX();

    m_SSXSCGroup.SetFitWidthFlag( false );
    m_SSXSCGroup.SetSameLineFlag( true );
    m_SSXSCGroup.AddLabel( "Tag", remain_x / 3 );
    m_SSXSCGroup.SetButtonWidth( remain_x / 3 );
    m_SSXSCGroup.AddButton( m_SSXSCInsideButton, "Inside" );
    m_SSXSCGroup.AddButton( m_SSXSCOutsideButton, "Outside" );

    m_SSXSCTestToggleGroup.Init( this );
    m_SSXSCTestToggleGroup.AddButton( m_SSXSCInsideButton.GetFlButton() );
    m_SSXSCTestToggleGroup.AddButton( m_SSXSCOutsideButton.GetFlButton() );

    m_SSXSCGroup.SetFitWidthFlag( true );
    m_SSXSCGroup.SetSameLineFlag( false );
    m_SSXSCGroup.ForceNewLine();


    m_SSXSCGroup.AddSlider( m_SSXSCCentUSlider, "Center U", 1, "%7.6f" );
    m_SSXSCGroup.AddSlider( m_SSXSCCentWSlider, "Center W", 1, "%7.6f" );


    m_SSXSCGroup.AddYGap();

    m_SSXSCGroup.AddDividerBox( "Type" );

    m_SSXSecTypeChoice.AddItem( "POINT", vsp::XS_POINT );
    m_SSXSecTypeChoice.AddItem( "CIRCLE", vsp::XS_CIRCLE );
    m_SSXSecTypeChoice.AddItem( "ELLIPSE", vsp::XS_ELLIPSE );
    m_SSXSecTypeChoice.AddItem( "SUPER_ELLIPSE", vsp::XS_SUPER_ELLIPSE );
    m_SSXSecTypeChoice.AddItem( "ROUNDED_RECTANGLE", vsp::XS_ROUNDED_RECTANGLE );
    m_SSXSecTypeChoice.AddItem( "GENERAL_FUSE", vsp::XS_GENERAL_FUSE );
    m_SSXSecTypeChoice.AddItem( "FUSE_FILE", vsp::XS_FILE_FUSE );
    m_SSXSecTypeChoice.AddItem( "FOUR_SERIES", vsp::XS_FOUR_SERIES );
    m_SSXSecTypeChoice.AddItem( "SIX_SERIES", vsp::XS_SIX_SERIES );
    m_SSXSecTypeChoice.AddItem( "BICONVEX", vsp::XS_BICONVEX );
    m_SSXSecTypeChoice.AddItem( "WEDGE", vsp::XS_WEDGE );
    m_SSXSecTypeChoice.AddItem( "EDIT_CURVE", vsp::XS_EDIT_CURVE );
    m_SSXSecTypeChoice.AddItem( "AF_FILE", vsp::XS_FILE_AIRFOIL );
    m_SSXSecTypeChoice.AddItem( "CST_AIRFOIL", vsp::XS_CST_AIRFOIL );
    m_SSXSecTypeChoice.AddItem( "KARMAN_TREFFTZ", vsp::XS_VKT_AIRFOIL );
    m_SSXSecTypeChoice.AddItem( "FOUR_DIGIT_MOD", vsp::XS_FOUR_DIGIT_MOD );
    m_SSXSecTypeChoice.AddItem( "FIVE_DIGIT", vsp::XS_FIVE_DIGIT );
    m_SSXSecTypeChoice.AddItem( "FIVE_DIGIT_MOD", vsp::XS_FIVE_DIGIT_MOD );
    m_SSXSecTypeChoice.AddItem( "16_SERIES", vsp::XS_ONE_SIX_SERIES );

    m_SSXSCGroup.SetFitWidthFlag( true );
    m_SSXSCGroup.SetSameLineFlag( false );

    m_SSXSCGroup.AddChoice( m_SSXSecTypeChoice, "Choose Type:" );

    m_SSXSCGroup.SetFitWidthFlag( false );
    m_SSXSCGroup.SetSameLineFlag( true );

    m_SSXSCGroup.SetButtonWidth( m_SSXSCGroup.GetW() / 2 );
    m_SSXSCGroup.AddButton( m_SSXSCShowXSecButton, "Show" );

    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCConvertCEDITGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetStdHeight() );
    m_SSXSCConvertCEDITGroup.SetButtonWidth( m_SSXSCGroup.GetW() / 2 );
    m_SSXSCConvertCEDITGroup.SetFitWidthFlag( false );
    m_SSXSCConvertCEDITGroup.AddButton( m_SSXSCConvertCEDITButton, "Convert CEDIT" );

    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCEditCEDITGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetStdHeight() );
    m_SSXSCEditCEDITGroup.SetFitWidthFlag( false );
    m_SSXSCEditCEDITGroup.SetButtonWidth( m_SSXSCGroup.GetW() / 2 );
    m_SSXSCEditCEDITGroup.AddButton( m_SSXSCEditCEDITButton, "Edit Curve" );

    m_SSXSCGroup.ForceNewLine();

    m_SSXSCGroup.SetFitWidthFlag( true );
    m_SSXSCGroup.SetSameLineFlag( false );

    //==== Circle XSec ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCCircleGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCCircleGroup.AddSlider(  m_SSXSCDiameterSlider, "Diameter", 10, "%6.5f" );

    //==== Ellipse XSec ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCEllipseGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCEllipseGroup.AddSlider(  m_SSXSCEllipseHeightSlider, "Height", 10, "%6.5f" );
    m_SSXSCEllipseGroup.AddSlider(  m_SSXSCEllipseWidthSlider, "Width", 10, "%6.5f" );

    //==== Super XSec ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCSuperGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperHeightSlider, "Height", 10, "%6.5f" );
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperWidthSlider,  "Width", 10, "%6.5f" );
    m_SSXSCSuperGroup.AddYGap();
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperMaxWidthLocSlider, "MaxWLoc", 2, "%6.5f" );
    m_SSXSCSuperGroup.AddYGap();
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperMSlider, "M", 10, "%6.5f" );
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperNSlider, "N", 10, "%6.5f" );
    m_SSXSCSuperGroup.AddYGap();
    m_SSXSCSuperGroup.AddButton( m_SSXSCSuperToggleSym, "T/B Symmetric Exponents" );
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperM_botSlider, "M Bottom", 10, "%6.5f" );
    m_SSXSCSuperGroup.AddSlider( m_SSXSCSuperN_botSlider, "N Bottom", 10, "%6.5f" );

    //==== Rounded Rect ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCRoundedRectGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRHeightSlider, "Height", 10, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRWidthSlider,  "Width", 10, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddYGap();
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRSkewSlider, "Skew", 2, "%6.5f");
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRVSkewSlider, "VSkew", 2, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRKeystoneSlider, "Keystone", 1, "%6.5f");

    m_SSXSCRoundedRectGroup.AddYGap();
    m_SSXSCRoundedRectGroup.SetSameLineFlag( true );
    m_SSXSCRoundedRectGroup.SetFitWidthFlag( false );

    int oldbw = m_SSXSCRoundedRectGroup.GetButtonWidth();

    m_SSXSCRoundedRectGroup.AddLabel( "Symmetry:", oldbw );

    m_SSXSCRoundedRectGroup.SetButtonWidth( m_SSXSCRoundedRectGroup.GetRemainX() / 4 );
    m_SSXSCRoundedRectGroup.AddButton( m_SSXSCRRRadNoSymToggle, "None" );
    m_SSXSCRoundedRectGroup.AddButton( m_SSXSCRRRadRLSymToggle, "R//L" );
    m_SSXSCRoundedRectGroup.AddButton( m_SSXSCRRRadTBSymToggle, "T//B" );
    m_SSXSCRoundedRectGroup.AddButton( m_SSXSCRRRadAllSymToggle, "All" );

    m_SSXSCRRRadSymRadioGroup.Init( this );
    m_SSXSCRRRadSymRadioGroup.AddButton( m_SSXSCRRRadNoSymToggle.GetFlButton() );
    m_SSXSCRRRadSymRadioGroup.AddButton( m_SSXSCRRRadRLSymToggle.GetFlButton() );
    m_SSXSCRRRadSymRadioGroup.AddButton( m_SSXSCRRRadTBSymToggle.GetFlButton() );
    m_SSXSCRRRadSymRadioGroup.AddButton( m_SSXSCRRRadAllSymToggle.GetFlButton() );

    m_SSXSCRoundedRectGroup.ForceNewLine();
    m_SSXSCRoundedRectGroup.SetSameLineFlag( false );
    m_SSXSCRoundedRectGroup.SetFitWidthFlag( true );

    m_SSXSCRoundedRectGroup.SetButtonWidth( oldbw );

    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRRadiusTRSlider, "TR Radius", 10, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRRadiusTLSlider, "TL Radius", 10, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRRadiusBLSlider, "BL Radius", 10, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddSlider( m_SSXSCRRRadiusBRSlider, "BR Radius", 10, "%6.5f" );
    m_SSXSCRoundedRectGroup.AddYGap();

    m_SSXSCRoundedRectGroup.AddButton( m_SSXSCRRKeyCornerButton, "Key Corner" );

    //==== General Fuse XSec ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCGenGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenHeightSlider, "Height", 10, "%6.5f" );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenWidthSlider, "Width", 10, "%6.5f" );
    m_SSXSCGenGroup.AddYGap();
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenMaxWidthLocSlider, "MaxWLoc", 1, "%6.5f" );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenCornerRadSlider, "CornerRad", 1, "%6.5f" );
    m_SSXSCGenGroup.AddYGap();
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenTopTanAngleSlider, "TopTanAng", 90, "%7.5f" );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenBotTanAngleSlider, "BotTanAng", 90, "%7.5f" );
    m_SSXSCGenGroup.AddYGap();
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenTopStrSlider, "TopStr", 1, "%7.5f" );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenBotStrSlider, "BotStr", 1, "%7.5f" );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenUpStrSlider, "UpStr", 1, "%7.5f" );
    m_SSXSCGenGroup.AddSlider( m_SSXSCGenLowStrSlider, "LowStr", 1, "%7.5f" );

    //==== Four Series AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCFourSeriesGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCFourSeriesGroup.AddOutput( m_SSXSCFourNameOutput, "Name" );
    m_SSXSCFourSeriesGroup.AddYGap();
    m_SSXSCFourSeriesGroup.AddSlider( m_SSXSCFourChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCFourSeriesGroup.AddSlider( m_SSXSCFourThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCFourSeriesGroup.AddYGap();

    actionToggleButtonWidth = 15;
    int actionSliderButtonWidth = m_SSXSCFourSeriesGroup.GetButtonWidth() - actionToggleButtonWidth;

    m_SSXSCFourSeriesGroup.SetSameLineFlag( true );

    m_SSXSCFourSeriesGroup.SetFitWidthFlag( false );
    m_SSXSCFourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_SSXSCFourSeriesGroup.AddButton( m_SSXSCFourCamberButton, "" );
    m_SSXSCFourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_SSXSCFourSeriesGroup.SetFitWidthFlag( true );
    m_SSXSCFourSeriesGroup.AddSlider( m_SSXSCFourCamberSlider, "Camber", 0.2, "%7.5f" );

    m_SSXSCFourSeriesGroup.ForceNewLine();

    m_SSXSCFourSeriesGroup.SetFitWidthFlag( false );
    m_SSXSCFourSeriesGroup.SetButtonWidth( actionToggleButtonWidth );
    m_SSXSCFourSeriesGroup.AddButton( m_SSXSCFourCLiButton, "" );
    m_SSXSCFourSeriesGroup.SetButtonWidth( actionSliderButtonWidth );
    m_SSXSCFourSeriesGroup.SetFitWidthFlag( true );
    m_SSXSCFourSeriesGroup.AddSlider( m_SSXSCFourCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_SSXSCFourSeriesGroup.ForceNewLine();

    m_SSXSCFourSeriesGroup.SetSameLineFlag( false );
    m_SSXSCFourSeriesGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_SSXSCFourCamberGroup.Init( this );
    m_SSXSCFourCamberGroup.AddButton( m_SSXSCFourCamberButton.GetFlButton() );
    m_SSXSCFourCamberGroup.AddButton( m_SSXSCFourCLiButton.GetFlButton() );

    vector< int > camb_val_map;
    camb_val_map.push_back( vsp::MAX_CAMB );
    camb_val_map.push_back( vsp::DESIGN_CL );
    m_SSXSCFourCamberGroup.SetValMapVec( camb_val_map );

    m_SSXSCFourSeriesGroup.AddSlider( m_SSXSCFourCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_SSXSCFourSeriesGroup.AddYGap();
    m_SSXSCFourSeriesGroup.AddButton( m_SSXSCFourInvertButton, "Invert Airfoil" );
    m_SSXSCFourSeriesGroup.AddYGap();
    m_SSXSCFourSeriesGroup.AddButton( m_SSXSCFourSharpTEButton, "Sharpen TE" );

    m_SSXSCFourSeriesGroup.AddYGap();
    m_SSXSCFourSeriesGroup.SetSameLineFlag( true );
    m_SSXSCFourSeriesGroup.SetFitWidthFlag( false );
    m_SSXSCFourSeriesGroup.SetButtonWidth( 125 );
    m_SSXSCFourSeriesGroup.AddButton( m_SSXSCFourFitCSTButton, "Fit CST" );
    m_SSXSCFourSeriesGroup.InitWidthHeightVals();
    m_SSXSCFourSeriesGroup.SetFitWidthFlag( true );
    m_SSXSCFourSeriesGroup.AddCounter( m_SSXSCFourDegreeCounter, "Degree", 125 );

    //==== Six Series AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCSixSeriesGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCSixSeriesGroup.AddOutput( m_SSXSCSixNameOutput, "Name" );
    m_SSXSCSixSeriesGroup.AddYGap();

    m_SSXSCSixSeriesChoice.AddItem( "63-" );
    m_SSXSCSixSeriesChoice.AddItem( "64-" );
    m_SSXSCSixSeriesChoice.AddItem( "65-" );
    m_SSXSCSixSeriesChoice.AddItem( "66-" );
    m_SSXSCSixSeriesChoice.AddItem( "67-" );
    m_SSXSCSixSeriesChoice.AddItem( "63A" );
    m_SSXSCSixSeriesChoice.AddItem( "64A" );
    m_SSXSCSixSeriesChoice.AddItem( "65A" );
    m_SSXSCSixSeriesGroup.AddChoice( m_SSXSCSixSeriesChoice, "Series" );

    m_SSXSCSixSeriesGroup.AddYGap();

    m_SSXSCSixSeriesGroup.AddSlider( m_SSXSCSixChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCSixSeriesGroup.AddSlider( m_SSXSCSixThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCSixSeriesGroup.AddYGap();
    m_SSXSCSixSeriesGroup.AddSlider( m_SSXSCSixIdealClSlider, "Ideal CL", 1, "%7.5f" );
    m_SSXSCSixSeriesGroup.AddSlider( m_SSXSCSixASlider, "a", 1, "%7.5f" );
    m_SSXSCSixSeriesGroup.AddYGap();
    m_SSXSCSixSeriesGroup.AddButton( m_SSXSCSixInvertButton, "Invert Airfoil" );
    m_SSXSCSixSeriesGroup.AddYGap();
    m_SSXSCSixSeriesGroup.SetSameLineFlag( true );
    m_SSXSCSixSeriesGroup.SetFitWidthFlag( false );
    m_SSXSCSixSeriesGroup.SetButtonWidth( 125 );
    m_SSXSCSixSeriesGroup.AddButton( m_SSXSCSixFitCSTButton, "Fit CST" );
    m_SSXSCSixSeriesGroup.InitWidthHeightVals();
    m_SSXSCSixSeriesGroup.SetFitWidthFlag( true );
    m_SSXSCSixSeriesGroup.AddCounter( m_SSXSCSixDegreeCounter, "Degree", 125 );

    //==== Biconvex AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCBiconvexGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCBiconvexGroup.AddSlider( m_SSXSCBiconvexChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCBiconvexGroup.AddSlider( m_SSXSCBiconvexThickChordSlider, "T/C", 1, "%7.5f" );

    //==== Wedge AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCWedgeGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCWedgeGroup.AddYGap();
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeThickLocSlider, "Thick X", 1, "%7.5f" );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeFlatUpSlider, "Flat Up", 1, "%7.5f" );
    m_SSXSCWedgeGroup.AddYGap();
    m_SSXSCWedgeGroup.AddButton( m_SSXSCWedgeSymmThickButton, "Symm Thickness" );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeThickLocLowSlider, "Thick X Low", 1, "%7.5f" );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeFlatLowSlider, "Flat Low", 1, "%7.5f" );
    m_SSXSCWedgeGroup.AddYGap();
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeZCamberSlider, "Camber", 1, "%7.5f" );
    m_SSXSCWedgeGroup.AddYGap();
    m_SSXSCWedgeGroup.SetSameLineFlag( true );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeUForeUpSlider, "U Fwd Up", 1, "%7.5f", m_SSXSCWedgeGroup.GetW() * 0.5 );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeDuUpSlider, "dU Flat Up", 1, "%7.5f" );
    m_SSXSCWedgeGroup.ForceNewLine();
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeUForeLowSlider, "U Fwd Low", 1, "%7.5f", m_SSXSCWedgeGroup.GetW() * 0.5 );
    m_SSXSCWedgeGroup.AddSlider( m_SSXSCWedgeDuLowSlider, "dU Flat Low", 1, "%7.5f" );
    m_SSXSCWedgeGroup.ForceNewLine();
    m_SSXSCWedgeGroup.SetSameLineFlag( false );
    m_SSXSCWedgeGroup.AddYGap();
    m_SSXSCWedgeGroup.AddButton( m_SSXSCWedgeInvertButton, "Invert Airfoil" );

    //==== Fuse File ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCFuseFileGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCFuseFileGroup.AddButton( m_SSXSCReadFuseFileButton, "Read File" );
    m_SSXSCFuseFileGroup.AddYGap();
    m_SSXSCFuseFileGroup.AddSlider( m_SSXSCFileHeightSlider, "Height", 10, "%7.3f" );
    m_SSXSCFuseFileGroup.AddSlider( m_SSXSCFileWidthSlider, "Width", 10, "%7.3f" );

    //==== Airfoil File ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCAfFileGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCAfFileGroup.AddButton( m_SSXSCAfReadFileButton, "Read File" );
    m_SSXSCAfFileGroup.AddYGap();
    m_SSXSCAfFileGroup.AddOutput( m_SSXSCAfFileNameOutput, "Name" );
    m_SSXSCAfFileGroup.AddYGap();
    m_SSXSCAfFileGroup.AddSlider( m_SSXSCAfFileChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCAfFileGroup.AddSlider( m_SSXSCAfFileThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCAfFileGroup.AddOutput( m_SSXSCAfFileBaseThickChordOutput, "Base T/C", "%7.5f" );
    m_SSXSCAfFileGroup.AddYGap();
    m_SSXSCAfFileGroup.AddButton( m_SSXSCAfFileInvertButton, "Invert Airfoil" );
    m_SSXSCAfFileGroup.AddYGap();
    m_SSXSCAfFileGroup.SetSameLineFlag( true );
    m_SSXSCAfFileGroup.SetFitWidthFlag( false );
    m_SSXSCAfFileGroup.SetButtonWidth( 125 );
    m_SSXSCAfFileGroup.AddButton( m_SSXSCAfFileFitCSTButton, "Fit CST" );
    m_SSXSCAfFileGroup.InitWidthHeightVals();
    m_SSXSCAfFileGroup.SetFitWidthFlag( true );
    m_SSXSCAfFileGroup.AddCounter( m_SSXSCAfFileDegreeCounter, "Degree", 125 );

    //==== CST Airfoil ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCCSTAirfoilGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );

    m_SSXSCCSTAirfoilGroup.AddOutput( m_SSXSCCSTThickChordOutput, "T/C", "%7.5f" );

    m_SSXSCCSTAirfoilGroup.SetSameLineFlag( true );
    m_SSXSCCSTAirfoilGroup.AddButton( m_SSXSCCSTContLERadButton, "Enforce Continuous LE Radius", m_SSXSCCSTAirfoilGroup.GetW() * 0.5 );
    m_SSXSCCSTAirfoilGroup.AddButton( m_SSXSCCSTInvertButton, "Invert Airfoil", m_SSXSCCSTAirfoilGroup.GetW() * 0.5 );
    m_SSXSCCSTAirfoilGroup.ForceNewLine();
    m_SSXSCCSTAirfoilGroup.SetSameLineFlag( false );

    m_SSXSCCSTAirfoilGroup.AddSlider( m_SSXSCCSTChordSlider, "Chord", 10, "%7.3f");

    m_SSXSCCSTAirfoilGroup.AddYGap();
    m_SSXSCCSTAirfoilGroup.AddDividerBox( "Upper Surface" );

    m_SSXSCCSTAirfoilGroup.SetSameLineFlag( true );
    m_SSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_SSXSCCSTAirfoilGroup.AddOutput( m_SSXSCUpDegreeOutput, "Degree", m_SSXSCCSTAirfoilGroup.GetButtonWidth() * 2 );
    m_SSXSCCSTAirfoilGroup.SetFitWidthFlag( false );
    m_SSXSCCSTAirfoilGroup.AddButton( m_SSXSCUpDemoteButton, "Demote" );
    m_SSXSCCSTAirfoilGroup.AddButton( m_SSXSCUpPromoteButton, "Promote" );

    m_SSXSCCSTAirfoilGroup.ForceNewLine();

    m_SSXSCCSTAirfoilGroup.SetSameLineFlag( false );
    m_SSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_SSXSCCSTUpCoeffScroll = m_SSXSCCSTAirfoilGroup.AddFlScroll( 60 );

    m_SSXSCCSTUpCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_SSXSCCSTUpCoeffScroll->box( FL_BORDER_BOX );
    m_SSXSCCSTUpCoeffLayout.SetGroupAndScreen( m_SSXSCCSTUpCoeffScroll, this );

    m_SSXSCCSTAirfoilGroup.AddYGap();

    m_SSXSCCSTAirfoilGroup.AddDividerBox( "Lower Surface" );

    m_SSXSCCSTAirfoilGroup.SetSameLineFlag( true );
    m_SSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_SSXSCCSTAirfoilGroup.AddOutput( m_SSXSCLowDegreeOutput, "Degree", m_SSXSCCSTAirfoilGroup.GetButtonWidth() * 2 );
    m_SSXSCCSTAirfoilGroup.SetFitWidthFlag( false );
    m_SSXSCCSTAirfoilGroup.AddButton( m_SSXSCLowDemoteButton, "Demote" );
    m_SSXSCCSTAirfoilGroup.AddButton( m_SSXSCLowPromoteButton, "Promote" );

    m_SSXSCCSTAirfoilGroup.ForceNewLine();

    m_SSXSCCSTAirfoilGroup.SetSameLineFlag( false );
    m_SSXSCCSTAirfoilGroup.SetFitWidthFlag( true );

    m_SSXSCCSTLowCoeffScroll = m_SSXSCCSTAirfoilGroup.AddFlScroll( 60 );
    m_SSXSCCSTLowCoeffScroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    m_SSXSCCSTLowCoeffScroll->box( FL_BORDER_BOX );
    m_SSXSCCSTLowCoeffLayout.SetGroupAndScreen( m_SSXSCCSTLowCoeffScroll, this );

    //==== VKT AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCVKTGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCVKTGroup.AddSlider( m_SSXSCVKTChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCVKTGroup.AddYGap();
    m_SSXSCVKTGroup.AddSlider( m_SSXSCVKTEpsilonSlider, "Epsilon", 1, "%7.5f" );
    m_SSXSCVKTGroup.AddSlider( m_SSXSCVKTKappaSlider, "Kappa", 1, "%7.5f" );
    m_SSXSCVKTGroup.AddSlider( m_SSXSCVKTTauSlider, "Tau", 10, "%7.5f" );
    m_SSXSCVKTGroup.AddOutput( m_SSXSCVKTThickChordOutput, "T/C", "%7.5f" );
    m_SSXSCVKTGroup.AddYGap();
    m_SSXSCVKTGroup.AddButton( m_SSXSCVKTInvertButton, "Invert Airfoil" );
    m_SSXSCVKTGroup.AddYGap();
    m_SSXSCVKTGroup.SetSameLineFlag( true );
    m_SSXSCVKTGroup.SetFitWidthFlag( false );
    m_SSXSCVKTGroup.SetButtonWidth( 125 );
    m_SSXSCVKTGroup.AddButton( m_SSXSCVKTFitCSTButton, "Fit CST" );
    m_SSXSCVKTGroup.InitWidthHeightVals();
    m_SSXSCVKTGroup.SetFitWidthFlag( true );
    m_SSXSCVKTGroup.AddCounter( m_SSXSCVKTDegreeCounter, "Degree", 125 );

    //==== Four Series AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCFourDigitModGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCFourDigitModGroup.AddOutput( m_SSXSCFourModNameOutput, "Name" );
    m_SSXSCFourDigitModGroup.AddYGap();
    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCFourDigitModGroup.AddYGap();

    m_SSXSCFourDigitModGroup.SetSameLineFlag( true );

    m_SSXSCFourDigitModGroup.SetFitWidthFlag( false );
    m_SSXSCFourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_SSXSCFourDigitModGroup.AddButton( m_SSXSCFourModCamberButton, "" );
    m_SSXSCFourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_SSXSCFourDigitModGroup.SetFitWidthFlag( true );
    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModCamberSlider, "Camber", 0.2, "%7.5f" );

    m_SSXSCFourDigitModGroup.ForceNewLine();

    m_SSXSCFourDigitModGroup.SetFitWidthFlag( false );
    m_SSXSCFourDigitModGroup.SetButtonWidth( actionToggleButtonWidth );
    m_SSXSCFourDigitModGroup.AddButton( m_SSXSCFourModCLiButton, "" );
    m_SSXSCFourDigitModGroup.SetButtonWidth( actionSliderButtonWidth );
    m_SSXSCFourDigitModGroup.SetFitWidthFlag( true );
    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModCLiSlider, "Ideal CL", 1, "%7.5f" );

    m_SSXSCFourDigitModGroup.ForceNewLine();

    m_SSXSCFourDigitModGroup.SetSameLineFlag( false );
    m_SSXSCFourDigitModGroup.SetButtonWidth( actionSliderButtonWidth + actionToggleButtonWidth );

    m_SSXSCFourModCamberGroup.Init( this );
    m_SSXSCFourModCamberGroup.AddButton( m_SSXSCFourModCamberButton.GetFlButton() );
    m_SSXSCFourModCamberGroup.AddButton( m_SSXSCFourModCLiButton.GetFlButton() );

    m_SSXSCFourModCamberGroup.SetValMapVec( camb_val_map );

    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_SSXSCFourDigitModGroup.AddYGap();
    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_SSXSCFourDigitModGroup.AddSlider( m_SSXSCFourModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_SSXSCFourDigitModGroup.AddYGap();
    m_SSXSCFourDigitModGroup.AddButton( m_SSXSCFourModInvertButton, "Invert Airfoil" );
    m_SSXSCFourDigitModGroup.AddYGap();
    m_SSXSCFourDigitModGroup.AddButton( m_SSXSCFourModSharpTEButton, "Sharpen TE" );
    m_SSXSCFourDigitModGroup.AddYGap();
    m_SSXSCFourDigitModGroup.SetSameLineFlag( true );
    m_SSXSCFourDigitModGroup.SetFitWidthFlag( false );
    m_SSXSCFourDigitModGroup.SetButtonWidth( 125 );
    m_SSXSCFourDigitModGroup.AddButton( m_SSXSCFourModFitCSTButton, "Fit CST" );
    m_SSXSCFourDigitModGroup.InitWidthHeightVals();
    m_SSXSCFourDigitModGroup.SetFitWidthFlag( true );
    m_SSXSCFourDigitModGroup.AddCounter( m_SSXSCFourModDegreeCounter, "Degree", 125 );

    //==== Five Digit AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCFiveDigitGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCFiveDigitGroup.AddOutput( m_SSXSCFiveNameOutput, "Name" );
    m_SSXSCFiveDigitGroup.AddYGap();
    m_SSXSCFiveDigitGroup.AddSlider( m_SSXSCFiveChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCFiveDigitGroup.AddSlider( m_SSXSCFiveThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCFiveDigitGroup.AddYGap();
    m_SSXSCFiveDigitGroup.AddSlider( m_SSXSCFiveCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_SSXSCFiveDigitGroup.AddSlider( m_SSXSCFiveCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_SSXSCFiveDigitGroup.AddYGap();
    m_SSXSCFiveDigitGroup.AddButton( m_SSXSCFiveInvertButton, "Invert Airfoil" );
    m_SSXSCFiveDigitGroup.AddYGap();
    m_SSXSCFiveDigitGroup.AddButton( m_SSXSCFiveSharpTEButton, "Sharpen TE" );
    m_SSXSCFiveDigitGroup.AddYGap();
    m_SSXSCFiveDigitGroup.SetSameLineFlag( true );
    m_SSXSCFiveDigitGroup.SetFitWidthFlag( false );
    m_SSXSCFiveDigitGroup.SetButtonWidth( 125 );
    m_SSXSCFiveDigitGroup.AddButton( m_SSXSCFiveFitCSTButton, "Fit CST" );
    m_SSXSCFiveDigitGroup.InitWidthHeightVals();
    m_SSXSCFiveDigitGroup.SetFitWidthFlag( true );
    m_SSXSCFiveDigitGroup.AddCounter( m_SSXSCFiveDegreeCounter, "Degree", 125 );

    //==== Five Digit Mod AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCFiveDigitModGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCFiveDigitModGroup.AddOutput( m_SSXSCFiveModNameOutput, "Name" );
    m_SSXSCFiveDigitModGroup.AddYGap();
    m_SSXSCFiveDigitModGroup.AddSlider( m_SSXSCFiveModChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCFiveDigitModGroup.AddSlider( m_SSXSCFiveModThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCFiveDigitModGroup.AddYGap();
    m_SSXSCFiveDigitModGroup.AddSlider( m_SSXSCFiveModCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_SSXSCFiveDigitModGroup.AddSlider( m_SSXSCFiveModCamberLocSlider, "CamberLoc", 1, "%7.5f" );
    m_SSXSCFiveDigitModGroup.AddYGap();
    m_SSXSCFiveDigitModGroup.AddSlider( m_SSXSCFiveModThicknessLocSlider, "T/CLoc", 0.5, "%7.5f" );
    m_SSXSCFiveDigitModGroup.AddSlider( m_SSXSCFiveModLERadIndexSlider, "LERadIndx", 5, "%7.5f" );
    m_SSXSCFiveDigitModGroup.AddYGap();
    m_SSXSCFiveDigitModGroup.AddButton( m_SSXSCFiveModInvertButton, "Invert Airfoil" );
    m_SSXSCFiveDigitModGroup.AddYGap();
    m_SSXSCFiveDigitModGroup.AddButton( m_SSXSCFiveModSharpTEButton, "Sharpen TE" );
    m_SSXSCFiveDigitModGroup.AddYGap();
    m_SSXSCFiveDigitModGroup.SetSameLineFlag( true );
    m_SSXSCFiveDigitModGroup.SetFitWidthFlag( false );
    m_SSXSCFiveDigitModGroup.SetButtonWidth( 125 );
    m_SSXSCFiveDigitModGroup.AddButton( m_SSXSCFiveModFitCSTButton, "Fit CST" );
    m_SSXSCFiveDigitModGroup.InitWidthHeightVals();
    m_SSXSCFiveDigitModGroup.SetFitWidthFlag( true );
    m_SSXSCFiveDigitModGroup.AddCounter( m_SSXSCFiveModDegreeCounter, "Degree", 125 );

    //==== 16 Series AF ====//
    m_SSXSCGroup.AddSubGroupLayout( m_SSXSCOneSixSeriesGroup, m_SSXSCGroup.GetW(), m_SSXSCGroup.GetRemainY() );
    m_SSXSCOneSixSeriesGroup.AddOutput( m_SSXSCOneSixSeriesNameOutput, "Name" );
    m_SSXSCOneSixSeriesGroup.AddYGap();
    m_SSXSCOneSixSeriesGroup.AddSlider( m_SSXSCOneSixSeriesChordSlider, "Chord", 10, "%7.3f" );
    m_SSXSCOneSixSeriesGroup.AddSlider( m_SSXSCOneSixSeriesThickChordSlider, "T/C", 1, "%7.5f" );
    m_SSXSCOneSixSeriesGroup.AddYGap();
    m_SSXSCOneSixSeriesGroup.AddSlider( m_SSXSCOneSixSeriesCLiSlider, "Ideal CL", 1, "%7.5f" );
    m_SSXSCOneSixSeriesGroup.AddYGap();
    m_SSXSCOneSixSeriesGroup.AddButton( m_SSXSCOneSixSeriesInvertButton, "Invert Airfoil" );
    m_SSXSCOneSixSeriesGroup.AddYGap();
    m_SSXSCOneSixSeriesGroup.AddButton( m_SSXSCOneSixSeriesSharpTEButton, "Sharpen TE" );
    m_SSXSCOneSixSeriesGroup.AddYGap();
    m_SSXSCOneSixSeriesGroup.SetSameLineFlag( true );
    m_SSXSCOneSixSeriesGroup.SetFitWidthFlag( false );
    m_SSXSCOneSixSeriesGroup.SetButtonWidth( 125 );
    m_SSXSCOneSixSeriesGroup.AddButton( m_SSXSCOneSixSeriesFitCSTButton, "Fit CST" );
    m_SSXSCOneSixSeriesGroup.InitWidthHeightVals();
    m_SSXSCOneSixSeriesGroup.SetFitWidthFlag( true );
    m_SSXSCOneSixSeriesGroup.AddCounter( m_SSXSCOneSixSeriesDegreeCounter, "Degree", 125 );

    m_SubSurfXSCCurrDisplayGroup = nullptr;

    SubSurfXSCDisplayGroup( &m_SSXSCPointGroup );

    vector < int > y_vals;

    y_vals.push_back( m_SSXSCCircleGroup.GetY() );
    y_vals.push_back( m_SSXSCEllipseGroup.GetY() );
    y_vals.push_back( m_SSXSCSuperGroup.GetY() );
    y_vals.push_back( m_SSXSCRoundedRectGroup.GetY() );
    y_vals.push_back( m_SSXSCGenGroup.GetY() );
    y_vals.push_back( m_SSXSCFourSeriesGroup.GetY() );
    y_vals.push_back( m_SSXSCSixSeriesGroup.GetY() );
    y_vals.push_back( m_SSXSCBiconvexGroup.GetY() );
    y_vals.push_back( m_SSXSCWedgeGroup.GetY() );
    y_vals.push_back( m_SSXSCFuseFileGroup.GetY() );
    y_vals.push_back( m_SSXSCAfFileGroup.GetY() );
    y_vals.push_back( m_SSXSCCSTAirfoilGroup.GetY() );
    y_vals.push_back( m_SSXSCCSTLowCoeffLayout.GetY() );
    y_vals.push_back( m_SSXSCVKTGroup.GetY() );
    y_vals.push_back( m_SSXSCFourDigitModGroup.GetY() );
    y_vals.push_back( m_SSXSCFiveDigitGroup.GetY() );
    y_vals.push_back( m_SSXSCFiveDigitModGroup.GetY() );
    y_vals.push_back( m_SSXSCOneSixSeriesGroup.GetY() );

    m_SSXSCGroup.SetY( *max_element( y_vals.begin(), y_vals.end() ) );
    m_SSXSCGroup.AddYGap();
    m_SSXSCAttrEditor.Init( &m_SSXSCGroup , subsurf_group , this, staticScreenCB , true , m_SSXSCGroup.GetY() , attr_h-17 );

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

    m_SSConGroup.AddSlider(m_SSConUSSlider, "Start U", 1, "%7.6f");
    m_SSConGroup.AddSlider(m_SSConUESlider, "End U", 1, "%7.6f");

    m_SSConGroup.AddButton( m_SSConEtaButton, "Use Eta for Spanwise Coordinate" );

    m_SSConGroup.AddSlider(m_SSConEtaSSlider, "Start Eta", 1, "%7.6f");
    m_SSConGroup.AddSlider(m_SSConEtaESlider, "End Eta", 1, "%7.6f");

    m_SSConGroup.AddYGap();
    m_SSConGroup.AddDividerBox( "Chordwise" );


    m_SSConGroup.SetFitWidthFlag(false);
    m_SSConGroup.SetSameLineFlag(true);

    m_SSConGroup.SetButtonWidth( m_SSConGroup.GetW() - 2 * m_SSConGroup.GetInputWidth() );


    m_SSConGroup.AddButton( m_SSConSEConstButton, "Constant" );
    m_SSConGroup.SetButtonWidth( m_SSConGroup.GetInputWidth() );
    m_SSConGroup.AddButton( m_SSConSRelButton, "L/C" );
    m_SSConGroup.AddButton( m_SSConSAbsButton, "L" );

    m_SSConGroup.SetButtonWidth( m_SSConGroup.GetW() / 3 );


    m_SSConGroup.SetFitWidthFlag(true);
    m_SSConGroup.SetSameLineFlag(false);
    m_SSConGroup.ForceNewLine();

    m_SSConSAbsRelToggleGroup.Init( this );
    m_SSConSAbsRelToggleGroup.AddButton( m_SSConSAbsButton.GetFlButton() );
    m_SSConSAbsRelToggleGroup.AddButton( m_SSConSRelButton.GetFlButton() );

    m_SSConGroup.AddSlider( m_SSConSLenSlider, "Start Length", 10.0, "%7.6f" );
    m_SSConGroup.AddSlider( m_SSConELenSlider, "End Length", 10.0, "%7.6f" );

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

    m_SSConGroup.AddSlider( m_SSConSAngleSlider, "Start Angle", 10.0, "%7.6f" );
    m_SSConGroup.AddSlider( m_SSConEAngleSlider, "End Angle", 10.0, "%7.6f" );

    m_SSConGroup.AddSlider( m_SSConTessSlider, "Num Points", 100, "%5.0f" );

    m_SSConGroup.AddYGap();
    m_SSConAttrEditor.Init( &m_SSConGroup , subsurf_group , this , staticScreenCB, true , m_SSConGroup.GetY() , attr_h);

    //==== SSFiniteLine ====//
    m_SSCommonGroup.AddSubGroupLayout( m_SSFLineGroup, m_SSCommonGroup.GetW(), m_SSCommonGroup.GetRemainY() );
    remain_x = m_SSFLineGroup.GetRemainX();

    m_SSFLineGroup.SetFitWidthFlag( true );
    m_SSFLineGroup.SetSameLineFlag( false );
    m_SSFLineGroup.ForceNewLine();

    m_SSFLineGroup.AddSlider( m_SSFLineUStartSlider, "U Start", 1, "%7.6f" );
    m_SSFLineGroup.AddSlider( m_SSFLineUEndSlider, "U End", 1, "%7.6f" );
    m_SSFLineGroup.AddSlider( m_SSFLineWStartSlider, "W Start", 1, "%7.6f" );
    m_SSFLineGroup.AddSlider( m_SSFLineWEndSlider, "W End", 1, "%7.6f" );

    m_RotActive = true;

    m_SSFLineGroup.AddYGap();
    m_SSFLineAttrEditor.Init( &m_SSFLineGroup , subsurf_group , this , staticScreenCB, true , m_SSFLineGroup.GetY() , attr_h);

    //=============== MassProp Tab ===================//
    m_MassPropLayout.SetGroupAndScreen( massprop_group, this );

    m_MassPropLayout.AddDividerBox( "Volume-Based Mass Properties" );

    m_MassPropLayout.AddInput( m_DensityInput, "Density", " %7.6f" );
    m_MassPropLayout.AddCounter( m_PriorCounter, "Priority" );

    m_MassPropLayout.AddYGap();
    m_MassPropLayout.AddDividerBox( "OML-Based Mass Properties" );

    m_MassPropLayout.AddButton( m_ThinShellButton, "Include Thin Shell" );
    m_MassPropLayout.AddInput( m_ShellMassAreaInput, "Mass/Area", " %7.6f" );

    m_MassPropLayout.AddYGap();
    m_MassPropLayout.AddDividerBox( "Specified Mass Properties" );

    m_MassPropLayout.AddSlider( m_MassSlider, "Mass ", 100, "%7.6f" );
    m_MassPropLayout.AddYGap();

    m_MassPropLayout.AddSlider( m_CGxSlider, "X CG", 100, "%7.6f" );
    m_MassPropLayout.AddSlider( m_CGySlider, "Y CG", 100, "%7.6f" );
    m_MassPropLayout.AddSlider( m_CGzSlider, "Z CG", 100, "%7.6f" );
    m_MassPropLayout.AddYGap();

    m_MassPropLayout.AddSlider( m_IxxSlider, "Ixx", 100, "%7.6f" );
    m_MassPropLayout.AddSlider( m_IyySlider, "Iyy", 100, "%7.6f" );
    m_MassPropLayout.AddSlider( m_IzzSlider, "Izz", 100, "%7.6f" );
    m_MassPropLayout.AddYGap();

    m_MassPropLayout.AddSlider( m_IxySlider, "Ixy", 100, "%7.6f" );
    m_MassPropLayout.AddSlider( m_IxzSlider, "Ixz", 100, "%7.6f" );
    m_MassPropLayout.AddSlider( m_IyzSlider, "Iyz", 100, "%7.6f" );


}

void GeomScreen::Show( )
{
    // inherit base class method, add a line to clear the entry fields of the AttributeEditor GUI when changing tab/screen
    VspScreen::Show( );
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

    Vehicle* veh = VehicleMgr.GetVehicle();

    bool wing_parent = false;
    bool routing_parent = false;
    bool orphaned_trans = false;
    bool orphaned_rot = false;

    if ( veh )
    {
        Geom* parent = veh->FindGeom( geom_ptr->GetParentID() );

        if ( parent )
        {
            WingGeom* wing_ptr = dynamic_cast< WingGeom* >( parent );
            if ( wing_ptr )
            {
                wing_parent = true;
            }

            RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( parent );
            if ( routing_ptr )
            {
                routing_parent = true;
            }
        }
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

    //==== Attributes ====//
    m_AttributeEditor.SetEditorCollID( geom_ptr->m_AttrCollection.GetID() );
    m_AttributeEditor.Update();

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
        snprintf( str, sizeof( str ),  "%3d %s", i, ancestorNames[i].c_str() );
        m_SymAncestorChoice.AddItem( str, i );
    }
    m_SymAncestorChoice.UpdateItems();
    m_SymAncestorChoice.Update( geom_ptr->m_SymAncestor.GetID() );

    m_SymAncestorOriginObjectToggle.Update( geom_ptr->m_SymAncestOriginFlag.GetID() );
    m_XYSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_XZSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_YZSymToggle.Update( geom_ptr->m_SymPlanFlag.GetID() );
    m_AxialToggleGroup.Update( geom_ptr->m_SymAxFlag.GetID() );
    m_AxialNSlider.Update( geom_ptr->m_SymRotN.GetID() );

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
    m_AttachUSlider.Activate();
    m_UScaleToggleGroup.Activate();
    m_AttachVSlider.Activate();
    m_AttachRSlider.Activate();
    m_RScaleToggleGroup.Activate();
    m_AttachSSlider.Activate();
    m_AttachTSlider.Activate();
    m_AttachLSlider.Activate();
    m_LScaleToggleGroup.Activate();
    m_AttachMSlider.Activate();
    m_AttachNSlider.Activate();
    m_AttachEtaSlider.Activate();

    m_TransToggleGroup.Update( geom_ptr->m_TransAttachFlag.GetID() );
    m_RotToggleGroup.Update( geom_ptr->m_RotAttachFlag.GetID() );
    m_UScaleToggleGroup.Update( geom_ptr->m_U01.GetID() );

    if ( geom_ptr->m_U01() )
    {
        m_AttachUSlider.Update( 1, geom_ptr->m_ULoc.GetID(), geom_ptr->m_U0NLoc.GetID());
    }
    else
    {
        m_AttachUSlider.Update( 2, geom_ptr->m_ULoc.GetID(), geom_ptr->m_U0NLoc.GetID());
    }

    m_AttachVSlider.Update( geom_ptr->m_WLoc.GetID() );
    m_RScaleToggleGroup.Update( geom_ptr->m_R01.GetID() );

    if ( geom_ptr->m_R01() )
    {
        m_AttachRSlider.Update( 1, geom_ptr->m_RLoc.GetID(), geom_ptr->m_R0NLoc.GetID());
    }
    else
    {
        m_AttachRSlider.Update( 2, geom_ptr->m_RLoc.GetID(), geom_ptr->m_R0NLoc.GetID());
    }

    m_AttachSSlider.Update( geom_ptr->m_SLoc.GetID() );
    m_AttachTSlider.Update( geom_ptr->m_TLoc.GetID() );
    m_LScaleToggleGroup.Update( geom_ptr->m_L01.GetID() );

    if ( geom_ptr->m_L01() )
    {
        m_AttachLSlider.Update( 1, geom_ptr->m_LLoc.GetID(), geom_ptr->m_L0LenLoc.GetID());
    }
    else
    {
        m_AttachLSlider.Update( 2, geom_ptr->m_LLoc.GetID(), geom_ptr->m_L0LenLoc.GetID());
    }

    m_AttachMSlider.Update( geom_ptr->m_MLoc.GetID() );
    m_AttachNSlider.Update( geom_ptr->m_NLoc.GetID() );

    m_AttachEtaSlider.Update( geom_ptr->m_EtaLoc.GetID() );
    if ( geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_UV && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_ROT_UV )
    {
        m_AttachUSlider.Deactivate();
        m_UScaleToggleGroup.Deactivate();
        m_AttachVSlider.Deactivate();
    }

    if ( geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_RST && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_ROT_RST )
    {
        m_AttachRSlider.Deactivate();
        m_RScaleToggleGroup.Deactivate();
        m_AttachSSlider.Deactivate();
        m_AttachTSlider.Deactivate();
    }

    if ( geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_LMN && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_ROT_LMN )
    {
        m_AttachLSlider.Deactivate();
        m_LScaleToggleGroup.Deactivate();
    }

    if ( geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_LMN && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_ROT_LMN &&
         geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_EtaMN && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_TRANS_EtaMN )
    {
        m_AttachMSlider.Deactivate();
        m_AttachNSlider.Deactivate();
    }

    if ( geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_EtaMN && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_ROT_EtaMN )
    {
        m_AttachEtaSlider.Deactivate();
    }

    if ( geom_ptr->m_ShellFlag.Get() )
    {
        m_ShellMassAreaInput.Activate();
    }
    else
    {
        m_ShellMassAreaInput.Deactivate();
    }

    if ( wing_parent )
    {
        m_TransEtaMNButton.Activate();
        m_RotEtaMNButton.Activate();
    }
    else
    {
        m_TransEtaMNButton.Deactivate();
        m_RotEtaMNButton.Deactivate();
        m_AttachEtaSlider.Deactivate();
    }

    if ( veh )
    {
        Geom* parent = veh->FindGeom( geom_ptr->GetParentID() );

        if ( parent && !routing_parent && !geom_ptr->IsParentJoint() )
        {
            m_TransToggleGroup.Activate();
            m_RotToggleGroup.Activate();
            m_UScaleToggleGroup.Activate();
            m_RScaleToggleGroup.Activate();
            m_LScaleToggleGroup.Activate();

            m_AttachUSlider.Activate();
            m_AttachVSlider.Activate();
            m_AttachRSlider.Activate();
            m_AttachSSlider.Activate();
            m_AttachTSlider.Activate();
            m_AttachLSlider.Activate();
            m_AttachMSlider.Activate();
            m_AttachNSlider.Activate();
            m_AttachEtaSlider.Activate();
        }
        else
        {
            m_TransToggleGroup.Deactivate();
            m_RotToggleGroup.Deactivate();
            m_UScaleToggleGroup.Deactivate();
            m_RScaleToggleGroup.Deactivate();
            m_LScaleToggleGroup.Deactivate();

            m_AttachUSlider.Deactivate();
            m_AttachVSlider.Deactivate();
            m_AttachRSlider.Deactivate();
            m_AttachSSlider.Deactivate();
            m_AttachTSlider.Deactivate();
            m_AttachLSlider.Deactivate();
            m_AttachMSlider.Deactivate();
            m_AttachNSlider.Deactivate();
            m_AttachEtaSlider.Deactivate();
        }

        // Handle orphaned trans/rot attachment flags; activate the "none" buttons for user to de-orphan geom
        if ( !parent && geom_ptr->m_TransAttachFlag() != vsp::ATTACH_TRANS_NONE )
        {
            orphaned_trans = true;
        }
        if ( !parent && geom_ptr->m_RotAttachFlag() != vsp::ATTACH_TRANS_NONE )
        {
            orphaned_rot = true;
        }

    }

    if ( orphaned_trans )
    {
        m_TransNoneButton.Activate();
    }
    if ( orphaned_rot )
    {
        m_RotNoneButton.Activate();
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
    m_SubSurfSelectSurface.AddItem( "ALL", -1 );
    int nmain = geom_ptr->GetNumMainSurfs();
    for ( int i = 0; i < nmain; ++i )
    {
        snprintf( str, sizeof( str ),  "Surf_%d", i );
        m_SubSurfSelectSurface.AddItem( str, i );
    }
    m_SubSurfSelectSurface.UpdateItems();

    if( m_SSCurrMainSurfIndx < -1 || m_SSCurrMainSurfIndx >= nmain )
    {
        m_SSCurrMainSurfIndx = -1;
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
            m_SSLineScaleToggleGroup.Update( ssline->m_Val01.GetID() );
            m_SSLineTestToggleGroup.Update( ssline->m_TestType.GetID() );
            m_SSLineConstSlider.Update( ssline->m_ConstVal.GetID() );
            m_SSLineConstSlider0N.Update( ssline->m_ConstVal0N.GetID() );

            if ( ssline->m_Val01() )
            {
                m_SSLineConstSlider.Activate();
                m_SSLineConstSlider0N.Deactivate();
            }
            else
            {
                m_SSLineConstSlider.Deactivate();
                m_SSLineConstSlider0N.Activate();
            }

            SubSurfDispGroup( &m_SSLineGroup );

            // update attribute pointer to SSLineAttrEditor
            m_SSLineAttrEditor.SetEditorCollID( subsurf->m_AttrCollection.GetID() );
            m_SSLineAttrEditor.Update();

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

            // update attribute pointer to SSRecAttrEditor
            m_SSRecAttrEditor.SetEditorCollID( subsurf->m_AttrCollection.GetID() );
            m_SSRecAttrEditor.Update();
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

            // update attribute pointer to SSEllAttrEditor
            m_SSEllAttrEditor.SetEditorCollID( subsurf->m_AttrCollection.GetID() );
            m_SSEllAttrEditor.Update();
        }
        else if ( subsurf->GetType() == vsp::SS_XSEC_CURVE )
        {
            SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( subsurf );
            assert( ssxsc );

            m_SSXSCTestToggleGroup.Update( ssxsc->m_TestType.GetID() );
            // m_SSXSCTessSlider.Update( ssxsc->m_Tess.GetID() );
            m_SSXSCCentUSlider.Update( ssxsc->m_CenterU.GetID() );
            m_SSXSCCentWSlider.Update( ssxsc->m_CenterW.GetID() );
            // m_SSXSCULenSlider.Update( ssxsc->m_ULength.GetID() );
            // m_SSXSCWLenSlider.Update( ssxsc->m_WLength.GetID() );
            // m_SSXSCThetaSlider.Update( ssxsc->m_Theta.GetID() );
            SubSurfDispGroup( & m_SSXSCGroup );

            // update attribute pointer to SSXSCAttrEditor

            XSecCurve* xsc = ssxsc->GetXSecCurve();

            vector < string > empty_coll_ids;
            empty_coll_ids.push_back( subsurf->m_AttrCollection.GetID() );

            if ( xsc )
            {
                XSecViewScreen *xsscreen = dynamic_cast < XSecViewScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_XSEC_SCREEN ) );
                Fl_Tabs *tabs = GetTabs();
                Fl_Group* sstab = GetTab( m_SubSurfTab_ind );
                if ( xsscreen && sstab && tabs && tabs->value() == sstab)
                {
                    xsscreen->SetXSecCurve( xsc );
                }

                empty_coll_ids.push_back( xsc->GetAttrCollection()->GetID() );

                m_SSXSecTypeChoice.SetVal( xsc->GetType() );

                if ( xsc->GetType() == vsp::XS_POINT )
                {
                    SubSurfXSCDisplayGroup( NULL );
                }
                else if ( xsc->GetType() == vsp::XS_SUPER_ELLIPSE )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCSuperGroup );

                    SuperXSec* super_xs = dynamic_cast< SuperXSec* >( xsc );
                    assert( super_xs );
                    m_SSXSCSuperHeightSlider.Update( super_xs->m_Height.GetID() );
                    m_SSXSCSuperWidthSlider.Update( super_xs->m_Width.GetID() );
                    m_SSXSCSuperMSlider.Update( super_xs->m_M.GetID() );
                    m_SSXSCSuperNSlider.Update( super_xs->m_N.GetID() );
                    m_SSXSCSuperToggleSym.Update( super_xs->m_TopBotSym.GetID() );
                    m_SSXSCSuperM_botSlider.Update( super_xs->m_M_bot.GetID() );
                    m_SSXSCSuperN_botSlider.Update( super_xs->m_N_bot.GetID() );
                    m_SSXSCSuperMaxWidthLocSlider.Update( super_xs->m_MaxWidthLoc.GetID() );

                    if ( super_xs->m_TopBotSym() )
                    {
                        m_SSXSCSuperM_botSlider.Deactivate();
                        m_SSXSCSuperN_botSlider.Deactivate();
                    }
                    else if ( !super_xs->m_TopBotSym() )
                    {
                        m_SSXSCSuperM_botSlider.Activate();
                        m_SSXSCSuperN_botSlider.Activate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_CIRCLE )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCCircleGroup );
                    CircleXSec* circle_xs = dynamic_cast< CircleXSec* >( xsc );
                    assert( circle_xs );

                    m_SSXSCDiameterSlider.Update( circle_xs->m_Diameter.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_ELLIPSE )
                {
                    SubSurfXSCDisplayGroup( & m_SSXSCEllipseGroup );

                    EllipseXSec* ellipse_xs = dynamic_cast< EllipseXSec* >( xsc );
                    m_SSXSCEllipseHeightSlider.Update( ellipse_xs->m_Height.GetID() );
                    m_SSXSCEllipseWidthSlider.Update( ellipse_xs->m_Width.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_ROUNDED_RECTANGLE )
                {
                    SubSurfXSCDisplayGroup( & m_SSXSCRoundedRectGroup );
                    RoundedRectXSec* rect_xs = dynamic_cast< RoundedRectXSec* >( xsc );
                    assert( rect_xs );

                    m_SSXSCRRHeightSlider.Update( rect_xs->m_Height.GetID() );
                    m_SSXSCRRWidthSlider.Update( rect_xs->m_Width.GetID() );
                    m_SSXSCRRRadSymRadioGroup.Update( rect_xs->m_RadiusSymmetryType.GetID() );
                    m_SSXSCRRRadiusBRSlider.Update( rect_xs->m_RadiusBR.GetID() );
                    m_SSXSCRRRadiusBLSlider.Update( rect_xs->m_RadiusBL.GetID() );
                    m_SSXSCRRRadiusTLSlider.Update( rect_xs->m_RadiusTL.GetID() );
                    m_SSXSCRRRadiusTRSlider.Update( rect_xs->m_RadiusTR.GetID() );
                    m_SSXSCRRKeyCornerButton.Update( rect_xs->m_KeyCornerParm.GetID() );
                    m_SSXSCRRSkewSlider.Update( rect_xs->m_Skew.GetID() );
                    m_SSXSCRRKeystoneSlider.Update( rect_xs->m_Keystone.GetID() );
                    m_SSXSCRRVSkewSlider.Update( rect_xs->m_VSkew.GetID() );

                    if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_NONE )
                    {
                        m_SSXSCRRRadiusBRSlider.Activate();
                        m_SSXSCRRRadiusBLSlider.Activate();
                        m_SSXSCRRRadiusTLSlider.Activate();
                    }
                    else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_RL )
                    {
                        m_SSXSCRRRadiusBRSlider.Activate();
                        m_SSXSCRRRadiusBLSlider.Deactivate();
                        m_SSXSCRRRadiusTLSlider.Deactivate();
                    }
                    else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_TB )
                    {
                        m_SSXSCRRRadiusBRSlider.Deactivate();
                        m_SSXSCRRRadiusTLSlider.Activate();
                        m_SSXSCRRRadiusBLSlider.Deactivate();
                    }
                    else if ( rect_xs->m_RadiusSymmetryType.Get() == vsp::SYM_ALL )
                    {
                        m_SSXSCRRRadiusBRSlider.Deactivate();
                        m_SSXSCRRRadiusBLSlider.Deactivate();
                        m_SSXSCRRRadiusTLSlider.Deactivate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_GENERAL_FUSE )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCGenGroup );
                    GeneralFuseXSec* gen_xs = dynamic_cast< GeneralFuseXSec* >( xsc );
                    assert( gen_xs );

                    m_SSXSCGenHeightSlider.Update( gen_xs->m_Height.GetID() );
                    m_SSXSCGenWidthSlider.Update( gen_xs->m_Width.GetID() );
                    m_SSXSCGenMaxWidthLocSlider.Update( gen_xs->m_MaxWidthLoc.GetID() );
                    m_SSXSCGenCornerRadSlider.Update( gen_xs->m_CornerRad.GetID() );
                    m_SSXSCGenTopTanAngleSlider.Update( gen_xs->m_TopTanAngle.GetID() );
                    m_SSXSCGenBotTanAngleSlider.Update( gen_xs->m_BotTanAngle.GetID() );
                    m_SSXSCGenTopStrSlider.Update( gen_xs->m_TopStr.GetID() );
                    m_SSXSCGenBotStrSlider.Update( gen_xs->m_BotStr.GetID() );
                    m_SSXSCGenUpStrSlider.Update( gen_xs->m_UpStr.GetID() );
                    m_SSXSCGenLowStrSlider.Update( gen_xs->m_LowStr.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FOUR_SERIES )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCFourSeriesGroup );
                    FourSeries* fs_xs = dynamic_cast< FourSeries* >( xsc );
                    assert( fs_xs );

                    m_SSXSCFourChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_SSXSCFourThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_SSXSCFourCamberSlider.Update( fs_xs->m_Camber.GetID() );
                    m_SSXSCFourCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_SSXSCFourCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                    if ( fs_xs->m_CamberInputFlag() == vsp::MAX_CAMB )
                    {
                        m_SSXSCFourCamberSlider.Activate();
                        m_SSXSCFourCLiSlider.Deactivate();
                    }
                    else
                    {
                        m_SSXSCFourCamberSlider.Deactivate();
                        m_SSXSCFourCLiSlider.Activate();
                    }
                    m_SSXSCFourCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_SSXSCFourInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_SSXSCFourNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_SSXSCFourSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_SSXSCFourDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_SIX_SERIES )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCSixSeriesGroup );
                    SixSeries* ss_xs = dynamic_cast< SixSeries* >( xsc );
                    assert( ss_xs );

                    m_SSXSCSixChordSlider.Update( ss_xs->m_Chord.GetID() );
                    m_SSXSCSixThickChordSlider.Update( ss_xs->m_ThickChord.GetID() );
                    m_SSXSCSixIdealClSlider.Update( ss_xs->m_IdealCl.GetID() );
                    m_SSXSCSixASlider.Update( ss_xs->m_A.GetID() );

                    m_SSXSCSixInvertButton.Update( ss_xs->m_Invert.GetID() );
                    m_SSXSCSixNameOutput.Update( ss_xs->GetAirfoilName() );
                    m_SSXSCSixSeriesChoice.Update( ss_xs->m_Series.GetID() );
                    m_SSXSCSixDegreeCounter.Update( ss_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_BICONVEX )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCBiconvexGroup );
                    Biconvex* bi_xs = dynamic_cast< Biconvex* >( xsc );
                    assert( bi_xs );

                    m_SSXSCBiconvexChordSlider.Update( bi_xs->m_Chord.GetID() );
                    m_SSXSCBiconvexThickChordSlider.Update( bi_xs->m_ThickChord.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_WEDGE )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCWedgeGroup );
                    Wedge* we_xs = dynamic_cast< Wedge* >( xsc );
                    assert( we_xs );

                    m_SSXSCWedgeChordSlider.Update( we_xs->m_Chord.GetID() );
                    m_SSXSCWedgeThickChordSlider.Update( we_xs->m_ThickChord.GetID() );
                    m_SSXSCWedgeThickLocSlider.Update( we_xs->m_ThickLoc.GetID() );
                    m_SSXSCWedgeZCamberSlider.Update( we_xs->m_ZCamber.GetID() );
                    m_SSXSCWedgeSymmThickButton.Update( we_xs->m_SymmThick.GetID() );
                    m_SSXSCWedgeThickLocLowSlider.Update( we_xs->m_ThickLocLow.GetID() );
                    m_SSXSCWedgeFlatUpSlider.Update( we_xs->m_FlatUp.GetID() );
                    m_SSXSCWedgeFlatLowSlider.Update( we_xs->m_FlatLow.GetID() );
                    m_SSXSCWedgeUForeUpSlider.Update( we_xs->m_UForeUp.GetID() );
                    m_SSXSCWedgeUForeLowSlider.Update( we_xs->m_UForeLow.GetID() );
                    m_SSXSCWedgeDuUpSlider.Update( we_xs->m_DuUp.GetID() );
                    m_SSXSCWedgeDuLowSlider.Update( we_xs->m_DuLow.GetID() );
                    m_SSXSCWedgeInvertButton.Update( we_xs->m_Invert.GetID() );

                    if ( we_xs->m_SymmThick() )
                    {
                        m_SSXSCWedgeThickLocLowSlider.Deactivate();
                        m_SSXSCWedgeFlatLowSlider.Deactivate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_FILE_FUSE )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCFuseFileGroup );
                    FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                    assert( file_xs );
                    m_SSXSCFileHeightSlider.Update( file_xs->m_Height.GetID() );
                    m_SSXSCFileWidthSlider.Update( file_xs->m_Width.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FILE_AIRFOIL )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCAfFileGroup );
                    FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                    assert( affile_xs );

                    m_SSXSCAfFileChordSlider.Update( affile_xs->m_Chord.GetID() );
                    m_SSXSCAfFileThickChordSlider.Update( affile_xs->m_ThickChord.GetID() );
                    m_SSXSCAfFileBaseThickChordOutput.Update( affile_xs->m_BaseThickness.GetID() );
                    m_SSXSCAfFileInvertButton.Update( affile_xs->m_Invert.GetID() );
                    m_SSXSCAfFileNameOutput.Update( affile_xs->GetAirfoilName() );
                    m_SSXSCAfFileDegreeCounter.Update( affile_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCCSTAirfoilGroup );
                    CSTAirfoil* cst_xs = dynamic_cast< CSTAirfoil* >( xsc );
                    assert( cst_xs );

                    int num_up = cst_xs->m_UpDeg() + 1;
                    int num_low = cst_xs->m_LowDeg() + 1;

                    char str[255];
                    snprintf( str, sizeof( str ),  "%d", cst_xs->m_UpDeg() );
                    m_SSXSCUpDegreeOutput.Update( str );
                    snprintf( str, sizeof( str ),  "%d", cst_xs->m_LowDeg() );
                    m_SSXSCLowDegreeOutput.Update( str );

                    m_SSXSCCSTChordSlider.Update(cst_xs->m_Chord.GetID());
                    m_SSXSCCSTInvertButton.Update( cst_xs->m_Invert.GetID() );
                    m_SSXSCCSTContLERadButton.Update( cst_xs->m_ContLERad.GetID() );
                    m_SSXSCCSTThickChordOutput.Update( cst_xs->m_ThickChord.GetID() );

                    if ( ( m_SSXSCUpCoeffSliderVec.size() != num_up ) || ( m_SSXSCLowCoeffSliderVec.size() != num_low ) )
                    {
                        RebuildSSCSTGroup( cst_xs );
                    }

                    for ( int i = 0; i < num_up; i++ )
                    {
                        Parm *p = cst_xs->m_UpCoeffParmVec[i];
                        if ( p )
                        {
                            m_SSXSCUpCoeffSliderVec[i].Update( p->GetID() );
                        }
                    }

                    for ( int i = 0; i < num_low; i++ )
                    {
                        Parm *p = cst_xs->m_LowCoeffParmVec[i];
                        if ( p )
                        {
                            m_SSXSCLowCoeffSliderVec[i].Update( p->GetID() );
                        }
                    }

                    if ( cst_xs->m_ContLERad() && num_low > 0 )
                    {
                        m_SSXSCLowCoeffSliderVec[0].Deactivate();
                    }
                }
                else if ( xsc->GetType() == vsp::XS_VKT_AIRFOIL )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCVKTGroup );
                    VKTAirfoil* vkt_xs = dynamic_cast< VKTAirfoil* >( xsc );
                    assert( vkt_xs );

                    m_SSXSCVKTChordSlider.Update( vkt_xs->m_Chord.GetID() );
                    m_SSXSCVKTEpsilonSlider.Update( vkt_xs->m_Epsilon.GetID() );
                    m_SSXSCVKTKappaSlider.Update( vkt_xs->m_Kappa.GetID() );
                    m_SSXSCVKTTauSlider.Update( vkt_xs->m_Tau.GetID() );
                    m_SSXSCVKTInvertButton.Update( vkt_xs->m_Invert.GetID() );
                    m_SSXSCVKTDegreeCounter.Update( vkt_xs->m_FitDegree.GetID() );
                    m_SSXSCVKTThickChordOutput.Update( vkt_xs->m_ThickChord.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FOUR_DIGIT_MOD )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCFourDigitModGroup );
                    FourDigMod* fs_xs = dynamic_cast< FourDigMod* >( xsc );
                    assert( fs_xs );

                    m_SSXSCFourModChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_SSXSCFourModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_SSXSCFourModCamberSlider.Update( fs_xs->m_Camber.GetID() );
                    m_SSXSCFourModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_SSXSCFourModCamberGroup.Update( fs_xs->m_CamberInputFlag.GetID() );
                    if ( fs_xs->m_CamberInputFlag() == vsp::MAX_CAMB )
                    {
                        m_SSXSCFourModCamberSlider.Activate();
                        m_SSXSCFourModCLiSlider.Deactivate();
                    }
                    else
                    {
                        m_SSXSCFourModCamberSlider.Deactivate();
                        m_SSXSCFourModCLiSlider.Activate();
                    }
                    m_SSXSCFourModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_SSXSCFourModInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_SSXSCFourModNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_SSXSCFourModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                    m_SSXSCFourModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                    m_SSXSCFourModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_SSXSCFourModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FIVE_DIGIT )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCFiveDigitGroup );
                    FiveDig* fs_xs = dynamic_cast< FiveDig* >( xsc );
                    assert( fs_xs );

                    m_SSXSCFiveChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_SSXSCFiveThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_SSXSCFiveCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_SSXSCFiveCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_SSXSCFiveInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_SSXSCFiveNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_SSXSCFiveSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_SSXSCFiveDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_FIVE_DIGIT_MOD )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCFiveDigitModGroup );
                    FiveDigMod* fs_xs = dynamic_cast< FiveDigMod* >( xsc );
                    assert( fs_xs );

                    m_SSXSCFiveModChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_SSXSCFiveModThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_SSXSCFiveModCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_SSXSCFiveModCamberLocSlider.Update( fs_xs->m_CamberLoc.GetID() );
                    m_SSXSCFiveModInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_SSXSCFiveModNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_SSXSCFiveModThicknessLocSlider.Update( fs_xs->m_ThickLoc.GetID() );
                    m_SSXSCFiveModLERadIndexSlider.Update( fs_xs->m_LERadIndx.GetID() );
                    m_SSXSCFiveModSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_SSXSCFiveModDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_ONE_SIX_SERIES )
                {
                    SubSurfXSCDisplayGroup( &m_SSXSCOneSixSeriesGroup );
                    OneSixSeries* fs_xs = dynamic_cast< OneSixSeries* >( xsc );
                    assert( fs_xs );

                    m_SSXSCOneSixSeriesChordSlider.Update( fs_xs->m_Chord.GetID() );
                    m_SSXSCOneSixSeriesThickChordSlider.Update( fs_xs->m_ThickChord.GetID() );
                    m_SSXSCOneSixSeriesCLiSlider.Update( fs_xs->m_IdealCl.GetID() );
                    m_SSXSCOneSixSeriesInvertButton.Update( fs_xs->m_Invert.GetID() );
                    m_SSXSCOneSixSeriesNameOutput.Update( fs_xs->GetAirfoilName() );
                    m_SSXSCOneSixSeriesSharpTEButton.Update( fs_xs->m_SharpTE.GetID() );
                    m_SSXSCOneSixSeriesDegreeCounter.Update( fs_xs->m_FitDegree.GetID() );
                }
                else if ( xsc->GetType() == vsp::XS_EDIT_CURVE )
                {
                    m_SSXSCEditCEDITGroup.Show();
                    m_SSXSCConvertCEDITGroup.Hide();
                    SubSurfXSCDisplayGroup( nullptr );
                }

                if ( xsc->GetType() != vsp::XS_EDIT_CURVE )
                {
                    m_SSXSCEditCEDITGroup.Hide();
                    m_SSXSCConvertCEDITGroup.Show();
                }
            }
            m_SSXSCAttrEditor.SetEditorCollID( empty_coll_ids );
            m_SSXSCAttrEditor.Update();

        }
        else if (subsurf->GetType() == vsp::SS_CONTROL)
        {
            SSControlSurf* sscon = dynamic_cast< SSControlSurf* >(subsurf);
            assert(sscon);

            m_SSConTestToggleGroup.Update(sscon->m_TestType.GetID());

            m_SSConEtaButton.Update(sscon->m_EtaFlag.GetID());

            m_SSConUESlider.Update(sscon->m_UEnd.GetID());
            m_SSConUSSlider.Update(sscon->m_UStart.GetID());

            m_SSConEtaESlider.Update(sscon->m_EtaEnd.GetID());
            m_SSConEtaSSlider.Update(sscon->m_EtaStart.GetID());

            if ( sscon->m_EtaFlag() )
            {
                m_SSConUSSlider.Deactivate();
                m_SSConUESlider.Deactivate();
                m_SSConEtaSSlider.Activate();
                m_SSConEtaESlider.Activate();
            }
            else
            {
                m_SSConUSSlider.Activate();
                m_SSConUESlider.Activate();
                m_SSConEtaSSlider.Deactivate();
                m_SSConEtaESlider.Deactivate();
            }

            m_SSConSAbsRelToggleGroup.Update(sscon->m_AbsRelFlag.GetID());
            m_SSConSEConstButton.Update(sscon->m_ConstFlag.GetID());

            if ( sscon->m_AbsRelFlag() == ABS )
            {
                m_SSConSLenSlider.Update( 2, sscon->m_StartLenFrac.GetID(), sscon->m_StartLength.GetID() );
                m_SSConELenSlider.Update( 2, sscon->m_EndLenFrac.GetID(), sscon->m_EndLength.GetID() );
            }
            else
            {
                m_SSConSLenSlider.Update( 1, sscon->m_StartLenFrac.GetID(), sscon->m_StartLength.GetID() );
                m_SSConELenSlider.Update( 1, sscon->m_EndLenFrac.GetID(), sscon->m_EndLength.GetID() );
            }

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

            m_SSConSurfTypeChoice.Update(sscon->m_SurfType.GetID());
            SubSurfDispGroup(&m_SSConGroup);

            // update attribute pointer to SSConAttrEditor
            m_SSConAttrEditor.SetEditorCollID( subsurf->m_AttrCollection.GetID() );
            m_SSConAttrEditor.Update();
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

            // update attribute pointer to SSFLineAttrEditor
            m_SSFLineAttrEditor.SetEditorCollID( subsurf->m_AttrCollection.GetID() );
            m_SSFLineAttrEditor.Update();
        }
    }
    else
    {
        SubSurfDispGroup( nullptr );

        // clear editor collIDs
        m_SSLineAttrEditor.SetEditorCollID();
        m_SSEllAttrEditor.SetEditorCollID();
        m_SSRecAttrEditor.SetEditorCollID();
        m_SSConAttrEditor.SetEditorCollID();
        m_SSFLineAttrEditor.SetEditorCollID();
        m_SSXSCAttrEditor.SetEditorCollID();

    }

    //==== SubSurfBrowser ====//
    int h_pos = m_SubSurfBrowser->hposition();
    int v_pos = m_SubSurfBrowser->vposition();
    m_SubSurfBrowser->clear();

    m_SubSurfBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.NAME:@b@.TYPE:@b@.SURF" );
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
            snprintf( buf, sizeof( buf ), "%0.2f", ssline->m_ConstVal() );

            ss_type.append( buf );
        }

        ss_surf_ind = subsurf_vec[i]->m_MainSurfIndx.Get();
        if ( ss_surf_ind == -1 )
        {
            snprintf( str, sizeof( str ),  "%s:%s:ALL", ss_name.c_str(), ss_type.c_str() );
        }
        else
        {
            snprintf( str, sizeof( str ),  "%s:%s:Surf_%d", ss_name.c_str(), ss_type.c_str(), ss_surf_ind );
        }
        m_SubSurfBrowser->add( str );
    }

    if ( geom_ptr->ValidSubSurfInd( SubSurfaceMgr.GetCurrSurfInd() ) )
    {
        m_SubSurfBrowser->select( SubSurfaceMgr.GetCurrSurfInd() + 2 );
    }

    m_SubSurfBrowser->hposition( h_pos );
    m_SubSurfBrowser->vposition( v_pos );

    //================= Mass Prop Tab =======================//
    m_DensityInput.Update( geom_ptr->m_Density.GetID() );
    m_ShellMassAreaInput.Update( geom_ptr->m_MassArea.GetID() );
    m_ThinShellButton.Update( geom_ptr->m_ShellFlag.GetID() );
    m_PriorCounter.Update( geom_ptr->m_MassPrior.GetID() );

    m_MassSlider.Update( geom_ptr->m_PointMass.GetID() );
    m_CGxSlider.Update( geom_ptr->m_CGx.GetID() );
    m_CGySlider.Update( geom_ptr->m_CGy.GetID() );
    m_CGzSlider.Update( geom_ptr->m_CGz.GetID() );
    m_IxxSlider.Update( geom_ptr->m_Ixx.GetID() );
    m_IyySlider.Update( geom_ptr->m_Iyy.GetID() );
    m_IzzSlider.Update( geom_ptr->m_Izz.GetID() );
    m_IxySlider.Update( geom_ptr->m_Ixy.GetID() );
    m_IxzSlider.Update( geom_ptr->m_Ixz.GetID() );
    m_IyzSlider.Update( geom_ptr->m_Iyz.GetID() );

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
    else if ( device == &m_CustomMaterialButton )
    {
        ( ( MaterialEditScreen* ) ( m_ScreenMgr->GetScreen( vsp::VSP_MATERIAL_EDIT_SCREEN ) ) )->m_OrigColor = geom_ptr->GetMaterial()->m_Name;
        geom_ptr->GetMaterial()->m_Name = "Custom";
        m_ScreenMgr->ShowScreen( vsp::VSP_MATERIAL_EDIT_SCREEN );
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
        SubSurface* ssurf = nullptr;
        ssurf = geom_ptr->AddSubSurf( m_SubSurfChoice.GetVal(), m_SSCurrMainSurfIndx );

        if ( ssurf )
        {
            ssurf->Update();
        }
        SetCurrSubSurf( geom_ptr->NumSubSurfs() - 1 );
    }
    else if ( device == &m_DelSubSurfButton )
    {
        geom_ptr->DelSubSurf( SubSurfaceMgr.GetCurrSurfInd() );
        SetCurrSubSurf( geom_ptr->NumSubSurfs() - 1 );
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
    if ( device == &m_SSXSecTypeChoice )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            int t = m_SSXSecTypeChoice.GetVal();
            sub_surf->SetXSecCurveType( t );

            if ( t == vsp::XS_EDIT_CURVE )
            {
                m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
            }
        }
    }
    else if ( device == &m_SSXSCShowXSecButton )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_XSEC_SCREEN );
    }
    else if ( device == &m_SSXSCConvertCEDITButton )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            EditCurveXSec* edit_xsec = sub_surf->ConvertToEdit();

            if ( edit_xsec )
            {
                m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
            }
        }
    }
    else if ( device == &m_SSXSCEditCEDITButton )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
    }
    else if ( device == &m_SSXSCReadFuseFileButton  )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == vsp::XS_FILE_FUSE  )
                {
                    FileXSec* file_xs = dynamic_cast< FileXSec* >( xsc );
                    assert( file_xs );
                    string newfile = m_ScreenMgr->FileChooser( "Fuselage Cross Section", "*.fxs" );

                    file_xs->ReadXsecFile( newfile );
                    file_xs->Update();
                    sub_surf->Update();
                }
            }
        }
    }
    else if ( device == &m_SSXSCAfReadFileButton   )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == vsp::XS_FILE_AIRFOIL  )
                {
                    FileAirfoil* affile_xs = dynamic_cast< FileAirfoil* >( xsc );
                    assert( affile_xs );
                    string newfile = m_ScreenMgr->FileChooser( "Airfoil File", "*.{af,dat}"  );

                    affile_xs->ReadFile( newfile );
                    affile_xs->Update();
                    sub_surf->Update();
                }
            }
        }
    }
    else if ( device == &m_SSXSCUpPromoteButton )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                    assert( cst_xs );

                    cst_xs->PromoteUpper();
                    cst_xs->Update();
                    sub_surf->Update();
                }
            }
        }
    }
    else if ( device == &m_SSXSCLowPromoteButton )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                    assert( cst_xs );

                    cst_xs->PromoteLower();
                    cst_xs->Update();
                    sub_surf->Update();
                }
            }
        }
    }
    else if ( device == &m_SSXSCUpDemoteButton )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                    assert( cst_xs );

                    cst_xs->DemoteUpper();
                    cst_xs->Update();
                    sub_surf->Update();
                }
            }
        }
    }
    else if ( device == &m_SSXSCLowDemoteButton )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                if ( xsc->GetType() == vsp::XS_CST_AIRFOIL )
                {
                    CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( xsc );
                    assert( cst_xs );

                    cst_xs->DemoteLower();
                    cst_xs->Update();
                    sub_surf->Update();
                }
            }
        }
    }
    else if ( ( device == &m_SSXSCFourFitCSTButton ) ||
            ( device == &m_SSXSCSixFitCSTButton ) ||
            ( device == &m_SSXSCAfFileFitCSTButton ) ||
            ( device == &m_SSXSCVKTFitCSTButton ) ||
            ( device == &m_SSXSCFourModFitCSTButton ) ||
            ( device == &m_SSXSCFiveFitCSTButton ) ||
            ( device == &m_SSXSCFiveModFitCSTButton ) ||
            ( device == &m_SSXSCOneSixSeriesFitCSTButton ) )
    {
        SSXSecCurve* sub_surf = dynamic_cast < SSXSecCurve* > ( geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() ) );
        if ( sub_surf )
        {
            XSecCurve* xsc = sub_surf->GetXSecCurve();
            if ( xsc )
            {
                Airfoil* af_xs = dynamic_cast<Airfoil*>( xsc );

                if ( af_xs )
                {
                    VspCurve c = af_xs->GetOrigCurve();
                    int deg = af_xs->m_FitDegree();

                    //bor_ptr->SetActiveAirfoilType( XS_CST_AIRFOIL );
                    sub_surf->SetXSecCurveType( vsp::XS_CST_AIRFOIL );

                    XSecCurve* newxsc = sub_surf->GetXSecCurve();
                    if ( newxsc )
                    {
                        if ( newxsc->GetType() == vsp::XS_CST_AIRFOIL )
                        {
                            CSTAirfoil* cst_xs = dynamic_cast<CSTAirfoil*>( newxsc );
                            assert( cst_xs );

                            cst_xs->FitCurve( c, deg );

                            cst_xs->Update();
                            sub_surf->Update();
                        }
                    }
                }
            }
        }
    }

    m_AttributeEditor.GuiDeviceCallBack( device );
    m_SSLineAttrEditor.GuiDeviceCallBack( device );
    m_SSRecAttrEditor.GuiDeviceCallBack( device );
    m_SSEllAttrEditor.GuiDeviceCallBack( device );
    m_SSConAttrEditor.GuiDeviceCallBack( device );
    m_SSFLineAttrEditor.GuiDeviceCallBack( device );
    m_SSXSCAttrEditor.GuiDeviceCallBack( device );

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
    m_SSXSCGroup.Hide();
    m_SSConGroup.Hide();
    m_SSFLineGroup.Hide();

    m_CurSubDispGroup = group;

    if ( group )
    {
        group->Show();
        m_SSCommonGroup.Show(); // Always show the Common Group if any other subsurface group is being displayed.
    }
}

void GeomScreen::SubSurfXSCDisplayGroup( GroupLayout* group )
{
    if ( m_SubSurfXSCCurrDisplayGroup == group )
    {
        return;
    }

    m_SSXSCSuperGroup.Hide();
    m_SSXSCCircleGroup.Hide();
    m_SSXSCEllipseGroup.Hide();
    m_SSXSCRoundedRectGroup.Hide();
    m_SSXSCGenGroup.Hide();
    m_SSXSCFourSeriesGroup.Hide();
    m_SSXSCSixSeriesGroup.Hide();
    m_SSXSCBiconvexGroup.Hide();
    m_SSXSCWedgeGroup.Hide();
    m_SSXSCFuseFileGroup.Hide();
    m_SSXSCAfFileGroup.Hide();
    m_SSXSCCSTAirfoilGroup.Hide();
    m_SSXSCVKTGroup.Hide();
    m_SSXSCFourDigitModGroup.Hide();
    m_SSXSCFiveDigitGroup.Hide();
    m_SSXSCFiveDigitModGroup.Hide();
    m_SSXSCOneSixSeriesGroup.Hide();

    m_SubSurfXSCCurrDisplayGroup = group;

    if ( group )
    {
        group->Show();
    }
}

void GeomScreen::RebuildSSCSTGroup( CSTAirfoil* cst_xs)
{
    if ( !cst_xs )
    {
        return;
    }

    if ( !m_SSXSCCSTUpCoeffScroll || !m_SSXSCCSTLowCoeffScroll )
    {
        return;
    }

    m_SSXSCCSTUpCoeffScroll->clear();
    m_SSXSCCSTUpCoeffLayout.SetGroup( m_SSXSCCSTUpCoeffScroll );
    m_SSXSCCSTUpCoeffLayout.InitWidthHeightVals();

    m_SSXSCUpCoeffSliderVec.clear();

    unsigned int num_up = cst_xs->m_UpDeg() + 1;

    m_SSXSCUpCoeffSliderVec.resize( num_up );

    for ( int i = 0; i < num_up; i++ )
    {
        m_SSXSCCSTUpCoeffLayout.AddSlider( m_SSXSCUpCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }




    m_SSXSCCSTLowCoeffScroll->clear();
    m_SSXSCCSTLowCoeffLayout.SetGroup( m_SSXSCCSTLowCoeffScroll );
    m_SSXSCCSTLowCoeffLayout.InitWidthHeightVals();

    m_SSXSCLowCoeffSliderVec.clear();

    unsigned int num_low = cst_xs->m_LowDeg() + 1;

    m_SSXSCLowCoeffSliderVec.resize( num_low );


    for ( int i = 0; i < num_low; i++ )
    {
        m_SSXSCCSTLowCoeffLayout.AddSlider( m_SSXSCLowCoeffSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
    }
}

void GeomScreen::SetCurrSubSurf( int ss_index )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    m_SSLineAttrEditor.SetEditorCollID();
    m_SSRecAttrEditor.SetEditorCollID();
    m_SSEllAttrEditor.SetEditorCollID();
    m_SSConAttrEditor.SetEditorCollID();
    m_SSFLineAttrEditor.SetEditorCollID();
    m_SSXSCAttrEditor.SetEditorCollID();

    SubSurfaceMgr.SetCurrSubSurfInd( ss_index );

    SubSurface* sub_surf = geom_ptr->GetSubSurf( SubSurfaceMgr.GetCurrSurfInd() );

    if ( sub_surf && sub_surf->GetAttrCollection() )
    {
        string ss_coll_id = sub_surf->GetAttrCollection()->GetID();

        switch ( sub_surf->GetType() )
        {
            case vsp::SS_LINE:
            {
                m_SSLineAttrEditor.SetEditorCollID( ss_coll_id );
                break;
            }
            case vsp::SS_RECTANGLE:
            {
                m_SSRecAttrEditor.SetEditorCollID( ss_coll_id );
                break;
            }
            case vsp::SS_ELLIPSE:
            {
                m_SSEllAttrEditor.SetEditorCollID( ss_coll_id );
                break;
            }
            case vsp::SS_CONTROL:
            {
                m_SSConAttrEditor.SetEditorCollID( ss_coll_id );
                break;
            }

            case vsp::SS_FINITE_LINE:
            {
                m_SSFLineAttrEditor.SetEditorCollID( ss_coll_id );
                break;
            }
            case vsp::SS_XSEC_CURVE:
            {
                vector < string > empty_coll_ids;
                empty_coll_ids.push_back( ss_coll_id );

                SSXSecCurve* ssxsc = dynamic_cast< SSXSecCurve* >( sub_surf );
                assert( ssxsc );

                XSecCurve* xsc = ssxsc->GetXSecCurve();

                if ( xsc && xsc->GetAttrCollection() )
                {
                    empty_coll_ids.push_back( xsc->GetAttrCollection()->GetID() );
                }

                m_SSXSCAttrEditor.SetEditorCollID( empty_coll_ids );
                break;
            }
            default:
            break;
        }
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
        SetCurrSubSurf( m_SubSurfBrowser->value() - 2 );
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

    //forward Fl_widget to attribute editor callback
    m_AttributeEditor.DeviceCB( w );
    m_SSLineAttrEditor.DeviceCB( w );
    m_SSRecAttrEditor.DeviceCB( w );
    m_SSEllAttrEditor.DeviceCB( w );
    m_SSConAttrEditor.DeviceCB( w );
    m_SSFLineAttrEditor.DeviceCB( w );
    m_SSXSCAttrEditor.DeviceCB( w );

    m_ScreenMgr->SetUpdateFlag( true );
}

void GeomScreen::CloseCallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
    Hide();
}

void GeomScreen::GetCollIDs( vector < string > &collIDVec )
{
    m_AttributeEditor.GetCollIDs( collIDVec );
    m_SSLineAttrEditor.GetCollIDs( collIDVec );
    m_SSRecAttrEditor.GetCollIDs( collIDVec );
    m_SSEllAttrEditor.GetCollIDs( collIDVec );
    m_SSConAttrEditor.GetCollIDs( collIDVec );
    m_SSFLineAttrEditor.GetCollIDs( collIDVec );
    m_SSXSCAttrEditor.GetCollIDs( collIDVec );
}

//=====================================================================//
//=====================================================================//
//=====================================================================//
XSecScreen::XSecScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile, const string & xsnamelabel, const string & xscnamelabel, bool add_xs_btn ) :
    GeomScreen( mgr, w, h, title, helpfile )
{
    m_XSecTab = AddTab( "XSec" );
    Fl_Group* xsec_group = AddSubGroup( m_XSecTab, 5 );

    m_XSecLayout.SetGroupAndScreen( xsec_group, this );

    m_XSecDivider = m_XSecLayout.AddDividerBox( "Cross Section" );

    m_XSecLayout.AddIndexSelector( m_XSecIndexSelector );

    // if XSec name input button has been intialized
    m_XSecNameInputActive = add_xs_btn;
    // if XSec name is controlled anywhere in the GUI for this geom (WingScreen for example has a separate XSec name controller)
    m_XSecNameInputControlled = add_xs_btn;

    m_XSecAliasLabel = xsnamelabel;
    m_XSecCurveAliasLabel = xscnamelabel;

    if ( m_XSecNameInputActive )
    {
        int stdwidth = m_XSecLayout.GetButtonWidth();
        int btnwidth = 5 * stdwidth / 6;
        m_XSecLayout.SetButtonWidth( 2 * btnwidth ); // 2x math operations here to get same rounding error as the IndexSelector buttonwidth
        string label = m_XSecAliasLabel;
        m_XSecLayout.AddInput( m_XSecNameInput, label.c_str() );
        m_XSecLayout.SetButtonWidth( stdwidth );
    }

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

    m_XSecLayout.SetSameLineFlag( true );
    m_XSecLayout.AddChoice( m_XSecTypeChoice, "Choose Type:", (show_w + convert_w) );
    m_XSecLayout.SetFitWidthFlag( false );
    m_XSecLayout.SetButtonWidth( show_w );
    m_XSecLayout.AddButton( m_ShowXSecButton, "Show" );

    m_XSecLayout.AddSubGroupLayout( m_ConvertCEDITGroup, m_XSecLayout.GetW(), m_XSecLayout.GetStdHeight() );
    m_ConvertCEDITGroup.SetButtonWidth( convert_w );
    m_ConvertCEDITGroup.SetFitWidthFlag( false );
    m_ConvertCEDITGroup.AddButton( m_ConvertCEDITButton, "Convert CEDIT" );

    m_XSecLayout.AddSubGroupLayout( m_EditCEDITButtonGroup, m_XSecLayout.GetW(), m_XSecLayout.GetStdHeight() );
    m_EditCEDITButtonGroup.SetFitWidthFlag( false );
    m_EditCEDITButtonGroup.SetButtonWidth( convert_w );
    m_EditCEDITButtonGroup.AddButton( m_EditCEDITButton, "Edit Curve" );

    m_XSecLayout.ForceNewLine();

    m_XSecLayout.InitWidthHeightVals();
    m_XSecLayout.SetFitWidthFlag( true );
    m_XSecLayout.SetSameLineFlag( false );
    m_XSecLayout.SetButtonWidth( convert_w );
    string label = m_XSecCurveAliasLabel;
    m_XSecLayout.AddInput( m_XSecCurveNameInput, label.c_str() );

    m_XSecLayout.AddYGap();

    m_XSecLayout.AddSubGroupLayout( m_EditCEDITGroup, m_XSecLayout.GetW(), m_XSecLayout.GetRemainY() );

    if (m_XSecDriversActive)
    {
        m_EditCEDITXSecDriverGroupBank.SetDriverGroup( &m_DefaultXSecDriverGroup );
        m_EditCEDITGroup.AddDriverGroupBank( m_EditCEDITXSecDriverGroupBank, xsec_driver_labels, 10, "%6.5f" );
    }
    else
    {
        m_EditCEDITGroup.AddSlider( m_EditCEDITHeightSlider, "Height", 10, "%6.5f" );
        m_EditCEDITGroup.AddSlider( m_EditCEDITWidthSlider, "Width", 10, "%6.5f" );
    }

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

    int oldbw = m_RoundedRectGroup.GetButtonWidth();

    m_RoundedRectGroup.AddLabel( "Symmetry:", oldbw );

    m_RoundedRectGroup.SetButtonWidth( m_RoundedRectGroup.GetRemainX() / 4 );
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

    m_RoundedRectGroup.SetButtonWidth( oldbw );

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

    m_SixSeriesChoice.AddItem( "63-" );
    m_SixSeriesChoice.AddItem( "64-" );
    m_SixSeriesChoice.AddItem( "65-" );
    m_SixSeriesChoice.AddItem( "66-" );
    m_SixSeriesChoice.AddItem( "67-" );
    m_SixSeriesChoice.AddItem( "63A" );
    m_SixSeriesChoice.AddItem( "64A" );
    m_SixSeriesChoice.AddItem( "65A" );
    m_SixSeriesGroup.AddChoice( m_SixSeriesChoice, "Series" );

    m_SixSeriesGroup.AddYGap();

    m_SixSeriesGroup.AddSlider( m_SixChordSlider, "Chord", 10, "%7.3f" );
    m_SixSeriesGroup.AddSlider( m_SixThickChordSlider, "T/C", 1, "%7.5f" );
    m_SixSeriesGroup.AddYGap();
    m_SixSeriesGroup.AddSlider( m_SixIdealClSlider, "Ideal CL", 1, "%7.5f" );
    m_SixSeriesGroup.AddSlider( m_SixASlider, "a", 1, "%7.5f" );
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
    m_WedgeGroup.SetSameLineFlag( true );
    m_WedgeGroup.AddSlider( m_WedgeUForeUpSlider, "U Fwd Up", 1, "%7.5f", m_WedgeGroup.GetW() * 0.5  );
    m_WedgeGroup.AddSlider( m_WedgeDuUpSlider, "dU Flat Up", 1, "%7.5f" );
    m_WedgeGroup.ForceNewLine();
    m_WedgeGroup.AddSlider( m_WedgeUForeLowSlider, "U Fwd Low", 1, "%7.5f", m_WedgeGroup.GetW() * 0.5  );
    m_WedgeGroup.AddSlider( m_WedgeDuLowSlider, "dU Flat Low", 1, "%7.5f" );
    m_WedgeGroup.ForceNewLine();
    m_WedgeGroup.SetSameLineFlag( false );
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

    m_CSTAirfoilGroup.SetSameLineFlag( true );
    m_CSTAirfoilGroup.AddButton( m_CSTContLERadButton, "Enforce Continuous LE Radius", m_CSTAirfoilGroup.GetW() * 0.5 );
    m_CSTAirfoilGroup.AddButton( m_CSTInvertButton, "Invert Airfoil", m_CSTAirfoilGroup.GetW() * 0.5 );
    m_CSTAirfoilGroup.ForceNewLine();
    m_CSTAirfoilGroup.SetSameLineFlag( false );

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

    vector < int > y_vals;
    y_vals.push_back( m_SuperGroup.GetY() );
    y_vals.push_back( m_CircleGroup.GetY() );
    y_vals.push_back( m_EllipseGroup.GetY() );
    y_vals.push_back( m_RoundedRectGroup.GetY() );
    y_vals.push_back( m_GenGroup.GetY() );
    y_vals.push_back( m_FourSeriesGroup.GetY() );
    y_vals.push_back( m_SixSeriesGroup.GetY() );
    y_vals.push_back( m_BiconvexGroup.GetY() );
    y_vals.push_back( m_WedgeGroup.GetY() );
    y_vals.push_back( m_FuseFileGroup.GetY() );
    y_vals.push_back( m_AfFileGroup.GetY() );
    y_vals.push_back( m_CSTAirfoilGroup.GetY() );
    y_vals.push_back( m_CSTLowCoeffLayout.GetY() );
    y_vals.push_back( m_VKTGroup.GetY() );
    y_vals.push_back( m_FourDigitModGroup.GetY() );
    y_vals.push_back( m_FiveDigitGroup.GetY() );
    y_vals.push_back( m_FiveDigitModGroup.GetY() );
    y_vals.push_back( m_OneSixSeriesGroup.GetY() );

    m_XSecLayout.SetY( *max_element( y_vals.begin(), y_vals.end() ) );
    m_XSecLayout.AddYGap();
    m_XsecAttributeEditor.Init( &m_XSecLayout, m_XSecLayout.GetGroup(), this, staticScreenCB, true, m_GenLayout.GetY(), 100);

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
        if ( m_XSecNameInputActive )
        {
            m_XSecNameInput.Update( xs->GetGroupAlias() );
        }

        XSecCurve* xsc = xs->GetXSecCurve();
        if (xsc)
        {
            XSecViewScreen *xsscreen = dynamic_cast < XSecViewScreen* > ( m_ScreenMgr->GetScreen( vsp::VSP_XSEC_SCREEN ) );

            if ( xsscreen )
            {
                xsscreen->SetXSecCurve( xsc );
            }

            m_XSecCurveNameInput.Update( xsc->GetGroupAlias() );

            //==== Attributes ====//
            m_XsecAttributeEditor.SetEditorCollID( xsc->GetAttrCollection()->GetID() );
            m_XsecAttributeEditor.Update();

            m_XSecTypeChoice.SetVal( xsc->GetType() );

            if (xsc->GetType() == XS_POINT)
            {
                DisplayGroup( nullptr );
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
                    m_FourCamberGroup.Deactivate();
                }
                else if (fs_xs->m_CamberInputFlag() == MAX_CAMB)
                {
                    m_FourCamberSlider.Activate();
                    m_FourCLiSlider.Deactivate();
                    m_FourCamberGroup.Activate();
                }
                else
                {
                    m_FourCamberSlider.Deactivate();
                    m_FourCLiSlider.Activate();
                    m_FourCamberGroup.Activate();
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
                snprintf( str, sizeof( str ),  "%d", cst_xs->m_UpDeg() );
                m_UpDegreeOutput.Update( str );
                snprintf( str, sizeof( str ),  "%d", cst_xs->m_LowDeg() );
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
                if (geom_ptr->GetType().m_Type == PROP_GEOM_TYPE)
                {
                    m_FourModCamberSlider.Deactivate();
                    m_FourModCLiSlider.Deactivate();
                    m_FourModCamberGroup.Deactivate();
                }
                else if (fs_xs->m_CamberInputFlag() == MAX_CAMB)
                {
                    m_FourModCamberSlider.Activate();
                    m_FourModCLiSlider.Deactivate();
                    m_FourModCamberGroup.Activate();
                }
                else
                {
                    m_FourModCamberSlider.Deactivate();
                    m_FourModCLiSlider.Activate();
                    m_FourModCamberGroup.Activate();
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
                m_EditCEDITButtonGroup.Show();
                m_ConvertCEDITGroup.Hide();

                DisplayGroup( &m_EditCEDITGroup );

                EditCurveXSec* edit_xsec = dynamic_cast<EditCurveXSec*>(xsc);

                if (m_XSecDriversActive)
                {
                    m_EditCEDITXSecDriverGroupBank.SetDriverGroup( edit_xsec->m_DriverGroup );
                    vector< string > parm_ids = edit_xsec->GetDriverParms();
                    m_EditCEDITXSecDriverGroupBank.Update( parm_ids );
                }
                else
                {
                    m_EditCEDITHeightSlider.Update( edit_xsec->m_Height.GetID() );
                    m_EditCEDITWidthSlider.Update( edit_xsec->m_Width.GetID() );
                }
            }

            if (xsc->GetType() != XS_EDIT_CURVE)
            {
                m_EditCEDITButtonGroup.Hide();
                m_ConvertCEDITGroup.Show();
            }
        }
    }

    GeomScreen::Update();


    return true;
}

void XSecScreen::GetCollIDs( vector < string > &collIDVec )
{
    m_XsecAttributeEditor.GetCollIDs( collIDVec );
    GeomScreen::GetCollIDs( collIDVec );
}

void XSecScreen::DisplayGroup( GroupLayout* group )
{
    if (m_CurrDisplayGroup == group)
    {
        return;
    }

    m_EditCEDITGroup.Hide();
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
    m_XsecAttributeEditor.DeviceCB( w );
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
    if (gui_device == &m_XSecNameInput)
    {
        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            xs->SetGroupAlias( m_XSecNameInput.GetString() );
        }
        ParmMgr.SetDirtyFlag( true );
    }
    else if ( gui_device == &m_XSecCurveNameInput )
    {
        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            if ( !m_XSecNameInputControlled )
            {
                xs->SetGroupAlias( m_XSecCurveNameInput.GetString() );
            }
    
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                xsc->SetGroupAlias( m_XSecCurveNameInput.GetString() );
            }
        }
        ParmMgr.SetDirtyFlag( true );
    }
    else if (gui_device == &m_XSecTypeChoice)
    {
        int t = m_XSecTypeChoice.GetVal();
        xsec_geom_ptr->SetActiveXSecType( t );

        if (t == XS_EDIT_CURVE)
        {
            m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
        }
    }
    else if (gui_device == &m_ShowXSecButton)
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_XSEC_SCREEN );
    }
    else if (gui_device == &m_ConvertCEDITButton)
    {
        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            EditCurveXSec* edit_xsec = xs->ConvertToEdit();

            if (edit_xsec)
            {
                m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
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
        m_ScreenMgr->ShowScreen( vsp::VSP_CURVE_EDIT_SCREEN );
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
                    string newfile = m_ScreenMgr->FileChooser( "Fuselage Cross Section", "*.fxs" );

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
                    string newfile = m_ScreenMgr->FileChooser( "Airfoil File", "*.{af,dat}" );

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

    m_XsecAttributeEditor.GuiDeviceCallBack( gui_device );
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
SkinScreen::SkinScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile ) :
    XSecScreen( mgr, w, h, title, helpfile, "Sect Alias", "Curve Alias" ) // do not combine xs and xsc
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


    int stdwidth = m_SkinLayout.GetButtonWidth();
    int btnwidth = 5 * stdwidth / 6;
    m_SkinLayout.SetButtonWidth( 2 * btnwidth ); // 2x math operations here to get same rounding error as the IndexSelector buttonwidth
    string label = m_XSecCurveAliasLabel;
    m_SkinLayout.AddInput( m_SkinXSecCurveNameInput, label.c_str() );
    m_SkinLayout.SetButtonWidth( stdwidth );
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
        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            m_SkinXSecCurveNameInput.Update( xsc->GetGroupAlias() );
        }

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
    else if ( gui_device == &m_SkinXSecCurveNameInput )
    {
        int xsid = geomxsec_ptr->m_ActiveXSec();
        SkinXSec* xs = (SkinXSec*) geomxsec_ptr->GetXSec( xsid );
        if ( xs )
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                xsc->SetGroupAlias( m_SkinXSecCurveNameInput.GetString() );
            }
        }
        ParmMgr.SetDirtyFlag( true );
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
EngineModelScreen::EngineModelScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile ) :
        SkinScreen( mgr, w, h, title, helpfile )
{
    m_SelectedEngineExtensionSetChoiceIndex = -1;
}

void EngineModelScreen::BuildEngineGUI( GroupLayout & layout )
{

    layout.AddYGap();

    layout.AddDividerBox( "Engine Definition" );

    m_EngineGeomIOChoice.AddItem( "NONE", vsp::ENGINE_GEOM_NONE );
    m_EngineGeomIOChoice.AddItem( "INLET", vsp::ENGINE_GEOM_INLET );
    m_EngineGeomIOChoice.AddItem( "BOTH", vsp::ENGINE_GEOM_INLET_OUTLET );
    m_EngineGeomIOChoice.AddItem( "OUTLET", vsp::ENGINE_GEOM_OUTLET );
    layout.AddChoice( m_EngineGeomIOChoice, "Geom Type:" );

    m_EngineGeomInChoice.AddItem( "FLOWTHROUGH", vsp::ENGINE_GEOM_FLOWTHROUGH );
    m_EngineGeomInChoice.AddItem( "FLOWPATH", vsp::ENGINE_GEOM_FLOWPATH );
    m_EngineGeomInChoice.AddItem( "INLET TO LIP", vsp::ENGINE_GEOM_TO_LIP );
    m_EngineGeomInChoice.AddItem( "INLET TO FACE", vsp::ENGINE_GEOM_TO_FACE );
    layout.AddChoice( m_EngineGeomInChoice, "Inlet Type:" );

    m_EngineGeomOutChoice.AddItem( "FLOWPATH", vsp::ENGINE_GEOM_FLOWPATH );
    m_EngineGeomOutChoice.AddItem( "OUTLET TO LIP", vsp::ENGINE_GEOM_TO_LIP );
    m_EngineGeomOutChoice.AddItem( "OUTLET TO FACE", vsp::ENGINE_GEOM_TO_FACE );
    layout.AddChoice( m_EngineGeomOutChoice, "Outlet Type:" );

    int button_w = layout.GetButtonWidth();
    int slider_w = layout.GetSliderWidth();
    int sm_but_w = 40;
    int toggle_w = 15;
    int add_w = 5;

    vector< int > val_map_vec { ENGINE_LOC_INDEX, ENGINE_LOC_U };

    layout.SetSameLineFlag( true );

    layout.AddYGap();

    layout.SetFitWidthFlag( false );
    layout.AddLabel( "Inlet Lip:", button_w, FL_CYAN );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineInLipIndexToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetSliderWidth( 50 );
    layout.AddCounter( m_EngineInLipCounter, "Indx" );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineInLipUToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetFitWidthFlag( true );
    layout.SetSliderWidth( slider_w );
    layout.AddSlider( m_EngineInLipUSlider, "U", 10.0, "%6.5f" );
    layout.ForceNewLine();

    m_EngineInLipToggleGroup.Init( this );
    m_EngineInLipToggleGroup.AddButton( m_EngineInLipIndexToggle.GetFlButton() );
    m_EngineInLipToggleGroup.AddButton( m_EngineInLipUToggle.GetFlButton() );
    m_EngineInLipToggleGroup.SetValMapVec( val_map_vec );

    layout.SetFitWidthFlag( false );
    layout.AddLabel( "Inlet Face:", button_w, FL_MAGENTA );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineInFaceIndexToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetSliderWidth( 50 );
    layout.AddCounter( m_EngineInFaceCounter, "Indx" );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineInFaceUToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetFitWidthFlag( true );
    layout.SetSliderWidth( slider_w );
    layout.AddSlider( m_EngineInFaceUSlider, "U", 10.0, "%6.5f" );
    layout.ForceNewLine();

    m_EngineInFaceToggleGroup.Init( this );
    m_EngineInFaceToggleGroup.AddButton( m_EngineInFaceIndexToggle.GetFlButton() );
    m_EngineInFaceToggleGroup.AddButton( m_EngineInFaceUToggle.GetFlButton() );
    m_EngineInFaceToggleGroup.SetValMapVec( val_map_vec );

    layout.SetFitWidthFlag( false );
    layout.AddLabel( "Outlet Face:", button_w, FL_GREEN );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineOutFaceIndexToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetSliderWidth( 50 );
    layout.AddCounter( m_EngineOutFaceCounter, "Indx" );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineOutFaceUToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetFitWidthFlag( true );
    layout.SetSliderWidth( slider_w );
    layout.AddSlider( m_EngineOutFaceUSlider, "U", 10.0, "%6.5f" );
    layout.ForceNewLine();

    m_EngineOutFaceToggleGroup.Init( this );
    m_EngineOutFaceToggleGroup.AddButton( m_EngineOutFaceIndexToggle.GetFlButton() );
    m_EngineOutFaceToggleGroup.AddButton( m_EngineOutFaceUToggle.GetFlButton() );
    m_EngineOutFaceToggleGroup.SetValMapVec( val_map_vec );

    layout.SetFitWidthFlag( false );
    layout.AddLabel( "Outlet Lip:", button_w, FL_YELLOW );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineOutLipIndexToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetSliderWidth( 50 );
    layout.AddCounter( m_EngineOutLipCounter, "Indx" );
    layout.AddX( add_w );
    layout.SetButtonWidth( toggle_w );
    layout.AddButton( m_EngineOutLipUToggle, "" );
    layout.SetButtonWidth( sm_but_w );
    layout.SetFitWidthFlag( true );
    layout.SetSliderWidth( slider_w );
    layout.AddSlider( m_EngineOutLipUSlider, "U", 10.0, "%6.5f" );
    layout.ForceNewLine();

    m_EngineOutLipToggleGroup.Init( this );
    m_EngineOutLipToggleGroup.AddButton( m_EngineOutLipIndexToggle.GetFlButton() );
    m_EngineOutLipToggleGroup.AddButton( m_EngineOutLipUToggle.GetFlButton() );
    m_EngineOutLipToggleGroup.SetValMapVec( val_map_vec );

    layout.SetSameLineFlag( false );

    layout.AddYGap();
    layout.AddDividerBox( "Engine Representation" );

    m_EngineInModeChoice.AddItem( "FLOWTHROUGH", vsp::ENGINE_MODE_FLOWTHROUGH );
    m_EngineInModeChoice.AddItem( "FLOWTHROUGH NEGATIVE", vsp::ENGINE_MODE_FLOWTHROUGH_NEG );
    m_EngineInModeChoice.AddItem( "TO LIP", vsp::ENGINE_MODE_TO_LIP );
    m_EngineInModeChoice.AddItem( "TO FACE", vsp::ENGINE_MODE_TO_FACE );
    m_EngineInModeChoice.AddItem( "TO FACE NEGATIVE", vsp::ENGINE_MODE_TO_FACE_NEG );
    m_EngineInModeChoice.AddItem( "EXTEND", vsp::ENGINE_MODE_EXTEND );
    layout.AddChoice( m_EngineInModeChoice, "Inlet Mode:" );

    m_EngineOutModeChoice.AddItem( "TO LIP", vsp::ENGINE_MODE_TO_LIP );
    m_EngineOutModeChoice.AddItem( "TO FACE", vsp::ENGINE_MODE_TO_FACE );
    m_EngineOutModeChoice.AddItem( "TO FACE NEGATIVE", vsp::ENGINE_MODE_TO_FACE_NEG );
    m_EngineOutModeChoice.AddItem( "EXTEND", vsp::ENGINE_MODE_EXTEND );
    layout.AddChoice( m_EngineOutModeChoice, "Outlet Mode:" );

    layout.AddYGap();
    layout.AddDividerBox( "Extension" );

    layout.SetFitWidthFlag( false );
    layout.SetSameLineFlag( true );

    layout.SetButtonWidth( layout.GetW() / 6 );
    layout.SetChoiceButtonWidth( layout.GetButtonWidth() );
    layout.SetSliderWidth( layout.GetButtonWidth() );

    layout.AddButton( m_EngineAutoExtensionFlagButton, "Auto" );
    layout.AddChoice( m_EngineAutoExtensionSetChoice, "Set:" );
    layout.SetFitWidthFlag( true );
    layout.AddSlider( m_EngineExtendDistanceSlider, "Distance", 10.0, "%7.3f" );

    m_SelectedEngineExtensionSetChoiceIndex = DEFAULT_SET;
}

bool EngineModelScreen::Update( )
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        Hide();
        return false;
    }

    SkinScreen::Update();

    GeomEngine* geomengine_ptr = dynamic_cast< GeomEngine* >( geom_ptr );
    assert( geomengine_ptr );

    // Reset all Choice entries to normal (enabled) vs. FL_MENU_INACTIVE
    // Most of these are not needed.
    m_EngineGeomIOChoice.SetFlagByVal( vsp::ENGINE_GEOM_NONE, 0 );
    m_EngineGeomIOChoice.SetFlagByVal( vsp::ENGINE_GEOM_INLET, 0 );
    m_EngineGeomIOChoice.SetFlagByVal( vsp::ENGINE_GEOM_INLET_OUTLET, 0 );
    m_EngineGeomIOChoice.SetFlagByVal( vsp::ENGINE_GEOM_OUTLET, 0 );

    m_EngineGeomInChoice.SetFlagByVal( vsp::ENGINE_GEOM_FLOWTHROUGH, 0 );
    m_EngineGeomInChoice.SetFlagByVal( vsp::ENGINE_GEOM_FLOWPATH, 0 );
    m_EngineGeomInChoice.SetFlagByVal( vsp::ENGINE_GEOM_TO_LIP, 0 );
    m_EngineGeomInChoice.SetFlagByVal( vsp::ENGINE_GEOM_TO_FACE, 0 );

    m_EngineGeomOutChoice.SetFlagByVal( vsp::ENGINE_GEOM_FLOWPATH, 0 );
    m_EngineGeomOutChoice.SetFlagByVal( vsp::ENGINE_GEOM_TO_LIP, 0 );
    m_EngineGeomOutChoice.SetFlagByVal( vsp::ENGINE_GEOM_TO_FACE, 0 );

    m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_FLOWTHROUGH, 0 );
    m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_FLOWTHROUGH_NEG, 0 );
    m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_LIP, 0 );
    m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, 0 );
    m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE_NEG, 0 );
    m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_EXTEND, 0 );

    m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_LIP, 0 );
    m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, 0 );
    m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE_NEG, 0 );
    m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_EXTEND, 0 );

    m_EngineGeomIOChoice.Update( geomengine_ptr->m_EngineGeomIOType.GetID() );
    m_EngineGeomInChoice.Update( geomengine_ptr->m_EngineGeomInType.GetID() );
    m_EngineGeomOutChoice.Update( geomengine_ptr->m_EngineGeomOutType.GetID() );

    m_EngineInLipToggleGroup.Update( geomengine_ptr->m_EngineInLipMode.GetID() );
    m_EngineInLipCounter.Update( geomengine_ptr->m_EngineInLipIndex.GetID() );
    m_EngineInLipUSlider.Update( geomengine_ptr->m_EngineInLipU.GetID() );

    m_EngineInFaceToggleGroup.Update( geomengine_ptr->m_EngineInFaceMode.GetID() );
    m_EngineInFaceCounter.Update( geomengine_ptr->m_EngineInFaceIndex.GetID() );
    m_EngineInFaceUSlider.Update( geomengine_ptr->m_EngineInFaceU.GetID() );

    m_EngineOutLipToggleGroup.Update( geomengine_ptr->m_EngineOutLipMode.GetID() );
    m_EngineOutLipCounter.Update( geomengine_ptr->m_EngineOutLipIndex.GetID() );
    m_EngineOutLipUSlider.Update( geomengine_ptr->m_EngineOutLipU.GetID() );

    m_EngineOutFaceToggleGroup.Update( geomengine_ptr->m_EngineOutFaceMode.GetID() );
    m_EngineOutFaceCounter.Update( geomengine_ptr->m_EngineOutFaceIndex.GetID() );
    m_EngineOutFaceUSlider.Update( geomengine_ptr->m_EngineOutFaceU.GetID() );

    m_EngineInModeChoice.Update( geomengine_ptr->m_EngineInModeType.GetID() );
    m_EngineOutModeChoice.Update( geomengine_ptr->m_EngineOutModeType.GetID() );


    // Deactivate choice entries based on type
    if ( geomengine_ptr->m_EngineGeomInType() != ENGINE_GEOM_FLOWTHROUGH )
    {
        m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_FLOWTHROUGH, FL_MENU_INACTIVE );
        m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_FLOWTHROUGH_NEG, FL_MENU_INACTIVE );
    }

    if ( geomengine_ptr->m_EngineGeomInType() == ENGINE_GEOM_TO_LIP )
    {
        m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, FL_MENU_INACTIVE );
        m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE_NEG, FL_MENU_INACTIVE );
    }

    if ( geomengine_ptr->m_EngineGeomOutType() == ENGINE_GEOM_TO_LIP )
    {
        m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, FL_MENU_INACTIVE );
        m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE_NEG, FL_MENU_INACTIVE );
    }

    if ( geomengine_ptr->m_EngineGeomOutType() == ENGINE_GEOM_FLOWPATH ||
         ( geomengine_ptr->m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET && geomengine_ptr->m_EngineGeomInType() == ENGINE_GEOM_FLOWPATH ) )
    {
        m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, FL_MENU_INACTIVE );
    }

    if ( geomengine_ptr->m_EngineGeomInType() == ENGINE_GEOM_FLOWPATH )
    {
        m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, FL_MENU_INACTIVE );
        if ( geomengine_ptr->m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET )
        {
            m_EngineInModeChoice.SetFlagByVal( vsp::ENGINE_MODE_FLOWTHROUGH_NEG, 0 );          // Re-activate, order matters!
        }
    }

    if ( geomengine_ptr->m_EngineGeomInType() == ENGINE_GEOM_FLOWTHROUGH )
    {
        if ( geomengine_ptr->m_EngineGeomIOType() == ENGINE_GEOM_INLET_OUTLET )
        {
            m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE, 0 );
            m_EngineOutModeChoice.SetFlagByVal( vsp::ENGINE_MODE_TO_FACE_NEG, 0 );
        }
    }

    // Update menu while keeping setting.  Required to deactivate entries.
    m_EngineGeomIOChoice.UpdateItems( true );
    m_EngineGeomInChoice.UpdateItems( true );
    m_EngineGeomOutChoice.UpdateItems( true );
    m_EngineInModeChoice.UpdateItems( true );
    m_EngineOutModeChoice.UpdateItems( true );

    m_EngineExtendDistanceSlider.Update( geomengine_ptr->m_ExtensionDistance.GetID() );

    m_EngineAutoExtensionFlagButton.Update( geomengine_ptr->m_AutoExtensionFlag.GetID() );

    m_ScreenMgr->LoadSetChoice( {&m_EngineAutoExtensionSetChoice}, {geomengine_ptr->m_AutoExtensionSet.GetID()} );


    return true;
}

void EngineModelScreen::CallBack( Fl_Widget *w )
{
    SkinScreen::CallBack( w );
}

void EngineModelScreen::GuiDeviceCallBack( GuiDevice* device )
{
    SkinScreen::GuiDeviceCallBack( device );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr )
    {
        return;
    }

    GeomEngine* geomengine_ptr = dynamic_cast< GeomEngine* >( geom_ptr );
    assert( geomengine_ptr );

    if ( device == &m_EngineAutoExtensionSetChoice )
    {
        geomengine_ptr->m_AutoExtensionSet = m_EngineAutoExtensionSetChoice.GetVal();
    }

}

//=====================================================================//
//=====================================================================//
//=====================================================================//
ChevronScreen::ChevronScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile ) :
        EngineModelScreen( mgr, w, h, title, helpfile )
{

    //==== XSec Modifications ====//

    Fl_Group* modify_tab = AddTab( "Modify" );
    Fl_Group* modify_group = AddSubGroup( modify_tab, 5 );

    m_ModifyLayout.SetButtonWidth( 70 );

    m_ModifyLayout.SetGroupAndScreen( modify_group, this );
    m_ModifyLayout.AddDividerBox( "XSec" );

    m_ModifyLayout.AddIndexSelector( m_XsecModIndexSelector );

    int stdwidth = m_ModifyLayout.GetButtonWidth();
    int btnwidth = 5 * stdwidth / 6;
    m_ModifyLayout.SetButtonWidth( 2 * btnwidth ); // 2x math operations here to get same rounding error as the IndexSelector buttonwidth
    string label = m_XSecCurveAliasLabel;
    m_ModifyLayout.AddInput( m_ChevronXSecCurveNameInput, label.c_str() );
    m_ModifyLayout.SetButtonWidth( stdwidth );

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

    m_ChevronExtentModeChoice.AddItem( "Start / End", vsp::CHEVRON_W01_SE );
    m_ChevronExtentModeChoice.AddItem( "Center / Width", vsp::CHEVRON_W01_CW );
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

    EngineModelScreen::Update();

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
            m_ChevronXSecCurveNameInput.Update( xsc->GetGroupAlias() );

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
    EngineModelScreen::GuiDeviceCallBack( gui_device );

    if ( gui_device == &m_ChevronXSecCurveNameInput )
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

        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                xsc->SetGroupAlias( m_ChevronXSecCurveNameInput.GetString() );
            }
        }
        ParmMgr.SetDirtyFlag( true );
    }

}

//==== Fltk  Callbacks ====//
void ChevronScreen::CallBack( Fl_Widget *w )
{
    EngineModelScreen::CallBack( w );
}


//=====================================================================//
//=====================================================================//
//=====================================================================//
BlendScreen::BlendScreen( ScreenMgr* mgr, int w, int h, const string & title, const string & helpfile ) :
    XSecScreen( mgr, w, h, title, helpfile, "Sect Alias", "Airfoil Alias", false ) //combine xs and xsc
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

    int stdwidth = m_BlendLayout.GetButtonWidth();
    int btnwidth = 5 * stdwidth / 6;
    m_BlendLayout.SetButtonWidth( 2 * btnwidth ); // 2x math operations here to get same rounding error as the IndexSelector buttonwidth
    string label = m_XSecCurveAliasLabel;
    m_BlendLayout.AddInput( m_BlendXSecCurveNameInput, label.c_str() );
    m_BlendLayout.SetButtonWidth( stdwidth );
    m_BlendLayout.AddYGap();

    m_BlendLayout.AddYGap();

    m_BlendLayout.AddDividerBox( "Leading Edge" );
    m_BlendLayout.AddYGap();

    m_BlendLayout.AddSubGroupLayout( m_InLELayout, m_BlendLayout.GetW()/2 - 2, m_BlendLayout.GetStdHeight() * 4 + m_BlendLayout.GetDividerHeight() );
    m_BlendLayout.AddX( m_BlendLayout.GetW()/2 + 2 );
    m_BlendLayout.AddSubGroupLayout( m_OutLELayout, m_BlendLayout.GetW()/2 - 2, m_BlendLayout.GetStdHeight() * 4 + m_BlendLayout.GetDividerHeight() );

    m_InLEChoice.AddItem( "FREE", vsp::BLEND_FREE );
    m_InLEChoice.AddItem( "ANGLES", vsp::BLEND_ANGLES );
    m_InLEChoice.AddItem( "IN_LE_TRAP", vsp::BLEND_MATCH_IN_LE_TRAP );
    m_InLEChoice.AddItem( "IN_TE_TRAP", vsp::BLEND_MATCH_IN_TE_TRAP );
    m_InLEChoice.AddItem( "OUT_LE_TRAP", vsp::BLEND_MATCH_OUT_LE_TRAP );
    m_InLEChoice.AddItem( "OUT_TE_TRAP", vsp::BLEND_MATCH_OUT_TE_TRAP );
    m_InLEChoice.AddItem( "IN_ANGLES", vsp::BLEND_MATCH_IN_ANGLES );
    m_InLEChoice.AddItem( "LE_ANGLES", vsp::BLEND_MATCH_LE_ANGLES );
    m_InLEChoice.SetFlagByVal( BLEND_MATCH_IN_ANGLES, FL_MENU_INVISIBLE );
    m_InLEChoice.SetFlagByVal( BLEND_MATCH_LE_ANGLES, FL_MENU_INVISIBLE );

    m_InLELayout.SetButtonWidth( bw );
    m_InLELayout.SetChoiceButtonWidth( m_InLELayout.GetButtonWidth() );
    m_InLELayout.AddDividerBox( "Inboard" );
    m_InLELayout.AddChoice( m_InLEChoice, "Match:" );
    m_InLELayout.AddSlider( m_InLESweep, "Sweep", angleRng, angleFmt );
    m_InLELayout.AddSlider( m_InLEDihedral, "Dihedral", dihRng, angleFmt );
    m_InLELayout.AddSlider( m_InLEStrength, "Strength", strengthRng, strengthFmt );

    m_OutLEChoice.AddItem( "FREE", vsp::BLEND_FREE );
    m_OutLEChoice.AddItem( "ANGLES", vsp::BLEND_ANGLES );
    m_OutLEChoice.AddItem( "IN_LE_TRAP", vsp::BLEND_MATCH_IN_LE_TRAP );
    m_OutLEChoice.AddItem( "IN_TE_TRAP", vsp::BLEND_MATCH_IN_TE_TRAP );
    m_OutLEChoice.AddItem( "OUT_LE_TRAP", vsp::BLEND_MATCH_OUT_LE_TRAP );
    m_OutLEChoice.AddItem( "OUT_TE_TRAP", vsp::BLEND_MATCH_OUT_TE_TRAP );
    m_OutLEChoice.AddItem( "IN_ANGLES", vsp::BLEND_MATCH_IN_ANGLES );
    m_OutLEChoice.AddItem( "LE_ANGLES", vsp::BLEND_MATCH_LE_ANGLES );
    m_OutLEChoice.SetFlagByVal( BLEND_MATCH_LE_ANGLES, FL_MENU_INVISIBLE );

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


    m_InTEChoice.AddItem( "FREE", vsp::BLEND_FREE );
    m_InTEChoice.AddItem( "ANGLES", vsp::BLEND_ANGLES );
    m_InTEChoice.AddItem( "IN_LE_TRAP", vsp::BLEND_MATCH_IN_LE_TRAP );
    m_InTEChoice.AddItem( "IN_TE_TRAP", vsp::BLEND_MATCH_IN_TE_TRAP );
    m_InTEChoice.AddItem( "OUT_LE_TRAP", vsp::BLEND_MATCH_OUT_LE_TRAP );
    m_InTEChoice.AddItem( "OUT_TE_TRAP", vsp::BLEND_MATCH_OUT_TE_TRAP );
    m_InTEChoice.AddItem( "IN_ANGLES", vsp::BLEND_MATCH_IN_ANGLES );
    m_InTEChoice.AddItem( "LE_ANGLES", vsp::BLEND_MATCH_LE_ANGLES );
    m_InTEChoice.SetFlagByVal( BLEND_MATCH_IN_ANGLES, FL_MENU_INVISIBLE );

    m_InTELayout.SetButtonWidth( bw );
    m_InTELayout.SetChoiceButtonWidth( m_InTELayout.GetButtonWidth() );
    m_InTELayout.AddDividerBox( "Inboard" );
    m_InTELayout.AddChoice( m_InTEChoice, "Match:" );
    m_InTELayout.AddSlider( m_InTESweep, "Sweep", angleRng, angleFmt );
    m_InTELayout.AddSlider( m_InTEDihedral, "Dihedral", dihRng, angleFmt );
    m_InTELayout.AddSlider( m_InTEStrength, "Strength", strengthRng, strengthFmt );


    m_OutTEChoice.AddItem( "FREE", vsp::BLEND_FREE );
    m_OutTEChoice.AddItem( "ANGLES", vsp::BLEND_ANGLES );
    m_OutTEChoice.AddItem( "IN_LE_TRAP", vsp::BLEND_MATCH_IN_LE_TRAP );
    m_OutTEChoice.AddItem( "IN_TE_TRAP", vsp::BLEND_MATCH_IN_TE_TRAP );
    m_OutTEChoice.AddItem( "OUT_LE_TRAP", vsp::BLEND_MATCH_OUT_LE_TRAP );
    m_OutTEChoice.AddItem( "OUT_TE_TRAP", vsp::BLEND_MATCH_OUT_TE_TRAP );
    m_OutTEChoice.AddItem( "IN_ANGLES", vsp::BLEND_MATCH_IN_ANGLES );
    m_OutTEChoice.AddItem( "LE_ANGLES", vsp::BLEND_MATCH_LE_ANGLES );

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
        XSecCurve* xsc = xs->GetXSecCurve();
        if ( xsc )
        {
            m_BlendXSecCurveNameInput.Update( xsc->GetGroupAlias() );
        }

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
        m_InTEChoice.SetFlagByVal( BLEND_MATCH_LE_ANGLES, 0 );

        m_OutLEChoice.SetFlagByVal( BLEND_MATCH_IN_ANGLES, 0 );

        m_OutTEChoice.SetFlagByVal( BLEND_MATCH_IN_ANGLES, 0 );
        m_OutTEChoice.SetFlagByVal( BLEND_MATCH_LE_ANGLES, 0 );

        m_InLEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_InLEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_InLEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_InLEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, 0 );

        m_InTEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_InTEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_InTEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_InTEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, 0 );

        m_OutLEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_OutLEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_OutLEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_OutLEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, 0 );

        m_OutTEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, 0 );
        m_OutTEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, 0 );
        m_OutTEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, 0 );
        m_OutTEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, 0 );

        if ( xs->m_InLEMode() != BLEND_ANGLES )
        {
            m_InTEChoice.SetFlagByVal( BLEND_MATCH_LE_ANGLES, FL_MENU_INACTIVE );
            m_OutLEChoice.SetFlagByVal( BLEND_MATCH_IN_ANGLES, FL_MENU_INACTIVE );
        }

        if ( xs->m_InTEMode() != BLEND_ANGLES && xs->m_InTEMode() != BLEND_MATCH_LE_ANGLES )
        {
            m_OutTEChoice.SetFlagByVal( BLEND_MATCH_IN_ANGLES, FL_MENU_INACTIVE );
        }

        if ( xs->m_OutLEMode() != BLEND_ANGLES && xs->m_OutLEMode() != BLEND_MATCH_IN_ANGLES )
        {
            m_OutTEChoice.SetFlagByVal( BLEND_MATCH_LE_ANGLES, FL_MENU_INACTIVE );
        }

        if ( firstxs )
        {
            m_InLEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_InLEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );

            m_InTEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_InTEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );

            m_OutLEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_OutLEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );

            m_OutTEChoice.SetFlagByVal( BLEND_MATCH_IN_LE_TRAP, FL_MENU_INACTIVE );
            m_OutTEChoice.SetFlagByVal( BLEND_MATCH_IN_TE_TRAP, FL_MENU_INACTIVE );
        }

        if ( lastxs )
        {
            m_InLEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_InLEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );

            m_InTEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_InTEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );

            m_OutLEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_OutLEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );

            m_OutTEChoice.SetFlagByVal( BLEND_MATCH_OUT_LE_TRAP, FL_MENU_INACTIVE );
            m_OutTEChoice.SetFlagByVal( BLEND_MATCH_OUT_TE_TRAP, FL_MENU_INACTIVE );
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

void BlendScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    if ( gui_device == &m_BlendXSecCurveNameInput )
    {
        //==== Find Fuselage Ptr ====//
        Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
        if (!geom_ptr)
        {
            return;
        }
        GeomXSec* xsec_geom_ptr = dynamic_cast<GeomXSec*>(geom_ptr);
        assert( xsec_geom_ptr );

        XSec* xs = xsec_geom_ptr->GetXSec( xsec_geom_ptr->m_ActiveXSec() );

        if (xs)
        {
            XSecCurve* xsc = xs->GetXSecCurve();
            if ( xsc )
            {
                xsc->SetGroupAlias( m_BlendXSecCurveNameInput.GetString() );
            }
        }

        ParmMgr.SetDirtyFlag( true );
    }
    XSecScreen::GuiDeviceCallBack( gui_device );
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
    m_GlWin = new VSPGUI::VspSubGlWindow( window_x, window_y, window_w_h, window_w_h, mgr, DrawObj::VSP_XSEC_SCREEN);
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

    m_XSecCurve = nullptr;
}

bool XSecViewScreen::Update()
{
    assert( m_ScreenMgr );

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    BasicScreen::Update();

    XSecCurve* xsc = m_XSecCurve;

    if( !xsc )
    {
        Hide();
        return false;
    }

    UpdateDrawObj();

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

    XSecCurve* xsc = m_XSecCurve;

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
        std::string fileName = m_ScreenMgr->FileChooser( "Select Image File", "*.{jpg,png,tga,bmp,gif}" );

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

void XSecViewScreen::UpdateDrawObj()
{
    XSecCurve* xsc = m_XSecCurve;
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if( xsc && veh )
    {
        double w = xsc->GetWidth();
        double h = xsc->GetHeight();

        m_CurveDO.m_Screen = DrawObj::VSP_XSEC_SCREEN;
        m_CurveDO.m_GeomID = XSECHEADER + xsc->GetID();
        m_CurveDO.m_GeomChanged = true;
        m_CurveDO.m_Visible = true;

        if( w == 0 && h == 0 )
        {
            vector< vec3d > pts( 1, vec3d( 0, 0, 0 ) );
            m_CurveDO.m_PntVec = pts;
            m_CurveDO.m_PointSize = 5.0;
            m_CurveDO.m_PointColor = veh->GetXSecLineColor() / 255.; // normalize
            m_CurveDO.m_Type = DrawObj::VSP_POINTS;
        }
        else
        {
            double scale = 1.0;
            if( w > h ) scale = 1.0 / w;
            else scale = 1.0 / h;

            Matrix4d mat;
            XSecSurf::GetBasicTransformation( vsp::Z_DIR, vsp::X_DIR, vsp::XS_SHIFT_MID, false, w * scale, mat );
            mat.scale( scale );

            VspCurve crv = xsc->GetCurve();
            crv.Transform( mat );

            vector< vec3d > pts;
            crv.TessAdapt( pts, 1e-2, 10 );
            m_CurveDO.m_PntVec = pts;
            m_CurveDO.m_LineWidth = 1.5;
            m_CurveDO.m_LineColor = veh->GetXSecLineColor() / 255.; // normalize
            m_CurveDO.m_Type = DrawObj::VSP_LINES;
        }
    }
}

void XSecViewScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    draw_obj_vec.push_back( &m_CurveDO );
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
    m_GlWin = new VSPGUI::VspSubGlWindow( x, y, w, h, mgr, DrawObj::VSP_FEA_XSEC_SCREEN );
    m_FLTK_Window->end();

    m_GlWin->getGraphicEngine()->getDisplay()->changeView( VSPGraphic::Common::VSP_CAM_TOP );
    m_GlWin->getGraphicEngine()->getDisplay()->getViewport()->showGridOverlay( false );
}

bool FeaXSecScreen::Update()
{
    assert( m_ScreenMgr );

    BasicScreen::Update();

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

    FeaProperty* fea_prop = StructureMgr.GetCurrProperty();

    if ( fea_prop )
    {
        xsec_type = fea_prop->m_CrossSectType();
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

//=====================================================================//
//=====================================================================//
//=====================================================================//


Background3DPreviewScreen::Background3DPreviewScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 500, 500, "3D Background" )
{
    int x = m_FLTK_Window->x();
    int y = m_FLTK_Window->y();
    int w = m_FLTK_Window->w();
    int h = m_FLTK_Window->h();

    m_FLTK_Window->begin();
    m_GlWin = new VSPGUI::BG3DSubGlWindow( x, y, w, h, mgr, DrawObj::VSP_3DBG_PREVIEW );
    m_FLTK_Window->end();

    ((VSP_Window*)m_FLTK_Window)->SetResizeCallback( staticResizeCB, this );

    m_GlWin->getGraphicEngine()->getDisplay()->getViewport()->showGridOverlay( false );
}

bool Background3DPreviewScreen::Update()
{
    BasicScreen::Update();

    SetZoom();

    m_GlWin->update();
    m_GlWin->redraw();

    return true;
}

void Background3DPreviewScreen::Show()
{
    VspScreen::Show();

    SetZoom();

    if ( m_GlWin )
    {
        m_GlWin->show();
    }

}

void Background3DPreviewScreen::SetZoom()
{
    Background3D *bg3d = Background3DMgr.GetCurrentBackground3D();

    if ( bg3d )
    {
        double whi = ( 1.0 * bg3d->m_ImageW() ) / ( 1.0 * bg3d->m_ImageH() );

        double px_w, px_h;

        px_w = m_GlWin->pixel_w() - 10;
        px_h = m_GlWin->pixel_h() - 10;

        double whs = px_w / px_h;

        double oz;
        if ( whi > whs )
        {
            oz = 1.0 / px_w;
        }
        else
        {
            oz = 1.0 / ( px_h * whi );
        }

        m_GlWin->getGraphicEngine()->getDisplay()->getCamera()->relativeZoom( oz );
    }
}

void Background3DPreviewScreen::ResizeCallBack( Fl_Widget *w )
{
    SetZoom();
    m_ScreenMgr->SetUpdateFlag( true );
}
