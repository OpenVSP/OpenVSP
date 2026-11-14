---
title:  'VSPAERO Results'
---

The VSPAERO Results Manager plots the results of VSPAERO runs: how each case converged, the span load, polars
across a sweep of flow conditions, pressure along slices through the surface, and the time history of unsteady and
rotor runs.  It keeps every run made in the session, so a run can be plotted alone or laid over others to compare a
change in the model or the settings.

| Tab                         | Plots                                                 | Active when                    |
|:----------------------------|:------------------------------------------------------|:-------------------------------|
| [Convergence](#convergence) | A variable, or its change, against wake iteration     | The run is steady              |
| [Load Dist.](#load-dist)    | Span load of each wing, or blade results by station   | The run has load results       |
| [Sweep](#sweep)             | One result against another across flow conditions     | Always                         |
| [Cp Slice](#cp-slice)       | Pressure coefficient along each slice                 | A run or set with slices       |
| [Unsteady](#unsteady)       | Histories, group and rotor results against time       | The run is unsteady            |
| [Runs](#runs)               | Nothing; chooses what the other tabs plot             | Always                         |

## Opening the Screen

The screen opens by itself when a VSPAERO run finishes.  A steady run opens on the Load Dist. tab, an unsteady run
on the Unsteady tab.  Show Results Mgr on the VSPAERO screen opens it again at any time.

Load Previous Results on the VSPAERO screen reads the results of the last run from its files and adds them to the
list as a new run, beside the runs already held.

## Runs

Every VSPAERO run is kept until it is deleted.  Launching a new run, or slicing the latest results with Slice
Latest \*.adb File, adds to the list and leaves earlier runs and slices in place.

The Runs tab lists each VSPAERO run, and each Cp slice set made apart from a run, in the order they were made.  The
columns give the check box, the run's number, its name, its kind (VSPAERO or Cp Slice) and when it was made.  Cp
slices made during a run belong to that run and are not listed separately.

The check boxes choose what the other tabs plot.  Click a box, or select a row and press Space, to check or uncheck
it.  With nothing checked, nothing is plotted.

A new run is checked alone, so each run is shown by itself when it finishes, as it would be with only one run
kept.  This works within each kind: a new VSPAERO run unchecks the other VSPAERO runs, and a new slice set unchecks
the other slice sets, but neither unchecks the other kind.  If every checked run is deleted, the newest run is
checked in its place.

Newest Only checks just the newest run and the newest separate slice set; use it to return to a single run after
comparing.  All checks everything.

Export to \*.csv on the VSPAERO screen writes the newest run, whichever runs are checked here.

### Names

A run is named by its number until it is given a name.  Double click a row, or select it and press Enter, to edit
the name in place; Enter, or a click elsewhere, keeps the new name.  The name is the run's nickname, shown also in
the [Results Viewer](ResultsViewer.md).

The numbers count every row of the list, so deleting a run renumbers the runs after it.  Name the runs before
comparing them: a name says what changed between them, and it does not change when the list does.

### Deleting Runs

Runs are deleted in the Results viewer (Analysis > Results...), where each run is a VSPAERO_Wrapper and each
separate slice set a CpSlice_Wrapper, listed with its nickname.  A deleted run leaves this list.

## Flow Conditions

Each plot tab has a Flow Condition list.  It lists each flow condition (angle of attack a, sideslip b, Mach M and,
where the run has one, Reynolds number Re) once, whichever of the checked runs computed it.  Selecting a condition
plots it from every checked run that has it, so two runs over the same sweep are compared condition by condition.
Several conditions may be selected at once; click a row to select it or clear it.

The list shows each value to two significant digits, but conditions are told apart by their exact values.  Two
conditions that differ only past the second digit (an angle of attack of 12 and 12.4, say) get two rows that read
the same.

When the checked runs change, or a new run arrives, every condition is selected again, and each tab selects its
default variable.

The variables a tab offers are those of the newest checked run.

## Comparing Runs

With one run plotted, each line has its own color.  With several, color shows the flow condition and variable, and
marker and dash show the run: the same condition from two runs is drawn in one color with two markers and dashes.
When only the runs differ (one condition and one variable from each), each run gets its own color as well.

Five markers and five dashes are used, so marker or dash alone tells five runs apart.  Past the last of the ten
colors, colors repeat with the next markers and dashes.

The legend, in a corner of the plot, names each line.  With two or more runs plotted it begins each entry with the
run's name, and on the Cp Slice tab with the name of the run, or of the slice set, the slices came from.  Each tab's
Legend choice, below its Lines and Points toggles, places the legend as in the
[Results Viewer](ResultsViewer.md#style-and-legend): Best, the default, puts it in the corner that covers the fewest
points and lines and moves it as the view changes; Upper Right, Upper Left, Lower Left and Lower Right fix the corner;
Off hides it.  The choice is saved with the model.

Keep the number of checked runs small.  Two or three runs read well; each further run multiplies the lines and the
legend entries, and the markers and dashes become hard to tell apart.

## Convergence

The Convergence tab plots the selected Y-Data against wake iteration for each selected flow condition.  CLtot is
selected by default.

Residual, on by default, plots the base 10 logarithm of the change in the variable from one wake iteration to the
next, rather than the variable itself.  Iterations where the value did not change are left out, as is the first,
which has no change.  A steadily falling residual shows the wake converging; turn Residual off to see the value
the case converged to.

The tab is inactive for unsteady runs, whose histories are plotted on the Unsteady tab.

## Load Dist.

Load plots the spanwise load of each wing against span location Y, one line for each wing, for each selected flow
condition.  The section lift times chord over reference chord, cl\*c/cref, is selected by default.

Blade plots blade-averaged results against blade station for the blades selected in the Group/Rotor Selection list;
CT_H is selected by default.  Blade is available only when the run has rotors.  These results are written for the
first flow condition only, so the Flow Condition list then shows just that one, inactive.

For an unsteady run, the Time Step selector picks which time step's loads are plotted, and Time shows its time.

## Sweep

The Sweep tab plots one result against another across the selected flow conditions: the final value of each
condition is one point, and the conditions of each run make one line, in the order they were computed.  CLtot
against CDtot, the drag polar, is selected by default.

Several variables may be selected in one of the X-Data and Y-Data lists, but then only one in the other.

Select Alpha as X-Data to plot lift or moment against angle of attack.  All the selected conditions of a run are
joined into one line, so a sweep over both angle of attack and Mach draws one zig-zag line through every point.
Select only the conditions at one Mach number to draw that Mach number's curve.

## Cp Slice

The Cp Slice tab plots the pressure coefficient along the selected slices for the selected flow conditions.  Slices
come from the checked runs that were made with Activate Slicer on, and from the checked slice sets.

The Slice list names each cut by its axis and position (Y= 1.50 is a cut at Y = 1.5) once, and the Flow Condition
list each condition once, across all the plotted runs and slice sets.  A line is drawn for every slice that has a
selected position and a selected condition.  With two runs sliced at the same positions, selecting one cut and one
condition plots that cut from both, for a direct comparison of the pressures.

X-Axis Plotting chooses the coordinate each kind of cut is plotted against.  Each choice is active when slices of
that kind are present, and each starts at X.  That suits a cut at constant Y or Z, plotted along the chord; a cut
at constant X, across a fuselage for instance, lies at one X, so set X Cut Position Axis to Y or Z.

Flip Y Axis turns the Cp axis over so negative Cp, suction, is drawn upward, as pressure distributions usually are.

## Unsteady

The Unsteady tab plots time histories of unsteady runs.  Data Type Selection chooses what is plotted:

- History plots the run's overall results against time for each selected flow condition.  For a P, Q or R stability
  run the default is the matching moment coefficient (CMx, CMy or CMz); otherwise CLtot.
- Group plots the results of each group selected in the Group/Rotor Selection list; CLtot by default.
- Rotor plots the results of each rotor selected there; Thrust by default.

Group and Rotor are available only when the run has groups.  Their results are written for the first flow
condition only, so the Flow Condition list then shows just that one, inactive.  A new unsteady run with groups
opens on Rotor.

The tab is active only for unsteady runs.  Whether the plotted runs are steady or unsteady is taken from the oldest
checked run, so check steady and unsteady runs separately.

## Lines, Points and Axis Limits

Each plot tab has its own Lines and Points toggles (Plot Lines and Plot Points on the Cp Slice tab), both on by
default.  Points alone suit a sweep of scattered conditions; lines alone suit long convergence histories, where
markers crowd together.  The legend samples show what is drawn.

Each tab also keeps its own axis limits.  Xmin, Xmax, Ymin and Ymax each have a toggle that makes the limit manual;
while it is off the limit is automatic, set from the data and rounded outward, and its slider shows the value.

## Using the Mouse in a Plot

Each plot zooms, pans and reads out points with the mouse, as the other plots in OpenVSP do.  Zooming or panning sets
all four limits of that tab to manual at the values shown; a double click makes them automatic again.

| Press, in OpenVSP's own scheme                          | In a plot                                                  |
|:--------------------------------------------------------|:-----------------------------------------------------------|
| Left drag, or Shift + left drag                         | Zoom to the box drawn                                      |
| Wheel                                                   | Zoom about the pointer; scrolling up zooms in              |
| Middle drag, or Ctrl or Cmd + left drag                 | Zoom about the middle of the plot; dragging down zooms in  |
| Right drag, or Alt + left drag                          | Pan                                                        |
| Double click                                            | Restore automatic limits                                   |
| Hover                                                   | Read out the point under the pointer                       |

These are the presses of OpenVSP's own scheme; each plot follows the scheme and 3D View settings chosen in the
preferences, as [Mouse and Keyboard](MouseKeyboard.md#plots) describes.

A box smaller than a few pixels across does not zoom, so a click does not change the limits.

When the pointer is within a few pixels of a drawn point, the point is highlighted and the readout in the top
right corner of the plot gives its legend entry, its index in the line (counted from zero) and its x and y.
Elsewhere the readout gives the coordinates of the pointer.  A line hidden by turning both Lines and Points off is
not read out.

## Window Size

The window can be resized.  A wider window widens the plots; a taller one lengthens the plots and the Flow
Condition lists beside them, which helps with long sweeps of conditions, and the list of runs on the Runs tab.  The
other lists and controls keep their size.
