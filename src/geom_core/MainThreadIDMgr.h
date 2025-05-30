#if !defined(VSP_MAIN_THREAD_MANAGER__INCLUDED_)
#define VSP_MAIN_THREAD_MANAGER__INCLUDED_

#include <thread>

class MainThreadIDMgrSingleton
{
public:
    static MainThreadIDMgrSingleton & getInstance()
    {
        static MainThreadIDMgrSingleton instance;
        return instance;
    }

    bool IsCurrentThreadMain() const;

private:
    MainThreadIDMgrSingleton();
    MainThreadIDMgrSingleton( MainThreadIDMgrSingleton const& copy ) = delete;            // Not Implemented
    MainThreadIDMgrSingleton& operator=( MainThreadIDMgrSingleton const& copy ) = delete; // Not Implemented

    std::thread::id m_MainThreadID;
};

#define MainThreadIDMgr MainThreadIDMgrSingleton::getInstance()

#endif
