//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AttributeScreen.cpp: GUI Stuff for working with attributes
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#include "AttributeScreen.h"
#include "AttributeEditor.h"
#include "AttributeManager.h"
#include "LinkMgr.h"
#include "ParmMgr.h"
#include "VehicleMgr.h"
#include "ManageGeomScreen.h"
#include "AdvLinkScreen.h"
#include "ManageMeasureScreen.h"
#include "ParmLinkScreen.h"
#include "ParmScreen.h"
#include "ScreenBase.h"

//===============================================================================//
//===============================================================================//
//===============================================================================//

AttributeExplorer::AttributeExplorer( ScreenMgr* mgr ) : BasicScreen( mgr, 800, 680, "Attribute Explorer", "AttributeExplorer.html" )
{
    m_ScreenMgr = mgr;

    int editor_ht = 520;

    int tree_w = 450;

    m_AttrDetailLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_FLTK_Window->resizable( m_AttrDetailLayout.GetGroup() );
    m_FLTK_Window->size_range( m_FLTK_Window->w()/2, m_FLTK_Window->h()/2 );

    m_AttrDetailLayout.AddX( 5 );
    m_AttrDetailLayout.AddSubGroupLayout( m_TreeGroupLayout, tree_w, m_AttrDetailLayout.GetRemainY() );

    m_AttrDetailLayout.AddX( tree_w + 5 );
    m_AttrDetailLayout.AddSubGroupLayout( m_CommonEntryLayout, m_AttrDetailLayout.GetW() - tree_w - 15, m_AttrDetailLayout.GetRemainY() );

    m_AttrDetailLayout.SetX( m_AttrDetailLayout.GetStartX() );
    m_AttrDetailLayout.SetY( editor_ht );
    m_AttrDetailLayout.AddSubGroupLayout( m_ResizableLayout, m_AttrDetailLayout.GetW(), m_AttrDetailLayout.GetRemainY());
    m_AttrDetailLayout.GetGroup()->resizable( m_ResizableLayout.GetGroup() );

    m_TreeGroupLayout.ForceNewLine();
    m_TreeGroupLayout.AddYGap();

    m_CommonEntryLayout.ForceNewLine();

    // insert AttrTree GUI
    m_TreeGroupLayout.AddDividerBox( "Attribute Search" );

    bool capitalize = true;
    int w_search_btn = m_TreeGroupLayout.GetW() / 3;

    m_TreeGroupLayout.SetChoiceButtonWidth( 0 );
    m_AttrTypeSearchChoice.AddItem( "All Types" , vsp::INVALID_TYPE );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::BOOL_DATA , capitalize ) , vsp::BOOL_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::INT_DATA, capitalize ), vsp::INT_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::DOUBLE_DATA, capitalize ), vsp::DOUBLE_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::STRING_DATA, capitalize ), vsp::STRING_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::PARM_REFERENCE_DATA, capitalize ), vsp::PARM_REFERENCE_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::VEC3D_DATA, capitalize ), vsp::VEC3D_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::INT_MATRIX_DATA, capitalize ), vsp::INT_MATRIX_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::DOUBLE_MATRIX_DATA, capitalize ), vsp::DOUBLE_MATRIX_DATA );
    m_AttrTypeSearchChoice.AddItem( NameValData::GetTypeName( vsp::ATTR_COLLECTION_DATA, capitalize ), vsp::ATTR_COLLECTION_DATA );

    m_ObjTypeSearchChoice.AddItem( "All Objects", vsp::ATTROBJ_FREE );
    m_ObjTypeSearchChoice.AddItem( "Parm", vsp::ATTROBJ_PARM );
    m_ObjTypeSearchChoice.AddItem( "Geom", vsp::ATTROBJ_GEOM );
    m_ObjTypeSearchChoice.AddItem( "Vehicle", vsp::ATTROBJ_VEH );
    m_ObjTypeSearchChoice.AddItem( "Subsurf", vsp::ATTROBJ_SUBSURF );
    m_ObjTypeSearchChoice.AddItem( "Measure", vsp::ATTROBJ_MEASURE );
    m_ObjTypeSearchChoice.AddItem( "Link", vsp::ATTROBJ_LINK );
    m_ObjTypeSearchChoice.AddItem( "AdvLink", vsp::ATTROBJ_ADVLINK );
    m_ObjTypeSearchChoice.AddItem( "Attribute Groups", vsp::ATTROBJ_ATTR );

    m_TreeGroupLayout.AddInput( m_AttrSearchIn, "Search");

    m_TreeGroupLayout.SetSameLineFlag( true );
    m_TreeGroupLayout.AddButton( m_CaseSensitiveButton, "Case Sensitive");
    m_TreeGroupLayout.AddChoice( m_ObjTypeSearchChoice, "Sort Type" );
    m_TreeGroupLayout.AddChoice( m_AttrTypeSearchChoice, "Sort Object" );

    m_CaseSensitiveButton.SetWidth( w_search_btn );
    m_AttrTypeSearchChoice.SetWidth( w_search_btn );
    m_ObjTypeSearchChoice.SetWidth( w_search_btn );

    m_AttrTypeSearchChoice.SetX( m_TreeGroupLayout.GetStartX() + w_search_btn );
    m_ObjTypeSearchChoice.SetX( m_TreeGroupLayout.GetStartX() + 2 * w_search_btn );

    m_TreeGroupLayout.ForceNewLine();
    m_TreeGroupLayout.SetSameLineFlag( false );

    m_TreeGroupLayout.AddYGap();
    m_TreeGroupLayout.AddDividerBox( "Attribute Tree" );

    int w_node_btn = m_TreeGroupLayout.GetW() / 2;
    m_TreeGroupLayout.SetSameLineFlag( true );

    m_TreeGroupLayout.AddButton(m_CloseTrigger, "[-] Close Inactive Nodes");
    m_TreeGroupLayout.AddButton(m_OpenTrigger, "[+] Open All Nodes");

    m_CloseTrigger.SetWidth( w_node_btn );
    m_OpenTrigger.SetWidth( w_node_btn );

    m_OpenTrigger.SetX( m_TreeGroupLayout.GetStartX() + w_node_btn );

    m_TreeGroupLayout.ForceNewLine();

    m_AttrTreeWidget.Init( mgr, &m_TreeGroupLayout, m_FLTK_Window, this, staticScreenCB, true, m_TreeGroupLayout.GetY(), editor_ht );
    m_TreeGroupLayout.GetGroup()->resizable( m_AttrTreeWidget.GetTreeWidget() );

    m_TreeGroupLayout.SetY( m_TreeGroupLayout.GetY() + editor_ht );

    int w_btn_tree = m_TreeGroupLayout.GetW() / 4;
    m_TreeGroupLayout.AddButton( m_CopyButton, "Copy" );
    m_TreeGroupLayout.AddButton( m_PasteButton, "Paste" );
    m_TreeGroupLayout.AddButton( m_CutButton, "Cut" );
    m_TreeGroupLayout.AddButton( m_DelButton, "Delete" );
    m_CopyButton.SetWidth( w_btn_tree );
    m_CutButton.SetWidth( w_btn_tree );
    m_PasteButton.SetWidth( w_btn_tree );
    m_DelButton.SetWidth( w_btn_tree );

    m_CutButton.SetX( m_TreeGroupLayout.GetStartX() + w_btn_tree );
    m_PasteButton.SetX( m_TreeGroupLayout.GetStartX() + 2 * w_btn_tree );
    m_DelButton.SetX( m_TreeGroupLayout.GetStartX() + 3 * w_btn_tree );

    m_CopyButton.SetShortcut( FL_COMMAND + 'c' , true );
    m_PasteButton.SetShortcut( FL_COMMAND + 'v' , true );
    m_CutButton.SetShortcut( FL_COMMAND + 'x' , true );
    m_DelButton.SetShortcut( FL_Delete , true );

    m_CommonEntryLayout.AddYGap(); //necessary to line up top of section
    m_CommonEntryLayout.AddDividerBox( "Add Attributes" );

    m_CommonEntryLayout.SetSameLineFlag( true );

    m_CommonEntryLayout.SetChoiceButtonWidth( 0 );
    int w_btn = m_CommonEntryLayout.GetW() / 2;
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::BOOL_DATA , capitalize ) , vsp::BOOL_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::INT_DATA, capitalize ), vsp::INT_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::DOUBLE_DATA, capitalize ), vsp::DOUBLE_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::STRING_DATA, capitalize ), vsp::STRING_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::PARM_REFERENCE_DATA, capitalize ), vsp::PARM_REFERENCE_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::VEC3D_DATA, capitalize ), vsp::VEC3D_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::INT_MATRIX_DATA, capitalize ), vsp::INT_MATRIX_DATA );
    m_AttrTypeChoice.AddItem( NameValData::GetTypeName( vsp::DOUBLE_MATRIX_DATA, capitalize ), vsp::DOUBLE_MATRIX_DATA );

    m_CommonEntryLayout.AddChoice( m_AttrTypeChoice, "Type" );
    m_CommonEntryLayout.AddButton( m_AttrAddTrigger, "Add" );

    m_AttrTypeChoice.SetWidth( w_btn );
    m_AttrAddTrigger.SetWidth( w_btn );

    m_AttrAddTrigger.SetX( m_CommonEntryLayout.GetStartX() + w_btn );

    m_CommonEntryLayout.ForceNewLine();
    m_CommonEntryLayout.SetSameLineFlag( false );

    m_CommonEntryLayout.AddButton( m_AttrAddGroupTrigger, "Add Group" );
    m_AttrAddGroupTrigger.SetWidth( w_btn );
    m_AttrAddGroupTrigger.SetX( m_CommonEntryLayout.GetStartX() + w_btn );

    m_CommonEntryLayout.AddYGap();
    m_DataLabel = m_CommonEntryLayout.AddDividerBox( "Attribute Data Entry" );

    m_CommonEntryLayout.AddInput( m_AttrNameIn, "Name" );
    m_CommonEntryLayout.AddInput( m_AttrDescIn, "Desc" );

    // create subgroup with no data entry gui; used only for the Attribute Group
    m_CommonEntryLayout.AddSubGroupLayout( m_EmptyEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );

    // create subgroup for toggling which label is shown in this screen (and to provide for future use of cell-data GUI for matrices/vectors)
    m_CommonEntryLayout.AddSubGroupLayout( m_ToggleEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );

    int toggle_label_width = m_ToggleEntryLayout.GetButtonWidth();
    int toggle_btn_width = m_ToggleEntryLayout.GetStdHeight();

    m_ToggleEntryLayout.SetSameLineFlag( true );

    m_ToggleEntryLayout.AddOutput( m_AttrToggleLabel, "Value" );
    m_ToggleEntryLayout.AddButton( m_AttrDataToggleIn, "" );
    m_ToggleEntryLayout.SetButtonWidth( 0 );
    m_ToggleEntryLayout.AddOutput( m_AttrToggleField, "" );

    m_AttrToggleLabel.SetWidth( toggle_label_width );
    m_AttrDataToggleIn.SetWidth( toggle_btn_width );
    m_AttrToggleField.SetWidth( m_ToggleEntryLayout.GetW() - toggle_label_width - toggle_btn_width );

    m_AttrDataToggleIn.SetX( m_CommonEntryLayout.GetStartX() + toggle_label_width );
    m_AttrToggleField.SetX( m_CommonEntryLayout.GetStartX() + toggle_label_width + toggle_btn_width );
    m_ToggleEntryLayout.SetSameLineFlag( false );
    m_ToggleEntryLayout.ForceNewLine();

    m_ToggleEntryLayout.AddResizeBox(); //sacrificial resizable component, prevents the rest of the layout from squishing

    m_CommonEntryLayout.AddSubGroupLayout( m_InlineEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );
    m_InlineEntryLayout.AddInput( m_InlineDataIn, "Data" );
    m_InlineEntryLayout.AddResizeBox(); //sacrificial resizable component, prevents the rest of the layout from squishing

    m_CommonEntryLayout.AddSubGroupLayout( m_StringEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );
    m_CommonEntryLayout.GetGroup()->resizable( m_StringEntryLayout.GetGroup() );

    // create text editor, and tie its widget callbacks to the staticScreenCB for updating the attribute when called
    bool resizable = true;
    m_DataText = m_StringEntryLayout.AddVspTextEditor( editor_ht, resizable );
    m_DataBuffer = new Fl_Text_Buffer;
    m_DataText->callback( staticScreenCB, this );
    m_DataText->buffer( m_DataBuffer );
    m_DataText->textfont( FL_COURIER );

    m_DataText->remove_key_binding( FL_Enter, FL_TEXT_EDITOR_ANY_STATE );
    m_DataText->remove_key_binding( FL_KP_Enter, FL_TEXT_EDITOR_ANY_STATE );
    m_DataText->add_key_binding( FL_Enter, FL_TEXT_EDITOR_ANY_STATE , VspTextEditor::kf_accept );
    m_DataText->add_key_binding( FL_Enter, FL_SHIFT , Fl_Text_Editor::kf_enter );
    m_DataText->add_key_binding( FL_Enter, FL_CTRL , Fl_Text_Editor::kf_enter );
    m_DataText->add_key_binding( FL_KP_Enter, FL_TEXT_EDITOR_ANY_STATE , VspTextEditor::kf_accept );
    m_DataText->add_key_binding( FL_KP_Enter, FL_SHIFT , Fl_Text_Editor::kf_enter );
    m_DataText->add_key_binding( FL_KP_Enter, FL_CTRL , Fl_Text_Editor::kf_enter );
    m_DataBuffer->text( "" );

    // add parm reference layout
    m_CommonEntryLayout.AddSubGroupLayout( m_ParmRefEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );
    m_ParmRefEntryLayout.AddParmPicker( m_AttrParmPicker );
    m_ParmRefEntryLayout.SetButtonWidth( m_ParmRefEntryLayout.GetW()/3 );
    m_ParmRefEntryLayout.AddInput( m_AttrParmIDInput, "Parm ID" );
    m_ParmRefEntryLayout.SetButtonWidth( m_ParmRefEntryLayout.GetW()/2 );
    m_ParmRefEntryLayout.AddSlider( m_AttrParmSlider, "AUTO_UPDATE", 10., "%6.5f" );

    // enable parm dropping methods
    ( (Vsp_Group*) m_ParmRefEntryLayout.GetGroup() )->SetAllowDrop( true );
    m_ParmRefEntryLayout.GetGroup()->callback( staticScreenCB, this );

    m_ParmRefEntryLayout.AddResizeBox(); //sacrificial resizable component, prevents the rest of the layout from squishing


    // add vec3d layout
    m_CommonEntryLayout.AddSubGroupLayout( m_Vec3dEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );
    m_Vec3dSpreadSingle = m_Vec3dEntryLayout.AddSpreadSheet < vec3d >( editor_ht - 2*m_Vec3dEntryLayout.GetStdHeight()  );
    m_Vec3dEntryLayout.GetGroup()->resizable( m_Vec3dSpreadSingle );
    m_Vec3dSpreadSingle->set_HeaderOffset( 'X' - 'A' );
    m_Vec3dSpreadSingle->set_ChangeCallback( staticScreenCB, this );

    int w_vec3d_btn = m_Vec3dEntryLayout.GetW() / 4;

    m_Vec3dEntryLayout.AddButton( m_AttrVec3dRowAdd , "Add Row" );
    m_Vec3dEntryLayout.AddButton( m_AttrVec3dRowDel , "Del Row" );
    m_AttrVec3dRowAdd.SetWidth( w_vec3d_btn );
    m_AttrVec3dRowDel.SetWidth( w_vec3d_btn );

    m_Vec3dEntryLayout.ForceNewLine();

    // add int & double matrix layouts
    m_CommonEntryLayout.AddSubGroupLayout( m_IntMatEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );
    m_IntMatrixSpreadSheet = m_IntMatEntryLayout.AddSpreadSheet < vector < int > >( editor_ht - 2*m_IntMatEntryLayout.GetStdHeight()  );
    m_IntMatEntryLayout.GetGroup()->resizable( m_IntMatrixSpreadSheet );
    m_IntMatrixSpreadSheet->set_ChangeCallback( staticScreenCB, this );

    int w_imat_btn = m_IntMatEntryLayout.GetW() / 4;

    m_IntMatEntryLayout.SetSameLineFlag( true );
    m_IntMatEntryLayout.AddButton( m_AttrImatRowAdd , "Add Row" );
    m_IntMatEntryLayout.AddButton( m_AttrImatColAdd , "Add Col" );
    m_IntMatEntryLayout.ForceNewLine();
    m_IntMatEntryLayout.AddButton( m_AttrImatRowDel , "Del Row" );
    m_IntMatEntryLayout.AddButton( m_AttrImatColDel , "Del Col" );

    m_AttrImatRowAdd.SetWidth( w_imat_btn );
    m_AttrImatRowDel.SetWidth( w_imat_btn );
    m_AttrImatColAdd.SetWidth( w_imat_btn );
    m_AttrImatColDel.SetWidth( w_imat_btn );
    m_AttrImatColAdd.SetX( m_CommonEntryLayout.GetStartX() + w_imat_btn );
    m_AttrImatColDel.SetX( m_CommonEntryLayout.GetStartX() + w_imat_btn );

    m_IntMatEntryLayout.ForceNewLine();

    m_CommonEntryLayout.AddSubGroupLayout( m_DblMatEntryLayout, m_CommonEntryLayout.GetW(), m_CommonEntryLayout.GetRemainY() );
    m_DoubleMatrixSpreadSheet = m_DblMatEntryLayout.AddSpreadSheet < vector < double > >( editor_ht - 2*m_DblMatEntryLayout.GetStdHeight() );
    m_DblMatEntryLayout.GetGroup()->resizable( m_DoubleMatrixSpreadSheet );
    m_DoubleMatrixSpreadSheet->set_ChangeCallback( staticScreenCB, this );

    int w_dmat_btn = m_DblMatEntryLayout.GetW() / 4;

    m_DblMatEntryLayout.SetSameLineFlag( true );
    m_DblMatEntryLayout.AddButton( m_AttrDmatRowAdd , "Add Row" );
    m_DblMatEntryLayout.AddButton( m_AttrDmatColAdd , "Add Col" );
    m_DblMatEntryLayout.ForceNewLine();
    m_DblMatEntryLayout.AddButton( m_AttrDmatRowDel , "Del Row" );
    m_DblMatEntryLayout.AddButton( m_AttrDmatColDel , "Del Col" );

    m_AttrDmatRowAdd.SetWidth( w_dmat_btn );
    m_AttrDmatRowDel.SetWidth( w_dmat_btn );
    m_AttrDmatColAdd.SetWidth( w_dmat_btn );
    m_AttrDmatColDel.SetWidth( w_dmat_btn );
    m_AttrDmatColAdd.SetX( m_CommonEntryLayout.GetStartX() + w_dmat_btn );
    m_AttrDmatColDel.SetX( m_CommonEntryLayout.GetStartX() + w_dmat_btn );

    m_DblMatEntryLayout.ForceNewLine();

    //initialize pointers etc.
    m_CurAttrGroup = nullptr;

    Vehicle* veh = VehicleMgr.GetVehicle();
    m_AttrBoolParmPtr = &veh->m_AttrBoolButtonParm;
    m_CaseSensParmPtr = &veh->m_AttrCaseSensitivity;

    m_AttrTypeChoice.SetVal( vsp::STRING_DATA );
    m_AttrTypeSearchChoice.SetVal( vsp::INVALID_TYPE );
    m_ObjTypeSearchChoice.SetVal( vsp::ATTROBJ_FREE );

    m_valid_collector_set = false;
    m_types_selected = 0;
}

