//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageGeomScreen.h"
#include "ScreenMgr.h"
#include "StlHelper.h"

using namespace vsp;


//==== Constructor ====//
ManageGeomScreen::ManageGeomScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 235, 645, "Geom Browser" )
{
    m_VehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_LastTopLine = 0;
    m_SetIndex = 0;
    m_TypeIndex = 0;
    m_CollapseFlag = false;
    m_LastSelectedGeomID = "NONE";

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_FLTK_Window->resizable( m_MainLayout.GetGroup() );
    m_FLTK_Window->size_range( m_FLTK_Window->w(), m_FLTK_Window->h() );

    m_MainLayout.AddX( 5 );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddSubGroupLayout( m_HeadLayout, m_MainLayout.GetRemainX() - 5, 50 );
    m_MainLayout.AddY( 70 );
    m_MainLayout.SetX( 0 );
    m_MainLayout.AddSubGroupLayout( m_BodyLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() );
    m_MainLayout.GetGroup()->resizable( m_BodyLayout.GetGroup() );

    m_HeadLayout.SetSameLineFlag( true );
    m_HeadLayout.SetChoiceButtonWidth( 0 );
    m_HeadLayout.AddChoice( m_GeomTypeChoice, "", m_HeadLayout.GetButtonWidth() );
    m_HeadLayout.SetFitWidthFlag( false );
    m_HeadLayout.AddButton( m_AddGeomButton, "Add" );
    m_HeadLayout.SetSameLineFlag( false );
    m_HeadLayout.ForceNewLine();
    m_HeadLayout.SetFitWidthFlag( true );

    m_HeadLayout.SetButtonWidth( 0 );
    m_HeadLayout.AddYGap();
    m_HeadLayout.AddDividerBox( "Active:" );
    m_HeadLayout.AddInput( m_ActiveGeomInput, "" );

    m_BodyLayout.SetSameLineFlag( true );
    m_BodyLayout.SetFitWidthFlag( false );
    m_BodyLayout.AddSubGroupLayout( m_LeftLayout, 15, m_BodyLayout.GetRemainY() );
    m_BodyLayout.AddX( 15 );
    m_BodyLayout.AddSubGroupLayout( m_MidLayout, 140, m_BodyLayout.GetRemainY() );
    m_BodyLayout.GetGroup()->resizable( m_MidLayout.GetGroup() );
    m_BodyLayout.AddX( 145 );
    m_BodyLayout.AddSubGroupLayout( m_RightLayout, m_BodyLayout.GetRemainX(), m_BodyLayout.GetRemainY());

    m_LeftLayout.SetStdHeight( 50 );
    m_LeftLayout.AddButton( m_MoveTopButton, "@2<<" );
    m_LeftLayout.AddYGap();
    m_LeftLayout.AddButton( m_MoveUpButton, "@2<" );
    m_LeftLayout.AddY( m_LeftLayout.GetRemainY() - 100 - 6 - 5 - m_LeftLayout.GetStdHeight() );
    m_LeftLayout.AddResizeBox();
    m_LeftLayout.AddButton( m_MoveDownButton, "@2>" );
    m_LeftLayout.AddYGap();
    m_LeftLayout.AddButton( m_MoveBotButton, "@2>>" );

    m_GeomBrowser = m_MidLayout.AddFlBrowser( m_MidLayout.GetH() - 5 );
    m_GeomBrowser->callback( staticScreenCB, this );
    m_GeomBrowser->type( FL_MULTI_BROWSER );


    m_RightLayout.AddButton( m_DeleteButton, "Delete" );

    m_RightLayout.AddYGap();
    m_RightLayout.AddDividerBox( "Clipboard" );
    m_RightLayout.AddButton( m_CopyButton, "Copy" );
    m_RightLayout.AddButton( m_PasteButton, "Paste" );
    m_RightLayout.AddButton( m_CutButton, "Cut" );

    m_CopyButton.GetFlButton()->shortcut( FL_CTRL + 'c' );
    m_PasteButton.GetFlButton()->shortcut( FL_CTRL + 'v' );
    m_CutButton.GetFlButton()->shortcut( FL_CTRL + 'x' );

    m_RightLayout.AddYGap();
    m_RightLayout.AddDividerBox( "Selection" );
    m_RightLayout.AddButton( m_SelectAllButton, "Sel All" );
    m_RightLayout.AddButton( m_PickButton, "Pick" );
    m_RightLayout.AddButton( m_ShowOnlyButton, "Show Only" );
    m_RightLayout.AddButton( m_ShowButton, "Show" );
    m_RightLayout.AddButton( m_NoShowButton, "NoShow" );

    m_RightLayout.AddYGap();
    m_RightLayout.AddDividerBox( "Surface" );

    m_RightLayout.SetChoiceButtonWidth( 0 );
    m_RightLayout.AddChoice( m_DisplayChoice, "" );

    m_RightLayout.AddButton( m_WireGeomButton, "Wire" );
    m_RightLayout.AddButton( m_HiddenGeomButton, "Hidden" );
    m_RightLayout.AddButton( m_ShadeGeomButton, "Shade" );
    m_RightLayout.AddButton( m_TextureGeomButton, "Texture" );
    m_RightLayout.AddButton( m_NoneGeomButton, "None" );

    m_RightLayout.AddYGap();
    m_RightLayout.AddDividerBox( "Lines" );
    m_RightLayout.AddButton( m_ShowSubToggle, "Sub" );
    m_RightLayout.AddButton( m_ShowFeatureToggle, "Feature" );

    m_RightLayout.AddYGap();
    m_RightLayout.AddDividerBox( "Sets" );
    m_RightLayout.SetChoiceButtonWidth( 0 );
    m_RightLayout.AddChoice( m_SetChoice, "" );
    m_RightLayout.AddButton( m_ShowOnlySetButton, "Show Only" );
    m_RightLayout.AddButton( m_ShowSetButton, "Show" );
    m_RightLayout.AddButton( m_NoShowSetButton, "NoShow" );
    m_RightLayout.AddButton( m_SelectSetButton, "Select" );

    m_RightLayout.AddResizeBox();

    CreateScreens();
}

