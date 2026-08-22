//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// FitModelMgr.h: interface for the Fit Model Singleton.
//
//////////////////////////////////////////////////////////////////////

#if !defined(FITMODEL__INCLUDED_)
#define FITMODEL__INCLUDED_

#include "APIDefines.h"
#include "VehicleMgr.h"
#include "Vehicle.h"
#include "Geom.h"
#include "Parm.h"

#include <vector>
#include <string>

#define MIN_FIT_FILE_VER 1
#define CURRENT_FIT_FILE_VER 2

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
        m_UType = vsp::FIT_MODEL_FREE;
        m_WType = vsp::FIT_MODEL_FREE;
        m_UClosed = false;
        m_WClosed = false;
        m_MatchGeom = "";
        m_SurfIndx = 0;
        m_Dist = 0.0;
    }

    virtual ~TargetPt()
    {
    }

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
    int GetSurfIndx() const
    {
        return m_SurfIndx;
    }
    void SetSurfIndx( int surfindx )
    {
        m_SurfIndx = surfindx;
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
    // The distance from this point to the surface point it is matched to, as of the last time
    // something moved one of them.  Stored rather than calculated on demand because the browser
    // displays it and the GUI refreshes often; evaluating the surface once per point per frame
    // would not pay for itself.  Not written to file -- it is derived, and Load recalculates it.
    double GetDist() const
    {
        return m_Dist;
    }
    void SetDist( double d )
    {
        m_Dist = d;
    }

    void UpdateDist();
    void UpdateDist( Geom* matchgeom );

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
    int m_SurfIndx;
    vec2d m_UW;
    vec3d m_Pt;
    double m_Dist;
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

    // Reorder the target points worst fit first.  Recomputes the distances before sorting, so the
    // order reflects the model as it stands rather than whenever the distances were last measured.
    void SortTargetPtsByDist();

    // Move one target point within the list.  Returns where it ended up, which is the index passed
    // in when there is nothing to move.
    int MoveTargetPt( int index, int reorder_type );

    // Move the selected target point, carrying the selection along with it.
    void MoveCurrTargetPt( int reorder_type );

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

    // Undo the last Optimize, SearchTargetUW or RefineTargetUW.  Each of those snapshots the
    // optimization vector before it runs; Undo puts it back.  One level deep -- the snapshot is
    // spent once it has been used.
    bool CanUndo();
    bool Undo();

    virtual void LoadDrawObjs( vector< DrawObj* > & draw_obj_vec );

    /*
    * Get Feedback Group's name.
    */
    virtual std::string getFeedbackGroupName();

    /*!
    * Process feedbacks.
    */
    void SelectPoint( const string &gid, int index );

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

    void AddSelectedPts( const string &tgtGeomID );

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
    FitModelMgrSingleton( FitModelMgrSingleton const& copy ) = delete;            // Not Implemented
    FitModelMgrSingleton& operator=( FitModelMgrSingleton const& copy ) = delete; // Not Implemented
    ~FitModelMgrSingleton();

    void Init();
    void Wype();

    void BuildPtrVec();
    void ParmToX( double *x );
    void XtoParm( const double *x );

    // Take the snapshot Undo restores.  BuildPtrVec must have been called first, since the
    // optimization vector is laid out from the pointer vectors it builds.
    void SaveUndoState();

    // Throw the snapshot away.  Reordering the target points has to do this: the snapshot is laid
    // out in target point order, and UndoSignature cannot tell an order change apart when the
    // points that swapped describe themselves identically, so restoring it would put one point's
    // surface coordinates onto another.
    void ForgetUndoState();

    // What the snapshot was taken against.  The layout of the optimization vector depends on which
    // Parms are variables and on the free/fixed state of every target point, so restoring a
    // snapshot taken against a different setup would write values into the wrong slots.  Comparing
    // this is what stops that.
    string UndoSignature();
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

    vector< double > m_XPrevious;
    string m_UndoSignature;
    bool m_UndoValid;

    DrawObj m_TargetPntDrawObj;
    DrawObj m_TargetLineDrawObj;

    // The selected target point, drawn blue.  Separate DrawObjs because color is a property of the
    // DrawObj rather than of the points within it.
    DrawObj m_HighlightPntDrawObj;
    DrawObj m_HighlightLineDrawObj;

    string m_SaveFitFileName;
    string m_LoadFitFileName;
};

#define FitModelMgr FitModelMgrSingleton::getInstance()

#endif
