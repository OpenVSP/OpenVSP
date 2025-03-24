//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "RoutingScreen.h"
#include "ScreenMgr.h"
#include "RoutingGeom.h"
#include "WingGeom.h"
#include "APIDefines.h"

//==== Constructor ====//
RoutingScreen::RoutingScreen( ScreenMgr* mgr ) : GeomScreen( mgr, 400, 800, "Routing" )
{
    Fl_Group* design_tab = AddTab( "Design" );
    Fl_Group* design_group = AddSubGroup( design_tab, 5 );

    m_DesignLayout.SetGroupAndScreen( design_group, this );
    m_DesignLayout.AddDividerBox( "Design" );

    //==== Design ====//

    int movebw = 20;
    int browser_h = 150;
    int start_x = m_DesignLayout.GetX();
    int start_y = m_DesignLayout.GetY();

    m_DesignLayout.AddSubGroupLayout( m_MovePointLayout, 20, browser_h );

    m_MovePointLayout.SetSameLineFlag( false );
    m_MovePointLayout.SetFitWidthFlag( false );

    m_MovePointLayout.SetButtonWidth( movebw );
    m_MovePointLayout.AddButton( m_MovePntTopButton, "@2<<" );
    m_MovePointLayout.AddYGap();
    m_MovePointLayout.AddButton( m_MovePntUpButton, "@2<" );
    m_MovePointLayout.AddY( browser_h - 4 * m_MovePointLayout.GetStdHeight() - 2 * m_MovePointLayout.GetGapHeight() );
    m_MovePointLayout.AddButton( m_MovePntDownButton, "@2>" );
    m_MovePointLayout.AddYGap();
    m_MovePointLayout.AddButton( m_MovePntBotButton, "@2>>" );

    m_DesignLayout.SetY( start_y );
    m_DesignLayout.AddX( movebw );
    m_DesignLayout.SetFitWidthFlag( true );


    m_DesignLayout.AddSubGroupLayout( m_PointBrowserLayout, m_DesignLayout.GetRemainX(), browser_h );
    m_DesignLayout.AddY( browser_h );

    // Pointer for the widths of each column in the browser to support resizing
    // Last column width must be 0
    static int col_widths[] = { 160, 85, 80, 85, 0 }; // widths for each column

    m_RoutingPointBrowser = m_PointBrowserLayout.AddColResizeBrowser( col_widths, 4, browser_h );
    m_RoutingPointBrowser->callback( staticScreenCB, this );
    m_RoutingPointBrowser->type( FL_MULTI_BROWSER );

    m_DesignLayout.SetX( start_x );

    m_DesignLayout.AddButton( m_AddRoutingPoint, "Add" );
    m_DesignLayout.AddButton( m_DelRoutingPoint, "Delete" );
    m_DesignLayout.AddButton( m_DelAllRoutingPoints, "Delete All" );

    m_DesignLayout.AddButton( m_SetRoutingPoint, "Set Point" );

    m_DesignLayout.AddButton( m_AddMultipleRoutingPoints, "Add Multiple" );
    m_DesignLayout.AddButton( m_StopAdding, "Stop Adding" );

    m_DesignLayout.AddButton( m_InsertRoutingPoint, "Insert Before" );

    m_DesignLayout.AddButton( m_InsertMultipleRoutingPoints, "Insert Multiple" );



    m_DesignLayout.AddYGap();

    m_DesignLayout.AddInput( m_PtNameInput, "Name" );


    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Parent" );

    m_GeomPicker.AddExcludeType( PT_CLOUD_GEOM_TYPE );
    m_DesignLayout.AddGeomPicker( m_GeomPicker );

    m_DesignLayout.AddChoice( m_SurfChoice, "Surface" );


    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Coordinate" );

    m_DesignLayout.AddChoice( m_RoutePtCoordChoice, "Coord Type" );

    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetSameLineFlag( false );

    int actionToggleButtonWidth = 35;
    int normalButtonWidth = 90;

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );

    m_DesignLayout.SetButtonWidth( actionToggleButtonWidth );
    m_DesignLayout.AddButton( m_RoutePtCoordU01Toggle, "01" );
    m_DesignLayout.AddButton( m_RoutePtCoordU0NToggle, "0N" );

    m_DesignLayout.SetFitWidthFlag( true );

    m_DesignLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_DesignLayout.AddSlider( m_RoutePtCoordUSlider, "U", 1, " %7.6f" );

    m_RoutePtCoordUScaleToggleGroup.Init( this );
    m_RoutePtCoordUScaleToggleGroup.AddButton( m_RoutePtCoordU0NToggle.GetFlButton() ); // 0 false added first
    m_RoutePtCoordUScaleToggleGroup.AddButton( m_RoutePtCoordU01Toggle.GetFlButton() ); // 1 true added second

    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.SetButtonWidth( normalButtonWidth );

    m_DesignLayout.AddSlider( m_RoutePtCoordWSlider, "W", 1, " %7.6f" );
    m_DesignLayout.AddYGap();

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );

    m_DesignLayout.SetButtonWidth( actionToggleButtonWidth );
    m_DesignLayout.AddButton( m_RoutePtCoordR01Toggle, "01" );
    m_DesignLayout.AddButton( m_RoutePtCoordR0NToggle, "0N" );

    m_DesignLayout.SetFitWidthFlag( true );

    m_DesignLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_DesignLayout.AddSlider( m_RoutePtCoordRSlider, "R", 1, " %7.6f" );

    m_RoutePtCoordRScaleToggleGroup.Init( this );
    m_RoutePtCoordRScaleToggleGroup.AddButton( m_RoutePtCoordR0NToggle.GetFlButton() ); // 0 false added first
    m_RoutePtCoordRScaleToggleGroup.AddButton( m_RoutePtCoordR01Toggle.GetFlButton() ); // 1 true added second

    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.SetButtonWidth( normalButtonWidth );

    m_DesignLayout.AddSlider( m_RoutePtCoordSSlider, "S", 1, " %7.6f" );
    m_DesignLayout.AddSlider( m_RoutePtCoordTSlider, "T", 1, " %7.6f" );
    m_DesignLayout.AddYGap();

    m_DesignLayout.SetFitWidthFlag( false );
    m_DesignLayout.SetSameLineFlag( true );

    m_DesignLayout.SetButtonWidth( actionToggleButtonWidth );
    m_DesignLayout.AddButton( m_RoutePtCoordL01Toggle, "01" );
    m_DesignLayout.AddButton( m_RoutePtCoordL0LenToggle, "0D" );

    m_DesignLayout.SetFitWidthFlag( true );

    m_DesignLayout.SetButtonWidth( normalButtonWidth - 2 * actionToggleButtonWidth );
    m_DesignLayout.AddSlider( m_RoutePtCoordLSlider, "L", 1, " %7.6f" );

    m_RoutePtCoordLScaleToggleGroup.Init( this );
    m_RoutePtCoordLScaleToggleGroup.AddButton( m_RoutePtCoordL0LenToggle.GetFlButton() ); // 0 false added first
    m_RoutePtCoordLScaleToggleGroup.AddButton( m_RoutePtCoordL01Toggle.GetFlButton() ); // 1 true added second

    m_DesignLayout.ForceNewLine();
    m_DesignLayout.SetFitWidthFlag( true );
    m_DesignLayout.SetSameLineFlag( false );

    m_DesignLayout.SetButtonWidth( normalButtonWidth );

    char eta[5];
    int indx = 0;
    indx += fl_utf8encode( 951, &eta[ indx ] ); // Greek character eta
    eta[ indx ] = 0;

    m_DesignLayout.AddSlider( m_RoutePtCoordEtaSlider, eta, 1, " %7.6f" );

    m_DesignLayout.AddSlider( m_RoutePtCoordMSlider, "M", 1, " %7.6f" );
    m_DesignLayout.AddSlider( m_RoutePtCoordNSlider, "N", 1, " %7.6f" );


    m_DesignLayout.AddYGap();
    m_DesignLayout.AddDividerBox( "Offset" );




    m_SelectionFlag = false;
    m_AddMultipleFlag = false;
    m_InsertMultipleFlag = false;
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
    Vehicle* veh = VehicleMgr.GetVehicle();

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

    if ( routing_ptr )
    {

        UpdateBrowser();


        RoutingPoint* rpt = routing_ptr->GetPt( routing_ptr->m_ActivePointIndex );;

        if ( rpt )
        {

            bool wing_parent = false;
            if ( veh )
            {
                Geom* parent = veh->FindGeom( rpt->GetParentID() );

                if ( parent )
                {
                    WingGeom* wing_ptr = dynamic_cast< WingGeom* >( parent );
                    if ( wing_ptr )
                    {
                        wing_parent = true;
                    }
                }
            }

            m_PtNameInput.Update( rpt->GetName() );

            m_GeomPicker.SetGeomChoice( rpt->GetParentID() );

            //==== Attachments ====//
            m_RoutePtCoordUSlider.Activate();
            m_RoutePtCoordUScaleToggleGroup.Activate();
            m_RoutePtCoordWSlider.Activate();
            m_RoutePtCoordRSlider.Activate();
            m_RoutePtCoordRScaleToggleGroup.Activate();
            m_RoutePtCoordSSlider.Activate();
            m_RoutePtCoordTSlider.Activate();
            m_RoutePtCoordLSlider.Activate();
            m_RoutePtCoordLScaleToggleGroup.Activate();
            m_RoutePtCoordMSlider.Activate();
            m_RoutePtCoordNSlider.Activate();
            m_RoutePtCoordEtaSlider.Activate();

            m_RoutePtCoordChoice.Update( rpt->m_CoordType.GetID() );
            m_RoutePtCoordUScaleToggleGroup.Update( rpt->m_U01Flag.GetID() );

            if ( rpt->m_U01Flag() )
            {
                m_RoutePtCoordUSlider.Update( 1, rpt->m_U.GetID(), rpt->m_U0N.GetID());
            }
            else
            {
                m_RoutePtCoordUSlider.Update( 2, rpt->m_U.GetID(), rpt->m_U0N.GetID());
            }

            m_RoutePtCoordWSlider.Update( rpt->m_W.GetID() );
            m_RoutePtCoordRScaleToggleGroup.Update( rpt->m_R01Flag.GetID() );

            if ( rpt->m_R01Flag() )
            {
                m_RoutePtCoordRSlider.Update( 1, rpt->m_R.GetID(), rpt->m_R0N.GetID());
            }
            else
            {
                m_RoutePtCoordRSlider.Update( 2, rpt->m_R.GetID(), rpt->m_R0N.GetID());
            }

            m_RoutePtCoordSSlider.Update( rpt->m_S.GetID() );
            m_RoutePtCoordTSlider.Update( rpt->m_T.GetID() );
            m_RoutePtCoordLScaleToggleGroup.Update( rpt->m_L01Flag.GetID() );

            if ( rpt->m_L01Flag() )
            {
                m_RoutePtCoordLSlider.Update( 1, rpt->m_L.GetID(), rpt->m_L0Len.GetID());
            }
            else
            {
                m_RoutePtCoordLSlider.Update( 2, rpt->m_L.GetID(), rpt->m_L0Len.GetID());
            }

            m_RoutePtCoordMSlider.Update( rpt->m_M.GetID() );
            m_RoutePtCoordNSlider.Update( rpt->m_N.GetID() );

            m_RoutePtCoordEtaSlider.Update( rpt->m_Eta.GetID() );
            if ( rpt->m_CoordType() != vsp::ROUTE_PT_UV )
            {
                m_RoutePtCoordUSlider.Deactivate();
                m_RoutePtCoordUScaleToggleGroup.Deactivate();
                m_RoutePtCoordWSlider.Deactivate();
            }

            if ( rpt->m_CoordType() != vsp::ROUTE_PT_RST )
            {
                m_RoutePtCoordRSlider.Deactivate();
                m_RoutePtCoordRScaleToggleGroup.Deactivate();
                m_RoutePtCoordSSlider.Deactivate();
                m_RoutePtCoordTSlider.Deactivate();
            }

            if ( rpt->m_CoordType() != vsp::ROUTE_PT_LMN )
            {
                m_RoutePtCoordLSlider.Deactivate();
                m_RoutePtCoordLScaleToggleGroup.Deactivate();
            }

            if ( rpt->m_CoordType() != vsp::ROUTE_PT_LMN && rpt->m_CoordType() != vsp::ROUTE_PT_EtaMN )
            {
                m_RoutePtCoordMSlider.Deactivate();
                m_RoutePtCoordNSlider.Deactivate();
            }

            if ( rpt->m_CoordType() != vsp::ROUTE_PT_EtaMN )
            {
                m_RoutePtCoordEtaSlider.Deactivate();
            }

            if ( wing_parent )
            {
            }
            else
            {
                m_RoutePtCoordEtaSlider.Deactivate();
            }



        }
        else
        {
            m_PtNameInput.Update( "" );
            m_GeomPicker.SetGeomChoice( "" );
        }

        m_GeomPicker.Update();



    }
    return true;
}

