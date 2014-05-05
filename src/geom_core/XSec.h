//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// XSec.h: Cross Section Base Class and Derived Classes
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(XSEC__INCLUDED_)
#define XSEC__INCLUDED_

#include "Parm.h"
#include "ParmContainer.h"
#include "Vec3d.h"
#include "VspSurf.h"
#include "XSecCurve.h"

#include <string>
#include <vector>
#include <memory>
#include <utility>

using std::string;

class Geom;

class XSec : public ParmContainer
{
public:
    XSec( XSecCurve *xsc, bool use_left );                                                      // Default Constructor
    virtual ~XSec();

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void Update() = 0;

    virtual void SetGroupDisplaySuffix( int num );
    virtual string GetGroupName()
    {
        return m_GroupName;
    }

    virtual string GetName();


    virtual VspCurve& GetUntransformedCurve();
    virtual VspCurve& GetCurve();

    virtual XSecCurve* GetXSecCurve()
    {
        return m_XSCurve;
    }

    virtual void SetXSecCurve( XSecCurve* xs_crv );

    virtual int  GetType()
    {
        return m_Type;
    }

    virtual Matrix4d* GetTransform();

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );

    //==== Copy Between Different Types ====//
    virtual void CopyFrom( XSec* xs );
    virtual void CopyBasePos( XSec* xs ) = 0;
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual xmlNodePtr EncodeXSec( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXSec( xmlNodePtr & node );

    virtual void SetScale( double scale );

    virtual double ComputeArea( int num_pnts );

    void SetTransformation( const Matrix4d &mat, bool center );



protected:

    Matrix4d m_rotation;
    bool m_center;

    int m_Type;

    string m_GroupName;
    int m_GroupSuffix;
    string m_ParentGeomID;

    VspCurve m_TransformedCurve;

    Matrix4d m_Transform;

    XSecCurve *m_XSCurve;

    virtual void ChangeID( string id );
};

class FuseXSec : public XSec
{
public:
    FuseXSec( XSecCurve *xsc, bool use_left );

    virtual void Update();

    virtual void SetRefLength( double len );

    virtual void CopyBasePos( XSec* xs );

    FractionParm m_XLocPercent;
    FractionParm m_YLocPercent;
    FractionParm m_ZLocPercent;

    Parm m_Spin;

    Parm m_XRotate;
    Parm m_YRotate;
    Parm m_ZRotate;

    Parm m_RefLength;

protected:
};

class StackXSec : public XSec
{
public:
    StackXSec( XSecCurve *xsc, bool use_left );

    virtual void SetScale( double scale );

    virtual void Update();

    virtual void CopyBasePos( XSec* xs );

    Parm m_XDelta;
    Parm m_YDelta;
    Parm m_ZDelta;

    Parm m_XRotate;
    Parm m_YRotate;
    Parm m_ZRotate;

    Parm m_XCenterRot;
    Parm m_YCenterRot;
    Parm m_ZCenterRot;

protected:
};

#endif // !defined(XSEC__INCLUDED_)
