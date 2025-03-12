//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "RoutingScreen.h"
#include "ScreenMgr.h"
#include "RoutingGeom.h"
#include "APIDefines.h"

//==== Constructor ====//
RoutingScreen::RoutingScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 657 + 25, "Routing" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_RoutingPointBrowser = m_DesignLayout.AddColResizeBrowser( col_widths, 4, 100 );
    m_RoutingPointBrowser->callback( staticScreenCB, this );
    m_RoutingPointBrowser->type( FL_MULTI_BROWSER );


    m_DesignLayout.AddButton( m_AddRoutingPoint, "Add" );
    m_DesignLayout.AddButton( m_DelRoutingPoint, "Delete" );
    m_DesignLayout.AddButton( m_DelAllRoutingPoints, "Delete All" );

    m_DesignLayout.AddButton( m_SetRoutingPoint, "Set Point" );

    m_DesignLayout.AddInput( m_PtNameInput, "Name" );


    m_GeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_DesignLayout.AddGeomPicker( m_GeomPicker );

    m_DesignLayout.AddSlider( m_USlider, "U", 1.0, "%5.3f" );
    m_DesignLayout.AddSlider( m_WSlider, "W", 1.0, "%5.3f" );

    m_SelectionFlag = false;
}


//==== Show Pod Screen ====//
void RoutingScreen::Show()
{
    if ( Update() )
    {
        GeomScreen::Show();
    }
}

//==== Update Pod Screen ====//
bool RoutingScreen::Update()
{
    assert( m_ScreenMgr );

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    if ( !geom_ptr || geom_ptr->GetType().m_Type != ROUTING_GEOM_TYPE )
    {
        Hide();
        return false;
    }

    GeomScreen::Update();

    m_SymmLayout.GetGroup()->deactivate();
    m_AttachLayout.GetGroup()->deactivate();

    //==== Update Clearance Specific Parms ====//
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );
    assert( routing_ptr );

    UpdateBrowser();


    RoutingPoint* rpt = routing_ptr->GetPt( m_RoutingPointBrowserSelect );;

    if ( rpt )
    {
        m_PtNameInput.Update( rpt->GetName() );

        m_GeomPicker.SetGeomChoice( rpt->GetParentID() );

        m_USlider.Update( rpt->m_U.GetID() );
        m_WSlider.Update( rpt->m_W.GetID() );



    }
    else
    {
        m_PtNameInput.Update( "" );
        m_GeomPicker.SetGeomChoice( "" );
    }

    m_GeomPicker.Update();




    return true;
}

void RoutingScreen::UpdateBrowser()
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );


    char str[255];

    int input_h_pos = m_RoutingPointBrowser->hposition();
    int input_v_pos = m_RoutingPointBrowser->vposition();

    m_RoutingPointBrowser->clear();

    m_RoutingPointBrowser->column_char( ':' );

    snprintf( str, sizeof( str ),  "@b@.PT_NAME:@b@.PARM:@b@.GROUP:@b@.CONTAINER" );
    m_RoutingPointBrowser->add( str );
    m_RoutingPointBrowser->addCopyText( "header" );

    vector < RoutingPoint* > pt_vec = routing_ptr->GetAllPt();

    for ( int i = 0 ; i < (int)pt_vec.size() ; i++ )
    {
        snprintf( str, sizeof( str ),  "%s:::\n", pt_vec[i]->GetName().c_str() );
        m_RoutingPointBrowser->add( str );
    }

    if ( m_RoutingPointBrowserSelect >= 0 && m_RoutingPointBrowserSelect < (int)pt_vec.size() )
    {
        m_RoutingPointBrowser->select( m_RoutingPointBrowserSelect + 2 );
    }

    m_RoutingPointBrowser->hposition( input_h_pos );
    m_RoutingPointBrowser->vposition( input_v_pos );
}


//==== Non Menu Callbacks ====//
void RoutingScreen::CallBack( Fl_Widget *w )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );

    GeomScreen::CallBack( w );

    if ( w == m_RoutingPointBrowser )
    {
        m_RoutingPointBrowserSelect = m_RoutingPointBrowser->value() - 2;
    }

}

