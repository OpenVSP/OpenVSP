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

    void Clear();

};

//=== Analysis ===//
class Analysis
{
public:

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

    int GetNumAnalysis() const;
    vector < string > ListAnalysis() const;
    Analysis* FindAnalysis( const string & name ) const;
    bool RegisterAnalysis( const string & name, Analysis* asys );

    string ExecAnalysis( const string & analysis );

    bool ValidAnalysisName( const string & analysis );
    bool ValidAnalysisInputDataIndex( const string & analysis, const string & name, int index = 0 );

    int GetNumInputData( const string & analysis, const string & name );
    int GetAnalysisInputType( const string & analysis, const string & name );

    const vector<int> & GetIntInputData( const string & analysis, const string & name, int index = 0 );
    const vector<double> & GetDoubleInputData( const string & analysis, const string & name, int index = 0 );
    const vector<string> & GetStringInputData( const string & analysis, const string & name, int index = 0 );
    const vector<vec3d> & GetVec3dInputData( const string & analysis, const string & name, int index = 0 );

    void SetAnalysisInputDefaults( const string & analysis );
    void SetIntAnalysisInput( const string & analysis, const string & name, const vector< int > & d, int index = 0 );
    void SetDoubleAnalysisInput( const string & analysis, const string & name, const vector< double > & d, int index = 0 );
    void SetStringAnalysisInput( const string & analysis, const string & name, const vector< string > & d, int index = 0 );
    void SetVec3dAnalysisInput( const string & analysis, const string & name, const vector< vec3d > & d, int index = 0 );

    void RegisterBuiltins();

private:
    AnalysisMgrSingleton();
    ~AnalysisMgrSingleton();
    AnalysisMgrSingleton( AnalysisMgrSingleton const& copy );          // Not Implemented
    AnalysisMgrSingleton& operator=( AnalysisMgrSingleton const& copy ); // Not Implemented

    map < string, Analysis* > m_AnalysisMap;    // Map unique name to analysis.

    //==== Default Return Vectors ====//
    vector< int > m_DefaultIntVec;
    vector< double > m_DefaultDoubleVec;
    vector< string > m_DefaultStringVec;
    vector< vec3d > m_DefaultVec3dVec;

};

#define AnalysisMgr AnalysisMgrSingleton::getInstance()

class BEMAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class CompGeomAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class EmintonLordAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class MassPropAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class PlanarSliceAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class ProjectionAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class WaveDragAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

// This can be deprecated
class VSPAERODegenGeomAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROComputeGeometryAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROSinglePointAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

class VSPAEROSweepAnalysis : public Analysis
{
public:

    virtual void SetDefaults();
    virtual string Execute();

};

#endif // !defined(ANALYSISMGR__INCLUDED_)
