#if !defined(VSP_MATERIAL__INCLUDED_)
#define VSP_MATERIAL__INCLUDED_

#include "ParmContainer.h"
#include "Parm.h"

#include <string>

/*!
* Material Information.
*/
class Material : public ParmContainer
{
public:
    /*!
    * Construct a material.
    */
    Material();
    /*!
    * Destructor.
    */
    virtual ~Material();

public:
    /*!
    * Override from ParmContainer.
    */
    virtual void ParmChanged( Parm* parm_ptr, int type );

public:
    /*!
    * Name of this Material.
    */
    std::string m_Name;

    /*!
    * Ambient values.
    */
    Parm m_AmbientR;
    Parm m_AmbientG;
    Parm m_AmbientB;
    Parm m_AmbientA;

    /*!
    * Diffuse values.
    */
    Parm m_DiffuseR;
    Parm m_DiffuseG;
    Parm m_DiffuseB;
    Parm m_DiffuseA;

    /*!
    * Specular values.
    */
    Parm m_SpecularR;
    Parm m_SpecularG;
    Parm m_SpecularB;
    Parm m_SpecularA;

    /*!
    * Shininess.
    */
    Parm m_Shininess;

    /*!
    * Emission values.
    */
    Parm m_EmissionR;
    Parm m_EmissionG;
    Parm m_EmissionB;
    Parm m_EmissionA;
};
#endif