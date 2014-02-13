#include "ManageLightingScreen.h"
#include "ScreenMgr.h"

ManageLightingScreen::ManageLightingScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_LightingUI = new LightingUI();
    m_FLTK_Window = m_LightingUI->UIWindow;

    m_XPosSlider.Init( this, m_LightingUI->xSlider, m_LightingUI->xInput, 2.0e12, "%4.2f" );
    m_YPosSlider.Init( this, m_LightingUI->ySlider, m_LightingUI->yInput, 2.0e12, "%4.2f" );
    m_ZPosSlider.Init( this, m_LightingUI->zSlider, m_LightingUI->zInput, 2.0e12, "%4.2f" );

    m_AmbSlider.Init( this, m_LightingUI->ambSlider, 1 );
    m_DiffSlider.Init( this, m_LightingUI->diffSlider, 1 );
    m_SpecSlider.Init( this, m_LightingUI->specSlider, 1 );

    m_LightingUI->UIWindow->position( 775, 50 );

    m_LightingUI->LightSourceDropDown->callback( staticCB, this );
    m_LightingUI->light0Button->callback( staticCB, this );
    m_LightingUI->light1Button->callback( staticCB, this );
    m_LightingUI->light2Button->callback( staticCB, this );
    m_LightingUI->light3Button->callback( staticCB, this );
    m_LightingUI->light4Button->callback( staticCB, this );
    m_LightingUI->light5Button->callback( staticCB, this );
    m_LightingUI->light6Button->callback( staticCB, this );
    m_LightingUI->light7Button->callback( staticCB, this );

    m_CurrentSelected = -1;
}

ManageLightingScreen::~ManageLightingScreen()
{
    delete m_LightingUI;
}

void ManageLightingScreen::Show()
{
    if( Update() )
    {
        m_FLTK_Window->show();
    }
}

void ManageLightingScreen::Hide()
{
    m_FLTK_Window->hide();
}

bool ManageLightingScreen::Update()
{
    char name[256];

    m_LightingUI->LightSourceDropDown->clear();

    // Only support up to eight light sources.
    for( int i = 0; i < 8; i++ )
    {
        sprintf( name, "Light %d", i );
        m_LightingUI->LightSourceDropDown->add( name );
    }
    if( m_CurrentSelected < 0 )
    {
        m_LightingUI->LightSourceDropDown->value( 0 );
        m_CurrentSelected = 0;
    }
    else
    {
        m_LightingUI->LightSourceDropDown->value( m_CurrentSelected );
    }

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    Vehicle::LightInfo * lightPtr = veh->FindLight( m_CurrentSelected );

    assert( lightPtr );

    m_XPosSlider.Update( lightPtr->XPos.GetID() );
    m_YPosSlider.Update( lightPtr->YPos.GetID() );
    m_ZPosSlider.Update( lightPtr->ZPos.GetID() );

    m_AmbSlider.Update( lightPtr->Amb.GetID() );
    m_DiffSlider.Update( lightPtr->Diff.GetID() );
    m_SpecSlider.Update( lightPtr->Spec.GetID() );

    m_LightingUI->light0Button->value( veh->FindLight( 0 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light1Button->value( veh->FindLight( 1 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light2Button->value( veh->FindLight( 2 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light3Button->value( veh->FindLight( 3 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light4Button->value( veh->FindLight( 4 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light5Button->value( veh->FindLight( 5 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light6Button->value( veh->FindLight( 6 )->Active.Get() == false ? 0 : 1 );
    m_LightingUI->light7Button->value( veh->FindLight( 7 )->Active.Get() == false ? 0 : 1 );

    return true;
}

void ManageLightingScreen::CallBack( Fl_Widget * w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if( w == m_LightingUI->LightSourceDropDown )
    {
        m_CurrentSelected = m_LightingUI->LightSourceDropDown->value();
    }
    else if( w == m_LightingUI->light0Button )
    {
        veh->FindLight( 0 )->Active.Set( m_LightingUI->light0Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light1Button )
    {
        veh->FindLight( 1 )->Active.Set( m_LightingUI->light1Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light2Button )
    {
        veh->FindLight( 2 )->Active.Set( m_LightingUI->light2Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light3Button )
    {
        veh->FindLight( 3 )->Active.Set( m_LightingUI->light3Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light4Button )
    {
        veh->FindLight( 4 )->Active.Set( m_LightingUI->light4Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light5Button )
    {
        veh->FindLight( 5 )->Active.Set( m_LightingUI->light5Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light6Button )
    {
        veh->FindLight( 6 )->Active.Set( m_LightingUI->light6Button->value() == 0 ? false : true );
    }
    else if( w == m_LightingUI->light7Button )
    {
        veh->FindLight( 7 )->Active.Set( m_LightingUI->light7Button->value() == 0 ? false : true );
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
