//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "NerfManageGeomScreen.h"
#include "ManageGeomScreen.h"
#include "ScreenMgr.h"
#include "StlHelper.h"

using namespace vsp;


//==== Constructor ====//
NerfManageGeomScreen::NerfManageGeomScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 235-10, 645, "Nerf Geom Browser" )
{
    m_VehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_LastTopLine = 0;
    m_SetIndex = 0;
    m_CollapseFlag = false;
    m_LastSelectedGeomID = "NONE";

    m_MainLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_FLTK_Window->resizable( m_MainLayout.GetGroup() );
    m_FLTK_Window->size_range( m_FLTK_Window->w(), m_FLTK_Window->h() );

    m_MainLayout.AddX( 5 );
    m_MainLayout.AddY( 25 );
    m_MainLayout.AddSubGroupLayout( m_HeadLayout, m_MainLayout.GetRemainX() - 5, 50 );
    m_MainLayout.AddY( 50 );
    m_MainLayout.SetX( 0 );
    m_MainLayout.AddSubGroupLayout( m_BodyLayout, m_MainLayout.GetRemainX() - 5, m_MainLayout.GetRemainY() );
    m_MainLayout.GetGroup()->resizable( m_BodyLayout.GetGroup() );

    m_HeadLayout.SetFitWidthFlag( true );

    m_HeadLayout.SetButtonWidth( 0 );
    m_HeadLayout.AddYGap();
    m_HeadLayout.AddDividerBox( "Active:" );
    m_HeadLayout.AddOutput( m_ActiveGeomOutput, "" );

    m_BodyLayout.SetSameLineFlag( true );
    m_BodyLayout.SetFitWidthFlag( false );
    m_BodyLayout.AddX( 5 );
    m_BodyLayout.AddSubGroupLayout( m_MidLayout, 140, m_BodyLayout.GetRemainY() );
    m_BodyLayout.GetGroup()->resizable( m_MidLayout.GetGroup() );
    m_BodyLayout.AddX( 145 );
    m_BodyLayout.AddSubGroupLayout( m_RightLayout, m_BodyLayout.GetRemainX(), m_BodyLayout.GetRemainY());

    m_GeomBrowser = m_MidLayout.AddFlBrowser( m_MidLayout.GetH() - 5 );
    m_GeomBrowser->callback( staticScreenCB, this );
    m_GeomBrowser->type( FL_MULTI_BROWSER );


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
}

//==== Destructor ====//
NerfManageGeomScreen::~NerfManageGeomScreen()
{
}

//==== Update Screen ====//
bool NerfManageGeomScreen::Update()
{
    BasicScreen::Update();

    if ( IsShown() )
    {
        LoadBrowser();
        LoadActiveGeomOutput();
        LoadSetChoice();
        LoadDisplayChoice();
        UpdateDrawType();
    }

    UpdateGeomScreens();

    return true;
}

//==== Update All Geom Screens ====//
void NerfManageGeomScreen::UpdateGeomScreens()
{
    ( ( ManageGeomScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) )->UpdateGeomScreens();
}

//==== Show Screen ====//
void NerfManageGeomScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    BasicScreen::Show();
}


//==== Hide Screen ====//
void NerfManageGeomScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Load Geom Browser ====//
void NerfManageGeomScreen::LoadBrowser()
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

void NerfManageGeomScreen::SelectGeomBrowser( const string &geom_id )
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
bool NerfManageGeomScreen::IsParentSelected( const string &geom_id, const vector< string > & selVec )
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

vector< string > NerfManageGeomScreen::GetSelectedBrowserItems()
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
void NerfManageGeomScreen::LoadActiveGeomOutput()
{
    vector< string > activeVec = m_VehiclePtr->GetActiveGeomVec();
    if ( activeVec.size() == 0 )
    {
        m_ActiveGeomOutput.Update( m_VehiclePtr->GetName().c_str() );
    }
    else if ( activeVec.size() == 1 )
    {
        Geom* gptr = m_VehiclePtr->FindGeom( activeVec[0] );
        if ( gptr )
        {
            m_ActiveGeomOutput.Update( gptr->GetName().c_str() );
        }
    }
    else
    {
        m_ActiveGeomOutput.Update( "<multiple>" );
    }
}

//==== Load Type Choice ====//
void NerfManageGeomScreen::LoadSetChoice()
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

//==== Load Display Choice ====//
void NerfManageGeomScreen::LoadDisplayChoice()
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
void NerfManageGeomScreen::GeomBrowserCallback()
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
void NerfManageGeomScreen::NoShowActiveGeoms( bool flag )
{
    vector<string> geom_id_vec = GetActiveGeoms();

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
void NerfManageGeomScreen::SelectAll()
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
void NerfManageGeomScreen::SelectSet( int set )
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
vector< string > NerfManageGeomScreen::GetActiveGeoms()
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

void NerfManageGeomScreen::SetGeomDisplayChoice( int type )
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

void NerfManageGeomScreen::SetGeomDisplayType( int type )
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

void NerfManageGeomScreen::EditName( const string &name )
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


//==== Show Hide Geom Screen Depending on Active Geoms ====//
void NerfManageGeomScreen::ShowHideGeomScreens()
{
    ( ( ManageGeomScreen* ) m_ScreenMgr->GetScreen( vsp::VSP_MANAGE_GEOM_SCREEN ) )->ShowHideGeomScreens();
}

//==== Show or Hide Subsurface Lines ====//
void NerfManageGeomScreen::SetSubDrawFlag( bool f )
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
void NerfManageGeomScreen::SetFeatureDrawFlag( bool f )
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
void NerfManageGeomScreen::CallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    if ( w == m_GeomBrowser )
    {
        GeomBrowserCallback();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void NerfManageGeomScreen::GuiDeviceCallBack( GuiDevice* device )
{
    if ( device == &m_NoShowButton )
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
    else if ( device == &m_ActiveGeomOutput )      // Geom or Aircraft Name
    {
        EditName( m_ActiveGeomOutput.GetString() );
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

std::string NerfManageGeomScreen::getFeedbackGroupName()
{
    return std::string("GeomGUIGroup");
}

void NerfManageGeomScreen::Set( const std::string &geomId )
{
    printf("%s\n", geomId.c_str());
    m_VehiclePtr->SetActiveGeom(geomId);

    ShowHideGeomScreens();
    m_ScreenMgr->SetUpdateFlag( true );
}

void NerfManageGeomScreen::TriggerPickSwitch()
{
    m_PickButton.GetFlButton()->value( !m_PickButton.GetFlButton()->value() );
    m_ScreenMgr->SetUpdateFlag( true );
}

void NerfManageGeomScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    UpdateDrawObjs();

    for( int i = 0; i < ( int )m_PickList.size(); i++ )
    {
        draw_obj_vec.push_back( &m_PickList[i] );
    }
}

void NerfManageGeomScreen::UpdateDrawObjs()
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

void NerfManageGeomScreen::UpdateDrawType()
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

    for ( int i = 0; i < (int)num_geoms; i++ )
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

    if ( num_geoms > 0 )
    {
        double flag_average = num_sub_on / (double)num_geoms;

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
}
