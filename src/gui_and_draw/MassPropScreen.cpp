//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "MassPropScreen.h"
#include "ScreenMgr.h"
#include "StlHelper.h"
#include "ParmMgr.h"

MassPropScreen::MassPropScreen( ScreenMgr *mgr ) : VspScreen( mgr )
{
    MassPropUI* ui = m_MassPropUI = new MassPropUI();
    ui->computeButton->callback( staticScreenCB, this );
    ui->numSlicesSlider->callback( staticScreenCB, this );
    ui->numSlicesInput->callback( staticScreenCB, this );
    ui->fileExportButton->callback( staticScreenCB, this );
    ui->setChoice->callback( staticScreenCB, this );
    ui->drawCgButton->callback( staticScreenCB, this );
    ui->numSlicesSlider->range( 10, 200 );
    m_FLTK_Window = ui->UIWindow;
    m_SelectedSetIndex = 0;

    m_CGDrawObj.m_PointSize = 10;
    m_CGDrawObj.m_Type = DrawObj::VSP_POINTS;
    m_CGDrawObj.m_Screen = DrawObj::VSP_MAIN_SCREEN;
    m_CGDrawObj.m_TextColor = vec3d( 0, 0, 0 );
    m_CGDrawObj.m_TextSize = 0;
    m_CGDrawObj.m_GeomID = ParmMgr.GenerateID( 4 ) + "_CG";

    m_FLTK_Window->callback( staticCloseCB, this );

}

MassPropScreen::~MassPropScreen()
{
    delete m_MassPropUI;
}

bool MassPropScreen::Update()
{
    LoadSetChoice();
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();
    char str[255];
    char format[10] = " %6.3f";

    m_MassPropUI->numSlicesSlider->value( vehiclePtr->m_NumMassSlices );
    sprintf( str, " %d", vehiclePtr->m_NumMassSlices );
    m_MassPropUI->numSlicesInput->value( str );

    vehiclePtr->m_DrawCGFlag = m_MassPropUI->drawCgButton->value();

    m_CGDrawObj.m_PntVec.clear();

    if ( vehiclePtr->m_DrawCGFlag )
    {
        m_CGDrawObj.m_Visible = true;
        m_CGDrawObj.m_PntVec.push_back( vehiclePtr->m_CG );
    }
    else
    {
        m_CGDrawObj.m_Visible = false;
    }

    sprintf( str, format, vehiclePtr->m_TotalMass );
    m_MassPropUI->totalMassOutput->value( str );

    vec3d cg = vehiclePtr->m_CG;
    sprintf( str, format, cg.x() );
    m_MassPropUI->xCgOuput->value( str );
    sprintf( str, format, cg.y() );
    m_MassPropUI->yCgOuput->value( str );
    sprintf( str, format, cg.z() );
    m_MassPropUI->zCgOuput->value( str );

    vec3d moi = vehiclePtr->m_IxxIyyIzz;
    sprintf( str, format, moi.x() );
    m_MassPropUI->ixxOuput->value( str );
    sprintf( str, format, moi.y() );
    m_MassPropUI->iyyOutput->value( str );
    sprintf( str, format, moi.z() );
    m_MassPropUI->izzOutput->value( str );

    vec3d pmoi = vehiclePtr->m_IxyIxzIyz;
    sprintf( str, format, pmoi.x() );
    m_MassPropUI->ixyOutput->value( str );
    sprintf( str, format, pmoi.y() );
    m_MassPropUI->ixzOutput->value( str );
    sprintf( str, format, pmoi.z() );
    m_MassPropUI->iyzOutput->value( str );

    m_MassPropUI->fileExportOutput->value( vehiclePtr->getExportFileName( vsp::MASS_PROP_TXT_TYPE ).c_str() );

    return true;
}

void MassPropScreen::Show()
{
    m_ScreenMgr->SetUpdateFlag( true );
    m_FLTK_Window->show();
}

void MassPropScreen::Hide()
{
    m_FLTK_Window->hide();
    m_ScreenMgr->SetUpdateFlag( true );
}

void MassPropScreen::LoadSetChoice()
{
    m_MassPropUI->setChoice->clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_MassPropUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_MassPropUI->setChoice->value( m_SelectedSetIndex );
}

void MassPropScreen::CallBack( Fl_Widget* w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    if ( w == m_MassPropUI->computeButton )
    {
        veh->MassPropsAndFlatten( m_SelectedSetIndex, veh->m_NumMassSlices );
    }
    else if ( w == m_MassPropUI->numSlicesSlider )
    {
        veh->m_NumMassSlices = ( int )m_MassPropUI->numSlicesSlider->value();
    }
    else if ( w == m_MassPropUI->numSlicesInput )
    {
        veh->m_NumMassSlices = atoi( m_MassPropUI->numSlicesInput->value() );
    }
    else if ( w == m_MassPropUI->fileExportButton )
    {
        string newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Choose mass properties output file", "*.txt" );
        veh->setExportFileName( vsp::MASS_PROP_TXT_TYPE, newfile );
    }
    else if ( w == m_MassPropUI->setChoice )
    {
        m_SelectedSetIndex = m_MassPropUI->setChoice->value();
    }

    m_ScreenMgr->SetUpdateFlag( true );
}

void MassPropScreen::CloseCallBack( Fl_Widget* w )
{
    Hide();
}

void MassPropScreen::LoadDrawObjs( vector< DrawObj* >& draw_obj_vec )
{
    if ( IsShown() )
    {
        draw_obj_vec.push_back( &m_CGDrawObj );
    }
}