void RoutingScreen::UpdateBrowser()
{
    Vehicle* vPtr = VehicleMgr.GetVehicle();

    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );

    m_RoutePtCoordChoice.ClearItems();
    m_RoutePtCoordChoice.AddItem( "Comp", vsp::ROUTE_PT_COMP );
    m_RoutePtCoordChoice.AddItem( "UW", vsp::ROUTE_PT_UV );
    m_RoutePtCoordChoice.AddItem( "RST", vsp::ROUTE_PT_RST );
    m_RoutePtCoordChoice.AddItem( "LMN", vsp::ROUTE_PT_LMN );

    if ( routing_ptr )
    {
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

        if ( routing_ptr->m_ActivePointIndex >= 0 && routing_ptr->m_ActivePointIndex < (int)pt_vec.size() )
        {
            m_RoutingPointBrowser->select( routing_ptr->m_ActivePointIndex + 2 );
        }
        else
        {
            routing_ptr->m_ActivePointIndex = -1;
        }

        m_RoutingPointBrowser->hposition( input_h_pos );
        m_RoutingPointBrowser->vposition( input_v_pos );



        m_SurfChoice.ClearItems();
        RoutingPoint* rpt = routing_ptr->GetPt( routing_ptr->m_ActivePointIndex );;
        if ( rpt )
        {
            Geom* parent_geom = vPtr->FindGeom( rpt->GetParentID() );

            if ( parent_geom )
            {
                int nsurf = parent_geom->GetNumTotalSurfs();

                for ( int i = 0; i < nsurf; ++i )
                {
                    snprintf( str, sizeof( str ),  "Surf_%d", i );
                    m_SurfChoice.AddItem( str );
                }
                m_SurfChoice.UpdateItems();


                if( rpt->m_SurfIndx() < 0 || rpt->m_SurfIndx() >= nsurf )
                {
                    rpt->m_SurfIndx = 0;
                }
                m_SurfChoice.SetVal( rpt->m_SurfIndx() );


                WingGeom* wing_parent = dynamic_cast < WingGeom * > ( parent_geom );
                if ( wing_parent )
                {
                    char etaMN[7];
                    int indx = 0;
                    indx += fl_utf8encode( 951, &etaMN[ indx ] ); // Greek character eta
                    etaMN[ indx ] = 'M';
                    etaMN[ indx + 1 ] = 'N';
                    etaMN[ indx + 2 ] = 0;
                    m_RoutePtCoordChoice.AddItem( etaMN, vsp::ROUTE_PT_EtaMN );
                }
            }
        }
    }

    m_RoutePtCoordChoice.UpdateItems();
}