//==== Gui Device CallBacks ====//
void RoutingScreen::GuiDeviceCallBack( GuiDevice* gui_device )
{
    assert( m_ScreenMgr );

    Vehicle *veh = VehicleMgr.GetVehicle();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );

    RoutingPoint* rpt = nullptr;

    if ( routing_ptr )
    {
        rpt = routing_ptr->GetPt( m_RoutingPointBrowserSelect );
    }

    if ( gui_device == &m_AddRoutingPoint )
    {
        if ( routing_ptr )
        {
            routing_ptr->AddPt();
        }
    }
    else if ( gui_device == &m_DelRoutingPoint )
    {
        if ( routing_ptr )
        {
            routing_ptr->DelPt( m_RoutingPointBrowserSelect );
        }
    }
    else if ( gui_device == &m_DelAllRoutingPoints )
    {
        if ( routing_ptr )
        {
            routing_ptr->DelAllPt();
        }
    }
    else if ( gui_device == &m_PtNameInput )
    {
        if ( rpt )
        {
            rpt->SetName( m_PtNameInput.GetString() );
        }
    }
    else if ( gui_device == & m_GeomPicker )
    {
        if ( rpt )
        {
            rpt->SetParentID( m_GeomPicker.GetGeomChoice() );
        }
    }
    else if ( gui_device == & m_SetRoutingPoint )
    {
        m_SelectionFlag = true;
        UpdatePickList();
    }
    else
    {
        return;
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void RoutingScreen::Set( const vec3d &placement, const std::string &targetGeomId )
{
    Vehicle* veh = VehicleMgr.GetVehicle();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );

    RoutingPoint* rpt = nullptr;

    if ( routing_ptr )
    {
        rpt = routing_ptr->GetPt( m_RoutingPointBrowserSelect );
    }

    Geom * geom = nullptr;

    if ( veh )
    {
        geom = veh->FindGeom( targetGeomId );
    }


    if( rpt && geom )
    {
        rpt->SetParentID( targetGeomId );

        if ( geom->GetNumTotalSurfs() > 0 )
        {
            int index;
            double u, w;
            geom->ProjPnt01I( placement, index, u, w );

            const VspSurf * surf = geom->GetSurfPtr( index );

            double umapmax = surf->GetUMapMax();
            double umax = surf->GetUMax();

            double uprm = surf->EvalUMapping( u * umax ) / umapmax;

            if ( uprm < 0 )
            {
                uprm = u;
            }

            rpt->m_U = uprm;
            rpt->m_W = w;

            // rpt->m_OriginIndx = index;
        }
        else
        {
            rpt->m_U = 1; // Set to dummy value to trigger update.
            rpt->m_U = 0;
            rpt->m_W = 0;
            // rpt->m_OriginIndx = 0;
        }
        routing_ptr->m_Picking = false;
        routing_ptr->UpdateParents();;
        routing_ptr->Update();
    }

    m_SelectionFlag = false;
    m_ScreenMgr->SetUpdateFlag( true );

}

std::string RoutingScreen::getFeedbackGroupName()
{
    return std::string( "RoutingGeomGUIGroup" );
}

void RoutingScreen::LoadDrawObjs( vector< DrawObj* > & draw_obj_vec )
{
    if ( m_SelectionFlag )
    {
        for( int i = 0; i < ( int )m_PickList.size(); i++ )
        {
            draw_obj_vec.push_back( &m_PickList[i] );
        }
    }
}

void RoutingScreen::UpdatePickList()
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    vector< Geom* > geom_vec = veh->FindGeomVec( veh->GetGeomVec() );

    m_PickList.clear();

    for ( int i = 0; i < ( int ) geom_vec.size(); i++ )
    {
        vector<DrawObj *> geom_drawObj_vec;
        geom_vec[i]->LoadMainDrawObjs( geom_drawObj_vec );

        for ( int j = 0; j < ( int ) geom_drawObj_vec.size(); j++ )
        {
            if ( geom_drawObj_vec[j]->m_Visible )
            {
                DrawObj pickDO;
                pickDO.m_Type = DrawObj::VSP_PICK_VERTEX;
                pickDO.m_GeomID = PICKVERTEXHEADER + geom_drawObj_vec[j]->m_GeomID;
                pickDO.m_PickSourceID = geom_drawObj_vec[j]->m_GeomID;
                pickDO.m_FeedbackGroup = getFeedbackGroupName();

                m_PickList.push_back( pickDO );
            }
        }
    }

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );

    if ( routing_ptr )
    {
        // Set picking to toggle visibility.
        routing_ptr->m_Picking = true;

        vector < DrawObj *> drawobj;
        routing_ptr->LoadDrawObjs( drawobj );

        for ( int i = 0; i < drawobj.size(); i++ )
        {
            DrawObj * currDrawObj = drawobj[i];

            if ( currDrawObj->m_Type == DrawObj::VSP_ROUTING )
            {
                currDrawObj->m_Routing.LiveIndex = m_RoutingPointBrowserSelect;
            }
        }
    }
}


