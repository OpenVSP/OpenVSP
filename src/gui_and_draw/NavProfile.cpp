//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// NavProfile.cpp: which mouse buttons, modifiers and keys do what in a 3D view.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>

#include <FL/Enumerations.H>

#include "NavProfile.h"

namespace VSPGUI
{

static const int NAV_ALL_BUTTONS = FL_BUTTON1 | FL_BUTTON2 | FL_BUTTON3;
static const int NAV_ALL_MODIFIERS = FL_SHIFT | FL_CTRL | FL_ALT | FL_META;

bool operator==( const NavBinding & a, const NavBinding & b )
{
    return a.m_Buttons == b.m_Buttons &&
           a.m_OtherButtons == b.m_OtherButtons &&
           a.m_Modifiers == b.m_Modifiers &&
           a.m_OtherModifiers == b.m_OtherModifiers &&
           a.m_HeldKey == b.m_HeldKey &&
           a.m_PressedButton == b.m_PressedButton &&
           a.m_Trigger == b.m_Trigger &&
           a.m_Where == b.m_Where &&
           a.m_Conditions == b.m_Conditions &&
           a.m_Action == b.m_Action &&
           a.m_PickFirst == b.m_PickFirst;
}

bool operator!=( const NavBinding & a, const NavBinding & b )
{
    return !( a == b );
}

NavBinding NavBind( int buttons, int modifiers, NavAction action )
{
    NavBinding b;
    b.m_Buttons = buttons;
    b.m_OtherButtons = 0;
    b.m_Modifiers = modifiers;
    b.m_OtherModifiers = 0;
    b.m_HeldKey = 0;
    b.m_PressedButton = 0;
    b.m_Trigger = NAV_ON_DRAG;
    b.m_Where = NAV_ANYWHERE;
    b.m_Conditions = 0;
    b.m_Action = action;
    b.m_PickFirst = false;
    return b;
}

NavBinding NavBindAtLeast( int buttons, int modifiers, NavAction action )
{
    NavBinding b = NavBind( buttons, modifiers, action );
    b.m_OtherButtons = NAV_ALL_BUTTONS & ~buttons;
    b.m_OtherModifiers = NAV_ALL_MODIFIERS & ~modifiers;
    return b;
}

NavPress::NavPress()
{
    m_Buttons = 0;
    m_Modifiers = 0;
    m_PressedButton = 0;
    m_Where = NAV_ANYWHERE;
    m_Conditions = 0;
}

NavProfile::NavProfile()
{
    m_ClickDistance = 0;
    m_ChordEnds = false;
}

// Fit Model's box selection takes the left button when it is the one pressed: with Shift, or
// always in box mode, while Fit Model is shown.  3.55 gave left + right to zoom first, so the
// right button may not be down.
static void AddBoxSelect( NavProfile & p )
{
    NavBinding box = NavBindAtLeast( FL_BUTTON1, FL_SHIFT, NAV_BOX_SELECT );
    box.m_OtherButtons = FL_BUTTON2;
    box.m_PressedButton = FL_BUTTON1;
    box.m_Conditions = NAV_COND_FIT_MODEL;
    p.m_Bindings.push_back( box );

    box = NavBindAtLeast( FL_BUTTON1, 0, NAV_BOX_SELECT );
    box.m_OtherButtons = FL_BUTTON2;
    box.m_PressedButton = FL_BUTTON1;
    box.m_Conditions = NAV_COND_FIT_MODEL | NAV_COND_SELECT_BOX;
    p.m_Bindings.push_back( box );
}

// In the presets the left button selects: a click selects the Geom under it.  Its press goes
// first to the tool picks (rotation center, rulers, probes, ...); a drag does nothing.
static void AddLeftPick( NavProfile & p )
{
    NavBinding pick = NavBind( FL_BUTTON1, 0, NAV_NONE );
    pick.m_OtherModifiers = FL_SHIFT | FL_CTRL | FL_ALT | FL_META;
    pick.m_PickFirst = true;
    p.m_Bindings.push_back( pick );

    NavBinding select = NavBind( FL_BUTTON1, 0, NAV_SELECT );
    select.m_OtherModifiers = FL_SHIFT | FL_CTRL | FL_META;
    select.m_Trigger = NAV_ON_CLICK;
    p.m_Bindings.push_back( select );
}

// On macOS, where Cmd does much of what Ctrl does elsewhere, a Cmd version of each Ctrl binding
// whose press is free.
static void AddCmdVariants( NavProfile & p )
{
#ifdef __APPLE__
    std::vector < NavBinding > bindings;
    for ( int i = 0; i < ( int )p.m_Bindings.size(); i++ )
    {
        const NavBinding & b = p.m_Bindings[i];
        bindings.push_back( b );

        if ( ( b.m_Modifiers & FL_CTRL ) == 0 || ( b.m_Modifiers & FL_META ) != 0 )
        {
            continue;
        }

        NavBinding cmd = b;
        cmd.m_Modifiers = ( b.m_Modifiers & ~FL_CTRL ) | FL_META;

        NavPress press;
        press.m_Buttons = cmd.m_Buttons;
        press.m_Modifiers = cmd.m_Modifiers;
        press.m_PressedButton = cmd.m_PressedButton;
        press.m_Where = cmd.m_Where;
        press.m_Conditions = cmd.m_Conditions;
        const NavBinding * taken = p.Match( press, cmd.m_Trigger );
        if ( !taken || taken->m_Action == NAV_NONE )
        {
            bindings.push_back( cmd );
        }
    }
    p.m_Bindings = bindings;
#endif
}

// A click on the model with these buttons and modifiers makes the point there the rotation
// center, and centers it.
static void AddSetPivotClick( NavProfile & p, int buttons, int modifiers )
{
    NavBinding b = NavBind( buttons, modifiers, NAV_SET_PIVOT );
    b.m_Trigger = NAV_ON_CLICK;
    p.m_Bindings.push_back( b );
}

// Common to the presets: clicks are told from drags, and chords end when a button lets go.
static NavProfile NewPreset( const char * name )
{
    NavProfile p;
    p.m_Name = name;
    p.m_ClickDistance = 4;
    p.m_ChordEnds = true;
    AddBoxSelect( p );
    return p;
}

const char * NavProfile::PresetKey( int preset )
{
    switch ( preset )
    {
    case NAV_PRESET_SOLIDWORKS:
        return "SolidWorks";
    case NAV_PRESET_CREO_BLENDER:
        return "CreoBlender";
    case NAV_PRESET_NX:
        return "NX";
    case NAV_PRESET_CATIA:
        return "CATIA";
    case NAV_PRESET_INVENTOR_FUSION:
        return "InventorFusion";
    case NAV_PRESET_3DS_MAX:
        return "3dsMax";
    case NAV_PRESET_MAYA:
        return "Maya";
    case NAV_PRESET_ABAQUS:
        return "Abaqus";
    case NAV_PRESET_RHINO:
        return "Rhino";
    case NAV_PRESET_ONSHAPE:
        return "Onshape";
    case NAV_PRESET_POINTWISE:
        return "Pointwise";
    case NAV_PRESET_HYPERMESH:
        return "HyperMesh";
    case NAV_PRESET_HYPERWORKS:
        return "HyperWorks";
    case NAV_PRESET_PARAVIEW:
        return "ParaView";
    case NAV_PRESET_ENSIGHT:
        return "EnSight";
    case NAV_PRESET_STARCCM:
        return "StarCCM";
    case NAV_PRESET_TECPLOT:
        return "Tecplot";
    default:
        return "OpenVSP";
    }
}

const char * NavProfile::PresetAlsoLike( int preset )
{
    switch ( preset )
    {
    case NAV_PRESET_SOLIDWORKS:
        return "ANSYS Mechanical, DesignModeler, Femap";
    case NAV_PRESET_CREO_BLENDER:
        return "Solid Edge, SketchUp, SpaceClaim, Patran";
    case NAV_PRESET_NX:
        return "FreeCAD";
    case NAV_PRESET_CATIA:
        return "3DEXPERIENCE CATIA";
    case NAV_PRESET_MAYA:
        return "Blender three-button emulation";
    case NAV_PRESET_HYPERMESH:
        return "before 2021, or its Legacy setting";
    case NAV_PRESET_HYPERWORKS:
        return "HyperMesh 2021 and later";
    case NAV_PRESET_OPENVSP:
        return "MATLAB";
    default:
        return "";
    }
}

std::string NavProfile::PresetLabel( int preset )
{
    std::string label = Preset( preset ).m_Name;
    std::string also = PresetAlsoLike( preset );
    if ( !also.empty() )
    {
        label += "  (" + also + ")";
    }
    return label;
}

NavProfile NavProfile::Preset( int preset )
{
    NavProfile p;

    switch ( preset )
    {
    case NAV_PRESET_SOLIDWORKS:
        // Middle button rotates; Ctrl pans, Shift zooms (family A1).
        p = NewPreset( "SolidWorks-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_SHIFT, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_ALT, NAV_ROLL ) );
        AddSetPivotClick( p, FL_BUTTON2, 0 );
        AddLeftPick( p );
        break;

    case NAV_PRESET_CREO_BLENDER:
        // Middle button rotates; Shift pans, Ctrl zooms (family A2).
        p = NewPreset( "Creo / Blender-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_SHIFT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ZOOM ) );
        AddSetPivotClick( p, FL_BUTTON2, FL_ALT );
        AddLeftPick( p );
        break;

    case NAV_PRESET_NX:
        // Middle button rotates; a second button with it pans (right) or zooms (left) (family B).
        p = NewPreset( "NX-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2 | FL_BUTTON3, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2 | FL_BUTTON1, 0, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_SHIFT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_CATIA:
        // Middle button pans; a second button with it rotates; Ctrl zooms.  A middle click
        // centers the point there.  Alt+right stands in for the middle button.
        p = NewPreset( "CATIA-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2 | FL_BUTTON1, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2 | FL_BUTTON3, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3 | FL_BUTTON1, FL_ALT, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_ALT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL | FL_ALT, NAV_ZOOM ) );
        AddSetPivotClick( p, FL_BUTTON2, 0 );
        AddLeftPick( p );
        break;

    case NAV_PRESET_INVENTOR_FUSION:
        // Middle button pans; Shift rotates, Ctrl+Shift zooms (family C).  Inventor's held
        // F2, F3 and F4 are left out: they are OpenVSP's saved views.
        p = NewPreset( "Inventor / Fusion-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_SHIFT, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL | FL_SHIFT, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_3DS_MAX:
        // Middle button pans; Alt rotates, Ctrl+Alt zooms.
        p = NewPreset( "3ds Max-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_ALT, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL | FL_ALT, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_MAYA:
        // Alt with each button: left rotates, middle pans, right zooms (family D).  Shift and
        // Ctrl with Alt+left pan and zoom, as Blender's three-button emulation does.
        p = NewPreset( "Maya-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_ALT, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_ALT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_ALT, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_SHIFT | FL_ALT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_CTRL | FL_ALT, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_ABAQUS:
        // Ctrl+Alt with each button: left rotates, middle pans, right zooms (family D).
        p = NewPreset( "Abaqus-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_CTRL | FL_ALT, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL | FL_ALT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL | FL_ALT, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_RHINO:
        // Right button rotates; Shift pans, Ctrl zooms (family E).
        p = NewPreset( "Rhino-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_SHIFT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_ONSHAPE:
        // Right button rotates; Ctrl+right or the middle button pans (family E).
        p = NewPreset( "Onshape-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_PAN ) );
        AddLeftPick( p );
        break;

    case NAV_PRESET_POINTWISE:
        // Ctrl+right rotates, Shift+right pans; the middle button zooms (family E').
        p = NewPreset( "Pointwise-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_SHIFT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ROLL ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_SHIFT, NAV_ZOOM_BOX ) );
        AddSetPivotClick( p, FL_BUTTON3, FL_CTRL | FL_SHIFT );
        AddLeftPick( p );
        break;

    case NAV_PRESET_HYPERMESH:
        // Ctrl with each button: left rotates, right pans, middle draws a zoom box.  A Ctrl+left
        // click centers the point there.
        p = NewPreset( "HyperMesh-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_CTRL, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ZOOM_BOX ) );
        AddSetPivotClick( p, FL_BUTTON1, FL_CTRL );
        AddLeftPick( p );
        break;

    case NAV_PRESET_HYPERWORKS:
        // Middle button rotates; right, or left and right together, pans; Alt+middle draws a
        // zoom box.  A Ctrl+middle click centers the point there.
        p = NewPreset( "HyperWorks-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1 | FL_BUTTON3, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_ALT, NAV_ZOOM_BOX ) );
        AddSetPivotClick( p, FL_BUTTON2, FL_CTRL );
        AddLeftPick( p );
        break;

    case NAV_PRESET_PARAVIEW:
    {
        // Left rotates, middle pans, right zooms; Shift and Ctrl give its other defaults (family F).
        p = NewPreset( "ParaView-like" );
        NavBinding rotate = NavBind( FL_BUTTON1, 0, NAV_ROTATE );
        rotate.m_PickFirst = true;
        p.m_Bindings.push_back( rotate );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_SHIFT, NAV_ROLL ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_SHIFT, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_SHIFT, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_ZOOM ) );
        AddLeftPick( p );
        break;
    }

    case NAV_PRESET_ENSIGHT:
    {
        // Left rotates, middle pans, right zooms; Ctrl turns each to its third axis or pans.
        p = NewPreset( "EnSight-like" );
        NavBinding rotate = NavBind( FL_BUTTON1, 0, NAV_ROTATE );
        rotate.m_PickFirst = true;
        p.m_Bindings.push_back( rotate );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_CTRL, NAV_ROLL ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, FL_CTRL, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_PAN ) );
        AddLeftPick( p );
        break;
    }

    case NAV_PRESET_STARCCM:
    {
        // Left rotates, Ctrl+left rolls, middle zooms, right pans.
        p = NewPreset( "STAR-CCM+-like" );
        NavBinding rotate = NavBind( FL_BUTTON1, 0, NAV_ROTATE );
        rotate.m_PickFirst = true;
        p.m_Bindings.push_back( rotate );
        p.m_Bindings.push_back( NavBind( FL_BUTTON1, FL_CTRL, NAV_ROLL ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ZOOM ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_PAN ) );
        AddLeftPick( p );
        break;
    }

    case NAV_PRESET_TECPLOT:
        // As with Tecplot's Selector tool: right pans, Ctrl+right rotates, middle zooms.
        p = NewPreset( "Tecplot-like" );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, 0, NAV_PAN ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON3, FL_CTRL, NAV_ROTATE ) );
        p.m_Bindings.push_back( NavBind( FL_BUTTON2, 0, NAV_ZOOM ) );
        AddLeftPick( p );
        break;

    default:
        p = Classic();
        p.m_Name = "OpenVSP";
        return p;
    }

    AddCmdVariants( p );
    return p;
}

NavProfile NavProfile::Classic()
{
    NavProfile p;
    p.m_Name = "OpenVSP Classic";

    // 3.55 tested each binding's buttons and modifiers as a subset of those down, so every
    // binding here allows any others.  Order matters: the first match wins.
    AddBoxSelect( p );

    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1 | FL_BUTTON3, 0, NAV_ZOOM ) );

    NavBinding roll = NavBindAtLeast( FL_BUTTON1, 0, NAV_ROLL );
    roll.m_HeldKey = 'z';
    p.m_Bindings.push_back( roll );

    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1, FL_ALT, NAV_PAN ) );
    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1, FL_CTRL, NAV_ZOOM ) );
    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON1, FL_META, NAV_ZOOM ) );

