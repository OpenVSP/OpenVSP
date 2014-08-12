//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GuiDevice.h"
#include "Parm.h"
#include "ParmMgr.h"
#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "LinkMgr.h"
#include "StlHelper.h"

#include <float.h>
#include <assert.h>
#include <cmath>

using std::max;
using std::min;

//==== Constructor ====//
GuiDevice::GuiDevice()
{
    m_Type = -1;
    m_Screen = NULL;
    m_ParmID = string( "NOT_DEFINED" );
    m_NewParmFlag = true;
    m_LastVal = 0.0;
    m_ResizableWidgetIndex = 0;
}

//==== Init ====//
void GuiDevice::Init( VspScreen* screen )
{
    m_Screen = screen;
}

//==== Add Widget ====//
void GuiDevice::AddWidget( Fl_Widget* w, bool resizable_flag )
{
    if ( w )
    {
        m_WidgetVec.push_back( w );
    }
    if ( resizable_flag )
    {
        m_ResizableWidgetIndex = (int)m_WidgetVec.size() - 1;
    }
}

//==== Activate ====//
void GuiDevice::Activate()
{
    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        m_WidgetVec[i]->activate();
    }
}

//==== Deactivate ====//
void GuiDevice::Deactivate()
{
    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        m_WidgetVec[i]->deactivate();
    }
}

//==== Set Parm Pointer ====//
Parm* GuiDevice::SetParmID( const string& parm_id )
{
    //==== Check For New ParmID ====//
    m_NewParmFlag = false;
    if ( parm_id != m_ParmID )
    {
        m_NewParmFlag = true;
    }

    m_ParmID = parm_id;
    return ParmMgr.FindParm( m_ParmID );
}

//==== Check If Val Should Be Updated ====//
bool GuiDevice::CheckValUpdate( double val )
{
    if ( m_NewParmFlag )
    {
        return true;
    }

    if ( fabs( val - m_LastVal ) < DBL_EPSILON )
    {
        return false;
    }
    return true;
}

//==== Update ====//
void GuiDevice::Update( const string& parm_id )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( parm_id );
    if ( !parm_ptr )
    {
        Deactivate();
        return;
    }

    SetValAndLimits( parm_ptr );

    if ( parm_ptr->GetActiveFlag() )
    {
        Activate();
    }
    else
    {
        Deactivate();
    }
}

//==== Set Total Width By Resizing First Widget Of List ====//
void GuiDevice::SetWidth( int w )
{
    if ( m_WidgetVec.size() == 0 )
        return;

    Fl_Widget* resize_widget = m_WidgetVec[0];

    if ( m_ResizableWidgetIndex < (int)m_WidgetVec.size() )
        resize_widget = m_WidgetVec[m_ResizableWidgetIndex];

    int total_w = GetWidth();
    int new_w = resize_widget->w() + w - total_w;

    //==== Limit Size ====//
    if ( new_w < 10 )
        new_w = 10;

    resize_widget->size( new_w, m_WidgetVec[0]->h() );
}

int GuiDevice::GetWidth( )
{
    int total_w = 0;
    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        total_w += m_WidgetVec[i]->w();
    }
    return total_w;
}

int GuiDevice::GetX()
{
    if ( m_WidgetVec.size() == 0 )
        return 0;

    int smallest_x = 10000;
    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        if ( m_WidgetVec[i]->x() < smallest_x )
            smallest_x = m_WidgetVec[i]->x();
    }
    return smallest_x;
}

void GuiDevice::SetX( int x )
{
    if ( m_WidgetVec.size() == 0 )
        return;

    int curr_x = x;
    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        Fl_Widget* widget = m_WidgetVec[i];
        widget->resize( curr_x, widget->y(), widget->w(), widget->h() );
        curr_x += widget->w();
    }
}

void GuiDevice::OffsetX( int x )
{
    if ( m_WidgetVec.size() == 0 )
        return;

    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        Fl_Widget* widget = m_WidgetVec[i];
        m_WidgetVec[i]->resize( widget->x() + x, widget->y(), widget->w(), widget->h() );
    }
}


//=====================================================================//
//======================           Input         ======================//
//=====================================================================//

//==== Constructor ====//
Input::Input() : GuiDevice()
{
    m_Type = GDEV_INPUT;
    m_Input = NULL;
    m_Format = string( " %7.5f" );
    m_ParmButtonFlag = false;
}

//==== Init ====//
void Input::Init( VspScreen* screen, Fl_Input* input, const char* format, Fl_Button* parm_button )
{
    assert( input );
    GuiDevice::Init( screen );
    AddWidget( parm_button );
    AddWidget( input, true );

    SetFormat( format );
    m_Input = input;
    m_Input->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Input->callback( StaticDeviceCB, this );

    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }
}

//==== Set Input Value and Limits =====//
void Input::SetValAndLimits( Parm* parm_ptr )
{
    assert( m_Input );
    double new_val = parm_ptr->Get();

    if ( CheckValUpdate( new_val ) )
    {
        sprintf( m_Str, m_Format.c_str(), new_val );
        m_Input->value( m_Str );
    }
    m_LastVal = new_val;

    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Update( parm_ptr->GetID() );
    }
}

//==== CallBack ====//
void Input::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
     Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Input )
    {
        double new_val = atof( m_Input->value() );
        parm_ptr->SetFromDevice( new_val );
        m_LastVal = new_val;
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================           Slider        ======================//
//=====================================================================//

//==== Constructor ====//
Slider::Slider( ) : GuiDevice()
{
    m_Type = GDEV_SLIDER;
    m_Slider = NULL;
    m_Range = 10.0;
    m_MinBound = 0.0;
    m_MaxBound = 0.0;
}

//==== Init ====//
void Slider::Init( VspScreen* screen,   Fl_Slider* slider_widget, double range )
{
    GuiDevice::Init( screen );
    AddWidget(slider_widget);
    SetRange( range );
    m_Slider = slider_widget;
    assert( m_Slider );
    m_Slider->callback( StaticDeviceCB, this );
}

//==== Set Slider Value and Limits =====//
void Slider::SetValAndLimits( Parm* parm_ptr )
{
    assert( m_Slider );
    double new_val = parm_ptr->Get();

    if ( m_NewParmFlag || new_val < m_MinBound || new_val > m_MaxBound )
    {
        m_MinBound = max( new_val - m_Range, parm_ptr->GetLowerLimit() );
        m_MaxBound = min( new_val + m_Range, parm_ptr->GetUpperLimit() );
        m_Slider->bounds( m_MinBound, m_MaxBound );
    }

    if ( CheckValUpdate( new_val ) )
    {
        m_Slider->value( new_val );
    }

    m_LastVal = new_val;
}

//==== CallBack ====//
void Slider::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Slider )
    {
        double new_val = m_Slider->value();
        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//================     Slider Adjustable Range       ==================//
//=====================================================================//

//==== Constructor ====//
SliderAdjRange::SliderAdjRange( ) : Slider()
{
    m_Type = GDEV_SLIDER_ADJ_RANGE;
    m_MinButton = NULL;
    m_MaxButton = NULL;
    m_MinStopState = SAR_NO_STOP;
    m_MaxStopState = SAR_NO_STOP;
    m_ButtonChangeFract = 0.1;
    m_Tol = 0.000001;
}

//==== Init ====//
void SliderAdjRange::Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                           Fl_Button* rbutton, double range )
{
    Slider::Init( screen, slider, range );

    ClearAllWidgets();
    AddWidget( lbutton );
    AddWidget( slider, true );
    AddWidget( rbutton );

    m_MinStopState = SAR_NO_STOP;
    m_MaxStopState = SAR_NO_STOP;

    m_MinButton = lbutton;
    m_MaxButton = rbutton;

    assert( m_MinButton );
    assert( m_MaxButton );

    m_MinButton->callback( StaticDeviceCB, this );
    m_MaxButton->callback( StaticDeviceCB, this );
}


