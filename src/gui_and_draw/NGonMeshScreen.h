#if !defined(NGonMeshScreen__INCLUDED_)
#define NGonMeshScreen__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class NGonMeshScreen : public GeomScreen
{
public:
    NGonMeshScreen( ScreenMgr* mgr );
    virtual ~NGonMeshScreen()                          {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

    GroupLayout m_OtherLayout;

    TriggerButton m_WriteVSPGEOMButton;

protected:

};


#endif // !defined(NGonMeshScreen__INCLUDED_)
