#include "ManageLightingScreen.h"
#include "ScreenMgr.h"

ManageLightingScreen::ManageLightingScreen( ScreenMgr * mgr ) : BasicScreen( mgr, 250, 325, "Manage Lighting" )
{
    m_GenLayout.SetGroupAndScreen( m_FLTK_Window, this );
    m_GenLayout.AddY( 25 );

    m_GenLayout.AddDividerBox( "Activate Lights" );

    m_GenLayout.SetButtonWidth( m_GenLayout.GetW() / 2 );

    m_GenLayout.SetFitWidthFlag( false );
    m_GenLayout.SetSameLineFlag( true );

    m_GenLayout.AddButton( m_LightButton0, "Light 0" );
    m_GenLayout.AddButton( m_LightButton1, "Light 1" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.AddButton( m_LightButton2, "Light 2" );
    m_GenLayout.AddButton( m_LightButton3, "Light 3" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.AddButton( m_LightButton4, "Light 4" );
    m_GenLayout.AddButton( m_LightButton5, "Light 5" );
    m_GenLayout.ForceNewLine();
    m_GenLayout.AddButton( m_LightButton6, "Light 6" );
    m_GenLayout.AddButton( m_LightButton7, "Light 7" );
    m_GenLayout.ForceNewLine();

    m_GenLayout.InitWidthHeightVals();

    m_GenLayout.SetFitWidthFlag( true );
    m_GenLayout.SetSameLineFlag( false );

    m_GenLayout.AddYGap();

    m_GenLayout.AddDividerBox( "Edit Lights" );

    char name[256];
    for( int i = 0; i < NUMOFLIGHTS; i++ )
    {
        sprintf( name, "Light %d", i );
        m_LightChoice.AddItem( name );
    }
    m_GenLayout.AddChoice( m_LightChoice, "Light:" );

    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Location" );
    m_GenLayout.AddSlider( m_XPosSlider, "X Loc", 100, "%4.2f" );
    m_GenLayout.AddSlider( m_YPosSlider, "Y Loc", 100, "%4.2f" );
    m_GenLayout.AddSlider( m_ZPosSlider, "Z Loc", 100, "%4.2f" );

    m_GenLayout.AddYGap();
    m_GenLayout.AddDividerBox( "Light Quality" );
    m_GenLayout.AddSlider( m_AmbSlider, "Ambient", 1.0, "%3.2f" );
    m_GenLayout.AddSlider( m_DiffSlider, "Diffuse", 1.0, "%3.2f" );
    m_GenLayout.AddSlider( m_SpecSlider, "Specular", 1.0, "%3.2f" );

    m_CurrentSelected = 0;
}

ManageLightingScreen::~ManageLightingScreen()
{
}

void ManageLightingScreen::Show()
{
    BasicScreen::Show();
    m_ScreenMgr->SetUpdateFlag( true );

}

bool ManageLightingScreen::Update()
{
    m_LightChoice.SetVal( m_CurrentSelected );

    Light * currLight = LightMgr.Get( m_CurrentSelected );

    if ( currLight )
    {
        m_XPosSlider.Update( currLight->m_X.GetID() );
        m_YPosSlider.Update( currLight->m_Y.GetID() );
        m_ZPosSlider.Update( currLight->m_Z.GetID() );

        m_AmbSlider.Update( currLight->m_Amb.GetID() );
        m_DiffSlider.Update( currLight->m_Diff.GetID() );
        m_SpecSlider.Update( currLight->m_Spec.GetID() );
    }

    m_LightButton0.Update( LightMgr.Get(0)->m_Active.GetID() );
    m_LightButton1.Update( LightMgr.Get(1)->m_Active.GetID() );
    m_LightButton2.Update( LightMgr.Get(2)->m_Active.GetID() );
    m_LightButton3.Update( LightMgr.Get(3)->m_Active.GetID() );
    m_LightButton4.Update( LightMgr.Get(4)->m_Active.GetID() );
    m_LightButton5.Update( LightMgr.Get(5)->m_Active.GetID() );
    m_LightButton6.Update( LightMgr.Get(6)->m_Active.GetID() );
    m_LightButton7.Update( LightMgr.Get(7)->m_Active.GetID() );

    return true;
}

void ManageLightingScreen::GuiDeviceCallBack( GuiDevice* device )
{
    assert( m_ScreenMgr );

    if ( device == &m_LightChoice )
    {
        m_CurrentSelected = m_LightChoice.GetVal();
    }
    m_ScreenMgr->SetUpdateFlag( true );
}

void ManageLightingScreen::LoadDrawObjs(vector< DrawObj* > & draw_obj_vec)
{
    // Create DrawObj that provides Global Lighting Setting.
    m_LightingDO.m_Type = DrawObj::VSP_SETTING;
    m_LightingDO.m_Screen = DrawObj::VSP_MAIN_SCREEN;

    // Clear Light info, redo list on every GetDrawObjs().
    m_LightingDO.m_LightingInfos.clear();

    std::vector< Light* > lightList = LightMgr.GetVec();

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
