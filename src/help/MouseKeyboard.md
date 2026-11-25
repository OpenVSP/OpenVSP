---
title:  'Mouse and Keyboard'
---

Most of OpenVSP is driven through its windows' visible controls, but a good deal of it also answers to the mouse and
keyboard in ways the controls do not show.  This page gathers all of them in one place: how the mouse moves the 3D
view and how to change that, the keyboard shortcuts, and what the Parm controls, lists and trees do beyond their
obvious use, including where a Parm can be dragged and dropped.

| Topic | Where |
|---|---|
| [Windows](#windows) | File > Preferences > General |
| [Toolbar](#toolbar) | under the menu bar |
| [Moving the view](#moving-the-view) | the 3D window |
| [Picking and selecting](#picking-and-selecting) | the 3D window |
| [Keyboard in the 3D window](#keyboard-in-the-3d-window) | the 3D window and View menu |
| [3D View preferences](#3d-view-preferences) | File > Preferences > 3D View |
| [Mouse button preferences](#mouse-button-preferences) | File > Preferences > Mouse Buttons |
| [SpaceMouse preferences](#spacemouse-preferences) | File > Preferences > SpaceMouse |
| [Schemes](#schemes) | File > Preferences |
| [Saving and loading preferences](#saving-and-loading-preferences) | File > Preferences |
| [Parm controls](#parm-controls) | every slider and value box |
| [Dragging and dropping Parms](#dragging-and-dropping-parms) | link, design variable and preset windows |
| [Plots](#plots) | results plots, the blade curve plot and the Edit Curve view |
| [Lists and trees](#lists-and-trees) | the Geom Browser and other lists |
| [Keyboard shortcuts](#keyboard-shortcuts) | menus and other windows |

On macOS, Option is the key other systems call Alt, and Cmd stands in for Ctrl in many shortcuts.  This page writes
Alt for Alt or Option.

## Windows

The Geom Browser sits in a panel along the right edge of the 3D view, its full height.  The button at the right of its
title hides it, and the small tab left at the right edge of the view, Model > Geometry or the
toolbar's Geometry browser button brings it back.

File > Preferences > General > Windows > Geom Browser sets where the panel goes.  Docked, the default, puts it beside
the view: the view narrows to make room while the browser is shown and widens again when it is hidden.  Floating puts
it over the 3D view instead.  A change takes effect at once and is kept for the next start.

File > Preferences > General > Windows > Screens sets how the other screens are shown.  With Free Windows, the
default, each is a window of its own.  With Captive Windows, each is a window inside the main window, over the 3D view:
drag its title bar to move it, drag its edges and corners to size it, and close it with the button at the right of its
title bar.  A captive window stays inside the main window, keeps its size as the main window is resized, and comes to
the top when it is clicked or typed in.  A change moves the open screens in or out at once and is kept for the next
start.

The Geom windows share one place, their right edges against the Geom Browser's panel, and each opens where the last
one was left.  A window made taller keeps its controls their size: its plot, 3D view, list or table, where it has one,
takes the extra height.

## Toolbar

The toolbar under the menu bar puts the commands used most a click away.  Each button does exactly what its menu
command does, and its tooltip names the command and its key.  A button for something that is on, such as Pick mode,
the rotation, the view layout, the axes or the border, is shown pressed in.  Every other button has a faint raised
edge, raised fully while the mouse is over it.

| Group | Buttons |
|---|---|
| Model | New, Open, Save, Undo |
| Geometry | the Geometry browser, Pick mode |
| View | Fit the selected Geoms, Fit everything, Zoom box, Center, and a menu of the standard views |
| Rotation | Ball, Turntable, Z Up, Set Rotation Center, View Normal To |
| Window | One view, two side by side, two one above the other, four views, the axes, the border |
| Output | Screenshot, Vector screenshot |
| At the right | Preferences, Help |

Window > Toolbar hides or shows it.  File > Preferences > General > Main Window > Toolbar also hides it or sets its
size: Small, Medium or Large.

## Moving the view

Out of the box the mouse moves the view as it always has in OpenVSP:

| Press | Does |
|---|---|
| Left drag | Rotate |
| Alt + left drag | Pan |
| Right drag | Pan |
| Middle drag | Zoom |
| Ctrl + left drag, or Cmd + left drag (the Windows or Super key elsewhere) | Zoom |
| Left + right drag | Zoom |
| z held + left drag | Roll about the view direction |
| Wheel | Zoom, about the point under the pointer |

The mouse buttons can instead follow the habits of another program; see [Schemes](#schemes).  Whatever the scheme, a
drag acts on the viewport it starts in, and the action is chosen when the drag starts: pressing or releasing a key
part way through does not switch from rotating to panning.  Esc during a drag puts the view back as it was at the
press.

### Ball and turntable

A rotate drag turns the view one of two ways.

- Ball: the model turns freely, as if it were inside a ball being rolled.  Any orientation can be reached.  What
  the ball does is set in the preferences: the Trackball (the default), the Rounded Arcball, OpenVSP Classic (the
  rotation of OpenVSP 3.55 and earlier), or the Tumbler.
- Turntable: horizontal motion turns the model about the vertical Z axis and vertical motion tips it toward or away
  from you, so Z always stays upright on the screen.  It suits aircraft, which have a natural up.

View > Turntable Rotation (t) and View > Ball Rotation (b) switch between them at any time; the preferences set which
one OpenVSP starts with.  Switching to the turntable first turns the view about the screen's axis until Z points up.
View > Z Up (z), or a tap of z on its own with no mouse button down, does that at any time.

The four balls differ in what a drag does:

- Rounded Arcball: the press point is grabbed on a sphere and follows the mouse, so a drag that comes back to where
  it started puts the view back exactly.  Starting a drag outside the sphere turns the view about the line of sight
  instead; the outline of the sphere is drawn while it turns.  Its size and speed are preferences.
- OpenVSP Classic: OpenVSP's rotation through version 3.55, kept exactly at the default Speed of 0.5.  Each mouse
  movement turns the view on its own, so moving the mouse in a circle and back to where it started leaves the view
  turned.
- Trackball: Bell's trackball surface, as in SGI's tools, measured from the press, so a drag that comes back to where
  it started puts the view back exactly.  Near the middle of the view it turns like OpenVSP Classic; farther out it
  turns the model less and less, tipping it on a move toward or away from the middle and rolling it on a move around.
- Tumbler: each mouse movement turns the view about the screen axis across it, by a fixed amount per point of
  movement.  Any orientation can be reached, but the result depends on the path: moving the mouse in a small circle
  rolls the model.  ParaView, EnSight, Pointwise and STAR-CCM+ turn this way.

### What the view turns about

The view turns about the rotation center.  View > Set Rotation Center (r) waits for a click on the model and moves
the rotation center there; View > Center (c) and Center All (C) put it at the middle of the selected or all Geoms.
With Rotate About set to Point under cursor in the preferences, each drag instead turns about the surface point under
the press, while the rotation center itself stays where it was.  A marker shows the point being turned about.

### Zooming

The wheel zooms about the point under the pointer by default; the preferences can make it zoom about the middle of
the view, reverse its direction, and scale its step with the amount scrolled, which suits trackpads.  A zoom drag
zooms about the middle of the view by default.

Zoom in and Zoom out can also be bound to a button, so that a press zooms one step and holding the button keeps
zooming; a mouse's Back and Forward side buttons suit them.  See [Mouse button preferences](#mouse-button-preferences).

View > Zoom Box (w) arms a one-time zoom box: the next left drag draws a rectangle, and the view zooms so the
rectangle fills it.  Esc, or a press of any other button, gives the box up.

### Saved views

F1 to F4 recall four saved views; Shift with F1 to F4 saves the current view into that slot.  The saved views last
until OpenVSP closes.  F5 to F12 show the standard views: top, front, left, left isometric, bottom, rear, right and
right isometric.

### Rotatrix trackball

When OpenVSP is built with the Rotatrix SDK and the Rotatrix host is running, the Rotatrix trackball moves the view.
The trackball icon at the bottom left of the main window is lit while it is connected; its tooltip gives the status, and
a click opens its preferences.  View > Rotatrix Trackball (d) turns it on or off.  Rotatrix computes the motion with its
own controls and chooses the point to turn about itself; View > Lock Rotatrix Pivot (l) asks it to keep turning about
the current one, and again frees it.  In the turntable mode OpenVSP keeps Z upright as Rotatrix turns the view.

### SpaceMouse

When OpenVSP is built with libspnavdev, it reads a 3Dconnexion SpaceMouse, or another six-axis device of theirs,
straight from USB; no 3Dconnexion driver is needed.  OpenVSP finds a device when it is plugged in, and while one is open
a SpaceMouse icon shows at the bottom left of the main window, beside the trackball's; its tooltip gives the status, and
a click opens its preferences.  By default the model moves as the cap does, as in 3Dconnexion's own object mode: push
the cap sideways or lift and press it to pan, pull it toward you to zoom in and push it away to zoom out, tilt it
forward or back to tip the model, tilt it left or right to roll it, and twist it to turn the model about the screen's
vertical axis, all about the rotation center.  The farther the cap moves, the faster the view moves; a small motion
about rest does nothing.  In the turntable mode the model turns about Z and tips about the screen's horizontal axis, and
the axis that rolls it does nothing.  Only the program with the keyboard focus follows the device, although other
programs can read it at the same time.

What each motion of the cap does is set on the SpaceMouse tab of the Preferences; see [SpaceMouse
preferences](#spacemouse-preferences).  A cap is hard to move along one axis alone: a twist often pushes it forward or
sideways too.  If the view pans or zooms when it should only turn, set Acting Axes to Only the axis pushed furthest.
With no 3Dconnexion driver, macOS also moves the cursor as the cap is pushed and scrolls as it is twisted, as a mouse
would.  While the cap is off center and OpenVSP has the focus, the cursor stays where it was, and the mouse cannot move
it until the cap is back at rest; the Cursor setting can let it follow the cap instead.  The 3D view also ignores the
wheel then, so a twist does not zoom as well; other windows may still scroll.

The device's buttons act through the bindings on the Mouse Buttons tab, where they can be changed; see [Mouse button
preferences](#mouse-button-preferences).  By default FIT fits the model on screen and MENU, or PANEL on older devices,
opens the SpaceMouse preferences.  On the larger devices, T, F, R, L, B and BK show the top, front, right, left,
bottom and rear views, ISO1 and ISO2 the left and right isometric views, Roll + and Roll - turn the view a quarter
turn clockwise and counterclockwise, Rot turns the cap's turns off and on, Pan/Zoom its pans and zoom, Dom switches
Acting Axes, and + and - make it faster and slower.  If 3Dconnexion's own driver is running, it may keep the device to
itself; quit it to use the device in OpenVSP.  On Linux the device must be readable by the user, which usually takes a
udev rule such as `SUBSYSTEM=="hidraw", ATTRS{idVendor}=="256f", TAG+="uaccess"` (046d for older devices), and
spacenavd must not hold it.

## Picking and selecting

Several tools ask for a point on the model and then wait for a click: Set Rotation Center (r), View Normal To (n),
the rulers, probes and protractors of the Measure window, routing points, and the points of a Fit Model target.
While one is waiting, a left press goes to it first, unless a binding listed before the one that gives presses to tool
picks takes it: in OpenVSP's own scheme, a left press with Ctrl, Cmd or Alt, or with z held, still zooms, pans or
rolls.

There are two ways to select a Geom by clicking on it:

- Pick mode, on the Geom Browser's Pick button or Edit > Toggle Pick Mode (p).  While it is on, clicking a Geom makes
  it the only one selected.
- In every scheme but OpenVSP's own, a left click selects the Geom under it without any mode: on its own it makes
  that Geom the only one selected, with Shift it adds it, with Ctrl or Cmd it adds or removes it, and a click on the
  background clears the selection.

Esc in the 3D window, when nothing is being dragged, clears the selection.

### Box selection in Fit Model

While the Fit Model window is open, a left drag with Shift held, or any left drag with its Select Region button on,
draws a rectangle and selects the target points inside it.  This comes before every other use of the left button and
cannot be changed in the preferences.

## Keyboard in the 3D window

The keys in the View and Edit menus act from any OpenVSP window, unless a text box is being typed in.  F1 to F4, and
z held for a roll, act only in the 3D window; elsewhere z is View > Z Up.

| Key | Does |
|---|---|
| t | Turntable rotation |
| b | Ball rotation |
| z (tap) | Turn the view until Z points up |
| z (held) | With the left button in OpenVSP's scheme, roll |
| w | Arm a zoom box |
| r | Set the rotation center at the next click |
| n | View normal to the surface at the next click |
| x | View the front of the selected Geom |
| c, C | Center on the selected Geoms, or on all |
| f, F | Fit the selected Geoms, or all, on screen |
| R | Reset all the views |
| F1 to F4 | Recall a saved view; with Shift, save one |
| F5 to F12 | Top, front, left, left isometric, bottom, rear, right, right isometric |
| p | Toggle Pick mode |
| d | Rotatrix trackball on or off (builds with Rotatrix only) |
| l | Lock or free the Rotatrix pivot (builds with Rotatrix only) |
| Esc | Cancel a drag or an armed zoom box; otherwise clear the selection |

## 3D View preferences

File > Preferences opens the preferences.  Changes take effect at once, so they can be tried with the window still
open.  Revert puts back what was set when the window opened, Defaults sets OpenVSP's own, and Close keeps the changes.

The 3D View tab:

| Section | Setting | What it does |
|---|---|---|
| | Scheme | Sets the mouse buttons and the settings below to those of a program; see [Schemes](#schemes).  Changing any setting afterwards shows Custom. |
| Rotation | Start With | Whether OpenVSP starts with the ball or the turntable.  The View menu switches between them at any time. |
| | Rotate About | The rotation center, or the surface point under each drag's press. |
| Ball | Type | Rounded Arcball, OpenVSP Classic, Trackball or Tumbler. |
| | Speed | How fast the ball turns, as a multiple from 0.1 to 2.  At the default, 0.5, OpenVSP Classic turns as OpenVSP did through 3.55, the Trackball turns as it does near the middle of the view, and a drag across the whole Rounded Arcball turns the view 180 degrees. |
| | Size | The Rounded Arcball's sphere, as a fraction of the view.  A drag that starts outside it rolls the view. |
| | Outline | Whether the Rounded Arcball's outline is drawn while it turns. |
| Turntable and Tumbler | Speed | Degrees turned per point of mouse movement. |
| Zoom | Wheel Direction | Whether rolling the wheel forward zooms in or out. |
| | Wheel Zooms | About the point under the pointer, or the middle of the view. |
| | Wheel Step | A fixed step per notch, or a step that grows with the amount scrolled. |
| | Drag Direction | Whether a zoom drag downward zooms in or out. |
| | Drag Zooms | About the middle of the view, or the point where the drag started. |
| | Drag Step | Classic, per event: a step for each mouse movement however small; or By distance: a zoom that grows with the distance moved. |
| | Zoom Speed | How fast the wheel by scroll amount and the drag by distance zoom. |
| 3D Device | Rotatrix | Whether to use the Rotatrix trackball when its host is running. |
| | Status | Whether OpenVSP is connected to the Rotatrix host. |

The Ball's Size and Outline are greyed out unless the Rounded Arcball is chosen, its Speed when the Tumbler is, which
turns at the Turntable and Tumbler Speed, and Zoom Speed unless Wheel Step is By scroll amount or Drag Step is By
distance.  The Rotatrix settings appear only in builds with the Rotatrix SDK.

## Mouse button preferences

The Mouse Buttons tab shows what each mouse press does, and lets any of it be changed.

At the top are three settings for all the bindings:

- Drag Starts: how far the mouse must move before a press counts as a drag.  Until then it may still be a click, so one
  button can drag and click to different effect, as a middle drag rotating and a middle click setting the rotation
  center.  OpenVSP's own scheme starts every drag at once.
- Release One of Two: with two buttons held together, as left and right zoom in OpenVSP's scheme, letting go of one
  either goes on with what the other button does, as OpenVSP always has, or stops, as CAD programs do.
- Alt + Left: Alt with the left button can act as the middle button, for a mouse or trackpad without one.  It applies
  only to presses no binding uses as they are.

Below them is the list of bindings, tried from the top: the first that fits a press acts.  A binding shown in red can
never act, because those above it take every press it would.  Add puts a copy of the selected binding above it, Delete
removes it, and Up and Down move it.  The selected binding's fields are:

| Field | What it means |
|---|---|
| Action | Rotate, Roll, Pan, Zoom, Zoom in, Zoom out, Zoom box, Rotation center, Select Geom, a command, or Tool picks only, which gives the press to a waiting tool and does nothing else. |
| On | Whether the binding acts on a drag or on a click. |
| Button | The button that must be down: Left, Middle, Right, a mouse's Back or Forward side button, or, while a SpaceMouse is connected, one of its buttons. |
| With | A second button that must be down with it, for a chord, or None. |
| Shift, Ctrl, Alt, Cmd | Whether each key must be up, down, or may be either.  Cmd is named Win on Windows and Super on Linux. |
| Other Buttons | Whether buttons the binding does not need may also be down. |
| Held Key | A letter or digit that must be held, such as z for roll. |
| Where | Anywhere, only on the model, or only on the background. |
| Tool Picks | Whether a waiting tool pick takes the press first. |

Zoom in and Zoom out zoom one wheel notch, about the pointer or the middle of the view as Wheel Zooms is set.  They act
at the press and, after a short pause, ten times a second while the button is held, however the mouse moves; On does
not apply to them.  A mouse's Back and Forward buttons bound to Zoom out and Zoom in zoom the view without the wheel.

The commands act once, at the press: Fit, SpaceMouse settings, the top, front, left, right, bottom, rear and left and
right isometric views, Roll 90 clockwise and counterclockwise, Z up, and five that change the SpaceMouse's own
settings: its turns or its pans and zoom on or off, its Acting Axes, and faster or slower.  A mouse's side buttons can
be bound to them too.

A SpaceMouse's buttons are bound in the same list, after the mouse's.  Their rows are shown, and the device's buttons
offered under Button, only while one is connected.  A device button's binding acts at its press, wherever the pointer
is, with the keys held then; it can take Zoom in, Zoom out, a command or None, and its Zoom in and Zoom out zoom about
the middle of the view.  The schemes keep the device's bindings as they are.

Rather than setting the fields by hand, press in the Try a Press box with the buttons and keys wanted: it shows what
that press does now, and Use This Press for the Selected Binding copies it into the selected binding.

## SpaceMouse preferences

The SpaceMouse tab appears in builds with libspnavdev.

| Section | Setting | What it does |
|---|---|---|
| Device | SpaceMouse | Whether to use a SpaceMouse when one is plugged in. |
| | Cursor | macOS only: whether the cursor is held still while the cap moves the view, or moves with the cap. |
| | Status | Whether a SpaceMouse is open, and its name. |
| Motion | Speed | How fast the SpaceMouse turns, pans and zooms the view, from Slowest to Fastest; each step doubles it. |
| | Turns | Off: the axes that turn the model do nothing. |
| | Pan and Zoom | Off: the axes that pan and zoom do nothing. |
| | Acting Axes | Every axis, or only the axis pushed furthest, which 3Dconnexion calls Dominant. |

Under Axes is a row for each of the cap's six motions: left and right, forward and back, up and down, tilting forward
and back, tilting left and right, and twisting.  Each row sets what that motion does: pan right, pan up, zoom in, turn
the model's top toward you, turn its right side away, roll it counterclockwise, or nothing; and whether it acts the
other way.  The bar at the end of each row shows how far the cap is pushed along that axis, so moving the cap shows
which row it drives; inside the shaded middle an axis does nothing.  Default Axes puts every row back as OpenVSP starts.
To zoom by lifting and pressing the cap rather than pulling and pushing it, as FreeCAD does, swap what the forward and
back and the up and down rows do.

## Schemes

A scheme sets the mouse buttons, and the rotation and zoom settings, to those of another program, so a user of that
program finds the view moving as expected.  None is an exact copy of its program.  The Scheme menu lists each with
other programs that work the same way, and its tooltip lists what the buttons do.

In every scheme but OpenVSP's own, a left click selects the Geom under it, a left press goes first to a waiting tool
pick, a press becomes a drag after the mouse moves 4 points, letting go of one of two buttons stops, the wheel and
zoom drags go by distance, and on macOS Cmd works wherever Ctrl does.

| Scheme | Rotate | Pan | Zoom | Also |
|---|---|---|---|---|
| OpenVSP (also MATLAB) | Left | Alt + left, or right | Middle, Ctrl + left, or left + right | Roll: z + left |
| SolidWorks (also ANSYS Mechanical, DesignModeler, Femap) | Middle | Ctrl + middle | Shift + middle | Roll: Alt + middle; rotation center: middle click; wheel reversed |
| Creo, Blender (also Solid Edge, SketchUp, SpaceClaim, Patran) | Middle | Shift + middle | Ctrl + middle | Rotation center: Alt + middle click |
| NX (also FreeCAD) | Middle | Middle + right, or Shift + middle | Left + middle, or Ctrl + middle | |
| CATIA (also 3DEXPERIENCE CATIA) | Middle + left or right, or Alt + left + right | Middle, or Alt + right | Ctrl + middle, or Ctrl + Alt + right | Rotation center: middle click |
| Inventor, Fusion | Shift + middle | Middle | Shift + Ctrl + middle | Wheel reversed; Rounded Arcball |
| 3ds Max | Alt + middle | Middle | Ctrl + Alt + middle | Rounded Arcball |
| Maya (also Blender's three-button emulation) | Alt + left | Alt + middle, or Shift + Alt + left | Alt + right, or Ctrl + Alt + left | Starts with the turntable |
| Abaqus | Ctrl + Alt + left | Ctrl + Alt + middle | Ctrl + Alt + right | Rounded Arcball |
| Rhino | Right | Shift + right | Ctrl + right | Starts with the turntable |
| Onshape | Right | Ctrl + right, or middle | | Turns about the point under the cursor |
| Pointwise | Ctrl + right | Shift + right | Middle | Roll: Ctrl + middle; zoom box: Shift + middle; rotation center: Shift + Ctrl + right click; Tumbler |
| HyperMesh (before 2021, or its Legacy setting) | Ctrl + left | Ctrl + right | | Zoom box: Ctrl + middle; rotation center: Ctrl + left click |
| HyperWorks (HyperMesh 2021 and later) | Middle | Right, or left + right | | Zoom box: Alt + middle; rotation center: Ctrl + middle click; turns about the point under the cursor; wheel reversed |
| ParaView | Left, or Shift or Ctrl + middle | Middle, or Shift + right | Right, or Ctrl + right | Roll: Shift + left; wheel zooms about the middle of the view; Tumbler |
| EnSight | Left | Middle, or Ctrl + right | Right, or Ctrl + middle | Roll: Ctrl + left; Tumbler |
| STAR-CCM+ | Left | Right | Middle | Roll: Ctrl + left; Tumbler |
| Tecplot | Ctrl + right | Right | Middle | |

The schemes leave the Rotatrix and SpaceMouse settings, and the SpaceMouse's button bindings, alone.  Changing
anything after choosing a scheme makes it Custom; choosing the scheme again puts all of its settings back.

## Saving and loading preferences

Save... at the bottom of the Preferences writes every preference to a file (*.vspprefs), and Load... reads one back
and puts it into use at once; Revert undoes a load.  The file is the same text on Windows, macOS and Linux, so it can
be loaded on another computer.  It includes the file chooser's favorite directories.  Any preference that OpenVSP
reads only as it starts takes effect the next time it starts.

## Parm controls

Each slider or value box has a button with the Parm's name to its left.

- Clicking the name button opens the Parm window: its description and limits, its attributes, and the links and
  advanced links it takes part in.  The name is drawn in dark magenta when the Parm has attributes.
- Dragging the name button picks the Parm up, to drop where a Parm is wanted; see
  [Dragging and dropping Parms](#dragging-and-dropping-parms).
- A value box takes arithmetic: typing `180/6` sets 30.  It understands `+ - * / ^`, brackets and exponents, but no
  functions or names.  A value applies on Enter or when the box loses the focus.
- Alt + drag, or a right-drag, on a value box scrubs its value like a slider; see [Value boxes](#value-boxes).
- The small buttons at either end of a slider change its range.  When the value is inside the range, the inward arrows
  halve the range about the value.  When the value is at an end, the outward arrow doubles the range, extending it past
  that end up to the Parm's own limit, and moves the value to the new end.  A bar means the Parm's own limit has been
  reached.  Typing a value outside the slider's range recenters the range on it.
- Clicking on a slider's track jumps the value there, and the arrow keys nudge a slider that has the focus by one
  hundredth of its range.
- Holding Alt while dragging a slider of a Geom's placement turns on Snap To's collision detection for that change, so
  the part stops at the clearance set in Model > Snap To.

### Value boxes

A value box shows a parameter's value as text; click in it to edit the text.  A value too long for its box shows its
leading digits and sign; while you edit it, the box scrolls to follow the cursor.

Alt (Option on macOS) + drag, or a right-drag, scrubs the value instead, like a slider: drag right to increase it and
left to decrease it.  Only sideways motion counts, and the scrub starts after a few points of it; a press released
sooner is an ordinary click.  Alt is needed only at the start of the drag.  The pointer becomes a left-right arrow over
a box while Alt is held, and while scrubbing.

The value box of a slider row scrubs at the slider's scale: dragging across the box as far as the slider is wide moves
the value through the slider's range, and the slider's range buttons change the rate too.  A scrub is not held to the
slider's range: past its end the slider recenters, and the value runs on to the Parm's own limit.  Beside a logarithmic
slider, equal motion multiplies the value by equal factors.  A value box without a slider scrubs at a fixed range of
its own.  A fraction shown beside its dimensional value steps with that value, so both boxes move the part alike.

While dragging, tap Shift to halve the sensitivity, Ctrl or Cmd (the Windows or Super key elsewhere) to double it, or
Alt (Option on macOS) to reset it.  Each box keeps its sensitivity for its next drag, while the box exists; a fraction
and its dimensional value share one.  Esc puts the value back as it was at the start of the drag, leaving nothing to
undo, and the rest of that drag does nothing.  Undo takes back a whole drag at once.  A right click without a drag
still opens the Cut / Copy / Paste menu.  Some Linux desktops take Alt + drag for moving windows; use a right-drag
there.  Unlike Alt + drag on a slider, scrubbing a value box does not apply Snap To.

Scrubbing works in the value boxes of Parms.  It does not work in index selectors, counters, table cells such as the
Parasite Drag table's, or name boxes, nor in a box whose value cannot be stepped, such as a dimensional value whose
reference is zero.

## Dragging and dropping Parms

Dragging a Parm's name button and dropping it is the quickest way to tell a window which Parm is meant.  The drop
can land anywhere in the area named below, not only on its Parm picker.

| Window | Drop on | Does |
|---|---|---|
| Model > Adv Link | the Input or Output area | Adds the Parm as an input or output variable; see [Advanced Parameter Linking](AdvLink.md). |
| Model > Link | the Parm A or Parm B area | Sets that end of the link. |
| Model > Design Variables | the Pick tab | Adds the Parm as a design variable. |
| Model > Fit Model | the Pick Vars or Var Tree tab | Adds the Parm as a fit variable. |
| Model > Variable Presets | the Group tab | Adds the Parm to the current group. |
| Model > Snap To | the window | Sets the Parm to move for collision detection. |
| Model > Attribute Explorer | the Parm reference field | Sets a Parm reference attribute. |

A dropped Parm is its ID as text, so it can also be dropped into any text box, such as an advanced link's code or a
script, to paste the ID.  On Linux it is also left for a middle-click paste.

The edit curve and blade curve plots take a drag of their points, and the Wave Drag window's area plot takes a drag
of its slicing line.

## Plots

The Results Viewer's plot, the VSPAERO Results Manager's plots, the Wave Drag area plot, the Propeller blade curve
plot and the Edit Curve view take the mouse alike, from the same bindings and settings as the 3D window.  A press does
in a plot what it does in the 3D window, in two dimensions: what rotates the view draws a box to zoom to, what pans or
zooms the view pans or zooms the plot, and Zoom in and Zoom out step the zoom once per press.  A press that would go
first to a waiting tool pick goes first to the point under it, or to Wave Drag's slice line, and moves it.  The wheel
and zoom drags follow the 3D View preferences' Wheel Direction, Wheel Zooms, Wheel Step, Drag Direction, Drag Zooms and
Zoom Speed, but a zoom drag always grows with the distance moved.  A double click returns to the whole plot, and
hovering over a results plot reads out the nearest point.

| Press, in OpenVSP's own scheme                          | In a plot                                                  |
|:--------------------------------------------------------|:-----------------------------------------------------------|
| Left drag, or Shift + left drag                         | Zoom to the box drawn                                      |
| Left drag starting on a point                           | Move the point                                             |
| Wheel                                                   | Zoom about the pointer; scrolling up zooms in              |
| Middle drag, or Ctrl or Cmd + left drag                 | Zoom about the middle of the plot; dragging down zooms in  |
| Right drag, or Alt + left drag                          | Pan                                                        |
| Double click                                            | Return to the whole plot                                   |

## Lists and trees

- Renaming: in the lists of names (Geoms, sets, links, advanced links, attributes, modes, measures, CFD sources, FEA
  parts and properties, variable presets, user Parms and more), a double-click on a name, or Enter or F2 with it
  selected, edits it in place.  Enter or a click elsewhere keeps the new name.  In the FEA structure's part list a
  double-click opens the part instead, and Enter or F2 renames it.
- Advanced Link: a double-click on an input or output variable copies its name to the clipboard, ready to paste into
  the code.
- Attributes: a double-click on an attribute in an attribute editor, on any tab, opens it in the Attribute Explorer.
- Run lists: in the Runs tabs of the VSPAERO Results Manager and Wave Drag, a click on a check box, or Space on the
  selected line, checks or unchecks a run.
- Column widths: in lists with column headings, dragging the line between two columns sets their width, and Ctrl+C
  (Cmd+C on macOS) copies the selected lines as text.
- Selecting several: Shift-click selects a range and Ctrl-click adds or removes one; dragging extends the selection.
  In the Geom Browser Ctrl is Ctrl even on macOS, and Alt-click also selects every Geom below the one clicked.  Where a
  list takes several, its Delete button deletes all the selected lines, and the controls that edit the selected item
  work only while one line is selected.  The list scrolls to keep a single selected line in view.
- Geom Browser: clicking the selected Geom again opens its window if it was closed.
- Geom Browser icons: the eye shows or hides, and the surface icon steps the draw type through wire, hidden, shaded,
  textured and none.  Both act on the whole selection when the clicked Geom is in it, and on the Geoms below a
  collapsed one; clicking the Vehicle's acts on every Geom.  Names in italic are made automatically and cannot be
  renamed.
- Trees answer the arrow keys: up and down move, Shift extends, left and right collapse and expand, and Space selects.
- Tables of values, such as the Attribute Explorer's: typing starts editing a cell, a double-click edits it, and
  Ctrl+C and Ctrl+V (Cmd on macOS) copy and paste a block as text separated by tabs.

## Keyboard shortcuts

Ctrl is Cmd on macOS for the menu shortcuts; Cut, Copy and Paste answer Ctrl there as well.  The menu shortcuts work
from every window, but a key without Ctrl, Cmd or Alt types into a text box that is being typed in.

| Key | Does |
|---|---|
| Ctrl+S | File > Save |
| Ctrl+Z | Edit > Undo Parameter Change |
| Ctrl+X, Ctrl+C, Ctrl+V | Edit > Cut, Copy and Paste the selected Geoms |
| Ctrl+A | Edit > Select All |
| p | Edit > Toggle Pick Mode |

The View menu's keys are listed under [Keyboard in the 3D window](#keyboard-in-the-3d-window).

Other windows have keys of their own:

- Attribute Explorer: Ctrl+X, Ctrl+C, Ctrl+V and Delete, as its buttons show.
- Attribute text and Vehicle Notes: Enter keeps the text; Shift+Enter or Ctrl+Enter starts a new line.
- Esc in any window, when nothing there uses it, clears the selection.
