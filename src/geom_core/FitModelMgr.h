//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FitModelMgr.h: interface for the Fit Model Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FITMODEL__INCLUDED_)
#define FITMODEL__INCLUDED_

#include "VehicleMgr.h"
#include "Vehicle.h"
#include "Geom.h"
#include "Parm.h"

#include <vector>
#include <string>

#define MIN_FIT_FILE_VER 1
#define CURRENT_FIT_FILE_VER 1

using std::string;
using std::vector;

struct SurfData
{
    Geom* m_GeomPtr;
    bool m_UClosed;
    bool m_WClosed;
};

class TargetPt
{
public:
    TargetPt()
    {
        m_UType = 0;
        m_WType = 0;
        m_UClosed = false;
        m_WClosed = false;
        m_MatchGeom = "";
    }

    virtual ~TargetPt()
    {
    }

    enum { FIXED, FREE };

    vec3d GetPt()
    {
        return m_Pt;
    }
    void SetPt( const vec3d &p )
    {
        m_Pt = p;
    }
    vec2d GetUW()
    {
        return m_UW;
    }
    void SetUW( const vec2d & uw )
    {
        m_UW = uw;
    }
    string GetMatchGeom()
    {
        return m_MatchGeom;
    }
    void SetMatchGeom( const string &geomid )
    {
        m_MatchGeom = geomid;
    }
    int GetUType()
    {
        return m_UType;
    }
    void SetUType( int type )
    {
        m_UType = type;
    }
    int GetWType()
    {
        return m_WType;
    }
    void SetWType( int type )
    {
        m_WType = type;
    }
    bool IsUClosed() const
    {
        return m_UClosed;
    }
    void SetUClosed( bool uClosed )
    {
        m_UClosed = uClosed;
    }
    bool IsWClosed() const
    {
        return m_WClosed;
    }
    void SetWClosed( bool wClosed )
    {
        m_WClosed = wClosed;
    }

    xmlNodePtr WrapXml( xmlNodePtr & node );
    xmlNodePtr UnwrapXml( xmlNodePtr & node );

    vec3d GetMatchPt();
    vec3d GetMatchPt(Geom* matchgeom);
    vec3d CalcDelta();
    vec3d CalcDelta( Geom* matchgeom );
    vec3d CalcDerivU( Geom* matchgeom );
    vec3d CalcDerivW( Geom* matchgeom );
    void SearchUW( Geom* matchgeom );
    void RefineUW( Geom* matchgeom );
    bool IsValid();

protected:

    int m_UType;
    int m_WType;
    bool m_UClosed;
    bool m_WClosed;
    string m_MatchGeom;
    vec2d m_UW;
    vec3d m_Pt;
};

//==== Fit Model Manager ====//
class FitModelMgrSingleton
{
public:
    static FitModelMgrSingleton& getInstance()
    {
        static FitModelMgrSingleton instance;
        return instance;
    }

    virtual void Renew();

    virtual bool AddCurrVar();
    virtual void DelCurrVar();
    virtual void DelAllVars();

    bool CheckForDuplicateVar( const string & p );

    virtual bool AddVar( const string& parm_id );
    virtual void DelVar( const string& parm_id );

    virtual void SetCurrVarIndex( int i )
    {
        m_CurrVarIndex = i;
    }
    virtual int  GetCurrVarIndex()
    {
        return m_CurrVarIndex;
    }
    virtual string GetCurrVar();
    virtual void CheckVars();

    virtual bool SortVars();

    virtual int GetNumVars()
    {
        return ( int )m_VarVec.size();
    }
    virtual string GetVar( int index );

    virtual vector < string > GetVarVec()
    {
        return m_VarVec;
    }

    virtual void SetWorkingParmID( string parm_id );

    virtual void ResetWorkingVar();

    void AddTargetPt( TargetPt *tpt );
    TargetPt* GetCurrTargetPt();
    virtual TargetPt* GetTargetPt( int index );
    void DelCurrTargetPt();
    void DelAllTargetPts();
    void ValidateTargetPts();

    int GetNumTargetPt()
    {
        return ( int )m_TargetPts.size();
    }
    int GetCurrTargetPtIndex()
    {
        return m_CurrTargetPtIndex;
    }
    void SetCurrTargetPtIndex( int i )
    {
        m_CurrTargetPtIndex = i;
    }

    int GetNumOptVars()
    {
        return m_NumOptVars;
    }

    void UpdateNumOptVars();

    void RefineTargetUW();
    void SearchTargetUW();

    void CalcMetrics( const double *x, double *y );
    void CalcMetricDeriv( const double *x, double *y, double *yprm );

    void UpdateDist();
    int Optimize();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    /*
    * Get Feedback Group's name.
    */
    virtual std::string getFeedbackGroupName();

    /*!
    * Process feedbacks.
    */
    void SelectPoint( string gid, int index );

    void SelectAllShown();
    void SelectNone();
    void SelectInv();
    void HideSelection();
    static void HideUnselected();
    void HideAll();
    void HideInv();
    static void ShowAll();
    void SetSelectOne();
    void SetSelectBox();

    void AddSelectedPts( string tgtGeomID );

    int GetNumSelected()
    {
        return m_NumSelected;
    }

    void SetGUIShown( bool f )
    {
        m_GUIShown = f;
    }
    bool IsGUIShown()
    {
        return m_GUIShown;
    }

    void SetSaveFitFileName( const string& fileName )
    {
        m_SaveFitFileName = fileName;
    }
    string GetSaveFitFileName()
    {
        return m_SaveFitFileName;
    }

    void SetLoadFitFileName( const string& fileName )
    {
        m_LoadFitFileName = fileName;
    }
    string GetLoadFitFileName()
    {
        return m_LoadFitFileName;
    }

    bool Save();
    int Load();

    double m_DistMetric;

private:

    FitModelMgrSingleton();
    FitModelMgrSingleton( FitModelMgrSingleton const& copy );            // Not Implemented
    FitModelMgrSingleton& operator=( FitModelMgrSingleton const& copy ); // Not Implemented

    void Init();
    void Wype();

    void BuildPtrVec();
    void ParmToX( double *x );
    void XtoParm( const double *x );
    static double Clamp01( double x, bool closed );

    bool m_GUIShown;

    int m_CurrVarIndex;
    int m_CurrTargetPtIndex;

    string m_WorkingParmID;

    string m_LastSelGeom;

    int m_NumSelected;

    vector < string > m_VarVec;
    vector < TargetPt* > m_TargetPts;

    // Normally a no-no to store Parm*'s.  However, these will be short-lived and will prevent
    // inner-loop lookup by the optimizer.
    vector < Parm* > m_ParmPtrVec;
    vector < Geom* > m_TargetGeomPtrVec;
    int m_NumOptVars;

    DrawObj m_TargetPntDrawObj;
    DrawObj m_TargetLineDrawObj;

    string m_SaveFitFileName;
    string m_LoadFitFileName;
};

#define FitModelMgr FitModelMgrSingleton::getInstance()

#endif