AttributeExplorer::~AttributeExplorer()
{
    m_DataText->buffer( NULL );
    delete m_DataBuffer;
}

bool AttributeExplorer::Update()
{

    // populate Tree with empty branch IDs
    GetEmptyColls();

    //==== Update Parm Picker ====//
    m_AttrParmPicker.Update();

    // Assign current Attribute AttachID, AttachType, AttributeName, and AttributeDataPtr
    // Update the Attribute GUI's pointer to the appropriate AttributeData object
    m_CaseSensitiveButton.Update( m_CaseSensParmPtr->GetID() );
    m_AttrDataToggleIn.Update( m_AttrBoolParmPtr->GetID() );
    if ( m_AttrBoolParmPtr->Get() )
    {
        m_AttrToggleField.Update( "True" );
    }
    else
    {
        m_AttrToggleField.Update( "False" );
    }

    m_AttrTreeWidget.SetSearchTerms( m_AttrTypeSearchChoice.GetVal(), m_ObjTypeSearchChoice.GetVal(), m_AttrSearchIn.GetString(), m_CaseSensParmPtr->Get() );
    m_AttrTreeWidget.Update();

    SetAttrData();
    UpdateAttrFields();

    return true;
};

void AttributeExplorer::SetAttrData()
{
    m_AttrIDs = m_AttrTreeWidget.GetTreeAttrID();
    m_CollIDs = m_AttrTreeWidget.GetTreeCollID();

    AttributeCollection* ac_ptr = nullptr;

    m_valid_collector_set = !( m_CollIDs.empty() );
    for ( int i = 0; i != m_CollIDs.size(); ++i )
    {
        ac_ptr = AttributeMgr.GetCollectionPtr( m_CollIDs.at( i ) );
        if ( !ac_ptr )
        {
            m_valid_collector_set = false;
        }
    }

    m_types_selected = NumAttrTypes();
};

