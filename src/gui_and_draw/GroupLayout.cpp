//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "GroupLayout.h"
#include "ScreenBase.h"
#include <FL/Fl_Value_Slider.H>

//==== Constructor ====//
GroupLayout::GroupLayout()
{
    Init();
    m_Screen = NULL;
    m_Group  = NULL;
}

//==== Constructor ====//
GroupLayout::GroupLayout( VspScreen* screen )
{
    Init();
    m_Screen = screen;
    m_Group  = NULL;
}

GroupLayout::GroupLayout( VspScreen* screen, Fl_Group* group )
{
    Init();
    m_Screen = screen;
    SetGroup( group );
}

//==== Constructor ====//
void GroupLayout::Init()
{
    m_FitWidthFlag = true;
    m_SameLineFlag = false;
    m_X = m_X = m_W = m_H = 0;
    m_StartX = m_StartY = 0;
    InitWidthHeightVals();
}

void GroupLayout::InitWidthHeightVals()
{
    m_StdHeight = 20;
    m_GapHeight = 6;
    m_DividerHeight = 15;
    m_ButtonWidth = 74;
    m_ChoiceButtonWidth = 110;
    m_RangeButtonWidth = 10;
    m_InputWidth = 60;
    m_SliderWidth = 110;
    m_CanvasWidth = 250;
    m_CanvasHeight = 250;

}


//==== Destructor ====//
GroupLayout::~GroupLayout()
{
    for ( int i = 0; i < m_Slider_Cleanup.size(); i++ )
    {
        delete m_Slider_Cleanup[i];
    }
    m_Slider_Cleanup.clear();
}

//==== Hide Group ====//
void GroupLayout::Hide()
{
    if ( m_Group )
    {
        m_Group->hide();
    }
}

//==== Show Group ====//
void GroupLayout::Show()
{
    if ( m_Group )
    {
        m_Group->show();
    }
}

//==== Get Ramaing X ======//
int GroupLayout::GetRemainX()
{
    return ( m_W + m_StartX ) - m_X;
}

//==== Get Ramaing Y ======//
int GroupLayout::GetRemainY()
{
    return ( m_H + m_StartY ) - m_Y;
}

//==== Add To X Position ======//
void GroupLayout::AddX( int offset )
{
    m_X += offset;
}

//==== Set X For NewLine ====//
void GroupLayout::NewLineX()
{
    if ( !m_SameLineFlag )
    {
        m_X = m_Group->x();
    }
}

//==== Set X And Y For NewLine Ignore Flags ====//
void GroupLayout::ForceNewLine()
{
    m_X = m_Group->x();
    m_Y += m_StdHeight;
}

//==== Add To Y Position ======//
void GroupLayout::AddY( int offset )
{
    if ( !m_SameLineFlag )
    {
        m_Y += offset;
    }
}

//==== Compute Width ======//
int GroupLayout::FitWidth( int used_w, int default_w )
{
    int w = default_w;
    if ( m_FitWidthFlag )
    {
        w = m_Group->w() -  used_w;

        Fl_Scroll* s = dynamic_cast< Fl_Scroll* >( m_Group );

        if ( s == NULL ) // Group is not a Fl_Scroll
        {
            return w;
        }

        int sw = s->scrollbar_size();

        if ( sw == 0 )  // Check size, if zero, global size is used.
        {
            sw = Fl::scrollbar_size();
        }

        w = w - sw;
    }

    return w;
}


//==== Set Group Ptr ====//
void GroupLayout::SetGroup( Fl_Group* group )
{
    m_Group = group;
    m_StartX = m_Group->x();
    m_StartY = m_Group->y();
    m_X = m_StartX;
    m_Y = m_StartY;
    m_W = m_Group->w();
    m_H = m_Group->h();
}
//==== Set Group And Screen ====//
void GroupLayout::SetGroupAndScreen( Fl_Group* group, VspScreen* screen )
{
    SetGroup( group );
    SetScreen( screen );
}

//==== Add Standard Parm Button With Label ====//
VspButton* GroupLayout::AddParmButton( const char* label )
{
    VspButton* button = new VspButton( m_X, m_Y, m_ButtonWidth, m_StdHeight );
    button->copy_label( label );
    button->box( FL_THIN_UP_BOX );
    button->labelfont( FL_HELVETICA_BOLD );
    button->labelsize( 12 );
    button->labelcolor( FL_BLACK );
    m_Group->add( button );
    AddX( m_ButtonWidth );

    return button;
}


//==== Create & Init Gui Slider Adjustable Range  ====//
void GroupLayout::AddSlider( SliderAdjRangeInput& slid_adj_input,
                             const char* label, double range, const char* format, int used_w, bool log_slider )

{
    assert( m_Group && m_Screen );

    int init_used_w = used_w + m_X - m_Group->x();

    //==== Parm Button ====//
    VspButton* button = AddParmButton( label );

    //==== Range Button ====//
    Fl_Repeat_Button* lbutton = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, "<" );
    lbutton->box( FL_THIN_UP_BOX );
    lbutton->labelcolor( ( Fl_Color )4 );
    m_Group->add( lbutton );
    AddX( m_RangeButtonWidth );

    //==== Slider ====//
    int sw = FitWidth( m_ButtonWidth + 2 * m_RangeButtonWidth + m_InputWidth + init_used_w, m_SliderWidth );
    Fl_Slider* slider = new Fl_Slider( m_X, m_Y, sw, m_StdHeight );
    slider->type( FL_HOR_NICE_SLIDER );
    slider->box( FL_THIN_DOWN_BOX );
    slider->color( FL_BACKGROUND2_COLOR );
    slider->selection_color( FL_SELECTION_COLOR );
    m_Group->add( slider );
    AddX( sw );

    //==== Range Button ====//
    Fl_Repeat_Button* rbutton = new Fl_Repeat_Button( m_X, m_Y,  m_RangeButtonWidth, m_StdHeight, "<" );
    rbutton->box( FL_THIN_UP_BOX );
    rbutton->labelcolor( ( Fl_Color )4 );
    m_Group->add( rbutton );
    AddX( m_RangeButtonWidth );

    //==== Input ====//
    Fl_Input* input = new Fl_Input( m_X, m_Y, m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    input->type( FL_FLOAT_INPUT );
#endif
    input->box( FL_THIN_DOWN_BOX );
    input->textsize( 12 );
    input->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( input );
    AddX( m_InputWidth );

    AddY( m_StdHeight );
    NewLineX();

    slid_adj_input.Init( m_Screen, slider, lbutton, rbutton, input, range, format, button, log_slider );

    if( strcmp( label, "AUTO_UPDATE" ) == 0 || strcmp( label, "" ) == 0 )
    {
        slid_adj_input.SetButtonNameUpdate( true );
    }
}