//==== Set Slider Value and Limits =====//
void SliderAdjRange::SetValAndLimits( Parm* parm_ptr )
{
    assert( m_Slider );
    double new_val = parm_ptr->Get();

    if ( m_NewParmFlag || new_val < m_MinBound || new_val > m_MaxBound )
    {
        if ( fabs( new_val - parm_ptr->GetLowerLimit() ) < m_Tol )
        {
            m_MinStopState = SAR_ABS_STOP;
        }
        if ( fabs( new_val - parm_ptr->GetUpperLimit() ) < m_Tol )
        {
            m_MaxStopState = SAR_ABS_STOP;
        }

        m_MinBound = max( new_val - m_Range, parm_ptr->GetLowerLimit() );
        m_MaxBound = min( new_val + m_Range, parm_ptr->GetUpperLimit() );
        m_Slider->bounds( m_MinBound, m_MaxBound );
    }

    FindStopState( parm_ptr );

    if ( CheckValUpdate( new_val ) )
    {
        m_Slider->value( new_val );
    }

    m_LastVal = new_val;
}


//==== CallBack ====//
void SliderAdjRange::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Slider )
    {
        double new_val = m_Slider->value();
        parm_ptr->SetFromDevice( new_val );
        FindStopState( parm_ptr );
    }
    else if ( w == m_MinButton )
    {
        MinButtonCB( parm_ptr );
    }
    else if ( w == m_MaxButton )
    {
        MaxButtonCB( parm_ptr );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//==== Figure Out Stop States =====//
void SliderAdjRange::FindStopState( Parm* parm_ptr )
{
    double val = parm_ptr->Get();

    if ( fabs( val - parm_ptr->GetLowerLimit() ) < m_Tol )
    {
        m_MinStopState = SAR_ABS_STOP;
        m_MinButton->label( "|" );
    }
    else if ( fabs( val - m_MinBound ) < m_Tol )
    {
        m_MinStopState = SAR_STOP;
        m_MinButton->label( "<" );
    }
    else
    {
        m_MinStopState = SAR_NO_STOP;
        m_MinButton->label( ">" );
    }
    if ( fabs( val - parm_ptr->GetUpperLimit() ) < m_Tol )
    {
        m_MaxStopState = SAR_ABS_STOP;
        m_MaxButton->label( "|" );
    }
    else if ( fabs( val - m_MaxBound ) < m_Tol )
    {
        m_MaxStopState = SAR_STOP;
        m_MaxButton->label( ">" );
    }
    else
    {
        m_MaxStopState = SAR_NO_STOP;
        m_MaxButton->label( "<" );
    }
}

//==== Min Button - Triggered By User =====//
void SliderAdjRange::MinButtonCB( Parm* parm_ptr )
{
    if ( m_MinStopState == SAR_ABS_STOP )
    {
        return;
    }

    if ( m_MinStopState == SAR_STOP )
    {
        m_MinBound =  m_MinBound - 0.5 * ( m_MaxBound - m_MinBound );
        m_Range = m_MaxBound - m_MinBound;
        parm_ptr->SetFromDevice( m_MinBound );
    }
    else
    {
        double val = parm_ptr->Get();
        m_Range *= 0.5;
        m_MinBound = max( val - m_Range, parm_ptr->GetLowerLimit() );
        m_MaxBound = min( val + m_Range, parm_ptr->GetUpperLimit() );
    }
    m_Slider->bounds( m_MinBound, m_MaxBound );
}

//==== Max Button - Triggered By User =====//
void SliderAdjRange::MaxButtonCB( Parm* parm_ptr )
{
    if ( m_MaxStopState == SAR_ABS_STOP )
    {
        return;
    }

    if ( m_MaxStopState == SAR_STOP )
    {
        m_MaxBound =  m_MaxBound + 0.5 * ( m_MaxBound - m_MinBound );
        m_Range = m_MaxBound - m_MinBound;
        parm_ptr->SetFromDevice( m_MaxBound );
    }
    else
    {
        double val = parm_ptr->Get();
        m_Range *= 0.5;
        m_MinBound = max( val - m_Range, parm_ptr->GetLowerLimit() );
        m_MaxBound = min( val + m_Range, parm_ptr->GetUpperLimit() );
    }
    m_Slider->bounds( m_MinBound, m_MaxBound );
}


//=====================================================================//
//======================        Log Slider        =====================//
//=====================================================================//

//==== Constructor ====//
LogSlider::LogSlider() : Slider()
{
    m_Type = GDEV_LOG_SLIDER;
}

//==== Set Slider Value and Limits =====//
void LogSlider::SetValAndLimits( Parm* parm_ptr )
{
    double m_Tol = 0.000001;
    assert( m_Slider );
    double new_val = parm_ptr->Get();

    if ( m_NewParmFlag || new_val < ( m_MinBound - m_Tol ) || new_val > ( m_MaxBound + m_Tol ) )
    {
        m_MinBound = max( new_val - m_Range, parm_ptr->GetLowerLimit() );
        m_MaxBound = min( new_val + m_Range, parm_ptr->GetUpperLimit() );
        m_Slider->bounds( log10( m_MinBound ), log10( m_MaxBound ) );
    }

    if ( CheckValUpdate( new_val ) )
    {
        m_Slider->value( log10( new_val ) );
    }

    m_LastVal = new_val;
}


//==== CallBack ====//
void LogSlider::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Slider )
    {
        double new_val = pow( 10, m_Slider->value() );
        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}


//=====================================================================//
//=================       Slider Input Combo     ======================//
//=====================================================================//

//==== Init ====//
void SliderInput::Init( VspScreen* screen, Fl_Slider* slider, Fl_Input* input,
                        double range, const char* format, Fl_Button* parm_button,
                        bool log_slider )
{
    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    m_LogSliderFlag = log_slider;
    if ( m_LogSliderFlag )
    {
        m_LogSlider.Init( screen, slider, range );
    }
    else
    {
        m_Slider.Init( screen, slider, range );
    }

    m_Input.Init( screen, input, format );

    ClearAllWidgets();
    AddWidget( parm_button );
    AddWidget( slider, true );
    AddWidget( input );

}

