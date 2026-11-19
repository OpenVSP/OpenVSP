//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// VspCaptiveWindow.cpp: screens shown as windows inside the main window, over the 3D view
//
//////////////////////////////////////////////////////////////////////

#include "VspCaptiveWindow.h"
#include "ScreenBase.h"
#include "VSPWindow.h"

#include <FL/Fl.H>

#include <algorithm>

static Fl_Event_Dispatch s_PrevDispatch = nullptr;

// The layer whose frames are raised, and the frame clicked that is raised once the button is up
static VspCaptiveLayer* s_Layer = nullptr;
static Fl_Widget_Tracker* s_Clicked = nullptr;

//==== Captive Layer ====//
VspCaptiveLayer::VspCaptiveLayer( int x, int y, int w, int h ) : Fl_Group( x, y, w, h )
{
    m_View = nullptr;
    m_ViewRight = 0;
    box( FL_NO_BOX );
    end();
}

void VspCaptiveLayer::SetView( Fl_Window* view )
{
    m_View = view;
    insert( *view, 0 );
}

void VspCaptiveLayer::SetViewRight( int w )
{
    if ( w < 0 )
    {
        w = 0;
    }
    if ( w == m_ViewRight )
    {
        return;
    }
    m_ViewRight = w;
    if ( m_View )
    {
        m_View->resize( x(), y(), this->w() - m_ViewRight, h() );
        m_View->redraw();
    }
}

void VspCaptiveLayer::resize( int x, int y, int w, int h )
{
    Fl_Widget::resize( x, y, w, h );
    if ( m_View )
    {
        m_View->resize( x, y, w - m_ViewRight, h );
    }
    for ( int i = 0; i < children(); i++ )
    {
        VspCaptiveFrame* f = dynamic_cast< VspCaptiveFrame* >( child( i ) );
        if ( f && f->GetBody() )
        {
            f->Clamp();
        }
        VspBodyHolder* holder = dynamic_cast< VspBodyHolder* >( child( i ) );
        if ( holder && !f )
        {
            holder->BodyResized();
        }
    }
}

// Mouse events are the window's under the mouse -- a frame's, another window's in the layer, or the 3D view's beneath
// them.  A wheel turned over a window stays with it, used or not, as it would in a window of its own, and is not
// offered to the windows under it, as FLTK otherwise does.
int VspCaptiveLayer::handle( int event )
{
    switch ( event )
    {
        case FL_PUSH:
        case FL_ENTER:
        case FL_MOVE:
        case FL_MOUSEWHEEL:
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:
        {
            Fl_Window* f = WindowAt( Fl::event_x(), Fl::event_y() );
            if ( !f )
            {
                return 0;
            }
            if ( event == FL_MOUSEWHEEL )
            {
                int ex = Fl::e_x;
                int ey = Fl::e_y;
                Fl::e_x -= f->x();
                Fl::e_y -= f->y();
                f->handle( event );
                Fl::e_x = ex;
                Fl::e_y = ey;
                return 1;
            }
            break;
        }
        default:
            break;
    }
    return Fl_Group::handle( event );
}

Fl_Window* VspCaptiveLayer::WindowAt( int x, int y )
{
    for ( int i = children() - 1; i >= 0; i-- )
    {
        Fl_Window* w = child( i )->as_window();
        if ( w && w->shown() && w->visible() && x >= w->x() && x < w->x() + w->w() && y >= w->y() && y < w->y() + w->h() )
        {
            return w;
        }
    }
    return nullptr;
}

VspCaptiveFrame* VspCaptiveLayer::FrameAt( int x, int y )
{
    for ( int i = children() - 1; i >= 0; i-- )
    {
        VspCaptiveFrame* f = dynamic_cast< VspCaptiveFrame* >( child( i ) );
        if ( f && f->FrameShown() && x >= f->x() && x < f->x() + f->w() && y >= f->y() && y < f->y() + f->h() )
        {
            return f;
        }
    }
    return nullptr;
}

VspCaptiveFrame* VspCaptiveLayer::TopFrame()
{
    for ( int i = children() - 1; i >= 0; i-- )
    {
        VspCaptiveFrame* f = dynamic_cast< VspCaptiveFrame* >( child( i ) );
        if ( f && f->FrameShown() )
        {
            return f;
        }
    }
    return nullptr;
}

void VspCaptiveLayer::UpdateActive()
{
    VspCaptiveFrame* top = TopFrame();
    for ( int i = 0; i < children(); i++ )
    {
        VspCaptiveFrame* f = dynamic_cast< VspCaptiveFrame* >( child( i ) );
        if ( f )
        {
            f->SetActive( f == top );
        }
    }
}

