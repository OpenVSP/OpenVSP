//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// ClippingMgr.h
//////////////////////////////////////////////////////////////////////


#if !defined(CLIPPINGMGR__INCLUDED_)
#define CLIPPINGMGR__INCLUDED_

#include "Vehicle.h"

#include <assert.h>

#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////
class ClippingMgr : public ParmContainer
{
public:
    ClippingMgr();
    virtual ~ClippingMgr();

    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    virtual void ParmChanged( Parm* parm_ptr, int type );

    void LoadDrawObjs( vector< DrawObj* > &draw_obj_vec );

    Parm m_XGTClip;
    Parm m_XLTClip;
    Parm m_YGTClip;
    Parm m_YLTClip;
    Parm m_ZGTClip;
    Parm m_ZLTClip;

    BoolParm m_XGTClipFlag;
    BoolParm m_XLTClipFlag;
    BoolParm m_YGTClipFlag;
    BoolParm m_YLTClipFlag;
    BoolParm m_ZGTClipFlag;
    BoolParm m_ZLTClipFlag;

    DrawObj m_ClipDO;
};

#endif



