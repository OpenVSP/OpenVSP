#include "Light.h"
#include "Vehicle.h"

Light::Light() : ParmContainer()
{
    m_Active.Init( "ActiveFlag", "Light_Parm", this, 0, 0, 1 );

    m_X.Init( "X", "Light_Parm", this, 0, -1.0e12, 1.0e12 );
    m_Y.Init( "Y", "Light_Parm", this, 0, -1.0e12, 1.0e12 );
    m_Z.Init( "Z", "Light_Parm", this, 0, -1.0e12, 1.0e12 );

    m_Amb.Init( "Ambient", "Light_Parm", this, 0, 0, 1.0 );
    m_Diff.Init( "Diffuse", "Light_Parm", this, 0, 0, 1.0 );
    m_Spec.Init( "Specular", "Light_Parm", this, 0, 0, 1.0 );
}
Light::~Light()
{
}

void Light::ParmChanged( Parm* parm_ptr, int type )
{
    if ( type == Parm::SET )
    {
        m_LateUpdateFlag = true;
        return;
    }

    VehicleMgr.GetVehicle()->ParmChanged( parm_ptr, type );
}
