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
#include <windows.h>
#endif

#include "Vehicle.h"
#include "VspUtil.h"
#include "StlHelper.h"

#include "IDMgr.h"
#include "ParmMgr.h"

//==== Default Results Data ====//
NameValData::NameValData()
{
    Init( "Undefined" );
}

//==== Copy Constructor ====//
NameValData::NameValData( NameValData* nvd )
{
    CopyFrom( nvd );
}

//==== Constructor With Name =====//
NameValData::NameValData( const string & name )
{
    Init( name );
}

//==== Constructors With Name & Data =====//
NameValData::NameValData( const string & name, const bool & b_data, const string & doc, const string & id )
{
    Init( name, vsp::BOOL_DATA, id );
    m_IntData.push_back( b_data );
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const int & i_data, const string & doc, const string & id )
{
    Init( name, vsp::INT_DATA, id );
    m_IntData.push_back( i_data );
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const double & d_data, const string & doc, const string & id )
{
    Init( name, vsp::DOUBLE_DATA, id );
    m_DoubleData.push_back( d_data );
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const string & s_data, const string & doc, const string & id )
{
    Init( name, vsp::STRING_DATA, id );
    m_StringData.push_back( s_data );
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const vec3d & v_data, const string & doc, const string & id )
{
    Init( name, vsp::VEC3D_DATA, id );
    m_Vec3dData.push_back( v_data );
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const NameValCollection &c_data, const string & doc, const string & id )
{
    Init( name, vsp::ATTR_COLLECTION_DATA, id );
    m_NameValCollectionData.push_back(c_data);
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const vector< int > & i_data, const string & doc, const string & id )
{
    Init( name, vsp::INT_DATA, id );
    m_IntData = i_data;
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const vector< double > & d_data, const string & doc, const string & id )
{
    Init( name, vsp::DOUBLE_DATA, id );
    m_DoubleData = d_data;
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const vector< string > & s_data, const string & doc, const string & id )
{
    Init( name, vsp::STRING_DATA, id );
    m_StringData = s_data;
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const vector< vec3d > & v_data, const string & doc, const string & id )
{
    Init( name, vsp::VEC3D_DATA, id );
    m_Vec3dData = v_data;
    m_Doc = doc;
}

NameValData::NameValData( const string &name, const vector< vector< int > > &imat_data, const string & doc, const string & id )
{
    Init( name, vsp::INT_MATRIX_DATA, id );
    m_IntMatData = imat_data;
    m_Doc = doc;
}

NameValData::NameValData( const string &name, const vector< vector< double > > &dmat_data, const string & doc, const string & id )
{
    Init( name, vsp::DOUBLE_MATRIX_DATA, id );
    m_DoubleMatData = dmat_data;
    m_Doc = doc;
}

NameValData::NameValData( const string & name, const vector< NameValCollection > &c_data, const string & doc, const string & id )
{
    Init( name, vsp::NAMEVAL_COLLECTION_DATA, id );
    m_NameValCollectionData = c_data;
    m_Doc = doc;
}

void NameValData::Init( const string & name, int type, const string & id )
{
    m_Name = name;
    m_Type = type;

    m_ID = id;
    if ( id.empty() )
    {
        m_ID = GenerateID();
    }

    m_ProtectFlag = false;
    m_AttachID = "NONE";
    m_AttributeEventGroup = vsp::ATTR_GROUP_NONE;
}

string NameValData::GetTypeName() const{
    return GetTypeName( m_Type );
}

string NameValData::GetTypeName( int type, bool capitalize, bool short_name )
{
    string string_out;
    string string_long;
    string string_mini;

    switch ( type ){
        case vsp::INVALID_TYPE:
            string_long = "invalid";
            string_mini = "N/A";
            break;
        case vsp::BOOL_DATA:
            string_long = "boolean";
            string_mini = "bool";
            break;
        case vsp::INT_DATA:
            string_long = "integer";
            string_mini = "int";
            break;
        case vsp::DOUBLE_DATA:
            string_long = "double";
            string_mini = "dbl";
            break;
        case vsp::STRING_DATA:
            string_long = "string";
            string_mini = "str";
            break;
        case vsp::PARM_REFERENCE_DATA:
            string_long = "parm";
            string_mini = "parm";
            break;
        case vsp::VEC3D_DATA:
            string_long = "vec3d";
            string_mini = "vec3d";
            break;
        case vsp::INT_MATRIX_DATA:
            string_long = "int matrix";
            string_mini = "imat";
            break;
        case vsp::DOUBLE_MATRIX_DATA:
            string_long = "double matrix";
            string_mini = "dmat";
            break;
        case vsp::NAMEVAL_COLLECTION_DATA:
            string_long = "nameval collection";
            string_mini = "nvc";
            break;
        case vsp::ATTR_COLLECTION_DATA:
            string_long = "attribute group";
            string_mini = "grp";
            break;
        default:
            string_long = "";
            string_mini = "";
            break;
    }

    if ( short_name )
    {
        string_out = string_mini;
    }
    else
    {
        string_out = string_long;
    }

    if ( capitalize )
    {
        string_out[0] = toupper(string_out[0]);
        int len = strlen(string_out.c_str())-1;
        for (int i = 0; i < len; i++)
        {
            if (isspace(string_out[i]))
            {
                string_out[i+1] = toupper(string_out[i+1]);
            }
        }
    }
    return string_out;
}

bool NameValData::GetBool( int i ) const
{
    if ( i >= 0 && i < ( int )m_IntData.size() )
    {
        return m_IntData[i];
    }
    return false;
}

int NameValData::GetInt( int i ) const
{
    if ( i >= 0 && i < ( int )m_IntData.size() )
    {
        return m_IntData[i];
    }
    return 0;
}

int NameValData::GetInt( int row, int col ) const
{
    if ( row >= 0 && row < ( int )m_IntMatData.size() )
    {
        if ( col >= 0 && col < ( int )m_IntMatData[row].size() )
        {
            return m_IntMatData[row][col];
        }
    }
    return 0;
}

double NameValData::GetDouble( int i ) const
{
    if ( i >= 0 && i < ( int )m_DoubleData.size() )
    {
        return m_DoubleData[i];
    }
    return 0;
}

double NameValData::GetDouble( int row, int col ) const
{
    if ( row >= 0 && row < ( int )m_DoubleMatData.size() )
    {
        if ( col >= 0 && col < ( int )m_DoubleMatData[row].size() )
        {
            return m_DoubleMatData[row][col];
        }
    }
    return 0;
}

string NameValData::GetString( int i ) const
{
    if ( i >= 0 && i < ( int )m_StringData.size() )
    {
        return m_StringData[i];
    }
    return string();
}

string NameValData::GetParmID( int i ) const
{
    if ( i >= 0 && i < ( int )m_ParmIDData.size() )
    {
        return m_ParmIDData[i];
    }
    return string();
}

vec3d NameValData::GetVec3d( int i ) const
{
    if ( i >= 0 && i < ( int )m_Vec3dData.size() )
    {
        return m_Vec3dData[i];
    }
    return vec3d();
}

NameValCollection NameValData::GetNameValCollection( int i ) const
{
    NameValCollection default_coll;
    if ( i >= 0 && i < ( int )m_NameValCollectionData.size() )
    {
        return m_NameValCollectionData[i];
    }
    return default_coll;
}

NameValCollection* NameValData::GetNameValCollectionPtr( int i )
{
    if ( i >= 0 && i < ( int )m_NameValCollectionData.size() )
    {
        return &m_NameValCollectionData[i];
    }
    return nullptr;
}

string NameValData::GenerateID()
{
    return GenerateRandomID( vsp::ID_LENGTH_ATTR );
}

void NameValData::ChangeID( string id ) //only for attributes
{
    m_ID = id;
}

//==== Copy NameValData ====//
void NameValData::CopyFrom( NameValData* nvd, vector < string > name_vector )
{
    xmlNodePtr root = xmlNewNode( NULL, ( const xmlChar * )"Vsp_Attributes" );

    nvd->EncodeXml( root );
    xmlNodePtr attr_node = XmlUtil::GetNode( root, "Attribute", 0 );
    DecodeXml( attr_node, name_vector );

    xmlFreeNode( root );
    // xmlFreeNode( attr_node );

}

string NameValData::TruncateString( string str, int len )
{
    if ( str.length() > len )
    {
        str.resize( len );
        return str.append( "..." );
    }
    return str;
}