//==== Create & Init Gui Slider Adjustable Range  ====//
void GroupLayout::AddSlider( SliderAdjRange2Input& slid_adj_input,
                             const char* label, double range, const char* format )
{
    assert( m_Group && m_Screen );

    int init_used_w = m_X - m_Group->x();

    //==== Parm Button ====//
    VspButton* button = AddParmButton( label );

    //==== Range Button ====//
    Fl_Repeat_Button* lbutton = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, "<" );
    lbutton->box( FL_THIN_UP_BOX );
    lbutton->labelcolor( ( Fl_Color )4 );
    m_Group->add( lbutton );
    AddX( m_RangeButtonWidth );

    //==== Slider ====//
    int sw = FitWidth( m_ButtonWidth + 2 * m_RangeButtonWidth + 2 * m_InputWidth + init_used_w, m_SliderWidth );
    Fl_Slider* slider = new Fl_Slider( m_X, m_Y, sw, m_StdHeight );
    slider->type( FL_HOR_NICE_SLIDER );
    slider->box( FL_THIN_DOWN_BOX );
    slider->color( FL_BACKGROUND2_COLOR );
    slider->selection_color( FL_SELECTION_COLOR );
    m_Group->add( slider );
    AddX( sw );

    //==== Range Button ====//
    Fl_Repeat_Button* rbutton = new Fl_Repeat_Button( m_X, m_Y,  m_RangeButtonWidth, m_StdHeight, "<" );
    rbutton->box( FL_THIN_UP_BOX );
    rbutton->labelcolor( ( Fl_Color )4 );
    m_Group->add( rbutton );
    AddX( m_RangeButtonWidth );

    //==== Input ====//
    Fl_Input* input1 = new Fl_Input( m_X, m_Y, m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    input1->type( FL_FLOAT_INPUT );
#endif
    input1->box( FL_THIN_DOWN_BOX );
    input1->textsize( 12 );
    input1->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
    m_Group->add( input1 );
    AddX( m_InputWidth );

    Fl_Input* input2 = new Fl_Input( m_X, m_Y, m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    input2->type( FL_FLOAT_INPUT );
#endif
    input2->box( FL_THIN_DOWN_BOX );
    input2->textsize( 12 );
    input2->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
    m_Group->add( input2 );
    AddX( m_InputWidth );

    AddY( m_StdHeight );
    NewLineX();

    slid_adj_input.Init( m_Screen, slider, lbutton, rbutton, input1, input2, range, format, button );

    if( strcmp( label, "AUTO_UPDATE" ) == 0 || strcmp( label, "" ) == 0 )
    {
        slid_adj_input.SetButtonNameUpdate( true );
    }
}

//==== Create & Init Gui Slider Adjustable Range  ====//
void GroupLayout::AddSlider( FractParmSlider& slid_adj_input,
                             const char* label, double range, const char* format )
{
    assert( m_Group && m_Screen );

    int init_used_w = m_X - m_Group->x();

    //==== Parm Button ====//
    VspButton* button = AddParmButton( label );

    //==== Range Button ====//
    Fl_Repeat_Button* lbutton = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, "<" );
    lbutton->box( FL_THIN_UP_BOX );
    lbutton->labelcolor( ( Fl_Color )4 );
    m_Group->add( lbutton );
    AddX( m_RangeButtonWidth );

    //==== Slider ====//
    int sw = FitWidth( m_ButtonWidth + 2 * m_RangeButtonWidth + 2 * m_InputWidth + init_used_w, m_SliderWidth );
    Fl_Slider* slider = new Fl_Slider( m_X, m_Y, sw, m_StdHeight );
    slider->type( FL_HOR_NICE_SLIDER );
    slider->box( FL_THIN_DOWN_BOX );
    slider->color( FL_BACKGROUND2_COLOR );
    slider->selection_color( FL_SELECTION_COLOR );
    m_Group->add( slider );
    AddX( sw );

    //==== Range Button ====//
    Fl_Repeat_Button* rbutton = new Fl_Repeat_Button( m_X, m_Y,  m_RangeButtonWidth, m_StdHeight, "<" );
    rbutton->box( FL_THIN_UP_BOX );
    rbutton->labelcolor( ( Fl_Color )4 );
    m_Group->add( rbutton );
    AddX( m_RangeButtonWidth );

    //==== Input ====//
    Fl_Input* input1 = new Fl_Input( m_X, m_Y, m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    input1->type( FL_FLOAT_INPUT );
#endif
    input1->box( FL_THIN_DOWN_BOX );
    input1->textsize( 12 );
    input1->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( input1 );
    AddX( m_InputWidth );

    Fl_Input* input2 = new Fl_Input( m_X, m_Y, m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    input2->type( FL_FLOAT_INPUT );
#endif
    input2->box( FL_THIN_DOWN_BOX );
    input2->textsize( 12 );
    input2->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( input2 );
    AddX( m_InputWidth );

    AddY( m_StdHeight );
    NewLineX();

    slid_adj_input.Init( m_Screen, slider, lbutton, rbutton, input1, input2, range, format, button );

    if( strcmp( label, "AUTO_UPDATE" ) == 0 || strcmp( label, "" ) == 0 )
    {
        slid_adj_input.SetButtonNameUpdate( true );
    }
}


//==== Create & Init Gui Slider Input  ====//
void GroupLayout::AddSlider( SliderInput& slider_input, const char* label, double range, const char* format, int used_w, bool log_slider )
{
    assert( m_Group && m_Screen );

    int init_used_w = m_X - m_Group->x();

    //==== Parm Button ====//
    VspButton* button = AddParmButton( label );

    //==== Slider ====//
    int sw = FitWidth( m_ButtonWidth + m_InputWidth + init_used_w + used_w, m_SliderWidth );
    Fl_Slider* slider = new Fl_Slider( m_X, m_Y, sw, m_StdHeight );
    slider->type( FL_HOR_NICE_SLIDER );
    slider->box( FL_THIN_DOWN_BOX );
    slider->color( FL_BACKGROUND2_COLOR );
    slider->selection_color( FL_SELECTION_COLOR );
    m_Group->add( slider );
    AddX( sw );

    //==== Input ====//
    Fl_Input* input = new Fl_Input( m_X, m_Y,  m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    input->type( FL_FLOAT_INPUT );
#endif
    input->box( FL_THIN_DOWN_BOX );
    input->textsize( 12 );
    input->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( input );
    AddX( m_InputWidth );

    AddY( m_StdHeight );
    NewLineX();

    slider_input.Init( m_Screen, slider, input, range, format, button, log_slider );

    if( strcmp( label, "AUTO_UPDATE" ) == 0 || strcmp( label, "" ) == 0 )
    {
        slider_input.SetButtonNameUpdate( true );
    }
}

//==== Create & Init Gui CheckButton  ====//
void GroupLayout::AddButton( CheckButton& cbutton, const char* label )
{
    assert( m_Group && m_Screen );

    //==== Add Check Button ====//
    int bw = FitWidth( 0, m_ButtonWidth );
    Fl_Check_Button* flbutton = new Fl_Check_Button( m_X, m_Y, bw, m_StdHeight, label );
    flbutton->box( FL_DOWN_BOX );
    flbutton->down_box( FL_DOWN_BOX );
    flbutton->labelfont( FL_HELVETICA_BOLD );
    flbutton->labelsize( 12 );
    flbutton->labelcolor( FL_DARK_BLUE );
    flbutton->copy_label( label );
    m_Group->add( flbutton );
    AddX( bw );

    AddY( m_StdHeight );
    NewLineX();

    cbutton.Init( m_Screen, flbutton );
}

//==== Create & Init Gui ToggleButton  ====//
void GroupLayout::AddButton( ToggleButton& tbutton, const char* label )
{
    assert( m_Group && m_Screen );

    //==== Add Check Button ====//
    int bw = FitWidth( 0, m_ButtonWidth );
    Fl_Light_Button* flbutton = new Fl_Light_Button( m_X, m_Y, bw, m_StdHeight );
    flbutton->labelfont( FL_HELVETICA_BOLD );
    flbutton->labelsize( 12 );
    flbutton->align( Fl_Align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP ) );
    flbutton->copy_label( label );
    flbutton->labelcolor( FL_DARK_BLUE );
    flbutton->copy_label( label );
    m_Group->add( flbutton );
    AddX( bw );

    AddY( m_StdHeight );
    NewLineX();

    tbutton.Init( m_Screen, flbutton );
}

//==== Create & Init Gui ToggleButton  ====//
void GroupLayout::AddButton( CheckButtonBit& cbutton, const char* label, int value )
{
    assert( m_Group && m_Screen );

    //==== Add Check Button ====//
    int bw = FitWidth( 0, m_ButtonWidth );
    Fl_Light_Button* flbutton = new Fl_Light_Button( m_X, m_Y, bw, m_StdHeight, label );
    flbutton->labelfont( FL_HELVETICA_BOLD );
    flbutton->labelsize( 12 );
    flbutton->align( Fl_Align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP ) );
    flbutton->labelcolor( FL_DARK_BLUE );
    flbutton->copy_label( label );
    m_Group->add( flbutton );
    AddX( bw );

    AddY( m_StdHeight );
    NewLineX();

    cbutton.Init( m_Screen, flbutton, value );
}


//==== Create & Init Gui TriggerButton  ====//
void GroupLayout::AddButton( TriggerButton& tbutton, const char* label, int used_w )
{
    assert( m_Group && m_Screen );

    //==== Add Check Button ====//
    int bw = FitWidth( used_w, m_ButtonWidth );
    Fl_Button* flbutton = new Fl_Button( m_X, m_Y, bw, m_StdHeight, label );
    flbutton->labelfont( FL_HELVETICA_BOLD );
    flbutton->labelsize( 12 );
    flbutton->labelcolor( FL_DARK_BLUE );
    flbutton->copy_label( label );
    m_Group->add( flbutton );
    AddX( bw );

    AddY( m_StdHeight );
    NewLineX();

    tbutton.Init( m_Screen, flbutton );
}

//==== Create & Init Gui ParmButton  ====//
void GroupLayout::AddButton( ParmButton& pbutton, const char* label )
{
    assert( m_Group && m_Screen );

    //==== Add Parm Button ====//
    int bw = FitWidth( 0, m_ButtonWidth );
    VspButton* flbutton = new VspButton( m_X, m_Y, bw, m_StdHeight, label );
    flbutton->copy_label( label );
    flbutton->labelfont( FL_HELVETICA_BOLD );
    flbutton->labelsize( 12 );
    flbutton->labelcolor( FL_DARK_BLUE );
    m_Group->add( flbutton );

    AddX( bw );
    AddY( m_StdHeight );
    NewLineX();

    pbutton.Init( m_Screen, flbutton );
}

//==== Create & Init Gui RadioButton ====//
void GroupLayout::AddButton( RadioButton& rbutton, const char* label, int val )
{
    assert( m_Group && m_Screen );

    //==== Add Radio Button ====//
    int bw = FitWidth( 0, m_ButtonWidth );
    Fl_Round_Button* flbutton = new Fl_Round_Button( m_X, m_Y, bw, m_StdHeight, label );
    flbutton->labelfont( FL_HELVETICA_BOLD );
    flbutton->labelsize( 12 );
    flbutton->labelcolor( FL_DARK_BLUE );
    flbutton->copy_label( label );
    m_Group->add( flbutton );
    AddX( bw );

    AddY( m_StdHeight );
    NewLineX();

    //Default RadioButton set to false
    rbutton.Init( m_Screen, flbutton, val);
}

//==== Create & Init Box Divider  ====//
Fl_Box* GroupLayout::AddDividerBox( const string& text, int used_w )
{
    assert( m_Group && m_Screen );

    //==== Add Divider Box ====//
    int dw = FitWidth( used_w, m_ButtonWidth );

    Fl_Box* flbox = new Fl_Box( m_X, m_Y, dw, m_DividerHeight );
    flbox->box( FL_BORDER_BOX );
    flbox->color( ( Fl_Color )12 );
    flbox->labelfont( FL_HELVETICA_BOLD );
    flbox->labelcolor( FL_BACKGROUND2_COLOR );
    flbox->copy_label( text.c_str() );
    m_Group->add( flbox );
    AddX( dw );

    AddY( m_DividerHeight );
    NewLineX();

    return flbox;
}

//==== Create & Init Resize Box  ====//
void GroupLayout::AddResizeBox( )
{
    assert( m_Group && m_Screen );

    //==== Add Divider Box ====//
    int dw = FitWidth( 0, m_ButtonWidth );

    Fl_Box* flbox = new Fl_Box( m_X, m_Y, dw, m_StdHeight );
    flbox->box( FL_NO_BOX );
    m_Group->add( flbox );
    m_Group->resizable( flbox );

    AddX( dw );

    AddY( m_StdHeight );
    NewLineX();
}

void GroupLayout::AddLegendEntry( const string& text, Fl_Color c )
{
    assert( m_Group && m_Screen );

    Fl_Button* button = new Fl_Button( m_X, m_Y, m_ButtonWidth, m_StdHeight );
    button->copy_label( text.c_str() );
    button->box( FL_THIN_UP_BOX );
    button->labelfont( FL_HELVETICA_BOLD );
    button->labelsize( 12 );
    button->labelcolor( FL_BLACK );
    m_Group->add( button );
    AddX( m_ButtonWidth );

    int dw = FitWidth( m_ButtonWidth, m_ButtonWidth );

    Fl_Box* flbox = new Fl_Box( m_X, m_Y, dw, m_StdHeight );
    flbox->box( FL_FLAT_BOX );
    flbox->color( c );
    m_Group->add( flbox );
    m_Group->resizable( flbox );

    AddX( dw );

    AddY( m_StdHeight );
    NewLineX();
}

//==== Create Cartesian Canvas ====//
Vsp_Canvas* GroupLayout::AddCanvas( int w, int h, double xmin, double xmax, double ymin, double ymax, const char *label, const char *xlabel, const char *ylabel )
{
    assert( m_Group && m_Screen );

    int hxaxis = 30;
    int wyaxis = 60;
    int margin = 10;

    Vsp_Canvas* canvas = new Vsp_Canvas( m_X + wyaxis, m_Y, w - wyaxis, h - hxaxis, label );
    Vsp_Canvas::current(canvas);
    canvas->box( FL_DOWN_BOX );
    canvas->color( FL_BACKGROUND2_COLOR );
    canvas->border( 0 );
    m_Group->add( canvas );

    Ca_X_Axis* xaxis = new Ca_X_Axis( m_X, m_Y + h - hxaxis, w + margin, hxaxis, xlabel );
    canvas->current_x(xaxis);    // this is needed to ensure that the axis gets associated with the right canvas
    xaxis->labelsize(14);
    xaxis->align( Fl_Align( FL_ALIGN_BOTTOM ) );
    xaxis->minimum( xmin );
    xaxis->maximum( xmax );
    xaxis->label_format( "%g" );
    xaxis->minor_grid_color( fl_gray_ramp( 20 ) );
    xaxis->major_grid_color( fl_gray_ramp( 15 ) );
    xaxis->label_grid_color( fl_gray_ramp( 10 ) );
    xaxis->grid_visible( CA_MINOR_GRID|CA_MAJOR_GRID|CA_LABEL_GRID );
    xaxis->major_step( 10 );
    xaxis->label_step( 10 );
    xaxis->axis_color( FL_BLACK );
    xaxis->axis_align( CA_BOTTOM );
    m_Group->add( xaxis );

    Ca_Y_Axis* yaxis = new Ca_Y_Axis( m_X, m_Y - margin, wyaxis, h + margin, ylabel );
    canvas->current_y(yaxis);    // this is needed to ensure that the axis gets associated with the right canvas
    yaxis->labelsize( 14 );
    yaxis->align( Fl_Align( FL_ALIGN_TOP ) );
    yaxis->minimum( ymin );
    yaxis->maximum( ymax );
    yaxis->label_format( "%.2f" );
    yaxis->minor_grid_color( fl_gray_ramp( 20 ) );
    yaxis->major_grid_color( fl_gray_ramp( 15 ) );
    yaxis->label_grid_color( fl_gray_ramp( 10 ) );
    yaxis->grid_visible( CA_MINOR_GRID|CA_MAJOR_GRID|CA_LABEL_GRID );
    yaxis->major_step( 10 );
    yaxis->label_step( 10 );
    yaxis->axis_color( FL_BLACK );
    yaxis->axis_align( CA_LEFT );
    m_Group->add( yaxis );      // this is needed for correct resize behavior

    Ca_Y_Axis* y2axis = new Ca_Y_Axis( m_X + w - wyaxis, m_Y - margin, wyaxis, h + margin, ylabel );
    y2axis->labelsize( 14 );
    y2axis->align( Fl_Align( FL_ALIGN_TOP ) );
    y2axis->minimum( ymin );
    y2axis->maximum( ymax );
    y2axis->label_format( "%.2f" );
    y2axis->major_step( 10 );
    y2axis->label_step( 10 );
    y2axis->axis_color( FL_BLACK );
    y2axis->axis_align( CA_RIGHT );
    m_Group->add( y2axis );      // this is needed for correct resize behavior

    canvas->m_Ymain = yaxis;
    canvas->m_Ysecondary = y2axis;
    canvas->m_Wwide = w - wyaxis;
    canvas->m_Wnarrow = w - 2 * wyaxis;

    AddX( w );
    AddY( h );
    NewLineX();

    canvas->HideSecondaryY();

    return canvas;
}

//==== Create Cartesian Point Line ====//
void AddPointLine( const vector <double> & xdata, const vector <double> & ydata, int linewidth, Fl_Color color, int pointsize, int pointstyle )
{
    int n = xdata.size();
    
    if ( n != ydata.size() )
    {
        printf("ERROR: xdata.size() != ydata.size() \n\tFile: %s \tLine:%d\n",__FILE__,__LINE__);
        return;
    }

    Ca_LinePoint* LP = 0;
    for( int i=0; i<n; i++ )
    {
        LP = new Ca_LinePoint( LP, xdata[i], ydata[i], linewidth, color, pointstyle|CA_BORDER, 0.0 );
    }
    // Second pass to plot symbols on top of line.
    for( int i=0; i<n; i++ )
    {
        new Ca_Point( xdata[i], ydata[i], color, pointstyle|CA_BORDER, pointsize );
    }
}

//==== Create Cartesian Point ====//
void AddPoint( const double & x, const double & y, Fl_Color color, int pointsize, int pointstyle )
{
    new Ca_Point( x, y, color, pointstyle|CA_BORDER, pointsize );
}

//==== Create Cartesian Point ====//
void AddPoint( const vector < double > & x, const vector < double > & y, Fl_Color color, int pointsize, int pointstyle )
{
    int n = x.size();

    for( int i=0; i<n; i++ )
    {
        new Ca_Point( x[i], y[i], color, pointstyle|CA_BORDER, pointsize );
    }
}

//==== Create & Init Text Input  ====//
void GroupLayout::AddInput( StringInput& text_input, const char* label, int used_w )
{
    assert( m_Group && m_Screen );

    //==== Button ====//
    if ( m_ButtonWidth > 0 )
    {
        AddParmButton( label );
    }

    //==== Add Text Input ====//
    int iw = FitWidth( m_ButtonWidth + used_w, m_InputWidth );
    Fl_Input* input = new Fl_Input( m_X, m_Y, iw, m_StdHeight );
#ifdef NOREGEXP
    input->type( FL_FLOAT_INPUT );
#endif
    input->box( FL_THIN_DOWN_BOX );
    input->textsize( 12 );
    input->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( input );
    AddX( iw );

    AddY( m_StdHeight );
    NewLineX();

    text_input.Init( m_Screen, input );
}

//==== Create & Init Text Output without label ====//
void GroupLayout::AddOutput( StringOutput& string_output)
{
    assert( m_Group && m_Screen );

    //==== Add Text Input ====//
    int iw = FitWidth( m_ButtonWidth, m_InputWidth );
    Fl_Output* output = new Fl_Output( m_X, m_Y, iw, m_StdHeight );
    output->color( ( Fl_Color )23 );
    output->labelfont( FL_HELVETICA_BOLD );
    output->labelsize( 12 );
    output->textfont( FL_HELVETICA_BOLD );
    output->textsize( 12 );
    m_Group->add( output );
    AddX( iw );

    AddY( m_StdHeight );
    NewLineX();

    string_output.Init( m_Screen, output );
}

//==== Create & Init Text Output  ====//
void GroupLayout::AddOutput( StringOutput& string_output, const char* label, int used_w )
{
    assert( m_Group && m_Screen );

    //==== Button ====//
    AddParmButton( label );

    //==== Add Text Input ====//
    int iw = FitWidth( m_ButtonWidth + used_w, m_InputWidth );
    Fl_Output* output = new Fl_Output( m_X, m_Y, iw, m_StdHeight );
    output->color( ( Fl_Color )23 );
    output->labelfont( FL_HELVETICA_BOLD );
    output->labelsize( 12 );
    output->textfont( FL_HELVETICA_BOLD );
    output->textsize( 12 );
    m_Group->add( output );
    AddX( iw );

    AddY( m_StdHeight );
    NewLineX();

    string_output.Init( m_Screen, output );
}

void GroupLayout::AddOutput( Output& output, const char* label, const char* format, int used_w )
{
    assert( m_Group && m_Screen );

    //==== Parm Button ====//
    VspButton* button = AddParmButton( label );

    //==== Add Text Input ====//
    int iw = FitWidth( m_ButtonWidth + used_w, m_InputWidth );
    Fl_Output* floutput = new Fl_Output( m_X, m_Y, iw, m_StdHeight );
    floutput->color( ( Fl_Color )23 );
    floutput->labelfont( FL_HELVETICA_BOLD );
    floutput->labelsize( 12 );
    floutput->textfont( FL_HELVETICA_BOLD );
    floutput->textsize( 12 );

    m_Group->add( floutput );
    AddX( iw );

    AddY( m_StdHeight );
    NewLineX();

    output.Init( m_Screen, floutput, format, button );
}

//==== Create & Init Float Input  ====//
void GroupLayout::AddInput( Input& input, const char* label, const char* format )
{
    assert( m_Group && m_Screen );

    //==== Parm Button ====//
    VspButton* button = AddParmButton( label );

    //==== Add Text Input ====//
    int iw = FitWidth( m_ButtonWidth, m_InputWidth );
    Fl_Input* flinput = new Fl_Input( m_X, m_Y, iw, m_StdHeight );
#ifdef NOREGEXP
    flinput->type( FL_FLOAT_INPUT );
#endif
    flinput->box( FL_THIN_DOWN_BOX );
    flinput->textsize( 12 );
    flinput->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( flinput );
    AddX( iw );

    AddY( m_StdHeight );
    NewLineX();

    input.Init( m_Screen, flinput, format, button );

    if( strcmp( label, "AUTO_UPDATE" ) == 0 || strcmp( label, "" ) == 0 )
    {
        input.SetButtonNameUpdate( true );
    }
}

void GroupLayout::AddInputEvenSpacedVector(Input& start_input, Input& end_input, Input& npts_input,const char * label,const char * format)
{
    assert( m_Group && m_Screen );

    bool tSameLineFlagOrig, tFitWidthFlagOrig;
    tSameLineFlagOrig = m_SameLineFlag;
    tFitWidthFlagOrig = m_FitWidthFlag;
    SetSameLineFlag( true );
    SetFitWidthFlag( false );

    // record the original settings for button and input width
    int tButtonWidthOrig, tInputWidthOrig;
    tButtonWidthOrig = m_ButtonWidth;
    tInputWidthOrig = m_InputWidth;

    // calculate the new input and button widths
    int tInputWidth, tButtonWidth;
    tButtonWidth = 40;
    tInputWidth = (m_W - m_ButtonWidth - 2* tButtonWidth)/3;

    // add the left button using the default button width
    string left_button_label;
    left_button_label = string(label) + " Start";
    SetInputWidth(tInputWidth);
    AddInput( start_input, left_button_label.c_str(), format );

    SetButtonWidth( tButtonWidth );
    AddInput( end_input, "End", format );
    AddInput( npts_input, "Npts", "%3.0f" );    // this is an integer input

    // reset object settings
    SetButtonWidth( tButtonWidthOrig );
    SetInputWidth( tInputWidthOrig );
    SetSameLineFlag( tSameLineFlagOrig );
    SetFitWidthFlag( tFitWidthFlagOrig );

    ForceNewLine();
}

//==== Create & Init Index Selector  ====//
void GroupLayout::AddIndexSelector( IndexSelector& selector, const char* label, int used_w )
{
    assert( m_Group && m_Screen );

    int butw = 5 * m_ButtonWidth / 6;

    VspButton* button = NULL;
    if ( label )
    {
        button = AddParmButton( label );
        // Shrink arrows.
        butw = butw / 2;
        used_w = used_w + m_ButtonWidth;
    }

    Fl_Button* but_ll = new Fl_Button( m_X, m_Y, butw, m_StdHeight, "<<" );
    but_ll->box( FL_THIN_UP_BOX );
    but_ll->labelfont( FL_HELVETICA_BOLD );
    but_ll->labelsize( 20 );
    but_ll->labelcolor( ( Fl_Color )4 );
    but_ll->align( Fl_Align( FL_ALIGN_CLIP ) );
    m_Group->add( but_ll );
    AddX( butw );

    Fl_Button* but_l = new Fl_Button( m_X, m_Y, butw, m_StdHeight, "<" );
    but_l->box( FL_THIN_UP_BOX );
    but_l->labelfont( FL_HELVETICA_BOLD );
    but_l->labelsize( 20 );
    but_l->labelcolor( ( Fl_Color )4 );
    but_l->align( Fl_Align( FL_ALIGN_CLIP ) );
    m_Group->add( but_l );
    AddX( butw );

    int iw = FitWidth( 4 * butw + used_w, m_InputWidth );

    Fl_Int_Input* int_inp = new Fl_Int_Input( m_X + 4, m_Y, iw - 8, m_StdHeight );
    int_inp->type( FL_INT_INPUT );
    int_inp->box( FL_THIN_DOWN_BOX );
    int_inp->labelfont( FL_HELVETICA_BOLD );
    int_inp->textsize( 14 );
    int_inp->align( Fl_Align( FL_ALIGN_CENTER ) );
    int_inp->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( int_inp );
    AddX( iw );

    Fl_Button* but_r = new Fl_Button( m_X, m_Y, butw, m_StdHeight, ">" );
    but_r->box( FL_THIN_UP_BOX );
    but_r->labelfont( FL_HELVETICA_BOLD );
    but_r->labelsize( 20 );
    but_r->labelcolor( ( Fl_Color )4 );
    but_r->align( Fl_Align( FL_ALIGN_CLIP ) );
    m_Group->add( but_r );
    AddX( butw );

    Fl_Button* but_rr = new Fl_Button( m_X, m_Y, butw, m_StdHeight, ">>" );
    but_rr->box( FL_THIN_UP_BOX );
    but_rr->labelfont( FL_HELVETICA_BOLD );
    but_rr->labelsize( 20 );
    but_rr->labelcolor( ( Fl_Color )4 );
    but_rr->align( Fl_Align( FL_ALIGN_CLIP ) );
    m_Group->add( but_rr );
    AddX( butw );

    selector.Init( m_Screen, but_ll, but_l, int_inp, but_r, but_rr, button );

    AddY( m_StdHeight );
    NewLineX();
}

//==== Add Color Picker ====//
void GroupLayout::AddColorPicker( ColorPicker& picker )
{
    assert( m_Group && m_Screen );

    //==== Button ====//
    Fl_Button* colorButton = new Fl_Button( m_X, m_Y, m_ButtonWidth, m_StdHeight * 2, "Color:" );
    colorButton->box( FL_THIN_UP_BOX );
    colorButton->labelfont( FL_HELVETICA_BOLD );
    colorButton->labelsize( 12 );
    colorButton->labelcolor( FL_BLACK );
    m_Group->add( colorButton );
    AddX( m_ButtonWidth );

    //==== Current Color Button ====//
    Fl_Button* primColorButton = new Fl_Button( m_X, m_Y, m_ButtonWidth / 2, m_StdHeight * 2 );
    primColorButton->box( FL_THIN_DOWN_BOX );
    primColorButton->color( ( Fl_Color )2 );
    m_Group->add( primColorButton );
    AddX( m_ButtonWidth / 2 + 2 );

    //==== Sliders ====//
    int ch = 2 * m_StdHeight / 3;
    int cw = m_StdHeight;

    int sw = FitWidth( m_ButtonWidth + m_ButtonWidth / 2 + 2 + 4 * cw, m_SliderWidth );
    Fl_Slider* sliders[3];
    for ( int i = 0 ; i < 3 ; i++ )
    {
        sliders[i] = new Fl_Value_Slider( m_X, m_Y + i * ch, sw, ch );
        sliders[i]->type( FL_HOR_NICE_SLIDER );
        sliders[i]->color( FL_BACKGROUND2_COLOR );
        sliders[i]->selection_color( ( Fl_Color )1 );
        sliders[i]->maximum( 255 );
        sliders[i]->step( 1 );
        m_Group->add( sliders[i] );
    }
    sliders[0]->selection_color( ( Fl_Color )1 );
    sliders[1]->selection_color( ( Fl_Color )2 );
    sliders[2]->selection_color( ( Fl_Color )4 );
    AddX( sw + 2 );

    //==== Color Buttons ====//
    vector< Fl_Button* > cvec;
    int color_index = 0;
    for ( int i = 0 ; i < 4 ; i++ )
    {
        for ( int j = 0 ; j < 3 ; j++ )
        {
            Fl_Button* b = new Fl_Button( m_X + i * cw, m_Y + j * ch, cw, ch );
            vec3d rgb = picker.GetIndexRGB( color_index );
            Fl_Color c = fl_rgb_color( ( int )rgb[0], ( int )rgb[1], ( int )rgb[2] );
            b->color( c );
            m_Group->add( b );
            cvec.push_back( b );
            color_index++;
        }
    }
    AddX( 3 * cw );

    picker.Init( m_Screen, colorButton, primColorButton, cvec, sliders );

    AddY( m_StdHeight * 2 );
    NewLineX();
}

//==== Create & Init Choice  ====//
void GroupLayout::AddChoice( Choice & choice, const char* label, int used_w )
{
    assert( m_Group && m_Screen );

    VspButton* button = NULL;

    if ( m_ChoiceButtonWidth > 0 )
    {
    //==== Choice Button ====//
        button = new VspButton( m_X, m_Y, m_ChoiceButtonWidth, m_StdHeight, label );
        button->box( FL_THIN_UP_BOX );
        button->labelfont( FL_HELVETICA_BOLD );
        button->labelsize( 12 );
        button->labelcolor( FL_BLACK );
        button->copy_label( label );
        m_Group->add( button );
        AddX( m_ChoiceButtonWidth );
    }

    //==== Choice Picker ====//
    int choice_w = FitWidth( m_ChoiceButtonWidth + used_w, m_SliderWidth );
    Fl_Choice* fl_choice = new Fl_Choice( m_X, m_Y, choice_w, m_StdHeight );
    fl_choice->down_box( FL_BORDER_BOX );
    fl_choice->textfont( FL_HELVETICA_BOLD );
    fl_choice->textsize( 12 );
    fl_choice->textcolor( FL_DARK_BLUE );
    m_Group->add( fl_choice );
    AddX( choice_w );

    choice.Init( m_Screen, fl_choice, button );

    //==== Add Choice Text ===//
    choice.UpdateItems();

    if( strcmp( label, "AUTO_UPDATE" ) == 0 || strcmp( label, "" ) == 0 )
    {
        choice.SetButtonNameUpdate( true );
    }

    AddY( m_StdHeight );
    NewLineX();

}

//==== Add Sub Group Layout At Current Position  ====//
void GroupLayout::AddCounter( Counter & count, const char* label, int used_w )
{
    assert( m_Group && m_Screen );

    //==== Counter Button ====//
    VspButton* button = NULL;
    if ( strcmp( label, "" ) != 0 )
    {
        button = AddParmButton( label );
        button->align( Fl_Align( FL_ALIGN_CLIP ) );
    }

    //==== Counter ====//
    int counter_w = FitWidth( m_ButtonWidth + used_w, m_SliderWidth );
    Fl_Counter* fl_counter = new Fl_Counter( m_X, m_Y, counter_w, m_StdHeight );
    fl_counter->type( FL_SIMPLE_COUNTER );
    fl_counter->minimum( 0 );
    fl_counter->maximum( 10000 );
    fl_counter->step( 1 );
    m_Group->add( fl_counter );

    AddX( counter_w );

    count.Init( m_Screen, fl_counter, button );

    AddY( m_StdHeight );
    NewLineX();

}

//==== Add Sub Group Layout At Current Position  ====//
void GroupLayout::AddSubGroupLayout( GroupLayout& layout, int w, int h )
{
    Fl_Group* grp = ( Fl_Group* ) new Vsp_Group( m_X, m_Y, w, h );
    m_Group->add( grp );
    layout.SetGroupAndScreen( grp,  m_Screen );
}

//==== Add Fl_CheckBrowser ====//
Fl_Check_Browser* GroupLayout::AddCheckBrowser( int h )
{
    int w = FitWidth( 0, m_DividerHeight );

    Fl_Check_Browser* check_browser = new Fl_Check_Browser( m_X, m_Y, w, h );
    check_browser->labelfont( FL_HELVETICA_BOLD );
    check_browser->textsize( 12 );
    check_browser->when( FL_WHEN_RELEASE_ALWAYS );
    m_Group->add( check_browser );
    AddX( w );

    AddY( m_StdHeight );
    NewLineX();

    return check_browser;
}


//==== Add Label ====//
void GroupLayout::AddLabel( const char* label, int width )
{

    Fl_Button* button = new Fl_Button( m_X, m_Y, width, m_StdHeight );
    button->copy_label( label );
    button->box( FL_THIN_UP_BOX );
    button->labelfont( FL_HELVETICA_BOLD );
    button->labelsize( 12 );
    button->labelcolor( FL_BLACK );
    m_Group->add( button );
    AddX( width );
}

//==== Add Parameter Picker ====//
void GroupLayout::AddParmPicker( ParmPicker & parm_picker )
{

    assert( m_Group && m_Screen );

    //==== Container Button ====//
    Fl_Button* button = new Fl_Button( m_X, m_Y, m_ChoiceButtonWidth, m_StdHeight, "Container" );
    button->box( FL_THIN_UP_BOX );
    button->labelfont( FL_HELVETICA_BOLD );
    button->labelsize( 12 );
    button->labelcolor( FL_BLACK );
    m_Group->add( button );
    AddX( m_ChoiceButtonWidth );

    //==== Container Picker ====//
    int choice_w = FitWidth( m_ChoiceButtonWidth, m_SliderWidth );
    Fl_Choice* container_choice = new Fl_Choice( m_X, m_Y, choice_w, m_StdHeight );
    container_choice->down_box( FL_BORDER_BOX );
    container_choice->textfont( FL_HELVETICA_BOLD );
    container_choice->textsize( 12 );
    container_choice->textcolor( FL_DARK_BLUE );
    m_Group->add( container_choice );
    AddX( choice_w );

    AddY( m_StdHeight );
    NewLineX();

    //==== Group Button ====//
    button = new Fl_Button( m_X, m_Y, m_ChoiceButtonWidth, m_StdHeight, "Group" );
    button->box( FL_THIN_UP_BOX );
    button->labelfont( FL_HELVETICA_BOLD );
    button->labelsize( 12 );
    button->labelcolor( FL_BLACK );
    m_Group->add( button );
    AddX( m_ChoiceButtonWidth );

    //==== Group Picker ====//
    choice_w = FitWidth( m_ChoiceButtonWidth, m_SliderWidth );
    Fl_Choice* group_choice = new Fl_Choice( m_X, m_Y, choice_w, m_StdHeight );
    group_choice->down_box( FL_BORDER_BOX );
    group_choice->textfont( FL_HELVETICA_BOLD );
    group_choice->textsize( 12 );
    group_choice->textcolor( FL_DARK_BLUE );
    m_Group->add( group_choice );
    AddX( choice_w );

    AddY( m_StdHeight );
    NewLineX();

    //==== Parm Button ====//
    button = new Fl_Button( m_X, m_Y, m_ChoiceButtonWidth, m_StdHeight, "Parm" );
    button->box( FL_THIN_UP_BOX );
    button->labelfont( FL_HELVETICA_BOLD );
    button->labelsize( 12 );
    button->labelcolor( FL_BLACK );
    m_Group->add( button );
    AddX( m_ChoiceButtonWidth );

    //==== Parm Picker ====//
    choice_w = FitWidth( m_ChoiceButtonWidth, m_SliderWidth );
    Fl_Choice* parm_choice = new Fl_Choice( m_X, m_Y, choice_w, m_StdHeight );
    parm_choice->down_box( FL_BORDER_BOX );
    parm_choice->textfont( FL_HELVETICA_BOLD );
    parm_choice->textsize( 12 );
    parm_choice->textcolor( FL_DARK_BLUE );
    m_Group->add( parm_choice );
    AddX( choice_w );

    AddY( m_StdHeight );
    NewLineX();

    parm_picker.Init( m_Screen, container_choice, group_choice, parm_choice );

}

//==== Add Parameter Tree Picker ====//
void GroupLayout::AddParmTreePicker( ParmTreePicker & parm_tree_picker, int w, int h )
{
    assert( m_Group && m_Screen );

    Fl_Tree* parm_tree = new Fl_Tree( m_X, m_Y, w, h );
    m_Group->add( parm_tree );
    AddY( h );
    NewLineX();

    parm_tree_picker.Init( m_Screen, parm_tree );
}

//==== Add Parameter Driver Group ====//
void GroupLayout::AddDriverGroupBank( DriverGroupBank & dgBank, const vector < string > &labels, double range, const char* format )
{
    assert( m_Group && m_Screen );

    bool oldSameLine = m_SameLineFlag;
    bool oldFitWidth = m_FitWidthFlag;

    SetSameLineFlag( true );
    SetFitWidthFlag( true );

    vector< vector< Fl_Button* > > buttons;
    buttons.resize( dgBank.GetDriverGroup()->GetNvar() );

    vector< SliderAdjRangeInput* > sliders;
    sliders.resize( dgBank.GetDriverGroup()->GetNvar() );

    for( int i = 0; i < dgBank.GetDriverGroup()->GetNvar(); i++ )
    {
        buttons[i].resize( dgBank.GetDriverGroup()->GetNchoice() );
        for( int j = 0; j < dgBank.GetDriverGroup()->GetNchoice(); j++ )
        {
            int roundW = 17;
            buttons[i][j] = new GroupBankRoundButton( m_X, m_Y, roundW, m_StdHeight );
            m_Group->add( buttons[i][j] );
            AddX( roundW );
        }
        AddX( 1 );

        sliders[i] = new SliderAdjRangeInput();
        m_Slider_Cleanup.push_back( sliders[i] );
        AddSlider( *sliders[i], labels[i].c_str(), range, format );

        ForceNewLine();
    }

    SetSameLineFlag( oldSameLine );
    SetFitWidthFlag( oldFitWidth );

    dgBank.Init( m_Screen, buttons, sliders );
}

GroupBankRoundButton::GroupBankRoundButton( int x, int y, int w, int h, const char *label ) : Fl_Round_Button( x, y, w, h, label )
{
}

void GroupBankRoundButton::draw()
{
    Fl_Round_Button::draw();
    if ( !active() )
    {
        int o = 5;
        fl_color( fl_inactive( selection_color() ) );
        fl_line( x() + o, y() + o, x() + w() - o, y() + h() - o );
        fl_line( x() + o, y() + h() - o, x() + w() - o, y() + o );
    }
}

//==== Add Fuselage Skin Control Group ====//
void GroupLayout::AddSkinControl( SkinControl & skin_control, const char* label, double range, const char* format )
{
    assert( m_Group && m_Screen );

    int sw = FitWidth( 3 * m_StdHeight + m_ButtonWidth + 2 * m_InputWidth + 4 * m_RangeButtonWidth, 2 * m_SliderWidth )/2;

    //==== Left Min Range Button ====//
    Fl_Repeat_Button* minbuttonL = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, "<" );
    minbuttonL->box( FL_THIN_UP_BOX );
    minbuttonL->labelcolor( ( Fl_Color )4 );
    m_Group->add( minbuttonL );
    AddX( m_RangeButtonWidth );

    //==== Left Slider ====//
    Fl_Slider* sliderL = new Fl_Slider( m_X, m_Y, sw, m_StdHeight );
    sliderL->type( FL_HOR_NICE_SLIDER );
    sliderL->box( FL_THIN_DOWN_BOX );
    sliderL->color( FL_BACKGROUND2_COLOR );
    sliderL->selection_color( FL_SELECTION_COLOR );
    m_Group->add( sliderL );
    AddX( sw );

    //==== Left Max Range Button ====//
    Fl_Repeat_Button* maxbuttonL = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, ">" );
    maxbuttonL->box( FL_THIN_UP_BOX );
    maxbuttonL->labelcolor( ( Fl_Color )4 );
    m_Group->add( maxbuttonL );
    AddX( m_RangeButtonWidth );

    //==== Left Input ====//
    Fl_Input* inputL = new Fl_Input( m_X, m_Y,  m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    inputL->type( FL_FLOAT_INPUT );
#endif
    inputL->box( FL_THIN_DOWN_BOX );
    inputL->textsize( 12 );
    inputL->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( inputL );
    AddX( m_InputWidth );

    //==== Left Set Check Button ====//
    Fl_Check_Button* setButtonL = new Fl_Check_Button( m_X, m_Y, m_StdHeight, m_StdHeight );
    m_Group->add( setButtonL );
    AddX( m_StdHeight );

    //==== Parm Button ====//
    int originalWidth = m_ButtonWidth;
    int rightWidth = 12;
    int leftWidth = originalWidth - rightWidth;
    SetButtonWidth(leftWidth);
    VspButton* parm_button_L = AddParmButton( label );
    SetButtonWidth(rightWidth);
    VspButton* parm_button_R = AddParmButton( "" );
    SetButtonWidth(originalWidth);

    //==== Set Equality Check Button ====//
    Fl_Check_Button* setButtonEqual = new Fl_Check_Button( m_X, m_Y, m_StdHeight, m_StdHeight );
    m_Group->add( setButtonEqual );
    AddX( m_StdHeight );

    //==== Right Set Check Button ====//
    Fl_Check_Button* setButtonR = new Fl_Check_Button( m_X, m_Y, m_StdHeight, m_StdHeight );
    m_Group->add( setButtonR );
    AddX( m_StdHeight );

    //==== Right Min Range Button ====//
    Fl_Repeat_Button* minbuttonR = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, "<" );
    minbuttonR->box( FL_THIN_UP_BOX );
    minbuttonR->labelcolor( ( Fl_Color )4 );
    m_Group->add( minbuttonR );
    AddX( m_RangeButtonWidth );

    //==== Right Slider ====//
    Fl_Slider* sliderR = new Fl_Slider( m_X, m_Y, sw, m_StdHeight );
    sliderR->type( FL_HOR_NICE_SLIDER );
    sliderR->box( FL_THIN_DOWN_BOX );
    sliderR->color( FL_BACKGROUND2_COLOR );
    sliderR->selection_color( FL_SELECTION_COLOR );
    m_Group->add( sliderR );
    AddX( sw );

    //==== Right Max Range Button ====//
    Fl_Repeat_Button* maxbuttonR = new Fl_Repeat_Button( m_X, m_Y, m_RangeButtonWidth, m_StdHeight, ">" );
    maxbuttonR->box( FL_THIN_UP_BOX );
    maxbuttonR->labelcolor( ( Fl_Color )4 );
    m_Group->add( maxbuttonR );
    AddX( m_RangeButtonWidth );

    //==== Right Input ====//
    Fl_Input* inputR = new Fl_Input( m_X, m_Y,  m_InputWidth, m_StdHeight );
