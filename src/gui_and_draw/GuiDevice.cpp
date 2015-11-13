//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GuiDevice.h"
#include "ParmMgr.h"
#include "ScreenBase.h"
#include "ScreenMgr.h"
#include "LinkMgr.h"
#include "AdvLinkMgr.h"
#include "StlHelper.h"
#include "Util.h"

using std::max;
using std::min;

VspSlider::VspSlider(int x, int y, int w, int h, const char *label ):Fl_Slider(x, y, w, h, label)
{
    m_ButtonPush = false;
}

int VspSlider::handle(int event)
{
    if ( event == FL_PUSH )
        m_ButtonPush = true;

    return Fl_Slider::handle( event );
}

VspButton::VspButton(int x, int y, int w, int h, const char *label):Fl_Button( x, y, w, h, label )
{
}

int VspButton::handle(int event)
{
    int ret = Fl_Button::handle(event);

    switch ( event ) {
    case FL_DRAG:
        if( callback() )
        {
            do_callback();
        }
        ret = 1;
        break;

    default:
        break;
    }

    return ret;
}

Vsp_Group::Vsp_Group( int x, int y, int w, int h ) : Fl_Group( x, y, w, h )
{
    m_AllowDrop = false;
}

int Vsp_Group::handle(int event)
{
    if( m_AllowDrop )
    {
        switch ( event )
        {
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:
            return 1;
            break;
        case FL_PASTE:
            if( callback() )
            {
                do_callback();
            }
            return 1;
            break;
        }
    }

    return Fl_Group::handle(event);
}

Vsp_Canvas::Vsp_Canvas( int x, int y, int w, int h, const char *label ) : Ca_Canvas( x, y, w, h, label )
{

}

int Vsp_Canvas::handle( int event )
{
    switch ( event )
    {
    case FL_PUSH:
    case FL_RELEASE:
    case FL_DRAG:
        if( callback() )
        {
            do_callback();
        }
        return 1;
        break;
    }
    return Ca_Canvas::handle( event );
}

//=====================================================================//
//======================           Input         ======================//
//=====================================================================//

//==== Constructor ====//
GuiDevice::GuiDevice()
{
    m_Type = -1;
    m_Index = -1;
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
        if ( m_WidgetVec[i] )
        {
            m_WidgetVec[i]->activate();
        }
    }
}