void NameValData::EncodeXml( xmlNodePtr & node )
{
    string attrXmlName = "Attribute";
    xmlNodePtr dnode = xmlNewChild( node, NULL, ( const xmlChar * )attrXmlName.c_str(), NULL );
    XmlUtil::SetStringProp( dnode, "ID", m_ID );
    XmlUtil::SetStringProp( dnode, "AttachID", m_AttachID );
    XmlUtil::SetStringProp( dnode, "Name", m_Name );
    XmlUtil::SetIntProp( dnode, "Type", m_Type );
    XmlUtil::SetIntProp( dnode, "EventGroup", m_AttributeEventGroup );
    if ( m_Type == vsp::BOOL_DATA )
    {
        XmlUtil::SetIntProp( dnode, "BoolData", GetBool( 0 ) );
    }
    else if ( m_Type == vsp::INT_DATA )
    {
        XmlUtil::SetIntProp( dnode, "IntData", GetInt( 0 ) );
    }
    else if ( m_Type == vsp::DOUBLE_DATA )
    {
        XmlUtil::SetDoubleProp( dnode, "DblData", GetDouble( 0 ) );
    }
    else if ( m_Type == vsp::STRING_DATA )
    {
        XmlUtil::SetStringProp( dnode, "StrData", GetString( 0 ) );
    }
    else if ( m_Type == vsp::PARM_REFERENCE_DATA )
    {
        XmlUtil::SetStringProp( dnode, "ParmIDData", GetParmID( 0 ) );
    }
    else if ( m_Type == vsp::VEC3D_DATA )
    {
        XmlUtil::AddVectorVec3dNode( dnode, "Vec3dData", GetVec3dData() );
    }
    else if ( m_Type == vsp::INT_MATRIX_DATA )
    {
        int nrow = 0;
        int ncol = 0;

        nrow = m_IntMatData.size();
        if ( nrow )
        {
            ncol = m_IntMatData[0].size();
        }

        std::vector < int > flatIntMatData;

        for ( int i = 0; i != m_IntMatData.size(); ++i )
        {
            for ( int j = 0; j != m_IntMatData[i].size(); ++j )
            {
                flatIntMatData.push_back( m_IntMatData[i][j] );
            }
        }

        XmlUtil::SetIntProp( dnode, "Rows", nrow );
        XmlUtil::SetIntProp( dnode, "Cols", ncol );
        XmlUtil::AddVectorIntNode( dnode, "IntMatData", flatIntMatData );
    }
    else if ( m_Type == vsp::DOUBLE_MATRIX_DATA )
    {
        int nrow = 0;
        int ncol = 0;

        nrow = m_DoubleMatData.size();
        if ( nrow )
        {
            ncol = m_DoubleMatData[0].size();
        }

        std::vector < double > flatDblMatData;

        for ( int i = 0; i != m_DoubleMatData.size(); ++i )
        {
            for ( int j = 0; j != m_DoubleMatData[i].size(); ++j )
            {
                flatDblMatData.push_back( m_DoubleMatData[i][j] );
            }
        }

        XmlUtil::SetIntProp( dnode, "Rows", nrow );
        XmlUtil::SetIntProp( dnode, "Cols", ncol );
        XmlUtil::AddVectorDoubleNode( dnode, "DoubleMatData", flatDblMatData );

    }

    XmlUtil::SetStringProp( dnode, "Desc", m_Doc );
    XmlUtil::SetIntProp( dnode, "Protection", m_ProtectFlag );
}

void NameValData::DecodeXml( xmlNodePtr & node, vector < string > name_vector )
{
    if ( node )
    {
        string default_str = "None";
        int default_int = 0;
        double default_dbl = 0.;

        int i_attr = 0;

        bool protect_flag = XmlUtil::FindIntProp( node, "Protection", default_int );

        string ID = IDMgr.RemapID( XmlUtil::FindStringProp( node, "ID", default_str ) );
        string attachID = IDMgr.RemapID( XmlUtil::FindStringProp( node, "AttachID", default_str ) );

        m_Name = XmlUtil::FindStringProp( node, "Name", default_str );
        m_Type = XmlUtil::FindIntProp( node, "Type", default_int );
        m_Doc = XmlUtil::FindStringProp( node, "Desc", default_str );
        m_AttributeEventGroup = XmlUtil::FindIntProp( node, "EventGroup", vsp::ATTR_GROUP_NONE );

        ChangeID( ID );
        SetAttrAttach( attachID );
        SetProtection( protect_flag );

        if ( m_Type == vsp::BOOL_DATA )
        {
            bool boolData = XmlUtil::FindIntProp( node, "BoolData", default_int );
            SetBoolData( { boolData } );
        }
        else if ( m_Type == vsp::INT_DATA )
        {
            int intData = XmlUtil::FindIntProp( node, "IntData", default_int );
            SetIntData( { intData } );
        }
        else if ( m_Type == vsp::DOUBLE_DATA )
        {
            double doubleData = XmlUtil::FindDoubleProp( node, "DblData", default_dbl );
            SetDoubleData( { doubleData } );
        }
        else if ( m_Type == vsp::STRING_DATA )
        {
            string stringData = XmlUtil::FindStringProp( node, "StrData", default_str );
            SetStringData( { stringData } );
        }
        else if ( m_Type == vsp::PARM_REFERENCE_DATA )
        {
            string parmIDData = XmlUtil::FindStringProp( node, "ParmIDData", default_str );
            SetParmIDData( { parmIDData } );
        }
        else if ( m_Type == vsp::VEC3D_DATA )
        {
            string attrXmlName = "Vec3dData";
            xmlNodePtr vec3dNode = XmlUtil::GetNode( node, attrXmlName.c_str(), 0 );
            SetVec3dData( XmlUtil::GetVectorVec3dNode( vec3dNode ) );
        }
        else if ( m_Type == vsp::INT_MATRIX_DATA )
        {
            int nrow = XmlUtil::FindIntProp( node, "Rows", default_int );
            int ncol = XmlUtil::FindIntProp( node, "Cols", default_int );

            std::vector < int > flatMatData;
            std::vector < std::vector < int > > matData;

            flatMatData = XmlUtil::ExtractVectorIntNode( node, "IntMatData" );

            int k = 0;
            for ( int i = 0; i != nrow; ++i )
            {
                matData.push_back( {} );
                for ( int j = 0; j != ncol; ++j )
                {
                    matData[i].push_back( flatMatData[k] );
                    ++k;
                }
            }
            SetIntMatData( matData );
        }
        else if ( m_Type == vsp::DOUBLE_MATRIX_DATA )
        {
            int nrow = XmlUtil::FindIntProp( node, "Rows", default_int );
            int ncol = XmlUtil::FindIntProp( node, "Cols", default_int );

            std::vector < double > flatMatData;
            std::vector < std::vector < double > > matData;

            flatMatData = XmlUtil::ExtractVectorDoubleNode( node, "DoubleMatData" );

            int k = 0;
            for ( int i = 0; i != nrow; ++i )
            {
                matData.push_back( {} );
                for ( int j = 0; j != ncol; ++j )
                {
                    matData[i].push_back( flatMatData[k] );
                    ++k;
                }
            }
            SetDoubleMatData( matData );
        }
    }
}

string NameValData::GetAsString( bool inline_data_flag )
{
    char str[255];
    string outstring;

    int vec_row = 0;
    int mat_row = 0;
    int mat_col = 0;
    int ac_size = 0;
    std::vector < NameValData* > nvd_vec;

    switch ( m_Type ){
        case vsp::INVALID_TYPE:
            outstring = "invalid";
            break;
        case vsp::BOOL_DATA:
            for ( unsigned int i = 0; i < m_IntData.size(); i++ )
            {
                if ( m_IntData[i] )
                {
                    snprintf( str, sizeof( str ), "True" );
                }
                else
                {
                    snprintf( str, sizeof( str ), "False" );
                }
                outstring.append( str );
            }
            break;
        case vsp::INT_DATA:
            for ( unsigned int i = 0; i < m_IntData.size(); i++ )
            {
                snprintf( str, sizeof( str ), "%d ", m_IntData[i] );
                outstring.append( str );
            }
            break;
        case vsp::DOUBLE_DATA:
            for ( unsigned int i = 0; i < m_DoubleData.size(); i++ )
            {
                snprintf( str, sizeof( str ), "%g ", m_DoubleData[i] );
                outstring.append( str );
            }
            break;
        case vsp::STRING_DATA:
            for ( unsigned int i = 0; i < m_StringData.size(); i++ )
            {
                outstring.append( m_StringData[i] + " " );
            }
            break;
        case vsp::PARM_REFERENCE_DATA:
            if ( m_ParmIDData.empty() )
            {
                outstring.append( string("-") );
            }
            for ( unsigned int i = 0; i < m_ParmIDData.size(); i++ )
            {
                Parm* p = ParmMgr.FindParm( m_ParmIDData[i] );
                if ( p )
                {
                    outstring.append( to_string( p->Get() ) );
                }
                else if ( !m_ParmIDData[i].empty() )
                {
                    outstring.append( string("INVALID PARM ID") );
                }
                else
                {
                    outstring.append( string("-") );
                }
            }
            break;
        case vsp::VEC3D_DATA:
            if ( !inline_data_flag )
            {
                for ( unsigned int i = 0; i < m_Vec3dData.size(); i++ )
                {
                    snprintf( str, sizeof( str ), "%g,%g,%g ", m_Vec3dData[i].x(), m_Vec3dData[i].y(), m_Vec3dData[i].z() );
                    outstring.append( str );
                }
            }
            else
            {
                vec_row = m_Vec3dData.size();
                if ( vec_row < 2 )
                {
                    snprintf( str, sizeof( str ), "%g,%g,%g ", m_Vec3dData[0].x(), m_Vec3dData[0].y(), m_Vec3dData[0].z() );
                }
                else
                {
                    snprintf( str, sizeof( str ), "(%d rows)", vec_row );
                }
                outstring.append( str );
            }
            break;
        case vsp::INT_MATRIX_DATA:
            if ( !inline_data_flag )
            {
                for ( unsigned int row = 0; row < m_IntMatData.size(); row++ )
                {
                    for ( unsigned int col = 0; col < m_IntMatData[row].size(); col++ )
                    {
                        snprintf( str, sizeof( str ), "%d ", m_IntMatData[row][col] );
                        outstring.append( str );
                    }
                    if ( row < m_IntMatData.size() - 1 )
                    {
                        snprintf( str, sizeof( str ), "\n\t\t%-20s \t\t \t", "");
                        outstring.append( str );
                    }
                }
            }
            else
            {
                mat_row = m_IntMatData.size();
                if ( mat_row )
                {
                    mat_col = m_IntMatData[0].size();
                }
                snprintf( str, sizeof( str ), "(%d x %d)", mat_row, mat_col );
                outstring.append( str );
            }
            break;
        case vsp::DOUBLE_MATRIX_DATA:
            if ( !inline_data_flag )
            {
                for ( unsigned int row = 0; row < m_DoubleMatData.size(); row++ )
                {
                    for ( unsigned int col = 0; col < m_DoubleMatData[row].size(); col++ )
                    {
                        snprintf( str, sizeof( str ), "%.*e ", DBL_DIG + 3, m_DoubleMatData[row][col] );
                        outstring.append( str );
                    }
                    if ( row < m_DoubleMatData.size() - 1 )
                    {
                        snprintf( str, sizeof( str ), "\n\t\t%-20s \t\t \t", "");
                        outstring.append( str );
                    }
                }
            }
            else
            {
                mat_row = m_DoubleMatData.size();
                if ( mat_row )
                {
                    mat_col = m_DoubleMatData[0].size();
                }
                snprintf( str, sizeof( str ), "(%d x %d)", mat_row, mat_col );
                outstring.append( str );
            }
            break;
        default:
            outstring = "unknown";
            break;
    }

    return outstring;
}


