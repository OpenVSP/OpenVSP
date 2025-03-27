//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AttributeEditor.cpp: Extensive attribute editor screen.
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#include "AttributeEditor.h"
#include "AttributeScreen.h"
#include "ParmMgr.h"
#include "ScreenMgr.h"

//==== Attribute Tree GUI ====//
AttributeTree::AttributeTree()
{
    m_Screen = NULL;
    m_FilterAttrType = vsp::INVALID_TYPE;
    m_FilterObjType = vsp::ATTROBJ_FREE;
    m_FilterStr = string( "" );
    m_RedrawFlag = true;
}

void AttributeTree::Init( ScreenMgr* mgr, GroupLayout * layout, Fl_Group* group, VspScreen *screen, Fl_Callback *cb, bool mod_start, int start_y, int browser_h )
{

    m_AttrTree = layout->AddTreeWithColumns( browser_h );
    m_AttrTree->showroot( true );
    m_AttrTree->callback( StaticDeviceCB, this );
    m_AttrTree->selectmode( FL_TREE_SELECT_MULTI );
    m_AttrTree->item_reselect_mode( FL_TREE_SELECTABLE_ALWAYS );
    m_AttrTree->sortorder( FL_TREE_SORT_NONE );
    // m_AttrTree->when( FL_WHEN_RELEASE_ALWAYS );

    m_ColWidths[0] = 120;
    m_ColWidths[1] = 120;
    m_ColWidths[2] = 40;
    m_ColWidths[3] = 0;

    m_AttrTree->column_widths( m_ColWidths );            // set column widths array
    m_AttrTree->resizing(true);                       // enable interactive resizing
    m_AttrTree->column_char('\t');                    // use tab char as column delimiter
    m_AttrTree->first_column_minw(100);               // minimum width of first column

    int fontsize = 12;
    m_AttrTree->item_labelsize( fontsize );

    m_Screen = screen;
    m_ScreenMgr = mgr;
}

void AttributeTree::Activate()
{
    m_AttrTree->activate();
}

void AttributeTree::Deactivate()
{
    m_AttrTree->deactivate();
}

void AttributeTree::Update()
{
    m_OpenBranchVec.clear();
    for ( int i = 0; i != m_OpenBranchUpdate.size(); ++i )
    {
        m_OpenBranchVec.push_back( m_OpenBranchUpdate.at(i) );
    }
    m_OpenBranchUpdate.clear();

    TrimCloseVec();

    if ( m_RedrawFlag )
    {
        UpdateTree();
    }
    SetRedrawFlag();
}