    NavBinding rotate = NavBindAtLeast( FL_BUTTON1, 0, NAV_ROTATE );
    rotate.m_PickFirst = true;
    p.m_Bindings.push_back( rotate );

    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON2, 0, NAV_ZOOM ) );
    p.m_Bindings.push_back( NavBindAtLeast( FL_BUTTON3, 0, NAV_PAN ) );

    return p;
}

// True if the binding matches the press, whatever its trigger.
static bool Matches( const NavBinding & b, const NavPress & press )
{
    if ( ( press.m_Buttons & ~b.m_OtherButtons ) != b.m_Buttons )
    {
        return false;
    }

    if ( ( press.m_Modifiers & ~b.m_OtherModifiers ) != b.m_Modifiers )
    {
        return false;
    }

    if ( b.m_HeldKey != 0 && press.m_HeldKeys.count( b.m_HeldKey ) == 0 )
    {
        return false;
    }

    if ( b.m_PressedButton != 0 && press.m_PressedButton != b.m_PressedButton )
    {
        return false;
    }

    if ( b.m_Where != NAV_ANYWHERE && b.m_Where != press.m_Where )
    {
        return false;
    }

    if ( ( press.m_Conditions & b.m_Conditions ) != b.m_Conditions )
    {
        return false;
    }

    return true;
}

