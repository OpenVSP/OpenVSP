//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// Created by Rob McDonald on 6/11/24.
//
//////////////////////////////////////////////////////////////////////

#ifndef VSP_TOP_RESULTSVIEWER_H
#define VSP_TOP_RESULTSVIEWER_H

#include "ScreenBase.h"

#include "SpreadSheetWidget.h"

class ResultsViewer : public BasicScreen
{
public:
    ResultsViewer( ScreenMgr* mgr );
    virtual ~ResultsViewer();

    void Show();
    void Hide();
    bool Update();
    void UpdateResultsChoice();
    void UpdateDataBrowser();

    void CallBack( Fl_Widget *w );
    virtual void CloseCallBack( Fl_Widget *w );
    static void staticScreenCB( Fl_Widget *w, void* data )
    {
        ( ( ResultsViewer* )data )->CallBack( w );
    }

    virtual void GuiDeviceCallBack( GuiDevice* device );

    void DisplayGroup( GroupLayout* group );

    void SetSelectedResult( string rid );

protected:

    GroupLayout m_MainLayout;
    GroupLayout m_BorderLayout;

    GroupLayout* m_CurrDisplayGroup;

    GroupLayout m_IntVectorLayout;
    GroupLayout m_DoubleVectorLayout;
    GroupLayout m_StringVectorLayout;
    GroupLayout m_Vec3dVectorLayout;
    GroupLayout m_DoubleMatrixLayout;


    Choice m_ResultsChoice;

    TriggerButton m_DeleteButton;
    TriggerButton m_DeleteAllButton;

    ColResizeBrowser* m_DataBrowser;

    SpreadColumnInt *m_IntVectorSpreadColumn;
    SpreadColumnDouble *m_DoubleVectorSpreadColumn;
    SpreadColumnString *m_StringVectorSpreadColumn;
    SpreadColumnVec3d *m_Vec3dVectorSpreadColumn;
    SpreadSheetDouble *m_DoubleMatrixSpreadSheet;


    vector < string > m_ResIDvec;

    vector < pair < string, int > > m_DataNameIndexVec;


    int m_ResultsViewerSelect;
    int m_DataBrowserSelect;
};


#endif //VSP_TOP_RESULTSVIEWER_H