//==== Captive Frame ====//
VspCaptiveFrame::VspCaptiveFrame( VspScreen* screen, Fl_Window* body, VspCaptiveLayer* layer, int x, int y ) :
    Fl_Double_Window( x, y, body->w() + 2 * EDGE, body->h() + TITLE_H + EDGE )
{
    m_Screen = screen;
    m_Body = body;
    m_Layer = layer;
    m_BodyBorder = ( body->border() != 0 );
    m_InLayout = false;
    m_Moving = false;
    m_SizeEdges = 0;
    m_EdgeCursor = false;
    m_StartMX = m_StartMY = 0;
    m_StartX = m_StartY = m_StartW = m_StartH = 0;

    box( FL_UP_BOX );

    m_TitleBox = new Fl_Box( EDGE, EDGE, w() - 2 * EDGE - TITLE_H, TITLE_H - EDGE );
    m_TitleBox->box( FL_FLAT_BOX );
    m_TitleBox->labelfont( FL_HELVETICA_BOLD );
    m_TitleBox->labelsize( 12 );
    m_TitleBox->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP );

    m_CloseButton = new Fl_Button( w() - EDGE - TITLE_H + 2, EDGE + 1, TITLE_H - 4, TITLE_H - EDGE - 2, "@1+" );
    m_CloseButton->box( FL_THIN_UP_BOX );
    m_CloseButton->labelsize( 10 );
    m_CloseButton->labelcolor( FL_DARK3 );
    m_CloseButton->visible_focus( 0 );
    m_CloseButton->callback( CloseCB, this );

    end();

    // The children are laid out here, not by FLTK's resizing
    resizable( nullptr );

    body->position( EDGE, TITLE_H );
    add( body );

    SetActive( false );

    layer->add( this );
    Clamp();
}

VspCaptiveFrame::~VspCaptiveFrame()
{
    if ( m_Body && m_Body->parent() == this )
    {
        remove( m_Body );
    }
}

void VspCaptiveFrame::SetTitle( const std::string &title )
{
    m_Title = title;
    // The title is a label, so @ is shown as itself
    std::string label;
    for ( size_t i = 0; i < m_Title.size(); i++ )
    {
        if ( m_Title[i] == '@' )
        {
            label += '@';
        }
        label += m_Title[i];
    }
    m_TitleBox->copy_label( ( "  " + label ).c_str() );
}

void VspCaptiveFrame::SetActive( bool active )
{
    if ( active )
    {
        m_TitleBox->color( FL_SELECTION_COLOR );
        m_TitleBox->labelcolor( FL_WHITE );
    }
    else
    {
        m_TitleBox->color( FL_DARK2 );
        m_TitleBox->labelcolor( FL_BLACK );
    }
    m_TitleBox->redraw();
}

void VspCaptiveFrame::ShowFrame()
{
    if ( !shown() )
    {
        // The Geom screens share one place on the screen, as windows of their own do
        VSP_Window* vw = dynamic_cast< VSP_Window* >( m_Body );
        Fl_Window* top = window();
        if ( vw && vw->GetGeomScreenFlag() && top )
        {
            position( VSP_Window::GetGeomRight() - m_Body->w() - top->x() - EDGE, VSP_Window::GetGeomY() - top->y() - TITLE_H );
        }
        Clamp();
    }

    // A frame shown is raised, as a window of its own is; one shown last is drawn above the others
    if ( FrameShown() )
    {
        Raise();
        return;
    }
    m_Layer->insert( *this, m_Layer->children() );
    show();
    m_Body->show();
    CaptiveShowSubwindows( m_Body );
    m_Layer->UpdateActive();
}

void VspCaptiveFrame::HideFrame()
{
    hide();
    m_Layer->UpdateActive();
}

bool VspCaptiveFrame::FrameShown()
{
    return shown() && visible();
}

void VspCaptiveFrame::Raise()
{
    int last = m_Layer->children() - 1;
    if ( m_Layer->TopFrame() == this && m_Layer->child( last ) == this )
    {
        return;
    }
    m_Layer->insert( *this, m_Layer->children() );

    if ( FrameShown() && !CaptiveRaiseNative( this ) )
    {
        // A window shown is drawn above those shown before it.  Hiding the frame takes the keyboard from the widget
        // in it that has it, which is given back.
        Fl_Widget_Tracker focus( Fl::focus() );
        hide();
        show();
        m_Body->show();
        CaptiveShowSubwindows( m_Body );
        if ( focus.widget() && contains( focus.widget() ) )
        {
            Fl::focus( focus.widget() );
            focus.widget()->redraw();
        }
    }
    m_Layer->UpdateActive();
}