const NavBinding * NavProfile::Match( const NavPress & press, int trigger ) const
{
    for ( int i = 0; i < ( int )m_Bindings.size(); i++ )
    {
        const NavBinding & b = m_Bindings[i];

        if ( b.m_Trigger == trigger && Matches( b, press ) )
        {
            return &b;
        }
    }
    return nullptr;
}

const char * NavModifierName( int bit )
{
#ifdef __APPLE__
    const char * meta = "Cmd";
    const char * alt = "Option";
#elif defined( _WIN32 )
    const char * meta = "Win";
    const char * alt = "Alt";
#else
    const char * meta = "Super";
    const char * alt = "Alt";
#endif

    if ( bit == FL_SHIFT )
    {
        return "Shift";
    }
    if ( bit == FL_CTRL )
    {
        return "Ctrl";
    }
    if ( bit == FL_ALT )
    {
        return alt;
    }
    if ( bit == FL_META )
    {
        return meta;
    }
    return "";
}

static std::string DescribeKeys( int buttons, int modifiers, int key )
{
    std::string d;
    if ( key != 0 )
    {
        d += ( char )key;
        d += "+";
    }

    const int mods[] = { FL_SHIFT, FL_CTRL, FL_ALT, FL_META };
    for ( int i = 0; i < 4; i++ )
    {
        if ( modifiers & mods[i] )
        {
            d += NavModifierName( mods[i] );
            d += "+";
        }
    }

    std::vector < std::string > names;
    if ( buttons & FL_BUTTON1 )
    {
        names.push_back( "Left" );
    }
    if ( buttons & FL_BUTTON2 )
    {
        names.push_back( "Middle" );
    }
    if ( buttons & FL_BUTTON3 )
    {
        names.push_back( "Right" );
    }
    for ( int i = 0; i < ( int )names.size(); i++ )
    {
        if ( i > 0 )
        {
            d += "+";
        }
        d += names[i];
    }
    return d;
}

