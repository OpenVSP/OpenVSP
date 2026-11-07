//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// QaspFieldGuide.h
//
// QASP addition (not present in upstream OpenVSP).
// An in-application help window aimed at people who are not aerospace engineers:
// plain-English definitions, "what happens if I change this" scenarios, a
// step-by-step first-aircraft walkthrough, and an explanation of what each
// analysis tool is for and how to read its output.
//
// The window is created on first use and re-raised afterwards, so it does not
// interfere with the normal ScreenMgr screen list.
//

#ifndef QASP_FIELD_GUIDE_H
#define QASP_FIELD_GUIDE_H

class Fl_Window;
class Fl_Browser;
class Fl_Text_Display;
class Fl_Text_Buffer;
class Fl_Input;
class Fl_Widget;

class QaspFieldGuide
{
public:

    // Opens the Field Guide (creates the window the first time it is called).
    static void Show();

private:

    QaspFieldGuide() {}
    virtual ~QaspFieldGuide() {}

    static void BuildWindow();

    static void TopicCB( Fl_Widget* w, void* data );
    static void CloseCB( Fl_Widget* w, void* data );
    static void SearchCB( Fl_Widget* w, void* data );

    static void LoadTopic( int index );
    static void FilterTopics();

    static Fl_Window*       s_Window;
    static Fl_Browser*      s_Topics;
    static Fl_Text_Display* s_Text;
    static Fl_Text_Buffer*  s_Buffer;
    static Fl_Input*        s_Search;

    // Index of the topic currently displayed (-1 for none).
    static int              s_Current;
};

#endif
