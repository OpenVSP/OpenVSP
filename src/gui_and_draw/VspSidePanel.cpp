//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspSidePanel.cpp: the Geom Browser's place at the right of the 3D view, over it or beside it, with a tab that brings
// it back
//
//////////////////////////////////////////////////////////////////////

#include "VspSidePanel.h"
#include "ScreenBase.h"

#include <FL/Fl.H>

//==== Side Panel ====//
VspSidePanel::VspSidePanel( VspCaptiveLayer* layer ) : Fl_Double_Window( layer->x() + layer->w() - 100, layer->y(), 100, layer->h() )
{
    m_Layer = layer;
    m_Shown = -1;
    m_LastShown = -1;
    m_InPlace = false;
    m_Docked = false;

    box( FL_UP_BOX );
    end();
    // The window held is placed here, not by FLTK's resizing
    resizable( nullptr );

    // Below the frames, which are added after it; hidden until a screen is shown in it
    layer->insert( *this, 0 );
    hide();

    m_Tab = new VspSideTab( this );
    layer->insert( *m_Tab, 1 );
    m_Tab->hide();

    Place();
}

VspSidePanel::~VspSidePanel()
{
    for ( int i = 0; i < ( int )m_Screens.size(); i++ )
    {
        Fl_Window* body = m_Screens[i]->GetFlWindow();
        if ( body && body->parent() == this )
        {
            remove( body );
        }
    }
}

int VspSidePanel::IndexOf( VspScreen* screen )
{
    for ( int i = 0; i < ( int )m_Screens.size(); i++ )
    {
        if ( m_Screens[i] == screen )
        {
            return i;
        }
    }
    return -1;
}

void VspSidePanel::TakeScreen( VspScreen* screen )
{
    if ( !screen || !screen->GetFlWindow() || IndexOf( screen ) >= 0 )
    {
        return;
    }
    Fl_Window* body = screen->GetFlWindow();

    m_BodyBorders.push_back( body->border() != 0 );
    m_BodyHeights.push_back( body->h() );
    m_Screens.push_back( screen );

    // A close button at the right of the screen's title, as the panel is closed as a window is
    Fl_Group* cur = Fl_Group::current();
    Fl_Group::current( nullptr );
    Fl_Button* close = new Fl_Button( body->w() - CLOSE_W - 2, 2, CLOSE_W, CLOSE_W, "@1+" );
    Fl_Group::current( cur );
    close->box( FL_THIN_UP_BOX );
    close->labelsize( 10 );
    close->labelcolor( FL_DARK3 );
    close->visible_focus( 0 );
    close->tooltip( "Hide the Geom Browser" );
    close->callback( CloseCB, this );
    body->add( close );
    m_CloseButtons.push_back( close );
    screen->SetTitleReserve( CLOSE_W + 4 );

    body->hide();
    body->clear_modal_states();
    body->position( EDGE, EDGE );
    add( body );
    body->hide();
}

void VspSidePanel::ReleaseScreen( VspScreen* screen )
{
    int i = IndexOf( screen );
    if ( i < 0 )
    {
        return;
    }

    // Placed on the screen where it was shown in the panel, at its own height
    Fl_Window* body = screen->GetFlWindow();
    Fl_Window* top = window();
    int rx = body->x();
    int ry = body->y();
    if ( top )
    {
        rx += top->x() + x();
        ry += top->y() + y();
    }

    HideScreen( screen );
    remove( body );
    body->remove( m_CloseButtons[i] );
    Fl::delete_widget( m_CloseButtons[i] );
    screen->SetTitleReserve( 0 );
    // Hiding leaves the window marked visible, which FLTK takes a window of its own to be only once shown
    body->clear_visible();
    body->border( m_BodyBorders[i] );
    body->resize( rx, ry, body->w(), m_BodyHeights[i] );

    Forget( i );
}

void VspSidePanel::ForgetScreen( VspScreen* screen )
{
    int i = IndexOf( screen );
    if ( i < 0 )
    {
        return;
    }
    Fl_Window* body = screen->GetFlWindow();
    if ( body && body->parent() == this )
    {
        remove( body );
    }
    Forget( i );
}

void VspSidePanel::Forget( int i )
{
    m_Screens.erase( m_Screens.begin() + i );
    m_BodyBorders.erase( m_BodyBorders.begin() + i );
    m_BodyHeights.erase( m_BodyHeights.begin() + i );
    m_CloseButtons.erase( m_CloseButtons.begin() + i );
    if ( m_Shown == i )
    {
        m_Shown = -1;
    }
    else if ( m_Shown > i )
    {
        m_Shown--;
    }
    if ( m_LastShown == i )
    {
        m_LastShown = -1;
    }
    else if ( m_LastShown > i )
    {
        m_LastShown--;
    }
}

