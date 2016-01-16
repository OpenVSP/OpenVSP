#ifndef _VSP_GRAPHIC_MATERIAL_H
#define _VSP_GRAPHIC_MATERIAL_H

namespace VSPGraphic
{
/*!
* Material class.
*/
class Material
{
public:
    /*!
    * Constructor a default material.
    */
    Material();
    /*!
    * Construct a material with set values.
    *
    * ambi - Ambient value for this material.  Ambient must be a four elements array.
    * diff - Diffuse value for this material.  Diffuse must be a four elements array.
    * spec - Specular value for this material.  Specular must be a four elements array.
    * emis - Emission value for this material.  Emission must be a four elements array.
    * shin - Shininess of the object.  Value is between 0 and 128.
    */
    Material(float ambi[], float diff[], float spec[], float emis[], float shin);
    /*!
    * Destructor.
    */
    virtual ~Material();

public:
    /*!
    * Apply Material to current context.
    */
    void bind();

public:
    /*!
    * Light values.
    */
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emission[4];
    float shininess;
};
}
#endif
