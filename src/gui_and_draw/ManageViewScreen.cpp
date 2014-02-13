#include "ManageViewScreen.h"
#include "ScreenMgr.h"

#include "MainVSPScreen.h"
#include "MainGLWindow.h"

ManageViewScreen::ManageViewScreen( ScreenMgr * mgr ) : VspScreen( mgr )
{
    m_ViewUI = new ViewUI();
    m_FLTK_Window = m_ViewUI->UIWindow;

    m_ViewUI->UIWindow->position( 775, 50 );

    m_ViewUI->xLocButton->callback( staticCB, this );
    m_ViewUI->xPosMinus1->callback( staticCB, this );
    m_ViewUI->xPosMinus2->callback( staticCB, this );
    m_ViewUI->xPosPlus1->callback( staticCB, this );
    m_ViewUI->xPosPlus2->callback( staticCB, this );

    m_ViewUI->yLocButton->callback( staticCB, this );
    m_ViewUI->yPosMinus1->callback( staticCB, this );
    m_ViewUI->yPosMinus2->callback( staticCB, this );
    m_ViewUI->yPosPlus1->callback( staticCB, this );
    m_ViewUI->yPosPlus2->callback( staticCB, this );

    m_ViewUI->zoomButton->callback( staticCB, this );
    m_ViewUI->zoomMinus1->callback( staticCB, this );
    m_ViewUI->zoomMinus2->callback( staticCB, this );
    m_ViewUI->zoomPlus1->callback( staticCB, this );
    m_ViewUI->zoomPlus2->callback( staticCB, this );
}

ManageViewScreen::~ManageViewScreen()
{
    delete m_ViewUI;
}

void ManageViewScreen::Show()
{
    if( Update() )
    {
        m_FLTK_Window->show();
    }
}

void ManageViewScreen::Hide()
{
    m_FLTK_Window->hide();
}

bool ManageViewScreen::Update()
{
    return true;
}

void ManageViewScreen::CallBack( Fl_Widget * w )
{
    MainVSPScreen * main =
        dynamic_cast<MainVSPScreen*>( m_ScreenMgr->GetScreen( m_ScreenMgr->VSP_MAIN_SCREEN ) );

    if( !main )
    {
        return;
    }

    VSPGUI::VspGlWindow * glwin = main->GetGLWindow();

    if( w == m_ViewUI->xPosMinus1 )
    {
        glwin->pan( -1, 0, true );
    }
    else if( w == m_ViewUI->xPosMinus2 )
    {
        glwin->pan( -1, 0, false );
    }
    else if( w == m_ViewUI->xPosPlus1 )
    {
        glwin->pan( 1, 0, true );
    }
    else if( w == m_ViewUI->xPosPlus2 )
    {
        glwin->pan( 1, 0, false );
    }
    else if( w == m_ViewUI->yPosMinus1 )
    {
        glwin->pan( 0, -1, true );
    }
    else if( w == m_ViewUI->yPosMinus2 )
    {
        glwin->pan( 0, -1, false );
    }
    else if( w == m_ViewUI->yPosPlus1 )
    {
        glwin->pan( 0, 1, true );
    }
    else if( w == m_ViewUI->yPosPlus2 )
    {
        glwin->pan( 0, 1, false );
    }
    else if( w == m_ViewUI->zoomMinus1 )
    {
        glwin->zoom( 1, true );
    }
    else if( w == m_ViewUI->zoomMinus2 )
    {
        glwin->zoom( 1, false );
    }
    else if( w == m_ViewUI->zoomPlus1 )
    {
        glwin->zoom( -1, true );
    }
    else if( w == m_ViewUI->zoomPlus2 )
    {
        glwin->zoom( -1, false );
    }
    m_ScreenMgr->SetUpdateFlag( true );
}