//==== Destructor ====//
ManageGeomScreen::~ManageGeomScreen()
{
    for ( int i = 0 ; i < ( int )m_GeomScreenVec.size() ; i++ )
    {
        delete m_GeomScreenVec[i];
    }
}

//==== Update Screen ====//
bool ManageGeomScreen::Update()
{
    if ( IsShown() )
    {
        LoadBrowser();
        LoadActiveGeomOutput();
        LoadSetChoice();
        LoadTypeChoice();
        LoadDisplayChoice();
        UpdateDrawType();
    }

    UpdateGeomScreens();

    return true;
}

//==== Update All Geom Screens ====//
void ManageGeomScreen::UpdateGeomScreens()
{
    for ( int i = 0 ; i < ( int )m_GeomScreenVec.size() ; i++ )
    {
        if ( m_GeomScreenVec[i]->IsShown() )
        {
            m_GeomScreenVec[i]->Update();
        }
    }
}

//==== Show Screen ====//
void ManageGeomScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}


//==== Hide Screen ====//
void ManageGeomScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Load Geom Browser ====//
void ManageGeomScreen::LoadBrowser()
{
    //==== Save List of Selected Geoms ====//
    vector< string > activeVec = m_VehiclePtr->GetActiveGeomVec();

    m_LastTopLine = m_GeomBrowser->topline();

    //==== Display Vehicle Name ====//
    m_GeomBrowser->clear();
    m_GeomBrowser->add( m_VehiclePtr->GetName().c_str() );

    //==== Get Geoms To Display ====//
    m_DisplayedGeomVec = m_VehiclePtr->GetGeomVec( true );

    //==== Step Thru Comps ====//
    for ( int i = 0 ; i < ( int )m_DisplayedGeomVec.size() ; i++ )
    {
        Geom* gPtr = m_VehiclePtr->FindGeom( m_DisplayedGeomVec[i] );
        if ( gPtr )
        {
            string str;
            //==== Check if Parent is Selected ====//
            if ( IsParentSelected( m_DisplayedGeomVec[i], activeVec ) )
            {
                str.append( "@b@." );
            }

            int numindents = gPtr->CountParents( 0 );
            for ( int j = 0 ; j < numindents ; j++ )
            {
                str.append( "--" );
            }

            if ( gPtr->IsParentJoint() )
            {
                str.append( "^^ " );
            }
            else if ( gPtr->m_TransAttachFlag() == vsp::ATTACH_TRANS_NONE &&
                      gPtr->m_RotAttachFlag() == vsp::ATTACH_ROT_NONE )
            {
                str.append( "> " );
            }
            else
            {
                str.append( "^ " );
            }

            if ( !gPtr->m_GuiDraw.GetDisplayChildrenFlag() )
            {
                str.append( "(+) " );
            }

            str.append( gPtr->GetName() );

            if ( gPtr->GetSetFlag( vsp::SET_NOT_SHOWN ) )
            {
                str.append( "(no show)" );
            }

            m_GeomBrowser->add( str.c_str() );
        }
    }

    //==== Restore List of Selected Geoms ====//
    for ( int i = 0 ; i < ( int )activeVec.size() ; i++ )
    {
        SelectGeomBrowser( activeVec[i] );
    }
}

