//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ManageGeomScreen.h"
#include "ScreenMgr.h"
#include "StlHelper.h"
#include "HingeGeom.h"

using namespace vsp;


//==== Constructor ====//
ManageGeomScreen::ManageGeomScreen( ScreenMgr* mgr ) : BasicScreen( mgr, 275, 645, "Geom Browser" )
{
    m_FLTK_Window->callback( staticCloseCB, this );

    m_VehiclePtr = m_ScreenMgr->GetVehiclePtr();

    m_SetIndex = SET_FIRST_USER;
    m_TypeIndex = 0;

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
    m_BodyLayout.AddSubGroupLayout( m_MidLayout, 180, m_BodyLayout.GetRemainY() );
    m_BodyLayout.GetGroup()->resizable( m_MidLayout.GetGroup() );
    m_BodyLayout.AddX( 185 );
    m_BodyLayout.AddSubGroupLayout( m_RightLayout, m_BodyLayout.GetRemainX(), m_BodyLayout.GetRemainY());

    m_LeftLayout.SetStdHeight( 50 );
    m_LeftLayout.AddButton( m_MoveTopButton, "@2<<" );
    m_LeftLayout.AddYGap();
    m_LeftLayout.AddButton( m_MoveUpButton, "@2<" );

    m_LeftLayout.AddY( ( m_LeftLayout.GetRemainY() - 100 - 6 - 5 - m_LeftLayout.GetGapHeight() ) / 2 - m_LeftLayout.GetStdHeight() );

    m_LeftLayout.AddButton( m_UpParentButton, "@<" );
    m_LeftLayout.AddYGap();
    m_LeftLayout.AddButton( m_DownParentButton, "@>" );

    m_LeftLayout.AddY( m_LeftLayout.GetRemainY() - 100 - 6 - 5 - m_LeftLayout.GetStdHeight() );
    m_LeftLayout.AddResizeBox();
    m_LeftLayout.AddButton( m_MoveDownButton, "@2>" );
    m_LeftLayout.AddYGap();
    m_LeftLayout.AddButton( m_MoveBotButton, "@2>>" );

    bool resizable = true;
    m_GeomBrowser = m_MidLayout.AddTreeWithIcons( m_MidLayout.GetH() - 5, resizable );
    m_GeomBrowser->showroot( true );
    m_GeomBrowser->callback( staticScreenCB, this );
    m_GeomBrowser->selectmode( FL_TREE_SELECT_MULTI );
    m_GeomBrowser->item_reselect_mode( FL_TREE_SELECTABLE_ALWAYS );
    m_GeomBrowser->sortorder( FL_TREE_SORT_NONE );
    m_GeomBrowser->connectorstyle( static_cast< Fl_Tree_Connector > ( TREE_LINE_CONN::STYLE_SOLID ) );
    m_GeomBrowser->connectorcolor( FL_FOREGROUND_COLOR );
    m_GeomBrowser->labelmarginleft( 0 );
    m_GeomBrowser->marginleft( 0 );
    m_GeomBrowser->connectorwidth( 11 );
    int fontsize = 12;
    m_GeomBrowser->item_labelsize( fontsize );

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

    m_VehSelected = false;
    m_VehOpen = true;
    m_RedrawFlag = true;
    m_NeedsShowHideGeoms = true;
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
    BasicScreen::Update();

    if ( m_NeedsShowHideGeoms )
    {
        m_NeedsShowHideGeoms = false;
        ShowHideGeomScreens();
    }

    if ( IsShown() )
    {
        LoadBrowser();
        LoadActiveGeomOutput();
        m_ScreenMgr->LoadSetChoice( {&m_SetChoice}, vector<int>({m_SetIndex}), false, SET_FIRST_USER );
        LoadTypeChoice();
        LoadDisplayChoice();
        UpdateDrawType();
    }

    UpdateGeomScreens();

    return true;
}