void AttributeTree::UpdateTree()
{
    bool use_auto_select = !( m_AutoSelectIDs.empty() );

    TreeRowItem* root;
    TreeRowItem* focus_item;
    TreeRowItem* child_item;

    AttributeCollection* ac;
    NameValData* nvd;

    string root_id = "";



    if ( m_AttrTree )
    {
        m_TreeItemArray.clear();
        m_AttrTree->clear();   //GUI element
    }

    string header_row = "AttrTree\tData\tType";
    root = m_AttrTree->AddRow( header_row.c_str() );

    root->labelbgcolor( FL_WHITE );
    root->labelcolor( FL_BLACK );
    root->labelfont( FL_HELVETICA_BOLD );

    if ( !m_AttrRootCollID.empty() )
    {
        root_id = AttributeMgr.GetObjectParent( m_AttrRootCollID );
    }

    vector < vector < vector < string > > > tree_id_coll_vecs = AttributeMgr.GetAttrTreeVec( root_id , m_OpenBranchVec, true, m_FilterAttrType, m_FilterObjType, m_FilterStr, m_FilterCase ); //

    vector < vector < string > > tree_id_vecs = tree_id_coll_vecs[0];
    vector < vector < string > > tree_coll_vecs = tree_id_coll_vecs[1];

    string id;
    vector < string > id_vec;

    if ( m_AttrTree )
    {
        for ( int i = 0; i != tree_id_vecs.size(); ++i )
        {
            focus_item = root;

            id_vec.clear();

            for ( int j = 0; j != tree_id_vecs[i].size(); ++j )
            {
                string id = tree_id_vecs[i][j];
                string coll_id = tree_coll_vecs[i][j];

                string row_str = AttributeMgr.GetName( id );

                id_vec.push_back( id );

                child_item = nullptr;

                if ( focus_item )
                {
                    child_item = m_AttrTree->GetItemByVecId( id_vec );
                }

                if ( !child_item )
                {
                    ac = AttributeMgr.GetCollectionPtr( coll_id );
                    nvd = AttributeMgr.GetAttributePtr( id );

                    if ( nvd )
                    {
                        row_str += '\t' + nvd->GetAsString( true ) + '\t' + nvd->GetTypeName( nvd->GetType(), true , true );
                    }
                    else
                    {
                        row_str += "\t-\t-";
                    }

                    child_item = m_AttrTree->AddRow( row_str.c_str(), focus_item );

                    // creating a child item inserts its id vec and tree item into the map of ID vectors, used to disambiguate same-named containers

                    // insert id vec into that child item...
                    child_item->SetRefVecID( id_vec );
                    if ( nvd )
                    {
                        child_item->SetRefAttrID( id );

                        if ( nvd->GetType() == vsp::ATTR_COLLECTION_DATA )
                        {
                            child_item->labelcolor( FL_DARK_MAGENTA );
                        }
                        else if ( nvd->GetType() == vsp::PARM_REFERENCE_DATA )
                        {
                            string pid = nvd->GetParmID( 0 );
                            Parm* p = ParmMgr.FindParm( pid );
                            if ( !pid.empty() && !p )
                            {
                                child_item->labelfont( FL_HELVETICA_BOLD );
                                child_item->labelcolor( FL_RED );
                            }
                        }
                    }
                    else
                    {
                        child_item->labelfont( FL_HELVETICA_BOLD );
                        child_item->labelcolor( FL_DARK_MAGENTA );
                    }

                    if ( ac )
                    {
                        child_item->SetRefCollID( coll_id );
                    }

                    if ( nvd || ac )
                    {
                        if ( !use_auto_select )
                        {
                            for ( int k = 0; k != m_SelectIDs.size(); ++k )
                            {
                                if ( !TreeWithColumns::CheckVecMatch( m_SelectIDs.at( k ), id_vec ) )
                                {
                                    m_AttrTree->select( child_item, 0 );
                                    m_TreeItemArray.push_back( child_item );
                                }
                            }
                        }
                        else
                        {
                            for ( int k = 0; k != m_AutoSelectIDs.size(); ++k )
                            {
                                if ( id.compare( m_AutoSelectIDs.at( k ) ) == 0 )
                                {
                                    m_AttrTree->select( child_item, 0 );
                                    m_TreeItemArray.push_back( child_item );
                                }
                            }
                        }
                    }

                    for ( int k = 0; k != m_CloseVec.size(); ++k )
                    {
                        if ( !TreeWithColumns::CheckVecMatch( m_CloseVec.at( k ), id_vec ) )
                        {
                            m_AttrTree->close( child_item, false );
                        }
                    }
                }
                focus_item = child_item;
            }
        }
        m_AutoSelectIDs.clear();
        m_AttrTree->redraw();
    }
    SetTreeAttrID();
}

void AttributeTree::SetTreeAttrID()
{
    // if selected_id_vec is size 1, assign attribute pointer
    // if all selected IDs share a collection, assign collection pointer
    m_SelectIDs.clear();
    m_AttrIDs.clear();
    m_CollIDs.clear();

    for ( int i = 0; i != m_TreeItemArray.size(); ++i )
    {
        if ( !m_TreeItemArray.at( i )->GetRefAttrID().empty() )
        {
            m_AttrIDs.push_back( m_TreeItemArray.at( i )->GetRefAttrID() );
        }
        if ( !m_TreeItemArray.at( i )->GetRefCollID().empty() )
        {
            m_CollIDs.push_back( m_TreeItemArray.at( i )->GetRefCollID() );
        }
        m_SelectIDs.push_back( m_TreeItemArray.at( i )->GetRefVecID() );
    }
}

void AttributeTree::AddEmptyCollID( const string & coll_id )
{
    bool already_contains = false;
    for (int i = 0; i != m_OpenBranchUpdate.size(); ++i )
    {
        if ( m_OpenBranchUpdate[i].compare( coll_id ) == 0 )
        {
            already_contains = true;
        }
    }
    if ( !already_contains )
    {
        m_OpenBranchUpdate.push_back( coll_id );
    }
}