void AttributeExplorer::AddEmptyCollID( vector < string > coll_ids )
{
    m_AttrTreeWidget.AddEmptyCollID( coll_ids );
};

void AttributeExplorer::GetEmptyColls()
{
    vector < VspScreen* > screen_vec = m_ScreenMgr->GetAllScreens();

    vector < string > coll_id_vec;

    for ( int i = 0; i != screen_vec.size(); ++i )
    {
        if ( screen_vec[i]->IsShown() )
        {
            screen_vec[i]->GetCollIDs( coll_id_vec );
        }

        if ( screen_vec[i]->GetScreenType() == vsp::VSP_MANAGE_GEOM_SCREEN )
        {
            vector < VspScreen* > geom_screen_vec = static_cast< ManageGeomScreen* >( screen_vec[i] )->GetGeomScreenVec();

            for ( int j = 0; j != geom_screen_vec.size(); ++j )
            {
                if ( geom_screen_vec[j]->IsShown() )
                {
                    geom_screen_vec[j]->GetCollIDs( coll_id_vec );
                }
            }
        }
    }

    AddEmptyCollID( coll_id_vec );
}
int AttributeExplorer::NumAttrTypes()
{
    NameValData* attr_ptr = nullptr;

    int num_attr_types = 0;
    int current_type = -2;
    bool type_in_vec = false;
    vector < int > attr_type_vec;

    for ( int i = 0; i != m_AttrIDs.size(); ++ i )
    {
        type_in_vec = false;
        current_type = -2;
        attr_ptr = AttributeMgr.GetAttributePtr( m_AttrIDs.at( i ) );

        if ( attr_ptr )
        {
            current_type = attr_ptr->GetType();
            for ( int j = 0; j != attr_type_vec.size(); ++j )
            {
                if ( attr_type_vec.at( j ) == current_type )
                {
                    type_in_vec = true;
                }
            }
            if ( !type_in_vec )
            {
                attr_type_vec.push_back( attr_ptr->GetType() );
                num_attr_types++;
            }
        }
    }
    return num_attr_types;
}

