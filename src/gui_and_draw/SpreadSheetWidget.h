//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Rob McDonald
//
//////////////////////////////////////////////////////////////////////

#if !defined(SPREADSHEET_WIDGET_INCLUDED_)
#define SPREADSHEET_WIDGET_INCLUDED_

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "VspUtil.h"
#include "StringUtil.h"

using std::vector;

template < typename T >
class SpreadSheet : public Fl_Table
{
public:
    SpreadSheet( int X, int Y, int W, int H, const char* L = 0 );

    SpreadSheet() {}

    // Apply value from input widget to values[row][col] array and hide (done editing)
    void set_value_hide();

    // Start editing a new cell: move the Fl_Int_Input widget to specified row/column
    //    Preload the widget with the cell's current value,
    //    and make the widget 'appear' at the cell's location.
    //
    void start_editing( int R, int C );

    // Tell the input widget it's done editing, and to 'hide'
    void done_editing();

    void set_value( int R, int C, const char * str );
    std::string get_value( int R, int C );
    std::string get_exact_value( int R, int C );
    void set_data( vector < T > *data );

    int get_data_nrow() { return (*m_Data).size(); };
    int get_data_ncol();

    void row_default_height( int h ) { m_DefHeight = h; };
    void col_default_width( int w ) { m_DefWidth = w; };

    void set_AllowPasteGrowCols( bool g ) { m_AllowPasteGrowCols = g; };
    bool get_AllowPasteGrowCols()         { return m_AllowPasteGrowCols; };
    void set_AllowPasteGrowRows( bool g ) { m_AllowPasteGrowRows = g; };
    bool get_AllowPasteGrowRows()         { return m_AllowPasteGrowRows; };

    void set_HeaderOffset( int o ) { m_HeaderOffset = o; };

    void set_LabelAlign( int a ) { m_LabelAlign = a; };

    void set_ChangeCallback( Fl_Callback *ccb, void *data ) { m_ChangeCallback = ccb; m_ChangeCallbackData = data; };

protected:

    int handle( int event );

    string col_header_txt( int C );

    void draw_cell( TableContext context, int = 0, int = 0, int = 0, int = 0, int = 0, int = 0 );

    void event_callback2();                               // table's event callback (instance)

    int copy(int clipboard);

    void paste( const char *str, int len );

    void update_size();

    virtual void update_size( int nrow, int ncol );

    static void event_callback( Fl_Widget*, void *v )     // table's event callback (static)
    {
        ( ( SpreadSheet* ) v )->event_callback2();
    }

    static void input_cb( Fl_Widget*, void* v )           // input widget's callback
    {
        ( ( SpreadSheet* ) v )->set_value_hide();
    }

    Fl_Multiline_Input *input;                                      // single instance of Fl_Input widget
    int row_edit, col_edit;                               // row/col being modified

    int m_DefHeight;
    int m_DefWidth;

    bool m_AllowPasteGrowCols;
    bool m_AllowPasteGrowRows;

    int m_HeaderOffset;

    int m_LabelAlign;

    vector < T > *m_Data;