std::string NavDescribe( const NavBinding & b )
{
    std::string d = DescribeKeys( b.m_Buttons, b.m_Modifiers, b.m_HeldKey );
    if ( b.m_Trigger == NAV_ON_CLICK )
    {
        d += " click";
    }
    else
    {
        d += " drag";
    }
    return d;
}

std::string NavProfile::Summary() const
{
    const NavAction actions[] = { NAV_ROTATE, NAV_PAN, NAV_ZOOM, NAV_ROLL, NAV_ZOOM_BOX, NAV_SET_PIVOT, NAV_SELECT };
    const char * labels[] = { "Rotate", "Pan", "Zoom", "Roll", "Zoom box", "Rotation center", "Select" };

    std::string s;
    for ( int a = 0; a < 7; a++ )
    {
        std::string list;
        for ( int i = 0; i < ( int )m_Bindings.size(); i++ )
        {
            if ( m_Bindings[i].m_Action == actions[a] )
            {
                if ( !list.empty() )
                {
                    list += ", ";
                }
                list += NavDescribe( m_Bindings[i] );
            }
        }
        if ( !list.empty() )
        {
            if ( !s.empty() )
            {
                s += "; ";
            }
            s += labels[a];
            s += ": ";
            s += list;
        }
    }
    return s;
}

