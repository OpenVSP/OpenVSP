//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//////////////////////////////////////////////////////////////////////

#include "AwaveScreen.h"
#include "ScreenMgr.h"
#include "EventMgr.h"
#include "Vehicle.h"
#include "StlHelper.h"
#include <assert.h>

AwaveScreen::AwaveScreen( ScreenMgr *mgr ) : VspScreen( mgr )
{
    AwaveUI* ui = m_AwaveUI = new AwaveUI();
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

    ui->AngleButton->callback( staticScreenCB, this );
    ui->AngleButton->value( 1 );
    ui->AngleSlider->callback( staticScreenCB, this );
    ui->AngleInput->callback( staticScreenCB, this );

    ui->NumberButton->callback( staticScreenCB, this );
    ui->NumberSlider->callback( staticScreenCB, this );
    ui->NumberInput->callback( staticScreenCB, this );

    ui->NumRotSectsSlider->callback( staticScreenCB, this );
    ui->NumRotSectsInput->callback( staticScreenCB, this );

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
    m_Angle = 45;
    m_MNumber = sqrt( ( double )2 );
    m_ComputeAngle = false;
    m_NumRotSecs = 5;
    m_AngleRange[0] = 1e-6;
    m_AngleRange[1] = 90;
    m_MNumberRange[0] = 1;
    m_MNumberRange[1] = 10;
    m_NumRotSecsRange[0] = 3;
    m_NumRotSecsRange[1] = 30;

}

AwaveScreen::~AwaveScreen()
{
    delete m_AwaveUI;
}

