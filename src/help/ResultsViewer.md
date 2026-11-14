---
title:  'Results Viewer'
---

Most analyses in OpenVSP store what they compute as a Result: Geometry Analysis cases, CompGeom, Planar Slice, Mass
Properties, Projected Area, DegenGeom, VSPAERO, Wave Drag and Parasite Drag among them.  The Results viewer is where
those Results can be inspected and compared.  The Data tab shows every value a Result holds; the Plot tab draws
Results data against each other, across several runs at once.  The viewer is opened by *Results...* at the top of the
Analysis menu, or by *Show Results* on the Geometry Analysis screen.

| Topic                                                   | Tab        |
|:--------------------------------------------------------|:-----------|
| [Quick Start](#quick-start)                             | Plot       |
| [Data Tab](#data-tab)                                   | Data       |
| [Plots and Curves](#plots-and-curves)                   | Plot       |
| [Results and Runs](#results-and-runs)                   | Plot       |
| [X and Y Variables](#x-and-y-variables)                 | Plot       |
| [Vectors or Across Results](#vectors-or-across-results) | Plot       |
| [Axes](#axes)                                           | Plot       |
| [Style and Legend](#style-and-legend)                   | Plot       |
| [Mouse in the Plot](#mouse-in-the-plot)                 | Plot       |
| [Export](#export)                                       | Data, Plot |
| [Plots in the Model File](#plots-in-the-model-file)     |            |
| [Plot Files](#plot-files)                               | Plot       |
| [Scripting](#scripting)                                 |            |

## Quick Start

To plot one value against another from an analysis:

1. Run the analysis at least once, so its Results exist.
2. On the Plot tab, press *Add* beside the Plot choice.  The new plot starts with one empty curve.
3. In the Results list, pick the kind of Result to plot.
4. Pick the X variable, and one or more Y variables in the Y list.
5. Set Runs to Newest to follow the latest run, or leave it at All to see every run together.

To compare runs as a design changes, run the analysis again after each change; the plot draws each new run as it is
made.

## Data Tab

The Result choice lists every Result in the order they were made, the newest last, and shows the newest when nothing
else has been picked.  Each entry reads nickname, case name, Results name, time and description; the nickname and case
name lead only when the Result has them.  A case name is the name of the Geometry Analysis case that made the Result,
so two cases of the same analysis type can be told apart.

A Nickname is a note for telling Results apart, such as "Baseline" or "Long tail".  It leads the Result's entry here,
in the Choose Results list, and in plot legends.  Results are not saved with the model, and neither are their
nicknames.

The Data list shows each value's name, type and description.  Picking one shows its contents below the list: a
column for a vector of integers, doubles or strings, X, Y and Z columns for a vector of 3D points, and a table for a
matrix of doubles.

*Delete* removes the Result shown and *Delete All* removes every Result.  Plots that drew them draw what remains.

## Plots and Curves

A plot is a set of axes with one or more curves on it.  The Plot choice above the plot picks which plot is shown and
edited, *Name* renames it, and *Add*, *Copy* and *Delete* make a new plot, copy the current one with all its curves,
or remove it.  A model can hold as many plots as are useful, for example one for lift, one for drag and one for mass.
*Save...* writes the current plot to a plot file of its own, and *Load...* adds the plot a plot file holds, so a plot
set up in one model can be used in another (see [Plot Files](#plot-files)).

A curve says what to plot: which kinds of Result, which of their runs, and which variables.  One curve can draw
several series, one for each Y variable and, in Vectors mode, for each run.  Each line of the Curves list
summarizes a curve as "Y vs X - kinds of Result (runs)".  Picking a line puts that curve in the controls below.
*Add* adds a curve that starts as a copy of the curve being edited, so a variation, such as the same variables from
another kind of Result, takes one or two changes.  *Delete* removes the curve being edited and *Delete All* removes
every curve.

Use more than one curve when series need different settings, such as different runs or different X variables on the
same axes; when they share them, one curve with several Y variables is simpler.

## Results and Runs

The Results list shows each kind of Result that exists, with the number of runs of it.  A kind is a Results name,
such as Mass_Properties or VSPAERO_Polar, together with its Geometry Analysis case name when it has one.  Several
kinds can be picked at once; their variables are then listed together.  A kind the curve plots stays listed, marked
"(none)", while no Result of it exists, as after a model is opened.

Runs picks which runs of each kind the curve plots:

| Runs         | Plots                                                                            |
|:-------------|:---------------------------------------------------------------------------------|
| All          | Every run of each kind                                                           |
| Chosen       | The runs checked on the Choose Results screen                                    |
| Newest       | Only the latest run of each kind                                                 |
| Newest - 1   | The run before the latest; Newest - 2 the one before that, and so on             |

The Newest - n entries go as far back as the curve's most-run kind allows.  A kind with fewer runs than that draws
nothing for that setting.

Newest is the setting to follow a design as it changes: each new run replaces the last on the plot.  All suits a
short series of runs that belong together.  A pair of curves, one at Newest and one at Newest - 1, shows the latest
change against the run before it.  Chosen picks out a few runs to compare from many.

### Choose Results

Picking Chosen, or pressing *Choose...*, opens the Choose Results screen for the curve being edited.  It lists every
Result of the curve's kinds, each by its nickname, case name, name, time and description, with a check box.  Every run
starts checked; unchecking one leaves it out of the plot, and *Check All* and *Check None* act on the whole list.  Only
the unchecked runs are remembered, so a Result made later is plotted until it is unchecked too.  Nicknames make the list
easier to read when many runs look alike.

The choice is kept with the curve while OpenVSP runs, but it is not saved with the model, since the Results it names
are not.

## X and Y Variables

The X choice and the Y list offer every numeric variable in the curve's runs, in the order first met.  The number
after each is its length, or "(none)" for a variable the curve plots that no current Result has.  Variables take
their labels from the Result's data names:

| Data                                     | Labels                                             |
|:-----------------------------------------|:---------------------------------------------------|
| Vector of doubles, integers or booleans  | name                                               |
| Second or later entry with the same name | name#1, name#2, ...                                |
| Vector of 3D points                      | name.x, name.y, name.z, and its magnitude \|name\| |
| Matrix                                   | each column, name[:,j], or each row, name[i,:]     |

Strings cannot be plotted.  The Matrix choice decides whether a matrix offers its columns or its rows.

Each Y variable picked makes its own series.  X is shared by the curve's series.  The first X entry, Index, plots
each value against its place in the vector, counted from 0.  When X and Y have different lengths, the shorter sets
how many points are drawn and a note under the plot says so.

## Vectors or Across Results

The Plot choice sets how a curve turns Results into series.

Vectors plots each run's vectors: one series for each run and each Y variable, with X and Y taken from the same run.
It is the mode for data that is already a curve in each Result, such as a planar slice's area distribution.  A run
that lacks the Y variable makes no series.

Across Results plots one point per run: one series for each Y variable, through all the curve's runs in the order
they were made.  Each point takes one element of each run's vector, the first or the last as Element chooses.  With
the first X entry, Result Order, the points are numbered 0, 1, 2, ... by run; with an X variable, X takes the same
element of that variable.  This is the mode for a value against a design change, such as total mass across a series
of Mass Properties runs, or a quantity against a value that was varied between runs.  Last suits Results whose
vectors build toward a final value, such as a VSPAERO convergence history, whose last entry is the converged
coefficient.  Runs that lack the data are skipped, and a note under the plot counts them.

## Axes

Each axis limit has a check box and a value.  Unchecked, the limit is automatic: it holds all the data, rounded to a
tidy value, and the box shows that value.  Checked, the value typed is used.  Zooming or panning with the mouse
checks all four and sets them to the new view.  *Auto*, or a double click in the plot, makes all four automatic
again.  Limits that cannot be drawn, such as a minimum above the maximum, fall back to the automatic ones.

*Log X* and *Log Y* make an axis logarithmic.  Automatic limits on a log axis are whole decades.  Points at zero or
below cannot be drawn on a log axis and are left out, as are points that are not finite on any axis; a note under the
plot counts them, and lines break where they were.

## Style and Legend

*Lines* draws lines through each series' points and *Points* draws a marker at each point.  A series with only one
point that can be drawn always shows its marker.

Series are styled so that what changes between them is visible.  When only one thing varies, either the runs or the Y
variables, each series takes the next color in turn, and past the tenth color the colors repeat with a new dash; the
markers cycle through five shapes.  When both vary, the color shows the Y variable and the dash and marker show the
run, so every series of one run shares a dash and marker, and every series of one variable shares a color.

The legend names only what tells the series apart: the run when the runs differ, the Y variable when the variables
differ, and the X variable when the curves use different ones; a lone series is named by both its run and its
variable.  A run is named by its nickname and its case name, or its Results name when it has no case name.  Runs
whose names repeat add the time they were made, and runs made in the same second are numbered.  Across Results, a
series is named by its runs' names and the number of runs.  Giving runs nicknames is the simplest way to a legend
that reads well.

Legend places the legend in a corner of the plot or turns it off.  Best puts it in the corner that covers the fewest
points and lines, and moves it as the view changes.

## Mouse in the Plot

| Press, in OpenVSP's own scheme                          | In a plot                                                  |
|:--------------------------------------------------------|:-----------------------------------------------------------|
| Left drag, or Shift + left drag                         | Zoom to the box drawn                                      |
| Wheel                                                   | Zoom about the pointer; scrolling up zooms in              |
| Middle drag, or Ctrl or Cmd + left drag                 | Zoom about the middle of the plot; dragging down zooms in  |
| Right drag, or Alt + left drag                          | Pan                                                        |
| Double click                                            | Return to the whole plot                                   |
| Hover                                                   | Read out the nearest point                                 |

These are the presses of OpenVSP's own scheme; the plot follows the scheme and 3D View settings chosen in the
preferences, as [Mouse and Keyboard](MouseKeyboard.md#plots) describes.  The keys give a mouse of one or two buttons
every action, on Windows, Linux and macOS alike.  Hovering within a few pixels of a drawn point circles it and reads
out, under the plot, its series, its index in the data, and its x and y; a series hidden by turning both Lines and
Points off is not read out.  Away from any point, the readout gives the x and y under the cursor.  A larger window gives
the plot the extra room, and a taller one lengthens the Results list too.

## Export

On the Data tab, *Export CSV* writes the Result shown to a CSV file.

On the Plot tab, *Export CSV* writes the data as plotted: two columns, X and Y, for each series, headed with the
series' legend name and the variable, with values written in full precision.  *Save Image* writes the plot as it is
shown, as a PNG file, or as an SVG file when the file name ends in .svg.

## Plots in the Model File

Plots and their curves are saved with the model; Results are not.  A curve names kinds of Result and variables, not
particular Results, so after a model is opened its plots stay empty, with a note that there are no Results for each
curve, until the analyses are run again.  Then they fill from the new runs.  A plot set up once can be used for every
later study of the same model.

Inserting a file brings its plots too, after the plots already in the model; a plot whose name is taken gets a number
added.  A new model starts with no plots.

## Plot Files

A plot that took a while to set up can be kept in a file of its own and loaded into any model.  *Save...* beside the
Plot choice writes the current plot to a plot file (\*.vspplot): its name, axes, style and every curve, with each
curve's kinds of Result, runs and variables.  No Results are written.  *Load...* reads a plot file and adds its plot
after the plots already in the model, as the current plot; a plot whose name is taken gets a number added.  A file
can be loaded more than once, and each load adds another plot.

A plot file names its data the same way the model file does: each kind of Result by its Results name and case name,
and each variable by its label.  It holds no Result IDs, so it does not depend on the model it was saved from.
Loaded into a model whose Results match, the plot draws them at once.  Loaded into a model without them, it behaves as
a plot does after a model is opened: the Results list keeps each kind the curve names, marked (none), the X and Y
lists keep its variables, the plot is empty with a note that there are no Results for the curve, and it fills when
the analyses are run.  A case name is the name of a Geometry Analysis case, so in another model the case must have the
same name for its Results to match.  Manual axis limits are kept as they were saved.  A curve that plots chosen runs
(Choose...) plots every run after a load, since which runs were left out names particular Results.

## Scripting

The API can build the same plots a user builds on the Plot tab, so a script that runs a study can leave its plots
ready to look at in the GUI, or saved in the model for later.  Plots and curves are identified by ID.  A plot's
axes and style, and a curve's mode, element, matrix choice and runs, are Parms: group "ResultsPlot" for a plot (XMin,
XMax, YMin, YMax, XMinManual, XMaxManual, YMinManual, YMaxManual, XLog, YLog, ShowLines, ShowPoints, Legend) and
group "ResultsCurve" for a curve (Mode, Element, MatrixRows, Run).  They are set with SetParmVal, and a plot is renamed
with SetContainerName.  The enums RESULTS_PLOT_MODE, RESULTS_PLOT_ELEMENT, RESULTS_PLOT_LEGEND and RESULTS_PLOT_RUN
give their values; Run is RESULTS_RUN_CHOSEN (-1), RESULTS_RUN_ALL (0), RESULTS_RUN_NEWEST (1), or n for the nth
newest.

Unlike *Add* on the Plot tab, AddResultsCurve adds an empty curve.

| Function                       | Purpose                                                                         |
|:-------------------------------|:--------------------------------------------------------------------------------|
| AddResultsPlot                 | Add a plot, with one empty curve, and make it active                            |
| CopyResultsPlot                | Copy a plot and its curves, and make the copy active                            |
| DeleteResultsPlot              | Delete a plot and its curves                                                    |
| DeleteAllResultsPlots          | Delete every plot                                                               |
| WriteResultsPlotFile           | Write a plot to a plot file, without Results                                    |
| ReadResultsPlotFile            | Add the plot a plot file holds, and make it active                              |
| GetAllResultsPlotIDs           | IDs of every plot, in the order the viewer lists them                           |
| SetActiveResultsPlot           | Make a plot the one the viewer shows                                            |
| GetActiveResultsPlot           | ID of the plot the viewer shows                                                 |
| AddResultsCurve                | Add an empty curve to a plot                                                    |
| DeleteResultsCurve             | Delete a curve                                                                  |
| GetResultsCurveIDs             | IDs of a plot's curves, in order                                                |
| SetResultsCurveSources         | Set a curve's kinds of Result, as Results names and case names                  |
| GetResultsCurveSourceNames     | Results names of a curve's kinds of Result                                      |
| GetResultsCurveSourceCaseNames | Case names of a curve's kinds of Result, empty where there is none              |
| SetResultsCurveXVar            | Set a curve's X variable by label; empty for the index or Result order          |
| GetResultsCurveXVar            | Label of a curve's X variable                                                   |
| SetResultsCurveYVars           | Set a curve's Y variables by label                                              |
| GetResultsCurveYVars           | Labels of a curve's Y variables                                                 |
| GetResultsCurveResultIDs       | IDs of the Results a curve plots now, oldest first                              |
| SetResultsCurveChosenIDs       | Choose the Results a curve plots when its runs are Chosen                       |
| GetResultsCurveChosenIDs       | IDs of the Results a curve plots when its runs are Chosen                       |
| GetResultsPlotVarLabels        | Labels of the variables in a Result that a curve can plot                       |
| GetResultsPlotVarData          | Values of one variable in a Result, as a curve plots them                       |
| SetResultsNickname             | Give a Result a nickname; an empty string removes it                            |
| GetResultsNickname             | A Result's nickname, empty when it has none                                     |
