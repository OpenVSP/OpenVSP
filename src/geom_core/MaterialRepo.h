#if !defined(VSP_MATERIAL_REPOSITORY__INCLUDE_)
#define VSP_MATERIAL_REPOSITORY__INCLUDE_

#include "ParmContainer.h"
#include "Parm.h"

#include <string>
#include <vector>

struct MaterialPref
{
    std::string name;

    double ambi[4];
    double diff[4];
    double spec[4];
    double emis[4];

    float shininess;
};


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

/*!
* Repository for materials.
*/
class MaterialRepoSingleton
{
public:
    /*!
    * Singleton entry.
    */
    static MaterialRepoSingleton& GetInstance()
    {
        static MaterialRepoSingleton repo;
        return repo;
    }

public:
    /*!
    * Find material with specific name.
    */
    bool FindMaterial( std::string name, MaterialPref& mat_out);
    /*!
    * Find mateiral with index.
    */
    bool FindMaterial( int index, MaterialPref& mat_out );
    /*!
    * Get all material names.
    */
    std::vector<std::string> GetNames();

protected:
    /*!
    * Constructor.
    */
    MaterialRepoSingleton();
    /*!
    * Destructor.
    */
    virtual ~MaterialRepoSingleton();

private:
    std::vector<MaterialPref> m_Materials;
};

#define MaterialRepo MaterialRepoSingleton::GetInstance()

#endif