void SliderInput::Update( const string& parm_id )
{
    if ( m_LogSliderFlag )
    {
        m_LogSlider.Update( parm_id );
    }
    else
    {
        m_Slider.Update( parm_id );
    }

    m_Input.Update( parm_id );

    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Update( parm_id );
    }
}


//=====================================================================//
//===========       Slider Adjustable Range Input Combo     ===========//
//=====================================================================//

//==== Init ====//
void SliderAdjRangeInput::Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                                Fl_Button* rbutton, Fl_Input* input, double range, const char* format, Fl_Button* parm_button )
{
    m_Type = GDEV_SLIDER_ADJ_RANGE_INPUT;
    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    m_Slider.Init( screen, slider, lbutton, rbutton, range );
    m_Input.Init( screen, input, format );

    ClearAllWidgets();
    AddWidget( parm_button );
    AddWidget( lbutton );
    AddWidget( slider, true );
    AddWidget( rbutton );
    AddWidget( input );

}

void SliderAdjRangeInput::Update( const string& parm_id )
{
    m_Slider.Update( parm_id );
    m_Input.Update( parm_id );
    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Update( parm_id );
    }
}

//=====================================================================//
//===========       Slider Adjustable Range 2 Input Combo     ===========//
//=====================================================================//

void SliderAdjRange2Input::Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                                 Fl_Button* rbutton, Fl_Input* input1, Fl_Input* input2,
                                 double range, const char* format, Fl_Button* parm_button )
{
    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    m_Slider.Init( screen, slider, lbutton, rbutton, range );
    m_Input1.Init( screen, input1, format );
    m_Input2.Init( screen, input2, format );

}

void SliderAdjRange2Input::Update( int slider_id, const string& parm_id_in1, const string& parm_id_in2 )
{
    if ( slider_id == 1 )
    {
        m_Slider.Update( parm_id_in1 );
        if ( m_ParmButtonFlag )
        {
            m_ParmButton.Update( parm_id_in1 );
        }
    }
    else
    {
        m_Slider.Update( parm_id_in2 );
        if ( m_ParmButtonFlag )
        {
            m_ParmButton.Update( parm_id_in2 );
        }
    }
    m_Input1.Update( parm_id_in1 );
    m_Input2.Update( parm_id_in2 );
}

void SliderAdjRange2Input::Activate()
{
    m_Slider.Activate();
    m_Input1.Activate();
    m_Input2.Activate();
    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Activate();
    }
}

void SliderAdjRange2Input::Deactivate()
{
    m_Slider.Deactivate();
    m_Input1.Deactivate();
    m_Input2.Deactivate();
    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Deactivate();
    }
}

//=====================================================================//
//======================           ParmButton        ==================//
//=====================================================================//

//==== Constructor ====//
ParmButton::ParmButton( ) : GuiDevice()
{
    m_Type = GDEV_PARM_BUTTON;
    m_Button = NULL;
    m_ButtonNameUpdate = false;
}

//==== Init ====//
void ParmButton::Init( VspScreen* screen, Fl_Button* button )
{
    GuiDevice::Init( screen );
    AddWidget(button);
    m_Button = button;
    assert( m_Button );
    m_Button->callback( StaticDeviceCB, this );
}

//==== Update ====//
void ParmButton::Update( const string& parm_id )
{
    GuiDevice::Update( parm_id );

    if( m_ButtonNameUpdate )
    {
        Parm* parm_ptr = SetParmID( parm_id );
        if ( parm_ptr )
        {
            m_Button->copy_label( parm_ptr->GetName().c_str() );
        }
    }
}

//==== Set Slider Value and Limits =====//
void ParmButton::SetValAndLimits( Parm* )
{
}


