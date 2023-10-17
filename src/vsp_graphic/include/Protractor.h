#ifndef _VSP_GRAPHIC_LABEL_PROTRACTOR_H
#define _VSP_GRAPHIC_LABEL_PROTRACTOR_H

#include "Marker.h"

#include "glm/glm.hpp"
#include <string>

namespace VSPGraphic
{
class TextMgr;

/*!
* Customized marker to render protractors.
*/
class Protractor : public Marker
{
public:
    /*!
    * Constructor.
    */
    Protractor();
    /*!
    * Destructor.
    */
    virtual ~Protractor();

public:
    /*!
    * Update current mouse location.  The mouse location is a glm::vec3
    * and must transformed to world space.
    */
    static void updateMouseLocation( const glm::vec3 &mouseLocInWorld);

    /*!
    * Place protractor at initial point. The protractor stretch from initial point
    * to mouse location.
    * v1 - initial or starting point of protractor.
    */
    void placeProtractor( const glm::vec3 &v1 );
    /*!
    * Place protractor at initial point. The protractor stretch from initial point
    * to mouse location.
    * v1 - initial or starting point of protractor.
    * v2 - mid or corner point of protractor.
    */
    void placeProtractor( const glm::vec3 &v1, const glm::vec3 &v2 );
    /*!
    * Place protractor in between initial point and finish point.  The offset
    * of protractor is at mouse location.
    * v1 - initial or start point of protractor.
    * v2 - mid or corner point of protractor.
    * v3 - finish or end point of protractor.
    */
    void placeProtractor( const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, const glm::vec3 &label_dir, const std::string & lbl );
    /*!
    * Place protractor in between initial point and finish point.  The protractor's
    * height is set to offset.
    * v1 - initial or start point of protractor.
    * v2 - mid or corner point of protractor.
    * v3 - finish or end point of protractor.
    * offset - placement of the protractor.
    */
    void placeProtractor( const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, const glm::vec3 &label_dir, const float & offset, const std::string & lbl, int dir );

public:
    /*!
    * Reset values of protractor.
    */
    void reset();

protected:
    virtual void _draw();

private:
    std::string _label;

    glm::vec3 _v1;
    glm::vec3 _v2;
    glm::vec3 _v3;

    glm::vec3 _label_dir;
    float _offset;

    int _dir;
};
}
#endif
