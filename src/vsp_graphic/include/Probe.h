#ifndef _VSP_GRAPHIC_LABEL_PROBE_H
#define _VSP_GRAPHIC_LABEL_PROBE_H

#include "Marker.h"

#include "glm/glm.hpp"
#include <string>

namespace VSPGraphic
{
class TextMgr;

/*!
* Customized marker to render probe.
*/
class Probe : public Marker
{
public:
    /*!
    * Constructor.
    */
    Probe();
    /*!
    * Destructor.
    */
    virtual ~Probe();

public:
    /*!
    * Update current mouse location.  The mouse location is a glm::vec3
    * and must transformed to world space.
    */
    static void updateMouseLocation(glm::vec3 mouseLocInWorld);

    /*!
    * Place probe at initial point. The probe stretch from initial point
    * to mouse location.
    * v1 - initial or starting point of probe.
    * norm - normal vector at v1.
    */
    void placeProbe(glm::vec3 v1, glm::vec3 norm, const std::string & lbl );

    /*!
    * Place probe in between initial point and finish point.  The offset
    * of probe is at mouse location.
    * v1 - initial or start point of probe.
    * norm - normal vector at v1.
    * len - length of probe line
    */
    void placeProbe(glm::vec3 v1, glm::vec3 norm, float len, const std::string & lbl );

public:
    /*!
    * Reset values of probe.
    */
    void reset();

protected:
    virtual void _draw();

private:
    std::string _label;

    glm::vec3 _v1;

    glm::vec3 _norm;

    float _len;
};
}
#endif
