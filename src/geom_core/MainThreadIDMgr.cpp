#include "MainThreadIDMgr.h"


MainThreadIDMgrSingleton::MainThreadIDMgrSingleton()
{
    m_MainThreadID = std::this_thread::get_id();
}

bool MainThreadIDMgrSingleton::IsCurrentThreadMain() const
{
    return m_MainThreadID == std::this_thread::get_id();
}