void ManageGeomScreen::SelectGeomBrowser( string geom_id )
{
    //==== Select If Match ====//
    for ( int i = 0 ; i < ( int )m_DisplayedGeomVec.size() ; i++ )
    {
        if ( m_DisplayedGeomVec[i] == geom_id )
        {
            m_GeomBrowser->select( i + 2 );
        }
    }

    //==== Position Browser ====//
    for ( int i = 0 ; i < ( int )m_DisplayedGeomVec.size() ; i++ )
    {
        if ( m_DisplayedGeomVec[i] == geom_id )
        {
            m_GeomBrowser->topline( i + 2 );
            break;
        }
    }

    if ( !m_CollapseFlag && m_LastTopLine < ( ( int )m_DisplayedGeomVec.size() - 2 ) )
    {
        m_GeomBrowser->topline( m_LastTopLine );
    }

}

//==== Is Parent (or Higher) Selected ====//
bool ManageGeomScreen::IsParentSelected( string geom_id, vector< string > & selVec )
{
    Geom* checkGeom = m_VehiclePtr->FindGeom( geom_id );
    while ( checkGeom )
    {
        if ( vector_contains_val( selVec, checkGeom->GetID() ) )
        {
            return true;
        }

        string parent_id = checkGeom->GetParentID();
        checkGeom = m_VehiclePtr->FindGeom( parent_id );
    }
    return false;
}

vector< string > ManageGeomScreen::GetSelectedBrowserItems()
{
    vector< string> selVec;

    if ( m_DisplayedGeomVec.size() == 0 )
    {
        return selVec;
    }

    //==== Account For Browser Lines Starting at 1 and Aircraft Name ====//
    for ( int i = 2 ; i <= m_GeomBrowser->size() ; i++ )
    {
        if ( m_GeomBrowser->selected( i ) && ( int )m_DisplayedGeomVec.size() > ( i - 2 ) )
        {
            selVec.push_back( m_DisplayedGeomVec[i - 2] );
        }
    }
    return selVec;
}

//==== Load Active Geom Output ====//
void ManageGeomScreen::LoadActiveGeomOutput()
{
    vector< string > activeVec = m_VehiclePtr->GetActiveGeomVec();
    if ( activeVec.size() == 0 )
    {
        m_ActiveGeomInput.Update( m_VehiclePtr->GetName().c_str() );
    }
    else if ( activeVec.size() == 1 )
    {
        Geom* gptr = m_VehiclePtr->FindGeom( activeVec[0] );
        if ( gptr )
        {
            m_ActiveGeomInput.Update( gptr->GetName().c_str() );
        }
    }
    else
    {
        m_ActiveGeomInput.Update( "<multiple>" );
    }
}

//==== Load Type Choice ====//
void ManageGeomScreen::LoadSetChoice()
{
    m_SetChoice.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector<string> set_name_vec = veh->GetSetNameVec();

    for ( int i = SET_FIRST_USER; i < ( int )set_name_vec.size(); i++ )
    {
        m_SetChoice.AddItem( set_name_vec[i].c_str() );
    }

    m_SetChoice.UpdateItems();
    m_SetChoice.SetVal( m_SetIndex );
}