//==== Non Menu Callbacks ====//
void RoutingScreen::CallBack( Fl_Widget *w )
{
    Geom* geom_ptr = m_ScreenMgr->GetCurrGeom();
    RoutingGeom* routing_ptr = dynamic_cast< RoutingGeom* >( geom_ptr );

    GeomScreen::CallBack( w );

    if ( w == m_RoutingPointBrowser )
    {
        routing_ptr->m_ActivePointIndex = m_RoutingPointBrowser->value() - 2;
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
        rpt = routing_ptr->GetPt( routing_ptr->m_ActivePointIndex );
    }

    if ( gui_device == &m_AddRoutingPoint )
    {
        if ( routing_ptr )
        {
            routing_ptr->AddPt();
            m_LiveIndex = routing_ptr->GetNumPt() - 1;
            routing_ptr->m_ActivePointIndex = m_LiveIndex;

            routing_ptr->Update();
            m_SelectionFlag = true;
            UpdatePickList();
        }
    }
    else if ( gui_device == &m_DelRoutingPoint )
    {
        if ( routing_ptr )
        {
            routing_ptr->DelPt( routing_ptr->m_ActivePointIndex );
            routing_ptr->Update();
        }
    }
    else if ( gui_device == &m_DelAllRoutingPoints )
    {
        if ( routing_ptr )
        {
            routing_ptr->DelAllPt();
            routing_ptr->Update();
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
        if ( routing_ptr )
        {
            m_LiveIndex = routing_ptr->m_ActivePointIndex;
            m_SelectionFlag = true;
            UpdatePickList();
        }
    }
    else if ( gui_device == &m_AddMultipleRoutingPoints )
    {
        if ( routing_ptr )
        {
            routing_ptr->AddPt();
            m_LiveIndex = routing_ptr->GetNumPt() - 1;
            routing_ptr->m_ActivePointIndex = m_LiveIndex;

            routing_ptr->Update();
            m_SelectionFlag = true;
            m_AddMultipleFlag = true;
            UpdatePickList();
        }
    }
    else if ( gui_device == & m_StopAdding )
    {
        if ( routing_ptr )
        {
            // Delete not-yet placed point.
            routing_ptr->DelPt( m_LiveIndex );
            routing_ptr->m_Picking = false;
            routing_ptr->Update();
            routing_ptr->m_ActivePointIndex = m_LiveIndex - 1;
            m_LiveIndex = -1;
        }
        m_SelectionFlag = false;
        m_AddMultipleFlag = false;
        m_InsertMultipleFlag = false;
    }
    else if ( gui_device == &m_InsertRoutingPoint )
    {
        if ( routing_ptr )
        {
            if ( routing_ptr->m_ActivePointIndex < 0 )
            {
                routing_ptr->m_ActivePointIndex = 0;
            }
            m_LiveIndex = routing_ptr->m_ActivePointIndex;
            routing_ptr->InsertPt( m_LiveIndex );

            routing_ptr->Update();
            m_SelectionFlag = true;
            UpdatePickList();
        }
    }
    else if ( gui_device == &m_InsertMultipleRoutingPoints )
    {
        if ( routing_ptr )
        {
            if ( routing_ptr->m_ActivePointIndex < 0 )
            {
                routing_ptr->m_ActivePointIndex = 0;
            }
            m_LiveIndex = routing_ptr->m_ActivePointIndex;
            routing_ptr->InsertPt( m_LiveIndex );

            routing_ptr->Update();
            m_SelectionFlag = true;
            m_InsertMultipleFlag = true;
            UpdatePickList();
        }
    }
    else if ( gui_device == &m_MovePntTopButton )
    {
        if ( routing_ptr )
        {
            int npt = routing_ptr->GetNumPt();
            if ( routing_ptr->m_ActivePointIndex >= 0 && routing_ptr->m_ActivePointIndex < npt )
            {
                routing_ptr->m_ActivePointIndex = routing_ptr->MovePt( routing_ptr->m_ActivePointIndex, vsp::REORDER_MOVE_TOP );
                routing_ptr->Update();
            }
        }
    }
    else if ( gui_device == &m_MovePntUpButton )
    {
        if ( routing_ptr )
        {
            int npt = routing_ptr->GetNumPt();
            if ( routing_ptr->m_ActivePointIndex >= 0 && routing_ptr->m_ActivePointIndex < npt )
            {
                routing_ptr->m_ActivePointIndex = routing_ptr->MovePt( routing_ptr->m_ActivePointIndex, vsp::REORDER_MOVE_UP );
                routing_ptr->Update();
            }
        }
    }
    else if ( gui_device == &m_MovePntDownButton )
    {
        if ( routing_ptr )
        {
            int npt = routing_ptr->GetNumPt();
            if ( routing_ptr->m_ActivePointIndex >= 0 && routing_ptr->m_ActivePointIndex < npt )
            {
                routing_ptr->m_ActivePointIndex = routing_ptr->MovePt( routing_ptr->m_ActivePointIndex, vsp::REORDER_MOVE_DOWN );
                routing_ptr->Update();
            }
        }
    }
    else if ( gui_device == &m_MovePntBotButton )
    {
        if ( routing_ptr )
        {
            int npt = routing_ptr->GetNumPt();
            if ( routing_ptr->m_ActivePointIndex >= 0 && routing_ptr->m_ActivePointIndex < npt )
            {
                routing_ptr->m_ActivePointIndex = routing_ptr->MovePt( routing_ptr->m_ActivePointIndex, vsp::REORDER_MOVE_BOTTOM );
                routing_ptr->Update();

            }
        }
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
        rpt = routing_ptr->GetPt( m_LiveIndex );

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

                rpt->m_SurfIndx = index;
            }
            else
            {
                rpt->m_U = 1; // Set to dummy value to trigger update.
                rpt->m_U = 0;
                rpt->m_W = 0;
                rpt->m_SurfIndx = 0;
            }

        }

        if ( m_AddMultipleFlag )
        {
            routing_ptr->AddPt();
            m_LiveIndex++;
            routing_ptr->m_ActivePointIndex = m_LiveIndex;

            routing_ptr->Update();

            UpdatePickList();
        }
        else if ( m_InsertMultipleFlag )
        {
            m_LiveIndex++;
            routing_ptr->m_ActivePointIndex = m_LiveIndex;
            routing_ptr->InsertPt( m_LiveIndex );

            routing_ptr->Update();

            UpdatePickList();
        }
        else // Normal adding.
        {
            m_SelectionFlag = false;
            routing_ptr->m_Picking = false;
            routing_ptr->Update();
            m_LiveIndex = -1;
        }

    }
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
                currDrawObj->m_Routing.LiveIndex = m_LiveIndex;
            }
        }
    }
}