void VspCaptiveFrame::BodyResized()
{
    if ( m_InLayout )
    {
        return;
    }
    resize( x(), y(), m_Body->w() + 2 * EDGE, m_Body->h() + TITLE_H + EDGE );
}

void VspCaptiveFrame::resize( int x, int y, int w, int h )
{
    Fl_Double_Window::resize( x, y, w, h );
    Layout();
}

void VspCaptiveFrame::Layout()
{
    m_InLayout = true;
    m_TitleBox->resize( EDGE, EDGE, w() - 2 * EDGE - TITLE_H, TITLE_H - EDGE );
    m_CloseButton->resize( w() - EDGE - TITLE_H + 2, EDGE + 1, TITLE_H - 4, TITLE_H - EDGE - 2 );
    if ( !m_Body )
    {
        m_InLayout = false;
        return;
    }
    int bw = w() - 2 * EDGE;
    int bh = h() - TITLE_H - EDGE;
    if ( m_Body->x() != EDGE || m_Body->y() != TITLE_H || m_Body->w() != bw || m_Body->h() != bh )
    {
        m_Body->resize( EDGE, TITLE_H, bw, bh );
    }
    m_InLayout = false;
    redraw();
}

void VspCaptiveFrame::Clamp()
{
    int lx = m_Layer->x();
    int ly = m_Layer->y();
    int lw = m_Layer->w();
    int lh = m_Layer->h();

    int nw = w();
    int nh = h();
    if ( CanSize() )
    {
        int minw, minh, maxw, maxh;
        SizeLimits( minw, minh, maxw, maxh );
        nw = std::max( minw, std::min( nw, lw ) );
        nh = std::max( minh, std::min( nh, lh ) );
    }

    int nx = std::max( lx, std::min( x(), lx + lw - nw ) );
    int ny = std::max( ly, std::min( y(), ly + lh - nh ) );

    if ( nx != x() || ny != y() || nw != w() || nh != h() )
    {
        resize( nx, ny, nw, nh );
    }
}

// The body sizes as its screen allows, the frame around it
bool VspCaptiveFrame::CanSize()
{
    if ( !m_Body->resizable() )
    {
        return false;
    }
    int minw, minh, maxw, maxh;
    m_Body->get_size_range( &minw, &minh, &maxw, &maxh );
    return !( maxw > 0 && maxh > 0 && minw == maxw && minh == maxh );
}

void VspCaptiveFrame::SizeLimits( int &minw, int &minh, int &maxw, int &maxh )
{
    int bminw, bminh, bmaxw, bmaxh;
    m_Body->get_size_range( &bminw, &bminh, &bmaxw, &bmaxh );
    minw = std::max( bminw, 100 ) + 2 * EDGE;
    minh = std::max( bminh, 60 ) + TITLE_H + EDGE;
    maxw = 0;
    maxh = 0;
    if ( bmaxw > 0 )
    {
        maxw = bmaxw + 2 * EDGE;
    }
    if ( bmaxh > 0 )
    {
        maxh = bmaxh + TITLE_H + EDGE;
    }
}

// The edges a press sizes: those within a corner's reach along an edge, or the edge alone
int VspCaptiveFrame::EdgesAt( int ex, int ey )
{
    if ( !CanSize() )
    {
        return 0;
    }
    const int corner = 14;
    int edges = 0;
    bool on_l = ex < EDGE;
    bool on_r = ex >= w() - EDGE;
    bool on_t = ey < EDGE;
    bool on_b = ey >= h() - EDGE;
    if ( on_l || on_r )
    {
        if ( on_l )
        {
            edges |= EDGE_L;
        }
        else
        {
            edges |= EDGE_R;
        }
        if ( ey < corner )
        {
            edges |= EDGE_T;
        }
        else if ( ey >= h() - corner )
        {
            edges |= EDGE_B;
        }
    }
    if ( on_t || on_b )
    {
        if ( on_t )
        {
            edges |= EDGE_T;
        }
        else
        {
            edges |= EDGE_B;
        }
        if ( ex < corner )
        {
            edges |= EDGE_L;
        }
        else if ( ex >= w() - corner )
        {
            edges |= EDGE_R;
        }
    }
    return edges;
}

