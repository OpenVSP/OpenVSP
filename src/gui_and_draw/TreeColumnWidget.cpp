//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// TreeColumnWidget.h: a modified Fl_Tree widget based on the code provided at Erco's
//             FLTK Cheat Page: http://seriss.com/people/erco/fltk/#Fl_Resize_Browser
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#include "TreeColumnWidget.h"

TreeRowItem::TreeRowItem( Fl_Tree *tree, const char *text ) : Fl_Tree_Item( tree )
{
    this->label( text );
    m_LeftMargin = 5;
}

int TreeRowItem::draw_item_content( int render_flag )
{
    TreeWithColumns *treewc = ( TreeWithColumns* )tree();
    Fl_Color fg = drawfgcolor();
    Fl_Color bg = drawbgcolor();
    const int *m_ColWidths = treewc->column_widths();

    // Our item's label dimensions
    int X = label_x(), Y = label_y(),
        W = label_w(), H = label_h(),
        RX = treewc->x() - treewc->hposition() + treewc->first_column_minw(),
        RY = Y+H-fl_descent();

    // Render background
    if ( render_flag )
    {
        if ( is_selected() )
        {
            fl_draw_box( prefs().selectbox(), X, Y, W, H, bg );
        }
        else
        {
            fl_color( bg );
            fl_rectf( X, Y, W, H );
        }
        fl_font( labelfont(), labelsize() );
    }

    // Render columns
    if ( render_flag )
    {
        // Erco says "remember to fl_pop_clip after draw/calculate..."
        fl_push_clip( X, Y, W, H );
    }

    {
        // Draw each column or calculate width of row for return.
        int t=0;
        const char *s = label();
        char delim_str[2] = { treewc->column_char(), 0 };  // strcspn() wants a char[]
        while ( *s )
        {
            int n = strcspn( s , delim_str ); // find index to next delimiter char in 's' (or eos if none)

            if ( n > 0 && render_flag )
            {
                int XX = ( t == 0 ) ? X : RX;

                // col width clipping
                int CW = m_ColWidths[t];

                // if first column, clip to edge of next col
                if ( t == 0 )
                {
                    CW = ( RX + m_ColWidths[0] ) - XX;
                }

                // If last column, clip to right edge of widget
                if ( *( s + n ) == 0 )
                {
                    CW = ( x() + w() - XX );
                }

                // Draw the text
                fl_color( fg );
                fl_push_clip( XX, Y, CW, H ); // prevent text from running into next column
                fl_draw( s, n, XX + m_LeftMargin, RY );
                fl_pop_clip(); // clip off

                // Draw vertical lines for all columns except first
                if ( t > 0 && treewc->column_separators() )
                {
                    fl_color( FL_BLACK );
                    fl_line( RX , Y , RX , Y + H );
                }
            }
            if ( *( s + n ) == treewc->column_char() )
            {
                s += n+1;                // skip field + delim
                RX += m_ColWidths[ t++ ]; // keep track of fixed column widths for all except right column
                continue;
            }
            else
            {
                // Last field? Return entire length of unclipped field
                RX += fl_width( s ) + m_LeftMargin;
                s += n;
            }
        }
    }

    if ( render_flag )
    {
        fl_pop_clip();
    }

    // return right most edge of what we've rendered
    return RX;
}

TreeWithColumns::TreeWithColumns( int X, int Y, int W, int H, const char *L ) : Fl_Tree( X, Y, W, H, L )
{
    m_ColSepFlag   = true;
    m_ResizeFlag  = true;
    m_ColChar       = '\t';
    m_ColWidths     = 0;
    m_FirstColMinW = 80;
    m_DragCol       = -1;
    m_LastCursor    = FL_CURSOR_DEFAULT;
    m_MinColWidth  = 2;

    // We need the default tree icons on all platforms.
    //    For some reason someone made Fl_Tree have different icons on the Mac,
    //    which doesn't look good for this application, so we force the icons
    //    to be consistent with the '+' and '-' icons and dotted connection lines.
    connectorstyle( FL_TREE_CONNECTOR_DOTTED );
    openicon( new Fl_Pixmap( tree_open_xpm ) );
    closeicon( new Fl_Pixmap( tree_close_xpm ) );
}