bool NavProfile::NeedsWhere() const
{
    for ( int i = 0; i < ( int )m_Bindings.size(); i++ )
    {
        if ( m_Bindings[i].m_Where != NAV_ANYWHERE )
        {
            return true;
        }
    }
    return false;
}

// Names for the text form.  These are stored, so they never change.
static const char * const ACTION_NAMES[] = { "None", "Rotate", "Roll", "Pan", "Zoom", "ZoomBox", "BoxSelect", "SetPivot", "Select" };
static const int NUM_ACTIONS = 9;

struct NavBit
{
    int m_Bit;
    const char * m_Name;
};

static const NavBit BUTTON_BITS[] = { { FL_BUTTON1, "Left" }, { FL_BUTTON2, "Middle" }, { FL_BUTTON3, "Right" } };
static const NavBit MODIFIER_BITS[] = { { FL_SHIFT, "Shift" }, { FL_CTRL, "Ctrl" }, { FL_ALT, "Alt" }, { FL_META, "Meta" } };
static const NavBit CONDITION_BITS[] = { { NAV_COND_FIT_MODEL, "FitModel" }, { NAV_COND_SELECT_BOX, "SelectBox" } };

// "Left+Middle" for those bits.
static std::string BitsText( int bits, const NavBit * set, int n )
{
    std::string t;
    for ( int i = 0; i < n; i++ )
    {
        if ( bits & set[i].m_Bit )
        {
            if ( !t.empty() )
            {
                t += "+";
            }
            t += set[i].m_Name;
        }
    }
    return t;
}