void VspCaptiveFrame::SetEdgeCursor( int edges )
{
    Fl_Cursor c = FL_CURSOR_DEFAULT;
    if ( edges == ( EDGE_L | EDGE_T ) || edges == ( EDGE_R | EDGE_B ) )
    {
        c = FL_CURSOR_NWSE;
    }
    else if ( edges == ( EDGE_R | EDGE_T ) || edges == ( EDGE_L | EDGE_B ) )
    {
        c = FL_CURSOR_NESW;
    }
    else if ( edges & ( EDGE_L | EDGE_R ) )
    {
        c = FL_CURSOR_WE;
    }
    else if ( edges & ( EDGE_T | EDGE_B ) )
    {
        c = FL_CURSOR_NS;
    }
    cursor( c );
    m_EdgeCursor = ( edges != 0 );
}

// The body's part of the frame, whose events are its own
bool VspCaptiveFrame::InBody( int ex, int ey )
{
    return ex >= EDGE && ex < w() - EDGE && ey >= TITLE_H && ey < h() - EDGE;
}

void VspCaptiveFrame::KeepGeomPosition()
{
    VSP_Window* vw = dynamic_cast< VSP_Window* >( m_Body );
    Fl_Window* top = window();
    if ( vw && vw->GetGeomScreenFlag() && top )
    {
        VSP_Window::SetGeomRight( top->x() + x() + EDGE + m_Body->w() );
        VSP_Window::SetGeomY( top->y() + y() + TITLE_H );
    }
}

int VspCaptiveFrame::handle( int event )
{
    int ex = Fl::event_x();
    int ey = Fl::event_y();

    switch ( event )
    {
        case FL_PUSH:
        {
            if ( Fl::event_inside( m_CloseButton ) || Fl::event_button() != FL_LEFT_MOUSE || InBody( ex, ey ) )
            {
                break;
            }
            m_SizeEdges = EdgesAt( ex, ey );
            m_Moving = ( m_SizeEdges == 0 && ey < TITLE_H );
            if ( !m_Moving && !m_SizeEdges )
            {
                break;
            }
            m_StartMX = Fl::event_x_root();
            m_StartMY = Fl::event_y_root();
            m_StartX = x();
            m_StartY = y();
            m_StartW = w();
            m_StartH = h();
            return 1;
        }
        case FL_DRAG:
        {
            int dx = Fl::event_x_root() - m_StartMX;
            int dy = Fl::event_y_root() - m_StartMY;
            if ( m_Moving )
            {
                int lx = m_Layer->x();
                int ly = m_Layer->y();
                int nx = std::max( lx, std::min( m_StartX + dx, lx + m_Layer->w() - w() ) );
                int ny = std::max( ly, std::min( m_StartY + dy, ly + m_Layer->h() - h() ) );
                position( nx, ny );
                return 1;
            }
            if ( m_SizeEdges )
            {
                int minw, minh, maxw, maxh;
                SizeLimits( minw, minh, maxw, maxh );
                int l = m_StartX;
                int t = m_StartY;
                int r = m_StartX + m_StartW;
                int b = m_StartY + m_StartH;
                int ll = m_Layer->x();
                int lt = m_Layer->y();
                int lr = ll + m_Layer->w();
                int lb = lt + m_Layer->h();
                if ( m_SizeEdges & EDGE_L )
                {
                    l = std::max( ll, std::min( l + dx, r - minw ) );
                    if ( maxw > 0 )
                    {
                        l = std::max( l, r - maxw );
                    }
                }
                if ( m_SizeEdges & EDGE_R )
                {
                    r = std::min( lr, std::max( r + dx, l + minw ) );
                    if ( maxw > 0 )
                    {
                        r = std::min( r, l + maxw );
                    }
                }
                if ( m_SizeEdges & EDGE_T )
                {
                    t = std::max( lt, std::min( t + dy, b - minh ) );
                    if ( maxh > 0 )
                    {
                        t = std::max( t, b - maxh );
                    }
                }
                if ( m_SizeEdges & EDGE_B )
                {
                    b = std::min( lb, std::max( b + dy, t + minh ) );
                    if ( maxh > 0 )
                    {
                        b = std::min( b, t + maxh );
                    }
                }
                resize( l, t, r - l, b - t );
                return 1;
            }
            break;
        }
        case FL_RELEASE:
        {
            if ( m_Moving || m_SizeEdges )
            {
                m_Moving = false;
                m_SizeEdges = 0;
                KeepGeomPosition();
                SetEdgeCursor( EdgesAt( ex, ey ) );
                return 1;
            }
            break;
        }
        case FL_ENTER:
        case FL_MOVE:
        {
            if ( !InBody( ex, ey ) )
            {
                SetEdgeCursor( EdgesAt( ex, ey ) );
            }
            else if ( m_EdgeCursor )
            {
                SetEdgeCursor( 0 );
            }
            break;
        }
        case FL_LEAVE:
        {
            if ( m_EdgeCursor )
            {
                SetEdgeCursor( 0 );
            }
            break;
        }
        default:
            break;
    }
    return Fl_Double_Window::handle( event );
}