bool AwaveScreen::Update()
{
    char str[255];
    char format[10] = " %6.3f";
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    LoadSetChoice();

    m_AwaveUI->txtFileOutput->value( veh->getExportFileName( Vehicle::SLICE_TXT_TYPE ).c_str() );

    vec3d maxBBox = veh->GetBndBox().GetMax();
    vec3d minBBox = veh->GetBndBox().GetMin();
    double max;
    double min;
    int axisIndex = m_AwaveUI->AxisChoice->value();

    min = minBBox[axisIndex];
    max = maxBBox[axisIndex];
    m_Norm.set_xyz( 0, 0, 0 );
    m_Norm[axisIndex] = 1;

    // Set Range again if axis has changed
    if ( m_lastAxis != m_AwaveUI->AxisChoice->value() )
    {
        m_BoundsRange[0] = min;
        m_BoundsRange[1] = max;
        m_lastAxis = m_AwaveUI->AxisChoice->value();
    }
    if ( m_StartVal < m_BoundsRange[0] )
    {
        m_BoundsRange[0] = m_StartVal;
    }
    if ( m_EndVal > m_BoundsRange[1] )
    {
        m_BoundsRange[1] = m_EndVal;
    }

    m_AwaveUI->StartSlider->range( m_BoundsRange[0], m_BoundsRange[1] );
    m_AwaveUI->StartSlider->value( m_StartVal );

    sprintf( str, format, m_StartVal );
    m_AwaveUI->StartInput->value( str );

    m_AwaveUI->EndSlider->range( m_BoundsRange[0], m_BoundsRange[1] );
    m_AwaveUI->EndSlider->value( m_EndVal );

    sprintf( str, format, m_EndVal );
    m_AwaveUI->EndInput->value( str );

    // Num Slices
    if ( m_numSlices < m_SliceRange[0] )
    {
        m_numSlices = m_SliceRange[0];
    }
    if ( m_numSlices > m_SliceRange[1] )
    {
        m_SliceRange[1] = m_numSlices;
    }

    m_AwaveUI->numSlicesSlider->range( m_SliceRange[0], m_SliceRange[1] );
    m_AwaveUI->numSlicesSlider->value( m_numSlices );

    sprintf( str, " %d", m_numSlices );
    m_AwaveUI->numSlicesInput->value( str );

    // Deactivate Bound Control if AutoBounds is on
    if ( m_AwaveUI->AutoBoundsButton->value() )
    {
        m_AwaveUI->StartSlider->deactivate();
        m_AwaveUI->StartInput->deactivate();
        m_AwaveUI->EndSlider->deactivate();
        m_AwaveUI->EndInput->deactivate();
    }
    else
    {
        m_AwaveUI->StartSlider->activate();
        m_AwaveUI->StartInput->activate();
        m_AwaveUI->EndSlider->activate();
        m_AwaveUI->EndInput->activate();
    }

    // Number Rot Slices
    if ( m_NumRotSecs < m_NumRotSecsRange[0] )
    {
        m_NumRotSecs = m_NumRotSecsRange[0];
    }
    if ( m_NumRotSecs > m_NumRotSecsRange[1] )
    {
        m_NumRotSecsRange[1] = m_NumRotSecs;
    }

    m_AwaveUI->NumRotSectsSlider->range( m_NumRotSecsRange[0], m_NumRotSecsRange[1] );
    m_AwaveUI->NumRotSectsSlider->value( m_NumRotSecs );
    sprintf( str, " %d", m_NumRotSecs );
    m_AwaveUI->NumRotSectsInput->value( str );

    // Angle or Mach Number
    if ( m_ComputeAngle )
    {
        if ( m_MNumber < m_MNumberRange[0] )
        {
            m_MNumber = m_MNumberRange[0];
        }

        m_Angle = asin( 1 / m_MNumber ) * RAD_2_DEG;

        m_AwaveUI->AngleSlider->deactivate();
        m_AwaveUI->AngleInput->deactivate();
        m_AwaveUI->NumberInput->activate();
        m_AwaveUI->NumberSlider->activate();
    }
    else
    {
        if ( m_Angle < m_AngleRange[0] )
        {
            m_Angle = m_AngleRange[0];
        }
        if ( m_Angle > m_AngleRange[1] )
        {
            m_Angle = m_AngleRange[1];
        }

        m_MNumber = 1 / sin( m_Angle * DEG_2_RAD );

        m_AwaveUI->NumberInput->deactivate();
        m_AwaveUI->NumberSlider->deactivate();
        m_AwaveUI->AngleInput->activate();
        m_AwaveUI->AngleSlider->activate();
    }

    m_AwaveUI->AngleSlider->range( m_AngleRange[0], m_AngleRange[1] );
    m_AwaveUI->AngleSlider->value( m_Angle );
    sprintf( str, format, m_Angle );
    m_AwaveUI->AngleInput->value( str );

    if ( m_MNumber > m_MNumberRange[1] )
    {
        m_MNumberRange[1] = m_MNumber;
    }

    m_AwaveUI->NumberSlider->range( m_MNumberRange[0], m_MNumberRange[1] );
    m_AwaveUI->NumberSlider->value( m_MNumber );
    sprintf( str, format, m_MNumber );
    m_AwaveUI->NumberInput->value( str );

    return true;
}

void AwaveScreen::Show()
{
    Update();
    m_FLTK_Window->show();

}

void AwaveScreen::Hide()
{
    m_FLTK_Window->hide();
}

void AwaveScreen::LoadSetChoice()
{
    m_AwaveUI->setChoice->clear();

    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();
    vector< string > set_name_vec = veh->GetSetNameVec();

    for ( int i = 0 ; i < ( int )set_name_vec.size() ; i++ )
    {
        m_AwaveUI->setChoice->add( set_name_vec[i].c_str() );
    }

    m_AwaveUI->setChoice->value( m_SelectedSetIndex );
}

