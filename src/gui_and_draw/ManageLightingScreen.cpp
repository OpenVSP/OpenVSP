#include "ManageLightingScreen.h"
#include "ScreenMgr.h"
#include "VehicleGuiDraw.h"
#include "Lights.h"

#include <assert.h>

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

    m_LightButton0.Init( this, m_LightingUI->light0Button );
    m_LightButton1.Init( this, m_LightingUI->light1Button );
    m_LightButton2.Init( this, m_LightingUI->light2Button );
    m_LightButton3.Init( this, m_LightingUI->light3Button );
    m_LightButton4.Init( this, m_LightingUI->light4Button );
    m_LightButton5.Init( this, m_LightingUI->light5Button );
    m_LightButton6.Init( this, m_LightingUI->light6Button );
    m_LightButton7.Init( this, m_LightingUI->light7Button );

    m_LightingUI->UIWindow->position( 775, 50 );

    m_LightingUI->LightSourceDropDown->callback( staticCB, this );
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

    for( int i = 0; i < NUMOFLIGHTS; i++ )
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
    Lights * lights = veh->getVehicleGuiDraw()->getLights();
    Light * currLight = lights->Get( m_CurrentSelected );

    assert( currLight );

    m_XPosSlider.Update( currLight->m_X.GetID() );
    m_YPosSlider.Update( currLight->m_Y.GetID() );
    m_ZPosSlider.Update( currLight->m_Z.GetID() );

    m_AmbSlider.Update( currLight->m_Amb.GetID() );
    m_DiffSlider.Update( currLight->m_Diff.GetID() );
    m_SpecSlider.Update( currLight->m_Spec.GetID() );

    assert(lights->Get(0));
    assert(lights->Get(1));
    assert(lights->Get(2));
    assert(lights->Get(3));
    assert(lights->Get(4));
    assert(lights->Get(5));
    assert(lights->Get(6));
    assert(lights->Get(7));

    m_LightButton0.Update( lights->Get(0)->m_Active.GetID() );   
    m_LightButton1.Update( lights->Get(1)->m_Active.GetID() );
    m_LightButton2.Update( lights->Get(2)->m_Active.GetID() );
    m_LightButton3.Update( lights->Get(3)->m_Active.GetID() );
    m_LightButton4.Update( lights->Get(4)->m_Active.GetID() );
    m_LightButton5.Update( lights->Get(5)->m_Active.GetID() );
    m_LightButton6.Update( lights->Get(6)->m_Active.GetID() );
    m_LightButton7.Update( lights->Get(7)->m_Active.GetID() );

    return true;
}

void ManageLightingScreen::CallBack( Fl_Widget * w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    Lights * lights = veh->getVehicleGuiDraw()->getLights();

    if( w == m_LightingUI->LightSourceDropDown )
    {
        m_CurrentSelected = m_LightingUI->LightSourceDropDown->value();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageLightingScreen::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    Lights * lights = veh->getVehicleGuiDraw()->getLights();

    // Create DrawObj that provides Global Lighting Setting.
    m_LightingDO.m_Type = DrawObj::VSP_SETTING;
    m_LightingDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;

    // Clear Light info, redo list on every GetDrawObjs().
    m_LightingDO.m_LightingInfos.clear();

    std::vector< Light* > lightList = lights->GetVec();

    // Load Lighting Info.
    for ( int i = 0; i < ( int )lightList.size() ; i++ )
    {
        DrawObj::LightSourceInfo info;

        info.Active = lightList[i]->m_Active.Get();
        info.X = ( float )lightList[i]->m_X.Get();
        info.Y = ( float )lightList[i]->m_Y.Get();
        info.Z = ( float )lightList[i]->m_Z.Get();
        info.Amb = ( float )lightList[i]->m_Amb.Get();
        info.Diff = ( float )lightList[i]->m_Diff.Get();
        info.Spec = ( float )lightList[i]->m_Spec.Get();

        m_LightingDO.m_LightingInfos.push_back( info );
    }
    draw_obj_vec.push_back( &m_LightingDO );
}
