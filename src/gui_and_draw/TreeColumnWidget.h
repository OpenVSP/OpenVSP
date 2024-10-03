//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// TreeColumnWidget.h: a modified Fl_Tree widget based on the code provided at Erco's
//             FLTK Cheat Page: http://seriss.com/people/erco/fltk/#Fl_Resize_Browser
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#if !defined(TREECOL_WIDGET_INCLUDED_)
#define TREECOL_WIDGET_INCLUDED_

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>

#include <vector>

using namespace std;

//==== Tree With Columns ====//
const char * const tree_open_xpm[] = {
  "11 11 3 1",
  ".    c #fefefe",
  "#    c #444444",
  "@    c #000000",
  "###########",
  "#.........#",
  "#.........#",
  "#....@....#",
  "#....@....#",
  "#..@@@@@..#",
  "#....@....#",
  "#....@....#",
  "#.........#",
  "#.........#",
  "###########"
};

const char * const tree_close_xpm[] = {
  "11 11 3 1",
  ".    c #fefefe",
  "#    c #444444",
  "@    c #000000",
  "###########",
  "#.........#",
  "#.........#",
  "#.........#",
  "#.........#",
  "#..@@@@@..#",
  "#.........#",
  "#.........#",
  "#.........#",
  "#.........#",
  "###########"
};

// DERIVE CUSTOM CLASS FROM Fl_Tree_Item TO SHOW DATA IN COLUMNS
class TreeRowItem : public Fl_Tree_Item
{
public:
    TreeRowItem( Fl_Tree *tree, const char *text );
    int draw_item_content( int render );

    void SetLeftMargin( const int & left_margin )
    {
        m_LeftMargin = left_margin;
    }

    int GetLeftMargin()
    {
       return m_LeftMargin;
    }

protected:

    int m_LeftMargin;
};

// Small convenience class to handle adding columns.
//    TreeRowItem does most of the work.

class TreeWithColumns : public Fl_Tree
{
    protected:
        int column_near_mouse();
        void change_cursor( Fl_Cursor newcursor );

        bool m_ColSepFlag;
        bool m_ResizeFlag;
        char m_ColChar;
        int *m_ColWidths;
        int m_FirstColMinW;
        int m_DragCol;
        int m_MinColWidth;
        Fl_Cursor m_LastCursor;

    public:
       TreeWithColumns( int X, int Y, int W, int H, const char *L = 0 );

        int  first_column_minw()
        {
            return m_FirstColMinW;
        }
        void first_column_minw( int val )
        {
            m_FirstColMinW = val;
        }

        // Enable/disable the vertical column lines
        void column_separators( bool val )
        {
            m_ColSepFlag = val;
        }
        bool column_separators() const
        {
            return m_ColSepFlag;
        }

        void resizing( bool val )
        {
            m_ResizeFlag = val;
        }
        bool resizing() const
        {
            return m_ResizeFlag;
        }

        // Change the column delimiter character
        void column_char( char val )
        {
            this->m_ColChar = val;
        }
        char column_char() const
        {
            return m_ColChar;
        }

        // Set the column array.
        //     Make sure the last entry is zero.
        //     User allocated array must remain allocated for lifetime of class instance.
        //     Must be large enough for all columns in data!
        void column_widths( int *val )
        {
            this->m_ColWidths = val;
        }
        int *column_widths() const
        {
            return m_ColWidths;
        }

        TreeRowItem *AddRow( const char *s, TreeRowItem *parent_item = nullptr );

        void GetSelectedItems( vector < TreeRowItem* > *item_vec );

        // Manage column resizing
        int handle( int e );

        // Hide these base class methods from the API; we don't want app using them,
        // as we expect all items in the tree to be TreeRowItems, not Fl_Tree_Items.
    private:
        using Fl_Tree::add;
};

// Handle custom drawing of the item
//
//    All we're responsible for is drawing the 'label' area of the item
//    and it's background. Fl_Tree gives us a hint as to what the
//    foreground and background colors should be via the fg/bg parameters,
//    and whether we're supposed to render anything or not.
//
//    The only other thing we must do is return the maximum X position
//    of scrollable content, i.e. the right most X position of content
//    that we want the user to be able to use the horizontal scrollbar
//    to reach.
//


#endif // TREECOL_WIDGET_INCLUDED_