#ifdef NOREGEXP
    inputR->type( FL_FLOAT_INPUT );
#endif
    inputR->box( FL_THIN_DOWN_BOX );
    inputR->textsize( 12 );
    inputR->when( FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE );
    m_Group->add( inputR );
    AddX( m_InputWidth );

    AddY( m_StdHeight );
    NewLineX();

    skin_control.Init( m_Screen,
        setButtonL,
        setButtonEqual,
        setButtonR,
        sliderL,
        minbuttonL,
        maxbuttonL,
        sliderR,
        minbuttonR,
        maxbuttonR,
        inputL,
        inputR,
        parm_button_L,
        parm_button_R,
        range, format);
}

void GroupLayout::AddSkinHeader( SkinHeader & skin_header, bool addcontchoice )
{
    assert( m_Group && m_Screen );
    int oldBW = GetButtonWidth();


    bool saveFitWidth = m_FitWidthFlag;
    bool saveSameLine = m_SameLineFlag;


    vector< VspButton* > buttons;

    // Size of Set label buttons
    int setW = 25;
    // Size of Equal label button
    int eqW = 20;

    // Width of choice
    int cw = m_ChoiceButtonWidth + m_SliderWidth;

    m_FitWidthFlag = true;

    // Gap width
    int gw = FitWidth( 2 * setW + eqW + oldBW + 2 * cw, 2 * m_ButtonWidth )/2;

    m_FitWidthFlag = false;
    m_SameLineFlag = true;

    Choice* cont_choice = NULL;

    if( addcontchoice )
    {
        cont_choice = new Choice();
        cont_choice->AddItem( "C0" );
        cont_choice->AddItem( "C1" );
        cont_choice->AddItem( "C2" );
        AddChoice( *cont_choice, "Enforce" );
    }
    else
    {
        gw += cw;
    }

    AddX( gw );

    //==== Left Set Button ====//
    SetButtonWidth( setW );
    buttons.push_back( AddParmButton( "Set" ) );

    AddX( oldBW );

    //==== Equal Button ====//
    SetButtonWidth( eqW );
    buttons.push_back( AddParmButton( "=" ) );

    //==== Right Set Button ====//
    SetButtonWidth( setW );
    buttons.push_back( AddParmButton( "Set" ) );

    NewLineX();

    m_FitWidthFlag = saveFitWidth;
    m_SameLineFlag = saveSameLine;

    skin_header.Init( m_Screen, cont_choice, buttons );

    ForceNewLine();

    SetButtonWidth( oldBW );
}

