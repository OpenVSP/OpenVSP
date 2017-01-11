//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// WaveDragMgr.h
//
// Michael Waddington
//////////////////////////////////////////////////////////////////////

#if !defined(WAVEDRAG__INCLUDED_)
#define WAVEDRAG__INCLUDED_

#include "VehicleMgr.h"
#include "Vehicle.h"
#include "Geom.h"
#include "Parm.h"

#include <vector>
#include <string>
using std::string;
using std::vector;

class WaveDragSingleton : public ParmContainer
{
public:
    enum AREA_PLOT_TYPE { AREA_TOTAL = 0,
                          AREA_COMPONENTS = 1,
                          AREA_BUILDUP = 2
    };

    static WaveDragSingleton& getInstance()
    {
        static WaveDragSingleton instance;
        return instance;
    }

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    void Update();

    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

    string SliceAndAnalyze();
    string SliceAndAnalyze( int set, int numSlices, int numRots, double Mach,
                           const vector <string> & Flow_vec, bool Symm );

    void SetupTheta( int ntheta );
    void Setup( int nslice, int ncomp );

    string WaveDragSlice( int set, int numSlices, int numRots, double Mach,
                       const vector <string> & Flow_vec, bool Symm );

    double WaveDrag( int r );

    double WaveDrag( const vector <double> & x_vec, const vector <double> & area_vec, vector <double> &coeff );

    void CalcDrag();

    void CalcVolArea( const vector < double > &x, const vector < double > &S, double length, double &volume, double &maxArea ) const;

    void FitBuildup();

    void PushSliceResults( Results* res );
    void PushDragResults( Results* res );

    void EvalELCurve( const vector < double > &Spts, const vector < double > &x, vector < double > &S, double &xMaxDrag, int itheta );
    void EvalELCurve( const vector < double > &conpnts, const vector < double > &Spts, const vector < double > &x, vector < double > &S, double &xMaxDrag );

    void SearsHaack( const vector < double > &x, vector < double > & S, double Vol, double len ) const;
    void vonKarman( const vector < double > &x, vector < double > & S, double Sbase ) const;
    void Lighthill( const vector < double > &x, vector < double > & S, double Smax ) const;

    // Run Tab Parms
    IntParm m_NumSlices;
    IntParm m_NumRotSects;
    IntParm m_SelectedSetIndex;
    Parm m_MachNumber;

    string m_RefGeomID;
    IntParm m_RefFlag;
    Parm m_Sref;

    BoolParm m_SymmFlag;

    // Plot Tab Parms
    IntParm m_ThetaIndex;
    BoolParm m_PlaneFlag;
    Parm m_SlicingLoc;
    IntParm m_IdealBodyType;
    IntParm m_AreaPlotType;
    BoolParm m_PointFlag;

    // Tool
    string m_lastmeshgeomID;

    // Viewing Plane
    DrawObj m_ShadeMeshViewPlane;
    DrawObj m_ViewPlaneLine;

    // Flow Tab
    vector<string> m_SSFlow_vec;

    // Slicing Results

    int m_NTheta;
    int m_NComp;
    int m_NSlice;

    vector < string > m_CompIDVec;

    bool m_AmbigSubSurf;

    vector < vector < vector < double > > > m_CompSliceAreaDist;
    vector < vector < double > > m_SliceAreaDist;

    vector < vector < double > > m_SliceAreaDistFlow;

    double m_InletArea;
    double m_ExitArea;

    vector < double > m_StartX;
    vector < double > m_EndX;

    vector < double > m_ThetaRad;
    vector < double > m_XNorm;

    // Area Stats

    vector < double > m_Volume;
    vector < double > m_MaxArea;
    vector < double > m_Length;
    double m_MaxMaxArea;

    int m_NumPtsFit;
    vector < double > m_XNormFit;
    vector < vector < vector < double > > > m_CompFitAreaDist;
    vector < vector < vector < double > > > m_BuildupFitAreaDist;
    vector < vector < double > > m_FitAreaDist;

    vector < vector < double > > m_FitAreaDistFlow;

    vector < vector < vector < double > > > m_BuildupAreaDist;


    vector < double > m_XMaxDrag;

    // Drag Results

    double m_CDWave;
    int m_iMaxDrag;

    enum { MANUAL_REF = 0, COMPONENT_REF, };

private:

    WaveDragSingleton();
    WaveDragSingleton( WaveDragSingleton const& copy );            // Not Implemented
    WaveDragSingleton& operator=( WaveDragSingleton const& copy ); // Not Implemented

};

#define WaveDragMgr WaveDragSingleton::getInstance()

#endif
