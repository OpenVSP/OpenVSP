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

#include <map>
#include <list>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

//==== Results Data - Named Vectors Of Ints/Double/Strings or Vec3d ====//
class NameValData
{
public:
    NameValData();
    NameValData( const string & name );
    NameValData( const string & name, const int & i_data );
    NameValData( const string & name, const double & d_data );
    NameValData( const string & name, const string & s_data );
    NameValData( const string & name, const vec3d & v_data );
    NameValData( const string & name, const vector< int > & i_data );
    NameValData( const string & name, const vector< double > & d_data );
    NameValData( const string & name, const vector< string > & s_data );
    NameValData( const string & name, const vector< vec3d > & v_data );
    NameValData( const string & name, const vector< vector< double > > &dmat_data );

    void Init( const string & name, int type = 0, int index = 0 );

    string GetName() const
    {
        return m_Name;
    }
    int GetType() const
    {
        return m_Type;
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
    const vector<vec3d> & GetVec3dData() const
    {
        return m_Vec3dData;
    }
    const vector< vector< double > > & GetDoubleMatData() const
    {
        return m_DoubleMatData;
    }

    int GetInt( int index ) const;
    double GetDouble( int index ) const;
    double GetDouble( int row, int col ) const;
    string GetString( int index ) const;
    vec3d GetVec3d( int index ) const;

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
    void SetVec3dData( const vector< vec3d > & d )
    {
        m_Vec3dData = d;
    }
    void SetDoubleMatData( const vector< vector< double > > & d )
    {
        m_DoubleMatData = d;
    }

protected:

    string m_Name;
    int m_Type;
    vector< int > m_IntData;
    vector< double > m_DoubleData;
    vector< string > m_StringData;
    vector< vec3d > m_Vec3dData;
    vector< vector< double > >  m_DoubleMatData;

};


//======================================================================================//
//======================================================================================//
//======================================================================================//


//==== A Collection of Results Data From One Computation ====//
class NameValCollection
{
public:
    NameValCollection() {};
    NameValCollection( const string & name, const string & id );

    string GetName()
    {
        return m_Name;
    }
    string GetID()
    {
        return m_ID;
    }

    void Add( const NameValData & d );
    void Add( const vector< vector< vec3d > > & d, string prefix );

    int GetNumData( const string & name );
    vector< string > GetAllDataNames();
    NameValData Find( const string & name, int index = 0 );
    NameValData* FindPtr( const string & name, int index = 0 );

protected:

    string m_Name;
    string m_ID;

    //==== All The Data For This Computation Result =====//
    map< string, vector< NameValData > > m_DataMap;

};

//==== A Collection of Results Data From One Computation ====//
class Results : public NameValCollection
{
public:

    Results( const string & name, const string & id );

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

    void Copy( NameValData* nvd );

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


    Results* CreateResults( const string & name );                      // Return Results Ptr

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

    vector< string > GetAllResultsNames();
    vector< string > GetAllDataNames( const string & results_id );

    void PrintResults( const string &fname, const std::string &results_id );
    void PrintResults( const std::string &results_id );
    void PrintResults( FILE * outputStream, const std::string &results_id );

    const vector<int> & GetIntResults( const string & id, const string & name, int index = 0 );
    const vector<double> & GetDoubleResults( const string & id, const string & name, int index = 0 );
    const vector<vector<double>> & GetDoubleMatResults( const string & id, const string & name, int index = 0 );
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
    vector< vector< double > > m_DefaultDoubleMat;


};

#define ResultsMgr ResultsMgrSingleton::getInstance()

#endif // !defined(RESULTSMGR__INCLUDED_)
