//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneNameSuffixScreen.h: interface for asking what to call a new set of Clones.
//
//////////////////////////////////////////////////////////////////////

#ifndef CLONENAMESUFFIXSCREEN_H
#define CLONENAMESUFFIXSCREEN_H

#include "ScreenMgr.h"
#include "ScreenBase.h"
#include "Vehicle.h"
#include "GuiDevice.h"

using namespace std;

class CloneNameSuffixScreen : public BasicScreen
{
public:
    CloneNameSuffixScreen( ScreenMgr* mgr );
    virtual ~CloneNameSuffixScreen();

    void Show();
    void Hide();
    bool Update();

    // Captures the Geoms to clone now; the window is not modal, so the selection may change
    // before OK is pressed.
    void SetupAndShow( const vector < string > &geom_id_vec );

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( CloneNameSuffixScreen* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:

    vector < string > m_GeomIDVec;

    GroupLayout m_GenLayout;
    GroupLayout m_BorderLayout;

    StringInput m_NameSuffixInput;

    TriggerButton m_OK;
    TriggerButton m_Cancel;
};

#endif  // CLONENAMESUFFIXSCREEN_H