void AwaveScreen::CallBack( Fl_Widget* w )
{
    Vehicle* veh = m_ScreenMgr->GetVehiclePtr();

    bool startChanged = false;
    bool endChanged = false;

    if ( w == m_AwaveUI->numSlicesSlider )
    {
        m_numSlices = ( int )m_AwaveUI->numSlicesSlider->value();
    }
    else if ( w == m_AwaveUI->numSlicesInput )
    {
        m_numSlices = atoi( m_AwaveUI->numSlicesInput->value() );
    }
    else if ( w == m_AwaveUI->NumRotSectsSlider )
    {
        m_NumRotSecs = ( int )m_AwaveUI->NumRotSectsSlider->value();
    }
    else if ( w == m_AwaveUI->NumRotSectsInput )
    {
        m_NumRotSecs = atoi( m_AwaveUI->NumRotSectsInput->value() );
    }
    else if ( w == m_AwaveUI->StartSlider )
    {
        m_StartVal = m_AwaveUI->StartSlider->value();
        startChanged = true;
    }
    else if ( w == m_AwaveUI->StartInput )
    {
        m_StartVal = atof( m_AwaveUI->StartInput->value() );
        startChanged = true;
    }
    else if ( w == m_AwaveUI->EndSlider )
    {
        m_EndVal = m_AwaveUI->EndSlider->value();
        endChanged = true;
    }
    else if ( w == m_AwaveUI->EndInput )
    {
        m_EndVal = atof( m_AwaveUI->EndInput->value() );
        endChanged = true;
    }
    else if ( w == m_AwaveUI->AngleButton )
    {
        if ( m_AwaveUI->AngleButton->value() )
        {
            m_AwaveUI->NumberButton->value( 0 );
            m_ComputeAngle = false;
        }
        else
        {
            m_AwaveUI->AngleButton->value( 0 );
            m_AwaveUI->NumberButton->value( 1 );
            m_ComputeAngle = true;
        }
    }
    else if ( w == m_AwaveUI->AngleSlider )
    {
        m_Angle = m_AwaveUI->AngleSlider->value();
    }
    else if ( w == m_AwaveUI->AngleInput )
    {
        m_Angle = atof( m_AwaveUI->AngleInput->value() );
    }
    else if ( w == m_AwaveUI->NumberButton )
    {
        if ( m_AwaveUI->NumberButton->value() )
        {
            m_AwaveUI->AngleButton->value( 0 );
            m_ComputeAngle = true;
        }
        else
        {
            m_AwaveUI->NumberButton->value( 0 );
            m_AwaveUI->AngleButton->value( 1 );
            m_ComputeAngle = false;
        }
    }
    else if ( w == m_AwaveUI->NumberSlider )
    {
        m_MNumber = m_AwaveUI->NumberSlider->value();
    }
    else if ( w == m_AwaveUI->NumberInput )
    {
        m_MNumber = atof( m_AwaveUI->NumberInput->value() );
    }
    else if ( w == m_AwaveUI->fileButton )
    {
        string newfile;
        newfile = m_ScreenMgr->GetSelectFileScreen()->FileChooser( "Choose slice areas output file", "*.txt" );
        veh->setExportFileName( Vehicle::SLICE_TXT_TYPE, newfile );
    }
    else if ( w == m_AwaveUI->setChoice )
    {
        m_SelectedSetIndex = m_AwaveUI->setChoice->value();
    }
    else if ( w == m_AwaveUI->startButton )
    {
        double AngleControlVal;

        if ( m_ComputeAngle )
        {
            AngleControlVal = m_MNumber;
        }
        else
        {
            AngleControlVal = m_Angle;
        }

        string id = veh->AwaveSliceAndFlatten( m_SelectedSetIndex, m_numSlices, m_NumRotSecs, AngleControlVal, m_ComputeAngle, m_Norm,
                                               !!m_AwaveUI->AutoBoundsButton->value(), m_StartVal, m_EndVal );
        if ( id.compare( "NONE" ) != 0 )
        {
            m_AwaveUI->outputTextDisplay->buffer()->loadfile( veh->getExportFileName( Vehicle::SLICE_TXT_TYPE ).c_str() );
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
