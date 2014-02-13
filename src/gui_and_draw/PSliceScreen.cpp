//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "PSliceScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include <assert.h>

PSliceScreen::PSliceScreen( ScreenMgr *mgr ) : VspScreen( mgr )
{
    PSliceUI* ui = m_PSliceUI = new PSliceUI();
    m_textBuffer = new Fl_Text_Buffer();
    ui->outputTextDisplay->buffer( m_textBuffer );

    char format[10] = " %6.3f";
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vec3d maxBBox = veh->GetBndBox().GetMax();
    vec3d minBBox = veh->GetBndBox().GetMin();

    ui->numSlicesSlider->callback( staticScreenCB, this );

    ui->numSlicesInput->callback( staticScreenCB, this );

    ui->AxisChoice->callback( staticScreenCB, this );
    ui->AxisChoice->value( 0 );

    ui->AutoBoundsButton->callback( staticScreenCB, this );
    ui->AutoBoundsButton->value( 1 );

    ui->StartSlider->callback( staticScreenCB, this );

    ui->StartInput->callback( staticScreenCB, this );

    ui->EndSlider->callback( staticScreenCB, this );

    ui->EndInput->callback( staticScreenCB, this );

    ui->fileButton->callback( staticScreenCB, this );
    ui->setChoice->callback( staticScreenCB, this );
    ui->startButton->callback( staticScreenCB, this );

    m_FLTK_Window = ui->UIWindow;
    m_SelectedSetIndex = 0;
    m_lastAxis = 0;
    m_BoundsRange[0] = 0;
    m_BoundsRange[1] = 10;
    m_StartVal = 0;
    m_EndVal = 10;
    m_Norm = vec3d( 1, 0, 0 );
    m_numSlices = 10;
    m_SliceRange[0] = 3;
    m_SliceRange[1] = 100;

}

PSliceScreen::~PSliceScreen()
{
    delete m_PSliceUI;
}

bool PSliceScreen::Update()
{
    char str[255];
    char format[10] = " %6.3f";
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    LoadSetChoice();

    m_PSliceUI->txtFileOutput->value( veh->getExportFileName( Vehicle::SLICE_TXT_TYPE ).c_str() );

    vec3d maxBBox = veh->GetBndBox().GetMax();
    vec3d minBBox = veh->GetBndBox().GetMin();
    double max;
    double min;
    int axisIndex = m_PSliceUI->AxisChoice->value();

    min = minBBox[axisIndex];
    max = maxBBox[axisIndex];
    m_Norm.set_xyz( 0, 0, 0 );
    m_Norm[axisIndex] = 1;

    // Set Range again if axis has changed
    if ( m_lastAxis != m_PSliceUI->AxisChoice->value() )
    {
        m_BoundsRange[0] = min;
        m_BoundsRange[1] = max;
        m_lastAxis = m_PSliceUI->AxisChoice->value();
    }
    if ( m_StartVal < m_BoundsRange[0] )
    {
        m_BoundsRange[0] = m_StartVal;
    }
    if ( m_EndVal > m_BoundsRange[1] )
    {
        m_BoundsRange[1] = m_EndVal;
    }

    m_PSliceUI->StartSlider->range( m_BoundsRange[0], m_BoundsRange[1] );
    m_PSliceUI->StartSlider->value( m_StartVal );

    sprintf( str, format, m_StartVal );
    m_PSliceUI->StartInput->value( str );

    m_PSliceUI->EndSlider->range( m_BoundsRange[0], m_BoundsRange[1] );
    m_PSliceUI->EndSlider->value( m_EndVal );

    sprintf( str, format, m_EndVal );
    m_PSliceUI->EndInput->value( str );

    // Num Slices
    if ( m_numSlices < m_SliceRange[0] )
    {
        m_numSlices = m_SliceRange[0];
    }
    if ( m_numSlices > m_SliceRange[1] )
    {
        m_SliceRange[1] = m_numSlices;
    }

    m_PSliceUI->numSlicesSlider->range( m_SliceRange[0], m_SliceRange[1] );
    m_PSliceUI->numSlicesSlider->value( m_numSlices );

    sprintf( str, " %d", m_numSlices );
    m_PSliceUI->numSlicesInput->value( str );

    // Deactivate Bound Control if AutoBounds is on
    if ( m_PSliceUI->AutoBoundsButton->value() )
    {
        m_PSliceUI->StartSlider->deactivate();
        m_PSliceUI->StartInput->deactivate();
        m_PSliceUI->EndSlider->deactivate();
        m_PSliceUI->EndInput->deactivate();
    }
    else
    {
        m_PSliceUI->StartSlider->activate();
        m_PSliceUI->StartInput->activate();
        m_PSliceUI->EndSlider->activate();
        m_PSliceUI->EndInput->activate();
    }

    return true;
}

void PSliceScreen::Show()
{
    Update();
    m_FLTK_Window->show();

}

void PSliceScreen::Hide()
{
    m_FLTK_Window->hide();
}

void PSliceScreen::LoadSetChoice()
{
    m_PSliceUI->setChoice->clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_PSliceUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_PSliceUI->setChoice->value( m_SelectedSetIndex );
}

void PSliceScreen::CallBack( Fl_Widget* w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    bool startChanged = false;
    bool endChanged = false;

    if ( w == m_PSliceUI->numSlicesSlider )
    {
        m_numSlices = ( int )m_PSliceUI->numSlicesSlider->value();
    }
    else if ( w == m_PSliceUI->numSlicesInput )
    {
        m_numSlices = atoi( m_PSliceUI->numSlicesInput->value() );
    }
    else if ( w == m_PSliceUI->StartSlider )
    {
        m_StartVal = m_PSliceUI->StartSlider->value();
        startChanged = true;
    }
    else if ( w == m_PSliceUI->StartInput )
    {
        m_StartVal = atof( m_PSliceUI->StartInput->value() );
        startChanged = true;
    }
    else if ( w == m_PSliceUI->EndSlider )
    {
        m_EndVal = m_PSliceUI->EndSlider->value();
        endChanged = true;
    }
    else if ( w == m_PSliceUI->EndInput )
    {
        m_EndVal = atof( m_PSliceUI->EndInput->value() );
        endChanged = true;
    }
    else if ( w == m_PSliceUI->fileButton )
    {
        string newfile;
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Choose slice areas output file", "*.txt" );
        veh->setExportFileName( Vehicle::SLICE_TXT_TYPE, newfile );
    }
    else if ( w == m_PSliceUI->setChoice )
    {
        m_SelectedSetIndex = m_PSliceUI->setChoice->value();
    }
    else if ( w == m_PSliceUI->startButton )
    {
        string id = veh->PSliceAndFlatten( m_SelectedSetIndex, m_numSlices, m_Norm,
                                           !!m_PSliceUI->AutoBoundsButton->value(), m_StartVal, m_EndVal );
        if ( id.compare( "NONE" ) != 0 )
        {
            m_PSliceUI->outputTextDisplay->buffer()->loadfile( veh->getExportFileName( Vehicle::SLICE_TXT_TYPE ).c_str() );
        }
    }

    // Check to make sure start is less than end
    if ( m_StartVal > m_EndVal )
    {
        if ( startChanged )
        {
            m_EndVal = m_StartVal;
        }
        else if ( endChanged )
        {
            m_StartVal = m_EndVal;
        }
    }

    m_ScreenMgr->SetUpdateFlag( true );
}