//==== Add Geom Picker ====//
void GroupLayout::AddGeomPicker( GeomPicker & geom_picker, int used_w, string text )
{

    assert( m_Group && m_Screen );

    //==== Geom Button ====//
    if ( m_ChoiceButtonWidth > 0 )
    {
        if ( text.length() == 0 )
        {
            text = string( "Geom" );
        }
        Fl_Button* button = new Fl_Button( m_X, m_Y, m_ChoiceButtonWidth, m_StdHeight );
        button->copy_label( text.c_str() );
        button->box( FL_THIN_UP_BOX );
        button->labelfont( FL_HELVETICA_BOLD );
        button->labelsize( 12 );
        button->labelcolor( FL_BLACK );
        m_Group->add( button );
        AddX( m_ChoiceButtonWidth );
        used_w += m_ChoiceButtonWidth;
    }

    //==== Geom Picker ====//
    int choice_w = FitWidth( used_w, m_SliderWidth );
    Fl_Choice* geom_choice = new Fl_Choice( m_X, m_Y, choice_w, m_StdHeight );
    geom_choice->down_box( FL_BORDER_BOX );
    geom_choice->textfont( FL_HELVETICA_BOLD );
    geom_choice->textsize( 12 );
    geom_choice->textcolor( FL_DARK_BLUE );
    m_Group->add( geom_choice );
    AddX( choice_w );

    AddY( m_StdHeight );
    NewLineX();

    geom_picker.Init( m_Screen, geom_choice );
}

