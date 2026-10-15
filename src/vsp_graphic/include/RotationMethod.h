#ifndef _VSP_GRAPHIC_ROTATION_METHOD_H
#define _VSP_GRAPHIC_ROTATION_METHOD_H

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace VSPGraphic
{
class Camera;

/*!
* How a mouse drag turns the view.  begin() is called at press and drag() for each
* mouse move.  Mouse locations are in window pixels, origin at the lower left.
*/
class RotationMethod
{
public:
    virtual ~RotationMethod()
    {
    }

    virtual void begin( Camera * cam, int x, int y ) = 0;
    virtual void drag( Camera * cam, int x, int y ) = 0;

protected:
    RotationMethod()
    {
        _version = 0;
    }

    /*!
    * For methods measured from the press: begin again from the current view and mouse
    * location if anything else turned the view since this method last set it.
    */
    void rebaseIfTurned( Camera * cam, int x, int y );

    // The camera's rotationVersion() after this method last read or set the rotation.
    int _version;
};

/*!
* OpenVSP's rotation through 3.55: ArcballCam::rotate() from the previous mouse
* location to the current one.  Kept exactly as it was.
*/
class ClassicBall : public RotationMethod
{
public:
    ClassicBall();

    virtual void begin( Camera * cam, int x, int y );
    virtual void drag( Camera * cam, int x, int y );

private:
    int _px;
    int _py;
};
}
#endif
