//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// parmLinkScreen.h: interface for the geomScreen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARMLINKCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_)
#define AFX_PARMLINKCREEN_H__015D1220_857A_11D7_AC31_0003473A025A__INCLUDED_


#include "GuiDevice.h"
#include "GroupLayout.h"
#include "ScreenBase.h"

#include <FL/Fl.H>
#include "parmLinkFlScreen.h"

#include <vector>
using std::vector;

class ParmLinkScreen  : public VspScreen
{
public:
    ParmLinkScreen( ScreenMgr* mgr );
    virtual ~ParmLinkScreen();

    void Show();
    void Hide();
    bool Update();


    //virtual void Hide();
    //virtual void CloseCB( Fl_Widget* w );
    //virtual void SetTitle( const char* name );
    //virtual void Parm_changed( Parm* parm )               {}
    //virtual void ClearButtonParms();

    //virtual void Show();
    //virtual void Show(Geom* geomPtr);

    //bool IsShown()                                { return !!parmLinkUI->UIWindow->shown(); }

    //void Position( int x, int y )             { parmLinkUI->UIWindow->position( x, y ); }

    void CallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ParmLinkScreen* )data )->CallBack( w );
    }


    virtual void CompGroupLinkChange();
    //virtual void Ipdate();

    //virtual void RegisterParmButton( ParmButton* b )  { m_ParmButtonVec.push_back( b ); }
    //virtual void RemoveAllRefs( GeomBase* g );

protected:

    ParmLinkUI* parmLinkUI;

    SliderInput m_OffsetSlider;
    SliderInput m_ScaleSlider;
    SliderInput m_LowerLimitSlider;
    SliderInput m_UpperLimitSlider;

    vector< string > FindParmNames( bool A_flag, vector< string > & parm_id_vec );

    //GroupLayout* m_User1Group;
    //GroupLayout* m_User2Group;
    //enum { NUM_USER_SLIDERS = 10, };
    //SliderAdjRangeInput m_UserSlider[NUM_USER_SLIDERS];

};

#endif
