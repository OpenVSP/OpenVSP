//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AttributeManager.h: Manage attributes for user-controlled auxiliary data
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#ifndef _ATTRMGRH_
#define _ATTRMGRH_

#include "ResultsMgr.h"

using std::string;

//==== Attribute Manager ====//
class AttributeMgrSingleton
{
public:
    ~AttributeMgrSingleton();

    static AttributeMgrSingleton& getInstance()
    {
        static AttributeMgrSingleton instance;
        return instance;
    }

    void Wype();

    void RegisterCollID( const string & collID, AttributeCollection* ac_ptr );
    void DeregisterCollID( const string & collID );
    bool GetCollRegistration( const string & collID )
    {
        return m_AttrCollMap.count( collID );
    }

    void RegisterAttrID( const string &attrID, NameValData* attr );
    void DeregisterAttrID( const string &attrID );
    bool GetAttrRegistration( const string &attrID )
    {
        return m_AttrPtrMap.count( attrID );
    }

    void SetAttrDirtyFlag( const string &attrID );
    void SetDirtyFlag( int f );
    bool GetDirtyFlag( int f );
    void ClearDirtyFlag( int f );

    void Update();

    AttributeCollection* GetCollectionPtr( const string &collID, bool only_populated_collections = false );
    static AttributeCollection* GetCollectionFromParentID( const string & id );
    vector < AttributeCollection* > GetAllCollectionPtrs( int permit_empty_collection_type = -2, vector < string > inc_ids = {} );

    NameValData* GetAttributePtr( const string &id );

    // api callable methods below
    string SummarizeAttributes();
    string SummarizeAttributesAsTree();

    vector < string > FindAllAttributes();
    vector < string > FindAttributesByName( const string & search_str );
    string FindAttributeByName( const string & search_str, int index );
    string FindAttributeInCollection( const string & obj_id, const string & search_str, int index );
    vector < string > FindAttributeNamesInCollection( const string &collID );
    vector < string > FindAttributesInCollection( const string &collID );
    vector < string > FindAttributedObjects();

    int GetObjectType( const string & id );
    string GetObjectTypeName( const string & id );
    string GetObjectName( const string &attachID );
    string GetObjectParent( const string & id );

    string GetChildCollection( const string &attachID );
    string GetGeomSetCollection( const int & index );

    string GetAttributeName( const string &attrID );
    string GetAttributeID( const string &collID, const string &attributeName, int index );

    string GetAttributeDoc( const string &attrID );

    int GetAttributeType( const string &attrID );
    string GetAttributeTypeName( const string &attrID );

    vector < int > GetAttributeBoolVal( const string &attrID );
    vector < int > GetAttributeIntVal( const string &attrID );
    vector < double > GetAttributeDoubleVal( const string &attrID );
    vector < string > GetAttributeStringVal( const string &attrID );
    vector < string > GetAttributeParmID( const string &attrID );
    vector < double > GetAttributeParmVal( const string &attrID );
    vector < string > GetAttributeParmName( const string &attrID );
    vector < vec3d > GetAttributeVec3dVal( const string &attrID );
    vector < vector < int > > GetAttributeIntMatrixVal( const string &attrID );
    vector < vector < double > > GetAttributeDoubleMatrixVal( const string &attrID );

    void SetAttributeName( const string &attrID, const string &name, bool updateFlag = true );
    void SetAttributeDoc( const string &attrID, const string &doc, bool updateFlag = true );
    void SetAttributeBool( const string &attrID, int value, bool updateFlag = true );
    void SetAttributeInt( const string &attrID, int value, bool updateFlag = true );
    void SetAttributeDouble( const string &attrID, double value, bool updateFlag = true );
    void SetAttributeString( const string &attrID, const string &value, bool updateFlag = true );
    void SetAttributeParmID( const string &attrID, const string &value, bool updateFlag = true );
    void SetAttributeVec3d( const string &attrID, const vector< vec3d > &value, bool updateFlag = true );
    void SetAttributeIntMatrix( const string &attrID, const vector< vector< int > > &value, bool updateFlag = true );
    void SetAttributeDoubleMatrix( const string &attrID, const vector< vector< double > > &value, bool updateFlag = true );

    void DeleteAttribute( const string &attrID, bool updateFlag = true );
    void DeleteAttribute( const vector < string > &attrIDs, bool updateFlag = true );

    string GuiAddAttribute( AttributeCollection* ac_ptr, const int & attrType, bool updateFlag = true );
    string AddAttributeBool( const string &collID, const string &attributeName, int value, bool updateFlag = true, const string &id = string() );
    string AddAttributeInt( const string &collID, const string &attributeName, int value, bool updateFlag = true, const string &id = string() );
    string AddAttributeDouble( const string &collID, const string &attributeName, double value, bool updateFlag = true, const string &id = string() );
    string AddAttributeString( const string &collID, const string &attributeName, const string &value, bool updateFlag = true, const string &id = string() );
    string AddAttributeParm( const string &collID, const string &attributeName, const string &parmID, bool updateFlag = true, const string &id = string() );
    string AddAttributeVec3d( const string &collID, const string &attributeName, const vector < vec3d > &value, bool updateFlag = true, const string &id = string() );
    string AddAttributeIntMatrix( const string &collID, const string &attributeName, const vector< vector< int > > &value, bool updateFlag = true, const string &id = string() );
    string AddAttributeDoubleMatrix( const string &collID, const string &attributeName, const vector< vector< double > > &value, bool updateFlag = true, const string &id = string() );
    string AddAttributeGroup( const string &collID, const string &attributeName, bool updateFlag = true, const string &id = string() );
    string AddAttributeUtil( const string &collID, NameValData* attr, bool updateFlag = true );

