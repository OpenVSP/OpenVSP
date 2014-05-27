#include "Material.h"

Material::Material()
{
    m_Name = "Default";

    m_AmbientR.Init("Ambient_R", "Material_Parm", this, 0.2, 0, 1, false);
    m_AmbientG.Init("Ambient_G", "Material_Parm", this, 0.2, 0, 1, false);
    m_AmbientB.Init("Ambient_B", "Material_Parm", this, 0.2, 0, 1, false);
    m_AmbientA.Init("Ambient_A", "Material_Parm", this, 1, 0, 1, false);

    m_DiffuseR.Init("Diffuse_R", "Material_Parm", this, 0.8, 0, 1, false);
    m_DiffuseG.Init("Diffuse_G", "Material_Parm", this, 0.8, 0, 1, false);
    m_DiffuseB.Init("Diffuse_B", "Material_Parm", this, 0.8, 0, 1, false);
    m_DiffuseA.Init("Diffuse_A", "Material_Parm", this, 1, 0, 1, false);

    m_SpecularR.Init("Specular_R", "Material_Parm", this, 0, 0, 1, false);
    m_SpecularG.Init("Specular_G", "Material_Parm", this, 0, 0, 1, false);
    m_SpecularB.Init("Specular_B", "Material_Parm", this, 0, 0, 1, false);
    m_SpecularA.Init("Specular_A", "Material_Parm", this, 1, 0, 1, false);

    m_EmissionR.Init("Emission_R", "Material_Parm", this, 0, 0, 1, false);
    m_EmissionG.Init("Emission_G", "Material_Parm", this, 0, 0, 1, false);
    m_EmissionB.Init("Emission_B", "Material_Parm", this, 0, 0, 1, false);
    m_EmissionA.Init("Emission_A", "Material_Parm", this, 1, 0, 1, false);

    m_Shininess.Init("Shininess", "Material_Parm", this, 128, 0, 128, false);
}
Material::~Material()
{
}

void Material::ParmChanged( Parm* parm_ptr, int type )
{
}