//==== Load Type Choice ====//
void ManageGeomScreen::LoadTypeChoice()
{
    m_GeomTypeChoice.ClearItems();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    int num_type  = veh->GetNumGeomTypes();
    int num_fixed = veh->GetNumFixedGeomTypes();
    int cnt = 1;

    for ( int i = 0 ; i < num_type ; i++ )
    {
        GeomType type = veh->GetGeomType( i );

        string item = type.m_Name.c_str();
        if ( i == ( num_fixed - 1 ) )
        {
            item = "_" + item;
        }

        if ( !type.m_FixedFlag )
        {
            item = StringUtil::int_to_string( cnt, "%d.  " ) + item;
            cnt++;
        }
        m_GeomTypeChoice.AddItem( item.c_str() );
    }

    m_GeomTypeChoice.UpdateItems();
    m_GeomTypeChoice.SetVal( m_TypeIndex );
}

//==== Add Geom - Type From Type Choice ====//
void ManageGeomScreen::AddGeom()
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    GeomType type = veh->GetGeomType( m_TypeIndex );

    if ( type.m_Type < NUM_GEOM_TYPE )
    {
        string added_id = m_VehiclePtr->AddGeom( type );
        m_VehiclePtr->SetActiveGeom( added_id );
        ShowHideGeomScreens();
    }
}

//==== Load Display Choice ====//
void ManageGeomScreen::LoadDisplayChoice()
{
    m_DisplayChoice.ClearItems();

    vector<string> geom_id_vec = GetActiveGeoms();
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );

    m_DisplayChoice.AddItem( "Normal" );
    m_DisplayChoice.AddItem( "Surface Degen" );
    m_DisplayChoice.AddItem( "Plate Degen" );
    m_DisplayChoice.AddItem( "Camber Degen" );

    bool mult_display_types = false;

    // Check for multiple display types
    for ( unsigned int i = 0; i < geom_vec.size(); i++ )
    {
        if ( geom_vec[i]->m_GuiDraw.GetDisplayType() != geom_vec[0]->m_GuiDraw.GetDisplayType() )
        {
            m_DisplayChoice.AddItem( "Multiple" );
            mult_display_types = true;
            break;
        }
    }

    m_DisplayChoice.UpdateItems();

    if ( !mult_display_types && geom_vec.size() >= 1 )
    {
        m_DisplayChoice.SetVal( geom_vec[0]->m_GuiDraw.GetDisplayType() );
    }
    else if ( mult_display_types )
    {
        m_DisplayChoice.SetVal( 4 ); // Set to "Multiple"
    }
    else
    {
        m_DisplayChoice.SetVal( 0 ); // Default to "Surface"
    }
}

//==== Item in Geom Browser Was Selected ====//
void ManageGeomScreen::GeomBrowserCallback()
{
    //==== Find Vector of All Selections ====//
    vector< string > selVec = GetSelectedBrowserItems();

    //==== Find Last Selected Geom ====//
    int last = m_GeomBrowser->value();
    if ( ( last >= 2 ) && Fl::event_state( FL_ALT ) )   // Select Children
    {
        Geom* lastSelGeom = m_VehiclePtr->FindGeom( m_DisplayedGeomVec[last - 2] );
        if ( lastSelGeom )
        {
            vector<string> cVec;
            lastSelGeom->LoadIDAndChildren( cVec );
            for ( int i = 1 ; i < ( int )cVec.size(); i++ )
            {
                SelectGeomBrowser( cVec[i] );
                selVec.push_back( cVec[i] );
            }
        }
    }

    //==== Check if Geom Already Selected ====//
    m_CollapseFlag = false;
    if ( m_LastSelectedGeomID != "NONE" && selVec.size() == 1 )
    {
        string lastSel = selVec[0];
        if ( lastSel == m_LastSelectedGeomID  )
        {
            m_CollapseFlag = true;
            Geom* lastSelGeom = m_VehiclePtr->FindGeom( m_LastSelectedGeomID );
            if ( lastSelGeom )
            {
                lastSelGeom->m_GuiDraw.ToggleDisplayChildrenFlag();
                if ( lastSelGeom->GetChildIDVec().size() == 0 )     // No Children Dont Collapse
                {
                    lastSelGeom->m_GuiDraw.SetDisplayChildrenFlag( true );
                }
            }
        }
    }
    m_LastSelectedGeomID = "NONE";
    if ( selVec.size() == 1 )
    {
        m_LastSelectedGeomID = selVec[0];
    }

    m_VehiclePtr->SetActiveGeomVec( selVec );
    LoadActiveGeomOutput();

    ShowHideGeomScreens();
}

