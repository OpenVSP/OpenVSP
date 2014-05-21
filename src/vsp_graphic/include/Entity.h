#ifndef _VSP_GRAPHIC_ENTITY_OBJECT_H
#define _VSP_GRAPHIC_ENTITY_OBJECT_H

#include "Renderable.h"
#include "TextureMgr.h"

namespace VSPGraphic
{
class Lighting;
class Material;

/*!
* This class represents a single geometry with surfaces in scene.
* Derived from Renderable.
*/
class Entity : public Renderable
{
public:
    /*!
    * Constructor.  Build an Entity Object without lighting.
    */
    Entity();
    /*!
    * Constructor.  Build an Entity Object with lighting.
    * lights - lighting information.
    */
    Entity( Lighting * lights );
    /*!
    * Destructor.
    */
    virtual ~Entity();

public:
    /*!
    * Set Lighting.
    */
    void setLighting( Lighting * lighting );
    /*!
    * Set Material.
    */
    void setMaterial( Material * material );

public:
    /*!
    * Get Texture Manager of this Entity.
    */
    TextureMgr * getTextureMgr()
    {
        return _textureMgr;
    }

protected:
    /*!
    * Preprocessing.
    */
    virtual void _predraw();
    /*!
    * Draw Geometry.
    */
    virtual void _draw();

protected:
    /*!
    * Draw shaded mesh.
    */
    virtual void _draw_Mesh_Shaded();
    /*!
    * Draw textured mesh.
    */
    virtual void _draw_Mesh_Textured();
    /*!
    * Draw wire frame.
    */
    virtual void _draw_Wire_Frame();
    /*!
    * Draw hidden.
    */
    virtual void _draw_Wire_Frame_Solid();

protected:
    /*!
    * Draw Mesh.
    */
    void _draw_Mesh();
    /*!
    * Draw Mesh with custom Color.
    */
    void _draw_Mesh( float r, float g, float b, float a = 1.f );
    /*!
    * Draw Wire.
    */
    void _draw_Wire();
    /*!
    * Draw Wire with custom color and width.
    */
    void _draw_Wire( float r, float g, float b, float a = 1.f, float lineWidth = 0.f );

protected:
    Material * _material;
    Lighting * _lighting;

    TextureMgr * _textureMgr;

private:
    Material * _getDefaultMaterial();

private:
    void _draw_Mesh_VBuffer();
    void _draw_Mesh_EBuffer();
    void _draw_Wire_VBuffer();
    void _draw_Wire_EBuffer();
    void _draw_VBuffer();
    void _draw_EBuffer();
};
}
#endif