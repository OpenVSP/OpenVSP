//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// RelAbsSlider.h: A slider on a size that is given either as a ratio to another or as itself.
//
//////////////////////////////////////////////////////////////////////

#if !defined(RELABSSLIDER__INCLUDED_)
#define RELABSSLIDER__INCLUDED_

#include "ScreenBase.h"
#include "GuiDevice.h"
#include "GroupLayout.h"

// Rel and Abs toggle buttons, over the two inputs of a slider that follows, at the width of an input each.  The
// slider's first input is the ratio, its second the size itself, and the flag says which of them is the input.
class RelAbsSlider
{
public:
    void Init( GroupLayout & layout, VspScreen* screen, const char* label, double range = 10.0,
               const char* format = "%7.3f" )
    {
        layout.SetFitWidthFlag( false );
        layout.SetSameLineFlag( true );
        layout.AddX( layout.GetW() - 2 * layout.GetInputWidth() );
        layout.SetButtonWidth( layout.GetInputWidth() );

        m_Group.Init( screen );
        layout.AddButton( m_RelButton, "Rel" );
        layout.AddButton( m_AbsButton, "Abs" );
        m_Group.AddButton( m_RelButton.GetFlButton() );
        m_Group.AddButton( m_AbsButton.GetFlButton() );
        vector< int > vals = { 1, 0 };
        m_Group.SetValMapVec( vals );

        layout.SetFitWidthFlag( true );
        layout.SetSameLineFlag( false );
        layout.ForceNewLine();
        layout.SetButtonWidth( layout.GetChoiceButtonWidth() );

        layout.AddSlider( m_Slider, label, range, format );
    }

    // Bind to the flag, the ratio and the size.
    void Update( const string & flag_id, const string & ratio_id, const string & abs_id, bool rel )
    {
        m_Group.Update( flag_id );
        if ( rel )
        {
            m_Slider.Update( 1, ratio_id, abs_id );
            m_Slider.ActivateInput1();
        }
        else
        {
            m_Slider.Update( 2, ratio_id, abs_id );
            m_Slider.ActivateInput2();
        }
    }

    void Deactivate()
    {
        m_Group.Deactivate();
        m_Slider.Deactivate();
    }

protected:
    ToggleRadioGroup m_Group;
    ToggleButton m_RelButton;
    ToggleButton m_AbsButton;
    SliderAdjRange2Input m_Slider;
};

#endif // !defined(RELABSSLIDER__INCLUDED_)