//==== Add Curve Editor ====//
void GroupLayout::AddPCurveEditor( PCurveEditor& curve_editor )
{
    assert( m_Group && m_Screen );

    SetSameLineFlag( true );

    AddOutput( curve_editor.m_CurveType, "Type", GetW() / 2 );

    Fl_Button* convbutton = new Fl_Button( m_X, m_Y, m_ButtonWidth, m_StdHeight, "Convert" );
    convbutton->labelfont( FL_HELVETICA_BOLD );
    convbutton->labelsize( 12 );
    convbutton->labelcolor( FL_DARK_BLUE );
    convbutton->copy_label( "Convert to:" );
    m_Group->add( convbutton );
    AddX( m_ButtonWidth );


    curve_editor.m_ConvertChoice.AddItem( "Linear" );
    curve_editor.m_ConvertChoice.AddItem( "Spline (PCHIP)" );
    curve_editor.m_ConvertChoice.AddItem( "Cubic Bezier" );
    curve_editor.m_ConvertChoice.AddItem( "Approximate Cubic Bezier" );

    m_ChoiceButtonWidth = 0;
    AddChoice( curve_editor.m_ConvertChoice, "Convert to:", GetW() / 2 + m_ButtonWidth );

    AddY( m_StdHeight );
    NewLineX();

    ForceNewLine();
    SetSameLineFlag( false );

    AddYGap();

    int canvas_w = FitWidth( 0, m_CanvasWidth ) - 5;
    Vsp_Canvas* canvas = NULL;
    string split_label;

    AddY( 25 );

    canvas = AddCanvas( canvas_w, m_CanvasHeight, 0, 1, 0, 1, "", "X", "Y" );

    split_label = "r/R Split";

    AddY( 25 );

    SetFitWidthFlag( false );
    SetSameLineFlag( true );

    //==== Add Split Button ====//
    int bw = FitWidth( 0, m_ButtonWidth );
    Fl_Button* spbutton = new Fl_Button( m_X, m_Y, bw, m_StdHeight, "Split" );
    spbutton->labelfont( FL_HELVETICA_BOLD );
    spbutton->labelsize( 12 );
    spbutton->labelcolor( FL_DARK_BLUE );
    spbutton->copy_label( "Split" );
    m_Group->add( spbutton );
    AddX( bw );

    SetFitWidthFlag( true );
    AddSlider( curve_editor.m_SplitPtSlider, split_label.c_str(), 1, "%3.2f", m_ButtonWidth );

    Fl_Light_Button* splitpickbutton = new Fl_Light_Button( m_X, m_Y, m_ButtonWidth, m_StdHeight, "Split Pick" );
    splitpickbutton->labelfont( FL_HELVETICA_BOLD );
    splitpickbutton->labelsize( 12 );
    splitpickbutton->align( Fl_Align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP ) );
    splitpickbutton->copy_label( "Split Pick" );
    splitpickbutton->labelcolor( FL_DARK_BLUE );
    m_Group->add( splitpickbutton );


    ForceNewLine();

    SetFitWidthFlag( false );

    //==== Add Delete Button and Toggle ====//
    bw = FitWidth( 0, m_ButtonWidth );
    Fl_Button* deletebutton = new Fl_Button( m_X, m_Y, m_ButtonWidth, m_StdHeight, "Del" );
    deletebutton->labelfont( FL_HELVETICA_BOLD );
    deletebutton->labelsize( 12 );
    deletebutton->labelcolor( FL_DARK_BLUE );
    deletebutton->copy_label( "Del" );
    m_Group->add( deletebutton );
    AddX( bw );

    m_ButtonWidth -= 15;

    SetFitWidthFlag( true );

    AddIndexSelector( curve_editor.m_PntSelector, NULL , 2 * ( m_ButtonWidth + 15 ) );

    m_ButtonWidth += 15;

    Fl_Light_Button* deletetoggle = new Fl_Light_Button( m_X, m_Y, bw, m_StdHeight, "Del Pick" );
    deletetoggle->labelfont( FL_HELVETICA_BOLD );
    deletetoggle->labelsize( 12 );
    deletetoggle->align( Fl_Align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP ) );
    deletetoggle->copy_label( "Del Pick" );
    deletetoggle->labelcolor( FL_DARK_BLUE );
    m_Group->add( deletetoggle );

    ForceNewLine();

    SetFitWidthFlag( true );
    SetSameLineFlag( false );

    AddYGap();
    AddDividerBox( "Control Points" );

    Fl_Scroll* ptscroll = AddFlScroll( GetRemainY() );
    ptscroll->type( Fl_Scroll::VERTICAL_ALWAYS );
    ptscroll->box( FL_BORDER_BOX );
    GroupLayout* ptlayout = new GroupLayout();
    ptlayout->SetGroupAndScreen( ptscroll, this->m_Screen );

    curve_editor.Init( m_Screen, canvas, ptscroll, spbutton, convbutton, deletebutton, deletetoggle, splitpickbutton, ptlayout );
}

