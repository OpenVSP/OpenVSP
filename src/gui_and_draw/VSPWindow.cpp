//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// Custom FLTK Window type to allow windows to be attached to each other
// Alex Gary
//
//////////////////////////////////////////////////////////////////////

#include <FL/Fl.H>
#include "VSPWindow.h"
#include <cstdlib>

int VSP_Window::m_x = 1;
int VSP_Window::m_y = 1;

VSP_Window::VSP_Window(int w, int h, const char* title) : Fl_Double_Window( w, h, title )
{
    m_GeomScreen = false;
    m_Parent = NULL;
    m_AttachType = WIN_ATT_NONE;
    m_AlignType = WIN_ALN_NONE;

}

VSP_Window::VSP_Window(int x, int y, int w, int h, const char* l) : Fl_Double_Window(x , y, w, h, l)
{
    m_GeomScreen = false;
    m_Parent = NULL;
    m_AttachType = WIN_ATT_NONE;
    m_AlignType = WIN_ALN_NONE;
}

void VSP_Window::SetAlignType( int t )
{
    if ( t >= WIN_ALN_NONE && t < WIN_ALN_NUM_TYPES )
        m_AlignType = t;
}

void VSP_Window::SetAttachType( int t )
{
    if ( t >= WIN_ATT_NONE && t < WIN_ATT_NUM_TYPES )
        m_AttachType = t;
}

void VSP_Window::AddChild( VSP_Window* win )
{
    unsigned int c;
    if ( win )
    {
        // Check if already a child
        for ( c = 0 ; c < m_Children.size(); c++ )
        {
            if ( m_Children[c] == win ) return;
        }
        m_Children.push_back( win );
        win->SetParent( this );
    }
}

void VSP_Window::RemoveChild( VSP_Window* win )
{
    unsigned int c;
    for ( c = 0 ; c < m_Children.size(); c++ )
    {
        if ( m_Children[c] == win )
        {
            m_Children.erase( m_Children.begin()+c );
            win->SetParent(NULL);
            return;
        }
    }
}

void VSP_Window::SetParent( VSP_Window* win)
{
    if ( m_Parent )
    {
        m_Parent->RemoveChild(this);
    }
    m_Parent = win;
}

void VSP_Window::position( int X, int Y )
{
    Fl_Double_Window::position(X,Y);
}

void VSP_Window::show()
{
    if ( m_GeomScreen )
    {
        position( m_x, m_y);
    }

    Fl_Double_Window::show();
}


void VSP_Window::resize( int x_in, int y_in, int w_in, int h_in )
{
    if ( m_GeomScreen && shown() )
    {
        m_x = x_in;
        m_y = y_in;
    }

    bool update_par = ( x_in != x() || y_in != y() || w_in != w() || h_in != h() );

    Fl_Double_Window::resize(x_in,y_in,w_in,h_in);

    // Update parents location if the parent exists

    if ( m_Parent && update_par )
    {
 //       UpdateParent();
    }

    UpdateChildren();
}

void VSP_Window::resizeFromParent(int x_in, int y_in, int w_in, int h_in)
{
    if ( m_GeomScreen && shown() )
    {
        m_x = x_in;
        m_y = y_in;
    }

    Fl_Double_Window::resize( x_in, y_in, w_in, h_in );
    UpdateChildren();
}

void VSP_Window::resizeFromChild(int x_in, int y_in, int w_in, int h_in, VSP_Window * child)
{
    if ( m_GeomScreen && shown() )
    {
        m_x = x_in;
        m_y = y_in;
    }

    Fl_Double_Window::resize( x_in, y_in, w_in, h_in );

    bool update = ( x_in != x() || y_in != y() || w_in != w() || h_in != h() );
    if ( update )
    {
//        UpdateParent();
        UpdateChildren( child );
    }
}

void VSP_Window::UpdateChildren( VSP_Window* child)
{
    unsigned int i;

    for ( i = 0 ; i < m_Children.size() ; i++ )
    {
        VSP_Window* c = m_Children[i];

        if ( c && c != child )
        {
            int new_x, new_y;
            new_x = c->x();
            new_y = c->y();

            if ( c->GetAttachType() == WIN_ATT_RSIDE )
            {
                new_x = x() + w();
            }
            else if ( c->GetAttachType() == WIN_ATT_LSIDE )
            {
                new_x = x() - c->w();
            }
            else if ( c->GetAttachType() == WIN_ATT_BOTTOM )
            {
                new_y = y() + h();
            }
            else if ( c->GetAttachType() == WIN_ATT_TOP )
            {
                new_y = y() + c->h();
            }

            if ( c->GetAlignType() == WIN_ALN_RIGHT )
            {
                new_x = x();
            }
            else if ( c->GetAlignType() == WIN_ALN_LEFT )
            {
                new_x = x() + w() - c->w();
            }
            else if ( c->GetAlignType() == WIN_ALN_TOP )
            {
                new_y = y();
            }
            else if ( c->GetAlignType() == WIN_ALN_BOTTOM )
            {
                new_y = y() + h() - c->h();
            }

            c->resizeFromParent(new_x,new_y,c->w(),c->h());

            // Show Child if Parent is shown
            if ( shown() && c->shown() ) c->show();
        }
    }
}

void VSP_Window::UpdateParent()
{
    if ( m_Parent )
    {
        int new_x, new_y;
        new_x = m_Parent->x();
        new_y = m_Parent->y();

        if ( m_AttachType == WIN_ATT_RSIDE )
        {
            new_x = x() - m_Parent->w();
        }
        else if ( m_AttachType == WIN_ATT_LSIDE )
        {
            new_x = x() + w();
        }
        else if ( m_AttachType == WIN_ATT_BOTTOM )
        {
            new_y = y() + m_Parent->h();
        }
        else if ( m_AttachType == WIN_ATT_TOP )
        {
            new_y = y() + h();
        }

        if ( m_AlignType == WIN_ALN_RIGHT )
        {
            new_x = x() + w() - m_Parent->w();
        }
        else if ( m_AlignType == WIN_ALN_LEFT )
        {
            new_x = x();
        }
        else if ( m_AlignType == WIN_ALN_TOP )
        {
            new_y = y();
        }
        else if ( m_AlignType == WIN_ALN_BOTTOM )
        {
            new_y = y() + h() - m_Parent->h();
        }

        m_Parent->resizeFromChild(new_x, new_y, m_Parent->w(), m_Parent->h(), this );
    }
}

int VSP_Window::handle( int fl_event )
{

    int x = Fl::event_x();
    int y = Fl::event_y();

//    switch( fl_event )
//    {
//    case FL_ENTER:
//        cursor( FL_CURSOR_CROSS );
//        return 1;
//
//    case FL_LEAVE:
//        cursor( FL_CURSOR_DEFAULT );
//        return 1;
//
//    case FL_PUSH:
//        printf("%d, %d\n",x,y);
//        return 1;
//    }
    return Fl_Double_Window::handle( fl_event );
}




