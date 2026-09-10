//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CloneScreen.h"
#include "ScreenMgr.h"
#include "CloneGeom.h"


//==== Constructor ====//
CloneScreen::CloneScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Clone" )
{
    Fl_Group* clone_tab = AddTab( "Clone" );
    Fl_Group* clone_group = AddSubGroup( clone_tab, 5 );

    m_CloneLayout.SetGroupAndScreen( clone_group, this );

    m_CloneLayout.AddDividerBox( "Original" );
    m_CloneLayout.AddChoice( m_OriginalChoice, "Original" );
    m_CloneLayout.AddYGap();

}


//==== Show Clone Screen ====//
void CloneScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Clone Screen ====//
bool CloneScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CLONE_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    assert( clone_ptr );

    // A Clone takes its size from the original.
    m_ScaleSlider.Deactivate();
    m_ScaleResetButton.Deactivate();
    m_ScaleAcceptButton.Deactivate();

    m_OriginalChoice.ClearItems();
    m_CompVec.clear();
    map <string, int> CompIDMap;
    int icomp = 0;

    Vehicle* veh = VehicleMgr.GetVehicle();

    if ( veh )
    {
        vector <string> geomVec = veh->GetGeomVec();

        for ( int i = 0; i < (int)geomVec.size(); i++ )
        {
            char str[256];
            Geom* g = veh->FindGeom( geomVec[i] );
            if ( g )
            {
                // A Clone of itself would have nothing to copy.
                if ( g->GetType().m_Type != HINGE_GEOM_TYPE && geomVec[i] != clone_ptr->GetID() )
                {
                    snprintf( str, sizeof( str ), "%d_%s", i, g->GetName().c_str() );
                    m_OriginalChoice.AddItem( str );
                    CompIDMap[ geomVec[i] ] = icomp;
                    m_CompVec.push_back( geomVec[i] );
                    icomp++;
                }
            }
        }
        m_OriginalChoice.UpdateItems();

        // find(), since [] would insert a missing ID and return 0.
        map <string, int>::iterator iorig = CompIDMap.find( clone_ptr->GetOriginalID() );
        if ( iorig != CompIDMap.end() )
        {
            m_OriginalChoice.SetVal( iorig->second );
        }
        else
        {
            m_OriginalChoice.SetVal( -1 );
        }
    }


    return true;
}


//==== Non Menu Callbacks ====//
void CloneScreen::CallBack( Fl_Widget *w )
{
    GeomScreen::CallBack( w );
}

void CloneScreen::GuiDeviceCallBack( GuiDevice *device )
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != CLONE_GEOM_TYPE )
    {
        return;
    }

    CloneGeom* clone_ptr = dynamic_cast< CloneGeom* >( geom_ptr );
    assert( clone_ptr );

    if ( device == &m_OriginalChoice )
    {
        int id = m_OriginalChoice.GetVal();
        if ( id >= 0 && id < ( int )m_CompVec.size() )
        {
            clone_ptr->SetOriginalID( m_CompVec[id] );
        }
    }

    GeomScreen::GuiDeviceCallBack( device );
}
