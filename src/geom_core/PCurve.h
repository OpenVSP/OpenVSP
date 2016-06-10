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

    enum {
        LINEAR, PCHIP, CEDIT,
    };

    IntParm m_CurveType;

    Parm m_SplitPt;

    IntParm m_ConvType;

    vector< Parm* > m_TParmVec;
    vector< Parm* > m_ValParmVec;

    virtual void ReservePts( int n );

    virtual void SetCurveName( const string & name );

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void AddPt();

    virtual void ParmChanged( Parm* parm_ptr, int type );

    virtual void Update();

    virtual double Comp( double t );
    virtual void Tessellate( vector < double > & t, vector < double > & v );

    virtual int GetNumPts()    { return m_ValParmVec.size(); }

    virtual vector < double > GetTVec();
    virtual vector < double > GetValVec();

    virtual void SetTVec( const vector < double > & vec );
    virtual void SetValVec( const vector < double > & vec );

    virtual void SetPt( double t, double v, int indx );

    virtual void Split();

    virtual void ConvertTo( int newtype );

    virtual void InitCurve( const vector < double > & tvec, const vector < double > & valvec );

    virtual void SetDispNames( const string & xname, const string & yname );
    virtual string GetXDsipName()    { return m_XDispName; }
    virtual string GetYDsipName()    { return m_YDispName; }

    virtual double IntegrateAF( double r0 );

    virtual void EnforcePtOrder( double rfirst, double rlast );

protected:

    virtual void ValidateCEDIT();
    virtual void ValidateCEDIT( vector < double > & vec );

    virtual void ClearPtOrder();

    Vsp1DCurve m_Curve;

    string m_CurveName;
    string m_GroupName;

    string m_XDispName;
    string m_YDispName;
};

#endif // !defined(VSP_PCURVE__INCLUDED_)
