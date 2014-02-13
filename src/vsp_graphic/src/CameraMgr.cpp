#include "CameraMgr.h"

#include "ArcballCam.h"
#include "Common.h"

namespace VSPGraphic
{
CameraMgr::CameraMgr()
{
    // Only ArcballCam is available right now, add
    // more Camera Type if needed.
    _cameraList.push_back( new ArcballCam() );
    _current = 0;
}
CameraMgr::~CameraMgr()
{
    for( int i = 0; i < ( int )_cameraList.size(); i++ )
    {
        delete _cameraList[i];
    }
    _cameraList.clear();
}

void CameraMgr::changeCameraStyle( Common::VSPenum style )
{
    switch( style )
    {
    case Common::VSP_CAM_TYPE_ARCBALL:
        _current = 0;
        break;

    default:
        // Unknown style, ignore changes.
        assert( false );
    }
}

Camera * CameraMgr::getCamera()
{
    return _cameraList[_current];
}
}