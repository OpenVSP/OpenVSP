//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// ParasiteDragScreen.cpp: implementation of the ParasiteDragScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "ParasiteDragScreen.h"
#include "ParasiteDragMgr.h"
#include "ParmMgr.h"
#include "APIDefines.h"
#include "StringUtil.h"
#include "FileUtil.h"

#include <utility>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <numeric>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define DRAG_TAB_WIDTH 310          // width of the tab group
#define DRAG_TABLE_WIDTH 460        // width of the drag build-up table
#define DRAG_TABLE_PERSISTENT_WIDTH 195 // width of persistent sectino of drag table
#define TOTAL_WINDOW_HEIGHT 560     // Entire Window Height
#define EXECUTE_LAYOUT_HEIGHT 65    // height needed for dividerbox and two buttons

ParasiteDragScreen::ParasiteDragScreen(ScreenMgr* mgr) : TabScreen(mgr,
    DRAG_TAB_WIDTH + DRAG_TABLE_WIDTH + DRAG_TABLE_PERSISTENT_WIDTH + 10, TOTAL_WINDOW_HEIGHT,
    "Parasite Drag", EXECUTE_LAYOUT_HEIGHT + 5, DRAG_TABLE_WIDTH + DRAG_TABLE_PERSISTENT_WIDTH + 10)
{
}

ParasiteDragScreen::~ParasiteDragScreen()
{
}

void ParasiteDragScreen::Show()
{
}

void ParasiteDragScreen::Hide()
{
}

bool ParasiteDragScreen::Update()
{
    return false;
}

void ParasiteDragScreen::CallBack(Fl_Widget * w)
{
}

void ParasiteDragScreen::GuiDeviceCallBack(GuiDevice * device)
{
}
