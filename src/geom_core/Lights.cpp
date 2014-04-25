#include "Lights.h"
#include "Vehicle.h"
#include "VehicleMgr.h"

Light::Light() : ParmContainer()
{
    m_Active.Init( "ActiveFlag", "Light_Parm", this, 0, 0, 1, false );

    m_X.Init( "X", "Light_Parm", this, 0, -1.0e12, 1.0e12, false );
    m_Y.Init( "Y", "Light_Parm", this, 0, -1.0e12, 1.0e12, false );
    m_Z.Init( "Z", "Light_Parm", this, 0, -1.0e12, 1.0e12, false );

    m_Amb.Init( "Ambient", "Light_Parm", this, 0, 0, 1.0, false );
    m_Diff.Init( "Diffuse", "Light_Parm", this, 0, 0, 1.0, false );
    m_Spec.Init( "Specular", "Light_Parm", this, 0, 0, 1.0, false );
}
Light::~Light()
{
}

void Light::ParmChanged( Parm* parm_ptr, int type )
{
    Vehicle* vPtr = VehicleMgr::getInstance().GetVehicle();
    if ( vPtr )
        vPtr->ParmChanged( parm_ptr, Parm::SET );
}


Lights::Lights()
{
    for( int i = 0; i < NUMOFLIGHTS; i++ )
    {
        m_Lights.push_back(new Light());
    }

    // Setup default lights.
    m_Lights[0]->m_Active = true;
    m_Lights[0]->m_X = 10.0;
    m_Lights[0]->m_Y = -50.0;
    m_Lights[0]->m_Z = 20.0;
    m_Lights[0]->m_Amb = 0.5;
    m_Lights[0]->m_Diff = 0.35;
    m_Lights[0]->m_Spec = 1.0;

    m_Lights[1]->m_Active = true;
    m_Lights[1]->m_X = 10.0;
    m_Lights[1]->m_Y = 15.0;
    m_Lights[1]->m_Z = 30.0;
    m_Lights[1]->m_Amb = 0.5;
    m_Lights[1]->m_Diff = 0.5;
    m_Lights[1]->m_Spec = 1.0;

    m_Lights[2]->m_Active = true;
    m_Lights[2]->m_X = -50.0;
    m_Lights[2]->m_Y = 30.0;
    m_Lights[2]->m_Z = 10.0;
    m_Lights[2]->m_Amb = 0.0;
    m_Lights[2]->m_Diff = 0.5;
    m_Lights[2]->m_Spec = 0.5;
}
Lights::~Lights()
{
    for( int i = 0; i < m_Lights.size(); i++ )
    {
        delete m_Lights[i];
    }
}

Light * Lights::Get( unsigned int index )
{
    if( index >= 0 && index < NUMOFLIGHTS )
    {
        return m_Lights[index];
    }
    return NULL;
}

std::vector< Light* > Lights::GetVec()
{
    return m_Lights;
}