    Fl_Callback* m_ChangeCallback;
    void* m_ChangeCallbackData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////


template < typename T >
SpreadSheet<T>::SpreadSheet( int X, int Y, int W, int H, const char* L ) : Fl_Table( X, Y, W, H, L )
{
    callback( &event_callback, ( void* )this );
    when( FL_WHEN_NOT_CHANGED | when() );

    // Create input widget that we'll use whenever user clicks on a cell
    input = new Fl_Multiline_Input( W / 2, H / 2, 0, 0 );
    input->hide();
    input->callback( input_cb, ( void* )this );
    input->when( FL_WHEN_ENTER_KEY_ALWAYS );            // callback triggered when user hits Enter
    input->textsize( 12 );

    end();

    row_edit = col_edit = 0;
    set_selection( 0, 0, 0, 0 );

    m_Data = nullptr;

    m_AllowPasteGrowCols = true;
    m_AllowPasteGrowRows = true;

    m_HeaderOffset = 0;

    m_LabelAlign = FL_ALIGN_RIGHT;

    m_ChangeCallback = nullptr;
    m_ChangeCallbackData = nullptr;
}

// Apply value from input widget to values[row][col] array and hide (done editing)
template < typename T >
void SpreadSheet<T>::set_value_hide()
{
    set_value( row_edit, col_edit, input->value() );
    input->hide();
    window()->cursor( FL_CURSOR_DEFAULT );              // XXX: if we don't do this, cursor can disappear!

    if ( m_ChangeCallback )
    {
        m_ChangeCallback ( this, m_ChangeCallbackData );
    }
}

// Start editing a new cell: move the Fl_Int_Input widget to specified row/column
//    Preload the widget with the cell's current value,
//    and make the widget 'appear' at the cell's location.
//
template < typename T >
void SpreadSheet<T>::start_editing( int R, int C )
{
    row_edit = R;                                       // Now editing this row/col
    col_edit = C;
    set_selection( R, C, R, C );                        // Clear any previous multicel selection
    int X, Y, W, H;
    find_cell( CONTEXT_CELL, R, C, X, Y, W, H );        // Find X/Y/W/H of cell
    input->resize( X, Y, W, H );                        // Move Fl_Input widget there
    string s = get_value( R, C );
    input->value( s.c_str() );                          // Load input widget with cell's current value
    input->insert_position( 0, s.size() );              // Select entire input field
    input->show();                                      // Show the input widget, now that we've positioned it
    input->take_focus();                                // Put keyboard focus into the input widget
}

// Tell the input widget it's done editing, and to 'hide'
template < typename T >
void SpreadSheet<T>::done_editing()
{
    if ( input->visible() )                             // input widget visible, ie. edit in progress?
    {
        set_value_hide();                                 // Transfer its current contents to cell and hide
        row_edit = 0;                                   // Disable these until needed again
        col_edit = 0;
    }
}

template < typename T >
void SpreadSheet<T>::set_data( vector < T > *data )
{
    m_Data = data;

    update_size();
}

template < typename T >
int SpreadSheet<T>::handle( int event )
{
    int R = callback_row();
    int C = callback_col();

    switch ( event )
    {
        case FL_UNFOCUS:
        case FL_FOCUS:
            damage_zone( current_row, current_col, select_row, select_col, R, C );
            break;
        case FL_PASTE:
        {
            paste( Fl::event_text(), Fl::event_length() );
            break;
        }
        default:
            break;
    }

    return Fl_Table::handle( event );
}

template < typename T >
string SpreadSheet<T>::col_header_txt( int index )
{
    index = index + m_HeaderOffset;
    int base = 'Z' - 'A' + 1;
    string name;
    do
    {
        name = string( 1, 'A' +  index % base ) + name;
        index = index / base - 1;
    } while ( index >= 0 );

    return name;
}

// Handle drawing all cells in table
template < typename T >
void SpreadSheet<T>::draw_cell( TableContext context, int R, int C, int X, int Y, int W, int H )
{
    static char s[30];
    switch ( context )
    {
    case CONTEXT_STARTPAGE:                     // table about to redraw
        break;

    case CONTEXT_COL_HEADER:                    // table wants us to draw a column heading (C is column)
        fl_font( FL_HELVETICA | FL_BOLD, 12 );    // set font for heading to bold
        fl_push_clip( X, Y, W, H );               // clip region for text
        {
            fl_draw_box( FL_THIN_UP_BOX, X, Y, W, H, col_header_color() );
            fl_color( FL_BLACK );
            fl_draw( col_header_txt( C ).c_str(), X, Y, W, H, FL_ALIGN_CENTER );
        }
        fl_pop_clip();
        return;

    case CONTEXT_ROW_HEADER:                    // table wants us to draw a row heading (R is row)
        fl_font( FL_HELVETICA | FL_BOLD, 12 );    // set font for row heading to bold
        fl_push_clip( X, Y, W, H );
        {
            fl_draw_box( FL_THIN_UP_BOX, X, Y, W, H, row_header_color() );
            fl_color( FL_BLACK );
            snprintf( s, sizeof(s), "%d", R + 1 );
            fl_draw( s, X, Y, W, H, FL_ALIGN_CENTER );
        }
        fl_pop_clip();
        return;

    case CONTEXT_CELL:                          // table wants us to draw a cell
        {
            if ( R == row_edit && C == col_edit && input->visible() )
            {
                return;                                 // dont draw for cell with input widget over it
            }
            // Background

            if ( R == select_row && C == select_col && Fl::focus() == this ) // Selected cell
            {
                fl_draw_box( FL_BORDER_BOX, X, Y, W - 1, H - 1, FL_WHITE );
            }
            else if ( is_selected( R, C ) && Fl::focus() == this ) // Selected field
            {
                fl_draw_box( FL_FLAT_BOX, X, Y, W, H, 52 ); // 52 light grey, larger nuber is lighter.
            }
            else
            {
                fl_draw_box( FL_THIN_UP_BOX, X, Y, W, H, FL_WHITE ); // Rest of table
            }

            // Text
            fl_push_clip( X + 3, Y + 3, W - 6, H - 6 );
            {
                fl_color( FL_BLACK );
                fl_font( FL_HELVETICA, 12 );          // ..in regular font
                fl_draw( get_value( R, C ).c_str(), X + 3, Y + 3, W - 6, H - 6, m_LabelAlign );
            }
            fl_pop_clip();
            return;
        }

    case CONTEXT_RC_RESIZE:                     // table resizing rows or columns
        if ( input->visible() )
        {
            find_cell( CONTEXT_TABLE, row_edit, col_edit, X, Y, W, H );
            input->resize( X, Y, W, H );
            init_sizes();
        }
        return;

    default:
        return;
    }
}

// Callback whenever someone clicks on different parts of the table
template < typename T >
void SpreadSheet<T>::event_callback2()
{
    int R = callback_row();
    int C = callback_col();
    TableContext context = callback_context();

    switch ( context )
    {
    case CONTEXT_CELL:                                  // A table event occurred on a cell
        {
            switch ( Fl::event() )                            // see what FLTK event caused it
            {
            case FL_PUSH:                                   // mouse click?
                done_editing();                               // finish editing previous
                take_focus();
                if ( Fl::event_clicks() > 0 )                // edit on double click
                    start_editing( R, C );                      // start new edit
                return;

            case FL_KEYBOARD:                               // key press in table?

                if ( ( Fl::event_state() & ( FL_CTRL | FL_COMMAND ) ) && Fl::event_key() == 'c' )
                {
                    copy( 1 );
                    return;
                }
                if ( ( Fl::event_state() & ( FL_CTRL | FL_COMMAND ) ) && Fl::event_key() == 'a' )
                {
                    set_selection( 0, 0, rows() - 1, cols() - 1 );
                    return;
                }
                if ( ( Fl::event_state() & ( FL_CTRL | FL_COMMAND ) ) && Fl::event_key() == 'v' )
                {
                    Fl::paste( *this, 1 );
                    return;
                }

                // For all 'normal' characters.
                if ( ( Fl::e_text[0] >= ' ' && Fl::e_text[0] <= '~' ) )
                {
                    start_editing( R, C );                       // start new edit
                    input->handle( Fl::event() );                // pass typed char to input
                    return;
                }

                switch ( Fl::event_key() )
                {
                    case FL_Enter:                                   // let enter key edit the cell
                        start_editing( R, C );                       // start new edit
                        break;
                    default:
                        input->handle( Fl::event() );            // pass other keys on without new edit.
                        break;
                }

                return;
            }
            return;
        }

    case CONTEXT_TABLE:                                 // A table event occurred on dead zone in table
    case CONTEXT_ROW_HEADER:                            // A table event occurred on row/column header
    case CONTEXT_COL_HEADER:
        done_editing();                                   // done editing, hide
        return;

    default:
        return;
    }
}

template < typename T >
int SpreadSheet<T>::copy( int clipboard )
{
    int rstart, cstart;
    int rend, cend;

    get_selection( rstart, cstart, rend, cend );

    if ( rstart != -1 && cstart != -1 )
    {
        string cp_str;
        for ( int r = rstart; r <= rend; r++ )
        {
            int c;
            for ( c = cstart; c < cend; c++ )
            {
                cp_str.append( get_exact_value( r, c ) );
                cp_str.append( "\t" );
            }
            cp_str.append( get_exact_value( r, c ) );

            if ( r < rend )
            {
                cp_str.append( "\n" );
            }
        }

        Fl::copy( cp_str.c_str(), cp_str.size(), clipboard );

        return 1;
    }
    return 0;
}

template < typename T >
void SpreadSheet<T>::paste( const char *str, int len )
{
    int datarow = get_data_nrow();
    int datacol = get_data_ncol();

    vector < vector < string > > table;
    StringUtil::parse_table( str, len, table );

    int trow = table.size();
    if ( trow > 0 )
    {
        bool change = false;
        if ( trow + select_row > datarow )
        {
            change = true;
            datarow = trow + select_row;

            if ( !m_AllowPasteGrowRows )
            {
                fl_beep( FL_BEEP_ERROR );
                return;
            }
        }

        int tcol = table[ 0 ].size();
        if ( tcol > 0 )
        {
            if ( tcol + select_col > datacol )
            {
                change = true;
                datacol = tcol + select_col;

                if ( !m_AllowPasteGrowCols )
                {
                    fl_beep( FL_BEEP_ERROR );
                    return;
                }
            }

            if ( change )
            {
                update_size( datarow, datacol );
            }

            for ( int i = 0; i < table.size(); i++ )
            {
                int idata = i + select_row;
                for ( int j = 0; j < table[ i ].size(); j++ )
                {
                    int jdata = j + select_col;

                    set_value( idata, jdata, table[ i ][ j ].c_str() );
                }
            }

            damage_zone( select_row, select_col, select_row + trow - 1, select_col + tcol - 1 );

            if ( m_ChangeCallback )
            {
                m_ChangeCallback ( this, m_ChangeCallbackData );
            }
        }
    }
}

template < typename T >
void SpreadSheet<T>::update_size()
{
    int r = rows();
    int c = cols();
    rows( get_data_nrow() );
    cols( get_data_ncol() );

    for ( int i = r; i < get_data_nrow(); i++ )
    {
        row_height( i, m_DefHeight );
    }

    col_width( 0, m_DefWidth );

    for ( int i = c; i < get_data_ncol(); i++ )
    {
        col_width( i, m_DefWidth );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


template < typename T >
string SpreadSheet< T >::get_value( int R, int C )
{
    return to_string( (*m_Data)[ R ] );
}

template < typename T >
string SpreadSheet< T >::get_exact_value( int R, int C )
{
    return get_value( R, C );
}

template < typename T >
int SpreadSheet< T >::get_data_ncol()
{
    if ( get_data_nrow() > 0 )
    {
        return 1;
    }
    return 0;
};

template < typename T >
void SpreadSheet< T >::update_size( int nrow, int ncol )
{
    bool change = false;

    if ( get_data_nrow() != nrow )
    {
        (*m_Data).resize( nrow );
        change = true;
    }

    if ( change )
    {
        update_size();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef SpreadSheet< int > SpreadColumnInt;

template <>
void SpreadSheet< int >::set_value( int R, int C, const char * str );

template <>
string SpreadSheet< int >::get_value( int R, int C );

///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef SpreadSheet< double > SpreadColumnDouble;

template <>
void SpreadSheet< double >::set_value( int R, int C, const char * str );

template <>
string SpreadSheet< double >::get_value( int R, int C );

template <>
string SpreadSheet< double >::get_exact_value( int R, int C );

///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef SpreadSheet< string > SpreadColumnString;

template <>
void SpreadSheet< string >::set_value( int R, int C, const char * str );

template <>
string SpreadSheet< string >::get_value( int R, int C );

///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef SpreadSheet< vec3d > SpreadColumnVec3d;

template <>
void SpreadSheet< vec3d >::set_value( int R, int C, const char * str );

template <>
string SpreadSheet< vec3d >::get_value( int R, int C );

template <>
string SpreadSheet< vec3d >::get_exact_value( int R, int C );

template <>
int SpreadSheet< vec3d >::get_data_ncol();

template <>
void SpreadSheet< vec3d >::update_size( int nrow, int ncol );

///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef SpreadSheet< vector < double > > SpreadSheetDouble;

template <>
void SpreadSheet< vector < double > >::set_value( int R, int C, const char * str );

template <>
string SpreadSheet< vector < double > >::get_value( int R, int C );

template <>
string SpreadSheet< vector < double > >::get_exact_value( int R, int C );

template <>
int SpreadSheet< vector < double > >::get_data_ncol();

template <>
void SpreadSheet< vector < double > >::update_size( int nrow, int ncol );

#endif // SPREADSHEET_WIDGET_INCLUDED_
