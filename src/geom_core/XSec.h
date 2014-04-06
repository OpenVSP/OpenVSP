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

    enum { FUSE_SEC, NUM_XSEC_TYPES
         };

    virtual void ParmChanged( Parm* parm_ptr, int type );
    virtual void Update();

    virtual void SetGroupDisplaySuffix( int num );
    virtual string GetGroupName()
    {
        return m_GroupName;
    }

    virtual void SetRefLength( double len );

    virtual VspCurve& GetUntransformedCurve();
    virtual VspCurve& GetCurve();

    virtual XSecCurve* GetXSecCurve()
    {
        return m_XSCurve;
    }

    virtual int  GetType()
    {
        return m_Type;
    }

    virtual void AddLinkableParms( vector< string > & linkable_parm_vec, const string & link_container_id = string() );

    //==== Copy Between Different Types ====//
    virtual void CopyFrom( XSec* xs );
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual xmlNodePtr EncodeXSec( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXSec( xmlNodePtr & node );

    virtual void SetScale( double scale );

    virtual double ComputeArea( int num_pnts );

    void SetTransformation( const Matrix4d &mat, bool center );

    FractionParm m_XLocPercent;
    FractionParm m_YLocPercent;
    FractionParm m_ZLocPercent;

    Parm m_Spin;

    Parm m_XRotate;
    Parm m_YRotate;
    Parm m_ZRotate;

protected:

    Matrix4d m_rotation;
    bool m_center;

    int m_Type;

    string m_RefLengthParmID;
    double m_RefLength;

    string m_GroupName;
    string m_ParentGeomID;

    VspCurve m_TransformedCurve;

    XSecCurve *m_XSCurve;

    virtual void ChangeID( string id );
};


#endif // !defined(XSEC__INCLUDED_)