//==== Deactivate ====//
void GuiDevice::Deactivate()
{
    for ( int i = 0 ; i < (int)m_WidgetVec.size() ; i++ )
    {
        if ( m_WidgetVec[i] )
        {
            m_WidgetVec[i]->deactivate();
        }
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

    if ( std::abs( val - m_LastVal ) < DBL_EPSILON )
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
void Input::Init( VspScreen* screen, Fl_Input* input, const char* format, VspButton* parm_button )
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
void Slider::Init( VspScreen* screen, Fl_Slider* sw, double range, bool log_slider )
{
    GuiDevice::Init( screen );

    m_LogSliderFlag = log_slider;

    //==== Create Vsp Slider and Hide Old One ====//
    m_Slider = new VspSlider( sw->x(), sw->y(), sw->w(), sw->h() );
    Fl_Group* parent = sw->parent();
    if ( parent )
        parent->add( m_Slider );
    m_Slider->parent( sw->parent() );
    m_Slider->type( sw->type() );
    m_Slider->box( sw->box() );
    m_Slider->slider( sw->slider() );
    m_Slider->slider_size( sw->slider_size() );
    m_Slider->color( sw->color() );
    m_Slider->color2( sw->color2() );

    sw->hide();

    AddWidget(m_Slider);
    SetRange( range );
    m_Slider->callback( StaticDeviceCB, this );

}

//==== Set Slider Value and Limits =====//
void Slider::SetValAndLimits( Parm* parm_ptr )
{
    double m_Tol = 0.000001;

    assert( m_Slider );
    double new_val = parm_ptr->Get();

    if ( m_NewParmFlag || new_val < ( m_MinBound - m_Tol ) || new_val > ( m_MaxBound + m_Tol ) )
    {
        m_MinBound = max( new_val - m_Range, parm_ptr->GetLowerLimit() );
        m_MaxBound = min( new_val + m_Range, parm_ptr->GetUpperLimit() );

        if ( m_LogSliderFlag )
        {
            m_Slider->bounds( log10( m_MinBound ), log10( m_MaxBound ) );
        }
        else
        {
            m_Slider->bounds( m_MinBound, m_MaxBound );
        }
    }

    if ( CheckValUpdate( new_val ) )
    {
        if ( m_LogSliderFlag )
        {
            m_Slider->value( log10( new_val ) );
        }
        else
        {
            m_Slider->value( new_val );
        }
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
        bool drag_flag = false;
        if ( m_Slider->GetButtonPush() )
        {
            m_Slider->SetButtonPush( false );
        }
        else
        {
            drag_flag = true;
        }

        double new_val;
        if ( m_LogSliderFlag )
        {
            new_val = pow( 10, m_Slider->value() );
           }
        else
        {
            new_val = m_Slider->value();
        }
        parm_ptr->SetFromDevice( new_val, drag_flag );
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
                           Fl_Button* rbutton, double range, bool log_slider )
{
    Slider::Init( screen, slider, range, log_slider );

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
        if ( std::abs( new_val - parm_ptr->GetLowerLimit() ) < m_Tol )
        {
            m_MinStopState = SAR_ABS_STOP;
        }
        if ( std::abs( new_val - parm_ptr->GetUpperLimit() ) < m_Tol )
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
        bool drag_flag = false;
        if ( m_Slider->GetButtonPush() )
        {
            m_Slider->SetButtonPush( false );
        }
        else
        {
            drag_flag = true;
        }
        double new_val = m_Slider->value();
        parm_ptr->SetFromDevice( new_val, drag_flag );
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

    if ( std::abs( val - parm_ptr->GetLowerLimit() ) < m_Tol )
    {
        m_MinStopState = SAR_ABS_STOP;
        m_MinButton->label( "|" );
    }
    else if ( std::abs( val - m_MinBound ) < m_Tol )
    {
        m_MinStopState = SAR_STOP;
        m_MinButton->label( "<" );
    }
    else
    {
        m_MinStopState = SAR_NO_STOP;
        m_MinButton->label( ">" );
    }
    if ( std::abs( val - parm_ptr->GetUpperLimit() ) < m_Tol )
    {
        m_MaxStopState = SAR_ABS_STOP;
        m_MaxButton->label( "|" );
    }
    else if ( std::abs( val - m_MaxBound ) < m_Tol )
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
//=================       Slider Input Combo     ======================//
//=====================================================================//

//==== Init ====//
void SliderInput::Init( VspScreen* screen, Fl_Slider* slider, Fl_Input* input,
                        double range, const char* format, VspButton* parm_button,
                        bool log_slider )
{
    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    m_Slider.Init( screen, slider, range, log_slider );

    m_Input.Init( screen, input, format );

    ClearAllWidgets();
    AddWidget( parm_button );
    AddWidget( slider, true );
    AddWidget( input );

}

void SliderInput::Update( const string& parm_id )
{
    m_Slider.Update( parm_id );

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
                                Fl_Button* rbutton, Fl_Input* input, double range, const char* format,
                                VspButton* parm_button, bool log_slider )
{
    m_Type = GDEV_SLIDER_ADJ_RANGE_INPUT;
    m_ParmButtonFlag = false;
    if ( parm_button )
    {
        m_ParmButtonFlag = true;
        m_ParmButton.Init( screen, parm_button );
    }

    m_Slider.Init( screen, slider, lbutton, rbutton, range, log_slider );
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
                                 double range, const char* format, VspButton* parm_button )
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
void ParmButton::Init( VspScreen* screen, VspButton* button )
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

    if ( LinkMgr.UsedInLink( parm_id ) )
        m_Button->color( fl_rgb_color( 180, 140, 140 ) );
    else if ( AdvLinkMgr.IsInputParm( parm_id ) )
        m_Button->color( fl_rgb_color( 180, 140, 140 ));
    else if ( AdvLinkMgr.IsOutputParm( parm_id ) )
        m_Button->color( fl_rgb_color( 180, 140, 140 ));
    else
        m_Button->color( FL_BACKGROUND_COLOR );

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
        switch ( Fl::event() ) {
        case FL_DRAG:
            Fl::copy( m_ParmID.c_str(), m_ParmID.size(), 0 );
            Fl::dnd();

            break;

        case FL_RELEASE:
            if( Fl::event_inside( w ) )
            {
                ParmMgr.SetActiveParm( m_ParmID );
                m_Screen->GetScreenMgr()->ShowScreen( ScreenMgr::VSP_PARM_SCREEN );
            }

            break;

        default:
            break;
        }
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

    BoolParm* iparm = dynamic_cast< BoolParm* >( p );
    if ( iparm->Get() == m_value )
    {
        m_Button->setonly();
    }
}

//==== Callback ====//
void RadioButton::DeviceCB( Fl_Widget* w )
{
    Parm* parm_ptr = SetParmID( m_ParmID );
//    if ( !parm_ptr )
//    {
//        return;
//    }

    if ( w == m_Button && parm_ptr)
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

    if ( w == m_Button && parm_ptr )
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
    AddWidget(button);
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

//==== Color ====//
void TriggerButton::SetColor( Fl_Color c )
{
    if ( m_Button )
    {
        m_Button->color( c );
        m_Button->damage( 1 );
    }
}

//==== Gets the underlying fl button ====//
Fl_Button* TriggerButton::GetFlButton()
{
    return m_Button;
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
void Counter::Init( VspScreen* screen, Fl_Counter* counter, VspButton* parm_button )
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
    m_Offset = 0;
}

//==== Init ====//
void Choice::Init( VspScreen* screen, Fl_Choice* fl_choice, VspButton* parm_button  )
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

    m_Choice->value( val - m_Offset );

    if ( m_ParmButtonFlag )
    {
        m_ParmButton.Update( p->GetID() );
    }

}

//==== Get Current Choice Val ====//
void Choice::SetVal( int val )
{
    assert( m_Choice );
    m_Choice->value( val - m_Offset );
}

//==== Get Current Choice Val ====//
int Choice::GetVal()
{
    assert( m_Choice );
    return m_Choice->value() + m_Offset;
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
        int new_val = m_Choice->value() + m_Offset;
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
                            double range, const char* format, VspButton* parm_button  )
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
    //jrg - I changed this because of a crash when input does not callback until a Fl_Choice is pressed
//    m_Input->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Input->when(  FL_WHEN_CHANGED );
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
//===========       Parmater Tree Picker                    ===========//
//=====================================================================//
ParmTreePicker::ParmTreePicker()
{
    m_Screen = NULL;
    m_EventType = NONE;
    m_EventParm = "";
}

void ParmTreePicker::Init( VspScreen* screen, Fl_Tree* parm_tree )
{
    GuiDevice::Init( screen );

    m_ParmTree = parm_tree;
    m_ParmTree->showroot( 0 );
    m_ParmTree->sortorder( FL_TREE_SORT_ASCENDING );
    m_ParmTree->callback( StaticDeviceCB, this );
    m_ParmTree->selectmode( FL_TREE_SELECT_NONE );

    m_ParmTreePrefs.sortorder( FL_TREE_SORT_ASCENDING );
    m_ParmTreePrefs.selectmode( FL_TREE_SELECT_NONE );
}

void ParmTreePicker::Activate()
{
    m_ParmTree->activate();
}

void ParmTreePicker::Deactivate()
{
    m_ParmTree->deactivate();
}

void ParmTreePicker::Update( const vector< string > &selected_ids )
{
    UpdateParmTree();
    UnselectAll();

    ContainerTreeIt cit;
    ParmTreeIt pit;

    for ( int i = 0; i < selected_ids.size(); i++ )
    {
        string pID = selected_ids[i];
        Parm *p = ParmMgr.FindParm( pID );
        if ( p )
        {
            string cID = p->GetContainerID();

            ParmContainer* pc = ParmMgr.FindParmContainer( cID );
            while ( pc )
            {
                pc = pc->GetParentContainerPtr();
                if ( pc )
                {
                    cID = pc->GetID();
                }
            }

            cit = m_TreeData.find( cID );

            if ( cit != m_TreeData.end() )
            {
                pit = (*cit).second.m_ParmMap.find( pID );
                if ( pit != (*cit).second.m_ParmMap.end() )
                {
                    (*pit).second.m_Button->value( 1 );
                }
            }
        }
    }
}

void ParmTreePicker::UnselectAll()
{
    ContainerTreeIt cit;
    ParmTreeIt pit;
    for ( cit = m_TreeData.begin(); cit != m_TreeData.end(); ++cit )
    {
        for ( pit = (*cit).second.m_ParmMap.begin(); pit != (*cit).second.m_ParmMap.end(); ++pit )
        {
            (*pit).second.m_Button->value( 0 );
        }
    }
}

void ParmTreePicker::DeviceCB( Fl_Widget* w )
{
    assert( m_Screen );

    m_EventType = NONE;
    m_EventParm = "";

    ContainerTreeIt cit;
    ParmTreeIt pit;
    bool done = false;
    for ( cit = m_TreeData.begin(); cit != m_TreeData.end(); ++cit )
    {
        for ( pit = (*cit).second.m_ParmMap.begin(); pit != (*cit).second.m_ParmMap.end(); ++pit )
        {
            if ( w == (*pit).second.m_Button )
            {
                int new_val = (*pit).second.m_Button->value();

                if ( new_val == 0 )
                {
                    m_EventType = UNSELECT;
                }
                else
                {
                    m_EventType = SELECT;
                }

                m_EventParm = (*pit).first;

                done = true;
                break;
            }
        }
        if ( done )
        {
            break;
        }
    }

    m_Screen->GuiDeviceCallBack( this );
}

void ParmTreePicker::RemoveParm( const string &ParmID, ContainerTreeData &ctd)
{
    ParmTreeIt it;
    it =  ctd.m_ParmMap.find( ParmID );
    if ( it != ctd.m_ParmMap.end() )
    {
        Parm *p = ParmMgr.FindParm( ParmID );
        if ( p )
        {
            string groupname = p->GetDisplayGroupName();

            vector < int > groupids = FindGroup( groupname, ctd.m_GroupVec );
            if ( groupids.size() != 0 )
            {
                int groupid = groupids[0];

                if ( (*it).second.m_TreeItemPtr )
                {
                    ctd.m_GroupVec[groupid].m_TreeItemPtr->remove_child( (*it).second.m_TreeItemPtr );
                }
            }
        }
        ctd.m_ParmMap.erase( it );
    }
}

int ParmTreePicker::FindGroup( Fl_Tree_Item* groupitem, const GroupTreeVec &m_GroupVec )
{
    for ( int i = 0; i < m_GroupVec.size(); i++ )
    {
        if ( m_GroupVec[i].m_TreeItemPtr == groupitem )
        {
            return i;
        }
    }
    return -1;
}

vector < int > ParmTreePicker::FindGroup( const string &GroupName, const GroupTreeVec &m_GroupVec )
{
    vector < int > retvec;

    for ( int i = 0; i < m_GroupVec.size(); i++ )
    {
          if ( m_GroupVec[i].m_GroupName == GroupName )
           {
               retvec.push_back( i );
        }
    }
    return retvec;
}

void ParmTreePicker::CleanGroupVec( GroupTreeVec &m_GroupVec )
{
    GroupTreeVec copyvec;
    for ( int i = 0; i < m_GroupVec.size(); i++ )
    {
        if ( m_GroupVec[i].m_Flag )
        {
            if ( m_GroupVec[i].m_TreeItemPtr )
            {
                m_ParmTree->remove( m_GroupVec[i].m_TreeItemPtr );
            }
        }
        else
        {
            copyvec.push_back( m_GroupVec[i] );
        }
    }
    m_GroupVec = copyvec;
}

void ParmTreePicker::RemoveContainer( const string &ContID )
{
    ContainerTreeIt it;

    it =  m_TreeData.find( ContID );
    if ( it != m_TreeData.end() )
    {
        if ( (*it).second.m_TreeItemPtr )
        {
            m_ParmTree->remove( (*it).second.m_TreeItemPtr );
        }
        m_TreeData.erase( it );
    }
}

void ParmTreePicker::AddParmEntry( const string &ParmID, ContainerTreeData &conttree, Fl_Tree_Item* groupitem )
{
    ParmTreeData td;
    Parm* p = ParmMgr.FindParm( ParmID );

    td.m_TreeItemPtr = groupitem->add( m_ParmTreePrefs, "" );
    td.m_Flag = false;

    td.m_Button = new Fl_Check_Button( 1, 1, 100, 20 );
    td.m_Button->copy_label( p->GetName().c_str() );
    td.m_Button->callback( StaticDeviceCB, this );
    td.m_TreeItemPtr->widget( td.m_Button );

    ( (Fl_Group * ) m_ParmTree)->add( td.m_Button );

    conttree.m_ParmMap.insert( std::make_pair( ParmID, td ) );
}

void ParmTreePicker::AddGroupEntry( const string &GroupName, ContainerTreeData &conttree )
{
    GroupTreeData td;

    td.m_GroupName = GroupName;
    td.m_Flag = false;
    td.m_TreeItemPtr = conttree.m_TreeItemPtr->add( m_ParmTreePrefs, GroupName.c_str() );
    td.m_TreeItemPtr->close();
    conttree.m_GroupVec.push_back( td );
}

void ParmTreePicker::AddContEntry( const string &ContID )
{
    ContainerTreeData td;

    ParmContainer* pc = ParmMgr.FindParmContainer( ContID );

    Fl_Tree_Item* root = m_ParmTree->root();

    Fl_Tree_Item* ci = root->add( m_ParmTreePrefs, pc->GetName().c_str() );

    ci->close();
    for ( int i = 0; i < ci->children(); i++ )
    {
        ci->child( i )->close();
    }

    td.m_TreeItemPtr = ci;
    td.m_Flag = false;

    m_TreeData.insert( std::make_pair( ContID, td ) );
}

void ParmTreePicker::ResetFlag( bool flg )
{
    ContainerTreeIt cit;
    ParmTreeIt pit;

    for ( cit = m_TreeData.begin(); cit != m_TreeData.end(); ++cit )
    {
        (*cit).second.m_Flag = flg;

        for ( int i = 0; i < (*cit).second.m_GroupVec.size(); i++ )
        {
            (*cit).second.m_GroupVec[i].m_Flag = flg;
        }

        for ( pit = (*cit).second.m_ParmMap.begin(); pit != (*cit).second.m_ParmMap.end(); ++pit )
        {
            (*pit).second.m_Flag = flg;
        }
    }
}

void ParmTreePicker::CleanGarbage()
{
    vector< string > parmlist;
    vector< string > contlist;

    ContainerTreeIt cit;
    ParmTreeIt pit;

    for ( cit = m_TreeData.begin(); cit != m_TreeData.end(); ++cit )
    {
        parmlist.clear();
        if( (*cit).second.m_Flag )
        {
            contlist.push_back( (*cit).first );
        }

        for ( pit = (*cit).second.m_ParmMap.begin(); pit != (*cit).second.m_ParmMap.end(); ++pit )
        {
            if( (*pit).second.m_Flag )
            {
                parmlist.push_back( (*pit).first );
            }
        }

        // Remove parameters first.
        for ( int i = 0; i < (int) parmlist.size(); i++ )
        {
            RemoveParm( parmlist[i], (*cit).second );
        }

        // Remove groups next.
        CleanGroupVec( (*cit).second.m_GroupVec );
    }

    // Remove containers last.
    for ( int i = 0; i < (int) contlist.size(); i++ )
    {
        RemoveContainer( contlist[i] );
    }
}

void ParmTreePicker::UpdateParmTree()
{
    int i, j;

    ResetFlag( true );

    LinkMgr.BuildLinkableParmData();

    vector< string > containerVec;
    LinkMgr.GetAllContainerVec( containerVec );

    for ( i = 0 ; i < ( int )containerVec.size() ; i++ )
    {
        string cID = containerVec[i];

        ParmContainer* pc = ParmMgr.FindParmContainer( cID );
        if ( pc )
        {
            ContainerTreeIt cit = m_TreeData.find( cID );

            if ( cit == m_TreeData.end() )
            {
                AddContEntry( cID );
                cit = m_TreeData.find( cID );
            }

            // Verify container (Geom) name is correct.
            if ( (*cit).second.m_TreeItemPtr )
            {
                if ( strcmp( (*cit).second.m_TreeItemPtr->label(), pc->GetName().c_str() ) != 0 )
                {
                    (*cit).second.m_TreeItemPtr->label( pc->GetName().c_str() );
                }
                (*cit).second.m_Flag = false;
            }

            vector< string > parms;
            pc->AddLinkableParms( parms );
            // Verify tree group names for active parms are correct.
            ParmTreeIt pit;
            for ( j = 0; j < ( int ) parms.size() ; j++ )
            {
                string pID = parms[j];

                pit =  (*cit).second.m_ParmMap.find( pID );
                if ( pit != (*cit).second.m_ParmMap.end() )
                {
                    Parm *p = ParmMgr.FindParm( pID );
                    if ( p )
                    {
                        (*pit).second.m_Flag = false;

                        Fl_Tree_Item* pi = (*pit).second.m_TreeItemPtr;
                        if ( pi )
                        {
                            Fl_Tree_Item* gi = pi->parent();
                            if ( gi )
                            {
                                string groupname = p->GetDisplayGroupName();

                                if ( strcmp( gi->label(), groupname.c_str() ) != 0 )
                                {
                                    gi->label( groupname.c_str() );
                                }
                            }
                        }
                    }
                }
            }

            // Verify vector of group names match those in tree
            for ( j = 0; j < (*cit).second.m_GroupVec.size(); j++ )
            {
                if ( (*cit).second.m_GroupVec[j].m_TreeItemPtr )
                {
                    if ( strcmp( (*cit).second.m_GroupVec[j].m_GroupName.c_str(), (*cit).second.m_GroupVec[j].m_TreeItemPtr->label() ) != 0 )
                    {
                        (*cit).second.m_GroupVec[j].m_GroupName = string( (*cit).second.m_GroupVec[j].m_TreeItemPtr->label() );
                    }
                }
            }

            // Loop over all parms, verifying group usage.
            for ( j = 0; j < ( int ) parms.size() ; j++ )
            {
                string pID = parms[j];
                Parm *p = ParmMgr.FindParm( parms[j] );
                if ( p )
                {
                    pit =  (*cit).second.m_ParmMap.find( pID );
                    if ( pit != (*cit).second.m_ParmMap.end() )
                    {
                        if ( (*pit).second.m_TreeItemPtr )
                        {
                            int imatch = FindGroup( (*pit).second.m_TreeItemPtr->parent(), (*cit).second.m_GroupVec );

                            if ( imatch >= 0 )
                            {
                                (*cit).second.m_GroupVec[imatch].m_Flag = false;
                            }
                        }
                    }
                }
            }

            // Clean up un-used groups.
            CleanGroupVec( (*cit).second.m_GroupVec );

            // Loop over parms, creating entries as needed.
            for ( j = 0; j < ( int ) parms.size() ; j++ )
            {
                string pID = parms[j];
                Parm *p = ParmMgr.FindParm( parms[j] );
                if ( p )
                {
                    pit =  (*cit).second.m_ParmMap.find( pID );
                    if ( pit == (*cit).second.m_ParmMap.end() )
                    {
                        string groupname = p->GetDisplayGroupName();

                        vector < int > groupids = FindGroup( groupname, (*cit).second.m_GroupVec );
                        if ( groupids.size() == 0 )
                        {
                            AddGroupEntry( groupname, (*cit).second );
                            groupids = FindGroup( groupname, (*cit).second.m_GroupVec );
                        }
                        int groupid = groupids[0];

                        if ( groupids.size() > 1 )
                        {
                            printf("Error: multiple same-name groups where there shouldn't be any. \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
                        }

                        AddParmEntry( pID, (*cit).second, (*cit).second.m_GroupVec[groupid].m_TreeItemPtr );
                    }
                    else
                    {
                        (*pit).second.m_Flag = false;
                    }
                }
            }
        }
    }

    CleanGarbage();
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
        Fl_Button* minButtonL,
        Fl_Button* maxButtonL,
        Fl_Slider* sliderR,
        Fl_Button* minButtonR,
        Fl_Button* maxButtonR,
        Fl_Input* inputL,
        Fl_Input* inputR,
        VspButton* parm_button,
        double range, const char* format)
{
    GuiDevice::Init( screen );

    m_SetButtonL.Init( screen, setButtonL );
    m_SetButtonEqual.Init( screen, setButtonEqual );
    m_SetButtonR.Init( screen, setButtonR );

    m_SliderL.Init( screen, sliderL, minButtonL, maxButtonL, range );
    m_SliderR.Init( screen, sliderR, minButtonR, maxButtonR, range );

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
        Choice* cont_choice , const vector< VspButton* > &buttons )
{
    GuiDevice::Init( screen );

    m_ContChoice = cont_choice;

    m_Buttons = buttons;
}

void SkinHeader::Activate()
{
    if ( !m_ContChoice )
    {
        return;
    }

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
    if ( !m_ContChoice )
    {
        return;
    }

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
    if ( !m_ContChoice )
    {
        return;
    }

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
    //==== Load Geom Choice ====//
    vector< string > allGeomVec = m_Vehicle->GetGeomVec();
    m_GeomVec.clear();
    m_GeomChoice->clear();
    for ( int i = 0 ; i < ( int )allGeomVec.size() ; i++ )
    {
        Geom* g = m_Vehicle->FindGeom( allGeomVec[i] );
        if ( g )
        {
            bool excludematch = false;

            for ( int j = 0; j < m_ExcludeTypes.size(); j++ )
            {
                if ( g->GetType().m_Type == m_ExcludeTypes[j] )
                {
                    excludematch = true;
                    break;
                }
            }

            if ( !excludematch )
            {
                bool includematch = false;

                if ( m_IncludeTypes.size() > 0 )
                {
                    for ( int j = 0; j < m_IncludeTypes.size(); j++ )
                    {
                        if ( g->GetType().m_Type == m_IncludeTypes[j] )
                        {
                            includematch = true;
                            break;
                        }
                    }
                }
                else
                {
                    includematch = true;
                }

                if ( includematch )
                {
                    m_GeomVec.push_back( allGeomVec[i] );

                    char str[256];
                    sprintf( str, "%d_%s", i, g->GetName().c_str() );
                    m_GeomChoice->add( str );
                }
            }
        }
    }


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

    int ind = 0;
    for ( int i = 0 ; i < ( int )m_GeomVec.size() ; i++ )
    {
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

void GeomPicker::AddExcludeType( int type )
{
    m_ExcludeTypes.push_back( type );
}

void GeomPicker::ClearExcludeType()
{
    m_ExcludeTypes.clear();
}

void GeomPicker::AddIncludeType( int type )
{
    m_IncludeTypes.push_back( type );
}

void GeomPicker::ClearIncludeType()
{
    m_IncludeTypes.clear();
}

//=====================================================================//
//===========            PCurve Editor                      ===========//
//=====================================================================//

PCurveEditor::PCurveEditor()
{
    m_Curve = NULL;

    m_canvas = NULL;
    m_PtLayout = NULL;

    m_LastHit = -1;

    m_FreezeAxis = false;
    m_DeleteActive = false;
    m_SplitActive = false;
}

void PCurveEditor::Init( VspScreen* screen, Vsp_Canvas* canvas, Fl_Scroll* ptscroll, Fl_Button *spbutton, Fl_Button *convbutton, Fl_Light_Button *deletebutton, Fl_Light_Button *splitpickbutton, GroupLayout *ptlayout )
{
    GuiDevice::Init( screen );

    m_canvas = canvas;
    m_PtScroll = ptscroll;
    m_SplitButton = spbutton;
    m_ConvertButton = convbutton;
    m_DeleteButton = deletebutton;
    m_SplitPickButton = splitpickbutton;
    m_PtLayout = ptlayout;

    m_canvas->callback( StaticDeviceCB, this );
    m_SplitButton->callback( StaticDeviceCB, this );
    m_DeleteButton->callback( StaticDeviceCB, this );
    m_SplitPickButton->callback( StaticDeviceCB, this );
    m_ConvertButton->callback( StaticDeviceCB, this );
}


void PCurveEditor::DeviceCB( Fl_Widget* w )
{
    if ( Fl::event_inside( m_canvas ) )
    {
    }

    if ( w == m_canvas )
    {
        int x = Fl::event_x();
        int y = Fl::event_y();

        if ( Fl::event() == FL_PUSH )
        {
            m_LastHit = ihit( x, y, 5 );

            if ( m_DeleteActive )
            {
                m_DeleteActive = false;

                if ( m_LastHit >= 0 )
                {
                    m_Curve->DeletePt( m_LastHit );
                }
            }

            if ( m_SplitActive )
            {
                m_SplitActive = false;

                double sx = m_canvas->current_x()->value( x );

                if ( sx > m_Curve->GetRFirst() && sx < m_Curve->GetRLast() )
                {
                    m_Curve->m_SplitPt = sx;
                    m_Curve->Split();
                }
            }
        }

        if ( Fl::event() == FL_DRAG && m_LastHit != -1 )
        {
            m_Curve->SetPt( m_canvas->current_x()->value( x ), m_canvas->current_y()->value( y ), m_LastHit );
            m_FreezeAxis = true;
        }

        if ( Fl::event() == FL_RELEASE )
        {
            m_FreezeAxis = false;
        }
    }
    else if ( w == m_SplitButton )
    {
        m_Curve->Split();
    }
    else if ( w == m_ConvertButton )
    {
        m_Curve->ConvertTo( m_Curve->m_ConvType() );
    }
    else if ( w == m_DeleteButton )
    {
        m_DeleteActive = !m_DeleteActive;
        if ( m_DeleteActive )
        {
            m_SplitActive = false;
        }
    }
    else if ( w == m_SplitPickButton )
    {
        m_SplitActive = !m_SplitActive;
        if ( m_SplitActive )
        {
            m_DeleteActive = false;
        }
    }

    m_Screen->GuiDeviceCallBack( this );
}


void PCurveEditor::Update( PCurve *curve )
{
    m_Curve = curve;
    Update();
}

void PCurveEditor::Update()
{
    if ( m_Curve && m_canvas )
    {
        Vsp_Canvas::current( m_canvas );
        m_canvas->clear();


        vector < double > xt;
        vector < double > yt;

        m_Curve->Tessellate( xt, yt );


        m_DeleteButton->value( m_DeleteActive );
        m_SplitPickButton->value( m_SplitActive );


        //add the data to the plot
        AddPointLine( xt, yt, 2, FL_BLUE );

        vector < double > xdata = m_Curve->GetTVec();
        vector < double > ydata = m_Curve->GetValVec();

        int ndata = xdata.size();

        if ( ndata > 0 )
        {
            if ( m_Curve->m_CurveType() != vsp::CEDIT )
            {
                AddPoint( xdata, ydata, FL_BLACK, 4, CA_DIAMOND );
            }
            else
            {
                int nseg = ( ndata - 1 ) / 3;

                vector < double > xend( nseg + 1 ); // Cubic segment endpoints
                vector < double > yend( nseg + 1 );

                vector < double > xmid( 2 * nseg ); // Cubic segment midpoints
                vector < double > ymid( 2 * nseg );

                int imid = 0;
                int iend = 0;

                for ( int i = 0; i < ndata; i++ )
                {
                    if ( ( i % 3 ) == 0)
                    {
                        xend[ iend ] = xdata[ i ];
                        yend[ iend ] = ydata[ i ];
                        iend++;

                        if ( i != 0 )
                        {
                            vector < double > xtan( 2 );
                            vector < double > ytan( 2 );
                            xtan[ 0 ] = xdata[ i - 1 ];
                            xtan[ 1 ] = xdata[ i ];
                            ytan[ 0 ] = ydata[ i - 1 ];
                            ytan[ 1 ] = ydata[ i ];

                            AddPointLine( xtan, ytan, 1, FL_GRAY0 );
                        }

                        if ( i != ndata - 1 )
                        {
                            vector < double > xtan( 2 );
                            vector < double > ytan( 2 );
                            xtan[ 0 ] = xdata[ i ];
                            xtan[ 1 ] = xdata[ i + 1 ];
                            ytan[ 0 ] = ydata[ i ];
                            ytan[ 1 ] = ydata[ i + 1 ];

                            AddPointLine( xtan, ytan, 1, FL_GRAY0 );
                        }
                    }
                    else
                    {
                        xmid[ imid ] = xdata[ i ];
                        ymid[ imid ] = ydata[ i ];
                        imid++;
                    }
                }

                AddPoint( xend, yend, FL_BLACK, 4, CA_DIAMOND );
                AddPoint( xmid, ymid, FL_GREEN, 4, CA_ROUND );
            }
        }

        if ( ! m_FreezeAxis )
        {
            double xmin, xmax, ymin, ymax;

            xmin = xt[0];
            xmax = xmin;
            ymin = yt[0];
            ymax = ymin;
            for ( int i = 1; i < xt.size(); i++ )
            {
                if ( xt[i] < xmin )
                {
                    xmin = xt[i];
                }
                if ( xt[i] > xmax )
                {
                    xmax = xt[i];
                }
                if ( yt[i] < ymin )
                {
                    ymin = yt[i];
                }
                if ( yt[i] > ymax )
                {
                    ymax = yt[i];
                }
            }

            for ( int i = 0; i < xdata.size(); i++ )
            {
                if ( xdata[i] < xmin )
                {
                    xmin = xdata[i];
                }
                if ( xdata[i] > xmax )
                {
                    xmax = xdata[i];
                }
                if ( ydata[i] < ymin )
                {
                    ymin = ydata[i];
                }
                if ( ydata[i] > ymax )
                {
                    ymax = ydata[i];
                }
            }

            xmin = magrounddn( xmin );
            xmax = magroundup( xmax );
            if ( xmin == xmax )
            {
                xmin -= 1;
                xmax += 1;
            }
            ymin = magrounddn( ymin );
            ymax = magroundup( ymax );
            if ( ymin == ymax )
            {
                ymin -= 1;
                ymax += 1;
            }

            m_canvas->current_x()->minimum( xmin );
            m_canvas->current_x()->maximum( xmax );
            m_canvas->current_x()->copy_label( m_Curve->GetXDsipName().c_str() );

            m_canvas->current_y()->minimum( ymin );
            m_canvas->current_y()->maximum( ymax );
            m_canvas->current_y()->copy_label( m_Curve->GetYDsipName().c_str() );
        }

        int n = m_Curve->GetNumPts();

        m_SplitPtSlider.Update( m_Curve->m_SplitPt.GetID() );

        m_ConvertChoice.Update( m_Curve->m_ConvType.GetID() );

        switch( m_Curve->m_CurveType() )
        {
        case vsp::LINEAR:
            m_CurveType.Update( "Linear" );
            break;
        case vsp::PCHIP:
            m_CurveType.Update( "Spline (PCHIP)" );
            break;
        case vsp::CEDIT:
            m_CurveType.Update( "Cubic Bezier" );
            break;
        }

        if ( n != m_XPtSliderVec.size() )
        {
            m_PtScroll->clear();
            m_PtLayout->SetGroup( m_PtScroll );
            m_PtLayout->InitWidthHeightVals();
            m_PtLayout->SetButtonWidth( 50 );

            m_XPtSliderVec.clear();
            m_YPtSliderVec.clear();

            m_XPtSliderVec.resize( n );
            m_YPtSliderVec.resize( n );

            m_PtLayout->SetSameLineFlag( true );

            for ( int i = 0; i < n; i++ )
            {
                m_PtLayout->AddSlider( m_XPtSliderVec[i], "AUTO_UPDATE", 2, "%9.5f", m_PtLayout->GetW() / 2.0 + 2 );
                m_PtLayout->AddX( 4 );
                m_PtLayout->AddSlider( m_YPtSliderVec[i], "AUTO_UPDATE", 2, "%9.5f" );
                m_PtLayout->ForceNewLine();
            }
        }


        for ( int i = 0; i < n; i++ )
        {
            Parm *p = m_Curve->m_TParmVec[i];
            if ( p )
            {
                m_XPtSliderVec[i].Update( p->GetID() );
            }
        }

        for ( int i = 0; i < n; i++ )
        {
            Parm *p = m_Curve->m_ValParmVec[i];
            if ( p )
            {
                m_YPtSliderVec[i].Update( p->GetID() );
            }
        }


    }
}

bool PCurveEditor::hittest( int mx, int my, double datax, double datay, int r )
{
    if ( m_canvas )
    {
        int dx = std::abs( m_canvas->current_x()->position( datax ) - mx );
        int dy = std::abs( m_canvas->current_y()->position( datay ) - my );

        if ( dx < r && dy < r )
        {
            return true;
        }
    }
    return false;
}

int PCurveEditor::ihit( int mx, int my, int r )
{
    vector < double > xdata = m_Curve->GetTVec();
    vector < double > ydata = m_Curve->GetValVec();

    for ( int i = 0; i < xdata.size(); i++ )
    {
        if ( hittest( mx, my, xdata[i], ydata[i], r ) )
        {
            return i;
        }
    }

    return -1;
}
