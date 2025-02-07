//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AnalysisMgr.h: Register, list, and execute analysis codes.
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(ANALYSISMGR__INCLUDED_)
#define ANALYSISMGR__INCLUDED_

#include "ResultsMgr.h"

#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

//=== RWCollection ===//

class RWCollection : public NameValCollection
{
public:
    RWCollection() {};

    virtual void Add( const NameValData & d );
    virtual void Add( const vector< vector< vec3d > > & d, const string &prefix, const string &doc );

    void Clear();

};

//=== Analysis ===//
class Analysis
{
public:

    Analysis( const char *name, const char *doc = "" )
    {
        m_Inputs.Init( name, doc );
    }

    virtual ~Analysis(){};

    virtual void SetDefaults() = 0;
    virtual string Execute() = 0;

    RWCollection m_Inputs;

};


//==== Analysis Manager ====//
class AnalysisMgrSingleton
{
public:
    static AnalysisMgrSingleton& getInstance()
    {
        static AnalysisMgrSingleton instance;
        return instance;
    }

    void Init();
    void Wype();
    void Renew();
    void ResetDefaults();

    int GetNumAnalysis() const;
    vector < string > ListAnalysis() const;
    Analysis* FindAnalysis( const string & name ) const;
    bool RegisterAnalysis( Analysis* asys );

    string ExecAnalysis( const string & analysis );

    bool ValidAnalysisName( const string & analysis );
    bool ValidAnalysisInputDataIndex( const string & analysis, const string & name, int index = 0 );

    int GetNumInputData( const string & analysis, const string & name );
    int GetAnalysisInputType( const string & analysis, const string & name );
    string GetAnalysisInputTypeName( const string & results_id, const string & data_name );
    string GetAnalysisInputDoc( const string & analysis, const string & name );

    void PrintAnalysisInputs( const string & fname, const std::string& analysis_name );
    void PrintAnalysisInputs( const std::string& analysis_name );
    void PrintAnalysisInputs( FILE * outputStream, const std::string& analysis_name );

    void PrintAnalysisDocs( const string & fname, const std::string& analysis_name );
    void PrintAnalysisDocs( const std::string& analysis_name );
    void PrintAnalysisDocs( FILE * outputStream, const string& analysis_name );

    const vector<int> & GetIntInputData( const string & analysis, const string & name, int index = 0 );
    const vector<double> & GetDoubleInputData( const string & analysis, const string & name, int index = 0 );
    const vector<string> & GetStringInputData( const string & analysis, const string & name, int index = 0 );
    const vector<vec3d> & GetVec3dInputData( const string & analysis, const string & name, int index = 0 );
    const vector<vector<int> > & GetIntMatInputData( const string & analysis, const string & name, int index );
    const vector<vector<double> > & GetDoubleMatInputData( const string & analysis, const string & name, int index );


    void SetAnalysisInputDefaults( const string & analysis );
    void SetIntAnalysisInput( const string & analysis, const string & name, const vector< int > & d, int index = 0 );
    void SetDoubleAnalysisInput( const string & analysis, const string & name, const vector< double > & d, int index = 0 );
    void SetStringAnalysisInput( const string & analysis, const string & name, const vector< string > & d, int index = 0 );
    void SetVec3dAnalysisInput( const string & analysis, const string & name, const vector< vec3d > & d, int index = 0 );

    void RegisterBuiltins();

    double GetLastAnalysisExecutionDuration()
    {
        return m_AnalysisExecutionDuration;
    }

private:
    AnalysisMgrSingleton();
    ~AnalysisMgrSingleton();
    AnalysisMgrSingleton( AnalysisMgrSingleton const& copy );          // Not Implemented
    AnalysisMgrSingleton& operator=( AnalysisMgrSingleton const& copy ); // Not Implemented

    map < string, Analysis* > m_AnalysisMap;    // Map unique name to analysis.

    double m_AnalysisExecutionDuration; // Time to execute most recent analysis

    //==== Default Return Vectors ====//
    vector< int > m_DefaultIntVec;
    vector< double > m_DefaultDoubleVec;
    vector< string > m_DefaultStringVec;
    vector< vec3d > m_DefaultVec3dVec;
    vector< vector< int > > m_DefaultIntMatVec;
    vector< vector< double > > m_DefaultDoubleMatVec;

};

#define AnalysisMgr AnalysisMgrSingleton::getInstance()

class BEMAnalysis : public Analysis
{
public:

    BEMAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class CompGeomAnalysis : public Analysis
{
public:

    CompGeomAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class DegenGeomAnalysis : public Analysis
{
public:

    DegenGeomAnalysis();

    virtual void SetDefaults();
    virtual string Execute();
};

class EmintonLordAnalysis : public Analysis
{
public:

    EmintonLordAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class MassPropAnalysis : public Analysis
{
public:

    MassPropAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class PlanarSliceAnalysis : public Analysis
{
public:

    PlanarSliceAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class ProjectionAnalysis : public Analysis
{
public:

    ProjectionAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class SurfacePatchAnalysis : public Analysis
{
public:

    SurfacePatchAnalysis();

    virtual void SetDefaults();
    virtual string Execute();
};

class WaveDragAnalysis : public Analysis
{
public:

    WaveDragAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

// This can be deprecated
class VSPAERODegenGeomAnalysis : public Analysis
{
public:

    VSPAERODegenGeomAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROComputeGeometryAnalysis : public Analysis
{
public:

    VSPAEROComputeGeometryAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROSinglePointAnalysis : public Analysis
{
public:

    VSPAEROSinglePointAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROSweepAnalysis : public Analysis
{
public:

    VSPAEROSweepAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROReadPreviousAnalysis : public Analysis
{
public:

    VSPAEROReadPreviousAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class ParasiteDragFullAnalysis : public Analysis
{
public:

    ParasiteDragFullAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

class CpSlicerAnalysis : public Analysis
{
    public:

    CpSlicerAnalysis();

    virtual void SetDefaults();
    virtual string Execute();

};

#endif // !defined(ANALYSISMGR__INCLUDED_)