void AttributeExplorer::UpdateAttrFields()
{
    bool attrDataBool = false;
    int attrDataInt;
    double attrDataDbl;
    string attrDataStr = "";
    string attrDescStr = "";

    string attrParmIDStr = "";
    string attrParmNameStr = "";
    string attrParmValStr = "";

    string attrBufferText = "";
    string attrInlineText = "";

    string name_str = "";

    //default to string data
    int attr_type = -2;

    // only update if just one selected? Or if all are the same type??

    GroupLayout* group_choice = nullptr;
    if ( m_types_selected == 1 )
    {
        int attr_type_multi = AttributeMgr.GetAttributePtr( m_AttrIDs.front() )->GetType();
        switch ( attr_type_multi )
        {
        case vsp::BOOL_DATA:
            group_choice = &m_ToggleEntryLayout;
            break;
        case vsp::STRING_DATA:
            group_choice = &m_StringEntryLayout;
            break;
        case vsp::PARM_REFERENCE_DATA:
            group_choice = &m_ParmRefEntryLayout;
            break;
        case vsp::INT_DATA:
            group_choice = &m_InlineEntryLayout;
            break;
        case vsp::DOUBLE_DATA:
            group_choice = &m_InlineEntryLayout;
            break;
        case vsp::ATTR_COLLECTION_DATA:
            group_choice = &m_EmptyEntryLayout;
            break;
        case vsp::VEC3D_DATA:
            group_choice = &m_Vec3dEntryLayout;
            break;
        case vsp::INT_MATRIX_DATA:
            group_choice = &m_IntMatEntryLayout;
            break;
        case vsp::DOUBLE_MATRIX_DATA:
            group_choice = &m_DblMatEntryLayout;
            break;
        default:
            group_choice = nullptr;
            break;
        }
    }

    // if single attribute selected, populate editor fields

    NameValData* attr_ptr = nullptr;
    AttributeCollection* ac_ptr = nullptr;

    if ( m_AttrIDs.size() == 1 && m_CollIDs.size() == 1 )
    {
        NameValData* attr_ptr = AttributeMgr.GetAttributePtr( m_AttrIDs.front() );
        AttributeCollection* ac_ptr = AttributeMgr.GetCollectionPtr( m_CollIDs.front() );

        if ( attr_ptr && ac_ptr )
        {
            name_str = attr_ptr->GetName();
            attr_type = attr_ptr->GetType();

            attrDataBool = attr_ptr->GetBool( 0 );
            attrDataInt = attr_ptr->GetInt( 0 );
            attrDataDbl = attr_ptr->GetDouble( 0 );
            attrDataStr = attr_ptr->GetString( 0 );

            attrDescStr = attr_ptr->GetDoc();

            // check for row/col naming attributes
            if ( attr_type == vsp::VEC3D_DATA
            || attr_type == vsp::INT_MATRIX_DATA
            || attr_type == vsp::DOUBLE_MATRIX_DATA )
            {
                NameValData* col_attr = ac_ptr->FindPtr( name_str+"_col" );
                NameValData* row_attr = ac_ptr->FindPtr( name_str+"_row" );

                if ( col_attr && col_attr->GetType() == vsp::STRING_DATA )
                {
                    m_Vec3dSpreadSingle->set_col_header_txt( col_attr->GetString( 0 ) );
                    m_IntMatrixSpreadSheet->set_col_header_txt( col_attr->GetString( 0 ) );
                    m_DoubleMatrixSpreadSheet->set_col_header_txt( col_attr->GetString( 0 ) );
                }
                else
                {
                    m_Vec3dSpreadSingle->set_col_user_header_flag( false );
                    m_IntMatrixSpreadSheet->set_col_user_header_flag( false );
                    m_DoubleMatrixSpreadSheet->set_col_user_header_flag( false );
                }

                if ( row_attr && row_attr->GetType() == vsp::STRING_DATA )
                {
                    m_Vec3dSpreadSingle->set_row_header_txt( row_attr->GetString( 0 ) );
                    m_IntMatrixSpreadSheet->set_row_header_txt( row_attr->GetString( 0 ) );
                    m_DoubleMatrixSpreadSheet->set_row_header_txt( row_attr->GetString( 0 ) );
                }
                else
                {
                    m_Vec3dSpreadSingle->set_row_user_header_flag( false );
                    m_IntMatrixSpreadSheet->set_row_user_header_flag( false );
                    m_DoubleMatrixSpreadSheet->set_row_user_header_flag( false );
                }
            }

            // populate appropriate fields based on attr type
            switch ( attr_type )
            {
            case vsp::BOOL_DATA:
                m_AttrBoolParmPtr->Set( attrDataBool );
                break;
            case vsp::STRING_DATA:
                attrBufferText = attrDataStr;
                break;
            case vsp::PARM_REFERENCE_DATA:
                {
                    attrParmIDStr = attr_ptr->GetParmID( 0 );
                    Parm* p = ParmMgr.FindParm( attr_ptr->GetParmID( 0 ) );
                    attrParmNameStr = string("");
                    if ( p )
                    {
                        string pc_name = p->GetContainer()->GetName();
                        string grp_name = p->GetGroupName();
                        string p_name = p->GetName();
                        attrParmNameStr = pc_name + " | " + grp_name + " | " + p_name;
                    }
                    m_AttrParmPicker.SetParmChoice( attr_ptr->GetParmID( 0 ) );
                    m_AttrParmPicker.Update();
                }
                break;
            case vsp::INT_DATA:
                attrInlineText = to_string( attrDataInt );
                break;
            case vsp::DOUBLE_DATA:
                attrInlineText = to_string( attrDataDbl );
                break;
            case vsp::ATTR_COLLECTION_DATA:
                break;
            case vsp::VEC3D_DATA:
                m_Vec3dSpreadSingle->set_data( &( attr_ptr->GetVec3dData() ) );
                break;
            case vsp::INT_MATRIX_DATA:
                m_IntMatrixSpreadSheet->set_data( &( attr_ptr->GetIntMatData() ) );
                break;
            case vsp::DOUBLE_MATRIX_DATA:
                m_DoubleMatrixSpreadSheet->set_data( &( attr_ptr->GetDoubleMatData() ) );
                break;
            }
        }
    }

    if ( m_valid_collector_set )
    {
        m_AttrAddTrigger.Activate();
        m_AttrAddGroupTrigger.Activate();
        m_PasteButton.Activate();
    }
    else
    {
        m_AttrAddTrigger.Deactivate();
        m_AttrAddGroupTrigger.Deactivate();
        m_PasteButton.Deactivate();
    }

    // update attribute fields
    m_AttrNameIn.Update( name_str );
    m_DataBuffer->text( attrBufferText.c_str() );
    m_InlineDataIn.Update( attrInlineText.c_str() );

    m_AttrParmSlider.Update( attrParmIDStr );
    m_AttrParmIDInput.Update( attrParmIDStr );

    m_AttrDescIn.Update( attrDescStr.c_str() );

    // toggle copy/paste/edit controls based on validity of attribute selection
    bool valid_attrs = !( m_AttrIDs.empty() );
    for ( int i = 0; i != m_AttrIDs.size(); ++i )
    {
        NameValData* a = AttributeMgr.GetAttributePtr( m_AttrIDs.at( i ) );
        if ( !( a && !a->IsProtected() ) )
        {
            valid_attrs = false;
        }
    }

    if ( !valid_attrs )
    {
        m_AttrNameIn.Deactivate();
        m_AttrDescIn.Deactivate();
        m_DelButton.Deactivate();

        m_AttrParmPicker.Deactivate();
        m_AttrVec3dRowAdd.Deactivate();
        m_AttrVec3dRowDel.Deactivate();
        m_AttrDmatRowAdd.Deactivate();
        m_AttrDmatRowDel.Deactivate();
        m_AttrDmatColAdd.Deactivate();
        m_AttrDmatColDel.Deactivate();
        m_AttrImatRowAdd.Deactivate();
        m_AttrImatRowDel.Deactivate();
        m_AttrImatColAdd.Deactivate();
        m_AttrImatColDel.Deactivate();

        m_CutButton.Deactivate();
        m_CopyButton.Deactivate();
    }
    else
    {
        m_AttrNameIn.Activate();
        m_AttrDescIn.Activate();
        m_DelButton.Activate();

        m_AttrParmPicker.Activate();
        m_AttrVec3dRowAdd.Activate();
        m_AttrVec3dRowDel.Activate();
        m_AttrDmatRowAdd.Activate();
        m_AttrDmatRowDel.Activate();
        m_AttrDmatColAdd.Activate();
        m_AttrDmatColDel.Activate();
        m_AttrImatRowAdd.Activate();
        m_AttrImatRowDel.Activate();
        m_AttrImatColAdd.Activate();
        m_AttrImatColDel.Activate();

        m_CutButton.Activate();
        m_CopyButton.Activate();
    }

    AttrTypeDispGroup( attr_type , group_choice );
}

