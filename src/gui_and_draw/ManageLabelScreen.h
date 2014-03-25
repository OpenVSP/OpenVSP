#ifndef _VSP_GUI_LABEL_MANAGER_SCREEN_H
#define _VSP_GUI_LABEL_MANAGER_SCREEN_H

#include "labelFlScreen.h"
#include "ScreenBase.h"
#include "DrawObj.h"
#include "GuiDevice.h"

class ScreenMgr;
class ManageLabelScreen : public VspScreen
{
public:
    ManageLabelScreen(ScreenMgr * mgr);
    virtual ~ManageLabelScreen();

public:
    virtual void Show();
    virtual void Hide();
    virtual bool Update();

    void CallBack(Fl_Widget * w);
    static void staticCB(Fl_Widget * w, void * data) { static_cast<ManageLabelScreen *>(data)->CallBack(w); }

public:
    void LoadDrawObjs(vector< DrawObj* > & draw_obj_vec);

public:
    void Set(std::string sourceId, unsigned int sourceIndex);
    void Set(vec3d placement);

    virtual std::string getFeedbackGroupName();

protected:
    std::string GenerateRuler();
    void RemoveRuler(std::string geomId);

protected:
    LabelUI * m_LabelUI;

    Slider m_RedSlider;
    Slider m_GreenSlider;
    Slider m_BlueSlider;

    Slider m_SizeSlider;

private:
    std::string GenerateName();

    DrawObj * Find(std::string geomID);

    void UpdateDrawObjs();
    void UpdatePickList();
    void UpdateNameInput();

private:
    std::vector<DrawObj> m_LabelList;
    std::vector<DrawObj> m_PickList;

    std::vector<std::string> m_Current;
};
#endif