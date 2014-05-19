#if !defined(VSP_MATERIAL_MANAGER__INCLUDED_)
#define VSP_MATERIAL_MANAGER__INCLUDED_

#include "Material.h"

/*!
* Manage and store Material for Geometry.
*/
class MaterialMgr
{
public:
    /*!
    * Construct a MaterialMgr.
    */
    MaterialMgr();
    /*!
    * Destructor.
    */
    virtual ~MaterialMgr();

public:
    /*!
    * Set Material.
    */
    void SetMaterial(Material * material);
    /*!
    * Set Material.
    */
    void SetMaterial(std::string name, double ambi[], double diff[], double spec[], double emis[], double shin);

    /*!
    * Get Material ptr.
    */
    Material * getMaterial();

private:
    Material m_Material;
};
#endif