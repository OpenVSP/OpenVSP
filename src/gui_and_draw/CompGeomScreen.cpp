//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CompGeomScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include <assert.h>

CompGeomScreen::CompGeomScreen( ScreenMgr* mgr ) : VspScreen( mgr )
{
    CompGeomUI* ui = m_CompGeomUI = new CompGeomUI();

    m_FLTK_Window = ui->UIWindow;
    m_textBuffer = new Fl_Text_Buffer();

    ui->setChoice->callback( staticScreenCB, this );
    ui->csvFileButton->callback( staticScreenCB, this );
    ui->csvFileChooseButton->callback( staticScreenCB, this );
    ui->executeButton->callback( staticScreenCB, this );
    ui->half_mesh_button->callback( staticScreenCB, this );
    ui->outputTextDisplay->callback( staticScreenCB, this );
    ui->tsvFileButton->callback( staticScreenCB, this );
    ui->tsvFileChooseButton->callback( staticScreenCB, this );
    ui->txtFileChooseButon->callback( staticScreenCB, this );
    ui->half_mesh_button->value( 0 );
    ui->outputTextDisplay->buffer( m_textBuffer );

    m_SelectedSetIndex = 0;
    m_HalfMesh = false;
}

bool CompGeomScreen::Update()
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();
    LoadSetChoice();

    if ( vehiclePtr->getExportCompGeomCsvFile() )
    {
        m_CompGeomUI->csvFileButton->value( 1 );
    }
    else
    {
        m_CompGeomUI->csvFileButton->value( 0 );
    }

    if ( vehiclePtr->getExportDragBuildTsvFile() )
    {
        m_CompGeomUI->tsvFileButton->value( 1 );
    }
    else
    {
        m_CompGeomUI->tsvFileButton->value( 0 );
    }

    m_CompGeomUI->csvFileOutput->value( vehiclePtr->getExportFileName( Vehicle::COMP_GEOM_CSV_TYPE ).c_str() );
    m_CompGeomUI->tsvFileOutput->value( vehiclePtr->getExportFileName( Vehicle::COMP_GEOM_TSV_TYPE ).c_str() );
    m_CompGeomUI->txtFileOutput->value( vehiclePtr->getExportFileName( Vehicle::COMP_GEOM_TXT_TYPE ).c_str() );

    return true;
}

void CompGeomScreen::LoadSetChoice()
{
    m_CompGeomUI->setChoice->clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_CompGeomUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_CompGeomUI->setChoice->value( m_SelectedSetIndex );
}

CompGeomScreen::~CompGeomScreen()
{
    delete m_textBuffer;
}

void CompGeomScreen::Show()
{
    Update();
    m_FLTK_Window->show();
}

void CompGeomScreen::Hide()
{
    m_FLTK_Window->hide();
}

void CompGeomScreen::CallBack( Fl_Widget *w )
{
    Vehicle* vehiclePtr = m_ScreenMgr->GetVehiclePtr();

    if ( w == m_CompGeomUI->csvFileButton )
    {
        vehiclePtr->setExportCompGeomCsvFile( !!m_CompGeomUI->csvFileButton->value() );
    }
    else if ( w == m_CompGeomUI->tsvFileButton )
    {
        vehiclePtr->setExportDragBuildTsvFile( !!m_CompGeomUI->tsvFileButton->value() );
    }
    else if ( w == m_CompGeomUI->csvFileChooseButton )
    {
        vehiclePtr->setExportFileName( Vehicle::COMP_GEOM_CSV_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select comp_geom output file.", "*.csv" ) );
    }
    else if ( w == m_CompGeomUI->tsvFileChooseButton )
    {
        vehiclePtr->setExportFileName( Vehicle::COMP_GEOM_TSV_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select comp_geom output file.", "*.tsv" ) );
    }
    else if ( w == m_CompGeomUI->txtFileChooseButon )
    {
        vehiclePtr->setExportFileName( Vehicle::COMP_GEOM_TXT_TYPE,
                                       m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Select comp_geom output file.", "*.txt" ) );
    }
    else if ( w == m_CompGeomUI->setChoice )
    {
        m_SelectedSetIndex = m_CompGeomUI->setChoice->value();
    }
    else if ( w == m_CompGeomUI->half_mesh_button )
    {
        m_HalfMesh = !!m_CompGeomUI->half_mesh_button->value();
    }
    else if ( w == m_CompGeomUI->executeButton )
    {
        string geom = vehiclePtr->CompGeomAndFlatten( m_SelectedSetIndex, 0, 0 , m_HalfMesh );
        if ( geom.compare( "NONE" ) != 0 )
        {
            m_CompGeomUI->outputTextDisplay->buffer()->loadfile( vehiclePtr->getExportFileName( Vehicle::COMP_GEOM_TXT_TYPE ).c_str() );
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}