// only used by double click opening of attribute explorer from the inline gui
void AttributeExplorer::SetTreeAutoSelectID( const string & id )
{
    m_AttrTreeWidget.SetAutoSelectID( { id } );
}

void AttributeExplorer::AttributeAdd()
{
    AttributeAdd( m_AttrTypeChoice.GetVal() );
}

void AttributeExplorer::AttributeAdd( int attrAddType )
{
    // do for every currently selected collection and group id...

    vector < string > new_ids;

    AttributeCollection* ac_ptr;

    for ( int i = 0; i != m_CollIDs.size(); ++i )
    {
        ac_ptr = AttributeMgr.GetCollectionPtr( m_CollIDs.at( i ) );
        if ( ac_ptr )
        {
            new_ids.push_back( AttributeMgr.GuiAddAttribute( ac_ptr, attrAddType ) );
        }
    }
    m_AttrTreeWidget.SetAutoSelectID( new_ids );
}

void AttributeExplorer::AttributeModify( GuiDevice* gui_device, Fl_Widget *w )
{
    NameValData* attr_ptr = nullptr;
    string attr_id = "NONE";
    int attrType = -1;

    if ( m_types_selected > 0 )
    {
        for ( int i = 0; i != m_AttrIDs.size(); ++ i )
        {
            attr_ptr = AttributeMgr.GetAttributePtr( m_AttrIDs.at( i ) );
            //AttributeModify will detect if the attribute has changed to a new & valid attribute, and create that attribute in the place of the old one
            if ( attr_ptr )
            {
                attr_id = attr_ptr->GetID();
                attrType = attr_ptr->GetType();

                //rename
                if ( gui_device == &m_AttrNameIn )
                {
                    AttributeMgr.SetAttributeName( attr_id, m_AttrNameIn.GetString() );
                }

                //change description
                else if ( gui_device == &m_AttrDescIn )
                {
                    AttributeMgr.SetAttributeDoc( attr_id, m_AttrDescIn.GetString() );
                }

                //modify bool data
                else if ( gui_device == &m_AttrDataToggleIn && attrType == vsp::BOOL_DATA )
                {
                    AttributeMgr.SetAttributeBool( attr_id, m_AttrBoolParmPtr->Get() );
                }

                //modify int data
                else if ( gui_device == &m_InlineDataIn && attrType == vsp::INT_DATA )
                {
                    if ( AttributeEditor::canMakeInt( m_InlineDataIn.GetString() ) )
                    {
                        AttributeMgr.SetAttributeInt( attr_id, stoi( m_InlineDataIn.GetString() ) );
                    }
                }

                //modify double data
                else if ( gui_device == &m_InlineDataIn && attrType == vsp::DOUBLE_DATA )
                {
                    if ( AttributeEditor::canMakeDbl( m_InlineDataIn.GetString() ) )
                    {
                        AttributeMgr.SetAttributeDouble( attr_id, stod( m_InlineDataIn.GetString() ) );
                    }
                }

                //modify string data
                else if ( static_cast<Fl_Text_Editor * >( w ) == m_DataText && attrType == vsp::STRING_DATA )
                {
                    AttributeMgr.SetAttributeString( attr_id, m_DataBuffer->text() );
                }

                //modify parm id data
                else if ( ( gui_device == &m_AttrParmIDInput || gui_device == &m_AttrParmPicker || w == m_ParmRefEntryLayout.GetGroup() ) && attrType == vsp::PARM_REFERENCE_DATA )
                {
                    // string p_id = ( gui_device == &m_AttrParmPicker ) ? m_AttrParmPicker.GetParmChoice() : Fl::event_text();
                    string p_id;

                    if ( gui_device == &m_AttrParmIDInput )
                    {
                        p_id = m_AttrParmIDInput.GetString();
                    }
                    else if ( gui_device == &m_AttrParmPicker )
                    {
                        p_id = m_AttrParmPicker.GetParmChoice();
                    }
                    else
                    {
                        p_id = Fl::event_text();
                    }

                    AttributeMgr.SetAttributeParmID( attr_id, p_id );

                    Parm* p = ParmMgr.FindParm( p_id );
                    if ( p )
                    {
                        m_AttrParmIDInput.Update( p_id );

                        double parm_range = ( abs( p->Get() ) > 0.00001 ) ? p->Get()*0.5 : 10.;

                        m_AttrParmSlider.SetRange( parm_range );
                    }
                }

                //vec3d & matrix data modification already done via the spreadsheet widgets

                //vec3d resizing
                else if ( gui_device == &m_AttrVec3dRowAdd && attrType == vsp::VEC3D_DATA )
                {
                    vector < vec3d > * attrVec3dPtr = &attr_ptr->GetVec3dData();
                    ResizeVector( attrVec3dPtr, 1, vec3d(0.,0.,0.) );
                }

                else if ( gui_device == &m_AttrVec3dRowDel && attrType == vsp::VEC3D_DATA )
                {
                    vector < vec3d > * attrVec3dPtr = &attr_ptr->GetVec3dData();
                    ResizeVector( attrVec3dPtr, -1, vec3d(0.,0.,0.) );
                }

                //int matrix resizing
                else if ( gui_device == &m_AttrImatRowAdd && attrType == vsp::INT_MATRIX_DATA )
                {
                    vector < vector < int > > * attrImatPtr = &attr_ptr->GetIntMatData();
                    ResizeMat( attrImatPtr, {1, 0}, 0 );
                }

                else if ( gui_device == &m_AttrImatRowDel && attrType == vsp::INT_MATRIX_DATA )
                {
                    vector < vector < int > > * attrImatPtr = &attr_ptr->GetIntMatData();
                    ResizeMat( attrImatPtr, {-1, 0}, 0 );
                }

                else if ( gui_device == &m_AttrImatColAdd && attrType == vsp::INT_MATRIX_DATA )
                {
                    vector < vector < int > > * attrImatPtr = &attr_ptr->GetIntMatData();
                    ResizeMat( attrImatPtr, {0, 1}, 0 );
                }

                else if ( gui_device == &m_AttrImatColDel && attrType == vsp::INT_MATRIX_DATA )
                {
                    vector < vector < int > > * attrImatPtr = &attr_ptr->GetIntMatData();
                    ResizeMat( attrImatPtr, {0, -1}, 0 );
                }

                //double matrix resizing
                else if ( gui_device == &m_AttrDmatRowAdd && attrType == vsp::DOUBLE_MATRIX_DATA )
                {
                    vector < vector < double > > * attrDmatPtr = &attr_ptr->GetDoubleMatData();
                    ResizeMat( attrDmatPtr, {1, 0}, 0. );
                }

                else if ( gui_device == &m_AttrDmatRowDel && attrType == vsp::DOUBLE_MATRIX_DATA )
                {
                    vector < vector < double > > * attrDmatPtr = &attr_ptr->GetDoubleMatData();
                    ResizeMat( attrDmatPtr, {-1, 0}, 0. );
                }

                else if ( gui_device == &m_AttrDmatColAdd && attrType == vsp::DOUBLE_MATRIX_DATA )
                {
                    vector < vector < double > > * attrDmatPtr = &attr_ptr->GetDoubleMatData();
                    ResizeMat( attrDmatPtr, {0, 1}, 0. );
                }

                else if ( gui_device == &m_AttrDmatColDel && attrType == vsp::DOUBLE_MATRIX_DATA )
                {
                    vector < vector < double > > * attrDmatPtr = &attr_ptr->GetDoubleMatData();
                    ResizeMat( attrDmatPtr, {0, -1}, 0. );
                }

                AttributeMgr.SetAttrDirtyFlag( attr_ptr->GetID() );
            }
        }
    }
}

