#include "Label.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

Label::Label() : ParmContainer()
{
    m_Stage = STAGE_ZERO;

    m_OriginU.Init("OriginU", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_OriginW.Init("OriginW", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);

    m_Red.Init("Red", "Label_Parm", this, 100, 0, 255, false);
    m_Green.Init("Green", "Label_Parm", this, 100, 0, 255, false);
    m_Blue.Init("Blue", "Label_Parm", this, 100, 0, 255, false);

    m_Size.Init("Size", "Label_Parm", this, 2.0, 1.0, 10.0, false);

    m_Visible = true;
}
Label::~Label()
{
}

void Label::ParmChanged( Parm* parm_ptr, int type )
{
    VehicleMgr::getInstance().GetVehicle()->ParmChanged( parm_ptr, type );
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

    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();
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

    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();
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
    m_RulerEndU.Init("RulerEndU", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_RulerEndW.Init("RulerEndW", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);

    m_XOffset.Init("X_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_YOffset.Init("Y_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);
    m_ZOffset.Init("Z_Offset", "Label_Parm", this, 0.0, -1.0e12, 1.0e12, false);

    m_Precision.Init("Precision", "Label_Parm", this, 0, 0, 10, false);
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