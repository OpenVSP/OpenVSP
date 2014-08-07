#if !defined(VSP_MATERIAL_MANAGER__INCLUDED_)
#define VSP_MATERIAL_MANAGER__INCLUDED_

#include "MaterialRepo.h"

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
    * Construct a MaterialMgr with a default material.
    */
    MaterialMgr(double ambi[], double diff[], double spec[], double emis[], double shin);
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

    /*!
    * Get default material.
    */
    Material * getDefault();

public:
    /*!
    * Encode Mateiral Info to XML.
    */
    xmlNodePtr EncodeXml( xmlNodePtr & node );
    /*!
    * Decode Mateiral Info from XML.
    */
    xmlNodePtr DecodeXml( xmlNodePtr & node );

private:
    Material m_Default;
    Material m_Material;
};
#endif
