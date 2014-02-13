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
    * Constructor.
    */
    Material();
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