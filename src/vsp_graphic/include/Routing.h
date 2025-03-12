#ifndef _VSP_GRAPHIC_LABEL_ROUTING_H
#define _VSP_GRAPHIC_LABEL_ROUTING_H

#include <vector>

#include "Marker.h"

#include "glm/glm.hpp"

namespace VSPGraphic
{

/*!
* Customized marker to render rulers.
*/
class Routing : public Marker
{
public:
    /*!
    * Constructor.
    */
    Routing();
    /*!
    * Destructor.
    */
    virtual ~Routing();

public:
    /*!
    * Update current mouse location.  The mouse location is a glm::vec3
    * and must transformed to world space.
    */
    static void updateMouseLocation( const glm::vec3 &mouseLocInWorld );

public:
    /*!
    * Reset values of ruler.
    */
    void reset();


    void setLiveIndex( int li );

    std::vector <glm::vec3> _v;

protected:
    virtual void _draw();

private:
    int _liveIndex;


};
}
#endif
