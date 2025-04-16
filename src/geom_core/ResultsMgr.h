//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ResultsMgr.h: Store results from analysis and computation
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(RESULTSMGR__INCLUDED_)
#define RESULTSMGR__INCLUDED_

#include "Vec3d.h"
#include "XmlUtil.h"
#include "APIDefines.h"

#include <map>
#include <list>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

//==== Results Data - Named Vectors Of Ints/Double/Strings or Vec3d ====//
class NameValCollection;
class AttributeCollection;

class NameValData
{
public:
    NameValData();
    NameValData( const string & name );
    NameValData( const string & name, const bool & b_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const int & i_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const double & d_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const string & s_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vec3d & v_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vector< int > & i_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vector< double > & d_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vector< string > & s_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vector< vec3d > & v_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vector< vector< int > > &imat_data, const string & doc, const string & id = string() );
    NameValData( const string & name, const vector< vector< double > > &dmat_data, const string & doc, const string & id = string() );
    ~NameValData();

    NameValData( const NameValData &nvd ) = delete;
    bool operator== ( const NameValData &nvd ) const = delete;

    string GetName() const
    {
        return m_Name;
    }
    int GetIndex() const
    {
        return m_Index;
    }
    string GetID() const
    {
        return m_ID;
    }
    int GetType() const
    {
        return m_Type;
    }
    string GetTypeName() const;
    static string GetTypeName( int type, bool capitalize = false, bool short_name = false );

    string GetDoc() const
    {
        return m_Doc;
    }

    const vector<int> & GetBoolData() const
    {
        return m_IntData;
    }
    const vector<int> & GetIntData() const
    {
        return m_IntData;
    }
    const vector<double> & GetDoubleData() const
    {
        return m_DoubleData;
    }
    const vector<string> & GetStringData() const
    {
        return m_StringData;
    }
    const vector<string> & GetParmIDData() const
    {
        return m_ParmIDData;
    }
    const vector<vec3d> & GetVec3dData() const
    {
        return m_Vec3dData;
    }
    const vector< vector< int > > & GetIntMatData() const
    {
        return m_IntMatData;
    }
    const vector< vector< double > > & GetDoubleMatData() const
    {
        return m_DoubleMatData;
    }
    vector<int> & GetBoolData()
    {
        return m_IntData;
    }
    vector<int> & GetIntData()
    {
        return m_IntData;
    }
    vector<double> & GetDoubleData()
    {
        return m_DoubleData;
    }
    vector<string> & GetStringData()
    {
        return m_StringData;
    }
    vector<string> & GetParmIDData()
    {
        return m_ParmIDData;
    }
    vector<vec3d> & GetVec3dData()
    {
        return m_Vec3dData;
    }
    vector< vector< int > > & GetIntMatData()
    {
        return m_IntMatData;
    }
    vector< vector< double > > & GetDoubleMatData()
    {
        return m_DoubleMatData;
    }

    bool GetBool( int index ) const;
    int GetInt( int index ) const;
    int GetInt( int row, int col ) const;
    double GetDouble( int index ) const;
    double GetDouble( int row, int col ) const;
    string GetString( int index ) const;
    string GetParmID( int index ) const;
    vec3d GetVec3d( int index ) const;
    AttributeCollection* GetAttributeCollectionPtr();

    void AddAttributeCollection();

    string GetAsString( bool inline_data_flag = false );

    void SetBoolData( const vector< int > & b )
    {
        m_IntData = b;
    }

    void SetIntData( const vector< int > & d )
    {
        m_IntData = d;
    }

    void SetDoubleData( const vector< double > & d )
    {
        m_DoubleData = d;
    }

    void SetStringData( const vector< string > & d )
    {
        m_StringData = d;
    }

    void SetParmIDData( const vector< string > & d )
    {
        m_ParmIDData = d;
    }

    void SetVec3dData( const vector< vec3d > & d )
    {
        m_Vec3dData = d;
    }

    void SetIntMatData( const vector< vector< int > > & i )
    {
        m_IntMatData = i;
    }

    void SetDoubleMatData( const vector< vector< double > > & d )
    {
        m_DoubleMatData = d;
    }

    void SetName( const string & name )
    {
        m_Name = name;
    }

    void SetIndex( const int & index )
    {
        m_Index = index;
    }