void AttributeTree::AddEmptyCollID( const vector < string > & coll_ids )
{
    for (int i = 0; i != coll_ids.size(); ++i )
    {
        AddEmptyCollID( coll_ids[i] );
    }
}

void AttributeTree::SetEmptyCollID( const vector < string > & coll_ids )
{
    m_OpenBranchUpdate = coll_ids;
}

void AttributeTree::SetSearchTerms( int attr_type, int attach_type, const string & attr_str, bool case_sens )
    {
        m_FilterAttrType = attr_type;
        m_FilterObjType = attach_type;
        m_FilterStr = attr_str;
        m_FilterCase = case_sens;

        if ( m_FilterAttrType != vsp::INVALID_TYPE || m_FilterObjType != vsp::ATTROBJ_FREE || !m_FilterStr.empty() )
        {
            SetAllNodes( true );
        }
    }

bool AttributeTree::ItemState( const vector < string > & id_vec )
{
    for ( int i = 0; i != m_CloseVec.size(); ++i )
    {
        if ( !TreeWithColumns::CheckVecMatch( m_CloseVec.at( i ), id_vec ) )
        {
            return false;
        }
    }
    return true;
}

bool AttributeTree::ItemState( TreeRowItem* tree_item )
{
    vector < string > id_vec = tree_item->GetRefVecID();
    bool item_state = ItemState( id_vec );
    return item_state;
}

void AttributeTree::SetState( const vector < string > & id_vec, bool open_state )
{
    if ( ItemState( id_vec ) != open_state )
    {
        ToggleState( id_vec );
    }
}

void AttributeTree::ToggleState( const vector < string > & id_vec )
{
    vector < vector < string > > :: iterator iter;
    if ( ItemState( id_vec ) )
    {
        m_CloseVec.push_back( id_vec );
    }
    else
    {
        iter = find( m_CloseVec.begin(), m_CloseVec.end(), id_vec );
        m_CloseVec.erase( iter );
    }
}

void AttributeTree::SetAllNodes( bool open_state )
{

    m_CloseVec.clear();
    if ( !open_state )
    {
        for ( Fl_Tree_Item *item = m_AttrTree->first(); item; item = m_AttrTree->next(item) )
        {
            TreeRowItem* tr_item = static_cast< TreeRowItem* >( item );
            if ( tr_item )
            {
                m_CloseVec.push_back( tr_item->GetRefVecID() );
            }
        }
    }
}

void AttributeTree::TrimCloseVec()
{
    bool vec_found;
    vector < vector < string > > close_vec_update;
    vector < vector < string > > open_vec_update;
    vector < string > open_vec;
    vector < string > open_vec_slice;

    string collid;
    string attachid;

    for ( int j = 0; j != m_OpenBranchVec.size(); ++j )
    {
        collid = m_OpenBranchVec[j];
        attachid = AttributeMgr.GetObjectParent( m_OpenBranchVec[j] );
        //find the map tree item with that ID, and get its vector to that point...

        for ( Fl_Tree_Item *item = m_AttrTree->first(); item; item = m_AttrTree->next(item) )
        {
            TreeRowItem* tr_item = static_cast< TreeRowItem* >( item );
            if ( tr_item )
            {
                vector < string > id_vec = tr_item->GetRefVecID();
                if ( !id_vec.empty() && id_vec.back() == attachid )
                {
                    open_vec = id_vec;
                    open_vec.push_back( collid );
                    open_vec_update.push_back( open_vec );
                }
            }
        }
    }

    for ( int i = 0; i != m_CloseVec.size(); ++i )
    {
        vec_found = false;
        // loop through the attrTree items...

        TreeRowItem* tr_item_check = m_AttrTree->GetItemByVecId( m_CloseVec.at(i) );
        if ( tr_item_check )
        {
            vec_found = true;
        }

        for ( int j = 0; j != open_vec_update.size(); ++j )
        {
            // if the keep_open_vector is at least as long as the close_vector, then compare the vector of strings, cutting the open-vec to the length of the close_vec
            if ( open_vec_update.at(j).size() >= m_CloseVec.at(i).size() )
            {
                open_vec_slice = vector < string >( open_vec_update.at(j).begin(), open_vec_update.at(j).begin() + m_CloseVec.at(i).size() );
                if ( !TreeWithColumns::CheckVecMatch( open_vec_slice, m_CloseVec.at(i) ) )
                {
                    vec_found = false;
                }
            }
        }

        if ( vec_found )
        {
            close_vec_update.push_back( m_CloseVec.at(i) );
        }
    }

    m_CloseVec = close_vec_update;
}