    int CopyAttribute( const string &attr_id, bool updateFlag = true );
    void CutAttribute( const string &attr_id, bool updateFlag = true );
    vector < string > PasteAttribute( const string &obj_id, bool updateFlag = true );

    int CopyAttributeUtil( const vector < string > &attr_ids, bool updateFlag = true );
    void CutAttributeUtil( const string &attr_id, bool updateFlag = true );
    void CutAttributeUtil( const vector < string > &attr_ids, bool updateFlag = true );
    vector < string > PasteAttributeUtil( const string &obj_id, bool updateFlag = true );
    vector < string > PasteAttributeUtil( const vector < string > &obj_ids, bool updateFlag = true );

    void SetAttributeProtection( const string &attr_id, bool protect_flag = true );

    //tree things
    vector < vector < vector < string > > > GetAttrTreeVec( const string & root_id = "", const vector<string> &inc_ids = {}, bool exclude_clipboard = true, int attr_type = vsp::INVALID_TYPE, int attach_type = vsp::ATTROBJ_FREE, const string & attr_search = "", bool case_sens = false );

    vector < vector < string > > GetCollParentVecs( const vector < vector < string > > & vecs_ids );
    vector < string > GetCollParentVec( const vector < string > & vec_ids );
    static string GetName( const string & id, bool return_name_input = true );
    bool AttrInClipboard( NameValData* attr );
    bool VecInClipboard( const vector < string > & stringVec );
    bool VecInClipboard( const vector < vector < string > > & stringVec );
    bool VecInClipboard( const vector < vector < vector < string > > > & stringVec );
    bool CheckTreeVecID( const string & id, const string & check_id = "" );
    vector <  string > GetTreeVec( const string & id, const string & root_id = "" );
    static bool AllUpper( const string & str );
    static string ToLower( const string & str );

    static vector < string > ExtendStringVector( vector < string > vec, const vector<string> &vec_add, const string &string_root = "", bool skip_empty_strings = true );
    static vector < vector < string > > TransposeExtendStringVector( const vector<string> &base_vec, const vector<string> &vec_add );
    static vector < vector < string > > ExtendNestedStringVector( vector < vector < string > > vec, vector < vector < string > > vec_add );

protected:
    NameValData* RemoveAttribute( const string &attrID, bool updateFlag = true );
    vector < NameValData* > RemoveAttribute( const vector < string > &attrIDs, bool updateFlag = true );

    static bool NestedVecSorter( const vector < vector < string > > & v1, const vector < vector < string > > & v2 );

    void WypeClipboard();
    int CheckCopyError( const vector < string > &attr_ids );

    map< string, AttributeCollection*> m_AttrCollMap;
    map< string, NameValData*> m_AttrPtrMap;

    vector < NameValData* > m_AttrClipboard;
    vector < bool > m_DirtyFlags;

private:
    AttributeMgrSingleton();
};

class TextTreeNode
{
public:
    TextTreeNode( const string &id, const string &name, int depth )
    {
        m_ID = id;
        m_Name = name;
        m_Depth = depth;
        m_IndentQty = 2;
        m_Children.clear();
    }

    ~TextTreeNode()
    {
        map< string, TextTreeNode*>::iterator child_iter;
        for ( child_iter = m_Children.begin(); child_iter != m_Children.end(); ++child_iter )
        {
            delete child_iter->second;
        }

        m_Children.clear();
    }

    void SetName( const string &name )
    {
        m_Name = name;
    }
    void SetID( const string &id )
    {
        m_ID = id;
    }
    void SetDepth( int depth )
    {
        m_Depth = depth;
    }
    void AddChild( const string & child_id, const string & child_name )
    {
        if ( m_Children.count( child_id ) == 0 )
        {
            TextTreeNode* child = new TextTreeNode( child_id, child_name, m_Depth+1 );
            m_Children.insert( {child_id, child} );
        }
    }

    bool HasChild( string child_id )
    {
        return m_Children.count( child_id );
    }

    string GetName()
    {
        return m_Name;
    }
    string GetID()
    {
        return m_ID;
    }
    int GetDepth()
    {
        return m_Depth;
    }
    TextTreeNode* GetChild( const string & child_id )
    {
        TextTreeNode* child = nullptr;
        if ( m_Children.count( child_id ) )
        {
            child = m_Children.at( child_id );
        }
        return child;
    }

    string PrintMe( const string &prior_lines = "" );

protected:
    map < string, TextTreeNode* > m_Children;
    string m_ID;
    string m_Name;
    int m_Depth;
    int m_IndentQty;
};

#define AttributeMgr AttributeMgrSingleton::getInstance()

#endif