#ifndef _VSP_CLIPPING_H
#define _VSP_CLIPPING_H

#include <vector>

namespace VSPGraphic
{

/*!
* ClipPlane class.
* This class represent a single clipping plane.
*/
class ClipPlane
{
public:
    /*!
    * Constructor.
    */
    ClipPlane();
    /*!
    * Destructor.
    */
    virtual ~ClipPlane();

public:

    /*!
    * Enable clipping plane.
    */
    void enable();
    /*!
    * Disable clipping plane.
    */
    void disable();
    /*!
    * Get if this clipping plane is enabled.
    */
    bool isEnabled();

    void setEqn( double e[4] );

    void predraw();

    void postdraw();

    void seticlip( int indx );

private:

    double _eqn[4];

    bool _enabled;

    int _iclip;

};

/*!
* Clipping class.
* This class manages set of clipping planes.
*/
class Clipping
{
public:
    /*!
    * Constructor.
    */
    Clipping();
    /*!
    * Destructor.
    */
    virtual ~Clipping();

public:

    void predraw();

    void postdraw();

    ClipPlane * getPlane( int i );

private:

    std::vector< ClipPlane > _cplanes;

};
}
#endif
