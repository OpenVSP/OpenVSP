#if !defined(WIREFRAMESCREEN__INCLUDED_)
#define WIREFRAMESCREEN__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"

#include <FL/Fl.H>

class WireScreen : public GeomScreen
{
public:
    WireScreen( ScreenMgr* mgr );
    virtual ~WireScreen()                          {}

    virtual void Show();
    virtual bool Update();

    virtual void CallBack( Fl_Widget *w );

protected:

    GroupLayout m_WireLayout;

    Choice m_TypeChoice;
    CheckButton m_InvertButton;

    CheckButton m_SwapIJButton;
    CheckButton m_RevIButton;
    CheckButton m_RevJButton;

    IndexSelector m_IStrideIndexSelector;
    IndexSelector m_JStrideIndexSelector;

    IndexSelector m_ISkipStartIndexSelector;
    IndexSelector m_ISkipEndIndexSelector;
    IndexSelector m_JSkipStartIndexSelector;
    IndexSelector m_JSkipEndIndexSelector;

};


#endif // !defined(WIREFRAMESCREEN__INCLUDED_)
