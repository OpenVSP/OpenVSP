#ifndef _VSP_GRAPHIC_LABEL_RULER_H
#define _VSP_GRAPHIC_LABEL_RULER_H

#include "Marker.h"

#include "glm/glm.hpp"
#include <string>

namespace VSPGraphic
{
class TextMgr;

/*!
* Customized marker to render rulers.
*/
class Ruler : public Marker
{
public:
    /*!
    * Constructor.
    */
    Ruler();
    /*!
    * Destructor.
    */
    virtual ~Ruler();

public:
    /*!
    * Update current mouse location.  The mouse location is a glm::vec3
    * and must transformed to world space.
    */
    static void updateMouseLocation( const glm::vec3 &mouseLocInWorld);

    /*!
    * Place ruler at initial point. The ruler stretch from initial point
    * to mouse location.
    * v1 - initial or starting point of ruler.
    */
    void placeRuler( const glm::vec3 &v1 );
    /*!
    * Place ruler in between initial point and finish point.  The offset
    * of ruler is at mouse location.
    * v1 - initial or start point of ruler.
    * v2 - finish or end point of ruler.
    */
    void placeRuler( const glm::vec3 &v1, const glm::vec3 &v2, const std::string & lbl );
    /*!
    * Place ruler in between initial point and finish point.  The ruler's
    * height is set to offset.
    * v1 - initial or start point of ruler.
    * v2 - finish or end point of ruler.
    * offset - placement of the ruler.
    */
    void placeRuler( const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &offset, const std::string & lbl, int dir );

public:
    /*!
    * Reset values of ruler.
    */
    void reset();

protected:
    virtual void _draw();

private:
    std::string _label;

    glm::vec3 _v1;
    glm::vec3 _v2;

    glm::vec3 _offset;

    int _dir;
};
}
#endif