//==== Show/NoShow Active Geoms and Children ====//
void ManageGeomScreen::NoShowActiveGeoms( bool flag )
{

    //==== Load Active Geom IDs And Children ====//
    vector<string> geom_id_vec;
    vector<string> active_geom_vec = m_VehiclePtr->GetActiveGeomVec();
    for ( int i = 0 ; i < ( int )active_geom_vec.size() ; i++ )
    {
        Geom* gPtr = m_VehiclePtr->FindGeom( active_geom_vec[i] );
        if ( gPtr )
        {
            gPtr->LoadIDAndChildren( geom_id_vec );
        }
    }

    //==== Set No Show Flag ====//
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i] )
        {
            geom_vec[i]->SetSetFlag( SET_SHOWN , !flag );
            geom_vec[i]->SetSetFlag( SET_NOT_SHOWN , flag );
        }
    }

    LoadBrowser();
}

//===== Select All Geoms ====//
void ManageGeomScreen::SelectAll()
{
    vector< string > all_geom_vec = m_VehiclePtr->GetGeomVec();
    m_VehiclePtr->SetActiveGeomVec( all_geom_vec );

    //==== Restore List of Selected Geoms ====//
    for ( int i = 0 ; i < ( int )all_geom_vec.size() ; i++ )
    {
        SelectGeomBrowser( all_geom_vec[i] );
    }

    LoadActiveGeomOutput();

    ShowHideGeomScreens();
}

//===== Select Geom Set ====//
void ManageGeomScreen::SelectSet( int set )
{
    vector< string > set_geom_vec = m_VehiclePtr->GetGeomSet( set );
    m_VehiclePtr->SetActiveGeomVec( set_geom_vec );

    //==== Restore List of Selected Geoms ====//
    for ( int i = 0 ; i < ( int )set_geom_vec.size() ; i++ )
    {
        SelectGeomBrowser( set_geom_vec[i] );
    }

    LoadActiveGeomOutput();
    ShowHideGeomScreens();
}

//==== Load Active Geom IDs and Children ===//
vector< string > ManageGeomScreen::GetActiveGeoms()
{
    //==== Load Active Geom IDs And Children ====//
    vector<string> geom_id_vec;
    vector<string> active_geom_vec = m_VehiclePtr->GetActiveGeomVec();
    for ( int i = 0 ; i < ( int )active_geom_vec.size() ; i++ )
    {
        Geom* gPtr = m_VehiclePtr->FindGeom( active_geom_vec[i] );
        if ( gPtr )
        {
            if ( gPtr->m_GuiDraw.GetDisplayChildrenFlag() )
            {
                geom_id_vec.push_back( active_geom_vec[i] );
            }
            else
            {
                gPtr->LoadIDAndChildren( geom_id_vec );
            }
        }
    }

    return geom_id_vec;
}

void ManageGeomScreen::SetGeomDisplayChoice( int type )
{
    vector<string> geom_id_vec = GetActiveGeoms();
    //==== Set Display Type ====//
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );
    for ( int i = 0; i < (int)geom_vec.size(); i++ )
    {
        if ( geom_vec[i] && type <= vsp::DISPLAY_TYPE::DISPLAY_DEGEN_CAMBER )
        {
            geom_vec[i]->m_GuiDraw.SetDisplayType( type );
            geom_vec[i]->ResetGeomChangedFlag( true ); // Redraw the wire mesh
        }
    }

    m_VehiclePtr->Update();
}

void ManageGeomScreen::SetGeomDisplayType( int type )
{
    vector<string> geom_id_vec = GetActiveGeoms();
    //==== Set Display Type ====//
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i] )
        {
            geom_vec[i]->m_GuiDraw.SetDrawType( type );
        }
    }

