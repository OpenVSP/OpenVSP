#include "ColorMgr.h"

ColorMgr::ColorMgr()
{
}

ColorMgr::~ColorMgr()
{
}

void ColorMgr::SetWireColor( double r, double g, double b, double a )
{
    m_WireColor.m_Red = r;
    m_WireColor.m_Green = g;
    m_WireColor.m_Blue = b;
    m_WireColor.m_Alpha = a;
}

void ColorMgr::SetOwnerID( const std::string &id )
{
    m_WireColor.SetParentContainer( id );
}

const Color * ColorMgr::GetWireColor() const
{
    return &m_WireColor;
}

xmlNodePtr ColorMgr::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = xmlNewChild( node, nullptr, BAD_CAST "Wire_Color", nullptr );

    m_WireColor.EncodeXml( child_node );

    return child_node;
}

xmlNodePtr ColorMgr::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr child_node = XmlUtil::GetNode( node, "Wire_Color", 0 );
    if( child_node )
    {
        m_WireColor.DecodeXml( child_node );
    }
    return child_node;
}