//==== Update Attachment IDs ====//

// define attached object ID
void NameValData::SetAttrAttach( string attachID )
{
    m_AttachID = attachID;
}



//======================================================================================//
//======================================================================================//
//======================================================================================//


NameValCollection::NameValCollection()
{
    m_ID = GenerateID();
    m_DataMap.clear();
}

NameValCollection::NameValCollection( const string & name, const string & id, const string & doc )
{
    m_Name = name;
    m_ID = id;
    m_Doc = doc;
    m_DataMap.clear();
}

NameValCollection::~NameValCollection()
{
    vector < NameValData* > nvd_ptrs = GetAllPtrs();
    for ( int i = 0; i != nvd_ptrs.size(); ++i )
    {
        delete nvd_ptrs[i];
    }
}

string NameValCollection::GenerateID()
{
    return GenerateRandomID( vsp::ID_LENGTH_ATTRCOLL );
}

//==== Add Data To Results - Can Have Data With The Same Name =====//
void NameValCollection::Add( const NameValData & d )
{
    NameValData* nvd_copy = new NameValData( d );

    //==== Find Name ====//
    string name = nvd_copy->GetName();

    map< string, vector< NameValData* > >::iterator iter = m_DataMap.find( name );
    if ( iter != m_DataMap.end() )     // Check For Duplicates
    {
        iter->second.push_back( nvd_copy );
    }
    else
    {
        m_DataMap[name].push_back( nvd_copy );
    }
}

void NameValCollection::Add(const vector<vector<vec3d> > & d, const string &prefix, const string &doc )
{
    string names[] = { prefix + "x", prefix + "y", prefix + "z"};
    for ( int dim = 0; dim < 3; dim++ )
    {
        vector< vector< double > > arr;
        arr.reserve( d.size() );
        for ( int i = 0; i < ( int )d.size(); i++ )
        {
            vector<double> row;
            row.reserve( d[i].size() );
            for ( int j = 0; j < ( int )d[i].size(); j++ )
            {
                row.push_back( d[i][j].v[dim] );
            }
            arr.push_back( row );
        }

        Add( NameValData( names[dim], arr, doc ) );
    }
}

//==== Find Res Data Given Name and Index ====//
NameValData* NameValCollection::FindPtr( const string & name, int index )
{
    map< string, vector< NameValData* > >::iterator iter = m_DataMap.find( name );

    if ( iter != m_DataMap.end() )
    {
        if ( index >= 0 && index < ( int )( iter->second.size() ) )
        {
            return iter->second[index];
        }
    }

    // due to prior implementation as a non-ptr finder, need to return a pointer to a default NameValData if not found in NameValCollection
    Vehicle* veh = VehicleMgr.GetVehicle();
    return veh->GetBlankNvd();
}

//==== Get Data Names ====//
vector< NameValData* > NameValCollection::GetAllPtrs()
{
    vector< NameValData* > ptr_vec;
    map< string, vector< NameValData* > >::iterator iter;

    for ( iter = m_DataMap.begin(); iter != m_DataMap.end(); ++iter )
    {
        for ( int i = 0; i != iter->second.size(); ++i )
        {
            ptr_vec.push_back( iter->second[i] );
        }
    }
    return ptr_vec;
}

//==== Get Data Names ====//
vector< string > NameValCollection::GetAllDataNames()
{
    vector< string > name_vec;
    map< string, vector< NameValData* > >::iterator iter;

    for ( iter = m_DataMap.begin() ; iter != m_DataMap.end() ; ++iter )
    {
        for ( int j = 0; j != iter->second.size(); ++j ) 
        {
            name_vec.push_back( iter->first );
        }
    }
    return name_vec;
}

//==== Get Number of Data Entries For This Name ====//
int NameValCollection::GetNumData( const string & name )
{
    map< string, vector< NameValData* > >::iterator iter = m_DataMap.find( name );
    if ( iter == m_DataMap.end() )
    {
        return 0;
    }

    return iter->second.size();
}


//======================================================================================//
//======================================================================================//
//======================================================================================//

Results::Results( const string & name, const string & id, const string & doc ) : NameValCollection( name, id, doc )
{
    SetDateTime();          // Set Time Stamp
}

//===== Find Current Time and Set Stamp =====//
void Results::SetDateTime()
{
    m_Timestamp = time( 0 ); // get time now
    struct tm * now = localtime( &m_Timestamp );

    m_Year = now->tm_year + 1900;
    m_Month = now->tm_mon + 1;
    m_Day = now->tm_mday;
    m_Hour = now->tm_hour;
    m_Min = now->tm_min;
    m_Sec = now->tm_sec;
}

//===== Write A CSV File With Everything =====//
void Results::WriteCSVFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        WriteCSVFile( fid );
        fclose( fid );          // Close File
    }
}

void Results::WriteCSVFile( FILE* fid )
{
    if ( fid )
    {

        fprintf( fid, "Results_Name,%s\n", m_Name.c_str() );
        fprintf( fid, "Results_Timestamp,%lld\n", m_Timestamp );
        fprintf( fid, "Results_Date,%d,%d,%d\n", m_Month, m_Day, m_Year );
        fprintf( fid, "Results_Time,%d,%d,%d\n", m_Hour, m_Min, m_Sec );

        map< string, vector< NameValData* > >::iterator iter;
        for ( iter = m_DataMap.begin() ; iter != m_DataMap.end() ; ++iter )
        {
            for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
            {
                fprintf( fid, "%s", iter->second[i]->GetName().c_str() );
                if ( iter->second[i]->GetType() == vsp::INT_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i]->GetIntData().size() ; d++ )
                    {
                        fprintf( fid, ",%d", iter->second[i]->GetIntData()[d] );
                    }
                }
                else if ( iter->second[i]->GetType() == vsp::DOUBLE_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i]->GetDoubleData().size() ; d++ )
                    {
                        fprintf( fid, ",%.*e", DBL_DIG + 3, iter->second[i]->GetDoubleData()[d] );
                    }
                }
                else if ( iter->second[i]->GetType() == vsp::INT_MATRIX_DATA )
                {
                    vector< vector< int > > current_int_mat_val = iter->second[i]->GetIntMatData();
                    for ( unsigned int row = 0; row < current_int_mat_val.size(); row++ )
                    {
                        for ( unsigned int col = 0; col < current_int_mat_val[row].size(); col++ )
                        {
                            fprintf( fid, ",%d", current_int_mat_val[row][col] );
                        }
                        if ( row < current_int_mat_val.size() - 1 )
                        {
                            fprintf( fid, "\n ");
                        }
                    }
                }
                else if ( iter->second[i]->GetType() == vsp::DOUBLE_MATRIX_DATA )
                {
                    vector< vector< double > > current_double_mat_val = iter->second[i]->GetDoubleMatData();
                    for ( unsigned int row = 0; row < current_double_mat_val.size(); row++ )
                    {
                        for ( unsigned int col = 0; col < current_double_mat_val[row].size(); col++ )
                        {
                            fprintf( fid, ",%.*e", DBL_DIG + 3, current_double_mat_val[row][col] );
                        }
                        if ( row < current_double_mat_val.size() - 1 )
                        {
                            fprintf( fid, "\n ");
                        }
                    }
                }
                else if ( iter->second[i]->GetType() == vsp::STRING_DATA )
                {
                    // If this is a "ResultsVec" wrapper result replace result UIDs with result names
                    if ( strcmp( iter->second[i]->GetName().c_str(), "ResultsVec" ) == 0 )
                    {
                        for ( int d = 0; d < (int)iter->second[i]->GetStringData().size(); d++ )
                        {
                            fprintf( fid, ",%s", ResultsMgr.FindResultsPtr(iter->second[i]->GetStringData()[d])->GetName().c_str() );
                        }
                    }
                    else
                    {
                        for ( int d = 0; d < (int)iter->second[i]->GetStringData().size(); d++ )
                        {
                            fprintf( fid, ",%s", iter->second[i]->GetStringData()[d].c_str() );
                        }
                    }
                }
                else if ( iter->second[i]->GetType() == vsp::VEC3D_DATA )
                {
                    for ( int d = 0 ; d < ( int )iter->second[i]->GetVec3dData().size() ; d++ )
                    {
                        vec3d v = iter->second[i]->GetVec3dData()[d];
                        fprintf( fid, ",%.*e,%.*e,%.*e", DBL_DIG + 3, v.x(), DBL_DIG + 3, v.y(), DBL_DIG + 3, v.z() );
                    }
                }
                fprintf( fid, "\n" );
            }
        }

        // Loop again to recursively call WriteCSV() if this result contains a "ResultsVec" wrapper result
        for ( iter = m_DataMap.begin(); iter != m_DataMap.end(); ++iter )
        {
            for ( int i = 0; i < (int)iter->second.size(); i++ )
            {
                if ( (iter->second[i]->GetType() == vsp::STRING_DATA) && (strcmp( iter->second[i]->GetName().c_str(), "ResultsVec" ) == 0) )
                {
                    for ( int d = 0; d < (int)iter->second[i]->GetStringData().size(); d++ )
                    {
                        Results * res = ResultsMgr.FindResultsPtr( iter->second[i]->GetStringData()[d] );
                        if ( res )
                        {
                            res->WriteCSVFile( fid );
                        }
                    }
                }
            }
        }   // end recursion

    }
}