//jrg FIX!!!
//  aircraftPtr->triggerDraw();
}

void ManageGeomScreen::EditName( string name )
{
    vector<string> active_geom_vec = m_VehiclePtr->GetActiveGeomVec();

    //==== Dont Change Multiple Names ====//
    if ( active_geom_vec.size() > 1 )
    {
        return;
    }

    if ( active_geom_vec.size() == 0 )
    {
        m_VehiclePtr->SetName( name );
    }
    else
    {
        Geom* g_ptr = m_VehiclePtr->FindGeom( active_geom_vec[0] );
        if ( g_ptr )
        {
            g_ptr->SetName( name );
        }
    }
//jrg FIX!!!
//  Trigger Edit Screen Update...
}


//void GeomScreen::s_select_none(int src) {
//  aircraftPtr->setActiveGeom(NULL);
//
//  if (src != ScriptMgr::SCRIPT)
//  {
//      deselectGeomBrowser();          // select none
//      screenMgrPtr->hideGeomScreens();
//      screenMgrPtr->getGroupScreen()->hide();
//      loadActiveGeomOutput();
//      aircraftPtr->flagActiveGeom();
//      aircraftPtr->triggerDraw();
//      geomUI->geomBrowser->value(1);
//  }
//  if (src == ScriptMgr::GUI) scriptMgr->addLine("select none");
//}

//==== Create Screens ====//
void ManageGeomScreen::CreateScreens()
{
    m_GeomScreenVec.resize( NUM_GEOM_SCREENS );
    m_GeomScreenVec[POD_GEOM_SCREEN] = new PodScreen( m_ScreenMgr );
    m_GeomScreenVec[FUSELAGE_GEOM_SCREEN] = new FuselageScreen( m_ScreenMgr );
    m_GeomScreenVec[MS_WING_GEOM_SCREEN] = new WingScreen( m_ScreenMgr );
    m_GeomScreenVec[BLANK_GEOM_SCREEN] = new BlankScreen( m_ScreenMgr );
    m_GeomScreenVec[MESH_GEOM_SCREEN] = new MeshScreen( m_ScreenMgr );
    m_GeomScreenVec[STACK_GEOM_SCREEN] = new StackScreen( m_ScreenMgr );
    m_GeomScreenVec[CUSTOM_GEOM_SCREEN] = new CustomScreen( m_ScreenMgr );
    m_GeomScreenVec[PT_CLOUD_GEOM_SCREEN] = new PtCloudScreen( m_ScreenMgr );
    m_GeomScreenVec[PROP_GEOM_SCREEN] = new PropScreen( m_ScreenMgr );
    m_GeomScreenVec[HINGE_GEOM_SCREEN] = new HingeScreen( m_ScreenMgr );
    m_GeomScreenVec[MULT_GEOM_SCREEN] = new MultTransScreen( m_ScreenMgr );
    m_GeomScreenVec[CONFORMAL_SCREEN] = new ConformalScreen( m_ScreenMgr );
    m_GeomScreenVec[ELLIPSOID_GEOM_SCREEN] = new EllipsoidScreen( m_ScreenMgr );
    m_GeomScreenVec[BOR_GEOM_SCREEN] = new BORScreen( m_ScreenMgr );
    m_GeomScreenVec[HUMAN_GEOM_SCREEN] = new HumanGeomScreen( m_ScreenMgr );
    m_GeomScreenVec[WIRE_FRAME_GEOM_SCREEN] = new WireScreen( m_ScreenMgr );

    for ( int i = 0 ; i < ( int )m_GeomScreenVec.size() ; i++ )
    {
        m_GeomScreenVec[i]->GetFlWindow()->set_non_modal();
    }
}

//==== Show Hide Geom Screen Depending on Active Geoms ====//
void ManageGeomScreen::ShowHideGeomScreens()
{
    //==== Hide All Geom Screens =====//
    for ( int i = 0 ; i < ( int )m_GeomScreenVec.size() ; i++ )
    {
        m_GeomScreenVec[i]->Hide();
    }
    //==== Show Screen - Each Screen Will Test Check Valid Active Geom Type ====//
    for ( int i = 0 ; i < ( int )m_GeomScreenVec.size() ; i++ )
    {
        m_GeomScreenVec[i]->Show();
    }

    m_GeomScreenVec[MULT_GEOM_SCREEN]->Show();
}

