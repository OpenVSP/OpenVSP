//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef PSLICESCREEN_H_
#define PSLICESCREEN_H_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "pSliceFlScreen.h"

#include "Vec3d.h"

using std::string;
using std::vector;

class PSliceScreen : public VspScreen
{
public:

    PSliceScreen( ScreenMgr* mgr );
    virtual ~PSliceScreen();
    void Show();
    void Hide();
    bool Update();
    void LoadSetChoice();
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( PSliceScreen* )data )->CallBack( w );
    }
    void CallBack( Fl_Widget *w );


protected:

    PSliceUI* m_PSliceUI;
    int m_SelectedSetIndex;
    double m_StartVal;
    double m_EndVal;
    double m_BoundsRange[2];
    int m_SliceRange[2];
    int m_numSlices;
    int m_lastAxis;
    Fl_Text_Buffer* m_textBuffer;
    vec3d m_Norm;

};

#endif /* PSLICESCREEN_H_ */
