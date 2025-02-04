//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AttributeManager.cpp: Manage attributes for user-controlled auxiliary data
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

// here we need the attribute objects to live. Define the classes and store their data.

#include "AttributeManager.h"
#include "ResultsMgr.h"
#include "APIDefines.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "XSec.h"

#include "VehicleMgr.h"
#include "LinkMgr.h"
#include "AdvLinkMgr.h"
#include "SubSurfaceMgr.h"
#include "MeasureMgr.h"
#include "VarPresetMgr.h"

#include "IDMgr.h"

#include <iomanip>
#include <sstream>

//==== Attribute Manager ====//
AttributeMgrSingleton::AttributeMgrSingleton()
{
    m_AttrClipboard.clear();
    m_DirtyFlags.resize( vsp::NUM_ATTR_EVENT_GROUPS, false );
}

AttributeMgrSingleton::~AttributeMgrSingleton()
{
    Wype();
}

void AttributeMgrSingleton::Wype()
{
    m_AttrClipboard.clear();

    m_AttrPtrMap.clear();

    m_DirtyFlags.clear();
    m_DirtyFlags.resize( vsp::NUM_ATTR_EVENT_GROUPS, false );
}


void AttributeMgrSingleton::SetAttrDirtyFlag( const string &attrID )
{
    NameValData* attr_ptr = GetAttributePtr( attrID );
    if ( attr_ptr )
    {
        int f = attr_ptr->GetAttributeEventGroup();
        SetDirtyFlag( f );
    }
}

void AttributeMgrSingleton::SetDirtyFlag( int f )
{
    if ( f >= 0 && f < vsp::NUM_ATTR_EVENT_GROUPS )
    {
        m_DirtyFlags[f] = true;
    }
}

bool AttributeMgrSingleton::GetDirtyFlag( int f )
{
    if ( f >= 0 && f < vsp::NUM_ATTR_EVENT_GROUPS )
    {
        return m_DirtyFlags[f];
    }
    return false;
}

void AttributeMgrSingleton::ClearDirtyFlag( int f )
{
    if ( f >= 0 && f < vsp::NUM_ATTR_EVENT_GROUPS )
    {
        m_DirtyFlags[f] = false;
    }
}


void AttributeMgrSingleton::Update()
{

    if ( m_DirtyFlags[ vsp::ATTR_GROUP_WATERMARK] )
    {
        // Normally handled here.  However, this is handled as a pull from
        // VspGlWindow::update()
        // Because vsp_graphic can access geom_core and not the other way around.


        // m_DirtyFlags[ vsp::ATTR_GROUP_WATERMARK] = false;
    }

}

void AttributeMgrSingleton::RegisterCollID( const string & collID, AttributeCollection* ac_ptr )
{
    if ( !m_AttrCollMap.count( collID ) )
    {
        m_AttrCollMap.insert( {collID, ac_ptr} );

        vector < AttributeCollection* > coll_vec;
        m_AttrCollMap.at( collID )->BuildCollectorVec( coll_vec );

        for ( int i = 0; i != coll_vec.size(); ++i )
        {
            if ( m_AttrCollMap.at( collID ) != coll_vec[i] )
            {
                AttributeMgr.RegisterCollID( coll_vec[i]->GetID(), coll_vec[i] );
            }
        }
    }
    else
    {
        m_AttrCollMap[ collID ] = ac_ptr;
    }
}

void AttributeMgrSingleton::DeregisterCollID( const string & collID )
{
    if ( m_AttrCollMap.count( collID ) )
    {
        vector < AttributeCollection* > coll_vec;
        m_AttrCollMap.at( collID )->BuildCollectorVec( coll_vec );

        for ( int i = 0; i != coll_vec.size(); ++i )
        {
            if ( m_AttrCollMap.at( collID ) != coll_vec[i] )
            {
                AttributeMgr.DeregisterCollID( coll_vec[i]->GetID() );
            }
        }

        m_AttrCollMap.erase( collID );
    }
}

void AttributeMgrSingleton::RegisterAttrID( const string &attrID, NameValData* attr )
{
    if ( !m_AttrPtrMap.count( attrID ) )
    {
        m_AttrPtrMap.insert( { attrID, attr } );
    }
    else
    {
        m_AttrPtrMap[attrID] = attr;
    }

    if ( attr->GetType() == vsp::ATTR_COLLECTION_DATA )
    {
        AttributeCollection* ac = attr->GetAttributeCollectionPtr( 0 );
        if ( ac )
        {
            vector < NameValData* > attr_vec = ac->GetAllPtrs();
            for ( int i = 0; i != attr_vec.size(); ++i )
            {
                RegisterAttrID( attr_vec[i]->GetID(), attr_vec[i] );
            }
        }
    }
}

void AttributeMgrSingleton::DeregisterAttrID( const string &attrID )
{
    if ( m_AttrPtrMap.count( attrID ) )
    {
        NameValData* attr = m_AttrPtrMap.at( attrID );
        if ( attr->GetType() == vsp::ATTR_COLLECTION_DATA )
        {
            AttributeCollection* ac = attr->GetAttributeCollectionPtr( 0 );
            if ( ac )
            {
                vector < NameValData* > attr_vec = ac->GetAllPtrs();
                for ( int i = 0; i != attr_vec.size(); ++i )
                {
                    DeregisterAttrID( attr_vec[i]->GetID() );
                }
            }
        }

        m_AttrPtrMap.erase( attrID );
    }
}

string AttributeMgrSingleton::SummarizeAttributes()
{
    string text_str = "";
    string attr_id;
    NameValData* attr;

    vector < vector < vector < string > > > tree_id_coll_vecs = AttributeMgr.GetAttrTreeVec();

    //vector to all attribute IDs
    vector < vector < string > > tree_id_vecs = tree_id_coll_vecs[0];


    text_str += "AttrID\tAttrName\tAttrType\tAttrData\tAttrDesc\tAttrPath\n";
    for ( int i = 0; i != tree_id_vecs.size(); ++i )
    {
        attr = GetAttributePtr( tree_id_vecs[i].back() );

        if ( attr )
        {
            text_str += attr->GetID();
            text_str += '\t';
            text_str += attr->GetName();
            text_str += '\t';
            text_str += attr->GetTypeName();
            text_str += '\t';
            text_str += attr->GetAsString( true );
            text_str += '\t';
            text_str += attr->GetDoc();
            text_str += '\t';
        }
        else
        {
            text_str += "-\t-\t-\t-\t-\t";
        }

        for ( int j = 0; j != tree_id_vecs[i].size(); ++j )
        {
            text_str += GetName(tree_id_vecs[i][j] , true );

            //tab delimited between entries
            if ( j < tree_id_vecs[i].size()-1 )
            {
                text_str += '\t';
            }
        }

        //newline delimited between vectors
        if ( i < tree_id_vecs.size()-1 )
        {
            text_str += '\n';
        }
    }

    return text_str;
}