//==== Show or Hide Subsurface Lines ====//
void ManageGeomScreen::SetSubDrawFlag( bool f )
{
    vector<string> geom_id_vec = GetActiveGeoms();
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i] )
        {
            geom_vec[i]->m_GuiDraw.SetDispSubSurfFlag( f );
        }
    }
}

//==== Show or Hide Feature Lines ====//
void ManageGeomScreen::SetFeatureDrawFlag( bool f )
{
    vector<string> geom_id_vec = GetActiveGeoms();
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );

    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i] )
        {
            geom_vec[i]->m_GuiDraw.SetDispFeatureFlag( f );
        }
    }

    m_VehiclePtr->Update();
}

//==== Callbacks ====//
void ManageGeomScreen::CallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    if ( w == m_GeomBrowser )
    {
        GeomBrowserCallback();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageGeomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    if ( device == &m_GeomTypeChoice )
    {
        m_TypeIndex = m_GeomTypeChoice.GetVal();
    }
    else if ( device == &m_AddGeomButton)
    {
        AddGeom();
    }
    else if ( device == &m_NoShowButton )
    {
        NoShowActiveGeoms( true );
    }
    else if ( device == &m_ShowButton )
    {
        NoShowActiveGeoms( false );
    }
    else if ( device == &m_ShowOnlyButton )
    {
        m_VehiclePtr->NoShowSet( SET_ALL );
        NoShowActiveGeoms( false );
    }
    else if ( device == &m_SelectAllButton )
    {
        SelectAll();
    }
    else if ( device == &m_ActiveGeomInput )      // Geom or Aircraft Name
    {
        EditName( m_ActiveGeomInput.GetString() );
    }
    else if ( device == &m_CutButton )
    {
        m_VehiclePtr->CutActiveGeomVec();
    }
    else if ( device == &m_DeleteButton )
    {
        m_VehiclePtr->DeleteActiveGeomVec();
    }
    else if ( device == &m_CopyButton )
    {
        m_VehiclePtr->CopyActiveGeomVec();
    }
    else if ( device == &m_PasteButton )
    {
        m_VehiclePtr->PasteClipboard();
    }
    else if ( device == &m_DisplayChoice )
    {
        SetGeomDisplayChoice( m_DisplayChoice.GetVal() );
    }
    else if ( device == &m_WireGeomButton )
    {
        SetGeomDisplayType( DRAW_TYPE::GEOM_DRAW_WIRE );
    }
    else if ( device == &m_HiddenGeomButton )
    {
        SetGeomDisplayType( DRAW_TYPE::GEOM_DRAW_HIDDEN );
    }
    else if ( device == &m_ShadeGeomButton )
    {
        SetGeomDisplayType( DRAW_TYPE::GEOM_DRAW_SHADE );
    }
    else if ( device == &m_TextureGeomButton )
    {
        SetGeomDisplayType( DRAW_TYPE::GEOM_DRAW_TEXTURE );
    }
    else if ( device == &m_NoneGeomButton )
    {
        SetGeomDisplayType( DRAW_TYPE::GEOM_DRAW_NONE );
    }
    else if ( device == &m_MoveUpButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_UP );
    }
    else if ( device == &m_MoveDownButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_DOWN );
    }
    else if ( device == &m_MoveTopButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_TOP );
    }
    else if ( device == &m_MoveBotButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_BOTTOM );
    }
    else if ( device == &m_SetChoice )
    {
        m_SetIndex = m_SetChoice.GetVal();
    }
    else if ( device == &m_ShowOnlySetButton )
    {
        m_VehiclePtr->ShowOnlySet( m_SetIndex + SET_FIRST_USER );
    }
    else if ( device == &m_ShowSetButton )
    {
        m_VehiclePtr->ShowSet( m_SetIndex + SET_FIRST_USER );
    }
    else if ( device == &m_NoShowSetButton )
    {
        m_VehiclePtr->NoShowSet( m_SetIndex + SET_FIRST_USER );
    }
    else if ( device == &m_SelectSetButton )
    {
        SelectSet( m_SetIndex + SET_FIRST_USER );
    }
    else if ( device == &m_ShowSubToggle )
    {
        if ( m_ShowSubToggle.GetFlButton()->value() )
            SetSubDrawFlag( true );
        else
            SetSubDrawFlag( false );
    }
    else if ( device == &m_ShowFeatureToggle )
    {
        if ( m_ShowFeatureToggle.GetFlButton()->value() )
            SetFeatureDrawFlag( true );
        else
            SetFeatureDrawFlag( false );
    }
