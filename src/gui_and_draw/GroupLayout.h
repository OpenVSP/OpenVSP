//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(GROUPLAYOUT__INCLUDED_)
#define GROUPLAYOUT__INCLUDED_
#include "Cartesian.H"

#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Round_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Terminal.H>

#include "GuiDevice.h"
#include "SpreadSheetWidget.h"
#include "TreeColumnWidget.h"
#include "TreeIconWidget.h"

#include <vector>
#include <string>

using std::string;

class GroupBankRoundButton : public Fl_Round_Button
{
public:
    GroupBankRoundButton( int x, int y, int w, int h, const char *label = 0 );
    void draw();
};

//  GroupLayout creates and arranges Fltk gui widgets in an input Fl_Group.
//  The widgets are sized and placed in the group in the order they are added.
//  The created Fltk widgets are used to initialize the GuiDevices.
//  GroupLayout maintains a current location and default sizes used to build the GuiDevices.

//  The layout starts in the upper left hand corner.
//  --> X
//  |
//  Y

//====GroupLayout - Handles Layout and Cleanup of Programmatically Created GUI  ====//
class GroupLayout
{
public:

    GroupLayout();
    GroupLayout( VspScreen* screen );
    GroupLayout( VspScreen* screen, Fl_Group* m_Group );
    virtual ~GroupLayout();

    //==== Provide Group and Screen Pointers ====//
    void SetGroup( Fl_Group* group );
    Fl_Group* GetGroup()                            { return m_Group; }
    void SetScreen( VspScreen* screen )
    {
        m_Screen = screen;
    }
    void SetGroupAndScreen( Fl_Group* group, VspScreen* screen );

    //==== Hide/Show All Gui Elements ====//
    void Hide();
    void Show();

    //===== Get/Set Current GUI Placement Coords ====//
    int GetX()              { return m_X; }
    int GetY()              { return m_Y; }
    void SetX( int x )      { m_X = x; }
    void SetY( int y )      { m_Y = y; }

    //==== Upper Left Coords Of Group ====//
    int GetStartX()         { return m_StartX; }
    int GetStartY()         { return m_StartY; }

    //==== Width Height of Group ===//
    int GetW()              { return m_W; }
    int GetH()              { return m_H; }

    //==== Remaining Space in Group ====//
    int GetRemainX();
    int GetRemainY();

    //==== Add X/Y to Current Location ====//
    void AddX( int offset );
    void AddY( int offset );

    //==== Add Fixed Distance (GapHeight) To Y Location ====//
    void AddYGap()          { m_Y += m_GapHeight; }

    //==== Move to Beginning of Next Line (ignore SameLineFlag) ====//
    void ForceNewLine( int dy = -1 );

    //==== Flag To Force GuiDevices to Fill Complete Line Width ====//
    void SetFitWidthFlag( bool f )      { m_FitWidthFlag = f; }

    //==== Dont Increment to Next Line After Adding GuiDevice ====//
    void SetSameLineFlag( bool f )      {  m_SameLineFlag = f; }

    //==== Reset To Predefinded Widget Width/Heights ====//
    void InitWidthHeightVals();

    //==== Set/Get Widget Width/Heights ====//
    void SetStdHeight( int h )          { m_StdHeight = h; }
    int  GetStdHeight()                 { return m_StdHeight; }
    void SetGapHeight( int h )          { m_GapHeight = h; }
    int GetGapHeight()                  { return m_GapHeight; }
    void SetDividerHeight( int h )      { m_DividerHeight = h; }
    int GetDividerHeight()              { return m_DividerHeight; }
    void SetButtonWidth( int w )        { m_ButtonWidth = w; }
    int GetButtonWidth()                { return m_ButtonWidth; }
    void SetChoiceButtonWidth( int w )  { m_ChoiceButtonWidth = w; }
    int GetChoiceButtonWidth()          { return m_ChoiceButtonWidth; }
    void SetRangeButtonWidth( int w )   { m_RangeButtonWidth = w; }
    int GetRangeButtonWidth()           { return m_RangeButtonWidth; }
    void SetInputWidth( int w )         { m_InputWidth = w; }
    int GetInputWidth()                 { return m_InputWidth; }
    void SetSliderWidth( int w )        { m_SliderWidth = w; }
    int GetSliderWidth()                { return m_SliderWidth; }
    int GetCanvasWidth()                { return m_CanvasWidth; }
    void SetCanvasWdith( int w )        { m_CanvasWidth = w; }
    int GetCanvasHeight()               { return m_CanvasHeight; }
    void SetCanvasHeight( int h )       { m_CanvasHeight = h; }

