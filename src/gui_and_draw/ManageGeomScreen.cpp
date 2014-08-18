//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageGeomScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include "StringUtil.h"
#include "APIDefines.h"
using namespace vsp;


#include <assert.h>


//==== Constructor ====//
ManageGeomScreen::ManageGeomScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    m_VehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_LastTopLine = 0;
    m_SetIndex = 0;
    m_TypeIndex = 0;
    m_CollapseFlag = false;
    m_LastSelectedGeomID = "NONE";

    GeomUI* ui = m_GeomUI = new GeomUI();

    m_FLTK_Window = ui->UIWindow;

    ui->geomTypeChoice->callback( staticScreenCB, this );
    ui->addGeomButton->callback( staticScreenCB, this );
    ui->geomBrowser->callback( staticScreenCB, this );
    ui->cutGeomButton->callback( staticScreenCB, this );
    ui->copyGeomButton->callback( staticScreenCB, this );
    ui->pasteGeomButton->callback( staticScreenCB, this );
    ui->noshowGeomButton->callback( staticScreenCB, this );
    ui->showGeomButton->callback( staticScreenCB, this );
    ui->selectAllGeomButton->callback( staticScreenCB, this );
    ui->activeGeomInput->callback( staticScreenCB, this );
    ui->setChoice->callback( staticScreenCB, this );
    ui->showSetButton->callback( staticScreenCB, this );

    ui->wireGeomButton->callback( staticScreenCB, this );
    ui->shadeGeomButton->callback( staticScreenCB, this );
    ui->hiddenGeomButton->callback( staticScreenCB, this );
    ui->textureGeomButton->callback( staticScreenCB, this );
    ui->showFeatureToggle->callback( staticScreenCB, this );
    ui->showSubToggle->callback( staticScreenCB, this );

    ui->moveUpButton->callback( staticScreenCB, this );
    ui->moveDownButton->callback( staticScreenCB, this );
    ui->moveTopButton->callback( staticScreenCB, this );
    ui->moveBotButton->callback( staticScreenCB, this );

    ui->pickGeomButton->callback( staticScreenCB, this );

    ui->cutGeomButton->shortcut( FL_CTRL + 'x' );
    ui->copyGeomButton->shortcut( FL_CTRL + 'c' );
    ui->pasteGeomButton->shortcut( FL_CTRL + 'v' );
    
    CreateScreens();
}

//==== Destructor ====//
ManageGeomScreen::~ManageGeomScreen()
{
    delete m_GeomUI;
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

    m_LastTopLine = m_GeomUI->geomBrowser->topline();

    //==== Display Vehicle Name ====//
    m_GeomUI->geomBrowser->clear();
    m_GeomUI->geomBrowser->add( m_VehiclePtr->GetName().c_str() );

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

            if ( gPtr->m_TransAttachFlag() == GeomXForm::ATTACH_TRANS_NONE &&
                    gPtr->m_RotAttachFlag() == GeomXForm::ATTACH_ROT_NONE )
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

            if ( gPtr->m_GuiDraw.GetNoShowFlag() )
            {
                str.append( "(no show)" );
            }

            m_GeomUI->geomBrowser->add( str.c_str() );
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
            m_GeomUI->geomBrowser->select( i + 2 );
        }
    }

    //==== Position Browser ====//
    for ( int i = 0 ; i < ( int )m_DisplayedGeomVec.size() ; i++ )
    {
        if ( m_DisplayedGeomVec[i] == geom_id )
        {
            m_GeomUI->geomBrowser->topline( i + 2 );
            break;
        }
    }

    if ( !m_CollapseFlag && m_LastTopLine < ( ( int )m_DisplayedGeomVec.size() - 2 ) )
    {
        m_GeomUI->geomBrowser->topline( m_LastTopLine );
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
    for ( int i = 2 ; i <= m_GeomUI->geomBrowser->size() ; i++ )
    {
        if ( m_GeomUI->geomBrowser->selected( i ) && ( int )m_DisplayedGeomVec.size() > ( i - 2 ) )
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
        m_GeomUI->activeGeomInput->value( m_VehiclePtr->GetName().c_str() );
    }
    else if ( activeVec.size() == 1 )
    {
        Geom* gptr = m_VehiclePtr->FindGeom( activeVec[0] );
        if ( gptr )
        {
            m_GeomUI->activeGeomInput->value( gptr->GetName().c_str() );
        }
    }
    else
    {
        m_GeomUI->activeGeomInput->value( "<multiple>" );
    }
}

//==== Load Type Choice ====//
void ManageGeomScreen::LoadSetChoice()
{
    m_GeomUI->setChoice->clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = SET_FIRST_USER ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_GeomUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_GeomUI->setChoice->value( m_SetIndex );

}


//==== Load Type Choice ====//
void ManageGeomScreen::LoadTypeChoice()
{
    m_GeomUI->geomTypeChoice->clear();

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
        m_GeomUI->geomTypeChoice->add( item.c_str() );

    }

    m_GeomUI->geomTypeChoice->value( m_TypeIndex );
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

//==== Item in Geom Browser Was Selected ====//
void ManageGeomScreen::GeomBrowserCallback()
{
    //==== Find Vector of All Selections ====//
    vector< string > selVec = GetSelectedBrowserItems();

    //==== Find Last Selected Geom ====//
    int last = m_GeomUI->geomBrowser->value();
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

//  m_ScreenMgr->UpdateAllScreens();
    ShowHideGeomScreens();

//jrg FIX!!!
//  aircraftPtr->triggerDraw();

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
            gPtr->m_GuiDraw.SetNoShowFlag( flag );
        }
    }

    //==== Set No Show Flag ====//
    vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        if ( geom_vec[i] )
        {
            geom_vec[i]->m_GuiDraw.SetNoShowFlag( flag );
        }
    }