//==== Write The Mass Prop Results ====//
void Results::WriteMassProp( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        int num_removed =  FindPtr( "Num_Degen_Triangles_Removed" )->GetInt( 0 );
        if ( num_removed != 0 )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", num_removed );
        }

        int num_open_remove = FindPtr( "Num_Open_Meshes_Removed" )->GetInt( 0 );
        if ( num_open_remove != 0 )
        {
            fprintf( fid, "WARNING: %d open meshes remove\n", num_open_remove );

            vector < string > remnames = FindPtr( "Meshes_Removed_Names" )->GetStringData();
            for ( int i = 0; i < remnames.size(); i++ )
            {
                fprintf( fid, "     Removed: %s\n", remnames[i].c_str() );
            }
        }

        int num_open_merge = FindPtr( "Num_Open_Meshes_Merged" )->GetInt( 0 );
        if ( num_open_merge != 0 )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", num_open_merge );

            vector < string > mernames = FindPtr( "Meshes_Merged_Names" )->GetStringData();
            for ( int i = 0; i < mernames.size(); i++ )
            {
                fprintf( fid, "     Merged: %s\n", mernames[i].c_str() );
            }
        }

        fprintf( fid, "...Mass Properties...\n" );
        int num_comps = FindPtr( "Num_Comps" )->GetInt( 0 );
        fprintf( fid, "%d Num Comps\n", num_comps );
        fprintf( fid, "%d Total Num Meshes\n", FindPtr( "Num_Total_Meshes" )->GetInt( 0 ) );
        fprintf( fid, "%d Total Num Tris\n", FindPtr( "Num_Total_Tris" )->GetInt( 0 ) );
        fprintf( fid, "\n" );
        fprintf( fid, "%.15e             Total Mass\n", FindPtr( "Total_Mass" )->GetDouble( 0 ) );
        vec3d total_cg = FindPtr( "Total_CG" )->GetVec3d( 0 );
        fprintf( fid, "%.15e %.15e %.15e       Center of Gravity\n", total_cg.x(), total_cg.y(), total_cg.z() );
        double ixx =  FindPtr( "Total_Ixx" )->GetDouble( 0 );
        double iyy =  FindPtr( "Total_Iyy" )->GetDouble( 0 );
        double izz =  FindPtr( "Total_Izz" )->GetDouble( 0 );
        fprintf( fid, "%.15e %.15e %.15e       Ixx, Iyy, Izz\n", ixx, iyy, izz );
        double ixy =  FindPtr( "Total_Ixy" )->GetDouble( 0 );
        double ixz =  FindPtr( "Total_Ixz" )->GetDouble( 0 );
        double iyz =  FindPtr( "Total_Iyz" )->GetDouble( 0 );
        fprintf( fid, "%.15e %.15e %.15e       Ixy, Ixz, Iyz\n", ixy, ixz, iyz );
        fprintf( fid, "%.15e             Volume\n", FindPtr( "Total_Volume" )->GetDouble( 0 ) );

        fprintf( fid, "\n" );
        fprintf( fid, "Name\tMass\tcgX\tcgY\tcgZ\tIxx\tIyy\tIzz\tIxy\tIxz\tIyz\tVolume\n" );

        for ( int i = 0 ; i < num_comps; i++ )
        {
            string comp_Id = FindPtr( "Comp_ID" )->GetString( i );
            string comp_name = FindPtr( "Comp_Name" )->GetString( i );
            double comp_mass = FindPtr( "Comp_Mass" )->GetDouble( i );
            vec3d comp_cg = FindPtr( "Comp_CG" )->GetVec3d( i );
            double compIxx = FindPtr( "Comp_Ixx" )->GetDouble( i );
            double compIyy = FindPtr( "Comp_Iyy" )->GetDouble( i );
            double compIzz = FindPtr( "Comp_Izz" )->GetDouble( i );
            double compIxy = FindPtr( "Comp_Ixy" )->GetDouble( i );
            double compIyz = FindPtr( "Comp_Iyz" )->GetDouble( i );
            double compIxz = FindPtr( "Comp_Ixz" )->GetDouble( i );
            double compVol = FindPtr( "Comp_Vol" )->GetDouble( i );

            fprintf( fid, "%s\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\n",
                     comp_name.c_str(), comp_mass, comp_cg.x(), comp_cg.y(), comp_cg.z(),
                     compIxx, compIyy, compIzz, compIxy, compIxz, compIyz, compVol );
        }


        fprintf( fid, "Name\tMass\tcgX\tcgY\tcgZ\tIxx\tIyy\tIzz\tIxy\tIxz\tIyz\tVolume\n" );
        fprintf( fid, "%s\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\n",
                 "Totals", FindPtr( "Total_Mass" )->GetDouble( 0 ), total_cg.x(), total_cg.y(), total_cg.z(),
                 ixx, iyy, izz, ixy, ixz, iyz, FindPtr( "Total_Volume" )->GetDouble( 0 ) );



        fprintf( fid, "\n...Filling Mass Properties (Volume Only -- No Shell or Point Mass)...\n" );

        int num_slice = FindPtr( "Num_Fill_Slice" )->GetInt( 0 );

        fprintf( fid, "%d Num Slice\n", num_slice );

        fprintf( fid, "\n" );
        fprintf( fid, "Slice\tMass\tcgX\tcgY\tcgZ\tIxx\tIyy\tIzz\tIxy\tIxz\tIyz\tVolume\n" );

        for ( int i = 0 ; i < num_slice ; i++ )
        {
            double fillslice = FindPtr( "Fill_Slice" )->GetDouble( i );
            double fillmass = FindPtr( "Fill_Mass" )->GetDouble( i );
            vec3d fillcg = FindPtr( "Fill_CG" )->GetVec3d( i );
            double fillIxx = FindPtr( "Fill_Ixx" )->GetDouble( i );
            double fillIyy = FindPtr( "Fill_Iyy" )->GetDouble( i );
            double fillIzz = FindPtr( "Fill_Izz" )->GetDouble( i );
            double fillIxy = FindPtr( "Fill_Ixy" )->GetDouble( i );
            double fillIyz = FindPtr( "Fill_Iyz" )->GetDouble( i );
            double fillIxz = FindPtr( "Fill_Ixz" )->GetDouble( i );
            double fillVol = FindPtr( "Fill_Vol" )->GetDouble( i );

            fprintf( fid, "%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\t%.15e\n",
                     fillslice, fillmass, fillcg.x(), fillcg.y(), fillcg.z(),
                     fillIxx, fillIyy, fillIzz, fillIxy, fillIxz, fillIyz, fillVol );
        }


        fclose( fid );          // Close File
    }
}

//==== Write the CompGeom Txt File =====//
void Results::WriteCompGeomTxtFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "...Comp Geom...\n" );
        fprintf( fid, "%d Num Comps\n",        FindPtr( "Num_Comps" )->GetInt( 0 ) );
        fprintf( fid, "%d Total Num Meshes\n", FindPtr( "Total_Num_Meshes" )->GetInt( 0 ) );
        fprintf( fid, "%d Total Num Tris\n",   FindPtr( "Total_Num_Tris" )->GetInt( 0 ) );
        fprintf( fid, "\n" );
        fprintf( fid, "Theo_Area   Wet_Area   Theo_Vol    Wet_Vol  Name\n" );

        int num_mesh = FindPtr( "Num_Meshes" )->GetInt( 0 );
        for ( int i = 0 ; i < num_mesh ; i++ )
        {
            fprintf( fid, "%9.3f  %9.3f  %9.3f  %9.3f  %-15s\n",
                     FindPtr( "Theo_Area" )->GetDouble( i ), FindPtr( "Wet_Area" )->GetDouble( i ),
                     FindPtr( "Theo_Vol" )->GetDouble( i ),  FindPtr( "Wet_Vol" )->GetDouble( i ),
                     FindPtr( "Comp_Name" )->GetString( i ).c_str() );
        }

        fprintf( fid, "-------------------------------------------------\n" );
        fprintf( fid, "%9.3f  %9.3f  %9.3f  %9.3f  %-15s\n",
                 FindPtr( "Total_Theo_Area" )->GetDouble( 0 ),   FindPtr( "Total_Wet_Area" )->GetDouble( 0 ),
                 FindPtr( "Total_Theo_Vol" )->GetDouble( 0 ),    FindPtr( "Total_Wet_Vol" )->GetDouble( 0 ), "Totals" );

        int num_tag = FindPtr( "Num_Tags" )->GetInt( 0 );
        if ( num_tag > 0 )
        {
            fprintf( fid, "\n" );
            fprintf( fid, "Tag_Theo_Area   Tag_Wet_Area    Tag_Name\n" );

            for ( int i = 0 ; i < num_tag ; i++ )
            {
                fprintf( fid, "    %9.3f      %9.3f    %-15s\n",
                         FindPtr( "Tag_Theo_Area" )->GetDouble( i ), FindPtr( "Tag_Wet_Area" )->GetDouble( i ),
                         FindPtr( "Tag_Name" )->GetString( i ).c_str() );
            }
        }

        if ( FindPtr( "Num_Degen_Tris_Removed" )->GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", FindPtr( "Num_Degen_Tris_Removed" )->GetInt( 0 ) );
        }
        if ( FindPtr( "Num_Open_Meshes_Removed" )->GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes removed\n", FindPtr( "Num_Open_Meshes_Removed" )->GetInt( 0 ) );

            vector < string > remnames = FindPtr( "Meshes_Removed_Names" )->GetStringData();
            for ( int i = 0; i < remnames.size(); i++ )
            {
                fprintf( fid, "     Removed: %s\n", remnames[i].c_str() );
            }
        }
        if ( FindPtr( "Num_Open_Meshes_Merged" )->GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", FindPtr( "Num_Open_Meshes_Merged" )->GetInt( 0 ) );

            vector < string > mernames = FindPtr( "Meshes_Merged_Names" )->GetStringData();
            for ( int i = 0; i < mernames.size(); i++ )
            {
                fprintf( fid, "     Merged: %s\n", mernames[i].c_str() );
            }
        }

        fclose( fid );      // Close File
    }
}