int TreeWithColumns::column_near_mouse()
{
    // exit if event not in browser area
    if ( !Fl::event_inside( _tix, _tiy, _tiw, _tih ) )
    {
        return -1;
    }

    int mousex = Fl::event_x() + hposition();
    int colx = x() + m_FirstColMinW;

    int colgrab = 4;

    // check which col divider, if any, are close to the mouse
    for ( int t = 0; m_ColWidths[t]; t++ )
    {
        colx += m_ColWidths[t];
        int diff = mousex - colx;
        // return col# if mouse near cols
        if ( diff >= -colgrab && diff <= colgrab )
        {
            return t;
        }
    }

    return -1;
}

void TreeWithColumns::change_cursor( Fl_Cursor newcursor )
{
    if ( newcursor == m_LastCursor )
    {
        return;
    }

    window()->cursor( newcursor );
    m_LastCursor = newcursor;
}

TreeRowItem* TreeWithColumns::AddRow( const char *s, TreeRowItem *parent_item )
{
    TreeRowItem *item = new TreeRowItem( this, s );

    // if called without a parent item argument, check root assignment
    if ( !parent_item )
    {
        // if TreeWithColumns has no root assigned or is on default root, designate this new item as root
        if ( !root() || ( strcmp( root()->label(), "ROOT" ) == 0 ) )
        {
            this->root( item );

            // Special colors for root item -- this is the "header"
            item->labelfgcolor( 0xffffff00 );
            item->labelbgcolor( 0x8888ff00 );

            // return created TreeRowItem
            return item;
        }
        else
        {
            // use root as parent
            parent_item = ( TreeRowItem* )root();
        }
    }

    // if parent item is in args, attach new item to it.
    parent_item->add( prefs(), "", item );

    // return created TreeRowItem
    return item;
}

void TreeWithColumns::GetSelectedItems( vector < TreeRowItem* > *item_vec )
{
    Fl_Tree_Item_Array base_array;

    get_selected_items( base_array );

    for( int i = 0; i != base_array.total(); ++i )
    {
        TreeRowItem* treeRowItem = static_cast < TreeRowItem* >( base_array[ i ] );
        if ( treeRowItem )
        {
            item_vec->push_back( treeRowItem );
        }
    }
}

int TreeWithColumns::handle( int e )
{
    //exit handle method if not resizing
    if ( !m_ResizeFlag )
    {
        return Fl_Tree::handle( e );
    }

    // Handle column resizing
    int ret = 0;
    switch ( e )
    {
        case FL_ENTER:
            ret = 1;
            break;
        case FL_MOVE:
            change_cursor( ( column_near_mouse() >= 0 ) ? FL_CURSOR_WE : FL_CURSOR_HAND );
            ret = 1;
            break;
        case FL_PUSH:
        {
            int whichcol = column_near_mouse();
            if ( whichcol >= 0 )
            {
                // drag a column if mouse close
                m_DragCol = whichcol;
                change_cursor( FL_CURSOR_DEFAULT );

                // eclipse event from Fl_Tree's handle()
                return 1;
            }
            break;
        }
        case FL_DRAG:
            if ( m_DragCol != -1 )
            {
                // Sum up column widths to determine position
                int mousex = Fl::event_x() + hposition();
                int w_new = mousex - ( x() + first_column_minw() );

                for ( int t = 0; m_ColWidths[t] && t < m_DragCol; t++ )
                {
                    w_new -= m_ColWidths[t];
                }

                // ensure w_new is at least minColWidth
                w_new = ( m_MinColWidth > w_new ) ? m_MinColWidth: w_new;

                // Apply new width, redraw interface
                m_ColWidths[ m_DragCol ] = w_new;

                recalc_tree();
                redraw();

                // eclipse event from Fl_Tree's handle()
                return 1;
            }
            break;
        case FL_LEAVE:
        case FL_RELEASE:
            // ensure normal cursor
            change_cursor( FL_CURSOR_DEFAULT );

            // release during drag mode?
            if ( m_DragCol != -1 )
            {
                // disable drag mode and eclipse event from Fl_Tree's handle()
                m_DragCol = -1;
                return 1;
            }

            ret = 1;
            break;
    }
    return( Fl_Tree::handle(e) ? 1 : ret );
}
