#if !defined(PTCLOUDSCREEN__INCLUDED_)
#define PTCLOUDSCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class PtCloudScreen : public GeomScreen
{
public:
    PtCloudScreen( ScreenMgr* mgr );
    virtual ~PtCloudScreen()                          {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );
    virtual void GuiDeviceCallBack( GuiDevice* device );

protected:
    GroupLayout m_ProjectLayout;
    TriggerButton m_ProjectButton;

    Choice m_DirChoice;
    GeomPicker m_GeomPicker;
    Choice m_SurfChoice;

};


#endif // !defined(PTCLOUDSCREEN__INCLUDED_)