    //==== Add FLTK Widgets and Initialize GUI Devices ====//
    Fl_Box* AddDividerBox( const string& text, int used_w = 0 );
    void AddResizeBox( );

    Vsp_Canvas* AddCanvas( int w, int h, double xmin, double xmax, double ymin, double ymax, const char *label, const char *xlabel, const char *ylabel );
    void AddLegendEntry( const string& text, Fl_Color c );

    void AddSlider(  SliderAdjRangeInput& slid_adj_input, const char* label,
                     double range, const char* format, int used_w = 0, bool log_slider = false );
    void AddSlider(  SliderAdjRange2Input& slid_adj_input, const char* label,
                     double range, const char* format );
    void AddSlider(  SliderAdjRange3Input& slid_adj_input, const char* label,
                     double range, const char* format );
    void AddSlider(  FractParmSlider& slid_adj_input, const char* label,
                     double range, const char* format );
    void AddSlider( SliderInput & slider_input, const char* label, double range, const char* format, int used_w = 0, bool log_slider = false );
    void AddLabel( const char* label, int width, int bgcolor = FL_BACKGROUND_COLOR );
    void AddButton(  CheckButton& check_button, const char* label, int used_w = 0 );
    void AddButton(  ToggleButton& toggle_button, const char* label, int used_w = 0 );
    void AddButton(  CheckButtonBit& check_bit_button, const char* label, int val );
    void AddButton(  TriggerButton& trigger_button, const char* label, int used_w  = 0 );
    void AddButton(  ParmButton& parm_button, const char* label );
    void AddButton( RadioButton& radio_button, const char* label, int val = 0);
    void AddInput(  StringInput& string_input, const char* label, int used_w = 0, double nline = 1 );
    void AddInput(  Input& input, const char* label, const char* format, int used_w = 0  );
    void AddInputEvenSpacedVector(Input& start_input, Input& end_input, Input& npts_input,const char * label,const char * format);
    void AddOutput( StringOutput& string_output );
    void AddOutput( StringOutput& string_output, const char* label, int used_w = 0 );
    void AddOutput( Output& output, const char* label, const char* format, int used_w = 0 );
    void AddIndexSelector( IndexSelector& selector, const char* label = nullptr, int used_w = 0 );
    void AddColorPicker( ColorPicker& picker );
    void AddChoice( Choice & choice, const char* label, int used_w = 0 );
    void AddCounter( Counter & count, const char* label = "", int used_w = 0 );
    void AddParmPicker( ParmPicker & parm_picker );
    void AddParmTreePicker( ParmTreePicker & parm_tree_picker, int w, int h );
    void AddDriverGroupBank( DriverGroupBank & driver_group, const vector < string > &labels,
                             double range, const char* format );
    void AddSkinControl( SkinControl & skin_control, const char* label, double range, const char* format );
    void AddSkinHeader( SkinHeader & skin_header, bool addcontchoice = true );
    void AddGeomPicker( GeomPicker & geom_picker, int used_w = 0, string text = string() );
    
    void AddPCurveEditor( PCurveEditor& curve_editor );

