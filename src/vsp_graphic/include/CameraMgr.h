#ifndef _VSP_GRAPHIC_CAMERA_H
#define _VSP_GRAPHIC_CAMERA_H

#include <vector>

#include "Common.h"
#include "glm/glm.hpp"

namespace VSPGraphic
{
class Camera;

/*!
* CameraMgr Class.
* This class manages multiple versions of camera type.
*/
class CameraMgr
{
public:
    /*!
    * Constructor.
    */
    CameraMgr();
    /*!
    * Destructor.
    */
    virtual ~CameraMgr();

public:
    /*!
    * Set Camera type.
    * Style(s): VSP_CAM_TYPE_ARCBALL
    * VSP_CAM_TYPE_ARCBALL by default.
    */
    void changeCameraStyle( Common::VSPenum type );
    /*!
    * Get Camera Object base on the current camera style.
    */
    Camera * getCamera();

private:
    std::vector<Camera *> _cameraList;
    int _current;
};
}
#endif