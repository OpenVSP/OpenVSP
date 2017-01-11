#ifndef _VSP_GUI_COR_MANAGER_SCREEN_H
#define _VSP_GUI_COR_MANAGER_SCREEN_H

#include "ScreenBase.h"
#include "DrawObj.h"

/*!
* Center of Rotation Screen.  This is a hidden screen, in another 
* word, user can not interact with this screen.  It's used to 
* provide backend functionalities for change center of rotation.
*/
class ManageCORScreen : public VspScreen
{
public:
    ManageCORScreen( ScreenMgr * mgr );
    virtual ~ManageCORScreen();

public:
    virtual void Show();
    virtual bool IsShown();
    virtual void Hide();
    virtual bool Update();

    void CallBack(Fl_Widget * w);
    static void staticCB(Fl_Widget * w, void * data) { static_cast< ManageCORScreen* >(data)->CallBack(w); }

public:
    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

public:
    /*!
    * Enable selection.
    */
    void EnableSelection();
    /*!
    * Set value.
    */
    void Set();

public:
    virtual std::string getFeedbackGroupName();

private:
    std::vector< DrawObj > m_PickList;
    bool m_SelectionFlag;
};
#endif