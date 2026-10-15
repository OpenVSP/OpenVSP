#ifndef _VSP_GRAPHIC_ROTATION_METHOD_H
#define _VSP_GRAPHIC_ROTATION_METHOD_H

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <vector>

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

    /*!
    * Circles that show the method's shape on screen, centered at cx, cy, in window pixels.
    * None by default.
    */
    virtual void outline( Camera * cam, float & cx, float & cy, std::vector < float > & radii )
    {
        cx = 0.0f;
        cy = 0.0f;
        radii.clear();
    }

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

    /*!
    * Multiplier on the angle each mouse movement turns; 1 is OpenVSP's.
    */
    void setSpeed( double speed );

    virtual void begin( Camera * cam, int x, int y );
    virtual void drag( Camera * cam, int x, int y );

private:
    int _px;
    int _py;
    double _speed;
};

/*!
* Bell's trackball, as in SGI's tools, measured from the press.  The mouse is mapped onto a
* sphere whose radius is half the smaller viewport dimension, about the viewport center,
* out to 1 / sqrt(2) of that radius, and beyond it onto a sheet that falls off as 1 / r,
* joined to the sphere with the same height and slope.  Near the center a drag turns the
* view as OpenVSP's classic ball does at the same speed; farther out a drag turns it less
* and less.  Each drag turns by the angle between the press and current points, times the
* speed, about their common normal, so a drag that returns to where it started restores
* the view.  Every point lies in front of the sphere's center, so the axis is always
* defined and no drag is restarted.  If something else turns the view during a drag, the
* drag continues from there.
*/
class Trackball : public RotationMethod
{
public:
    Trackball();

    /*!
    * Multiplier on the angle between the press and current points.
    */
    void setSpeed( double speed );

    virtual void begin( Camera * cam, int x, int y );
    virtual void drag( Camera * cam, int x, int y );

    /*!
    * Point on the ball under a mouse location, in eye coordinates.
    */
    glm::dvec3 ballPoint( Camera * cam, int x, int y );

private:
    double _speed;

    glm::dvec3 _p0;
    glm::dquat _rot0;
};

/*!
* Shoemake's arcball with a rounded edge (Shambaugh, 2024).  The mouse is mapped onto a
* sphere joined by a circular fillet to a flat ring.  Each drag rotates by a multiple of
* the angle between the press and current points, so the result depends only on those two
* points: a drag that returns to where it started restores the view.  Outside the
* fillet the rotation is a pure roll about the view direction.  If something else turns
* the view during a drag, the drag continues from there.
*/
class RoundedArcball : public RotationMethod
{
public:
    RoundedArcball();

    /*!
    * size - ball diameter as a fraction of the smaller viewport dimension, 0.1 to 1.5.
    * border - fillet width as a fraction of the sphere radius, 0 to 1.  A narrow fillet
    * makes the rotation speed up sharply where the sphere meets it: 5.5 times the speed at
    * the center for 0.2, 2.6 times for 0.5.
    */
    void setShape( double size, double border );

    /*!
    * Rotation angle as a multiple of the angle between the press and current points on
    * the ball.  2 is Shoemake's: one crossing of the ball turns 360 degrees, and separate
    * drags compose exactly.  1 turns half as fast.  At any scale but 2 a drag is restarted
    * from the current point before the two points become opposite, where the axis is
    * undefined.
    */
    void setAngleScale( double scale );

    /*!
    * Center the ball on a window location, such as the screen position of a pivot, instead
    * of the viewport center.
    */
    void setCenter( bool use, float cx, float cy );

    virtual void begin( Camera * cam, int x, int y );
    virtual void drag( Camera * cam, int x, int y );

    /*!
    * The edge of the sphere, where the fillet starts, and the outer edge of the fillet.
    * Outside the outer circle a drag rolls.
    */
    virtual void outline( Camera * cam, float & cx, float & cy, std::vector < float > & radii );

    /*!
    * Point on the ball under a mouse location, in eye coordinates.
    */
    glm::dvec3 ballPoint( Camera * cam, int x, int y );

private:
    double _size;
    double _border;
    double _angleScale;

    bool _useCenter;
    float _cx;
    float _cy;

    glm::dvec3 _p0;
    glm::dquat _rot0;
};

/*!
* Tumbler: each mouse move turns the view about the screen axis perpendicular to the move,
* by an angle proportional to its length.  Horizontal motion turns about the screen's
* vertical axis and vertical motion about its horizontal axis.  Any orientation can be
* reached, but the result depends on the path: a drag that returns to where it started
* does not in general restore the view.
*/
class Tumbler : public RotationMethod
{
public:
    Tumbler();

    /*!
    * gain - radians per window pixel.
    */
    void setGain( double gain );

    virtual void begin( Camera * cam, int x, int y );
    virtual void drag( Camera * cam, int x, int y );

private:
    double _gain;

    int _px;
    int _py;
};

/*!
* Turntable: horizontal motion turns the model about a fixed world axis, vertical motion
* tips it about a horizontal screen axis.  Measured from the press, so the result depends
* only on the total motion.  When the up axis points down the screen at press, horizontal
* motion is reversed for that drag so the near side still follows the mouse.  If something
* else turns the view during a drag, the drag continues from there.
*/
class Turntable : public RotationMethod
{
public:
    Turntable();

    /*!
    * gain - radians per window pixel.
    * up - world axis to turn about.
    */
    void setParams( double gain, const glm::dvec3 & up );

    virtual void begin( Camera * cam, int x, int y );
    virtual void drag( Camera * cam, int x, int y );

private:
    double _gain;
    glm::dvec3 _up;

    int _x0;
    int _y0;
    double _reverse;
    glm::dvec3 _tipAxis;
    glm::dquat _rot0;
};
}
#endif
