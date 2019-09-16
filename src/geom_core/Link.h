//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Link.h: Contain and manage links between two parms
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_LINK__INCLUDED_)
#define VSP_LINK__INCLUDED_

#include <string>
#include <climits>

#include "Parm.h"
#include "ParmContainer.h"

using std::string;

//==== Link ====//
class Link : public ParmContainer
{
public:
    Link();
    virtual ~Link();

    virtual void SetParmA( string id );
    virtual void SetParmB( string id );
    virtual string GetParmA() const             { return m_ParmA; }
    virtual string GetParmB() const             { return m_ParmB; }
    virtual void InitOffsetScale();

    virtual void SetOffsetFlag( bool f )        { m_OffsetFlag = f; }
    virtual bool GetOffsetFlag() const          { return m_OffsetFlag; }

    virtual void SetScaleFlag( bool f )         { m_ScaleFlag = f; }
    virtual bool GetScaleFlag() const           { return m_ScaleFlag; }

    virtual void SetLowerLimitFlag( bool f )    { m_LowerLimitFlag = f; }
    virtual bool GetLowerLimitFlag() const      { return m_LowerLimitFlag; }

    virtual void SetUpperLimitFlag( bool f )    { m_UpperLimitFlag = f; }
    virtual bool GetUpperLimitFlag() const      { return m_UpperLimitFlag; }

    virtual void ParmChanged( Parm* parm_ptr, int type );

    xmlNodePtr EncodeXml( xmlNodePtr & node );
    xmlNodePtr DecodeXml( xmlNodePtr & node );

    Parm m_Offset;
    Parm m_Scale;
    Parm m_LowerLimit;
    Parm m_UpperLimit;

protected:

    string m_ParmA;
    string m_ParmB;

    bool m_OffsetFlag;
    bool m_ScaleFlag;
    bool m_LowerLimitFlag;
    bool m_UpperLimitFlag;

private:

    //==== Cannot Copy Link Because Of Parm Registration With ParmMgr ====//
    Link( Link const& copy );               // Not Implemented
    Link& operator=( Link const& copy );    // Not Implemented


};



#endif // !defined(VSP_LINK__INCLUDED_)
