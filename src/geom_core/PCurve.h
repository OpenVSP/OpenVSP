//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// PCurve.h: Parametric curve class
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_PCURVE__INCLUDED_)
#define VSP_PCURVE__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"
#include "Vsp1DCurve.h"

class PCurve : public ParmContainer
{
public:
    PCurve();
    ~PCurve();

    IntParm m_CurveType;

    Parm m_SplitPt;

    IntParm m_ConvType;

    vector< Parm* > m_TParmVec;
    vector< Parm* > m_ValParmVec;
    vector < BoolParm* > m_EnforceG1Vec; // indicates whether or not to enforce G1 continuity for each CEDIT point on the curve,

    virtual void InitParms();

    virtual void ReservePts( int n );

    virtual void SetCurveName( const string & name );
    virtual string GetCurveName()     { return m_CurveName; }

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddPt();
    virtual void DeletePt();
    virtual void DeletePt( int indx );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void Update();

    virtual double Comp( double t );
    virtual double Compdt( double t );
    virtual void Tessellate( vector < double > & t, vector < double > & v );

    virtual int GetNumPts()    { return m_ValParmVec.size(); }

    virtual vector < double > GetTVec();
    virtual vector < double > GetValVec();
    virtual vector < bool > GetG1Vec();

    virtual void SetTVec( const vector < double > & vec );
    virtual void SetValVec( const vector < double > & vec );
    virtual void SetG1Vec( const vector < bool >& vec );

    virtual void RenameParms();

    virtual void SetPt( double t, double v, int indx, bool force_update = false );

    virtual int Split();
    virtual int Split( const double & tsplit );

    virtual double GetRFirst();
    virtual double GetRLast();

    virtual void Approximate();
    virtual void ConvertTo( int newtype );

    virtual void InitCurve( const vector < double >& tvec, const vector < double >& valvec, vector < bool > c1vec = {} );

    virtual void SetCurve( const vector < double > & tvec, const vector < double > & valvec, int newtype, vector < bool > c1vec = {} );

    virtual void SetDispNames( const string & xname, const string & yname );
    virtual string GetXDsipName()    { return m_XDispName; }
    virtual string GetYDsipName()    { return m_YDispName; }

    virtual void SetParmNames( const string & xname, const string & yname );

    virtual double IntegrateCrv( );
    virtual double IntegrateCrv( double r0 );
    virtual double IntegrateCrv_r( );
    virtual double IntegrateCrv_r( double r0 );
    virtual double IntegrateCrv_rsq( );
    virtual double IntegrateCrv_rsq( double r0 );
    virtual double IntegrateCrv_rcub( );
    virtual double IntegrateCrv_rcub( double r0 );

    virtual void EnforcePtOrder( double rfirst, double rlast );
    virtual void EnforcePtOrder( );

    void BinCubicTMap( vector < double > &tmap, vector < double > &tdisc );
    void GetTMap( vector < double > &tmap, vector < double > &tdisc );

    virtual Vsp1DCurve * GetCurve()    { return &m_Curve; }

    virtual void SetSelectPntID( int id );
    virtual int GetSelectedPntID() { return m_SelectPntID; }

protected:

    virtual void ValidateCEDIT();
    virtual void ValidateCEDIT( vector < double > & vec );

    virtual void UpdateG1Parms();

    virtual void ClearPtOrder();

    virtual void EnforceG1( int new_index = -1 );

    Vsp1DCurve m_Curve;

    string m_CurveName;
    string m_GroupName;

    string m_XDispName;
    string m_YDispName;

    string m_XParmName;
    string m_YParmName;

    int m_SelectPntID;

    bool m_EnforceG1Next; // flag to indicate if G1 should be enforced with the next or previous point. 
};

#endif // !defined(VSP_PCURVE__INCLUDED_)