    void SetType( const int & type )
    {
        m_Type = type;
    }

    void SetDoc( const string & doc )
    {
        m_Doc = doc;
    }

    void SetAttrAttach( string attachID );

    string GetAttachID() const
    {
        return m_AttachID;
    }

    void SetProtection( bool protect_flag )
    {
        m_ProtectFlag = protect_flag;
    }

    bool IsProtected() const
    {
        return m_ProtectFlag;
    }

    int GetAttributeEventGroup() const
    {
        return m_AttributeEventGroup;
    }

    void SetAttributeEventGroup( int g )
    {
        m_AttributeEventGroup = g;
    }

    string GenerateID();
    void ChangeID( string id );

    void CopyFrom( NameValData* nvd );

    static string TruncateString( string str , int len );

    virtual void EncodeXml( xmlNodePtr & node ) const;
    virtual void DecodeXml( xmlNodePtr & node );

protected:

    void Init( const string & name, int type = 0, const string & id = string() );

    string m_Name;
    int m_Index;
    int m_Type;
    string m_Doc;
    string m_ID;
    vector< int > m_IntData;
    vector< double > m_DoubleData;
    vector< string > m_StringData;
    vector< string > m_ParmIDData;
    vector< vec3d > m_Vec3dData;
    vector< vector< int > >  m_IntMatData;
    vector< vector< double > >  m_DoubleMatData;

    AttributeCollection* m_AttributeCollection;

    bool m_ProtectFlag;
    string m_AttachID;
    int m_AttributeEventGroup;
};

//======================================================================================//
//======================================================================================//
//======================================================================================//

//==== A Collection of Results Data From One Computation ====//
class NameValCollection
{
public:
    NameValCollection();
    NameValCollection( const string & name, const string & id, const string & doc  );
    ~NameValCollection();

    void Wype();

    string GenerateID();

    void Init( const char *name, const char *doc = "" )
    {
        m_Name = name;
        m_Doc = doc;
        m_DataMap.clear();
    }

    void SetName( const string & name )
    {
        m_Name = name;
    }

    void SetDoc( const string & doc )
    {
        m_Doc = doc;
    }

    string GetName() const
    {
        return m_Name;
    }

    string GetID() const
    {
        return m_ID;
    }

    string GetDoc() const
    {
        return m_Doc;
    }

    virtual void Add( NameValData* d );
    virtual void Add( const vector< vector< vec3d > > & d, const string &prefix, const string &doc );

    virtual void Unregister( NameValData* d ){};
    virtual void Del( NameValData* d );
    virtual int Remove( NameValData* d );

    int GetNumData( const string & name );
    vector< string > GetAllDataNames();

    NameValData* FindPtr( const string & name, int index = 0 );
    vector< NameValData* > GetAllPtrs();

protected:

    string m_Name;
    string m_ID;
    string m_Doc;

    //==== All The Data For This Computation Result =====//
    map< string, vector< NameValData* > > m_DataMap;

};

//======================================================================================//
//======================================================================================//
//======================================================================================//

class AttributeCollection : public NameValCollection
{
public:

    AttributeCollection();
    AttributeCollection( const string & name, const string & id, const string & doc  );
    ~AttributeCollection();

    void Add( NameValData* d, const int & attr_event_group = vsp::ATTR_GROUP_NONE, bool set_event_group = false );


    void Unregister( NameValData* d );
    void Del( NameValData* d );
    int Remove( NameValData* d );

    void SetCollAttach( const string & attachID , int attachType );

    string GetNewAttrName( int attr_type );
    void RenameAttr( NameValData* attr, const string & newName );

    string GetAttachID() const;
    int GetAttachType() const;
    bool GetAttrDataFlag() const;
    int GetNumAttrs() const;

    vector< string > GetAllAttrNames();
    vector< string > GetAllAttrIDs();

    void BuildCollectorVec( vector < AttributeCollection* > & in_vec );

    void ChangeID( const string & id );

    virtual void EncodeXml( xmlNodePtr & node ) const;
    virtual void DecodeXml( xmlNodePtr & node, bool retainIDs = false );

protected:

    string m_AttachID;
    int m_AttachType;
};

//======================================================================================//
//======================================================================================//
//======================================================================================//

//==== A Collection of Results Data From One Computation ====//
class Results : public NameValCollection
{
public:

