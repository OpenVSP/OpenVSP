//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AttributeEditor.h: Extensive attribute editor screen.
// Tim Cuatt
//
//////////////////////////////////////////////////////////////////////

#if !defined(ATTR_EDITOR_)
#define ATTR_EDITOR_

#include "GroupLayout.h"
#include "AttributeManager.h"
#include "TreeColumnWidget.h"

class ScreenMgr;

using std::string;

//==== Attribute Tree ====//
class AttributeTree : public GuiDevice
{
public:
    AttributeTree();
    void Init( GroupLayout * layout, Fl_Group* group, VspScreen *screen, Fl_Callback *cb, bool mod_start = false, int start_y = 0, int browser_h = 150 );
    void Activate();
    void Deactivate();
    void Update();
    void UpdateTree();
    void SetRedrawFlag()
    {
        m_RedrawFlag = true;
    }
    void ClearRedrawFlag()
    {
        m_RedrawFlag = false;
    }
    void SetTreeRootID( const string & attrCollectionID = "" );
    void SetTreeAttrID();
    void AddEmptyCollID( const string & coll_id );
    void AddEmptyCollID( const vector < string > & coll_ids );
    void SetEmptyCollID( const vector < string > & coll_ids );

    void SetSearchTerms( int attr_type, int attach_type,const string & attr_str, bool case_sens );

    bool ItemState( const vector < string > & local_id_vec );
    bool ItemState( TreeRowItem* tree_item );

    void SetState( const vector < string > & local_id_vec, bool open_state );
    void ToggleState( const vector < string > & local_id_vec );
    void SetAllNodes( bool open_state );
    void TrimCloseVec();

    void ClearExtraIDs();

    void SetAutoSelectID( const vector < string > & ids );

    vector < string > GetSelectedID()
    {
        vector < string > ids;
        for ( int i = 0; i != m_SelectIDs.size(); ++i )
        {
            if ( !m_SelectIDs.at( i ).empty() )
            {
                ids.push_back( m_SelectIDs.at( i ).back() );
            }
        }
        return ids;
    }

    vector < string > GetTreeAttrID()
    {
        return m_AttrIDs;
    }

    vector < string > GetTreeCollID()
    {
        return m_CollIDs;
    }

    string GetTreeRootCollID()
    {
        return m_AttrRootCollID;
    }

    Fl_Tree* GetTreeWidget()
    {
        return m_AttrTree;
    }

    void DeviceCB( Fl_Widget *w );

protected:

    vector < string > m_AutoSelectIDs;

    virtual void SetValAndLimits( Parm* p )                      {} // Do Nothing


    int m_ColWidths[4];

    int m_FilterAttrType;
    int m_FilterObjType;
    string m_FilterStr;
    bool m_FilterCase;

    GroupLayout m_TreeGroup;

    vector < vector < string > > m_SelectIDs; //build select ID from positively-ID'd anchor point (anything with an actual ID) and vector path from that one.
    vector < string > m_AttrIDs;
    vector < string > m_CollIDs;
    string m_AttrRootCollID;

    vector < string > m_OpenBranchUpdate;
    vector < string > m_OpenBranchVec;

    vector < vector < string > > m_CloseVec;  // id vector of open/closed local node vectors

    TreeWithColumns* m_AttrTree;

    bool m_RedrawFlag;

private:

    vector < TreeRowItem* > m_TreeItemArray;

};

class AttributeEditor : public GuiDevice
{
public:
    // Initialize the member GUI devices and set their callbacks
    void Init( GroupLayout * layout, Fl_Group* group, VspScreen *screen, Fl_Callback *cb, bool mod_start = false, int start_y = 0, int browser_h = 150 );
    void Show();
    void Update();
    void SetEditorCollID( string collID = "" );
    string GetAttrCollID()
    {
        return m_AttrCollID;
    }

    void DeviceCB( Fl_Widget *w );
    void GuiDeviceCallBack( GuiDevice* gui_device );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( AttributeEditor* )data )->DeviceCB( w );
    }

    static bool canMakeInt( const string& str );
    static bool canMakeDbl( const string& str );

    virtual void SetValAndLimits(){};

protected:

    virtual void SetValAndLimits( Parm* p )                      {} // Do Nothing

    AttributeTree m_AttrTreeWidget;

    GroupLayout m_AttrCommonGroup;
    GroupLayout m_AttrStrGroup;
    GroupLayout m_AttrToggleGroup;

    bool m_ShowState;

private:

    string m_AttrCollID;

};

#endif