//==== CallBack ====//
void ParmButton::DeviceCB( Fl_Widget* w )
{
    if ( w == m_Button )
    {
        ParmMgr.SetActiveParm( m_ParmID );

        m_Screen->GetScreenMgr()->ShowScreen( ScreenMgr::VSP_PARM_SCREEN );
//      m_Screen->GetScreenMgr()->ShowParmScreen(parm_ptr, Fl::event_x_root(), Fl::event_y_root());

    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================           CheckButton        ==================//
//=====================================================================//

//==== Constructor ====//
CheckButton::CheckButton( ) : GuiDevice()
{
    m_Type = GDEV_CHECK_BUTTON;
    m_Button = NULL;
}

//==== Init ====//
void CheckButton::Init( VspScreen* screen, Fl_Check_Button* button  )
{
    GuiDevice::Init( screen );
    AddWidget(button);
    m_Button = button;
    assert( m_Button );
    m_Button->callback( StaticDeviceCB, this );
}

//==== Set Slider Value and Limits =====//
void CheckButton::SetValAndLimits( Parm* p )
{
    assert( m_Button );
    if ( !p )
    {
        return;
    }

    BoolParm* bparm = dynamic_cast< BoolParm* >( p );
    assert( bparm );
    if ( bparm->Get() == true )
    {
        m_Button->value( 1 );
    }
    else
    {
        m_Button->value( 0 );
    }
}

//==== CallBack ====//
void CheckButton::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Button )
    {
        int new_val = m_Button->value();
        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================           CheckButtonBit       ==================//
//=====================================================================//

//==== Constructor ====//
CheckButtonBit::CheckButtonBit() : GuiDevice()
{
    m_Type = GDEV_CHECK_BUTTON_BIT;
    m_Button = NULL;
    m_value = 0;
}

//==== Init ====//
void CheckButtonBit::Init( VspScreen* screen, Fl_Button* button, int value )
{
    GuiDevice::Init( screen );
    AddWidget(button);
    m_Button = button;
    m_value = value;
    assert( m_Button );
    m_Button->callback( StaticDeviceCB, this );
}
//==== Set Button Value ====//
void CheckButtonBit::SetValAndLimits( Parm* p )
{
    assert( m_Button );
    if ( !p )
    {
        return;
    }

    IntParm* iparm = dynamic_cast< IntParm* >( p );
    if ( iparm->Get() & m_value )
    {
        m_Button->set();
    }
    else
    {
        m_Button->clear();
    }
}

//==== Callback ====//
void CheckButtonBit::DeviceCB( Fl_Widget* w )
{
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }
    IntParm* int_parm_ptr = dynamic_cast< IntParm* >( parm_ptr );
    assert( int_parm_ptr );
    if ( w == m_Button )
    {
        int new_val;

        if ( m_Button->value() )
        {
            new_val = int_parm_ptr->Get() | m_value;    // Add Flag
        }
        else
        {
            new_val = int_parm_ptr->Get() & ~m_value;    // Remove Flag
        }

        int_parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================           RadioButton       ==================//
//=====================================================================//

//==== Constructor ====//
RadioButton::RadioButton() : GuiDevice()
{
    m_Type = GDEV_RADIO_BUTTON;
    m_Button = NULL;
    m_value = 0;
}

//==== Init ====//
void RadioButton::Init( VspScreen* screen, Fl_Button* button, int value )
{
    GuiDevice::Init( screen );
    AddWidget(button);
    m_Button = button;
    m_value = value;
    assert( m_Button );
    m_Button->callback( StaticDeviceCB, this );
}

//==== Set Button Value ====//
void RadioButton::SetValAndLimits( Parm* p )
{
    assert( m_Button );
    if ( !p )
    {
        return;
    }

    IntParm* iparm = dynamic_cast< IntParm* >( p );
    if ( iparm->Get() == m_value )
    {
        m_Button->setonly();
    }
}

//==== Callback ====//
void RadioButton::DeviceCB( Fl_Widget* w )
{
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Button )
    {
        int new_val = m_value;
        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================         Toggle Button        =================//
//=====================================================================//

//==== Constructor ====//
ToggleButton::ToggleButton() : GuiDevice()
{
    m_Type = GDEV_TOGGLE_BUTTON;
    m_Button = NULL;
}

//==== Init ====//
void ToggleButton::Init( VspScreen* screen, Fl_Button* button )
{
    GuiDevice::Init( screen );
    AddWidget(button);
    m_Button = button;
    assert( m_Button );
    m_Button->callback( StaticDeviceCB, this );
}

//==== Set Button Value ====//
void ToggleButton::SetValAndLimits( Parm* p )
{
    assert( m_Button );
    if ( !p )
    {
        return;
    }

    BoolParm* bool_p = dynamic_cast<BoolParm*>( p );
    assert( bool_p );

    if ( bool_p->Get() )
    {
        m_Button->set();
    }
    else
    {
        m_Button->clear();
    }
}

//==== Callback ====//
void ToggleButton::DeviceCB( Fl_Widget* w )
{
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Button )
    {
        bool new_val;
        if ( m_Button->value() )
        {
            new_val = true;
        }
        else
        {
            new_val = false;
        }

        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================         Toggle Radio Group       =============//
//=====================================================================//

//==== Constructor ====//
ToggleRadioGroup::ToggleRadioGroup() : GuiDevice()
{
    m_Type = GDEV_TOGGLE_RADIO_GROUP;

}

//==== Init ====//
void ToggleRadioGroup::Init( VspScreen* screen )
{
    GuiDevice::Init( screen );
}

//===== Add Another Toggle Button To Group ====//
void ToggleRadioGroup::AddButton( Fl_Button* button )
{
    assert( button );
    m_ButtonVec.push_back( button );
    button->callback( StaticDeviceCB, this );
}

//==== Set Button Value ====//
void ToggleRadioGroup::SetValAndLimits( Parm* p )
{
    if ( !p )
    {
        return;
    }

    int val = ( int )( p->Get() + 0.5 );

    //==== Check If Val Is Mapped To Index ====//
    for ( int i = 0 ; i < ( int )m_ValMapVec.size() ; i++ )
    {
        if ( val == m_ValMapVec[i] )
        {
            val = i;
            break;
        }
    }

    //==== Set Buttons Based on Value of Parm ====//
    for ( int i = 0 ; i < ( int )m_ButtonVec.size() ; i++ )
    {
        if ( val == i )
        {
            m_ButtonVec[i]->set();
        }
        else
        {
            m_ButtonVec[i]->clear();
        }
    }
}


//==== Callback ====//
void ToggleRadioGroup::DeviceCB( Fl_Widget* w )
{
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    //==== Find Out Which Button Was Pushed ====//
    for ( int i = 0 ; i < ( int )m_ButtonVec.size() ; i++ )
    {
        if ( w == m_ButtonVec[i] )
        {
            int val = i;
            if ( i < ( int )m_ValMapVec.size() )
            {
                val = m_ValMapVec[i];
            }

            parm_ptr->SetFromDevice( val );
            SetValAndLimits( parm_ptr );
        }
    }

    m_Screen->GuiDeviceCallBack( this );
}

//==== Set Val Map Vec ====//
void ToggleRadioGroup::SetValMapVec( vector< int > & val_map_vec )
{
    m_ValMapVec = val_map_vec;
}




//=====================================================================//
//======================         Trigger Button        =================//
//=====================================================================//

//==== Constructor ====//
TriggerButton::TriggerButton() : GuiDevice()
{
    m_Type = GDEV_TRIGGER_BUTTON;
    m_Button = NULL;
}

//==== Init ====//
void TriggerButton::Init( VspScreen* screen, Fl_Button* button )
{
    GuiDevice::Init( screen );
    AddWidget(button);
    m_Button = button;
    assert( m_Button );
    m_Button->callback( StaticDeviceCB, this );
}

//==== Callback ====//
void TriggerButton::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );

    if ( w == m_Button )
    {
        if ( parm_ptr )
            parm_ptr->SetFromDevice( 1.0 );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================           Counter            =================//
//=====================================================================//
//==== Constructor ====//
Counter::Counter() : GuiDevice()
{
    m_Type = GDEV_COUNTER;
    m_Counter = NULL;
}

//==== Init ====//
void Counter::Init( VspScreen* screen, Fl_Counter* counter, Fl_Button* parm_button )
{
    assert( counter );

    GuiDevice::Init( screen );
    m_Counter = counter;
    m_Counter->callback( StaticDeviceCB, this );

    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    ClearAllWidgets();
    AddWidget(parm_button);
    AddWidget(counter, true);
 }

//==== Set Counter Value ====//
void Counter::SetValAndLimits( Parm* p )
{
    assert( m_Counter );
    if ( !p )
    {
        return;
    }
    m_Counter->value( p->Get() );
}

//==== Callback ====//
void Counter::DeviceCB( Fl_Widget* w )
{
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_Counter )
    {
        double new_val = m_Counter->value();
        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//======================           Choice            ==================//
//=====================================================================//

//==== Constructor ====//
Choice::Choice( ) : GuiDevice()
{
    m_Type = GDEV_CHOICE;
    m_Choice = NULL;
}

//==== Init ====//
void Choice::Init( VspScreen* screen, Fl_Choice* fl_choice, Fl_Button* parm_button  )
{
    GuiDevice::Init( screen );
    m_Choice = fl_choice;
    assert( m_Choice );
    m_Choice->callback( StaticDeviceCB, this );

    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    ClearAllWidgets();
    AddWidget( parm_button );
    AddWidget( fl_choice, true );

}

//==== Set Slider Value and Limits =====//
void Choice::SetValAndLimits( Parm* p )
{
    assert( m_Choice );
    if ( !p )
    {
        return;
    }

    IntParm* iparm = dynamic_cast< IntParm* >( p );
    assert( iparm );
    int val = iparm->Get();

    m_Choice->value( val );

    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Update( p->GetID() );
    }

}

//==== Get Current Choice Val ====//
void Choice::SetVal( int val )
{
    assert( m_Choice );
    m_Choice->value( val );
}

//==== Get Current Choice Val ====//
int Choice::GetVal()
{
    assert( m_Choice );
    return m_Choice->value();
}

 void Choice::UpdateItems()
 {
    //==== Add Choice Text ===//
    m_Choice->clear();
    for ( int i = 0 ; i < ( int )m_Items.size() ; i++ )
    {
        m_Choice->add( m_Items[i].c_str() );
    }
    m_Choice->value( 0 );
 }


//==== CallBack ====//
void Choice::DeviceCB( Fl_Widget* w )
{
    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );

    if ( w == m_Choice && parm_ptr )
    {
        int new_val = m_Choice->value();
        parm_ptr->SetFromDevice( new_val );
    }

    m_Screen->GuiDeviceCallBack( this );
}

//==== Set Total Width By Resizing First Widget Of List ====//
void Choice::SetWidth( int w )
{
    if ( m_WidgetVec.size() == 0 )
        return;

    int dev_w = w/(int)m_WidgetVec.size();

    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        m_WidgetVec[i]->size( dev_w, m_WidgetVec[i]->h() );

    }
}



//=====================================================================//
//===========       Fract Slider Input Input Combo          ===========//
//=====================================================================//

//==== Constructor ====//
FractParmSlider::FractParmSlider() : GuiDevice()
{
    m_Type = GDEV_FRACT_PARM_SLIDER;
    m_ResultFlInput = NULL;
    m_Format = string( " %7.5f" );
}


//==== Init ====//
void FractParmSlider::Init( VspScreen* screen, Fl_Slider* slider, Fl_Button* lbutton,
                            Fl_Button* rbutton, Fl_Input* fract_input, Fl_Input* result_input,
                            double range, const char* format, Fl_Button* parm_button  )
{
    GuiDevice::Init( screen );
    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    m_Slider.Init( screen, slider, lbutton, rbutton, range );
    m_FractInput.Init( screen, fract_input, format );
    m_ResultFlInput = result_input;
    m_ResultFlInput->callback( StaticDeviceCB, this );

    ClearAllWidgets();
    AddWidget( parm_button );
    AddWidget( lbutton );
    AddWidget( slider, true );
    AddWidget( rbutton );
    AddWidget( fract_input );
    AddWidget( result_input );
}

void FractParmSlider::Update( const string& parm_id )
{

    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( parm_id );
    if ( !parm_ptr )
    {
        Deactivate();
        return;
    }

    SetValAndLimits( parm_ptr );

    //if ( parm_ptr->GetActiveFlag() )
    //  Activate();
    //else
    //  Deactivate();

    if ( parm_ptr->GetActiveFlag() )
    {
        m_ResultFlInput->activate();
    }
    else
    {
        m_ResultFlInput->deactivate();
    }


    m_Slider.Update( parm_id );
    m_FractInput.Update( parm_id );
    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Update( parm_id );
    }
}


//==== Set Input Value and Limits =====//
void FractParmSlider::SetValAndLimits( Parm* parm_ptr )
{
    assert( m_ResultFlInput );

    FractionParm* fract_parm_ptr = dynamic_cast< FractionParm* > ( parm_ptr );
    assert( fract_parm_ptr );

    double new_val = fract_parm_ptr->GetResult();

    if ( CheckValUpdate( new_val ) )
    {
        sprintf( m_Str, m_Format.c_str(), new_val );
        m_ResultFlInput->value( m_Str );
    }
}

//==== CallBack ====//
void FractParmSlider::DeviceCB( Fl_Widget* w )
{
    assert( m_Screen );

    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( !parm_ptr )
    {
        return;
    }

    if ( w == m_ResultFlInput )
    {
        double new_val = atof( m_ResultFlInput->value() );

        FractionParm* fract_parm_ptr = dynamic_cast< FractionParm* > ( parm_ptr );
        assert( fract_parm_ptr );

        fract_parm_ptr->SetResultFromDevice( new_val );
        m_LastVal = new_val;
    }

    m_Screen->GuiDeviceCallBack( this );
}

////=====================================================================//
////===============       Radio Button Group      =======================//
////=====================================================================//
//
////==== Constructor ====//
//RadioButtonGroup::RadioButtonGroup()
//{
//}
//
////==== Destructor ====//
//RadioButtonGroup::~RadioButtonGroup()
//{
//  for ( int i = 0 ; i < (int)m_buttonVec.size() ; i ++ )
//      delete m_buttonVec[i];
//}
//
////==== Add Button to Vector ====//
//void RadioButtonGroup::AddButton( VspScreen* screen, Fl_Round_Button* button, int value )
//{
//  m_buttonVec.push_back(new RadioButton());
//  m_buttonVec.back()->Init(screen, button, value);
//}
//
////==== Update ====//
//void RadioButtonGroup::Update(const string& parm_id)
//{
//  for ( int i = 0 ; i < (int)m_buttonVec.size() ; i++ )
//      m_buttonVec[i]->Update( parm_id );
//}
//
////==== Activate ====//
//void RadioButtonGroup::Activate()
//{
//  for ( int i = 0 ; i < (int)m_buttonVec.size() ; i++ )
//      m_buttonVec[i]->Activate();
//}
//
////==== Deactivate ====///
//void RadioButtonGroup::Deactivate()
//{
//  for ( int i = 0 ; i < (int)m_buttonVec.size() ; i++ )
//      m_buttonVec[i]->Deactivate();
//}

//=====================================================================//
//===========      String Input                             ===========//
//=====================================================================//

void StringInput::Init( VspScreen* screen, Fl_Input* input )
{
    GuiDevice::Init( screen );
    AddWidget(input);
    m_Type = GDEV_STRING_INPUT;
    GuiDevice::Init( screen );

    assert( input );
    m_Input = input;
    m_Input->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Input->callback( StaticDeviceCB, this );
}

//==== Update ====//
void StringInput::Update( const string & val )
{
    m_String = val;
    m_Input->value( m_String.c_str() );
}

//==== CallBack ====//
void StringInput::DeviceCB( Fl_Widget* w )
{
    if ( w == m_Input )
    {
        m_String = string( m_Input->value() );
        m_Screen->GuiDeviceCallBack( this );
    }
}


//=====================================================================//
//===========      String Output                            ===========//
//=====================================================================//
void StringOutput::Init( VspScreen* screen, Fl_Output* output )
{
    GuiDevice::Init( screen );

    assert( output );
    m_Output = output;

    AddWidget( output );
}

//==== Update ====//
void StringOutput::Update( const string & val )
{
    m_String = val;
    m_Output->value( m_String.c_str() );
}

//=====================================================================//
//===========       Index Selector                          ===========//
//=====================================================================//
IndexSelector::IndexSelector()
{
    m_Type = GDEV_INDEX_SELECTOR;
    m_Screen = NULL;
    m_Input = NULL;
    m_Index = 0;
    m_MinIndex = 0;
    m_MaxIndex = 1000000;

    m_SmallInc = 1;
    m_BigInc = 10;
}

void IndexSelector::Init( VspScreen* screen, Fl_Button* ll_but,  Fl_Button* l_but,
                          Fl_Int_Input* input, Fl_Button* r_but, Fl_Button* rr_but )
{
    GuiDevice::Init( screen );

    m_LLButton = ll_but;
    m_LButton  = l_but;
    m_Input    = input;
    m_RButton  = r_but;
    m_RRButton = rr_but;

    SetIndex( m_Index );

    m_LLButton->callback( StaticDeviceCB, this );
    m_LButton->callback( StaticDeviceCB, this );
    m_Input->callback( StaticDeviceCB, this );
    m_RButton->callback( StaticDeviceCB, this );
    m_RRButton->callback( StaticDeviceCB, this );

    AddWidget( ll_but );
    AddWidget( l_but );
    AddWidget( input, true );
    AddWidget( r_but );
    AddWidget( rr_but );
}

//==== Set Total Width By Resizing First Widget Of List ====//
void IndexSelector::SetWidth( int w )
{
    if ( m_WidgetVec.size() == 0 )
        return;

    int dev_w = w/5;

    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        m_WidgetVec[i]->size( dev_w, m_WidgetVec[i]->h() );

    }
}



void IndexSelector::SetIndex( int index )
{
    //==== Check Bounds ====//
    m_Index = index;
    if ( m_Index < m_MinIndex )
    {
        m_Index = m_MinIndex;
    }
    else if ( m_Index > m_MaxIndex )
    {
        m_Index = m_MaxIndex;
    }

    if ( m_Input  )
    {
        sprintf( m_Str, "   %d", m_Index );
        m_Input->value( m_Str );
    }
}

void IndexSelector::SetMinMaxLimits( int min, int max )
{
    m_MinIndex = min;
    m_MaxIndex = max;
    SetIndex( m_Index );
}

void IndexSelector::SetBigSmallIncrements( int big_inc, int small_inc )
{
    m_BigInc = big_inc;
    m_SmallInc = small_inc;
}

void IndexSelector::DeviceCB( Fl_Widget* w )
{
    int ind = m_Index;
    if ( w == m_Input )
    {
        ind = atoi( m_Input->value() );
    }
    else if ( w == m_LLButton )
    {
        ind -= m_BigInc;
    }
    else if ( w == m_LButton )
    {
        ind -= m_SmallInc;
    }
    else if ( w == m_RButton )
    {
        ind += m_SmallInc;
    }
    else if ( w == m_RRButton )
    {
        ind += m_BigInc;
    }

    SetIndex( ind );

    //==== Set ParmID And Check For Valid ParmPtr ====//
    Parm* parm_ptr = SetParmID( m_ParmID );
    if ( parm_ptr )
    {
        parm_ptr->SetFromDevice( m_Index );
    }

    m_Screen->GuiDeviceCallBack( this );
}

void IndexSelector::SetValAndLimits( Parm* parm_ptr )
{
    assert( m_Input );

    int min_limit = (int)(parm_ptr->GetLowerLimit() + 0.5 );
    int max_limit = (int)(parm_ptr->GetUpperLimit() + 0.5 );

    SetMinMaxLimits( min_limit, max_limit );

    int new_val = ( int )( parm_ptr->Get() + 0.5 );

    if ( CheckValUpdate( new_val ) )
    {
        SetIndex( new_val );
    }
    m_LastVal = new_val;
}


//=====================================================================//
//===========       Color Picker                            ===========//
//=====================================================================//
ColorPicker::ColorPicker()
{
    m_Type = GDEV_COLOR_PICKER;
    m_Screen = NULL;
}

void ColorPicker::Init( VspScreen* screen, Fl_Button* title, Fl_Button* result,
                        vector< Fl_Button* > buttons, Fl_Slider* rgb_sliders[3] )
{
    GuiDevice::Init( screen );
    m_Color = vec3d( 0, 0, 255 );
    m_ColorResult = result;
    m_ColorButtons = buttons;
    m_RGB_Sliders[0] = rgb_sliders[0];
    m_RGB_Sliders[1] = rgb_sliders[1];
    m_RGB_Sliders[2] = rgb_sliders[2];

    for ( int i = 0 ; i < ( int )m_ColorButtons.size() ; i++ )
    {
        m_ColorButtons[i]->callback( StaticDeviceCB, this );
    }
    m_RGB_Sliders[0]->callback( StaticDeviceCB, this );
    m_RGB_Sliders[1]->callback( StaticDeviceCB, this );
    m_RGB_Sliders[2]->callback( StaticDeviceCB, this );
}

vec3d ColorPicker::GetIndexRGB( int index )
{
    if ( index == 0 )
    {
        return vec3d( 255, 0, 0 );
    }
    if ( index == 1 )
    {
        return vec3d( 0, 255, 0 );
    }
    if ( index == 2 )
    {
        return vec3d( 0, 0, 255 );
    }
    if ( index == 3 )
    {
        return vec3d( 0, 255, 255 );
    }
    if ( index == 4 )
    {
        return vec3d( 128, 0, 0 );
    }
    if ( index == 5 )
    {
        return vec3d( 0, 128, 0 );
    }
    if ( index == 6 )
    {
        return vec3d( 0, 0, 128 );
    }
    if ( index == 7 )
    {
        return vec3d( 128, 128, 0 );
    }
    if ( index == 8 )
    {
        return vec3d( 0, 128, 128 );
    }
    if ( index == 9 )
    {
        return vec3d( 128, 0, 128 );
    }
    if ( index == 10 )
    {
        return vec3d( 128, 128, 128 );
    }
    if ( index == 11 )
    {
        return vec3d( 0, 0, 0 );
    }

    return vec3d( 0, 0, 0 );
}

void ColorPicker::Update( const vec3d& rgb )
{
    m_Color = rgb;
    m_RGB_Sliders[0]->value( ( int )m_Color[0] );
    m_RGB_Sliders[1]->value( ( int )m_Color[1] );
    m_RGB_Sliders[2]->value( ( int )m_Color[2] );
    m_ColorResult->color( fl_rgb_color( ( uchar )m_Color[0], ( uchar )m_Color[1], ( uchar )m_Color[2] ) );
    m_ColorResult->redraw();

}

void ColorPicker::DeviceCB( Fl_Widget* w )
{
    assert( m_Screen );

    //==== Check Color Buttons ====//
    for ( int i = 0 ; i < ( int )m_ColorButtons.size() ; i++ )
    {
        if ( w == m_ColorButtons[i] )
        {
            vec3d rgb = GetIndexRGB( i );
            Update( rgb );
        }
    }
    //==== Check Color Sliders ====//
    for ( int i = 0 ; i < 3 ; i++ )
    {
        if ( w == m_RGB_Sliders[i] )
        {
            int r = ( int )m_RGB_Sliders[0]->value();
            int g = ( int )m_RGB_Sliders[1]->value();
            int b = ( int )m_RGB_Sliders[2]->value();
            m_Color.set_xyz( r, g, b );

            m_ColorResult->color( fl_rgb_color( r, g, b ) );
            m_ColorResult->redraw();
        }
    }
    m_Screen->GuiDeviceCallBack( this );
}

//=====================================================================//
//===========       Parmater Picker                         ===========//
//=====================================================================//
ParmPicker::ParmPicker()
{
    m_Screen = NULL;
}

void ParmPicker::Init( VspScreen* screen, Fl_Choice* container_choice,
                       Fl_Choice* group_choice, Fl_Choice* parm_choice  )
{
    GuiDevice::Init( screen );

    m_ContainerChoice = container_choice;
    m_GroupChoice = group_choice;
    m_ParmChoice = parm_choice;

    m_ContainerChoice->callback( StaticDeviceCB, this );
    m_GroupChoice->callback( StaticDeviceCB, this );
    m_ParmChoice->callback( StaticDeviceCB, this );

}

void ParmPicker::Activate()
{
    m_ContainerChoice->activate();
    m_GroupChoice->activate();
    m_ParmChoice->activate();
}

void ParmPicker::Deactivate()
{
    m_ContainerChoice->deactivate();
    m_GroupChoice->deactivate();
    m_ParmChoice->deactivate();
}

void ParmPicker::Update( )
{
    int i;
    char str[256];

    LinkMgr.BuildLinkableParmData();

    if( m_ParmIDChoice.size() == 0 )
    {
        m_ParmIDChoice = LinkMgr.GetUserParmId( 0 );
    }

    //==== Container Names ====//
    m_ContainerChoice->clear();
    vector < string > containerNameVec;
    int ind = LinkMgr.GetCurrContainerVec( m_ParmIDChoice, containerNameVec );
    for ( i = 0 ; i < ( int )containerNameVec.size() ; i++ )
    {
        sprintf( str, "%d-%s", i,  containerNameVec[i].c_str() );
        m_ContainerChoice->add( str );
    }
    m_ContainerChoice->value( ind );

    //==== Group Names ====//
    m_GroupChoice->clear();
    vector < string > groupNameVec;
    ind = LinkMgr.GetCurrGroupNameVec( m_ParmIDChoice, groupNameVec );
    for ( i = 0 ; i < ( int )groupNameVec.size() ; i++ )
    {
        m_GroupChoice->add( groupNameVec[i].c_str() );
    }
    m_GroupChoice->value( ind );

    //==== Parm Names =====//
    m_ParmChoice->clear();
    vector < string > parmIDVec;
    ind = LinkMgr.GetCurrParmIDVec( m_ParmIDChoice, parmIDVec );
    vector< string > parmNameVec = FindParmNames( parmIDVec );
    for ( i = 0 ; i < ( int )parmNameVec.size() ; i++ )
    {
        m_ParmChoice->add( parmNameVec[i].c_str() );
    }
    m_ParmChoice->value( ind );

}

void ParmPicker::DeviceCB( Fl_Widget* w )
{
    assert( m_Screen );

    if ( w == m_ContainerChoice  ||
            w == m_GroupChoice ||
            w == m_ParmChoice )
    {
        m_ParmIDChoice = LinkMgr.PickParm(  m_ContainerChoice->value(), m_GroupChoice->value(), m_ParmChoice->value() );
        m_Screen->GetScreenMgr()->SetUpdateFlag( true );
    }

    m_Screen->GuiDeviceCallBack( this );
}

vector< string > ParmPicker::FindParmNames( vector< string > & parm_id_vec )
{
    vector< string > name_vec;
    for ( int i = 0 ; i < ( int )parm_id_vec.size() ; i++ )
    {
        string name;

        Parm* p = ParmMgr.FindParm( parm_id_vec[i] );
        if ( p )
        {
            name = p->GetName();
        }

        name_vec.push_back( name );
    }
    return name_vec;
}

//=====================================================================//
//===========       Driver Group Bank                       ===========//
//=====================================================================//
DriverGroupBank::DriverGroupBank()
{
    m_Screen = NULL;
    m_DriverGroup = NULL;
}

void DriverGroupBank::Init( VspScreen* screen, vector< vector < Fl_Button* > > buttons, vector< SliderAdjRangeInput* > sliders )
{
    GuiDevice::Init( screen );

    m_Buttons = buttons;
    m_Sliders = sliders;

    for( int i = 0; i < m_DriverGroup->GetNvar(); i++ )
    {
        for( int j = 0; j < m_DriverGroup->GetNchoice(); j++ )
        {
            m_Buttons[i][j]->callback( StaticDeviceCB, this );
        }
    }
}


void DriverGroupBank::DeviceCB( Fl_Widget* w )
{
    assert( m_Screen );

    int imatch, jmatch;
    if( WhichButton( w, imatch, jmatch ) )
    {
        vector< int > newchoices = m_DriverGroup->GetChoices();
        newchoices[jmatch] = imatch;

        if( m_DriverGroup->ValidDrivers( newchoices ) )
        {
            m_DriverGroup->SetChoice( jmatch, imatch );
        }
    }


    m_Screen->GuiDeviceCallBack( this );
}

void DriverGroupBank::Update( vector< string > & parm_ids )
{
     //==== Update Sliders ====//
    for ( int i = 0 ; i < (int)m_Sliders.size() ; i++ )
    {
        if ( i < (int)parm_ids.size() )
        {
            m_Sliders[i]->Update( parm_ids[i] );
        }
    }

    vector< int > checkchoices;
    vector< int > currchoices = m_DriverGroup->GetChoices();

    for( int i = 0; i < m_DriverGroup->GetNvar(); i++ )
    {
        if( vector_contains_val( currchoices, i ) )
        {
            m_Sliders[i]->Activate();
        }
        else
        {
            m_Sliders[i]->Deactivate();
        }

        for( int j = 0; j < m_DriverGroup->GetNchoice(); j++ )
        {
            checkchoices = m_DriverGroup->GetChoices();
            checkchoices[j] = i;

            if( m_DriverGroup->ValidDrivers( checkchoices ) )
            {
                m_Buttons[i][j]->activate();
            }
            else
            {
                m_Buttons[i][j]->deactivate();
            }

            if( currchoices[j] == i )
            {
                m_Buttons[i][j]->value( 1 );
            }
            else
            {
                m_Buttons[i][j]->value( 0 );
            }

        }
    }
}

void DriverGroupBank::Activate()
{

}

void DriverGroupBank::Deactivate()
{

}

bool DriverGroupBank::WhichButton( Fl_Widget *w, int &imatch, int &jmatch )
{
    imatch = -1;
    jmatch = -1;
    for( int i = 0; i < m_DriverGroup->GetNvar(); i++ )
    {
        for( int j = 0; j < m_DriverGroup->GetNchoice(); j++ )
        {
            if( w == m_Buttons[i][j] )
            {
                imatch = i;
                jmatch = j;
                return true;
            }
        }
    }
    return false;
}

//=====================================================================//
//===========       Skin Control Group                      ===========//
//=====================================================================//
SkinControl::SkinControl()
{
    m_Screen = NULL;

}

void SkinControl::Init( VspScreen* screen,
        Fl_Check_Button* setButtonL,
        Fl_Check_Button* setButtonEqual,
        Fl_Check_Button* setButtonR,
        Fl_Slider* sliderL,
        Fl_Slider* sliderR,
        Fl_Input* inputL,
        Fl_Input* inputR,
        Fl_Button* parm_button,
        double range, const char* format)
{
    GuiDevice::Init( screen );

    m_SetButtonL.Init( screen, setButtonL );
    m_SetButtonEqual.Init( screen, setButtonEqual );
    m_SetButtonR.Init( screen, setButtonR );

    m_SliderL.Init( screen, sliderL, range );
    m_SliderR.Init( screen, sliderR, range );

    m_InputL.Init( screen, inputL, format );
    m_InputR.Init( screen, inputR, format );

    m_ParmButton.Init( screen, parm_button );
}


void SkinControl::Update( const string& parmL_id, const string& setL_id, const string& eq_id,
        const string& setR_id, const string& parmR_id )
{
    m_SliderL.Update( parmL_id );
    m_InputL.Update( parmL_id );

    m_SetButtonL.Update( setL_id );
    m_SetButtonEqual.Update( eq_id );
    m_SetButtonR.Update( setR_id );

    m_SliderR.Update( parmR_id );
    m_InputR.Update( parmR_id );

    m_ParmButton.Update( parmL_id );

    Parm* setL = ParmMgr.FindParm( setL_id );
    Parm* eq = ParmMgr.FindParm( eq_id );
    Parm* setR = ParmMgr.FindParm( setR_id );

    if( !setL->Get() )
    {
        this->DeactivateLeft();
        this->DeactivateEqual();
    }
    else if( eq->Get() )
    {
        this->DeactivateRSet();
    }

    if( !setR->Get() ) this->DeactivateRight();

    if( eq->Get() ) this->DeactivateRight();

}

void SkinControl::Activate()
{
    m_SetButtonL.Activate();
    m_SetButtonEqual.Activate();
    m_SetButtonR.Activate();

    m_SliderL.Activate();
    m_InputL.Activate();
    m_SliderR.Activate();
    m_InputR.Activate();

    m_ParmButton.Activate();
}

void SkinControl::Deactivate()
{
    m_SetButtonL.Deactivate();
    m_SetButtonEqual.Deactivate();
    m_SetButtonR.Deactivate();

    m_SliderL.Deactivate();
    m_InputL.Deactivate();
    m_SliderR.Deactivate();
    m_InputR.Deactivate();

    m_ParmButton.Deactivate();
}

void SkinControl::DeactivateLeft()
{
    m_SliderL.Deactivate();
    m_InputL.Deactivate();
}

void SkinControl::DeactivateRight()
{
    m_SliderR.Deactivate();
    m_InputR.Deactivate();
}

void SkinControl::DeactivateEqual()
{
	m_SetButtonEqual.Deactivate();
}

void SkinControl::DeactivateSet()
{
    m_SetButtonL.Deactivate();
    m_SetButtonR.Deactivate();
}

void SkinControl::DeactivateLSet()
{
    m_SetButtonL.Deactivate();
}

void SkinControl::DeactivateRSet()
{
    m_SetButtonR.Deactivate();
}



//==== CallBack ====//
void SkinControl::DeviceCB( Fl_Widget* w )
{

    m_Screen->GuiDeviceCallBack( this );
}


//=====================================================================//
//===========       Skin Header Group                       ===========//
//=====================================================================//
SkinHeader::SkinHeader()
{
    m_Screen = NULL;

    m_ContChoice = NULL;
}

void SkinHeader::Init( VspScreen* screen,
        Choice* cont_choice , const vector< Fl_Button* > &buttons )
{
    GuiDevice::Init( screen );

    m_ContChoice = cont_choice;

    m_Buttons = buttons;
}

void SkinHeader::Activate()
{
    assert( m_ContChoice );
    m_ContChoice->Activate();

    for( int i = 0; i < (int)m_Buttons.size(); i++ )
    {
        assert( m_Buttons[i] );
        m_Buttons[i]->activate();
    }
}

void SkinHeader::Deactivate()
{
    assert( m_ContChoice );
    m_ContChoice->Deactivate();

    for( int i = 0; i < (int)m_Buttons.size(); i++ )
    {
        assert( m_Buttons[i] );
        m_Buttons[i]->deactivate();
    }
}

void SkinHeader::DeactiveContChoice()
{
    assert( m_ContChoice );
    m_ContChoice->Deactivate();
}


//==== CallBack ====//
void SkinHeader::DeviceCB( Fl_Widget* w )
{
    m_Screen->GuiDeviceCallBack( this );
}






//=====================================================================//
//===========       Group Device                            ===========//
//=====================================================================//
Group::Group()
{
    m_Type = GDEV_GROUP;
    m_Group = NULL;
}

void Group::Hide()
{
    if ( m_Group )
    {
        m_Group->hide();
    }
}
void Group::Show()
{
    if ( m_Group )
    {
        m_Group->show();
    }
}

//=====================================================================//
//===========       Tab Device                          ===========//
//=====================================================================//
Tab::Tab()
{
    m_Type = GDEV_TAB;
}

//=====================================================================//
//===========            Geom Picker                        ===========//
//=====================================================================//
GeomPicker::GeomPicker()
{
    m_Screen = NULL;
    m_Vehicle = VehicleMgr.GetVehicle();
}

void GeomPicker::Init( VspScreen* screen, Fl_Choice* geom_choice )
{
    GuiDevice::Init( screen );

    m_GeomChoice = geom_choice;

    m_GeomChoice->callback( StaticDeviceCB, this );
}

void GeomPicker::Activate()
{
    m_GeomChoice->activate();
}

void GeomPicker::Deactivate()
{
    m_GeomChoice->deactivate();
}

void GeomPicker::Update( )
{
    int i;

    //==== Load Geom Choice ====//
    m_GeomVec = m_Vehicle->GetGeomVec();

    if ( m_GeomVec.size() == 0 )
    {
        m_GeomIDChoice = string();
    }
    else
    {
        if ( !m_Vehicle->FindGeom( m_GeomIDChoice ) )
        {
            m_GeomIDChoice = m_GeomVec[0];
        }
    }

    m_GeomChoice->clear();
    int ind = 0;
    for ( i = 0 ; i < ( int )m_GeomVec.size() ; i++ )
    {
        char str[256];
        sprintf( str, "%d_%s", i, m_Vehicle->FindGeom( m_GeomVec[i] )->GetName().c_str() );
        m_GeomChoice->add( str );
        if ( m_GeomIDChoice == m_GeomVec[i] )
        {
            ind = i;
        }
    }
    m_GeomChoice->value( ind );
}

void GeomPicker::DeviceCB( Fl_Widget* w )
{
    assert( m_Screen );

    if ( w == m_GeomChoice )
    {
        m_GeomIDChoice = m_GeomVec[ m_GeomChoice->value() ];
        m_Screen->GetScreenMgr()->SetUpdateFlag( true );
    }

    m_Screen->GuiDeviceCallBack( this );
}