//==== Write The CompGeom CSV File ====//
void Results::WriteCompGeomCsvFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "Name, Theo_Area, Wet_Area, Theo_Vol, Wet_Vol\n" );
        int num_mesh = FindPtr( "Num_Meshes" )->GetInt( 0 );
        for ( int i = 0 ; i < num_mesh ; i++ )
        {
            fprintf( fid, "%s,%f,%f,%f,%f\n",
                     FindPtr( "Comp_Name" )->GetString( i ).c_str(),
                     FindPtr( "Theo_Area" )->GetDouble( i ), FindPtr( "Wet_Area" )->GetDouble( i ),
                     FindPtr( "Theo_Vol" )->GetDouble( i ),  FindPtr( "Wet_Vol" )->GetDouble( i ) );
        }
        fprintf( fid, "%s,%f,%f,%f,%f\n", "Totals",
                 FindPtr( "Total_Theo_Area" )->GetDouble( 0 ),   FindPtr( "Total_Wet_Area" )->GetDouble( 0 ),
                 FindPtr( "Total_Theo_Vol" )->GetDouble( 0 ),    FindPtr( "Total_Wet_Vol" )->GetDouble( 0 ) );

        int num_tag = FindPtr( "Num_Tags" )->GetInt( 0 );
        if ( num_tag > 0 )
        {
            fprintf( fid, "\nTag_Name, Tag_Theo_Area, Tag_Wet_Area\n" );
            for ( int i = 0 ; i < num_tag ; i++ )
            {
                fprintf( fid, "%s,%f,%f\n",
                         FindPtr( "Tag_Name" )->GetString( i ).c_str(),
                         FindPtr( "Tag_Theo_Area" )->GetDouble( i ), FindPtr( "Tag_Wet_Area" )->GetDouble( i ) );
            }
        }

        fclose( fid );
    }
}

//==== Write The CompGeom Drag BuildUp File ====//
void Results::WriteDragBuildFile( const string & file_name )
{

    FILE* fid = fopen( file_name.c_str(), "w" );

    if ( fid )
    {
        fprintf( fid, "Name\tTheo_Area\tWet_Area\tTheo_Vol\tWet_Vol\tMin_Chord\tAve_Chord\tMax_Chord\t" );
        fprintf( fid, "Min_TC_Ratio\tAvg_TC_Ratio\tMax_TC_Ratio\tAve_Sweep\tLength\tMax_Xsec_Area\tLen_Dia_Ratio\n" );

        int num_mesh = FindPtr( "Num_Meshes" )->GetInt( 0 );
        for ( int i = 0 ; i < num_mesh ; i++ )
        {
            fprintf( fid, "%s\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
                     FindPtr( "Comp_Name" )->GetString( i ).c_str(),
                     FindPtr( "Theo_Area" )->GetDouble( i ), FindPtr( "Wet_Area" )->GetDouble( i ),
                     FindPtr( "Theo_Vol" )->GetDouble( i ),  FindPtr( "Wet_Vol" )->GetDouble( i ),
                     FindPtr( "Min_Chord" )->GetDouble( i ), FindPtr( "Avg_Chord" )->GetDouble( i ), FindPtr( "Max_Chord" )->GetDouble( i ),
                     FindPtr( "Min_TC" )->GetDouble( i ),    FindPtr( "Avg_TC" )->GetDouble( i ),    FindPtr( "Max_TC" )->GetDouble( i ),
                     FindPtr( "Avg_Sweep" )->GetDouble( i ),
                     FindPtr( "Length" )->GetDouble( i ),
                     FindPtr( "Max_Area" )->GetDouble( i ),
                     FindPtr( "Length_Dia" )->GetDouble( i ) );
        }

        fclose( fid );
    }
}

void Results::WriteParasiteDragFile( const string & file_name )
{
    FILE* file_id = fopen(file_name.c_str(), "w");

    if (file_id)
    {
        fprintf(file_id, "PARASITE DRAG BUILD UP DATA \n\n");

        string AltLabel = FindPtr("Alt_Label")->GetString(0);
        string VinfLabel = FindPtr("Vinf_Label")->GetString(0);
        string SrefLabel = FindPtr("Sref_Label")->GetString(0);
        double mach = FindPtr("FC_Mach")->GetDouble(0);
        double hinf = FindPtr("FC_Alt")->GetDouble(0);
        double vinf = FindPtr("FC_Vinf")->GetDouble(0);
        double sref = FindPtr("FC_Sref")->GetDouble(0);
        fprintf(file_id, "Mach, %s, %s, %s\n", AltLabel.c_str(), VinfLabel.c_str(), SrefLabel.c_str());
        fprintf(file_id, "%f, %f, %f, %f \n\n", mach, hinf, vinf, sref);

        string TempLabel = FindPtr("Temp_Label")->GetString(0);
        string PresLabel = FindPtr("Pres_Label")->GetString(0);
        string RhoLabel = FindPtr("Rho_Label")->GetString(0);
        double temp = FindPtr("FC_Temp")->GetDouble(0);
        double pres = FindPtr("FC_Pres")->GetDouble(0);
        double rho = FindPtr("FC_Rho")->GetDouble(0);
        string lamcfeqnname = FindPtr("LamCfEqnName")->GetString(0);
        string turbcfeqnname = FindPtr("TurbCfEqnName")->GetString(0);

        fprintf(file_id, "%s, %s, %s \n", TempLabel.c_str(), PresLabel.c_str(), RhoLabel.c_str());
        fprintf(file_id, "%f, %f, %f \n\n", temp, pres, rho);
        fprintf(file_id, "Lam Cf Eqn, Turb Cf Eqn \n");
        fprintf(file_id, "%s, %s \n", lamcfeqnname.c_str(), turbcfeqnname.c_str());

        fprintf(file_id, "\n");

        string SwetLabel = FindPtr("Swet_Label")->GetString(0);
        string LrefLabel = FindPtr("Lref_Label")->GetString(0);
        string fLabel = FindPtr("f_Label")->GetString(0);

        fprintf(file_id, "Component Name,%s,%s,t/c or d/l,FF,FF Eqn Type,Re,%% Lam,Cf,Q (Interference Factor),%s,Cd,%% Total \n",
            SwetLabel.c_str(), LrefLabel.c_str(), fLabel.c_str() );

        int num_comp = FindPtr("Num_Comp")->GetInt(0);
        for (int i = 0; i < num_comp; i++)
        {
            string label = FindPtr("Comp_Label")->GetString(i);
            double swet = FindPtr("Comp_Swet")->GetDouble(i);
            double lref = FindPtr("Comp_Lref")->GetDouble(i);
            double fineRat = FindPtr("Comp_FineRat")->GetDouble(i);
            int FFType = FindPtr("Comp_FFEqn")->GetInt(i);
            double FF = 0;
            if (FFType == vsp::FF_B_MANUAL || FFType == vsp::FF_W_MANUAL)
            {
                FF = FindPtr("Comp_FFIn")->GetDouble(i);
            }
            else
            {
                FF = FindPtr("Comp_FFOut")->GetDouble(i);
            }
            string FFEqnName = FindPtr("Comp_FFEqnName")->GetString(i);
            double Re = FindPtr("Comp_Re")->GetDouble(i);
            double PercLam = FindPtr("Comp_PercLam")->GetDouble(i);
            double Cf = FindPtr("Comp_Cf")->GetDouble(i);
            double Q = FindPtr("Comp_Q")->GetDouble(i);
            double f = FindPtr("Comp_f")->GetDouble(i);
            double CD = FindPtr("Comp_CD")->GetDouble(i);
            double PercTotalCD = 100 * FindPtr("Comp_PercTotalCD")->GetDouble(i);

            fprintf(file_id, "%s,", label.c_str());
            fprintf(file_id, "%f, %f, %f,", swet, lref, fineRat);
            fprintf(file_id, "%f, %s,", FF, FFEqnName.c_str());
            fprintf(file_id, "%f, %f,", Re, PercLam);
            fprintf(file_id, "%f, %f,", Cf, Q);
            fprintf(file_id, "%f, %f, %f \n", f, CD, PercTotalCD);

        }

        fprintf(file_id, "\n");
        fprintf(file_id, "Excrescences, Type, Input\n");

        int num_excres = FindPtr("Num_Excres")->GetInt(0);
        for (int i = 0; i < num_excres; i++)
        {
            string label = FindPtr("Excres_Label")->GetString(i);
            string type = FindPtr("Excres_Type")->GetString(i);
            double input = FindPtr("Excres_Input")->GetDouble(i);
            double f = FindPtr( "Excres_f" )->GetDouble( i );
            double amount = FindPtr("Excres_Amount")->GetDouble(i);
            double perctotalcd = 100 * FindPtr("Excres_PercTotalCD")->GetDouble(i);

            fprintf(file_id, " %s, %s, %f, , , , , , , ,%f, %f, %f \n",
                label.c_str(), type.c_str(), input, f, amount, perctotalcd);
        }

        double geomftotal = FindPtr("Geom_f_Total")->GetDouble(0);
        double geomcdtotal = FindPtr("Geom_CD_Total")->GetDouble(0);
        double geomperctotal = 100 * FindPtr("Geom_Perc_Total")->GetDouble(0);
        double excresftotal = FindPtr("Excres_f_Total")->GetDouble(0);
        double excrescdtotal = FindPtr("Excres_CD_Total")->GetDouble(0);
        double excresperctotal = 100 * FindPtr("Excres_Perc_Total")->GetDouble(0);
        double totalftotal = FindPtr("Total_f_Total")->GetDouble(0);
        double totalcdtotal = FindPtr("Total_CD_Total")->GetDouble(0);
        double totalperctotal = 100 * FindPtr("Total_Perc_Total")->GetDouble(0);

        fprintf(file_id, "\n");
        fprintf(file_id, " , , , , , , , , , %s, %f, %f, %f \n", "Geometry Sub-Total:", geomftotal, geomcdtotal, geomperctotal);
        fprintf(file_id, " , , , , , , , , , %s, %f, %f, %f \n", "Excrescence Sub-Total:", excresftotal, excrescdtotal, excresperctotal);
        fprintf(file_id, " , , , , , , , , , %s, %f, %f, %f \n", "Totals:", totalftotal, totalcdtotal, totalperctotal);

        fclose(file_id);
    }
}

