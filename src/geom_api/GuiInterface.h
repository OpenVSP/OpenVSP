//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VehicleMgr.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(GUIINTERFACE__INCLUDED_)
#define GUIINTERFACE__INCLUDED_

class ScreenMgr;
class Vehicle;


//==== GUI Interface ====//
class GuiInterface
{
private:
    GuiInterface();
    ~GuiInterface();
    GuiInterface( GuiInterface const& copy );          // Not Implemented
    GuiInterface& operator=( GuiInterface const& copy ); // Not Implemented

    ScreenMgr* m_ScreenMgr;
    Vehicle* m_Vehicle;
public:
    static GuiInterface& getInstance()
    {
        static GuiInterface instance;
        return instance;
    }
    void InitGui( Vehicle* vPtr );
    void StartGui( );
    void InitGuiAPI( Vehicle* vPtr );
    void StartGuiAPI( );
    void UpdateGui( );
    void PopupMsg( const char * message, bool lock_out );
};

#endif // !defined(GUIINTERFACE__INCLUDED_)
