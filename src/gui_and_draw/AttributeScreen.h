//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AttributeScreen.h: GUI Stuff for working with attributes
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#if !defined(VSP_ATTRSCREEN_INCLUDED_)
#define VSP_ATTRSCREEN_INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "AttributeEditor.h"
#include "ScreenMgr.h"
#include "ResultsMgr.h"

#include <FL/Fl.H>

using std::string;
using std::vector;

class AttributeExplorer: public BasicScreen
{
public:
    AttributeExplorer( ScreenMgr* mgr );
    virtual ~AttributeExplorer();

    void Show();
    bool Update( );

    void SetAttrData( );
    void AddEmptyCollID( const vector<string> &coll_ids );
    void GetEmptyColls();

    int NumAttrTypes();

    void UpdateAttrFields();

    void SetTreeAutoSelectID( const string & id );
    void AttributeAdd();
    void AttributeAdd( int attrAddType );
    void AttributeModify( GuiDevice* gui_device, Fl_Widget* w );

    template < class T >
    void ResizeVector( vector < T > *vecPtr, const int delta, T const &new_val )
    {
        if ( delta > 0 )
        {
            // if adding rows, push back blanks
            for ( int i = 0; i!= delta; ++i )
            {
                vecPtr->push_back( new_val );
            }
        }
        else if ( delta < 0 )
        {
            // if deleting, pop from back
            for ( int i = 0; i!= delta; --i )
            {
                // ensure deletion only happens if 2 or more rows exist
                if ( vecPtr->size() > 1 )
                {
                    vecPtr->pop_back();
                }
            }
        }
    }

    template < class T >
    void ResizeMat( vector < vector < T > > *matPtr, const pair < int, int > row_col_delta, T const & new_val )
    {
        int n_row = 0;
        int n_col = 0;

        // if there are any rows, get column size from first row's size
        if ( matPtr->size() )
        {
            n_row = matPtr->size();
            n_col = matPtr[0].size();
        }

        // modify existing rows
        for ( int i = 0; i != n_row; ++i )
        {
            vector < T > * rowPtr = &(matPtr->at(i));
            ResizeVector( rowPtr, row_col_delta.second, new_val );
        }

        if ( row_col_delta.first > 0 )
        {
            // if adding rows, push back blanks
            for ( int i = 0; i!= row_col_delta.first; ++i )
            {
                vector < T > new_row( n_col + row_col_delta.second, new_val );
                matPtr->push_back( new_row );
            }
        }
        else if ( row_col_delta.first < 0 )
        {
            // if deleting, pop from back
            for ( int i = 0; i!= row_col_delta.first; --i )
            {
                // ensure deletion only happens if 2 or more rows exist
                if ( matPtr->size() > 1 )
                {
                    matPtr->pop_back();
                }
            }
        }
    }

    template < class T >
    pair < int, int > GetMatSize( vector < vector < T > > *matPtr )
    {
        int n_row = matPtr->size();
        int n_col = 0;

        // if there are any rows, get column size from first row's size
        if ( n_row )
        {
            n_col = matPtr->at(0).size();
        }
        return { n_row, n_col };
    }

    virtual void CallBack( Fl_Widget *w );

    void GuiDeviceCallBack( GuiDevice* gui_device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( AttributeExplorer* )data )->CallBack( w );
    }

    void AttrTypeDispGroup( int attr_type, GroupLayout * group );

protected:

    Choice m_AttrTypeSearchChoice;
    Choice m_ObjTypeSearchChoice;
    StringInput m_AttrSearchIn;
    BoolParm* m_CaseSensParmPtr;
    ToggleButton m_CaseSensitiveButton;
    TriggerButton m_CloseTrigger;
    TriggerButton m_OpenTrigger;

    AttributeTree m_AttrTreeWidget;

    ScreenMgr* m_ScreenMgr;

    GroupLayout m_AttrDetailLayout;

    TriggerButton m_AttrAddTrigger;
    TriggerButton m_AttrAddGroupTrigger;
    Choice m_AttrTypeChoice;

    StringInput m_AttrNameIn;
    StringInput m_AttrDescIn;

    SpreadColumnVec3d *m_Vec3dSpreadSingle;
    SpreadSheetInt *m_IntMatrixSpreadSheet;
    SpreadSheetDouble *m_DoubleMatrixSpreadSheet;

    Fl_Box* m_DataLabel;

    GroupLayout m_TreeGroupLayout;
    GroupLayout m_TreeWidgetLayout;
    GroupLayout m_CommonEntryLayout;
    GroupLayout m_ResizableLayout;

    GroupLayout m_EmptyEntryLayout;
    GroupLayout m_ToggleEntryLayout;
    GroupLayout m_InlineEntryLayout;
    GroupLayout m_StringEntryLayout;
    GroupLayout m_ParmRefEntryLayout;
    GroupLayout m_Vec3dEntryLayout;
    GroupLayout m_IntMatEntryLayout;
    GroupLayout m_DblMatEntryLayout;

    TriggerButton m_AttrVec3dRowAdd;
    TriggerButton m_AttrVec3dRowDel;

    TriggerButton m_AttrDmatRowAdd;
    TriggerButton m_AttrDmatRowDel;
    TriggerButton m_AttrDmatColAdd;
    TriggerButton m_AttrDmatColDel;

    TriggerButton m_AttrImatRowAdd;
    TriggerButton m_AttrImatRowDel;
    TriggerButton m_AttrImatColAdd;
    TriggerButton m_AttrImatColDel;

    BoolParm* m_AttrBoolParmPtr;
    StringOutput m_AttrToggleLabel;
    ToggleButton m_AttrDataToggleIn;
    StringOutput m_AttrToggleField;

    StringInput m_InlineDataIn;

    ParmPicker m_AttrParmPicker;
    StringInput m_AttrParmIDInput;
    SliderAdjRangeInput m_AttrParmSlider;

    VspTextEditor* m_DataText;
    Fl_Text_Buffer* m_DataBuffer;

    TriggerButton m_CopyButton;
    TriggerButton m_PasteButton;
    TriggerButton m_CutButton;
    TriggerButton m_DelButton;

    GroupLayout* m_CurAttrGroup;

    vector < string > m_AttrIDs;
    vector < string > m_CollIDs;

    bool m_valid_collector_set;
    int m_types_selected;

};

#endif