void Results::WriteSliceFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        if ( FindPtr( "Num_Degen_Tris_Removed" )->GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d degenerate triangle removed\n", FindPtr( "Num_Degen_Tris_Removed" )->GetInt( 0 ) );
        }
        if ( FindPtr( "Num_Open_Meshes_Removed" )->GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes removed\n", FindPtr( "Num_Open_Meshes_Removed" )->GetInt( 0 ) );

            vector < string > remnames = FindPtr( "Meshes_Removed_Names" )->GetStringData();
            for ( int i = 0; i < remnames.size(); i++ )
            {
                fprintf( fid, "     Removed: %s\n", remnames[i].c_str() );
            }
        }
        if ( FindPtr( "Num_Open_Meshes_Merged" )->GetInt( 0 ) )
        {
            fprintf( fid, "WARNING: %d open meshes merged\n", FindPtr( "Num_Open_Meshes_Merged" )->GetInt( 0 ) );

            vector < string > mernames = FindPtr( "Meshes_Merged_Names" )->GetStringData();
            for ( int i = 0; i < mernames.size(); i++ )
            {
                fprintf( fid, "     Merged: %s\n", mernames[i].c_str() );
            }
        }

        fprintf( fid, "...Slice...\n" );
        fprintf( fid, "%d Num Comps\n", FindPtr( "Num_Comps" )->GetInt( 0 ) );
        fprintf( fid, "%d Total Num Meshes\n", FindPtr( "Num_Meshes" )->GetInt( 0 ) );
        fprintf( fid, "%d Total Num Tris\n", FindPtr( "Num_Tris" )->GetInt( 0 ) );
        vec3d norm_axis = FindPtr( "Axis_Vector" )->GetVec3d( 0 );
        fprintf( fid, "%1.5f %1.5f %1.5f Axis Vector\n", norm_axis.x(), norm_axis.y(), norm_axis.z() );

        fprintf( fid, "\n" );
        fprintf( fid, "    Loc    XCenter  YCenter  ZCenter         Area\n" );
        for ( int s = 0 ; s <  FindPtr( "Num_Slices" )->GetInt( 0 ) ; s++ )
        {
            vec3d area_center = FindPtr( "Slice_Area_Center" )->GetVec3d( s );
            fprintf( fid, "%9.3f %9.3f %9.3f %9.3f %9.3f\n", FindPtr( "Slice_Loc" )->GetDouble( s ), area_center[0],
                    area_center[1], area_center[2], FindPtr( "Slice_Area" )->GetDouble( s ) );
        }

        fclose( fid );
    }

}

void Results::WriteWaveDragFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "...Wave Drag Slice...\n" );

        fprintf( fid, "Inlet Area: %f\n", FindPtr( "Inlet_Area" )->GetDouble( 0 ) );
        fprintf( fid, "Exit Area: %f\n", FindPtr( "Exit_Area" )->GetDouble( 0 ) );

        int num_cone_sections = FindPtr( "Num_Cone_Sections" )->GetInt( 0 );
        int num_slices = FindPtr( "Num_Slices" )->GetInt( 0 );

        fprintf( fid, "\n" );
        for ( int i = 0; i < num_cone_sections ; i++ )
        {
            fprintf( fid, "Theta: %6.2f, Start: %6.2f, End: %6.2f\n", FindPtr( "Theta" )->GetDouble( i ), FindPtr( "Start_X" )->GetDouble( i ), FindPtr( "End_X" )->GetDouble( i ) );

            for ( int s = 0 ; s < num_slices ; s++ )
            {
                fprintf( fid, "%19.8f, ", FindPtr( "X_Norm" )->GetDouble( s ) );
                fprintf( fid, "%19.8f", FindPtr( "Slice_Area", i )->GetDouble( s ) );
                fprintf( fid, "\n" );
            }
            fprintf( fid, "\n" );
        }

        double CD0w = FindPtr( "CDWave" )->GetDouble( 0 );
        fprintf( fid, "CDWave: %19.8f \n", CD0w );

        double Mach = FindPtr( "Mach" )->GetDouble( 0 );
        fprintf( fid, "Mach: %19.8f \n", Mach);

        string Set_Name = FindPtr( "Set_Name" )->GetString( 0 );
        fprintf( fid, "Set Name: %s \n", Set_Name.c_str());

        fclose( fid );
    }

}

void Results::WriteBEMFile( const string & file_name )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if ( fid )
    {
        fprintf( fid, "...BEM Propeller...\n" );

        int num_sect = FindPtr( "Num_Sections" )->GetInt( 0 );
        int num_blade = FindPtr( "Num_Blade" )->GetInt( 0 );
        double diam = FindPtr( "Diameter" )->GetDouble( 0 );
        double beta34 = FindPtr( "Beta34" )->GetDouble( 0 );
        double feather = FindPtr( "Feather" )->GetDouble( 0 );
        double precone = FindPtr( "Pre_Cone" )->GetDouble( 0 );
        vec3d cen = FindPtr( "Center" )->GetVec3d( 0 );
        vec3d norm = FindPtr( "Normal" )->GetVec3d( 0 );

        fprintf( fid, "Num_Sections: %d\n", num_sect );
        fprintf( fid, "Num_Blade: %d\n", num_blade );
        fprintf( fid, "Diameter: %.8f\n", diam );
        fprintf( fid, "Beta 3/4 (deg): %.8f\n", beta34 );
        fprintf( fid, "Feather (deg): %.8f\n", feather );
        fprintf( fid, "Pre_Cone (deg): %.8f\n", precone );
        fprintf( fid, "Center: %.8f, %.8f, %.8f\n", cen.x(), cen.y(), cen.z() );
        fprintf( fid, "Normal: %.8f, %.8f, %.8f\n", norm.x(), norm.y(), norm.z() );

        vector < double > r_vec = FindPtr( "Radius" )->GetDoubleData();
        vector < double > chord_vec = FindPtr( "Chord" )->GetDoubleData();
        vector < double > twist_vec = FindPtr( "Twist" )->GetDoubleData();
        vector < double > rake_vec = FindPtr( "Rake" )->GetDoubleData();
        vector < double > skew_vec = FindPtr( "Skew" )->GetDoubleData();
        vector < double > sweep_vec = FindPtr( "Sweep" )->GetDoubleData();
        vector < double > thick_vec = FindPtr( "Thick" )->GetDoubleData();
        vector < double > cli_vec = FindPtr( "CLi" )->GetDoubleData();
        vector < double > axial_vec = FindPtr( "Axial" )->GetDoubleData();
        vector < double > tangential_vec = FindPtr( "Tangential" )->GetDoubleData();

        fprintf( fid, "\nRadius/R, Chord/R, Twist (deg), Rake/R, Skew/R, Sweep, t/c, CLi, Axial, Tangential\n" );
        for ( int i = 0; i < num_sect; i++ )
        {
            fprintf( fid, "%.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f, %.8f\n", r_vec[i], chord_vec[i], twist_vec[i], rake_vec[i], skew_vec[i], sweep_vec[i], thick_vec[i], cli_vec[i], axial_vec[i], tangential_vec[i] );
        }

        for ( int i = 0; i < num_sect; i++ )
        {
            char str[255];
            snprintf( str, sizeof( str ),  "%03d", i );
            vector < double > xpts = FindPtr( "XSection_" + string( str ) )->GetDoubleData();
            vector < double > ypts = FindPtr( "YSection_" + string( str ) )->GetDoubleData();

            fprintf( fid, "\nSection %d X, Y\n", i );

            for ( int j = 0; j < xpts.size(); j++ )
            {
                fprintf( fid, "%.8f, %.8f\n", xpts[j], ypts[j] );
            }
        }

        fclose( fid );
    }
}