//==== Add Fl Browser ====//
Fl_Browser* GroupLayout::AddFlBrowser( int height )
{
    assert( m_Group && m_Screen );

    Fl_Browser* browser = new Fl_Browser( m_X, m_Y, m_W, height);
    browser->type( FL_HOLD_BROWSER );
    browser->textsize(12);
    m_Group->add( browser );

    AddY( height );
    NewLineX();

    return browser;
}

//==== Add Column Resize Browser ====//
ColResizeBrowser* GroupLayout::AddColResizeBrowser( int* width_array_ptr, size_t num_col, int height )
{
    assert( m_Group && m_Screen && width_array_ptr );

    if ( width_array_ptr[num_col] != 0 )
    {
        printf( "Error:  Column width array not zero terminated or column length mismatch: ");

        BasicScreen *bs = dynamic_cast< BasicScreen* >( m_Screen );
        if ( bs )
        {
            printf( " %s", bs->GetTitle().c_str() );
        }

        Fl_Group *g = m_Group;
        while( g )
        {
            if( g->label() )
            {
                printf( " %s", g->label() );
            }

            g = g->parent();
        }
        printf( "\n" );
    }

    ColResizeBrowser* browser = new ColResizeBrowser( m_X, m_Y, m_W, height );
    browser->type( FL_MULTI_BROWSER );
    browser->textsize( 12 );
    browser->labelfont( 13 );
    browser->labelsize( 12 );
    browser->column_widths( width_array_ptr );
    browser->num_col( num_col );
    m_Group->add( browser );

    AddY( height );
    NewLineX();

    return browser;
}