void VspCaptiveFrame::CloseCB( Fl_Widget* w, void* data )
{
    VspCaptiveFrame* f = static_cast< VspCaptiveFrame* >( data );
    f->m_Screen->CloseFromFrame();
}

Fl_Window* VspCaptiveFrame::ReleaseBody()
{
    // The body's place on the screen, which the frame's window is at
    Fl_Window* top = window();
    int rx = x() + EDGE;
    int ry = y() + TITLE_H;
    while ( top )
    {
        rx += top->x();
        ry += top->y();
        top = top->window();
    }

    hide();
    remove( m_Body );
    Fl_Window* body = m_Body;
    m_Body = nullptr;
    // Hiding the frame leaves the body marked visible, which FLTK takes a window of its own to be only once shown
    body->clear_visible();
    body->border( m_BodyBorder );
    body->position( rx, ry );
    return body;
}

void VspCaptiveFrame::ForgetBody()
{
    if ( m_Body && m_Body->parent() == this )
    {
        remove( m_Body );
    }
    m_Body = nullptr;
}

VspCaptiveFrame* VspCaptiveFrame::FrameOf( Fl_Widget* w )
{
    while ( w )
    {
        VspCaptiveFrame* f = dynamic_cast< VspCaptiveFrame* >( w );
        if ( f )
        {
            return f;
        }
        w = w->parent();
    }
    return nullptr;
}

void VspCaptiveFrame::InstallRaiseHook( VspCaptiveLayer* layer )
{
    s_Layer = layer;
    if ( Fl::event_dispatch() == Dispatch )
    {
        return;
    }
    s_PrevDispatch = Fl::event_dispatch();
    Fl::event_dispatch( Dispatch );
}

// The frame the mouse is over in a window: the window's own frame, or for the main window the frame on top under the
// mouse, as some platforms give events in a subwindow to the window it is in
static VspCaptiveFrame* FrameUnderMouse( Fl_Window* w )
{
    VspCaptiveFrame* f = VspCaptiveFrame::FrameOf( w );
    if ( f || !s_Layer || w != s_Layer->window() )
    {
        return f;
    }
    return s_Layer->FrameAt( Fl::event_x(), Fl::event_y() );
}

// A press in a frame raises it before the press is handled, so a menu it opens is not covered.  Where a frame is
// raised by showing it again, it is raised once the button is up instead, so the press is not lost.  A key raises
// the frame the key went to.
int VspCaptiveFrame::Dispatch( int event, Fl_Window* w )
{
    if ( event == FL_PUSH )
    {
        VspCaptiveFrame* f = FrameUnderMouse( w );
        if ( f && f->m_Body )
        {
            if ( CaptiveCanRaiseNative() )
            {
                f->Raise();
            }
            else
            {
                delete s_Clicked;
                s_Clicked = new Fl_Widget_Tracker( f );
            }
        }
    }

    int ret;
    if ( s_PrevDispatch )
    {
        ret = s_PrevDispatch( event, w );
    }
    else
    {
        ret = Fl::handle_( event, w );
    }

    if ( event == FL_RELEASE && s_Clicked && !Fl::event_buttons() )
    {
        VspCaptiveFrame* f = dynamic_cast< VspCaptiveFrame* >( s_Clicked->widget() );
        delete s_Clicked;
        s_Clicked = nullptr;
        if ( f && f->m_Body && !Fl::grab() && !Fl::modal() )
        {
            f->Raise();
        }
    }

    if ( event == FL_KEYBOARD && !Fl::grab() && !Fl::modal() )
    {
        VspCaptiveFrame* f = FrameOf( Fl::focus() );
        if ( f && f->m_Body )
        {
            f->Raise();
        }
    }
    return ret;
}

void CaptiveShowSubwindows( Fl_Group* g )
{
    for ( int i = 0; i < g->children(); i++ )
    {
        Fl_Widget* c = g->child( i );
        Fl_Window* win = c->as_window();
        if ( win && win->visible() && !win->shown() )
        {
            win->show();
        }
        Fl_Group* cg = c->as_group();
        if ( cg && cg->visible() )
        {
            CaptiveShowSubwindows( cg );
        }
    }
}
