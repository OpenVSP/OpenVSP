#include "Label.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

Label::Label() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_Type = TYPE_LABEL;

    m_OriginU.Init("OriginU", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);
    m_OriginW.Init("OriginW", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);

    m_Red.Init("Red", "Label_Parm", this, 100, 0, 255);
    m_Green.Init("Green", "Label_Parm", this, 100, 0, 255);
    m_Blue.Init("Blue", "Label_Parm", this, 100, 0, 255);

    m_Size.Init("Size", "Label_Parm", this, 2.0, 1.0, 10.0);

    m_Visible = true;
}
Label::~Label()
{
}

void Label::ParmChanged( Parm* parm_ptr, int type )
{
    VehicleMgr.GetVehicle()->ParmChanged( parm_ptr, type );
}

xmlNodePtr Label::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "Type", m_Type );
        XmlUtil::AddStringNode( parmcontain_node, "Stage", m_Stage );
        XmlUtil::AddStringNode( parmcontain_node, "OriginGeomID", m_OriginGeomID );
    }

    return parmcontain_node;
}

xmlNodePtr Label::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = ParmContainer::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_Stage = XmlUtil::FindString( parmcontain_node, "Stage", STAGE_ZERO );
        m_OriginGeomID = XmlUtil::FindString( parmcontain_node, "OriginGeomID", "" );
    }

    return parmcontain_node;
}

void Label::Reset()
{
    m_Stage = STAGE_ZERO;

    m_OriginU = 0.0;
    m_OriginW = 0.0;

    m_Red = 100.0;
    m_Green = 100.0;
    m_Blue = 100.0;

    m_Size = 2.0;
}

vec2d Label::MapToUW(std::string geomId, vec3d xyz)
{
    double u, w;
    u = w = 0.0;

    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom(geomId);

    assert(geom);

    if(geom)
    {
        geom->GetSurfPtr()->FindNearest01(u, w, xyz);
    }

    return vec2d(u, w);
}

vec3d Label::MapToXYZ(std::string geomId, vec2d uw)
{
    vec3d xyz;

    Vehicle* vPtr = VehicleMgr.GetVehicle();
    Geom * geom = vPtr->FindGeom(geomId);

    assert(geom);

    if(geom)
    {
        xyz = geom->GetSurfPtr()->CompPnt01(uw.x(), uw.y());
    }

    return xyz;
}

Ruler::Ruler() : Label()
{
    m_Type = TYPE_RULER;

    m_RulerEndU.Init("RulerEndU", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);
    m_RulerEndW.Init("RulerEndW", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);

    m_XOffset.Init("X_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);
    m_YOffset.Init("Y_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);
    m_ZOffset.Init("Z_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12);

    m_Precision.Init("Precision", "Label_Parm", this, 0, 0, 10);
}
Ruler::~Ruler()
{
}

void Ruler::Reset()
{
    Label::Reset();

    m_RulerEndU = 0.0;
    m_RulerEndW = 0.0;

    m_XOffset = 0.0;
    m_YOffset = 0.0;
    m_ZOffset = 0.0;

    m_Precision = 0.0;
}

xmlNodePtr Ruler::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = Label::EncodeXml( node );

    if ( parmcontain_node )
    {
        XmlUtil::AddStringNode( parmcontain_node, "RulerEndGeomID", m_RulerEndGeomID );
    }

    return parmcontain_node;
}

xmlNodePtr Ruler::DecodeXml( xmlNodePtr & node )
{
    xmlNodePtr parmcontain_node = Label::DecodeXml( node );

    if ( parmcontain_node )
    {
        m_RulerEndGeomID = XmlUtil::FindString( parmcontain_node, "RulerEndGeomID", "" );
    }

    return parmcontain_node;
}
