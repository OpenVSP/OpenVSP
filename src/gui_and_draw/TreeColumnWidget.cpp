//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// TreeColumnWidget.h: a modified Fl_Tree widget based on the code provided at Erco's
//             FLTK Cheat Page: http://seriss.com/people/erco/fltk/#Fl_Resize_Browser
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#include "ScreenBase.h"
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

    m_PopupInput = nullptr;
    m_PopupGroup = nullptr;
    m_ParentDevice = nullptr;

    m_PopupDrawFlag = false;
    m_RecentPopup = false;

    m_HotKeyFlag = true;
    m_DoubleClickFlag = true;
    m_CBReason = TREE_CALLBACK_UNKNOWN;
}

void TreeWithColumns::GetAllChildren( vector < TreeRowItem* > & child_vec )
{
    for ( Fl_Tree_Item *tree_item = first(); tree_item; tree_item = next( tree_item ) )
    {
        TreeRowItem* tree_row_item = dynamic_cast< TreeRowItem * >( tree_item );
        if ( tree_row_item )
        {
            child_vec.push_back( tree_row_item );
        }
    }
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

TreeRowItem* TreeWithColumns::GetItemByRefId( const string & attr_id, const string & coll_id, const vector < string > & id_vec )
{
    TreeRowItem* ret_item = nullptr;

    for ( Fl_Tree_Item *tree_item = first(); tree_item; tree_item = next(tree_item) )
    {
        TreeRowItem* tree_row_item = dynamic_cast< TreeRowItem* >( tree_item );
        if ( tree_row_item )
        {
            if ( !attr_id.empty() && tree_row_item->GetRefAttrID() == attr_id )
            {
                return tree_row_item;
            }
            else if ( !coll_id.empty() && tree_row_item->GetRefCollID() == coll_id )
            {
                return tree_row_item;
            }
            else if ( !id_vec.empty() && !CheckVecMatch( tree_row_item->GetRefVecID(), id_vec ) )
            {
                return tree_row_item;
            }
        }
    }

    return ret_item;
}

TreeRowItem* TreeWithColumns::GetItemByAttrId( const string & attr_id )
{
    return GetItemByRefId( attr_id, string(), vector<string>() );
}

TreeRowItem* TreeWithColumns::GetItemByCollId( const string & coll_id )
{
    return GetItemByRefId( string(), coll_id, vector<string>() );
}

TreeRowItem* TreeWithColumns::GetItemByVecId( const vector < string > & id_vec )
{
    return GetItemByRefId( string(), string(), id_vec );
}


void TreeWithColumns::GetSelectedItems( vector < TreeRowItem* > *item_vec )
{
    Fl_Tree_Item_Array base_array;

    get_selected_items( base_array );

    for( int i = 0; i != base_array.total(); ++i )
    {
        TreeRowItem* treeRowItem = dynamic_cast < TreeRowItem* >( base_array[ i ] );
        if ( treeRowItem )
        {
            item_vec->push_back( treeRowItem );
        }
    }
}

// return pos int for failure states, 0 for perfect match
int TreeWithColumns::CheckVecMatch( const vector < string > & vec1, const vector < string > & vec2 )
{
    if ( vec1.size() != vec2.size() )
    {
        // fail type 1 = unequal length of vectors
        return 1;
    }
    string str1;
    string str2;

    for ( int i = 0; i != vec1.size(); ++i )
    {
        str1 = vec1.at(i);
        str2 = vec2.at(i);
        if ( str1.compare(str2) != 0 )
        {
            // fail type 2 = one of the internal strings doesn't match
            return 2;
        }
    }
    // if no failures detected, return 0 for exact match
    return 0;
}

void TreeWithColumns::Init( GuiDevice* device, Fl_Group* group )
{
    m_ParentDevice = device;
    m_PopupGroup = group;
    callback( StaticTreeCB, this );
}

void TreeWithColumns::InitPopupInput()
{
    if ( !m_PopupGroup )
    {
        return;
    }

    int x, y, w, h;
    GetItemDims( x, y, w, h, m_PopupID );

    m_PopupInput = new Fl_Input( x, y, w, h );
    m_PopupInput->box( FL_THIN_DOWN_BOX );
    m_PopupInput->textsize( 12 );
    m_PopupInput->when( FL_WHEN_ENTER_KEY );
    m_PopupInput->callback( StaticTreeCB, this );

    m_PopupGroup->add( m_PopupInput );
}

void TreeWithColumns::SetPopupState( bool draw_flag )
{
    if ( !m_PopupDrawFlag && draw_flag )
    {
        m_RecentPopup = true;
    }
    m_PopupDrawFlag = draw_flag;
}

void TreeWithColumns::SetPopupID( const string & attr_id )
{
    m_PopupID = attr_id;
}

void TreeWithColumns::SetPopupText( const string & text )
{
    if ( m_PopupInput )
    {
        m_PopupInput->value( text.c_str() );
    }
}

bool TreeWithColumns::GetPopupState()
{
    if ( m_PopupInput )
    {
        return m_PopupDrawFlag;
    }
    return false;
}

const string TreeWithColumns::GetPopupValue()
{
    string ret = string("");
    if ( m_PopupInput )
    {
        ret = m_PopupInput->value();
    }
    return ret;
}

void TreeWithColumns::InsertPopupInput( const string & text, const string & attr_id )
{
    // Set loc, which updates size of popup window
    SetPopupID( attr_id );

    // Build the popup if it doesn't exist yet
    if ( !m_PopupInput )
    {
        InitPopupInput();
    }

    // Set popup to show on draw
    SetPopupState( true );

    // Set popup text
    SetPopupText( text );
}

void TreeWithColumns::HidePopupInput()
{
    SetPopupState( false );
}

void TreeWithColumns::GetItemDims( int &X, int &Y, int &W, int &H, const string & attr_id, int col )
{
    TreeRowItem* tree_item = GetItemByAttrId( attr_id );
    if ( tree_item )
    {
        W = m_ColWidths[col];
        if ( col == 0 )
        {
            X = tree_item->label_x();
            Y = tree_item->label_y();
            W += x() - hposition() + first_column_minw() - X;
            H = tree_item->label_h();
        }
        else
        {
            X = x() - hposition() + first_column_minw();
            for ( int i = 0; i < col; i++ )
            {
                X += m_ColWidths[i];
            }
            Y = tree_item->label_y();
            H = tree_item->label_h();
        }
    }
}

void TreeWithColumns::TreeCB( Fl_Widget* w )
{
    // TreeCB is tied to 3 events;
    //      1. TreeWithColumns::handle()
    //      2.        Fl_Input::handle()
    //      3.         Fl_Tree::handle()
    // so the behavior must be determined here; with a member var to store the callback reason for this Browser

    // init to reason "unknown"
    m_CBReason = TREE_CALLBACK_UNKNOWN;

    if ( Fl::callback_reason() == FL_REASON_USER )
    {
        // "user" reason only possible from TreeWithColumns::handle()
        m_CBReason = TREE_CALLBACK_POPUP_OPEN;
    }
    else if ( w == m_PopupInput || GetPopupState() )
    {
        // any callback reason with an open popup will trigger a rename event
        m_CBReason = TREE_CALLBACK_POPUP_ENTER;
    }
    else if ( Fl::callback_reason() == FL_REASON_SELECTED
           || Fl::callback_reason() == FL_REASON_RESELECTED )
    {
        // if no popup visible and callback on browser, browser is selected
        m_CBReason = TREE_CALLBACK_SELECT;
    }

    window()->cursor( FL_CURSOR_DEFAULT );              // XXX: if we don't do this, cursor can disappear!
    m_ParentDevice->DeviceCB( this );
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
        case FL_KEYBOARD:
        {
            // eclipse the Fl_Tree's "enter" hotkey for opening/closing items; needs to be protected for rename callback only
            int key = Fl::event_key();
            if ( m_HotKeyFlag && (key == FL_Enter || key == FL_KP_Enter || key == FL_F + 2) )
            {
                // open popup input if hotkey supported
                if (callback())
                {
                    // set reason to "opened," then perform callback.
                    do_callback( FL_REASON_USER );
                }
                return 1;
            }
            break;
        }
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

            // handle release for popup opening/closing
            if ( m_DoubleClickFlag && Fl::event_clicks() != 0 )
            {
                // ensure the item receiving double click is the one that is open
                Fl_Tree_Item* click_item = find_clicked(0);
                if ( callback() && click_item && !click_item->event_on_collapse_icon( prefs() ) )
                {
                    // set reason to "user," then perform callback.
                    do_callback( FL_REASON_USER );
                }
                return 1;
            }
            ret = 1;
            break;
    }
    return( Fl_Tree::handle(e) ? 1 : ret );
}

void TreeWithColumns::draw()
{
    // draw browser
    Fl_Tree::draw();

    // draw popup
    if ( m_PopupInput && m_PopupDrawFlag )
    {
        int x, y, w, h;

        GetItemDims( x, y, w, h, m_PopupID );

        m_PopupInput->resize( x, y, w, h );
        m_PopupInput->redraw();

        // If popup recently shown/created, then ensure it takes focus, ignore if just scrolling/redrawing
        if ( m_RecentPopup )
        {
            m_PopupInput->activate();
            m_PopupInput->show();
            m_PopupInput->take_focus();

            m_RecentPopup = false;
        }
    }
    else if ( m_PopupInput )
    {
        m_PopupInput->hide();
        m_PopupInput->deactivate();
    }
}