// As BitsText, but "Any" stands for every bit of the set but the required ones in except.
static std::string OtherBitsText( int bits, const NavBit * set, int n, int except )
{
    int all = 0;
    for ( int i = 0; i < n; i++ )
    {
        all |= set[i].m_Bit;
    }
    if ( bits != 0 && bits == ( all & ~except ) )
    {
        return "Any";
    }
    return BitsText( bits, set, n );
}

static bool ParseBits( const std::string & text, const NavBit * set, int n, int & bits )
{
    bits = 0;
    if ( text.empty() )
    {
        return true;
    }

    size_t start = 0;
    while ( start <= text.size() )
    {
        size_t end = text.find( '+', start );
        if ( end == std::string::npos )
        {
            end = text.size();
        }
        std::string name = text.substr( start, end - start );

        bool found = false;
        for ( int i = 0; i < n && !found; i++ )
        {
            if ( name == set[i].m_Name )
            {
                bits |= set[i].m_Bit;
                found = true;
            }
        }
        if ( !found )
        {
            return false;
        }
        start = end + 1;
    }
    return true;
}

static bool ParseOtherBits( const std::string & text, const NavBit * set, int n, int except, int & bits )
{
    if ( text == "Any" )
    {
        bits = 0;
        for ( int i = 0; i < n; i++ )
        {
            bits |= set[i].m_Bit;
        }
        bits &= ~except;
        return true;
    }
    return ParseBits( text, set, n, bits );
}

static std::string Trim( const std::string & s )
{
    size_t a = s.find_first_not_of( " \t\r\n" );
    if ( a == std::string::npos )
    {
        return std::string();
    }
    size_t b = s.find_last_not_of( " \t\r\n" );
    return s.substr( a, b - a + 1 );
}

std::string NavProfile::BindingsText() const
{
    std::string t;
    for ( int i = 0; i < ( int )m_Bindings.size(); i++ )
    {
        const NavBinding & b = m_Bindings[i];
        NavBinding def = NavBind( b.m_Buttons, b.m_Modifiers, b.m_Action );

        t += ACTION_NAMES[ b.m_Action ];
        t += ": Buttons=" + BitsText( b.m_Buttons, BUTTON_BITS, 3 );
        if ( b.m_Modifiers != 0 )
        {
            t += " Mods=" + BitsText( b.m_Modifiers, MODIFIER_BITS, 4 );
        }
        if ( b.m_OtherButtons != def.m_OtherButtons )
        {
            t += " OtherButtons=" + OtherBitsText( b.m_OtherButtons, BUTTON_BITS, 3, b.m_Buttons );
        }
        if ( b.m_OtherModifiers != def.m_OtherModifiers )
        {
            t += " OtherMods=" + OtherBitsText( b.m_OtherModifiers, MODIFIER_BITS, 4, b.m_Modifiers );
        }
        if ( b.m_HeldKey != 0 )
        {
            t += " Key=" + std::to_string( b.m_HeldKey );
        }
        if ( b.m_PressedButton != 0 )
        {
            t += " Press=" + BitsText( b.m_PressedButton, BUTTON_BITS, 3 );
        }
        if ( b.m_Trigger == NAV_ON_CLICK )
        {
            t += " Trigger=Click";
        }
        if ( b.m_Where == NAV_ON_GEOMETRY )
        {
            t += " Where=Geometry";
        }
        else if ( b.m_Where == NAV_ON_BACKGROUND )
        {
            t += " Where=Background";
        }
        if ( b.m_Conditions != 0 )
        {
            t += " When=" + BitsText( b.m_Conditions, CONDITION_BITS, 2 );
        }
        if ( b.m_PickFirst )
        {
            t += " PickFirst";
        }
        t += "\n";
    }
    return t;
}

