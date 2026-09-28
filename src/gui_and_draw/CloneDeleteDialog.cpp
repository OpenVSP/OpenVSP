//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CloneDeleteDialog.cpp: asks what becomes of the Clones of Geoms being deleted or cut.
//
//////////////////////////////////////////////////////////////////////

#include "CloneDeleteDialog.h"

#include "APIDefines.h"
#include "Vehicle.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>

using std::string;
using std::vector;

// Set by the button that closes the window.  The dialog is modal, so one is enough.
static int s_CloneDeleteChoice = -1;

static void CloneDeleteChoiceCB( Fl_Widget* w, long choice )
{
    s_CloneDeleteChoice = ( int )choice;
    w->window()->hide();
}

// Closing the window, or Escape, is Cancel.
static void CloneDeleteCloseCB( Fl_Widget* w, void* )
{
    s_CloneDeleteChoice = -1;
    w->hide();
}

void DeleteOrCutActiveGeomVec( Vehicle* veh, bool cut )
{
    if ( !veh )
    {
        return;
    }

    vector< string > clone_vec = veh->FindClonesOf( veh->GetActiveGeomVec() );

    if ( clone_vec.empty() )
    {
        if ( cut )
        {
            veh->CutActiveGeomVec();
        }
        else
        {
            veh->DeleteActiveGeomVec();
        }
        return;
    }

    int clone_delete = AskCloneDelete( veh, clone_vec, cut );
    if ( clone_delete < 0 )
    {
        return;
    }

    if ( cut )
    {
        veh->CutActiveGeomVec( clone_delete );
    }
    else
    {
        veh->DeleteActiveGeomVec( clone_delete );
    }
}

int AskCloneDelete( Vehicle* veh, const vector< string > & clone_vec, bool cut )
{
    string verb = "Deleting";
    string with_label = "Delete Too";
    string with_text = "delete them along with it";
    if ( cut )
    {
        verb = "Cutting";
        with_label = "Cut Too";
        with_text = "cut them along with it, to be pasted with it";
    }

    string count = "1 Clone";
    if ( clone_vec.size() != 1 )
    {
        count = std::to_string( clone_vec.size() ) + " Clones";
    }

    string message = verb + " this would leave " + count + " with nothing to copy:\n\n";

    // Taking them along also takes their own Clones, which the list does not show.
    vector< string > all_vec = veh->FindAllClonesOf( veh->GetActiveGeomVec() );
    int nmore = ( int )all_vec.size() - ( int )clone_vec.size();
    if ( nmore == 1 )
    {
        with_text += ", and the 1 Clone of them";
    }
    else if ( nmore > 1 )
    {
        with_text += ", and the " + std::to_string( nmore ) + " Clones of them";
    }

    // List the first few; summarize the rest.
    const int max_listed = 8;
    for ( int i = 0; i < ( int )clone_vec.size() && i < max_listed; i++ )
    {
        Geom* clone = veh->FindGeom( clone_vec[i] );
        if ( clone )
        {
            // Escape '@', which an FLTK label treats as a symbol.
            string name = clone->GetName();
            for ( size_t pos = name.find( '@' ); pos != string::npos; pos = name.find( '@', pos + 2 ) )
            {
                name.insert( pos, "@" );
            }
            message += "    " + name + "\n";
        }
    }
    if ( ( int )clone_vec.size() > max_listed )
    {
        message += "    ... and " + std::to_string( clone_vec.size() - max_listed ) + " more\n";
    }

    message += "\nLeave Empty -- keep them where they are, copying nothing\n"
               "Replace -- make each a full copy of what it was copying\n" +
               with_label + " -- " + with_text;

    const int win_w = 460;
    const int margin = 15;
    const int button_w = 100;
    const int button_h = 25;
    const int gap = ( win_w - 2 * margin - 4 * button_w ) / 3;

    // Size the window to the text.  Measure as the label draws it: symbols on, 3 px inset per side.
    fl_font( FL_HELVETICA, FL_NORMAL_SIZE );
    int text_w = win_w - 2 * margin - 6;
    int text_h = 0;
    fl_measure( message.c_str(), text_w, text_h, 1 );

    int win_h = margin + text_h + margin + button_h + margin;

    Fl_Double_Window* win = new Fl_Double_Window( win_w, win_h, "Clones" );
    win->callback( CloneDeleteCloseCB );

    Fl_Box* text = new Fl_Box( margin, margin, win_w - 2 * margin, text_h );
    text->align( FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_WRAP );
    text->labelsize( FL_NORMAL_SIZE );
    text->copy_label( message.c_str() );

    int y = margin + text_h + margin;
    int x = margin;

    Fl_Button* cancel = new Fl_Button( x, y, button_w, button_h, "Cancel" );
    cancel->callback( CloneDeleteChoiceCB, -1 );
    x += button_w + gap;

    Fl_Button* leave = new Fl_Button( x, y, button_w, button_h, "Leave Empty" );
    leave->callback( CloneDeleteChoiceCB, vsp::CLONE_DELETE_LEAVE_EMPTY );
    x += button_w + gap;

    Fl_Button* replace = new Fl_Button( x, y, button_w, button_h, "Replace" );
    replace->callback( CloneDeleteChoiceCB, vsp::CLONE_DELETE_REPLACE );
    x += button_w + gap;

    Fl_Button* with = new Fl_Button( x, y, button_w, button_h );
    with->copy_label( with_label.c_str() );
    with->callback( CloneDeleteChoiceCB, vsp::CLONE_DELETE_WITH_ORIGINAL );

    win->end();
    win->set_modal();

    // Center on the screen the pointer is on.
    int sx, sy, sw, sh;
    Fl::screen_work_area( sx, sy, sw, sh );
    win->position( sx + ( sw - win_w ) / 2, sy + ( sh - win_h ) / 2 );

    s_CloneDeleteChoice = -1;
    win->show();
    while ( win->shown() )
    {
        Fl::wait();
    }

    delete win;

    return s_CloneDeleteChoice;
}