// Copy a NameValData pointer to this Result
void Results::Copy( NameValData* nvd )
{
    switch ( nvd->GetType() )
    {
        case ( vsp::DOUBLE_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetDoubleData(), nvd->GetDoc().c_str() ) ) );
            break;
        }
        case ( vsp::INT_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetIntData(), nvd->GetDoc().c_str() ) ) );
            break;
        }
        case ( vsp::STRING_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetStringData(), nvd->GetDoc().c_str() ) ) );
            break;
        }
        case ( vsp::INT_MATRIX_DATA ):
            Add( ( NameValData( nvd->GetName(), nvd->GetIntMatData(), nvd->GetDoc().c_str() ) ) );
            break;
        case ( vsp::DOUBLE_MATRIX_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetDoubleMatData(), nvd->GetDoc().c_str() ) ) );
            break;
        }
        case ( vsp::VEC3D_DATA ):
        {
            Add( ( NameValData( nvd->GetName(), nvd->GetVec3dData(), nvd->GetDoc().c_str() ) ) );
            break;
        }
    }
}

//======================================================================================//
//======================================================================================//
//======================================================================================//


//==== Constructor ====//
ResultsMgrSingleton::ResultsMgrSingleton()
{

}
//==== Destructor ====//
ResultsMgrSingleton::~ResultsMgrSingleton()
{
    DeleteAllResults();
}


//==== Create and Add Results Object and Return Ptr ====//
Results* ResultsMgrSingleton::CreateResults( const string & name, const string & doc )
{
    string id = GenerateRandomID( 7 );
    Results* res_ptr = new Results( name, id, doc );     // Create Results

    m_ResultsMap[id] = res_ptr;                     // Map ID to Ptr
    m_NameIDMap[name].push_back( id );              // Map Name to Vector of IDs
    return res_ptr;
}


//==== Create Results And Load With Geometry Data ====//
string ResultsMgrSingleton::CreateGeomResults( const string & geom_id, const string & name )
{
    Results* res_ptr = CreateResults( name, "Geometry mesh results." );

    Vehicle* veh = VehicleMgr.GetVehicle();
    if ( !veh )
    {
        return string();
    }

    Geom* geom_ptr = veh->FindGeom( geom_id );
    if ( !geom_ptr )
    {
        return string();
    }

    geom_ptr->CreateGeomResults( res_ptr );

    return res_ptr->GetID();
}


//==== Delete All Results ====//
void ResultsMgrSingleton::DeleteAllResults()
{
    //==== Delete All Created Results =====//
    map< string, Results* >::iterator iter;
    for ( iter = m_ResultsMap.begin() ; iter != m_ResultsMap.end() ; ++iter )
    {
        delete iter->second;
    }
    m_ResultsMap.clear();
    m_NameIDMap.clear();
}

//==== Delete All Results ====//
void ResultsMgrSingleton:: DeleteResult( const string & id )
{
    //==== Find Name Given ID ====//
    map< string, vector< string > >::iterator iter;

    //==== Find Name To Match ID And Remove====//
    for ( iter = m_NameIDMap.begin() ; iter != m_NameIDMap.end() ; ++iter )
    {
        bool found = false;
        for ( int i = 0 ; i < ( int )iter->second.size() ; i++ )
        {
            if ( iter->second[i] == id )
            {
                found = true;
            }
        }
        if ( found )
        {
            vector_remove_val( iter->second, id );
            if ( iter->second.size() == 0 )
            {
                m_NameIDMap.erase( iter );
            }

            break;
        }
    }

    //==== Delete All Created Results =====//
    map< string, Results* >::iterator res_iter;

    res_iter = m_ResultsMap.find( id );

    if ( res_iter != m_ResultsMap.end() )
    {
        delete res_iter->second;
        m_ResultsMap.erase( res_iter );
    }
}


//==== Find The Number of Results Given Name ====//
int ResultsMgrSingleton::GetNumResults( const string & name )
{
    map< string, vector< string > >::iterator iter = m_NameIDMap.find( name );

    if ( iter == m_NameIDMap.end() )
    {
        return 0;
    }

    return ( int )( iter->second.size() );
}

//==== Find Results ID Given Name and Optional Index =====//
string ResultsMgrSingleton::FindResultsID( const string & name, int index )
{
    map< string, vector< string > >::iterator iter = m_NameIDMap.find( name );
    if ( iter == m_NameIDMap.end() )
    {
        return string();
    }

    if ( index < 0 || index >= ( int )( iter->second.size() ) )
    {
        return string();
    }

    return iter->second[index];
}

//==== Find The Latest Results ID For the Given Name
string ResultsMgrSingleton::FindLatestResultsID( const string & name )
{
    map< string, vector< string > >::iterator iter = m_NameIDMap.find( name );
    if ( iter == m_NameIDMap.end() )
    {
        return string();
    }

    string latest_id;
    time_t latest_stamp = 0;
    for ( int i = 0 ; i < ( int )( iter->second.size() ) ; i++ )
    {
        string id = iter->second[i];
        Results* res_ptr = FindResultsPtr( id );

        if ( res_ptr && res_ptr->GetTimestamp() >= latest_stamp )
        {
            latest_stamp = res_ptr->GetTimestamp();
            latest_id = id;
        }
    }
    return latest_id;
}


//==== Find Results Ptr Given ID =====//
Results* ResultsMgrSingleton::FindResultsPtr( const string & id )
{
    map< string, Results* >::iterator id_iter = m_ResultsMap.find( id );

    if ( id_iter ==  m_ResultsMap.end() )
    {
        return NULL;
    }

    return id_iter->second;
}

//==== Find Results Ptr Given Name and Optional Index =====//
Results* ResultsMgrSingleton::FindResults( const string & name, int index )
{
    string id = FindResultsID( name, index );
    return FindResultsPtr( id );
}


//==== Get Results TimeStamp Given ID ====//
time_t ResultsMgrSingleton::GetResultsTimestamp( const string & results_id )
{
    map< string, Results* >::iterator iter = m_ResultsMap.find( results_id );

    if ( iter ==  m_ResultsMap.end() )
    {
        return vsp::INT_DATA;
    }

    return iter->second->GetTimestamp();
}


//==== Get Results TimeStamp Given ID ====//
int ResultsMgrSingleton::GetNumData( const string & results_id, const string & data_name )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return 0;
    }

    return results_ptr->GetNumData( data_name );
}

int ResultsMgrSingleton::GetResultsType( const string & results_id, const string & data_name )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return vsp::INVALID_TYPE;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( data_name );
    if ( !rd_ptr )
    {
        return vsp::INVALID_TYPE;
    }

    return rd_ptr->GetType();
}

string ResultsMgrSingleton::GetResultsTypeName( const string & results_id, const string & data_name )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return "";
    }

    NameValData* rd_ptr = results_ptr->FindPtr( data_name );
    if ( !rd_ptr )
    {
        return "";
    }

    return rd_ptr->GetTypeName();
}

string ResultsMgrSingleton::GetResultsEntryDoc( const string & results_id, const string & data_name )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return "";
    }

    NameValData* rd_ptr = results_ptr->FindPtr( data_name );
    if ( !rd_ptr )
    {
        return "";
    }

    return rd_ptr->GetDoc();
}

//==== Get The Names of All Results ====//
vector< string > ResultsMgrSingleton::GetAllResultsNames()
{
    vector< string > name_vec;
    map< string, vector< string > >::iterator iter;
    for ( iter = m_NameIDMap.begin() ; iter != m_NameIDMap.end() ; ++iter )
    {
        name_vec.push_back( iter->first );
    }
    return name_vec;
}

//==== Get The Names of All Data for A Givent Result ====//
vector< string > ResultsMgrSingleton::GetAllDataNames( const string & results_id )
{
    vector< string > name_vec;
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return name_vec;
    }

    return results_ptr->GetAllDataNames();
}

vector< string > ResultsMgrSingleton::GetAllResultsIDs()
{
    vector < string > res_ids;

    map< string, Results* >::iterator iter;
    for ( iter = m_ResultsMap.begin() ; iter != m_ResultsMap.end() ; ++iter )
    {
        res_ids.push_back( iter->first );
    }

    return res_ids;
}

void ResultsMgrSingleton::PrintResults( const string &fname, const string &results_id )
{
    FILE *fp;
    fp = fopen( fname.c_str(), "w" );
    if ( fp )
    {
        PrintResults( fp, results_id );
        fclose( fp );
    }
}

void ResultsMgrSingleton::PrintResults( const string &results_id )
{
    PrintResults( stdout, results_id );
    fflush( stdout );
}

