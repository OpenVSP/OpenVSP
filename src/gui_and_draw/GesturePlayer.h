//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// GesturePlayer.h: replays scripted mouse and key events into the main GL window.
// For development and regression testing only.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_GUI_GESTURE_PLAYER_H
#define VSP_GUI_GESTURE_PLAYER_H

class MainVSPScreen;

// When the environment variable VSP_GESTURE_SCRIPT names a script, play it into the main
// window, write the camera and view Parms after each event to the file named by
// VSP_GESTURE_OUT, then exit.  Does nothing otherwise.
void StartGesturePlayer( MainVSPScreen * screen );

#endif
