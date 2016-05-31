//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Link.cpp: implementation of the Parm and Parm Container classes.
//
//////////////////////////////////////////////////////////////////////

#include "Link.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VehicleMgr.h"

//==== Constructor ====//
Link::Link()
{
    m_Name = "Link";
    m_OffsetFlag = true;
    m_Offset = 0.0;
    m_ScaleFlag = false;
    m_Scale = 1.0;
    m_LowerLimitFlag = false;
    m_LowerLimit = -1.0e12;
    m_UpperLimitFlag = false;
    m_UpperLimit =  1.0e12;

    m_Offset.Init( "Offset", "Link", this, 0.0, -1.0e12,  1.0e12 );
    m_Scale.Init( "Scale", "Link", this,   0.0, 1.0e-12, 1.0e12 );
    m_LowerLimit.Init( "LowerLimit", "Link", this, -100, -1.0e12, 1.0e12 );
    m_UpperLimit.Init( "UpperLimit", "Link", this,  100,    -1.0e12, 1.0e12 );

}

//==== Destructor ====//
Link::~Link()
{

}

void Link::SetParmA( string id  )
{
    m_ParmA = id;
}

void Link::SetParmB( string id )
{
    m_ParmB = id;
}

void Link::InitOffsetScale()
{
    Parm* pA = ParmMgr.FindParm( m_ParmA );
    Parm* pB = ParmMgr.FindParm( m_ParmB );

    if ( pA && pB )
    {
        double valA = pA->Get();
        double valB = pB->Get();

        //==== Initialize Offset and Scale ====//
        m_Offset = valB - valA;
        m_Scale  = 1.0;
        if ( fabs( valA ) > 0.0000001 )
        {
            m_Scale = valB / valA;
        }

        m_LowerLimit = pB->GetLowerLimit();
        m_UpperLimit = pB->GetUpperLimit();
    }
}

void Link::ParmChanged( Parm* parm_ptr, int type )
{
    Parm* pA = ParmMgr.FindParm( m_ParmA );

    if ( pA )
    {
        LinkMgr.ParmChanged( pA->GetID(), true );
    }
}

xmlNodePtr Link::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr link_node = xmlNewChild( node, NULL, BAD_CAST"Link", NULL );

    XmlUtil::AddStringNode( link_node, "ParmAID", m_ParmA );
    XmlUtil::AddStringNode( link_node, "ParmBID", m_ParmB );

    m_Offset.EncodeXml( link_node );
    m_Scale.EncodeXml( link_node );
    m_LowerLimit.EncodeXml( link_node );
    m_UpperLimit.EncodeXml( link_node );

    XmlUtil::AddIntNode( link_node, "OffsetFlag", m_OffsetFlag );
    XmlUtil::AddIntNode( link_node, "ScaleFlag", m_ScaleFlag );
    XmlUtil::AddIntNode( link_node, "LowerLimitFlag", m_LowerLimitFlag );
    XmlUtil::AddIntNode( link_node, "UpperLimitFlag", m_UpperLimitFlag );

    return link_node;
}

xmlNodePtr Link::DecodeXml( xmlNodePtr & link_node )
{
    if ( link_node )
    {
        m_ParmA = ParmMgr.RemapID( XmlUtil::FindString( link_node, "ParmAID", string() ) );
        m_ParmB = ParmMgr.RemapID( XmlUtil::FindString( link_node, "ParmBID", string() ) );

        m_Offset.DecodeXml( link_node );
        m_Scale.DecodeXml( link_node );
        m_LowerLimit.DecodeXml( link_node );
        m_UpperLimit.DecodeXml( link_node );

        m_OffsetFlag = !!XmlUtil::FindInt( link_node, "OffsetFlag", m_OffsetFlag );
        m_ScaleFlag = !!XmlUtil::FindInt( link_node, "ScaleFlag", m_ScaleFlag );
        m_LowerLimitFlag = !!XmlUtil::FindInt( link_node, "LowerLimitFlag", m_LowerLimitFlag );
        m_UpperLimitFlag = !!XmlUtil::FindInt( link_node, "UpperLimitFlag", m_UpperLimitFlag );
    }
    return link_node;
}