//jrg FIX!!!
//  aircraftPtr->triggerDraw();
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

//jrg FIX!!!
//  aircraftPtr->triggerDraw();

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
}

//==== Callbacks ====//
void ManageGeomScreen::CallBack( Fl_Widget *w )
{
    if ( w == m_GeomUI->geomTypeChoice )
    {
        m_TypeIndex = m_GeomUI->geomTypeChoice->value();
    }
    else if ( w == m_GeomUI->addGeomButton )
    {
        AddGeom();
    }
    else if ( w == m_GeomUI->geomBrowser )
    {
        GeomBrowserCallback();
    }
    else if ( w == m_GeomUI->noshowGeomButton )
    {
        NoShowActiveGeoms( true );
    }
    else if ( w == m_GeomUI->showGeomButton )
    {
        NoShowActiveGeoms( false );
    }
    else if ( w == m_GeomUI->selectAllGeomButton )
    {
        SelectAll();
    }
    else if ( w == m_GeomUI->activeGeomInput )      // Geom or Aircraft Name
    {
        EditName( m_GeomUI->activeGeomInput->value() );
    }
    else if ( w == m_GeomUI->cutGeomButton )
    {
        m_VehiclePtr->CutActiveGeomVec();
    }
    else if ( w == m_GeomUI->copyGeomButton )
    {
        m_VehiclePtr->CopyActiveGeomVec();
    }
    else if ( w == m_GeomUI->pasteGeomButton )
    {
        m_VehiclePtr->PasteClipboard();
    }
    else if ( w == m_GeomUI->wireGeomButton )
    {
        SetGeomDisplayType( GeomGuiDraw::GEOM_DRAW_WIRE );
    }
    else if ( w == m_GeomUI->hiddenGeomButton )
    {
        SetGeomDisplayType( GeomGuiDraw::GEOM_DRAW_HIDDEN );
    }
    else if ( w == m_GeomUI->shadeGeomButton )
    {
        SetGeomDisplayType( GeomGuiDraw::GEOM_DRAW_SHADE );
    }
    else if ( w == m_GeomUI->textureGeomButton )
    {
        SetGeomDisplayType( GeomGuiDraw::GEOM_DRAW_TEXTURE );
    }
    else if ( w == m_GeomUI->moveUpButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_UP );
    }
    else if ( w == m_GeomUI->moveDownButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_DOWN );
    }
    else if ( w == m_GeomUI->moveTopButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_TOP );
    }
    else if ( w == m_GeomUI->moveBotButton )
    {
        m_VehiclePtr->ReorderActiveGeom( Vehicle::REORDER_MOVE_BOTTOM );
    }
    else if ( w == m_GeomUI->setChoice )
    {
        m_SetIndex = m_GeomUI->setChoice->value();
    }
    else if ( w == m_GeomUI->showSetButton  )
    {
        m_VehiclePtr->SetShowSet( m_SetIndex + SET_FIRST_USER );
    }
    else if ( w == m_GeomUI->showSubToggle )
    {
    	if ( m_GeomUI->showSubToggle->value() )
    		SetSubDrawFlag( true );
    	else
    		SetSubDrawFlag( false );
    }
    else if ( w == m_GeomUI->showFeatureToggle )
    {
        if ( m_GeomUI->showFeatureToggle->value() )
            SetFeatureDrawFlag( true );
        else
            SetFeatureDrawFlag( false );
    }
    else if ( w == m_GeomUI->pickGeomButton )
    {
    }

    m_ScreenMgr->SetUpdateFlag( true );

}

std::string ManageGeomScreen::getFeedbackGroupName()
{
	return std::string("GeomGUIGroup");
}

void ManageGeomScreen::Set( std::string geomId )
{
	m_VehiclePtr->SetActiveGeom(geomId);

	ShowHideGeomScreens();
	m_ScreenMgr->SetUpdateFlag( true );
}

void ManageGeomScreen::TriggerPickSwitch()
{
    m_GeomUI->pickGeomButton->value(!m_GeomUI->pickGeomButton->value());
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

    if( m_GeomUI->pickGeomButton->value() == 1 )
    {
        vector< Geom* > geom_vec = m_VehiclePtr->FindGeomVec( m_VehiclePtr->GetGeomVec( false ) );
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
	if ( num_geoms == 0 ) m_GeomUI->showSubToggle->value(0);

	int num_sub_on = 0;
	int num_feature_on = 0;

	for ( int i = 0; i < (int)geom_vec.size(); i++ )
	{
		if ( geom_vec[i] && geom_vec[i]->m_GuiDraw.GetDispSubSurfFlag() )
			num_sub_on++;

		if ( geom_vec[i] && geom_vec[i]->m_GuiDraw.GetDispFeatureFlag() )
			num_feature_on++;

	}

	double flag_average = num_sub_on/(double)num_geoms;
	if ( flag_average > 0.5 )
		m_GeomUI->showSubToggle->value(1);
	else
		m_GeomUI->showSubToggle->value(0);

	flag_average = num_feature_on/(double)num_geoms;
	if ( flag_average > 0.5 )
		m_GeomUI->showFeatureToggle->value(1);
	else
		m_GeomUI->showFeatureToggle->value(0);
}