void ManageGeomScreen::GetCollIDs( vector < string > &collIDVec )
{
    vector< string > activeVec = m_VehiclePtr->GetActiveGeomVec();

    for ( int i = 0; i != activeVec.size(); ++i )
    {
        Geom* g = m_VehiclePtr->FindGeom( activeVec.at( i ) );
        if ( g )
        {
            AttributeCollection* ac = g->GetAttrCollection();
            if ( ac )
            {
                collIDVec.push_back( ac->GetID() );
            }
        }
    }
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
    BasicScreen::Show();
}


//==== Hide Screen ====//
void ManageGeomScreen::Hide()
{
    m_FLTK_Window->hide();
}

//==== Load Geom Browser ====//
void ManageGeomScreen::LoadBrowser()
{
    if ( !m_VehiclePtr )
    {
        return;
    }

    //==== Save List of Selected Geoms ====//
    vector< string > activeVec = m_VehiclePtr->GetActiveGeomVec();

    vector < string > geom_id_vec = m_VehiclePtr->GetGeomVec();
    vector < Geom* > geom_ptr_vec = m_VehiclePtr->FindGeomVec( geom_id_vec );

    //==== Rebuild Tree if Redraw Required ====//
    if ( m_RedrawFlag )
    {
        //==== Clear old tree ====//
        m_GeomBrowser->clear();

        //==== Display Vehicle Name ====//
        string vid_str = m_VehiclePtr->GetID();
        string vstr = m_VehiclePtr->GetName();

        TreeIconItem* geom_tree_item;

        geom_tree_item = m_GeomBrowser->AddRow( vid_str.c_str(), nullptr );
        geom_tree_item->SetRefID( vid_str );

        //==== Step Thru Geoms ====//
        for ( int i = 0 ; i < ( int )geom_ptr_vec.size() ; i++ )
        {
            if ( geom_ptr_vec[i] )
            {
                string id_str = geom_ptr_vec[i]->GetID();
                TreeIconItem* parent_item = nullptr;
                string parent_id = geom_ptr_vec[i]->GetParentID();
                
                // Note to DEVS: if geom_vec is ALWAYS hierachically provided, this method works, but issue is what if a child is read before its parent is registered?
                if ( parent_id != string( "NONE" ) )
                {
                    parent_item = m_GeomBrowser->GetItemByRefId( parent_id );
                }

                if ( parent_item )
                {
                    geom_tree_item = m_GeomBrowser->AddRow( id_str.c_str(), parent_item );
                }
                else
                {
                    geom_tree_item = m_GeomBrowser->AddRow( id_str.c_str() );
                }
                geom_tree_item->SetRefID( id_str );
            }
        }
    }
    SetRedrawFlag();

    //==== Update Tree Items' Label Fonts & Colors ====//
    for ( Fl_Tree_Item *tree_item = m_GeomBrowser->first(); tree_item; tree_item = m_GeomBrowser->next(tree_item) )
    {
        TreeIconItem* geom_tree_item = dynamic_cast< TreeIconItem* >( tree_item );
        if ( geom_tree_item )
        {
            string id = geom_tree_item->GetRefID();

            Geom* gPtr = m_VehiclePtr->FindGeom( id );

            Vehicle* vPtr = ( id == m_VehiclePtr->GetID() ) ? m_VehiclePtr : nullptr;

            // Set icons for vehicle item, set to indeterminate if vehicles' geoms have multiple states of show or surf
            if ( vPtr )
            {
                string vstr = vPtr->GetName();
                geom_tree_item->label( vstr.c_str() );
                if ( vPtr->GetAttrCollection()->GetAttrDataFlag() )
                {
                    geom_tree_item->labelcolor( FL_DARK_MAGENTA );
                }

                if ( !m_VehOpen )
                {
                    m_GeomBrowser->close( const_cast< TreeIconItem* >( geom_tree_item ), false );
                }

                bool indet_show = false;
                bool indet_surf = false;

                vector < Geom* > geom_vec;
                bool veh_in_vec = ( find( m_SelVec.begin(), m_SelVec.end(), id ) != m_SelVec.end() );
                if ( m_SelVec.size() > 1 && veh_in_vec )
                {
                    geom_vec = m_VehiclePtr->FindGeomVec( m_SelVec );
                }
                else
                {
                    geom_vec = geom_ptr_vec;
                }

                // if at least one geom in vehicle with valid pointer, initialize vehicle icons to its show/surf state
                if ( !geom_vec.empty() && geom_vec[0] )
                {
                    geom_tree_item->SetShowState( geom_vec[0]->GetSetFlag( vsp::SET_SHOWN ) );
                    geom_tree_item->SetSurfState( geom_vec[0]->m_GuiDraw.GetDrawType() );
                }
                if ( geom_vec.size() > 1 )
                {
                    for ( int i = 1; i < geom_vec.size(); i++ )
                    {
                        if ( geom_vec[0] && geom_vec[i] )
                        {
                            // if any of the subsequent geoms have conflicting state, set show / surf to indeterminate state for vehicle item
                            if ( !indet_show && geom_vec[i]->GetSetFlag( vsp::SET_SHOWN ) != geom_vec[0]->GetSetFlag( vsp::SET_SHOWN ) )
                            {
                                indet_show = true;
                            }
                            if ( !indet_surf && geom_vec[i]->m_GuiDraw.GetDrawType() != geom_vec[0]->m_GuiDraw.GetDrawType() )
                            {
                                indet_surf = true;
                            }
                        }
                    }
                }
                geom_tree_item->SetIndetShowState( indet_show );
                geom_tree_item->SetIndetSurfState( indet_surf );
            }

            else if ( gPtr )
            {
                //==== Update Geom Labels ====//

                string label_str;

                bool orphaned_attach = false;
                bool has_attr = gPtr->GetAttrCollection()->GetAttrDataFlag();

                string parent_id = gPtr->GetParentID();
                Geom* parent_ptr = m_VehiclePtr->FindGeom( parent_id );

                if ( gPtr->m_TransAttachFlag() != vsp::ATTACH_TRANS_NONE ||
                     gPtr->m_RotAttachFlag() != vsp::ATTACH_ROT_NONE )
                {
                    geom_tree_item->SetHConnLine( TREE_LINE_CONN::STYLE_THICK );

                    if ( !parent_ptr )
                    {
                        orphaned_attach = true;
                    }
                }

                // Is this geom a HINGE? Change its child lines to double style
                HingeGeom* hPtr = dynamic_cast < HingeGeom* > ( gPtr );
                if ( hPtr )
                {
                    geom_tree_item->SetChildVConnLine( TREE_LINE_CONN::STYLE_DOUBLE );
                }

                if ( parent_ptr )
                {
                    // Is this geom ATTACHED to a hinge? Change its attachment lines to double style
                    HingeGeom* parent_hPtr = dynamic_cast < HingeGeom* > ( parent_ptr );
                    if ( parent_hPtr )
                    {
                        geom_tree_item->SetHConnLine( TREE_LINE_CONN::STYLE_DOUBLE );
                        geom_tree_item->SetVConnLine( TREE_LINE_CONN::STYLE_DOUBLE );
                    }
                }

                bool indet_show = false;
                bool indet_surf = false;
                // if Geom has hidden children as a collapsed parent item, show indeterminate svg icons to indicate mixed options in hidden children
                if ( !gPtr->m_GuiDraw.GetDisplayChildrenFlag() )
                {
                    vector < string > g_vec;
                    gPtr->LoadIDAndChildren( g_vec );
                    for ( int i = 0; i < g_vec.size(); i++ )
                    {
                        Geom* gChild = m_VehiclePtr->FindGeom( g_vec[i] );
                        if ( gChild )
                        {
                            // if any children of a collapsed geom item conflict show/surf state, set its icon as indeterminate
                            if ( gChild->GetSetFlag( vsp::SET_SHOWN ) != gPtr->GetSetFlag( vsp::SET_SHOWN ) )
                            {
                                indet_show = true ;
                            }
                            if ( gChild->m_GuiDraw.GetDrawType() != gPtr->m_GuiDraw.GetDrawType() )
                            {
                                indet_surf = true;
                            }
                        }
                    }
                }
                geom_tree_item->SetIndetShowState( indet_show );
                geom_tree_item->SetIndetSurfState( indet_surf );

                // Set label to geom name
                label_str.append( gPtr->GetName() );

                // Set icons of tree based on geom's show/surf state (to be shown if not indeterminate)
                geom_tree_item->SetShowState( gPtr->GetSetFlag( vsp::SET_SHOWN ) );
                geom_tree_item->SetSurfState( gPtr->m_GuiDraw.GetDrawType() );
                geom_tree_item->label( label_str.c_str() );

                // Set label color if it has attributes or is orphaned
                int label_color = has_attr ? FL_DARK_MAGENTA : FL_FOREGROUND_COLOR;
                if ( orphaned_attach )
                {
                    label_color = fl_lighter( label_color );
                }
                geom_tree_item->labelcolor( label_color );

                // Close item if display children flag false
                if ( !gPtr->m_GuiDraw.GetDisplayChildrenFlag() )
                {
                    m_GeomBrowser->close( const_cast< TreeIconItem* >( geom_tree_item ), false );
                }
            }
        }
    }
    m_GeomBrowser->redraw();

    //==== Select Vehicle if Previously Clicked ====//
    if ( m_VehSelected )
    {
        SelectGeomBrowser();
    }
    else
    {
        //==== Restore List of Selected Geoms ====//
        for ( int i = 0 ; i < ( int )activeVec.size() ; i++ )
        {
            SelectGeomBrowser( activeVec[i] );
        }
    }
}