void VspSidePanel::ShowScreen( VspScreen* screen )
{
    int i = IndexOf( screen );
    if ( i < 0 )
    {
        return;
    }

    // One screen at a time
    for ( int j = 0; j < ( int )m_Screens.size(); j++ )
    {
        if ( j != i )
        {
            m_Screens[j]->GetFlWindow()->hide();
        }
    }

    m_Shown = i;
    m_LastShown = i;
    Place();

    Fl_Window* body = screen->GetFlWindow();
    show();
    body->show();
    CaptiveShowSubwindows( body );
}

void VspSidePanel::HideScreen( VspScreen* screen )
{
    int i = IndexOf( screen );
    if ( i < 0 )
    {
        return;
    }

    screen->GetFlWindow()->hide();
    if ( m_Shown == i )
    {
        m_Shown = -1;
        hide();
        Place();
    }
}

bool VspSidePanel::ScreenShown( VspScreen* screen )
{
    int i = IndexOf( screen );
    return i >= 0 && i == m_Shown && shown() && visible();
}

void VspSidePanel::Toggle()
{
    if ( m_Shown >= 0 )
    {
        m_Screens[m_Shown]->Hide();
        return;
    }

    // The screen shown last, else the first that shows, as a screen turned off does not
    if ( m_LastShown >= 0 )
    {
        m_Screens[m_LastShown]->Show();
    }
    for ( int i = 0; i < ( int )m_Screens.size() && m_Shown < 0; i++ )
    {
        m_Screens[i]->Show();
    }
}

void VspSidePanel::Place()
{
    if ( m_InPlace )
    {
        return;
    }
    m_InPlace = true;

    int lx = m_Layer->x();
    int ly = m_Layer->y();
    int lw = m_Layer->w();
    int lh = m_Layer->h();

    int i = m_Shown;
    if ( i < 0 )
    {
        i = m_LastShown;
    }
    if ( i < 0 && !m_Screens.empty() )
    {
        i = 0;
    }
    int pw = 0;
    if ( i >= 0 )
    {
        // The full height of the view, the screen's window as wide as it is
        Fl_Window* body = m_Screens[i]->GetFlWindow();
        pw = OuterW( body->w() );
        resize( lx + lw - pw, ly, pw, lh );
        int bh = lh - 2 * EDGE;
        if ( body->x() != EDGE || body->y() != EDGE || body->h() != bh )
        {
            body->resize( EDGE, EDGE, body->w(), bh );
        }
        m_CloseButtons[i]->resize( body->w() - CLOSE_W - 2, 2, CLOSE_W, CLOSE_W );
    }

    // Docked and shown, the panel takes its width from the view
    if ( m_Docked && m_Shown >= 0 )
    {
        m_Layer->SetViewRight( pw );
    }
    else
    {
        m_Layer->SetViewRight( 0 );
    }

    // The tab, at the top of the view's right edge, is there while the panel is hidden
    m_Tab->position( lx + lw - VspSideTab::W, ly + VspSideTab::TOP );
    Fl_Window* top = m_Layer->window();
    if ( m_Shown >= 0 )
    {
        m_Tab->hide();
    }
    else if ( top && top->shown() && !m_Tab->visible() )
    {
        m_Tab->show();
    }

    m_InPlace = false;
}

void VspSidePanel::SetDocked( bool docked )
{
    m_Docked = docked;
    Place();
}

void VspSidePanel::BodyResized()
{
    Place();
}

void VspSidePanel::CloseCB( Fl_Widget* w, void* data )
{
    VspSidePanel* panel = ( VspSidePanel* ) data;
    panel->Toggle();
}

//==== Side Tab ====//
VspSideTab::VspSideTab( VspSidePanel* panel ) : Fl_Double_Window( 0, 0, W, H )
{
    m_Panel = panel;

    box( FL_NO_BOX );
    m_Button = new Fl_Button( 0, 0, W, H, "@<" );
    m_Button->box( FL_UP_BOX );
    m_Button->labelsize( 10 );
    m_Button->labelcolor( FL_DARK3 );
    m_Button->visible_focus( 0 );
    m_Button->tooltip( "Show the Geom Browser" );
    m_Button->callback( PressCB, this );
    end();
    resizable( nullptr );
}

void VspSideTab::PressCB( Fl_Widget* w, void* data )
{
    VspSideTab* tab = ( VspSideTab* ) data;
    tab->m_Panel->Toggle();
}