void ResultsMgrSingleton::PrintResults( FILE * outputStream, const string &results_id )
{
    Results* res_ptr = FindResultsPtr( results_id );

    if ( res_ptr )
    {
        fprintf( outputStream, "\n\n%s\n", res_ptr->GetName().c_str() );
        fprintf( outputStream, "%s\n", res_ptr->GetDoc().c_str() );
    }

    fprintf( outputStream, "   %-30s%-13s\t%s\t%s\n", "[result_name]", "[type]", "[#]", "[current values-->]" );

    vector<string> results_names = GetAllDataNames( results_id );
    for ( unsigned int i_result_name = 0; i_result_name < results_names.size(); i_result_name++ )
    {
        int current_result_type = GetResultsType( results_id, results_names[i_result_name] );
        string current_result_type_name = GetResultsTypeName( results_id, results_names[i_result_name] );
        unsigned int current_result_num_data = ( unsigned int )GetNumData( results_id, results_names[i_result_name] );
        fprintf( outputStream, "   %-30s%-13s\t%d", results_names[i_result_name].c_str(), current_result_type_name.c_str(), current_result_num_data );
        // print out the current value (this needs to handle different types and vector lengths
        fprintf( outputStream, "\t" );
        for ( unsigned int i_val = 0; i_val < current_result_num_data; i_val++ )
        {
            switch( current_result_type )
            {
            case vsp::RES_DATA_TYPE::INT_DATA :
            {
                vector<int> current_int_val = GetIntResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_int_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%d ", current_int_val[j_val] );
                }
                break;
            }
            case vsp::RES_DATA_TYPE::DOUBLE_DATA :
            {
                vector<double> current_double_val = GetDoubleResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_double_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%.*e ", DBL_DIG + 3, current_double_val[j_val] );
                }
                break;
            }
            case vsp::RES_DATA_TYPE::INT_MATRIX_DATA :
            {
                vector< vector< int > > current_int_mat_val = GetIntMatResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int row = 0; row < current_int_mat_val.size(); row++ )
                {
                    for ( unsigned int col = 0; col < current_int_mat_val[row].size(); col++ )
                    {
                        fprintf( outputStream, "%d ", current_int_mat_val[row][col] );
                    }
                    if ( row < current_int_mat_val.size() - 1 )
                    {
                        fprintf( outputStream, "\n\t\t%-20s \t\t \t", "");
                    }
                }
                break;
            }
            case vsp::RES_DATA_TYPE::DOUBLE_MATRIX_DATA :
            {
                vector< vector< double > > current_double_mat_val = GetDoubleMatResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int row = 0; row < current_double_mat_val.size(); row++ )
                {
                    for ( unsigned int col = 0; col < current_double_mat_val[row].size(); col++ )
                    {
                        fprintf( outputStream, "%.*e ", DBL_DIG + 3, current_double_mat_val[row][col] );
                    }
                    if ( row < current_double_mat_val.size() - 1 )
                    {
                        fprintf( outputStream, "\n\t\t%-20s \t\t \t", "");
                    }
                }
                break;
            }
            case vsp::RES_DATA_TYPE::STRING_DATA :
            {
                vector<string> current_string_val = GetStringResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_string_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%s ", current_string_val[j_val].c_str() );
                }
                //Recursive call if results vector found
                if( strcmp( results_names[i_result_name].c_str(), "ResultsVec" ) == 0 )
                {
                    vector < string > res_vec = GetStringResults( results_id, "ResultsVec", 0 );
                    for ( unsigned int i = 0; i < res_vec.size(); i++ )
                    {
                        PrintResults( outputStream, res_vec[i] );
                    }
                }
                break;
            }
            case vsp::RES_DATA_TYPE::VEC3D_DATA :
            {
                vector<vec3d> current_vec3d_val = GetVec3dResults( results_id, results_names[i_result_name], i_val );
                for ( unsigned int j_val = 0; j_val < current_vec3d_val.size(); j_val++ )
                {
                    fprintf( outputStream, "%.*e,%.*e,%.*e ", DBL_DIG + 3, current_vec3d_val[j_val].x(), DBL_DIG + 3, current_vec3d_val[j_val].y(), DBL_DIG + 3, current_vec3d_val[j_val].z() );
                }
                break;
            }
            }    //end switch

        }    // end for

        fprintf( outputStream, "\n" );
    }
}

void ResultsMgrSingleton::PrintResultsDocs( const string &fname, const string &results_id )
{
    FILE *fp;
    fp = fopen( fname.c_str(), "w" );
    if ( fp )
    {
        PrintResultsDocs( fp, results_id );
        fclose( fp );
    }
}

void ResultsMgrSingleton::PrintResultsDocs( const string &results_id )
{
    PrintResultsDocs( stdout, results_id );
    fflush( stdout );
}

void ResultsMgrSingleton::PrintResultsDocs( FILE * outputStream, const string &results_id )
{
    Results * res_ptr = FindResultsPtr( results_id );

    if ( !res_ptr )
    {
        return;
    }

    fprintf( outputStream, "%s\n", res_ptr->GetName().c_str() );
    fprintf( outputStream, "%s\n", res_ptr->GetDoc().c_str() );

    fprintf( outputStream, "   %-30s%-13s\t%s\n", "[result_name]", "[type]", "[doc]" );

    vector<string> results_names = GetAllDataNames( results_id );
    for ( unsigned int i_result_name = 0; i_result_name < results_names.size(); i_result_name++ )
    {
        string current_result_type = GetResultsTypeName( results_id, results_names[i_result_name] );
        string current_result_doc = GetResultsEntryDoc( results_id, results_names[ i_result_name ] );
        fprintf( outputStream, "   %-30s%-13s\t%s\n", results_names[i_result_name].c_str(), current_result_type.c_str(), current_result_doc.c_str() );
    }
}


//==== Get Int Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<int> & ResultsMgrSingleton::GetIntResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultIntVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultIntVec;
    }

    return rd_ptr->GetIntData();
}

//==== Get Double Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<double> & ResultsMgrSingleton::GetDoubleResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultDoubleVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultDoubleVec;
    }

    return rd_ptr->GetDoubleData();
}

//==== Get Int Matrix Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<vector<int>> & ResultsMgrSingleton::GetIntMatResults( const string &id, const string &name,
                          int index )
{
    Results* results_ptr = FindResultsPtr( id );
    if ( !results_ptr )
    {
        return m_DefaultIntMat;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultIntMat;
    }

    return rd_ptr->GetIntMatData();
}

//==== Get Double Matrix Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<vector<double>> & ResultsMgrSingleton::GetDoubleMatResults( const string &id, const string &name,
                          int index )
{
    Results* results_ptr = FindResultsPtr( id );
    if ( !results_ptr )
    {
        return m_DefaultDoubleMat;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultDoubleMat;
    }

    return rd_ptr->GetDoubleMatData();
}

//==== Get string Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<string> & ResultsMgrSingleton::GetStringResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultStringVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultStringVec;
    }

    return rd_ptr->GetStringData();
}

//==== Get Vec3d Results Given Results ID and Name of Data and Index (Default 0) ====//
const vector<vec3d> & ResultsMgrSingleton::GetVec3dResults( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return m_DefaultVec3dVec;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return m_DefaultVec3dVec;
    }

    return rd_ptr->GetVec3dData();
}

//==== Check If Results ID is Valid ====//
bool ResultsMgrSingleton::ValidResultsID( const string & results_id )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return false;
    }
    return true;
}

//==== Check If Data Name and Index is Valid ====//
bool ResultsMgrSingleton::ValidDataNameIndex( const string & results_id, const string & name, int index )
{
    Results* results_ptr = FindResultsPtr( results_id );
    if ( !results_ptr )
    {
        return false;
    }

    NameValData* rd_ptr = results_ptr->FindPtr( name, index );
    if ( !rd_ptr )
    {
        return false;
    }

    return true;
}




//==== Write Some Test Results ====//
void ResultsMgrSingleton::WriteTestResults()
{
    //==== Write Two Sets of Results ====//
    for ( int s = 0 ; s < 2 ; s++ )
    {
        Results* res = ResultsMgr.CreateResults( "Test_Results", "Test results." );

//      printf( "Timestamp = %d \n", res->GetTimestamp() );

        res->Add( NameValData( "Test_Int", s + 1, "Test integer result." ) );
        res->Add( NameValData( "Test_Int", s + 2, "Test integer result." ) );
        res->Add( NameValData( "Test_Double", ( s + 1 ) * 0.1, "Test double result." ) );
        res->Add( NameValData( "Test_String", "This Is A Test", "Test string result." ) );
        res->Add( NameValData( "Test_Vec3d", vec3d( s, s * 2, s * 4 ), "Test vec3d result." ) );

        vector< double > dvec;
        for ( int i = 0 ; i < 5 ; i++ )
        {
            dvec.push_back( i * ( s + 1 ) );
        }
        res->Add( NameValData( "Test_Double_Vec", dvec, "Test double vector result." ) );
    }


}


//==== Test Speed ====//
void ResultsMgrSingleton::TestSpeed()
{
//  vector< vec3d > pnt_vec;
//  for ( int i = 0 ; i < 10000000 ; i++ )
//      pnt_vec.push_back( vec3d( i, 2*i, 3*i ) );
//
//  Results* res = CreateResults( "TestVec" );
//  res->Add( ResData( "Pnt_Vec", pnt_vec)  );
//
//  int start_tics = GetTickCount();
//
//  //==== Sum Vectors ====//
//  vec3d sum;
//  for ( int i = 0 ; i < (int)pnt_vec.size() ; i++ )
//      sum = sum + pnt_vec[i];
//
//  int tics1 = GetTickCount();
//
//  Results* test_res = FindLatestResult( "TestVec" );
//  ResData pnt_vec_result = test_res->Find( "Pnt_Vec", 0 );
//
////    const vector< vec3d > & res_pnt_vec = pnt_vec_result->GetVec3dData();
//  vector< vec3d > res_pnt_vec = pnt_vec_result->GetVec3dData();
//
//  //==== Sum Vectors ====//
//  vec3d sum1;
//  for ( int i = 0 ; i < (int)res_pnt_vec.size() ; i++ )
//      sum1 = sum1 + res_pnt_vec[i];

//  int tics2 = GetTickCount();
//
//  if ( dist( sum, sum1 ) > 0.000001 )
//      printf("Not Good\n" );
//
//  int del_t0 = tics1 - start_tics;
//  int del_t1 = tics2 - tics1;
//  printf(" Results Speed %d %d \n", del_t0, del_t1 );
}


int ResultsMgrSingleton::WriteCSVFile( const string & file_name, const vector < string > &resids )
{
    FILE* fid = fopen( file_name.c_str(), "w" );
    if( fid )
    {
        for( unsigned int iRes=0; iRes<resids.size(); iRes++ )
        {
            Results* resptr = ResultsMgr.FindResultsPtr( resids[iRes] );
            if( resptr )
            {
                resptr->WriteCSVFile( fid );    //append this result to the csv file
            }
        }
        fclose( fid );          // Close File
        return vsp::VSP_OK;
    }
    else
    {
        return vsp::VSP_FILE_WRITE_FAILURE;
    }
}