void ManageGeomScreen::SelectGeomBrowser( const string &geom_id )
{
    //==== Change id selection to VehID if bool flag active ====//
    string select_id = m_VehSelected ? m_VehiclePtr->GetID() : geom_id;

    //==== Select ID If Match ====//
    TreeIconItem* geom_tree_item = m_GeomBrowser->GetItemByRefId( geom_id );
    if ( geom_tree_item )
    {
        int do_callback = 0;
        m_GeomBrowser->select( geom_tree_item, do_callback );
    }
}

vector< string > ManageGeomScreen::GetSelectedBrowserItems()
{
    vector< TreeIconItem* > tree_items;
    vector< string> id_vec;

    m_GeomBrowser->GetSelectedItems( &(tree_items) );

    for ( int i = 0; i < tree_items.size() ; i++ )
    {
        string id = tree_items.at(i)->GetRefID();
        if ( !id.empty() )
        {
            id_vec.push_back( id );
        }
    }
    return id_vec;
}

//==== Load Active Geom Output ====//
void ManageGeomScreen::LoadActiveGeomOutput()
{
    vector< string > activeVec = m_VehiclePtr->GetActiveGeomVec();
    if ( m_VehSelected )
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
    else if ( activeVec.size() > 1 )
    {
        m_ActiveGeomInput.Update( "<multiple>" );
    }
    else
    {
        m_ActiveGeomInput.Update( "" );
    }
}