//    else if ( w == m_GeomUI->pickGeomButton )
//    {
//    }

    m_ScreenMgr->SetUpdateFlag( true );
}

std::string ManageGeomScreen::getFeedbackGroupName()
{
    return std::string("GeomGUIGroup");
}

void ManageGeomScreen::Set( std::string geomId )
{
    printf("%s\n", geomId.c_str());
    m_VehiclePtr->SetActiveGeom(geomId);

    ShowHideGeomScreens();
    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageGeomScreen::TriggerPickSwitch()
{
    m_PickButton.GetFlButton()->value( !m_PickButton.GetFlButton()->value() );
    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageGeomScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    UpdateDrawObjs();

    for( int i = 0; i < ( int )m_PickList.size(); i++ )
    {
        draw_obj_vec.push_back( &m_PickList[i] );
    }
}

void ManageGeomScreen::UpdateDrawObjs()
{
    m_PickList.clear();

    if( m_PickButton.GetFlButton()->value() == 1 )
    {
        vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( m_VehiclePtr->GetGeomVec() );
        for( int i = 0; i < ( int )geom_vec.size(); i++ )
        {
            std::vector< DrawObj* > geom_drawobj_vec;
            geom_vec[i]->LoadDrawObjs( geom_drawobj_vec );

            for( int j = 0; j < ( int )geom_drawobj_vec.size(); j++ )
            {
                if( geom_drawobj_vec[j]->m_Visible )
                {
                    // Ignore bounding boxes.
                    if( geom_drawobj_vec[j]->m_GeomID.compare(0, string(BBOXHEADER).size(), BBOXHEADER) != 0 )
                    {
                        DrawObj pickObj;
                        pickObj.m_Type = DrawObj::VSP_PICK_GEOM;
                        pickObj.m_GeomID = PICKGEOMHEADER + geom_drawobj_vec[j]->m_GeomID;
                        pickObj.m_PickSourceID = geom_drawobj_vec[j]->m_GeomID;
                        pickObj.m_FeedbackGroup = getFeedbackGroupName();

                        m_PickList.push_back( pickObj );
                    }
                }
            }
        }
    }
}

void ManageGeomScreen::UpdateDrawType()
{
    vector<string> geom_id_vec = GetActiveGeoms();
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );
    int num_geoms = (int)geom_vec.size();

    // Handle case where there are not any geoms selected.
    if ( num_geoms == 0 )
    {
        m_ShowSubToggle.GetFlButton()->value( 0 );
    }

    int num_sub_on = 0;
    int num_feature_on = 0;

    for ( int i = 0; i < (int)geom_vec.size(); i++ )
    {
        if ( geom_vec[i] && geom_vec[i]->m_GuiDraw.GetDispSubSurfFlag() )
        {
            num_sub_on++;
        }

        if ( geom_vec[i] && geom_vec[i]->m_GuiDraw.GetDispFeatureFlag() )
        {
            num_feature_on++;
        }
    }

    double flag_average = num_sub_on/(double)num_geoms;
    if ( flag_average > 0.5 )
    {
        m_ShowSubToggle.GetFlButton()->value( 1 );
    }
    else
    {
        m_ShowSubToggle.GetFlButton()->value( 0 );
    }

    flag_average = num_feature_on / (double)num_geoms;
    if ( flag_average > 0.5 )
    {
        m_ShowFeatureToggle.GetFlButton()->value( 1 );
    }
    else
    {
        m_ShowFeatureToggle.GetFlButton()->value( 0 );
    }
}