string TextTreeNode::PrintMe( string prior_lines )
{
    // 1. print self name
    // 2. print self name of first child
    // 3. print self name of remaining kids
    int max_id_len = 11;

    // each print self name calls PrintMe
    string tree_text = prior_lines;
    if ( m_Depth )
    {
        for ( int i = 0; i != ( m_IndentQty*m_Depth - 1 ); ++ i )
        {
            tree_text += '-';
        }
        if ( m_Depth )
        tree_text += ' ';
    }

    if ( m_ID != m_Name )
    {
        tree_text += m_ID;
        if ( max_id_len > m_ID.size() )
        {
            for ( int i = m_ID.size() ; i != max_id_len; ++i )
            {
                tree_text += ' ';
            }
        }
        tree_text += ' ';
        tree_text += m_Name;
        tree_text += '\n';
    }
    else
    {
        tree_text += m_Name;
        tree_text += '\n';
    }

    map< string, TextTreeNode*>::iterator child_iter;
    TextTreeNode* child = nullptr;
    for ( child_iter = m_Children.begin(); child_iter != m_Children.end(); ++child_iter )
    {
        child = child_iter->second;
        if ( child )
        {
            tree_text = child->PrintMe( tree_text );
        }
    }
    return tree_text;
}

string AttributeMgrSingleton::SummarizeAttributesAsTree()
{
    string new_id;

    vector < vector < vector < string > > > tree_id_coll_vecs = AttributeMgr.GetAttrTreeVec();
    vector < vector < string > > tree_id_vecs = tree_id_coll_vecs[0];

    TextTreeNode tree( "Root", "Root", 0 );
    TextTreeNode* cur_node;
    TextTreeNode* new_node;

    for ( int i = 0; i != tree_id_vecs.size(); ++i )
    {
        //reset to root each time
        cur_node = &(tree);

        //go through the ids in the tree vec
        for ( int j = 0; j != tree_id_vecs[i].size(); ++j )
        {
            new_id = tree_id_vecs[i][j];

            // protect against nullptrs
            if ( cur_node )
            {
                // Add child (method has protections to prevent making doubles)
                cur_node->AddChild( new_id, GetName( new_id ) );

                //now the new node is either found or made. access with a protected check to see if it exists first
                if ( cur_node->HasChild( new_id ) )
                {
                    cur_node = cur_node->GetChild( new_id );
                }
            }
        }
    }

    string text = tree.PrintMe();
    return text;
}

vector < string > AttributeMgrSingleton::FindAllAttributes()
{
    vector< string > attribute_ids;
    map < string, NameValData* > ::iterator attrIter;
    for ( attrIter = m_AttrPtrMap.begin(); attrIter != m_AttrPtrMap.end(); ++attrIter )
    {
        attribute_ids.push_back( attrIter->first );
    }
    return attribute_ids;
}

vector < string > AttributeMgrSingleton::FindAttributesByName( const string & search_str )
{
    vector< string > attribute_ids;

    string lower_str = ToLower( search_str );
    string lower_aname;

    map < string, NameValData* > ::iterator attrIter;
    for ( attrIter = m_AttrPtrMap.begin(); attrIter != m_AttrPtrMap.end(); ++attrIter )
    {
        lower_aname = ToLower( attrIter->second->GetName() );

        if ( lower_aname.find( lower_str.c_str() ) != string::npos )
            {
                attribute_ids.push_back( attrIter->first );
            }
    }
    return attribute_ids;
}

string AttributeMgrSingleton::FindAttributeByName( const string & search_str, int index )
{
    string attribute_id = "ID_ERROR";

    vector < string > attribute_ids = FindAttributesByName( search_str );

    if ( index >-1 && index < attribute_ids.size() )
    {
        attribute_id = attribute_ids.at( index );
    }
    return attribute_id;
}

string AttributeMgrSingleton::FindAttributeInCollection( const string & obj_id, const string & search_str, int index )
{
    // if collection id,  search collection
    // if object id, search its collection

    vector< string > attribute_ids;
    vector< string > local_attribute_ids;
    vector< string > local_attribute_names;

    string lower_str = ToLower( search_str );
    string lower_aname;

    string attribute_id = "ID_ERROR";

    AttributeCollection* coll;
    AttributeCollection* coll_from_obj;

    coll = GetCollectionPtr( obj_id );
    coll_from_obj = GetCollectionFromParentID( obj_id );

    if ( coll && coll->GetAttrDataFlag( ) )
    {
        local_attribute_ids = coll->GetAllAttrIDs();
        local_attribute_names = coll->GetAllAttrNames();
        for ( int i = 0; i != local_attribute_ids.size(); ++i )
        {
            lower_aname = ToLower( local_attribute_names[i] );
            if ( lower_aname.find( lower_str.c_str() ) != string::npos )
            {
                attribute_ids.push_back( local_attribute_ids[i] );
            }
        }
    }

    else if ( coll_from_obj && coll_from_obj->GetAttrDataFlag( ) )
    {
        local_attribute_ids = coll_from_obj->GetAllAttrIDs();
        local_attribute_names = coll_from_obj->GetAllAttrNames();
        for ( int i = 0; i != local_attribute_ids.size(); ++i )
        {
            lower_aname = ToLower( local_attribute_names[i] );
            if ( lower_aname.find( lower_str.c_str() ) != string::npos )
            {
                attribute_ids.push_back( local_attribute_ids[i] );
            }
        }
    }

    if ( index >-1 && index < attribute_ids.size() )
    {
        attribute_id = attribute_ids.at( index );
    }
    return attribute_id;
}

vector< string > AttributeMgrSingleton::FindAttributedObjects()
{
    vector< string > attach_ids;
    map< string, AttributeCollection*>::iterator collIter;
    AttributeCollection* coll;
    for ( collIter = m_AttrCollMap.begin(); collIter != m_AttrCollMap.end(); ++collIter )
    {
        coll = collIter->second;
        if ( coll && coll->GetAttrDataFlag( ) )
        {
            attach_ids.push_back( coll->GetAttachID() );
        }
    }
    return attach_ids;
}

string AttributeMgrSingleton::GetObjectName( const string &attachID )
{
    return GetName( attachID, false );
}

string AttributeMgrSingleton::GetChildCollection( const string &attachID )
{
    AttributeCollection* coll = nullptr;
    string id = "ID ERROR";

    coll = GetCollectionFromParentID( attachID );
    if ( coll )
    {
        id = coll->GetID();
    }
    return id;
}

string AttributeMgrSingleton::GetGeomSetCollection( const int & index )
{
    string ac_id = string("NONE");
    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( veh )
    {
        ac_id = veh->GetGeomSetAttrColl( index );
    }
    return ac_id;
}

vector< string > AttributeMgrSingleton::FindAttributesInCollection( const string &collID )
{
    vector< string > attribute_ids;
    AttributeCollection* coll = nullptr;
    coll = GetCollectionPtr( collID );

    if ( coll )
    {
        attribute_ids = coll->GetAllAttrIDs();
    }
    return attribute_ids;
}

vector< string > AttributeMgrSingleton::FindAttributeNamesInCollection( const string &collID )
{
    vector< string > attribute_names;
    AttributeCollection* coll = nullptr;
    coll = GetCollectionPtr( collID );

    if ( coll )
    {
        attribute_names = coll->GetAllAttrNames();
    }
    return attribute_names;
}

string AttributeMgrSingleton::GetAttributeName( const string &attrID )
{
    string name = "NAME ERROR";
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        name = attribute_data->GetName();
    }
    return name;
}