//==== Load Type Choice ====//
void ManageGeomScreen::LoadTypeChoice()
{
    m_GeomTypeChoice.ClearItems();
    m_GeomTypeChoice.ClearFlags();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    int num_type  = veh->GetNumGeomTypes();
    int num_fixed = veh->GetNumFixedGeomTypes();
    int cnt = 1;

    for ( int i = 0 ; i < num_type ; i++ )
    {
        GeomType type = veh->GetGeomType( i );

        string item = type.m_Name.c_str();

        if ( !type.m_FixedFlag )
        {
            item = StringUtil::int_to_string( cnt, "%d.  " ) + item;
            cnt++;
        }
        m_GeomTypeChoice.AddItem( item.c_str() );
    }

    m_GeomTypeChoice.SetFlagByVal( num_fixed - 1, FL_MENU_DIVIDER );

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
        SetNeedsShowHideGeoms();
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
    if ( !m_VehiclePtr )
    {
        return;
    }

    string id;
    int icon_event = 0;
    bool show_state;
    int surf_state;

    // Check if any tree icons are triggered and handle open/close here; show/surf icon handling after selVec populated.
    TreeIconItem* tree_item = dynamic_cast< TreeIconItem* >( m_GeomBrowser->callback_item() );
    if ( tree_item )
    {
        // if item is clicked on collapse icon, toggle it and set that bool to the geom/vehicle's DisplayChildrenFlag
        if ( tree_item->event_on_collapse_icon( m_GeomBrowser->prefs() ) )
        {
            id = tree_item->GetRefID();
            Geom* g = m_VehiclePtr->FindGeom( id );
            Vehicle* vPtr = ( id == m_VehiclePtr->GetID() ) ? m_VehiclePtr : nullptr;

            bool open_flag = tree_item->is_open();
            if( g )
            {
                g->m_GuiDraw.SetDisplayChildrenFlag( open_flag );
            }
            else if ( vPtr )
            {
                m_VehOpen = open_flag;
            }

            if ( !open_flag )
            {
                vector< TreeIconItem* > tree_items;
                m_GeomBrowser->GetSelectedItems( &(tree_items) );
                for ( int i = 0; i < tree_items.size(); i ++ )
                {
                    if ( tree_items[i]->CheckParent( tree_item ) )
                    {
                        m_GeomBrowser->select_only( tree_item, false );
                    }
                }
            }
        }
    }

    // Get the tree item associated with the event, if any
    string icon_event_id = string();
    TreeIconItem* tree_icon_item = m_GeomBrowser->GetEventItem();
    if ( tree_icon_item )
    {
        int e = tree_icon_item->GetIconEvent();
        if ( e > 0 )
        {
            tree_icon_item->IndexIcon();
            icon_event = e;
            show_state = tree_icon_item->GetShowState();
            surf_state = tree_icon_item->GetSurfState();
            icon_event_id = tree_icon_item->GetRefID();
        }
        m_GeomBrowser->ClearEventItem();
    }

    //==== Find Vector of All Selections ====//
    vector< string > selVec;

    // Generate selection vector from scratch if not an icon event
    if ( icon_event == 0 )
    {
        m_SelVec = GetSelectedBrowserItems();
    }
    else // if icon event happens, append the associated tree item to the selection vector
    {
        bool has_item = false;
        for ( int i = 0; i < m_SelVec.size(); i++ )
        {
            if ( m_SelVec[i] == icon_event_id )
            {
                has_item = true;
            }
        }

        if ( !has_item )
        {
            // if the icon event item not in current selection vector, then select that item only
            TreeIconItem* sel_item = nullptr;

            m_SelVec.clear();
            m_SelVec.push_back( icon_event_id );

            sel_item = m_GeomBrowser->GetItemByRefId( icon_event_id );
            if ( sel_item )
            {
                m_GeomBrowser->select_only( sel_item, 0 );
            }
        }
    }
    selVec = m_SelVec;


    if ( selVec.size() == 1 && selVec[0] == m_VehiclePtr->GetID() )
    {
        m_ScreenMgr->ShowScreen( vsp::VSP_VEH_SCREEN );
        m_VehSelected = true;
        SelectGeomBrowser();
    }

    else
    {
        m_ScreenMgr->HideScreen( vsp::VSP_VEH_SCREEN );
        m_VehSelected = false;
    }

    //==== Handle show/surf icon event with selVec geom vector ====//
    if ( icon_event > 0 )
    {
        for ( int i = 0; i < selVec.size(); i++ )
        {
            id = selVec[i];
            Geom* g = m_VehiclePtr->FindGeom( id );

            vector < string > hidden_id_vec;

            if ( g )
            {
                if ( g->m_GuiDraw.GetDisplayChildrenFlag() )
                {
                    hidden_id_vec.push_back( id );
                }
                else
                {
                    bool check_display_flag = false;
                    g->LoadIDAndChildren( hidden_id_vec, check_display_flag );
                }
            }
            else if ( id == m_VehiclePtr->GetID() && selVec.size() == 1 )
            {
                hidden_id_vec = m_VehiclePtr->GetGeomVec();
            }
            for ( int j = 0; j < hidden_id_vec.size(); j++ )
            {
                Geom* hg = m_VehiclePtr->FindGeom( hidden_id_vec[j] );
                if( hg )
                {
                    if ( icon_event == 1 )
                    {
                        hg->SetSetFlag( SET_SHOWN , show_state );
                        hg->SetSetFlag( SET_NOT_SHOWN , !show_state );
                    }
                    else if ( icon_event == 2)
                    {
                        hg->m_GuiDraw.SetDrawType( surf_state );
                    }
                }
            }
        }
    }

    // Select all children with Alt-click
    if ( selVec.size() > 0 && Fl::event_state( FL_ALT ) )
    {
        Geom* lastSelGeom = m_VehiclePtr->FindGeom( selVec.back() );
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

    //==== Downselect to only Geoms from selVec====//
    vector < string > selGeomVec;
    for ( int i = 0; i < selVec.size(); i++ )
    {
        Geom* g = m_VehiclePtr->FindGeom( selVec[i] );
        if ( g )
        {
            selGeomVec.push_back( selVec[i] );
        }
    }

    m_VehiclePtr->SetActiveGeomVec( selGeomVec );

    // ROB comment: these may automatically happen anyways; try and cut these?
    SetNeedsShowHideGeoms();
    LoadActiveGeomOutput();
}

//==== Show/NoShow Active Geoms and Children ====//
void ManageGeomScreen::NoShowActiveGeoms( bool flag )
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
void ManageGeomScreen::SelectAll()
{
    vector < string > geom_vec = m_VehiclePtr->GetGeomVec();
    m_VehiclePtr->SetActiveGeomVec( geom_vec );

    m_VehSelected = false;

    //==== Restore List of Selected Geoms ====//
    for ( int i = 0 ; i < ( int )geom_vec.size() ; i++ )
    {
        SelectGeomBrowser( geom_vec[i] );
    }

    LoadActiveGeomOutput();
    SetNeedsShowHideGeoms();
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
    SetNeedsShowHideGeoms();
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
            geom_vec[i]->SetDirtyFlag( GeomBase::TESS );
            geom_vec[i]->SetLateUpdateFlag( true );
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

void ManageGeomScreen::EditName( const string &name )
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


//==== Create Screens ====//
void ManageGeomScreen::CreateScreens()
{
    m_GeomScreenVec.resize( vsp::NUM_GEOM_SCREENS );
    m_GeomScreenVec[vsp::POD_GEOM_SCREEN] = new PodScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::FUSELAGE_GEOM_SCREEN] = new FuselageScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::MS_WING_GEOM_SCREEN] = new WingScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::BLANK_GEOM_SCREEN] = new BlankScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::MESH_GEOM_SCREEN] = new MeshScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::STACK_GEOM_SCREEN] = new StackScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::CUSTOM_GEOM_SCREEN] = new CustomScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::PT_CLOUD_GEOM_SCREEN] = new PtCloudScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::PROP_GEOM_SCREEN] = new PropScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::HINGE_GEOM_SCREEN] = new HingeScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::MULT_GEOM_SCREEN] = new MultTransScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::CONFORMAL_SCREEN] = new ConformalScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::ELLIPSOID_GEOM_SCREEN] = new EllipsoidScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::BOR_GEOM_SCREEN] = new BORScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::HUMAN_GEOM_SCREEN] = new HumanGeomScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::WIRE_FRAME_GEOM_SCREEN] = new WireScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::ROUTING_GEOM_SCREEN] = new RoutingScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::AUXILIARY_GEOM_SCREEN] = new AuxiliaryGeomScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::SUPER_CONE_GEOM_SCREEN] = new SuperConeScreen( m_ScreenMgr );
    m_GeomScreenVec[vsp::GEAR_GEOM_SCREEN] = new Gearcreen( m_ScreenMgr );

    for ( int i = 0 ; i < ( int )m_GeomScreenVec.size() ; i++ )
    {
        m_GeomScreenVec[i]->GetFlWindow()->set_non_modal();
    }
}