    Fl_Browser* AddFlBrowser( int height );
    ColResizeBrowser* AddColResizeBrowser( int* width_array_ptr, size_t num_col, int height );
    Fl_Text_Editor* AddFlTextEditor( int height , bool resizable = false );
    VspTextEditor* AddVspTextEditor( int height, Fl_Text_Buffer* buffer, Fl_Callback* cb, VspScreen* screen, bool resizable = false, Fl_Font font = FL_COURIER );
    Fl_Text_Display* AddFlTextDisplay( int height );
    Fl_Tree* AddFlTree( int height, bool resizable = false );
    TreeWithColumns* AddTreeWithColumns( int height, bool resizable = false );
    TreeWithIcons* AddTreeWithIcons( int height, bool resizable = false );
    Fl_Terminal* AddFlTerminal( int height );

    Fl_Scroll* AddFlScroll( int height );
    Fl_Sys_Menu_Bar* AddFlMenuBar( int height );

    Fl_Color_Chooser* AddFlColorChooser( int height );

    template < typename T >
    SpreadSheet<T> * AddSpreadSheet( int height, const char *format );

    //==== Add Another GroupLayout as a SubSet of This GroupLayout ====//
    //==== Subgroups can be Used To Create Multiple Column Layouts ====//
    void AddSubGroupLayout( GroupLayout& layout, int w, int h );

    //==== Standard Non-Parm Components ====//
    Fl_Check_Browser* AddCheckBrowser( int h );

private:

    void Init();
    void NewLineX( );
    int  FitWidth( int used_w, int default_w );

    VspButton* AddParmButton( const char* label );

    VspScreen* m_Screen;
    Fl_Group* m_Group;

    int m_X;
    int m_Y;
    int m_StartX;
    int m_StartY;
    int m_W;
    int m_H;

    bool m_FitWidthFlag;
    bool m_SameLineFlag;

    int m_StdHeight;
    int m_DividerHeight;
    int m_GapHeight;
    int m_ButtonWidth;
    int m_ChoiceButtonWidth;
    int m_RangeButtonWidth;
    int m_InputWidth;
    int m_SliderWidth;
    int m_CanvasWidth;
    int m_CanvasHeight;

    GroupLayout( GroupLayout const& copy ) = delete;          // Not Implemented
    GroupLayout& operator=( GroupLayout const& copy ) = delete; // Not Implemented

    vector< SliderAdjRangeInput* > m_Slider_Cleanup;
};

void AddPointLine( const vector <double> & xdata, const vector <double> & ydata, int linewidth, Fl_Color color, int pointsize = 0, int pointstyle = 1 );
void AddPoint( const double & x, const double & y, Fl_Color color, int pointsize = 2, int pointstyle = CA_ROUND );
void AddPoint( const vector <double> & xdata,  const vector <double> & ydata, Fl_Color color, int pointsize = 2, int pointstyle = CA_ROUND );
Fl_Color ColorWheel( int i, int nunique, int nhue = 7 );
int StyleWheel( int i );
void DebugLabelSize( Fl_Widget *widget );


template < typename T >
SpreadSheet<T> * GroupLayout::AddSpreadSheet( int height, const char *format )
{
    assert( m_Group && m_Screen );

    SpreadSheet<T> * sheet = new SpreadSheet<T>( m_X, m_Y, m_W, height, "" );
    sheet->SetFormat( format );

    m_Group->add( sheet );

    // Table rows
    sheet->row_header( 1 );
    sheet->row_header_width( m_ButtonWidth );
    sheet->row_resize( 1 );
    sheet->row_default_height( m_StdHeight );
    // Table cols
    sheet->col_header( 1 );
    sheet->col_header_height( m_StdHeight );
    sheet->col_resize( 1 );
    sheet->col_default_width( m_InputWidth );

    AddX( m_W );

    AddY( height );
    NewLineX();

    return sheet;
}


#endif // !defined(GUIDEVICE__INCLUDED_)