void AttributeTree::ClearExtraIDs()
{
    m_OpenBranchUpdate.clear();
    m_OpenBranchVec.clear();
}

void AttributeTree::SetAutoSelectID( const vector < string > & ids )
{
    m_AutoSelectIDs = ids;
}

void AttributeTree::SetTreeRootID( const string & attrCollectionID )
{
    m_AttrRootCollID = attrCollectionID;
    AttributeCollection* ac = AttributeMgr.GetCollectionPtr( m_AttrRootCollID );
    if ( ac )
    {
        AddEmptyCollID( m_AttrRootCollID );
    }
}

void AttributeTree::DeviceCB( Fl_Widget *w )
{
    TreeRowItem* tree_item = static_cast < TreeRowItem* >( m_AttrTree->find_clicked() );

    m_TreeItemArray.clear();
    m_AttrTree->GetSelectedItems( &(m_TreeItemArray) );

    // if item is clicked on collapse icon, toggle it and store that in the open/close vecs
    if ( tree_item && tree_item->event_on_collapse_icon( m_AttrTree->prefs() ) )
    {
        vector < string > id_vec = tree_item->GetRefVecID();

        // if tree_item is in the openVec, then add it to the closed vec and remove it from openVec
        if ( !id_vec.empty() )
        {
            ToggleState( id_vec );
        }
    }

    SetTreeAttrID();

    if ( m_Screen )
    {
        m_Screen->CallBack( w );
    }
}

// ==== Attribute Editor ==== //
void AttributeEditor::Init( ScreenMgr* mgr, GroupLayout * layout, Fl_Group* group, VspScreen *screen, Fl_Callback *cb, bool mod_start, int start_y, int browser_h )
{
    GuiDevice::Init( screen );

    m_ScreenMgr = mgr;

    // ==== build layout ==== //
    layout->AddDividerBox( "Attributes (Double Click For Explorer)" );
    layout->AddSubGroupLayout( m_AttrCommonGroup, layout->GetW(), layout->GetRemainY() );

    if ( mod_start )
    {
        m_AttrCommonGroup.SetY( start_y );
    }

    m_AttrTreeWidget.Init( mgr, layout, group, screen, cb, true, m_AttrCommonGroup.GetY(), browser_h );
    m_ShowState = false;
}

void AttributeEditor::Show()
{
    Update();
}

void AttributeEditor::Update()
{
    m_AttrTreeWidget.Update();
}

void AttributeEditor::SetEditorCollID( string collID )
{
    m_AttrCollID = collID;
    m_AttrTreeWidget.SetTreeRootID( collID );
}

void AttributeEditor::DeviceCB( Fl_Widget* w )
{
    // for GUI events in the column browser:
    if ( w == m_AttrTreeWidget.GetTreeWidget() )
    {
        if ( Fl::event_clicks() != 0 )
        {
            vector < string > ids = m_AttrTreeWidget.GetSelectedID();
            if ( ids.size() == 1 )
            {
                static_cast< AttributeExplorer* >( m_ScreenMgr->GetScreen( vsp::VSP_ATTRIBUTE_EXPLORER_SCREEN ) )->SetTreeAutoSelectID( ids.front() );
            }
            m_ScreenMgr->ShowScreen( vsp::VSP_ATTRIBUTE_EXPLORER_SCREEN );
        }
        m_AttrTreeWidget.ClearRedrawFlag();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void AttributeEditor::GuiDeviceCallBack( GuiDevice* gui_device )
{
    m_ScreenMgr->SetUpdateFlag( true );
}

bool AttributeEditor::canMakeInt( const::string& str )
{
    bool int_flag = false;
    try
    {
        stoi( str );
        int_flag = true;
    }
    catch ( ... ) {}
    return int_flag;
}

bool AttributeEditor::canMakeDbl( const::string& str )
{
    bool dbl_flag = false;
    try
    {
        stod( str );
        dbl_flag = true;
    }
    catch ( ... ) {}
    return dbl_flag;
}