// Control which GUI group is shown/hidden for attribute edit window, and set data label descriptor
void AttributeExplorer::AttrTypeDispGroup( int attr_type, GroupLayout * group )
{
    // rename data entry label
    bool cap = true;
    bool short_name = false;

    string data_name;

    string header_str = "Attribute Explorer";

    if ( m_types_selected == 1 )
    {
        NameValData* nvd_temp = AttributeMgr.GetAttributePtr( m_AttrIDs.front() );
        if ( nvd_temp )
        {
            attr_type = nvd_temp->GetType();
        }

        data_name = NameValData::GetTypeName( attr_type, cap, short_name );

        if ( data_name.size() )
        {
            data_name += " ";
        }
        data_name += "Data";
    }
    else if ( m_types_selected > 1 )
    {
        data_name = "Multiple Data Types";
    }
    else
    {
        data_name = "No Attributes Selected";
    }

    NameValData* attr_ptr = nullptr;
    AttributeCollection* ac_ptr = nullptr;
    if ( m_AttrIDs.size() == 1 && m_CollIDs.size() == 1 )
    {
        attr_ptr = AttributeMgr.GetAttributePtr( m_AttrIDs.front() );
        ac_ptr = AttributeMgr.GetCollectionPtr( m_CollIDs.front() );
    }

    if ( attr_ptr )
    {
        pair < int, int > row_col_size = { 0, 0 };

        if ( attr_ptr->GetType() == vsp::INT_MATRIX_DATA )
        {
            row_col_size = GetMatSize( &(attr_ptr->GetIntMatData()) );
        }
        else if ( attr_ptr->GetType() == vsp::DOUBLE_MATRIX_DATA )
        {
            row_col_size = GetMatSize( &(attr_ptr->GetDoubleMatData()) );
        }
        if ( row_col_size.first && row_col_size.second )
        {
            data_name += " : " + to_string(row_col_size.first) + " x " + to_string(row_col_size.second);
        }
    }

    m_DataLabel->copy_label( data_name.c_str() );

    // set name of explorer to current attr collection
    if ( ac_ptr )
    {
        header_str += " : ";
        header_str += AttributeMgr.GetName( ac_ptr->GetAttachID() );
    }
    SetTitle(header_str);

    // select active GUI group by datatype
    if ( m_CurAttrGroup == group && group )
    {
        return;
    }

    m_EmptyEntryLayout.Hide();
    m_ToggleEntryLayout.Hide();
    m_InlineEntryLayout.Hide();
    m_StringEntryLayout.Hide();
    m_ParmRefEntryLayout.Hide();
    m_Vec3dEntryLayout.Hide();
    m_IntMatEntryLayout.Hide();
    m_DblMatEntryLayout.Hide();

    m_CurAttrGroup = group;

    if ( group )
    {
        group->Show();
    }

}