    Results( const string & name, const string & id, const string & doc );

    virtual void Add( NameValData* d );
    virtual void Add( const vector< vector< vec3d > > & d, const string &prefix, const string &doc );

    void SetDateTime();

    void WriteCSVFile( const string & file_name );
    void WriteCSVFile( FILE* fid );
    void WriteMassProp( const string & file_name );
    void WriteCompGeomTxtFile( const string & file_name );
    void WriteCompGeomCsvFile( const string & file_name );
    void WriteDragBuildFile( const string & file_name );
    void WriteParasiteDragFile( const string & file_name );
    void WriteSliceFile( const string & file_name );
    void WriteWaveDragFile( const string & file_name );
    void WriteBEMFile( const string & file_name );

    time_t GetTimestamp()
    {
        return m_Timestamp;
    }

protected:

    time_t m_Timestamp;
    int m_Sec, m_Min, m_Hour;
    int m_Day, m_Month, m_Year;

};

//======================================================================================//
//======================================================================================//
//======================================================================================//




//==== Results Manager ====//
class ResultsMgrSingleton
{
public:
    static ResultsMgrSingleton& getInstance()
    {
        static ResultsMgrSingleton instance;
        return instance;
    }


    Results* CreateResults( const string & name, const string & doc );                      // Return Results Ptr

    string CreateGeomResults( const string & geom_id, const string & name );

    void DeleteAllResults();
    void DeleteResult( const string & id );

    int GetNumResults( const string & name );
    Results* FindResults( const string & name, int index = 0 );
    Results* FindResultsPtr( const string & id );
    string FindResultsID( const string & name, int index = 0 );
    string FindLatestResultsID( const string & name );
    int GetNumData( const string & results_id, const string & data_name );

    int GetResultsType( const string & results_id, const string & data_name );
    string GetResultsTypeName( const string & results_id, const string & data_name );
    string GetResultsEntryDoc( const string & results_id, const string & data_name );

    vector< string > GetAllResultsNames();
    vector< string > GetAllDataNames( const string & results_id );
    vector< string > GetAllResultsIDs();

    void PrintResults( const string &fname, const std::string &results_id );
    void PrintResults( const std::string &results_id );
    void PrintResults( FILE * outputStream, const std::string &results_id );

    void PrintResultsDocs( const string &fname, const std::string &results_id );
    void PrintResultsDocs( const std::string &results_id );
    void PrintResultsDocs( FILE * outputStream, const std::string &results_id );

    const vector<int> & GetIntResults( const string & id, const string & name, int index = 0 );
    const vector<double> & GetDoubleResults( const string & id, const string & name, int index = 0 );
    const vector<vector<int> > & GetIntMatResults( const string & id, const string & name, int index = 0 );
    const vector<vector<double> > & GetDoubleMatResults( const string & id, const string & name, int index = 0 );
    const vector<string> & GetStringResults( const string & id, const string & name, int index = 0 );
    const vector<vec3d> & GetVec3dResults( const string & id, const string & name, int index = 0 );
    time_t GetResultsTimestamp( const string & results_id );

    bool ValidResultsID( const string & results_id );
    bool ValidDataNameIndex( const string & results_id, const string & name, int index = 0 );

    void WriteTestResults();        // Write Some Test Results
    void TestSpeed();               // Test Speed

    static int WriteCSVFile( const string & file_name, const vector < string > &resids );

private:
    ResultsMgrSingleton();
    ~ResultsMgrSingleton();
    ResultsMgrSingleton( ResultsMgrSingleton const& copy );          // Not Implemented
    ResultsMgrSingleton& operator=( ResultsMgrSingleton const& copy ); // Not Implemented

    map< string, Results* > m_ResultsMap;                   // Map ID to Results
    map< string, vector< string > > m_NameIDMap;            // Map Name to ID

    //==== Default Return Vectors ====//
    vector< int > m_DefaultIntVec;
    vector< double > m_DefaultDoubleVec;
    vector< string > m_DefaultStringVec;
    vector< vec3d > m_DefaultVec3dVec;
    vector< vector< int > > m_DefaultIntMat;
    vector< vector< double > > m_DefaultDoubleMat;


};

#define ResultsMgr ResultsMgrSingleton::getInstance()

#endif // !defined(RESULTSMGR__INCLUDED_)
