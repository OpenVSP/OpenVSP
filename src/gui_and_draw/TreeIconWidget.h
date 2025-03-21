//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// TreeIconWidget.h: a modified Fl_Tree widget based on the code provided at Erco's
//             FLTK Cheat Page: https://www.seriss.com/people/erco/fltk/#TreeWithClickIcons
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#if !defined(TREEICON_WIDGET_INCLUDED_)
#define TREEICON_WIDGET_INCLUDED_

#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_SVG_Image.H>

#include "APIDefines.h"

#include <map>
#include <list>
#include <vector>
#include <string>

using namespace std;
const char * const mini_tree_open_xpm[] = {
    "9 9 3 1",
    ".    c #fefefe",
    "#    c #444444",
    "@    c #000000",
    "#########",
    "#.......#",
    "#...@...#",
    "#...@...#",
    "#.@@@@@.#",
    "#...@...#",
    "#...@...#",
    "#.......#",
    "#########"
};

const char * const mini_tree_close_xpm[] = {
    "9 9 3 1",
    ".    c #fefefe",
    "#    c #444444",
    "@    c #000000",
    "#########",
    "#.......#",
    "#.......#",
    "#.......#",
    "#.@@@@@.#",
    "#.......#",
    "#.......#",
    "#.......#",
    "#########"
};

enum TREE_LINE_CONN {
    STYLE_NONE=0,     ///< Use no lines connecting items
    STYLE_DOTTED=1,   ///< Use dotted lines connecting items (default)
    STYLE_SOLID=2,    ///< Use solid lines connecting items
    STYLE_THICK=3,    ///< Use 3px wide lines connecting items
    STYLE_DOUBLE=4,   ///< Use two lines connecting items
  };

//
// Fl_Tree clickable custom item icons with right justification
// erco 11/28/21
//

class TreeIconItem : public Fl_Tree_Item
{
    public:
        TreeIconItem( Fl_Tree* tree, const char* name );
        int IconHandle();

        void IndexIcon();

        int GetIconEvent()
        {
            return m_EventType;
        }

        int EventInShow();
        int EventInSurf();

        void SetIndetShowState( bool state )
        {
            m_IndetShowState = state;
        }
        void SetIndetSurfState( bool state )
        {
            m_IndetSurfState = state;
        }

        void SetShowState( bool state );
        bool GetShowState()
        {
            return m_ShowState;
        }
        void SetSurfState( int surf );
        int GetSurfState()
        {
            return m_SurfState;
        }

        void SetRefID( const string & ref_id )
        {
            m_RefID = ref_id;
        }
        string GetRefID()
        {
            return m_RefID;
        }

        void SetHConnLine( int conn_line )
        {
            m_HConnLine = conn_line;
        }
        void SetVConnLine( int conn_line )
        {
            m_VConnLine = conn_line;
        }
        void SetChildVConnLine( int conn_line )
        {
            m_ChildVConnLine = conn_line;
        }

        void SetHConnLineColor( Fl_Color color )
        {
            m_HConnLineColor = color;
        }

        int CheckParent( TreeIconItem* parent_item );

    protected:
        int draw_item_content( int render );
        void draw_horizontal_connector( int x1, int x2, int y, const Fl_Tree_Prefs &prefs );
        void draw_vertical_connector( int x1, int x2, int y, const Fl_Tree_Prefs &prefs );
        int event_inside( int v[4] );
        void update_icon();

        int m_ShowIconXy[4];
        int m_SurfIconXy[4];

        Fl_SVG_Image* m_ShowIconSvg;
        Fl_SVG_Image* m_SurfIconSvg;

        Fl_SVG_Image* m_SurfSvgVec[5];

        bool m_IndetShowState;
        bool m_IndetSurfState;

        bool m_ShowState;
        int m_SurfState;
        int m_EventType;

        bool m_CollapseIcon;
        int m_HConnLine;
        int m_VConnLine;
        int m_ChildVConnLine;

        Fl_Color m_HConnLineColor;

        string m_RefID;
};

class TreeWithIcons : public Fl_Tree
{
    public:
        TreeWithIcons( int X, int Y, int W, int H, const char *L = 0 );

        TreeIconItem *AddRow( const char *s, TreeIconItem *parent_item = nullptr );

        TreeIconItem* GetItemByRefId( const string & ref_id );
        void GetSelectedItems( vector < TreeIconItem* > *item_vec );

        int handle( int e );

        // Hide these base class methods from the API; we don't want app using them,
        // as we expect all items in the tree to be TreeIconItems, not Fl_Tree_Items.
    private:
        using Fl_Tree::add;
};

#endif // TREEICON_WIDGET_INCLUDED_