string AttributeMgrSingleton::GetAttributeID( const string &collID, const string &attributeName, int index )
{
    string id = "ID ERROR";
    NameValData* attr;
    AttributeCollection* coll = nullptr;
    coll = GetCollectionPtr( collID );
    if ( coll )
    {
        attr = coll->FindPtr( attributeName, index );
        if ( attr )
        {
            id = attr->GetID();
        }
    }
    return id;
}

string AttributeMgrSingleton::GetAttributeDoc( const string &attrID )
{
    string doc = "DOC ERROR";
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        doc = attribute_data->GetDoc();
    }
    return doc;
}

int AttributeMgrSingleton::GetAttributeType( const string &attrID )
{
    int attr_type = -1;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_type = attribute_data->GetType();
    }
    return attr_type;
}

string AttributeMgrSingleton::GetAttributeTypeName( const string &attrID )
{
    string attr_type_name = "INVALID ATTRIBUTE TYPE";
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_type_name = attribute_data->GetTypeName();
    }
    return attr_type_name;
}

vector< int > AttributeMgrSingleton::GetAttributeBoolVal( const string &attrID )
{
    vector< int > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetBoolData();
    }
    return attr_value;
}

vector< int > AttributeMgrSingleton::GetAttributeIntVal( const string &attrID )
{
    vector< int > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetIntData();
    }
    return attr_value;
}

vector< double > AttributeMgrSingleton::GetAttributeDoubleVal( const string &attrID )
{
    vector< double > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetDoubleData();
    }
    return attr_value;
}

vector< string > AttributeMgrSingleton::GetAttributeStringVal( const string &attrID )
{
    vector< string > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetStringData();
    }
    return attr_value;
}

vector < string > AttributeMgrSingleton::GetAttributeParmID( const string &attrID )
{
    vector< string > p_ids;

    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        p_ids = attribute_data->GetParmIDData();
    }

    return p_ids;
}

vector < double > AttributeMgrSingleton::GetAttributeParmVal( const string &attrID )
{
    vector< string > p_ids;
    vector< double > p_vals;
    Parm* p;

    p_ids = GetAttributeParmID( attrID );

    for ( int i = 0; i != p_ids.size(); ++i )
    {
        p = ParmMgr.FindParm( p_ids[i] );
        p_vals.push_back( ( p ) ? p->Get() : double(0.) );
    }
    return p_vals;
}

vector < string > AttributeMgrSingleton::GetAttributeParmName( const string &attrID )
{
    vector< string > p_ids;
    vector< string > p_names;

    Parm* p;

    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        p_ids = attribute_data->GetParmIDData();
    }

    for ( int i = 0; i != p_ids.size(); ++i )
    {
        p = ParmMgr.FindParm( p_ids[i] );
        p_names.push_back( ( p ) ? p->GetName() : string("NONE") );
    }
    return p_names;
}

vector< vec3d > AttributeMgrSingleton::GetAttributeVec3dVal( const string &attrID )
{
    vector< vec3d > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetVec3dData();
    }
    return attr_value;
}

vector< vector < int > > AttributeMgrSingleton::GetAttributeIntMatrixVal( const string &attrID )
{
    vector< vector < int > > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetIntMatData();
    }
    return attr_value;
}

vector< vector< double > > AttributeMgrSingleton::GetAttributeDoubleMatrixVal( const string &attrID )
{
    vector< vector< double > > attr_value;
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );
    if ( attribute_data )
    {
        attr_value = attribute_data->GetDoubleMatData();
    }
    return attr_value;
}

void AttributeMgrSingleton::DeleteAttribute( const string &attrID, bool updateFlag )
{
    NameValData* attribute_data = nullptr;
    attribute_data = GetAttributePtr( attrID );

    if ( attribute_data )
    {
        string collID = attribute_data->GetAttachID();
        if ( m_AttrCollMap.count( collID ) == 1 )
        {
            SetAttrDirtyFlag( attrID );
            if ( updateFlag )
            {
                Update();
            }
            m_AttrCollMap.at( collID )->DelAttr( attrID );
        }
    }
}

void AttributeMgrSingleton::DeleteAttribute( const vector < string > &attrIDs, bool updateFlag )
{
    for ( int i = 0; i != attrIDs.size(); ++i )
    {
        DeleteAttribute( attrIDs.at( i ), updateFlag );
    }
}

string AttributeMgrSingleton::GuiAddAttribute( AttributeCollection* ac_ptr, const int & attrType, bool updateFlag )
{
    string attrID = string();
    string attrName = ac_ptr->GetNewAttrName( attrType );
    string attrDesc = "";

    bool attrDataBool = false;
    string attrDataString;
    int attrDataInt = 0;
    double attrDataDouble = 0.0;
    NameValData* attrAddPtr;
    vec3d attrDataVec3d(0., 0., 0.);
    vector< vector< int > > attrDataIntMat = {{0 ,0 ,0 },{0 ,0 ,0 },{0 ,0 ,0 }};
    vector< vector< double > > attrDataDblMat = {{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}};

    NameValData attrAdd;
    switch ( attrType )
    {
    case vsp::BOOL_DATA:
        attrAdd = NameValData( attrName, attrDataBool, attrDesc );
        attrAdd.SetType( vsp::BOOL_DATA );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::INT_DATA:
        attrAdd = NameValData( attrName, attrDataInt, attrDesc );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::DOUBLE_DATA:
        attrAdd = NameValData( attrName, attrDataDouble, attrDesc );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::STRING_DATA:
        attrAdd = NameValData( attrName, attrDataString, attrDesc );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::PARM_REFERENCE_DATA:
        attrAdd = NameValData( attrName );
        attrAdd.SetType( vsp::PARM_REFERENCE_DATA );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::ATTR_COLLECTION_DATA:
        attrAdd = NameValData( attrName );
        attrID = attrAdd.GetID();
        attrAdd.AddAttributeCollection();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::VEC3D_DATA:
        attrAdd = NameValData( attrName, attrDataVec3d, attrDesc );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::INT_MATRIX_DATA:
        attrAdd = NameValData( attrName, attrDataIntMat, attrDesc );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    case vsp::DOUBLE_MATRIX_DATA:
        attrAdd = NameValData( attrName, attrDataDblMat, attrDesc );
        attrID = attrAdd.GetID();
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        ac_ptr->Add( attrAdd );
        break;
    }
    if ( updateFlag )
    {
        Update();
    }
    return attrID;
}

