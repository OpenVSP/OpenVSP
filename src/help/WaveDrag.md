---
title:  'Wave Drag'
---

The Wave Drag tool estimates the zero-lift supersonic wave drag of a model by the supersonic area rule.  It cuts
the model with planes inclined at the Mach angle, measures the area of each cut, fits a smooth area curve through
those areas, and computes the wave drag of that curve.  This is repeated with the planes rolled to several angles
about the X axis, and the average is divided by a reference area to give the wave drag coefficient.  The plot shows
the area distribution for one roll angle at a time, for the whole model, for each component, or as a buildup, and
can compare several runs.

| Topic                                                               | Where              |
|:--------------------------------------------------------------------|:-------------------|
| [Quick Start](#quick-start)                                         |                    |
| [How the Wave Drag Is Computed](#how-the-wave-drag-is-computed)     |                    |
| [Setup](#setup)                                                     | Setup tab          |
| [Inflow and Outflow Faces](#inflow-and-outflow-faces)               | Inflow/Outflow tab |
| [Plot Controls](#plot-controls)                                     | Plot tab           |
| [Comparing Runs](#comparing-runs)                                   | Runs tab           |
| [Running and the Model Afterward](#running-and-the-model-afterward) | Execute area       |
| [Using the Plot](#using-the-plot)                                   | Plot area          |
| [Choosing Settings](#choosing-settings)                             |                    |
| [Scripting](#scripting)                                             |                    |

## Quick Start

1. On the Setup tab, choose the Geometry Set to analyze and set the Mach number.
2. Under Reference Area, choose From Model and pick the reference wing, or choose Manual and enter Sref.
3. Leave Run with X-Z Symmetry on if the set is symmetric about the X-Z plane.
4. If the model has engine inlets or exhausts, mark their faces on the Inflow/Outflow tab.
5. Press START.  The wave drag coefficient appears as CD0_w, and the plot shows the area distribution.
6. Run again with more slices and rotations to check that CD0_w has settled (see
   [Choosing Settings](#choosing-settings)).

## How the Wave Drag Is Computed

### Mach planes and rotation angles

The supersonic area rule replaces the model, for each direction around it, by an equivalent body of revolution whose
area distribution is the area the model presents to Mach planes facing that direction.  A Mach plane is inclined to
the free stream (the +X direction) at the Mach angle

$$
\mu = \arcsin\left(\frac{1}{M}\right)
$$

and is rolled about the X axis by the rotation angle $\theta$.  With $\beta = \sqrt{M^2 - 1}$, the slicing plane at
axis location $X$ is

$$
x - \beta\,\left( y \sin\theta + z \cos\theta \right) = X
$$

where $y$ and $z$ are measured from the center of the model's bounding box.  $X$ is where the plane crosses the line
parallel to the X axis through that center; it is the horizontal axis of the plot.  At $\theta = 0$ the plane leans
aft going up (+Z), so each cut collects the disturbances that reach an observer far above the model at one
instant; at $\theta = 90^\circ$ it leans aft toward +Y.  At Mach 1 the planes are normal to the X axis and every
rotation angle gives the same cuts.

### Slice areas

For each rotation angle, OpenVSP finds the first and last planes that touch the model and spaces Num Slices planes
evenly between them, both ends included.  The area of each cut is measured projected onto the Y-Z plane, which is
the area the area rule uses.  The cut honors the trimmed outer surface of the set: area inside two components counts
once, and area inside a negative component does not count.  Each part of a cut is credited to the component it lies
inside, which gives the per-component areas; where components overlap, it goes to the one with the highest Priority on
its Mass tab, and on a tie to the one earlier in the component order.

### The fitted area curve and the drag

The slice areas are joined by the Eminton-Lord curve: of all area distributions that pass through the slice areas
and start and end at the end areas, it is the one with least wave drag.  Its wave drag, $D/q$, follows in closed form
from the slice areas and their locations, and is the drag OpenVSP reports for that rotation angle.  A body whose base
area differs from its nose area adds a term for the difference; it is zero for a closed body.  The "EmintonLord"
analysis computes the same $D/q$ for any area distribution (see [Scripting](#scripting)).

The fitted curve is drawn at 100 points.  The location along it where the curve bends most sharply (the largest
magnitude of its second derivative) is kept as the location of largest drag for that rotation angle.

### Reference area and the coefficient

The wave drag coefficient is the mean of the rotation angles' drags divided by the reference area.  With $N$ rotation
angles evenly spaced around the full circle, $\theta_k = 360^\circ\,k/N$,

$$
C_{D_w} = \frac{1}{S_{ref}}\,\frac{1}{N} \sum_{k=0}^{N-1} \frac{D}{q}(\theta_k)
$$

The reference area is either entered (Manual) or taken from a wing (From Model), in which case it follows that
wing's total area.

### Symmetry

When Run with X-Z Symmetry is on, the $N$ rotation angles span only half the circle, from the top to the bottom
through the +Y side, both ends included: $\theta_k = 180^\circ\,k/(N - 1)$.  The other half is taken to mirror it,
and the mean is a trapezoidal one:

$$
C_{D_w} = \frac{1}{S_{ref}}\,\frac{1}{N - 1}\left[ \frac{1}{2}\,\frac{D}{q}(\theta_0) +
  \sum_{k=1}^{N-2} \frac{D}{q}(\theta_k) + \frac{1}{2}\,\frac{D}{q}(\theta_{N-1}) \right]
$$

The whole model is still sliced; symmetry changes only which rotation angles are used.  For the same number of
rotations it takes the same time and resolves the half circle twice as finely.

### Inflow and outflow

An engine nacelle or inlet duct is open: air passes through it.  Its faces are closed surfaces in the model, and
slicing them as solid would count the whole nacelle cross section, including the air that flows through, as body
area.  Marking a face as flow-through tells OpenVSP to treat the captured flow instead as a stream tube.

Each marked face is a sub-surface of a component.  A face whose surface points forward is an inlet; one whose surface
points aft is an outlet.  OpenVSP moves an inlet face straight forward to well ahead of the model, and an outlet face
straight aft to well behind it, and joins each to its component with straight sides parallel to the X axis.  The
stream tubes then belong to the body.  Their areas, the inlet area $S_{in}$ and exit area $S_{out}$, are measured
across the tubes ahead of and behind the model, and the drag is computed from the area less a linear variation
between them:

$$
S_{flow}(\xi) = S(\xi) - S_{in}\,(1 - \xi) - S_{out}\,\xi
$$

A face whose surface points both forward and aft (part of it each way) is ambiguous.  OpenVSP reports "Ambiguous
flow-through subsurface detected" and does not treat that face as flow-through.

## Setup

The Setup tab holds the case.

- Geometry Set or Mode chooses what is sliced.  A Mode applies its settings to the model first, then slices the
  Mode's set.  The Mode choice is available only when the model has Modes.
- Num Slices (3 to 100, default 20) is the number of slicing planes at each rotation angle.
- Num Rotations (3 to 30, default 10) is the number of rotation angles.
- Run with X-Z Symmetry (default on) spans the rotation angles over half the circle; see [Symmetry](#symmetry).
- Mach (1 to 5, default 1.5) sets the Mach angle of the planes.
- Reference Area: Manual uses the Sref entered.  From Model uses the Ref. Wing chosen from the model's wings and shows
  its total area in Sref.
- Output File names the text file written by each run.  The Mach number and the set name are added to the name, for
  example `model_WaveDrag_M1.500000_Shown.txt`, so runs at other conditions do not overwrite each other.  The file
  lists the inlet and exit areas, each rotation angle's slice range and slice areas, CDWave, the Mach number and the
  set.

## Inflow and Outflow Faces

The Inflow/Outflow tab lists every sub-surface in the model as "component: sub-surface".  Check a sub-surface to treat
it as a flow-through face; see [Inflow and outflow](#inflow-and-outflow).  Only faces on components in the set being
sliced have an effect.

Make each face a sub-surface that covers exactly the opening and faces one way, such as the flat front or rear cap of
a nacelle.  A face that wraps around onto the side of the component is ambiguous.

## Plot Controls

The Plot tab chooses what the area plot shows.

### Displayed rotation

The index selector steps through the rotation angles of the most recent run; the angle in degrees is shown beside
it.  The vertical axis is scaled to the largest area at any angle, so it stays still while stepping through angles and
the areas can be compared by eye.

### Slice reference

The vertical line on the plot marks the axis location X Loc.  Drag the line, or set X Loc, to move it.  With Plane
on, the slicing plane at X Loc and the displayed rotation angle is drawn in the 3D view, which shows which part of
the model a point on the plot comes from.

- X moves X Loc to the location of largest drag at the displayed rotation angle.
- X, Rot first displays the rotation angle with the largest drag, then moves X Loc to its location of largest drag.

X Loc, the plane and these buttons belong to the most recent run.

### Optimal distribution

The Select choice draws an ideal body in blue, or None, against the newest plotted run, sized from that run's equivalent
body at the displayed angle, with length $L$, volume $V$ (the integral of the fitted area less inflow and outflow) and
largest area $S_{max}$:

- Sears-Haack Body (Len-Vol), the least-drag closed body of given length and volume,
  $S(\xi) = \frac{16\,V}{3\,\pi\,L}\left[ 4\,\xi\,(1 - \xi) \right]^{3/2}$.
- Von Karman Ogive (Len-Diam), the least-drag nose of given length and base area, here $S_{max}$, reached at the
  end: $S(\xi) = \frac{2\,S_{max}}{\pi}\left[ \arcsin\sqrt{\xi} - (1 - 2\xi)\sqrt{\xi\,(1 - \xi)} \right]$.
- Lighthill Body (Len-Diam), the least-drag body of given length and largest area,
  $S(\xi) = S_{max}\left[ 2\sqrt{\xi\,(1 - \xi)} - (1 - 2\xi)^2 \operatorname{arccosh}\frac{1}{|1 - 2\xi|}
  \right]$.

The gap between the model's curve and the ideal body shows where area could be moved to reduce drag.  The ideal
body is drawn against the area less inflow and outflow in the Total style, and has the inflow and outflow ramp added
back in the Parts and Buildup styles, so it matches the curves it is drawn with.

### Plot style

- Total plots the fitted area less inflow and outflow, the curve whose drag is computed.
- Parts plots the fitted area of each component, with the total in black.  These areas include any stream tubes.
- Buildup plots, for each component, the area of that component and every component after it in the model's
  component order.  The first curve is the whole model, and the gap between one curve and the next is that
  component's area.  Each curve is fitted on its own.
- Points adds markers at the slice areas, which shows how well the slices resolve the curve.

The legend, in a corner of the plot, lists each curve with its color, dash and marker.  Legend places it as in the
[Results Viewer](ResultsViewer.md#style-and-legend): Best, the default, puts it in the corner that covers the fewest
points and lines and moves it as the view changes; Upper Right, Upper Left, Lower Left and Lower Right fix the corner;
Off hides it.  The choice is saved with the model.

## Comparing Runs

The Runs tab lists every Wave Drag run in the session, numbered in order, with its name, Mach number, set and the time
it was made.  Check runs to plot them together.  A new run is plotted alone; check earlier runs to add them.  Click a
box, or select a row and press Space, to check or uncheck it.  Double click a run, or select it and press Enter, to give
it a name, which then labels it in the legend.  Runs are deleted in the [Results Viewer](ResultsViewer.md) (Analysis >
Results...), where each is a WaveDrag Result listed with its name.

Each plotted run is drawn from its stored results in the chosen plot style, at the displayed rotation angle.  A run
that does not have that angle (it was made with another number of rotations, or with symmetry set the other way) is
drawn at its nearest angle, and its legend entry gives that angle.

With several runs plotted, each run has its own dash and marker.  In the Total style each run also has its own
color.  In the Parts and Buildup styles a component keeps one color in every run: components are matched by the
component itself, then by name, so a component that was changed between runs, or one of the same name in another
model, is drawn in the same color.  In the Parts style the totals stay black, dashed by run.

Use this to compare design changes: run, name the run, change the model, run again, and check both.  The Buildup or
Parts style then shows which component's area changed.

## Running and the Model Afterward

START runs the analysis; CD0_w then shows the wave drag coefficient of that run.

A run leaves a Mesh component of the sliced geometry and its slices in the model, named in the results.  When the
Shown set is sliced, the components stay shown along with the Mesh; on any other set only the Mesh is shown
afterward.  The next Wave Drag run deletes the previous Mesh before it reads the set.  Other analyses do not: delete
the Mesh, or hide it, before running another analysis on the Shown set, or it will be counted with the components it
was made from.

## Using the Plot

The mouse works as in the other plots.

| Press, in OpenVSP's own scheme                          | In a plot                                                  |
|:--------------------------------------------------------|:-----------------------------------------------------------|
| Left drag, or Shift + left drag                         | Zoom to the box drawn                                      |
| Left drag starting on the slice line                    | Move the slice line (X Loc)                                |
| Wheel                                                   | Zoom about the pointer; scrolling up zooms in              |
| Middle drag, or Ctrl or Cmd + left drag                 | Zoom about the middle of the plot; dragging down zooms in  |
| Right drag, or Alt + left drag                          | Pan                                                        |
| Double click                                            | Restore the automatic limits                                 |
| Hover                                                   | Read out the nearest point, or the cursor                  |

These are the presses of OpenVSP's own scheme; the plot follows the scheme and 3D View settings chosen in the
preferences, as [Mouse and Keyboard](MouseKeyboard.md#plots) describes.

The readout names the curve and gives the point's index and coordinates when the cursor is within a few pixels of a
curve, and otherwise gives the cursor's coordinates.  Zoomed or panned limits stay, through new runs and changes of
angle or style, until a double click restores the automatic ones.

The window can be resized.  The plot takes all the added width and height; the tabs and the Execute area keep their
size.

## Choosing Settings

Run time grows in proportion to Num Slices times Num Rotations.  The wave drag depends on the curvature of the area
distribution, and a coarse set of slices misses the sharp area changes at wing and tail leading and trailing edges,
so CD0_w often rises as slices are added.  Raise Num Slices until CD0_w stops changing, then raise Num Rotations the
same way; for a model with wings expect to need more than the defaults.  Compare designs at the same settings.

Use Run with X-Z Symmetry for any model that is symmetric about the X-Z plane; it doubles the angular resolution at
no cost.  Turn it off for an asymmetric model, such as one with stores on one side or an oblique wing.

To reduce wave drag, look for where the drag comes from before changing the shape.  X, Rot finds the rotation angle with
the most drag and the location where its area curve bends most sharply, and the Plane shows the cut there in the 3D
view.  The Buildup style then shows which components make up the area at that location, and the Parts style shows how
each component's own area rises and falls.  Smoothing the total curve toward the Sears-Haack body, by moving area away
from the peak, is the area rule's goal.

## Scripting

The Wave Drag tool runs as the analysis "WaveDrag".  Its inputs default to the screen's settings.

| Input       | Meaning                                                              |
|:------------|:---------------------------------------------------------------------|
| Set         | Geometry Set to slice                                                |
| NumSlices   | Number of slices at each rotation angle                              |
| NumRotSects | Number of rotation angles                                            |
| Mach        | Mach number                                                          |
| SSFlow_vec  | IDs of the flow-through sub-surfaces                                 |
| SymmFlag    | Rotation angles over half the circle, as Run with X-Z Symmetry       |
| UseModeFlag | Slice a Mode's set instead of Set                                    |
| ModeID      | Mode to apply                                                        |

The reference area is the Sref Parm of the WaveDragSettings container.  SetWaveDragRefWingID makes it follow a wing
from the next Update, and an empty ID returns it to Manual.

The results are a "WaveDrag" Result.  Quantities kept for each rotation angle are stored as one vector per angle, read
with the angle's index; per-component quantities are stored one vector per angle and component, at index
itheta * Num_Comps + icomp.

| Result                      | Contents                                                               |
|:----------------------------|:-----------------------------------------------------------------------|
| CDWave                      | Wave drag coefficient                                                  |
| Mach, Set_Name              | Mach number and the name of the set sliced                             |
| Mesh_GeomID                 | ID of the Mesh component left in the model                             |
| Num_Slices, Num_Cone_Sections, Num_Comps | Number of slices, rotation angles and components          |
| Theta                       | Rotation angles, degrees                                               |
| Start_X, End_X              | Axis locations of the first and last slice, for each angle             |
| X_Norm                      | Normalized slice locations                                             |
| X_Norm_Fit                  | Normalized locations of the fitted curve's points                      |
| Slice_Area                  | Slice areas (one vector per angle)                                     |
| Slice_Area_Flow             | Slice areas less inflow and outflow (one vector per angle)             |
| Fit_Area, Fit_Area_Flow     | Fitted areas, with and without inflow and outflow (one vector per angle) |
| Length, Volume, MaxArea     | Equivalent body length, volume and largest area, for each angle        |
| X_Max_Drag                  | Normalized location of largest drag, for each angle                    |
| Inlet_Area, Exit_Area       | Stream tube areas ahead of and behind the model                        |
| Ambiguous_Subsurf_Flag      | 1 if a flow-through face was ambiguous                                 |
| Comp_ID, Comp_Name          | Component IDs and names, in component order                            |
| Comp_Slice_Area, Comp_Fit_Area | Slice and fitted areas of one component (index itheta * Num_Comps + icomp) |
| Buildup_Area, Buildup_Fit_Area | Slice and fitted areas of one component and every one after it (same index) |

A location along the axis is Start_X + X_Norm * (End_X - Start_X) for that angle.  For example, the areas of the
third component at the first rotation angle are

```python
import openvsp as vsp

rid = vsp.ExecAnalysis( "WaveDrag" )
ncomp = vsp.GetIntResults( rid, "Num_Comps" )[0]
itheta = 0
icomp = 2
area = vsp.GetDoubleResults( rid, "Comp_Slice_Area", itheta * ncomp + icomp )
```

The "EmintonLord" analysis computes the drag of any area distribution: given stations X_vec and areas Area_vec (at
least three, the same number of each), it returns Donq, the $D/q$ of that curve.