//==== Show Hide Geom Screen Depending on Active Geoms ====//
void ManageGeomScreen::ShowHideGeomScreens()
{
    //==== Show Screen - Each Screen Will Test Check Valid Active Geom Type ====//
    for ( int i = 0; i < ( int ) m_GeomScreenVec.size(); i++ )
    {
        if ( !m_ScreenMgr->IsGeomScreenDisabled( i ) )
        {
            m_GeomScreenVec[ i ]->Show();
        }
        else
        {
            m_GeomScreenVec[ i ]->Hide();
        }
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

    m_VehiclePtr->Update();
}

//==== Callbacks ====//
void ManageGeomScreen::CallBack( Fl_Widget *w )
{
    assert( m_ScreenMgr );

    if ( w == m_GeomBrowser )
    {
        GeomBrowserCallback();

        //==== Existing geomtree items persist through update ====//
        ClearRedrawFlag();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageGeomScreen::CloseCallBack( Fl_Widget *w )
{
    m_ScreenMgr->SetUpdateFlag( true );
    Hide();
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
        m_VehiclePtr->ReorderActiveGeom( vsp::REORDER_MOVE_UP );
    }
    else if ( device == &m_MoveDownButton )
    {
        m_VehiclePtr->ReorderActiveGeom( vsp::REORDER_MOVE_DOWN );
    }
    else if ( device == &m_MoveTopButton )
    {
        m_VehiclePtr->ReorderActiveGeom( vsp::REORDER_MOVE_TOP );
    }
    else if ( device == &m_UpParentButton )
    {
        m_VehiclePtr->ReparentActiveGeom( vsp::REORDER_MOVE_UP );
    }
    else if ( device == &m_DownParentButton )
    {
        m_VehiclePtr->ReparentActiveGeom( vsp::REORDER_MOVE_DOWN );
    }
    else if ( device == &m_MoveBotButton )
    {
        m_VehiclePtr->ReorderActiveGeom( vsp::REORDER_MOVE_BOTTOM );
    }
    else if ( device == &m_SetChoice )
    {
        m_SetIndex = m_SetChoice.GetVal();
    }
    else if ( device == &m_ShowOnlySetButton )
    {
        m_VehiclePtr->ShowOnlySet( m_SetIndex );
    }
    else if ( device == &m_ShowSetButton )
    {
        m_VehiclePtr->ShowSet( m_SetIndex );
    }
    else if ( device == &m_NoShowSetButton )
    {
        m_VehiclePtr->NoShowSet( m_SetIndex );
    }
    else if ( device == &m_SelectSetButton )
    {
        SelectSet( m_SetIndex );
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

VspScreen* ManageGeomScreen::GetGeomScreen( int id )
{
    if( id >= 0 && id < vsp::NUM_GEOM_SCREENS )
    {
        return m_GeomScreenVec[id];
    }
    // Should not reach here.
    assert( false );
    return nullptr;
}

VspScreen* ManageGeomScreen::GetShownGeomScreen()
{
    for ( int i = 0; i < ( int ) m_GeomScreenVec.size(); i++ )
    {
        if ( m_GeomScreenVec[ i ]->IsShown() )
        {
            return m_GeomScreenVec[ i ];
        }
    }
    return nullptr;
}

std::string ManageGeomScreen::getFeedbackGroupName()
{
    return std::string("GeomGUIGroup");
}

void ManageGeomScreen::Set( const std::string &geomId )
{
    printf("%s\n", geomId.c_str());
    m_VehiclePtr->SetActiveGeom(geomId);

    SetNeedsShowHideGeoms();
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