//==== Add Fl Text Editor ====//
Fl_Text_Editor* GroupLayout::AddFlTextEditor( int height )
{
    assert( m_Group && m_Screen );

    Fl_Text_Editor* text_editor = new Fl_Text_Editor (m_X, m_Y, m_W, height, "");

    m_Group->add( text_editor );

    AddY( height );
    NewLineX();

    return text_editor;
}

//==== Add Fl Text Display ====//
Fl_Text_Display* GroupLayout::AddFlTextDisplay( int height )
{
    assert( m_Group && m_Screen );

    Fl_Text_Display* text_display = new Fl_Text_Display (m_X, m_Y, m_W, height, "");

    m_Group->add( text_display );

    AddY( height );
    NewLineX();

    return text_display;
}

//==== Add Fl Scroll panel ====//
Fl_Scroll* GroupLayout::AddFlScroll( int height )
{
    assert( m_Group && m_Screen );

    Fl_Scroll* scroll = new Fl_Scroll (m_X, m_Y, m_W, height, "");

    m_Group->add( scroll );

    AddY( height );
    NewLineX();

    return scroll;
}

Fl_Sys_Menu_Bar* GroupLayout::AddFlMenuBar( int height )
{
    assert( m_Group && m_Screen );

    Fl_Sys_Menu_Bar *menubar = new Fl_Sys_Menu_Bar( m_X, m_Y, m_W, height );

    m_Group->add( menubar );

#ifndef __APPLE__
    menubar->box( FL_EMBOSSED_BOX );
    AddY( height );
    NewLineX();
#endif

    return menubar;
}