void AttributeExplorer::CallBack( Fl_Widget *w )
{
    // if either text editor is called, run AttributeModify without changing the name
    if ( static_cast<Fl_Text_Editor * >( w ) == m_DataText )
    {
        AttributeModify( nullptr, m_DataText );
    }
    else if ( w == m_Vec3dSpreadSingle
           || w == m_IntMatrixSpreadSheet
           || w == m_DoubleMatrixSpreadSheet )
    {
        AttributeModify( nullptr, w );
    }
    else if ( w == ( m_ParmRefEntryLayout.GetGroup() ) && Fl::event_inside( w ) )
    {
        if( Fl::event() == FL_PASTE || Fl::event() == FL_DND_RELEASE )
        {
            AttributeModify( nullptr, w );
        }
    }

    if ( w == m_AttrTreeWidget.GetTreeWidget() )
    {
        m_AttrTreeWidget.ClearRedrawFlag();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void AttributeExplorer::GuiDeviceCallBack( GuiDevice* gui_device )
{
    // if name input is called, run AttributeModify with a name-change gui device input
    if ( gui_device == &m_AttrAddTrigger )
    {
        AttributeAdd();
    }
    else if ( gui_device == &m_AttrAddGroupTrigger )
    {
        AttributeAdd( vsp::ATTR_COLLECTION_DATA );
    }
    else if ( gui_device == &m_DelButton )
    {
        AttributeMgr.DeleteAttribute( m_AttrIDs );
        m_AttrIDs.clear();
    }
    else if ( gui_device == &m_OpenTrigger )
    {
        m_AttrTreeWidget.SetAllNodes( true );
    }
    else if ( gui_device == &m_CloseTrigger )
    {
        m_AttrTreeWidget.SetAllNodes( false );
    }
    else if ( gui_device == &m_CutButton )
    {
        AttributeMgr.CutAttributeUtil( m_AttrIDs );
    }
    else if ( gui_device == &m_CopyButton )
    {
        AttributeMgr.CopyAttributeUtil( m_AttrIDs );
    }
    else if ( !m_CollIDs.empty() && ( gui_device == &m_PasteButton ) )
    {
        AttributeMgr.PasteAttributeUtil( m_CollIDs );
    }
    else
    {
        AttributeModify( gui_device, nullptr );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}