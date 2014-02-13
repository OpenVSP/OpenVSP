//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#ifndef AwaveScreen_H_
#define AwaveScreen_H_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include "awaveFlScreen.h"

#include "Vec3d.h"

using std::string;
using std::vector;

class AwaveScreen : public VspScreen
{
public:

    AwaveScreen( ScreenMgr* mgr );
    virtual ~AwaveScreen();
    void Show();
    void Hide();
    bool Update();
    void LoadSetChoice();
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( AwaveScreen* )data )->CallBack( w );
    }
    void CallBack( Fl_Widget *w );


protected:

    AwaveUI* m_AwaveUI;
    int m_SelectedSetIndex;
    double m_StartVal;
    double m_EndVal;
    double m_BoundsRange[2];
    int m_SliceRange[2];
    int m_numSlices;
    int m_lastAxis;
    int m_NumRotSecs;
    int m_NumRotSecsRange[2];
    double m_Angle;
    double m_MNumber;
    double m_AngleRange[2];
    double m_MNumberRange[2];
    bool m_ComputeAngle;
    Fl_Text_Buffer* m_textBuffer;
    vec3d m_Norm;

};

#endif /* AwaveScreen_H_ */