string AttributeMgrSingleton::AddAttributeBool( const string &collID, const string &attributeName, int value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    attrAdd.SetType( vsp::BOOL_DATA );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeInt( const string &collID, const string &attributeName, int value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeDouble( const string &collID, const string &attributeName, double value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeString( const string &collID, const string &attributeName, const string &value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeParm( const string &collID, const string &attributeName, const string &parmID, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName );
    attrAdd.SetType( vsp::PARM_REFERENCE_DATA );
    attrAdd.SetParmIDData( { parmID } );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeVec3d( const string &collID, const string &attributeName, const vector < vec3d > &value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeIntMatrix( const string &collID, const string &attributeName, const vector< vector< int > > &value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeDoubleMatrix( const string &collID, const string &attributeName, const vector< vector< double > > &value, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName, value, "" );
    id = AddAttributeUtil( collID, attrAdd, updateFlag );
    return id;
}

string AttributeMgrSingleton::AddAttributeGroup( const string &collID, const string &attributeName, bool updateFlag )
{
    string id = string();
    NameValData attrAdd = NameValData( attributeName );
    attrAdd.AddAttributeCollection();
    id = AddAttributeUtil( collID, attrAdd, false );
    if ( updateFlag )
    {
        Update();
    }
    return id;
}

string AttributeMgrSingleton::AddAttributeUtil( const string &collID, NameValData &attrAdd, bool updateFlag )
{
    string id = string();
    if ( m_AttrCollMap.count( collID ) == 1 )
    {
        attrAdd.SetAttrAttach( collID );
        SetDirtyFlag( attrAdd.GetAttributeEventGroup() );
        id = attrAdd.GetID();
        m_AttrCollMap.at( collID )->Add( attrAdd );
        if ( updateFlag )
        {
            Update();
        }
    }
    return id;
}

void AttributeMgrSingleton::SetAttributeName( const string &attrID, const string &name, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    AttributeCollection* coll;
    if ( attr )
    {
        string collID = attr->GetAttachID();
        coll = GetCollectionPtr( collID );
    }

    if ( attr && coll )
    {
        coll->RenameAttr( attrID, name );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeDoc( const string &attrID, const string &doc, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );
    if ( attr )
    {
        attr->SetDoc( doc );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeBool( const string &attrID, int value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetBoolData( { value } );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeInt( const string &attrID, int value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetIntData( { value } );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeDouble( const string &attrID, double value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetDoubleData( { value } );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeString( const string &attrID, const string &value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetStringData( { value } );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeParmID( const string &attrID, const string &value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetParmIDData( { value } );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeVec3d( const string &attrID, const vector < vec3d > &value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetVec3dData( value );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeIntMatrix( const string &attrID, const vector< vector< int > > &value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetIntMatData( value );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

void AttributeMgrSingleton::SetAttributeDoubleMatrix( const string &attrID, const vector< vector< double > > &value, bool updateFlag )
{
    NameValData* attr = GetAttributePtr( attrID );

    if ( attr )
    {
        attr->SetDoubleMatData( value );
        SetAttrDirtyFlag( attrID );
        if ( updateFlag )
        {
            Update();
        }
    }
}

AttributeCollection* AttributeMgrSingleton::GetCollectionPtr( string collID, bool only_populated_collections )
{
    AttributeCollection* attrPtr = nullptr;
    if ( ( m_AttrCollMap.count( collID ) == 1 ) && ( collID.size() > 0 ) )
    {
        if ( !only_populated_collections || m_AttrCollMap.at( collID )->GetAttrDataFlag() )
        {
            attrPtr = m_AttrCollMap.at( collID );
        }
    }
    return attrPtr;
}

// check if VSP object is compatible with attributeCollections or IS an attributecollection
AttributeCollection* AttributeMgrSingleton::GetCollectionFromParentID( const string & id )
{
    AdvLink* advlink_ptr = AdvLinkMgr.GetLink( AdvLinkMgr.GetLinkIndex( id ) );

    if ( advlink_ptr )
    {
        return advlink_ptr->GetAttrCollection();
    }

    // then, check parm and advlink, which aren't derived from ParmContainer
    if ( id.size() == vsp::ID_LENGTH_PARM || IDMgr.NonRandomID( id ) )
    {
        Parm* parm_ptr = ParmMgr.FindParm( id );
        if ( parm_ptr )
        {
            return parm_ptr->GetAttrCollection();
        }
    }

    // first, check if it's an attribute that contains a collection (an attribute group)
    if ( id.size() == vsp::ID_LENGTH_ATTR || IDMgr.NonRandomID( id ) )
    {
        NameValData* nvd_ptr = AttributeMgr.GetAttributePtr( id );
        if ( nvd_ptr && nvd_ptr->GetType() == vsp::ATTR_COLLECTION_DATA )
        {
            return nvd_ptr->GetAttributeCollectionPtr( 0 );
        }
    }

    // next, check if it's an attribute collection already, then return itself.
    if ( id.size() == vsp::ID_LENGTH_ATTRCOLL || IDMgr.NonRandomID( id ) )
    {
        AttributeCollection* ac_ptr = AttributeMgr.GetCollectionPtr( id );
        if ( ac_ptr )
        {
            return ac_ptr;
        }
    }

    // then, check if parmContainer-derived object, and return its attrcollection
    if ( id.size() == vsp::ID_LENGTH_PARMCONTAINER )
    {
        ParmContainer* pc_ptr = ParmMgr.FindParmContainer( id );
        if ( pc_ptr )
        {
            return pc_ptr->GetAttrCollection();
        }
    }

    return nullptr;
}


vector < AttributeCollection* > AttributeMgrSingleton::GetAllCollectionPtrs( int permit_empty_collection_type, vector < string > inc_ids )
{
    vector < AttributeCollection* > AttrColls;

    map< string, AttributeCollection*>:: iterator collIter;

    AttributeCollection* coll;

    int iter_count = 0;

    for ( collIter = m_AttrCollMap.begin(); collIter != m_AttrCollMap.end(); ++collIter )
        {
            iter_count++;
            coll = collIter->second;

            if ( coll )
            {
                bool has_id = ( find( inc_ids.begin(), inc_ids.end(), coll->GetID() ) != inc_ids.end() );

                if ( has_id || coll->GetAttrDataFlag() || coll->GetAttachType() == permit_empty_collection_type )
                {
                    AttrColls.push_back(coll);
                }
            }
        }

    return AttrColls;
}

int AttributeMgrSingleton::CopyAttributeUtil( const string &attr_id, bool updateFlag )
{
    NameValData* nvd_ptr = GetAttributePtr( attr_id );
    if ( !nvd_ptr || nvd_ptr->IsProtected() )
    {
        return 1;
    }
    m_AttrClipboard.clear();
    NameValData nvd;
    nvd.CopyFrom( nvd_ptr );
    m_AttrClipboard.push_back( nvd );
    SetAttrDirtyFlag( nvd.GetID() );
    if ( updateFlag )
    {
        Update();
    }
    return 0;
}

int AttributeMgrSingleton::CopyAttributeUtil( const vector < string > &attr_ids, bool updateFlag )
{
    // check vector of attribute ids for any copy errors before clearing the clipboard and copying onto it
    NameValData* a = nullptr;
    vector < NameValData* > nvd_ptr_vec;
    for ( int i = 0; i != attr_ids.size(); ++ i )
    {
        a = GetAttributePtr( attr_ids.at( i ) );
        if ( a && !a->IsProtected() )
        {
            nvd_ptr_vec.push_back( a );
        }
        else
        {
            return 1;
        }
    }

    // go through and populate the clipboard
    m_AttrClipboard.clear();
    for ( int i = 0; i != attr_ids.size(); ++ i )
    {
        NameValData nvd;
        nvd.CopyFrom( nvd_ptr_vec.at( i ) );
        m_AttrClipboard.push_back( nvd );
        SetAttrDirtyFlag( nvd.GetID() );
    }

    if ( updateFlag )
    {
        Update();
    }

    return 0;
}

void AttributeMgrSingleton::CutAttributeUtil( const string &attr_id, bool updateFlag )
{
    vector < string > attr_id_vec = { attr_id };
    CutAttributeUtil( attr_id_vec, updateFlag );
}

void AttributeMgrSingleton::CutAttributeUtil( const vector < string > &attr_ids, bool updateFlag )
{
    int copy_error = CopyAttributeUtil( attr_ids, false );
    if ( !copy_error )
    {
        DeleteAttribute( attr_ids, false );

        for ( int i = 0; i != attr_ids.size(); ++i )
        {
            m_AttrClipboard.at( i ).ChangeID( attr_ids.at( i ) );
        }
    }
    if ( updateFlag )
    {
        Update();
    }
}

vector < string > AttributeMgrSingleton::PasteAttributeUtil( const string &coll_id, bool updateFlag )
{
    vector < string > coll_ids = { coll_id };
    return PasteAttributeUtil( coll_ids, updateFlag );
}

vector < string > AttributeMgrSingleton::PasteAttributeUtil( const vector < string > &coll_ids, bool updateFlag )
{
    int paste_error = 0;
    vector < string > attr_paste_ids;
    vector < AttributeCollection* > ac_vec;
    for ( int i = 0 ; i != coll_ids.size(); ++i )
    {
        ac_vec.push_back( GetCollectionPtr( coll_ids.at( i ) ) );
        if ( !ac_vec.at( i ) )
        {
            paste_error = 1;
        }
    }

    if ( paste_error )
    {
        return attr_paste_ids;
    }

    for ( int i = 0 ; i != ac_vec.size(); ++i )
    {
        for ( int j = 0; j != m_AttrClipboard.size(); ++j )
        {
            string lastreset = IDMgr.ResetRemapID();
            NameValData nvd_ref = m_AttrClipboard[j];
            NameValData nvd;
            if ( &(nvd_ref) )
            {
                nvd.CopyFrom( &(nvd_ref) );
                nvd.SetAttrAttach( ac_vec.at( i )->GetID() );
                string attrID = nvd.GetID();
                ac_vec.at( i )->Add( nvd );
                SetAttrDirtyFlag( attrID );
                attr_paste_ids.push_back( attrID );
            }
            IDMgr.ResetRemapID( lastreset );
        }
    }
    if ( updateFlag )
    {
        Update();
    }
    return attr_paste_ids;
}

int AttributeMgrSingleton::CopyAttribute( const string &attr_id, bool updateFlag )
{
    return CopyAttributeUtil( attr_id, updateFlag );
}

void AttributeMgrSingleton::CutAttribute( const string &attr_id, bool updateFlag )
{
    CutAttributeUtil( attr_id, updateFlag );
}

vector < string > AttributeMgrSingleton::PasteAttribute( const string &obj_id, bool updateFlag )
{
    string coll_id = obj_id;
    AttributeCollection* ac = GetCollectionPtr( obj_id );
    if ( !ac )
    {
        ac = GetCollectionFromParentID( obj_id );
        coll_id = ac->GetID();
    }
    return PasteAttributeUtil( coll_id, updateFlag );
}

// extend a vector of strings with another vector of strings (concatenate) with option of adding a common root to the extension vector
vector < string > AttributeMgrSingleton::ExtendStringVector( vector < string > vec, vector < string > vec_add, string string_root, bool skip_empty_strings )
{
    for ( int i = 0; i != vec_add.size(); i++ )
    {
        if ( !skip_empty_strings || vec_add[i].size() )
        {
            vec.push_back(string_root + vec_add[i]);
        }
    }
    return vec;
}

// repeat base vec with different appended element each time
vector < vector < string > > AttributeMgrSingleton::TransposeExtendStringVector( vector < string > base_vec, vector < string > vec_add )
{
    vector < vector < string > > output_nested_vec;

    for ( int i = 0; i != vec_add.size(); i++ )
    {
        output_nested_vec.push_back( AttributeMgrSingleton::ExtendStringVector( base_vec, { vec_add[i] } ) );
    }
    return output_nested_vec;
}

// extend a vector of vectors of strings with another vector of vectors of strings (concatenate)
vector < vector < string > > AttributeMgrSingleton::ExtendNestedStringVector( vector < vector < string > > vec, vector < vector < string > > vec_add )
{
    for ( int i = 0; i != vec_add.size(); i++ )
    {
        vec.push_back(vec_add[i]);
    }
    return vec;
}

vector < vector < string > > AttributeMgrSingleton::GetCollParentVecs( const vector < vector < string > > & vecs_ids )
{
    vector < vector < string > > vecs_colls;
    for ( int i = 0; i != vecs_ids.size(); i ++ )
    {
        vecs_colls.push_back( GetCollParentVec (vecs_ids.at( i ) ) );
    }
    return vecs_colls;
}

vector < string > AttributeMgrSingleton::GetCollParentVec( const vector < string > & vec_ids )
{
    // iterate through vector. if current ID is an attribute collection, set write string to that ID.
    // write that ID for each index until overwritten with a new one.

    // ideally, multiple indices will possess the same attribute collection, such as when you have a filler branch such as "/Attributes" or "/Parms" which is used organizationally in the tree but refer to no VSP objects
    vector < string > vec_colls;
    if ( !vec_ids.size() )
    {
        return vec_colls;
    }

    string id;
    string curr_coll = "NONE";
    NameValData* nvd;
    AttributeCollection* ac;

    for ( int i = 0; i != vec_ids.size(); i++ )
    {
        id = vec_ids.at( i );
        ac = GetCollectionFromParentID( id );
        if ( ac )
        {
            curr_coll = ac->GetID();
        }
        vec_colls.push_back( curr_coll );
    }
    return vec_colls;
}

string AttributeMgrSingleton::GetObjectParent( const string & id )
{
    string parent_id = "NONE";

    if ( id.size() == vsp::ID_LENGTH_PARM || IDMgr.NonRandomID( id ) )
    {
        Parm* parm_ptr = ParmMgr.FindParm( id );
        if ( parm_ptr )
        {
            parent_id = parm_ptr->GetContainerID();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_ATTR || IDMgr.NonRandomID( id ) )
    {
        NameValData* nvd_ptr = GetAttributePtr( id );
        if ( nvd_ptr )
        {
            parent_id = nvd_ptr->GetAttachID();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_ATTRCOLL || IDMgr.NonRandomID( id ) )
    {
        AttributeCollection* ac_ptr = GetCollectionPtr( id );
        if ( ac_ptr )
        {
            parent_id = ac_ptr->GetAttachID();
        }
    }

    if ( !AllUpper( id ) )
    {
        return parent_id;
    }

    if ( id.size() == vsp::ID_LENGTH_PRESET_SETTING )
    {
        Setting* s = VarPresetMgr.FindSetting( id );
        if ( s )
        {
            parent_id = s->GetGroupID();
        }
    }

    // parmcontainer type things with parents
    if ( id.size() == vsp::ID_LENGTH_PARMCONTAINER )
    {
        ParmContainer* pc = ParmMgr.FindParmContainer( id );

        if ( pc )
        {
            int pc_type = pc->GetParmContainerType();
            if ( pc_type == vsp::ATTROBJ_GEOM )
            {
                Geom* geom_ptr = VehicleMgr.GetVehicle()->FindGeom( id );
                if ( geom_ptr )
                {
                    parent_id = geom_ptr->GetParentID();
                }
            }
            else if ( pc_type == vsp::ATTROBJ_SUBSURF )
            {
                SubSurface* ss_ptr = SubSurfaceMgr.GetSubSurf( id );
                if ( ss_ptr )
                {
                    parent_id = ss_ptr->GetCompID();
                }
            }
            else if ( pc_type == vsp::ATTROBJ_XSEC )
            {
                ParmContainer* parent_pc = pc->GetParentContainerPtr();
                if ( parent_pc )
                {
                    parent_pc = parent_pc->GetParentContainerPtr();
                    if ( parent_pc )
                    {
                        parent_pc = parent_pc->GetParentContainerPtr();
                        if ( parent_pc )
                        {
                            parent_id = parent_pc->GetID();
                        }
                    }
                }
            }
            else if ( pc_type == vsp::ATTROBJ_SEC )
            {
                ParmContainer* parent_pc = pc->GetParentContainerPtr();
                if ( parent_pc )
                {
                    parent_pc = parent_pc->GetParentContainerPtr();

                    if ( parent_pc )
                    {
                        parent_id = parent_pc->GetID();
                    }
                }
            }
            else
            {
                ParmContainer* parent_pc = pc->GetParentContainerPtr();
                if ( parent_pc )
                {
                    parent_id = parent_pc->GetID();
                }
            }
        }
    }

    if ( parent_id.empty() )
    {
        parent_id = "NONE";
    }
    return parent_id;
}

string AttributeMgrSingleton::GetName( const string & id, bool return_name_input )
{
    AdvLink* advlink_ptr = AdvLinkMgr.GetLink( AdvLinkMgr.GetLinkIndex( id ) );
    if (advlink_ptr)
    {
        return advlink_ptr->GetName();
    }

    if ( id.size() == vsp::ID_LENGTH_PARM || IDMgr.NonRandomID( id ) )
    {
        Parm* parm_ptr = ParmMgr.FindParm( id );
        if (parm_ptr)
        {
            return parm_ptr->GetName();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_ATTR || IDMgr.NonRandomID( id ) )
    {
        NameValData* nvd_ptr = AttributeMgr.GetAttributePtr( id );
        if ( nvd_ptr )
        {
            return nvd_ptr->GetName();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_ATTRCOLL || IDMgr.NonRandomID( id ) )
    {
        AttributeCollection* ac_ptr = AttributeMgr.GetCollectionPtr( id );
        if (ac_ptr)
        {
            return ac_ptr->GetName();
        }
    }

    // make a bypass if the string input is NOT an id, returning the string as input (e.g., "Geoms" node in the id vector for tree construction)
    if ( return_name_input && !AllUpper( id ) )
    {
        return id;
    }

    if ( id.size() == vsp::ID_LENGTH_PARMCONTAINER )
    {
        ParmContainer* pc_ptr = ParmMgr.FindParmContainer( id );
        ParmContainer* pc_parent;
        Link* link_ptr = dynamic_cast<Link*>(pc_ptr);

        if ( link_ptr )
        {
            return link_ptr->GetName() + '_' + GetName( link_ptr->GetParmA() ) + "_" + GetName( link_ptr->GetParmB() );
        }
        else if ( pc_ptr )
        {
            XSecSurf* xsec_surf = dynamic_cast< XSecSurf* >( pc_ptr );
            int pc_type = pc_ptr->GetParmContainerType();
            if ( pc_type == vsp::ATTROBJ_XSEC )
            {
                pc_parent = pc_ptr->GetParentContainerPtr();
                if ( pc_parent )
                {
                    XSecSurf* xsp_ptr = dynamic_cast<XSecSurf*>( pc_parent->GetParentContainerPtr() );
                    if ( xsp_ptr )
                    {
                        int xs_i = xsp_ptr->FindXSecIndex( pc_parent->GetID() );
                        string name_str = string("Xsec_");
                        name_str += to_string(xs_i);
                        return name_str;
                    }
                }
                return string("ERROR XSEC NAME");
            }
            else if ( pc_type == vsp::ATTROBJ_SEC )
            {
                XSecSurf* xsp_ptr = dynamic_cast<XSecSurf*>( pc_ptr->GetParentContainerPtr() );
                if ( xsp_ptr )
                {
                    int xs_i = xsp_ptr->FindXSecIndex( pc_ptr->GetID() );
                    string name_str = string("Sect_");
                    name_str += to_string(xs_i);
                    return name_str;
                }
                return string("ERROR SECT NAME");
            }
            else if ( xsec_surf )
            {
                return string("XSecs");
            }
            return pc_ptr->GetName();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_PRESET_SETTING )
    {
        Setting* s = VarPresetMgr.FindSetting( id );
        if ( s )
        {
            return s->GetName();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_PRESET_GROUP )
    {
        SettingGroup* sg = VarPresetMgr.FindSettingGroup( id );
        if ( sg )
        {
            return sg->GetName();
        }
    }

    return "NONE";
}

//==== Check if string is all uppercase ====//
bool AttributeMgrSingleton::AllUpper( const string & str )
{
    for ( int i = 0; i != str.size(); i++ )
    {
        if ( !isupper(str[i]) )
        {
            return false;
        }
    }
    return true;
}

string AttributeMgrSingleton::ToLower( const string & str )
{
    string lower_str;
    for ( int i = 0; i != str.size(); ++i )
    {
        lower_str += tolower( str.at(i) );
    }
    return lower_str;
}

bool AttributeMgrSingleton::VecInClipboard( const vector < string > & stringVec )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    for ( int i = 0; i != stringVec.size(); ++i )
    {
        if ( veh->IDinClipboard( stringVec[i] ) )
        {
            return true;
        }
    }
    return false;
}

bool AttributeMgrSingleton::VecInClipboard( const vector < vector < string > > & stringVec )
{
    for ( int i = 0; i != stringVec.size(); ++ i )
    {
        if ( VecInClipboard( stringVec[i]) )
        {
            return true;
        }
    }
    return false;
}

bool AttributeMgrSingleton::VecInClipboard( const vector < vector < vector < string > > > & stringVec )
{
    for ( int i = 0; i != stringVec.size(); ++ i )
    {
        if ( VecInClipboard( stringVec[i]) )
        {
            return true;
        }
    }
    return false;
}


bool AttributeMgrSingleton::CheckTreeVecID( const string & id, const string & check_id )
{
    string parent_id = GetObjectParent( id );

    bool check_id_flag = ( check_id.compare(id) == 0 );

    if ( check_id_flag || check_id.empty() )
    {
        return true;
    }

    while ( parent_id.compare("NONE") != 0 && parent_id.compare("") != 0 && !check_id_flag )
    {
        check_id_flag = ( check_id.compare( parent_id ) == 0 );
        parent_id = GetObjectParent( parent_id );
    }

    return check_id_flag;
}

vector < string > AttributeMgrSingleton::GetTreeVec( const string & id, const string & root_id )
{
    string parent_id = GetObjectParent( id );
    vector < string > return_vec = { id };

    bool check_root_id = root_id.size();              //only check for root_id if its size > 0
    bool root_id_flag = ( root_id.compare(id) == 0 ); //flag for root_id occurence in while-loop; if compare == 0, then flag = true. If !=0, flag = false
    // if id == root_id, skip the while loop and return the string

    while ( parent_id.compare("NONE") != 0 && parent_id.compare("") != 0 && !root_id_flag )
    {
        // if parent ID is valid and string doesn't contain the root_id, add to the return string
        return_vec = ExtendStringVector( { parent_id }, return_vec );

        root_id_flag = ( root_id.compare( parent_id ) == 0 );

        parent_id = GetObjectParent( parent_id );
    }

    if ( check_root_id && !root_id_flag ) //if checking for the root_id and the string did not contain it, return zero-length string
    {
        return {};
    }
    // if not checking for the root_id or it has contained the root_id, return the string as constructed
    return return_vec;
}

NameValData* AttributeMgrSingleton::GetAttributePtr( const string &id )
{
    if ( m_AttrPtrMap.count( id ) )
    {
        return m_AttrPtrMap.at( id );
    }

    return nullptr;
}

string AttributeMgrSingleton::GetObjectTypeName( const string & id )
{
    int objType = GetObjectType( id );
    string name = "NONE";
    switch ( objType ){
        case vsp::ATTROBJ_PARM:
            return string( "Parm" );
        case vsp::ATTROBJ_GEOM:
            return string( "Geom" );
        case vsp::ATTROBJ_FREE:
            return string( "Double" );
        case vsp::ATTROBJ_VEH:
            return string( "Vehicle" );
        case vsp::ATTROBJ_SUBSURF:
            return string( "SubSurface" );
        case vsp::ATTROBJ_MEASURE:
            return string( "Measure" );
        case vsp::ATTROBJ_LINK:
            return string( "Link" );
        case vsp::ATTROBJ_ADVLINK:
            return string( "AdvLink" );
        case vsp::ATTROBJ_ATTR:
            return string( "Attribute" );
        case vsp::ATTROBJ_COLLECTION:
            return string( "AttributeCollection" );
        default:
            return string( "NONE ");
    }
}

int AttributeMgrSingleton::GetObjectType( const string & id )
{
    AdvLink* advlink_ptr = AdvLinkMgr.GetLink( AdvLinkMgr.GetLinkIndex( id ) );
    if (advlink_ptr)
    {
        return vsp::ATTROBJ_ADVLINK;
    }

    if ( id.size() == vsp::ID_LENGTH_PARM || IDMgr.NonRandomID( id ) )
    {
        Parm* parm_ptr = ParmMgr.FindParm( id );
        if (parm_ptr)
        {
            return vsp::ATTROBJ_PARM;
        }
    }

    if ( id.size() == vsp::ID_LENGTH_ATTR || IDMgr.NonRandomID( id ) )
    {
        NameValData* nvd_ptr = GetAttributePtr( id );
        if ( nvd_ptr )
        {
            return vsp::ATTROBJ_ATTR;
        }
    }

    if ( id.size() == vsp::ID_LENGTH_ATTRCOLL || IDMgr.NonRandomID( id ) )
    {
        AttributeCollection* ac_ptr = GetCollectionPtr( id );
        if ( ac_ptr )
        {
            return vsp::ATTROBJ_COLLECTION;
        }
    }

    if ( id.size() == vsp::ID_LENGTH_PARMCONTAINER )
    {
        ParmContainer* pc_ptr = ParmMgr.FindParmContainer( id );
        if ( pc_ptr )
        {
            return pc_ptr->GetParmContainerType();
        }
    }

    if ( id.size() == vsp::ID_LENGTH_PRESET_GROUP )
    {
        SettingGroup* sg = VarPresetMgr.FindSettingGroup( id );
        if ( sg )
        {
            return vsp::ATTROBJ_VARGROUP;
        }
    }

    if ( id.size() == vsp::ID_LENGTH_PRESET_SETTING )
    {
        Setting* s = VarPresetMgr.FindSetting( id );
        if ( s )
        {
            return vsp::ATTROBJ_VARSETTING;
        }
    }

    return vsp::ATTROBJ_FREE;
}

vector< vector< vector< string > > > AttributeMgrSingleton::GetAttrTreeVec( const string & root_id, vector < string > inc_ids, bool exclude_clipboard, int filter_attr_type, int filter_attach_type, const string & filter_attr_string, bool filter_attr_case_sens )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    vector< AttributeCollection* > attrCollPtrs = AttributeMgr.GetAllCollectionPtrs( vsp::ATTROBJ_ATTR, inc_ids );

    // bools for checking attribute type and name
    bool check_type = ( filter_attr_type > -1 );
    bool check_str = ( filter_attr_string.size() );
    bool check_obj = ( filter_attach_type != vsp::ATTROBJ_FREE );

    bool include_collection;
    bool force_add_empty;

    //structure:
    // vec of 2-len vec
    //      1st vec = vec of tree string
    //      2nd vec = vec of tree string's next-highest attributecollectionID at each index

    vector< vector< vector< string > > > attr_parent_vectors;

    vector< vector< string > > attribute_vectors;
    vector< vector< string > > parent_vectors;

    vector< string > nvd_ids;

    bool check_root_id = root_id.size();
    bool special_parmission;

    string collID;
    string attachID;
    int attachType;
    string attrName;
    AttributeCollection* ac;

    //iterate through all populated attribute collection objects
    for ( int icoll = 0; icoll < attrCollPtrs.size(); icoll ++ )
    {
        ac = attrCollPtrs[icoll];
        collID = ac->GetID();
        attachID = ac->GetAttachID();
        attachType = GetObjectType( attachID );

        nvd_ids = ac->GetAllAttrIDs();

        // carry through any empty collections forwarded by active gui elements
        // attribute groups are exception since they already show up as attributes themselves
        force_add_empty = false;
        if ( !nvd_ids.size() && attachType != vsp::ATTROBJ_ATTR )
        {
            force_add_empty = true;
        }

        include_collection = true;

        if ( check_type || check_str || check_obj )
        {
            vector < string > nvd_ids_subset;
            for ( int i = 0; i != nvd_ids.size(); ++i )
            {
                bool add_id = true;
                NameValData* nvd = AttributeMgr.GetAttributePtr( nvd_ids.at(i) );

                string nvd_name_check;
                string search_name_check;

                if ( filter_attr_case_sens )
                {
                    nvd_name_check = nvd->GetName();
                    search_name_check = filter_attr_string;
                }
                else
                {
                    nvd_name_check = ToLower( nvd->GetName() );
                    search_name_check = ToLower( filter_attr_string );
                }

                if ( check_type && nvd->GetType() != filter_attr_type )
                {
                    add_id = false;
                }

                if ( check_str && nvd_name_check.find( search_name_check.c_str() ) == string::npos )
                {
                    add_id = false;
                }

                if ( check_obj && attachType != filter_attach_type )
                {
                    add_id = false;
                }

                if ( add_id )
                {
                    nvd_ids_subset.push_back( nvd_ids.at(i) );
                }
            }
            // if a previously-allocated collection is emptied, prevent its showing. If it was empty to begin with, you're looking at an active empty collection and it must be shown to user.
            if ( !nvd_ids_subset.size() && !force_add_empty )
            {
                include_collection = false;
            }
            nvd_ids = nvd_ids_subset;
        }

        if ( include_collection )
        {
            vector < string > vecbranch;

            //need new methods to work with vectors instead of a delimited string- but it will be much safer than guessing a safe delimiter

            Geom* geom_ptr;
            Parm* parm_ptr;
            ParmContainer* pc_ptr;
            SubSurface* ss_ptr;

            Link* link_ptr;
            AdvLink* avdlink_ptr;

            Probe* probe_ptr;
            RSTProbe* rst_ptr;
            Ruler* ruler_ptr;
            Protractor* protractor_ptr;

            string pcID;

            vector< vector< string > > branch_id_vectors;
            vector< vector< string > > branch_coll_vectors;

            special_parmission = false;

            while ( attachType == vsp::ATTROBJ_ATTR || attachType == vsp::ATTROBJ_COLLECTION )
            {
                AttributeCollection* ac_check = GetCollectionPtr( attachID );
                if ( !ac_check )
                {
                    vecbranch = ExtendStringVector( { attachID }, vecbranch );
                }
                collID = attachID;
                attachID = GetObjectParent( attachID );
                attachType = GetObjectType( attachID );
            }

            vecbranch = ExtendStringVector( { collID }, vecbranch );

            // if a parm, append branch string with parm's name, 'Parms' node, and move attach ID up to its container
            if ( attachType ==  vsp::ATTROBJ_PARM )
            {
                vecbranch = ExtendStringVector( { "Parms", attachID }, vecbranch );
                special_parmission = CheckTreeVecID( attachID, root_id );


                attachID = GetObjectParent( attachID );
                attachType = GetObjectType( attachID ); // get the TYPE of the parm-container that holds the original parm
            }

            if ( attachType == vsp::ATTROBJ_GEOM
            || attachType == vsp::ATTROBJ_SEC
            || attachType == vsp::ATTROBJ_XSEC
            || attachType == vsp::ATTROBJ_SUBSURF )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    bool exclude_orphan = false;
                    if ( attachType == vsp::ATTROBJ_SUBSURF )
                    {
                        vecbranch = ExtendStringVector( { "SubSurfs", attachID }, vecbranch );
                        attachID = GetObjectParent( attachID );
                        if ( attachID == "NONE" ) //I should know which one is the failure state... casting a wide net implies that it's a poorly controlled method
                        {
                            exclude_orphan = true;
                        }
                    }
                    else if ( attachType == vsp::ATTROBJ_XSEC )
                    {
                        vecbranch = ExtendStringVector( { "XSecs", attachID }, vecbranch );
                        attachID = GetObjectParent( attachID );
                        if ( attachID == "NONE" ) //I should know which one is the failure state... casting a wide net implies that it's a poorly controlled method
                        {
                            exclude_orphan = true;
                        }
                    }
                    else if ( attachType == vsp::ATTROBJ_SEC )
                    {
                        vecbranch = ExtendStringVector( { "Sections", attachID }, vecbranch );
                        attachID = GetObjectParent( attachID );
                        if ( attachID == "NONE" ) //I should know which one is the failure state... casting a wide net implies that it's a poorly controlled method
                        {
                            exclude_orphan = true;
                        }
                    }

                    vecbranch = ExtendStringVector( GetTreeVec( attachID, root_id ), vecbranch );
                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "Geoms" }, vecbranch );
                    }
                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );

                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !exclude_orphan && !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType ==  vsp::ATTROBJ_VEH )
            {
                if ( !check_root_id || veh->GetID().compare( root_id ) == 0 )
                {
                    vecbranch = ExtendStringVector( { veh->GetID() }, vecbranch );

                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType == vsp::ATTROBJ_MEASURE )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    vecbranch.insert( vecbranch.begin() , attachID );

                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "Measures" }, vecbranch );
                    }

                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType == vsp::ATTROBJ_LINK
                || attachType == vsp::ATTROBJ_ADVLINK )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    vecbranch.insert( vecbranch.begin() , attachID );
                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "Links" }, vecbranch );
                    }

                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType == vsp::ATTROBJ_MODE )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    vecbranch.insert( vecbranch.begin() , attachID );
                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "Modes" }, vecbranch );
                    }

                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType == vsp::ATTROBJ_SET )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    vecbranch.insert( vecbranch.begin() , attachID );
                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "Sets" }, vecbranch );
                    }

                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType == vsp::ATTROBJ_VARGROUP || attachType == vsp::ATTROBJ_VARSETTING )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    if ( attachType == vsp::ATTROBJ_VARSETTING )
                    {
                        vecbranch = ExtendStringVector( { "Settings", attachID }, vecbranch );
                        attachID = GetObjectParent( attachID );
                    }

                    vecbranch.insert( vecbranch.begin() , attachID );
                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "VarPresetGroups" }, vecbranch );
                    }

                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
            else if ( attachType == vsp::ATTROBJ_FREE )
            {
                if ( !check_root_id || CheckTreeVecID( attachID, root_id ) || special_parmission )
                {
                    if ( attachID != "NONE" )
                    {
                        vecbranch.insert( vecbranch.begin() , attachID );
                    }
                    if ( !check_root_id )
                    {
                        vecbranch = ExtendStringVector( { "Free" }, vecbranch );
                    }
                    branch_id_vectors = TransposeExtendStringVector( vecbranch, nvd_ids );
                    if ( !nvd_ids.size() )
                    {
                        branch_id_vectors.push_back( vecbranch );
                    }
                    branch_coll_vectors = GetCollParentVecs( branch_id_vectors );

                    if ( !VecInClipboard( { branch_id_vectors, branch_coll_vectors } ) )
                    {
                        attribute_vectors = ExtendNestedStringVector( attribute_vectors, branch_id_vectors );
                        parent_vectors = ExtendNestedStringVector( parent_vectors, branch_coll_vectors );
                    }
                }
            }
        }
    }

    vector< vector< vector < string > > > name_id_parent_vec; //for sorting
    string name_concat;

    for ( int i = 0; i != attribute_vectors.size(); ++i )
    {
        name_concat.clear();
        for ( int j = 0; j != attribute_vectors[i].size(); ++j )
        {
            name_concat += GetName( attribute_vectors[i][j] );
        }
        name_id_parent_vec.push_back( { { name_concat }, attribute_vectors[i], parent_vectors[i] } );
    }

    sort( name_id_parent_vec.begin(), name_id_parent_vec.end(), AttributeMgrSingleton::NestedVecSorter );

    vector < vector < string > > sorted_attribute_vectors;
    vector < vector < string > > sorted_parent_vectors;

    for ( int i = 0; i != name_id_parent_vec.size(); ++i )
    {
        sorted_attribute_vectors.push_back(name_id_parent_vec[i][1]);
        sorted_parent_vectors.push_back(name_id_parent_vec[i][2]);
    }

    attr_parent_vectors.push_back( sorted_attribute_vectors );
    attr_parent_vectors.push_back( sorted_parent_vectors );

    return attr_parent_vectors;

}

bool AttributeMgrSingleton::NestedVecSorter( const vector < vector < string > > & v1, const vector < vector < string > > & v2 )
{
    return v1[0][0] < v2[0][0];
}