//==== Add Fl Color Chooser ====//
Fl_Color_Chooser* GroupLayout::AddFlColorChooser(int height)
{
    assert( m_Group && m_Screen );

    Fl_Color_Chooser* color_chooser = new Fl_Color_Chooser (m_X, m_Y, m_W, height, "");

    m_Group->add( color_chooser );

    AddY( height );
    NewLineX();

    return color_chooser;
}

// nhue prime is good
Fl_Color ColorWheel( int i, int nunique, int nhue )
{
    // Golden ratio.
    // double phi = ( 1.0 + sqrt( 5.0 ) ) / 2.0;

    double h, s, v, r, g, b;

    int npass = ( nunique / nhue ) + 1; // floor implied by integer division.

    int hbase = i % nhue;

    // h is [0,6)
    h = ( hbase +  (i - hbase) * 1.0 / (nhue * npass) ) * 6.0 / nhue;

    // Uniformly distributed pseudo-random.
    // h = ( i * phi - floor( i * phi ) ) * 6.0;
    // s and v are [0,1]
    s = 0.7;
    v = 0.95;

    Fl_Color_Chooser::hsv2rgb( h, s, v, r, g, b );
    Fl_Color c = fl_rgb_color( r * 255, g * 255, b * 255 );
    return c;
}

// Rotate through Cartesian symbol styles.
int StyleWheel( int i )
{
    // There are 5 true styles, but the 0th style is no-symbol at all.
    int nstyle = 5;
    return i % nstyle + 1;
}
