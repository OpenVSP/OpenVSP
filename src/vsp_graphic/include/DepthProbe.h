#ifndef _VSP_GRAPHIC_DEPTH_PROBE_H
#define _VSP_GRAPHIC_DEPTH_PROBE_H

#include <set>

#include "glm/glm.hpp"

namespace VSPGraphic
{
class Scene;
class Viewport;

/*!
* Finds the surface under a window pixel.  The scene's meshes are drawn, filled and depth only,
* into a small offscreen buffer around the pixel, so the answer does not depend on draw style:
* wire frame and transparent surfaces are hit like shaded ones.  Clipping planes still apply.
* The buffer is not multisampled, so silhouettes give the depth of a surface or none, never a
* blend of the two.  A GL context must be current.
*/
class DepthProbe
{
public:
    DepthProbe();
    virtual ~DepthProbe();

    /*!
    * The world point nearest the eye within radius pixels of window pixel x, y, in viewport vp.
    * Pixel x, y covers [x, x+1) x [y, y+1), origin at the lower left.  Returns false if no
    * surface is there, or the GL cannot render offscreen.  When objectId is given, it is set
    * to the getID() of the scene object hit.  When only is given, only the scene objects whose
    * getID() is in it can be hit, and the others do not hide them.
    */
    bool probe( Scene * scene, Viewport * vp, int x, int y, int radius, glm::vec3 & hit,
                unsigned int * objectId = nullptr, const std::set < unsigned int > * only = nullptr );

private:
    bool _setup( int size );

    unsigned int _fbo;
    unsigned int _color;
    unsigned int _depth;
    int _size;
};
}
#endif