bool NavProfile::SetBindingsText( const std::string & text )
{
    std::vector < NavBinding > bindings;

    size_t start = 0;
    while ( start < text.size() )
    {
        size_t end = text.find( '\n', start );
        if ( end == std::string::npos )
        {
            end = text.size();
        }
        std::string line = Trim( text.substr( start, end - start ) );
        start = end + 1;
        if ( line.empty() )
        {
            continue;
        }

        size_t colon = line.find( ':' );
        if ( colon == std::string::npos )
        {
            return false;
        }
        std::string action = Trim( line.substr( 0, colon ) );
        int ia = -1;
        for ( int i = 0; i < NUM_ACTIONS; i++ )
        {
            if ( action == ACTION_NAMES[i] )
            {
                ia = i;
            }
        }
        if ( ia < 0 )
        {
            return false;
        }

        // Read the fields first: the defaults of the "other" masks depend on the required ones.
        std::string buttons, mods, other_buttons, other_mods, key, press, trigger, where, when;
        bool pick = false;
        bool has_other_buttons = false;
        bool has_other_mods = false;

        std::string rest = line.substr( colon + 1 );
        size_t f = 0;
        while ( f < rest.size() )
        {
            size_t g = rest.find( ' ', f );
            if ( g == std::string::npos )
            {
                g = rest.size();
            }
            std::string field = rest.substr( f, g - f );
            f = g + 1;
            if ( field.empty() )
            {
                continue;
            }
            if ( field == "PickFirst" )
            {
                pick = true;
                continue;
            }

            size_t eq = field.find( '=' );
            if ( eq == std::string::npos )
            {
                return false;
            }
            std::string k = field.substr( 0, eq );
            std::string v = field.substr( eq + 1 );
            if ( k == "Buttons" )
            {
                buttons = v;
            }
            else if ( k == "Mods" )
            {
                mods = v;
            }
            else if ( k == "OtherButtons" )
            {
                other_buttons = v;
                has_other_buttons = true;
            }
            else if ( k == "OtherMods" )
            {
                other_mods = v;
                has_other_mods = true;
            }
            else if ( k == "Key" )
            {
                key = v;
            }
            else if ( k == "Press" )
            {
                press = v;
            }
            else if ( k == "Trigger" )
            {
                trigger = v;
            }
            else if ( k == "Where" )
            {
                where = v;
            }
            else if ( k == "When" )
            {
                when = v;
            }
            else
            {
                return false;
            }
        }

        int ib = 0;
        int im = 0;
        if ( !ParseBits( buttons, BUTTON_BITS, 3, ib ) || ib == 0 || !ParseBits( mods, MODIFIER_BITS, 4, im ) )
        {
            return false;
        }
        NavBinding b = NavBind( ib, im, ( NavAction )ia );

        if ( has_other_buttons && !ParseOtherBits( other_buttons, BUTTON_BITS, 3, ib, b.m_OtherButtons ) )
        {
            return false;
        }
        if ( has_other_mods && !ParseOtherBits( other_mods, MODIFIER_BITS, 4, im, b.m_OtherModifiers ) )
        {
            return false;
        }
        if ( !key.empty() )
        {
            char * stop = nullptr;
            long k = strtol( key.c_str(), &stop, 10 );
            if ( *stop != '\0' || k <= 0 || k > 0xffff )
            {
                return false;
            }
            b.m_HeldKey = ( int )k;
        }
        if ( !press.empty() && !ParseBits( press, BUTTON_BITS, 3, b.m_PressedButton ) )
        {
            return false;
        }
        if ( trigger == "Click" )
        {
            b.m_Trigger = NAV_ON_CLICK;
        }
        else if ( !trigger.empty() && trigger != "Drag" )
        {
            return false;
        }
        if ( where == "Geometry" )
        {
            b.m_Where = NAV_ON_GEOMETRY;
        }
        else if ( where == "Background" )
        {
            b.m_Where = NAV_ON_BACKGROUND;
        }
        else if ( !where.empty() && where != "Anywhere" )
        {
            return false;
        }
        if ( !when.empty() && !ParseBits( when, CONDITION_BITS, 2, b.m_Conditions ) )
        {
            return false;
        }
        b.m_PickFirst = pick;

        bindings.push_back( b );
    }

    m_Bindings = bindings;
    return true;
}

bool NavProfile::SameBehavior( const NavProfile & other ) const
{
    return m_Bindings == other.m_Bindings &&
           m_ClickDistance == other.m_ClickDistance &&
           m_ChordEnds == other.m_ChordEnds;
}

}
