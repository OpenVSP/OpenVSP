//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ClippingMgr.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

ClippingMgr::ClippingMgr() : ParmContainer()
{
    m_Name = "ClippingMgr";

    m_XGTClip.Init( "XGTClip", "Clipping", this, 1.0, -1.0e12, 1.0e12 );
    m_XGTClip.SetDescript( "X > Clipping Plane" );

    m_XLTClip.Init( "XLTClip", "Clipping", this, -1.0, -1.0e12, 1.0e12 );
    m_XLTClip.SetDescript( "X < Clipping Plane" );

    m_YGTClip.Init( "YGTClip", "Clipping", this, 1.0, -1.0e12, 1.0e12 );
    m_YGTClip.SetDescript( "Y > Clipping Plane" );

    m_YLTClip.Init( "YLTClip", "Clipping", this, -1.0, -1.0e12, 1.0e12 );
    m_YLTClip.SetDescript( "Y < Clipping Plane" );

    m_ZGTClip.Init( "ZGTClip", "Clipping", this, 1.0, -1.0e12, 1.0e12 );
    m_ZGTClip.SetDescript( "Z > Clipping Plane" );

    m_ZLTClip.Init( "ZLTClip", "Clipping", this, -1.0, -1.0e12, 1.0e12 );
    m_ZLTClip.SetDescript( "Z < Clipping Plane" );

    m_XGTClipFlag.Init( "XGTClipFlag", "Clipping", this, false, 0, 1 );
    m_XLTClipFlag.Init( "XLTClipFlag", "Clipping", this, false, 0, 1 );
    m_YGTClipFlag.Init( "YGTClipFlag", "Clipping", this, false, 0, 1 );
    m_YLTClipFlag.Init( "YLTClipFlag", "Clipping", this, false, 0, 1 );
    m_ZGTClipFlag.Init( "ZGTClipFlag", "Clipping", this, false, 0, 1 );
    m_ZLTClipFlag.Init( "ZLTClipFlag", "Clipping", this, false, 0, 1 );
}

ClippingMgr::~ClippingMgr()
{
}

xmlNodePtr ClippingMgr::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = xmlNewChild( node, NULL, BAD_CAST"ClippingMgr", NULL );

    ParmContainer::EncodeXml( cfdsetnode );

    return cfdsetnode;
}

xmlNodePtr ClippingMgr::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr cfdsetnode = XmlUtil::GetNode( node, "ClippingMgr", 0 );
    if ( cfdsetnode )
    {
        ParmContainer::DecodeXml( cfdsetnode );
    }

    return cfdsetnode;
}

void ClippingMgr::LoadDrawObjs( vector< DrawObj* > &draw_obj_vec )
{
    m_ClipDO.m_Type = DrawObj::VSP_CLIP;

    m_ClipDO.m_ClipLoc[0] = m_XGTClip();
    m_ClipDO.m_ClipLoc[1] = m_XLTClip();
    m_ClipDO.m_ClipLoc[2] = m_YGTClip();
    m_ClipDO.m_ClipLoc[3] = m_YLTClip();
    m_ClipDO.m_ClipLoc[4] = m_ZGTClip();
    m_ClipDO.m_ClipLoc[5] = m_ZLTClip();

    m_ClipDO.m_ClipFlag[0] = m_XGTClipFlag();
    m_ClipDO.m_ClipFlag[1] = m_XLTClipFlag();
    m_ClipDO.m_ClipFlag[2] = m_YGTClipFlag();
    m_ClipDO.m_ClipFlag[3] = m_YLTClipFlag();
    m_ClipDO.m_ClipFlag[4] = m_ZGTClipFlag();
    m_ClipDO.m_ClipFlag[5] = m_ZLTClipFlag();

    draw_obj_vec.push_back( &m_ClipDO );
}
