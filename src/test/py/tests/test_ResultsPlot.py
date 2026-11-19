# Results plots through the API: what is saved with the model, and what an inserted file brings.

import openvsp as vsp
import os
import tempfile


def pop_errors():
    """Take everything off the error stack and return what it said."""
    mgr = vsp.ErrorMgrSingleton.getInstance()
    return [ mgr.PopLastError().m_ErrorString for _ in range( mgr.GetNumTotalErrors() ) ]


# Every plot setting, each away from its default.
PLOT_SETTINGS = [ ( "XMinManual", 1.0 ), ( "XMaxManual", 1.0 ), ( "YMinManual", 1.0 ), ( "YMaxManual", 1.0 ),
                  ( "XMin", -2.5 ), ( "XMax", 7.5 ), ( "YMin", 0.5 ), ( "YMax", 9.0 ), ( "XLog", 1.0 ), ( "YLog", 1.0 ),
                  ( "ShowLines", 0.0 ), ( "ShowPoints", 0.0 ), ( "Legend", vsp.RESULTS_LEGEND_LOWER_LEFT ) ]


def make_plot():
    """A plot with two curves, every setting away from its default."""
    plot = vsp.AddResultsPlot()
    vsp.SetContainerName( plot, "Lift" )
    for name, val in PLOT_SETTINGS:
        vsp.SetParmVal( vsp.FindParm( plot, name, "ResultsPlot" ), val )

    first = vsp.GetResultsCurveIDs( plot )[0]
    vsp.SetResultsCurveSources( first, [ "Test_Results", "VSPAERO_Polar" ], [ "", "Cruise" ] )
    vsp.SetResultsCurveXVar( first, "Test_Double_Vec" )
    vsp.SetResultsCurveYVars( first, [ "Test_Vec3d.x", "|Test_Vec3d|" ] )
    vsp.SetParmVal( vsp.FindParm( first, "Element", "ResultsCurve" ), vsp.RESULTS_PLOT_LAST )
    vsp.SetParmVal( vsp.FindParm( first, "MatrixRows", "ResultsCurve" ), 1.0 )
    vsp.SetParmVal( vsp.FindParm( first, "Run", "ResultsCurve" ), 3.0 )

    second = vsp.AddResultsCurve( plot )
    vsp.SetResultsCurveSources( second, [ "Test_Results" ], [ "" ] )
    vsp.SetResultsCurveYVars( second, [ "Test_Double" ] )
    vsp.SetParmVal( vsp.FindParm( second, "Mode", "ResultsCurve" ), vsp.RESULTS_PLOT_ACROSS_RESULTS )
    vsp.SetParmVal( vsp.FindParm( second, "Element", "ResultsCurve" ), vsp.RESULTS_PLOT_LAST )
    vsp.SetParmVal( vsp.FindParm( second, "Run", "ResultsCurve" ), 2.0 )
    return plot


def describe( plot ):
    """Everything about a plot that is saved, by value rather than by ID."""
    curves = []
    for c in vsp.GetResultsCurveIDs( plot ):
        curves.append( ( list( vsp.GetResultsCurveSourceNames( c ) ),
                         list( vsp.GetResultsCurveSourceCaseNames( c ) ),
                         vsp.GetResultsCurveXVar( c ),
                         list( vsp.GetResultsCurveYVars( c ) ),
                         [ vsp.GetParmVal( vsp.FindParm( c, n, "ResultsCurve" ) ) for n in ( "Mode", "Element", "MatrixRows", "Run" ) ] ) )
    parms = [ vsp.GetParmVal( vsp.FindParm( plot, n, "ResultsPlot" ) ) for n, _ in PLOT_SETTINGS ]
    return ( parms, curves )


def testAPlotMadeThroughTheApiIsSavedWithTheModel():
    vsp.VSPRenew()
    plot = make_plot()
    before = describe( plot )

    written = os.path.join( tempfile.mkdtemp(), "plot.vsp3" )
    vsp.WriteVSPFile( written )
    vsp.VSPRenew()
    assert len( vsp.GetAllResultsPlotIDs() ) == 0

    vsp.ReadVSPFile( written )
    plots = vsp.GetAllResultsPlotIDs()
    assert len( plots ) == 1
    assert vsp.GetContainerName( plots[0] ) == "Lift"
    assert describe( plots[0] ) == before
    assert pop_errors() == []


def testAnInsertedPlotIsANewPlot():
    """Inserting a file into the model it came from collides every ID; the inserted plot and its curves get new ones."""
    vsp.VSPRenew()
    plot = make_plot()
    before = describe( plot )

    written = os.path.join( tempfile.mkdtemp(), "plot.vsp3" )
    vsp.WriteVSPFile( written )
    vsp.InsertVSPFile( written, "" )

    plots = vsp.GetAllResultsPlotIDs()
    assert len( plots ) == 2
    assert plots[0] == plot
    assert vsp.GetContainerName( plots[1] ) == "Lift 1"
    assert describe( plots[1] ) == before

    curves = list( vsp.GetResultsCurveIDs( plots[0] ) ) + list( vsp.GetResultsCurveIDs( plots[1] ) )
    assert len( set( curves ) ) == 4
    assert pop_errors() == []


def testAnInsertedLinkDrivesTheInsertedPlot():
    """The link is read before the plot it names, so the plot's IDs have to be known before either is read."""
    vsp.VSPRenew()
    plot = make_plot()
    curve = vsp.GetResultsCurveIDs( plot )[1]
    run = vsp.FindParm( curve, "Run", "ResultsCurve" )
    vsp.AddAdvLink( "L" )
    vsp.AddAdvLinkInput( 0, run, "run" )
    vsp.Update()

    written = os.path.join( tempfile.mkdtemp(), "plot.vsp3" )
    vsp.WriteVSPFile( written )
    vsp.InsertVSPFile( written, "" )
    vsp.Update()

    inserted_curve = vsp.GetResultsCurveIDs( vsp.GetAllResultsPlotIDs()[1] )[1]
    inputs = [ vsp.GetParmContainer( vsp.GetAdvLinkInputParms( i )[0] ) for i in range( len( vsp.GetAdvLinkNames() ) ) ]
    assert inputs == [ curve, inserted_curve ]
    assert pop_errors() == []


def testCurveCallsRefuseWhatIsNotACurve():
    vsp.VSPRenew()
    plot = vsp.AddResultsPlot()
    pop_errors()

    vsp.SetResultsCurveYVars( plot, [ "Y" ] )
    assert len( pop_errors() ) == 1
    curve = vsp.GetResultsCurveIDs( plot )[0]
    assert list( vsp.GetResultsCurveYVars( curve ) ) == []

    vsp.SetResultsCurveSources( curve, [ "A", "B" ], [ "" ] )
    assert len( pop_errors() ) == 1
    assert list( vsp.GetResultsCurveSourceNames( curve ) ) == []


def testChosenResultsAreThoseOfTheCurvesKinds():
    """A curve plots the chosen Results of its kinds only, and nothing once its chosen Results are gone."""
    vsp.VSPRenew()
    vsp.WriteTestResults()
    plot = vsp.AddResultsPlot()
    curve = vsp.GetResultsCurveIDs( plot )[0]
    vsp.SetResultsCurveSources( curve, [ "Test_Results" ], [ "" ] )
    older, newer = vsp.GetResultsCurveResultIDs( curve )
    assert newer == vsp.FindLatestResultsID( "Test_Results" )

    other = vsp.CreateGeomResults( vsp.AddGeom( "POD" ), "Other" )
    vsp.SetParmVal( vsp.FindParm( curve, "Run", "ResultsCurve" ), vsp.RESULTS_RUN_CHOSEN )
    vsp.SetResultsCurveChosenIDs( curve, [ newer, other ] )
    assert list( vsp.GetResultsCurveResultIDs( curve ) ) == [ newer ]

    vsp.DeleteResult( newer )
    assert list( vsp.GetResultsCurveResultIDs( curve ) ) == []

    vsp.SetResultsCurveChosenIDs( curve, [ "NOTARESULT" ] )
    assert len( pop_errors() ) == 1
    assert list( vsp.GetResultsCurveChosenIDs( curve ) ) == []



def testNewResultsAreChosen():
    """Chosen starts from every run and leaves some out; runs made later are plotted until they are left out."""
    vsp.VSPRenew()
    vsp.DeleteAllResults()
    vsp.WriteTestResults()
    plot = vsp.AddResultsPlot()
    curve = vsp.GetResultsCurveIDs( plot )[0]
    vsp.SetResultsCurveSources( curve, [ "Test_Results" ], [ "" ] )
    vsp.SetParmVal( vsp.FindParm( curve, "Run", "ResultsCurve" ), vsp.RESULTS_RUN_CHOSEN )
    older, newer = vsp.GetResultsCurveResultIDs( curve )

    vsp.SetResultsCurveChosenIDs( curve, [ newer ] )
    assert list( vsp.GetResultsCurveResultIDs( curve ) ) == [ newer ]

    vsp.WriteTestResults()
    made = [ r for r in vsp.FindResultsIDs( "Test_Results" ) if r not in ( older, newer ) ]
    assert len( made ) == 2
    assert list( vsp.GetResultsCurveResultIDs( curve ) ) == [ newer ] + made
    assert list( vsp.GetResultsCurveChosenIDs( curve ) ) == [ newer ] + made
    assert pop_errors() == []


def testAPlotFileCarriesThePlotToAnotherModel():
    """A plot file holds the plot without Results; read into a new model the plot is the same and plots its Results."""
    vsp.VSPRenew()
    vsp.DeleteAllResults()
    plot = make_plot()
    before = describe( plot )

    written = os.path.join( tempfile.mkdtemp(), "lift.vspplot" )
    vsp.WriteResultsPlotFile( plot, written )

    vsp.VSPRenew()
    vsp.AddGeom( "POD" )
    read = vsp.ReadResultsPlotFile( written )
    assert list( vsp.GetAllResultsPlotIDs() ) == [ read ]
    assert vsp.GetActiveResultsPlot() == read
    assert vsp.GetContainerName( read ) == "Lift"
    assert describe( read ) == before

    # No Results of the curves' kinds yet; then some are made, and the second curve plots the run before the newest.
    second = vsp.GetResultsCurveIDs( read )[1]
    assert list( vsp.GetResultsCurveResultIDs( second ) ) == []
    vsp.WriteTestResults()
    assert list( vsp.GetResultsCurveResultIDs( second ) ) == list( vsp.FindResultsIDs( "Test_Results" ) )[:1]
    assert pop_errors() == []


def testAPlotFileReadTwiceGivesTwoPlots():
    """Reading a file into the model it came from collides every ID; each plot read gets new ones and a free name."""
    vsp.VSPRenew()
    plot = make_plot()
    before = describe( plot )

    written = os.path.join( tempfile.mkdtemp(), "lift.vspplot" )
    vsp.WriteResultsPlotFile( plot, written )
    first = vsp.ReadResultsPlotFile( written )
    second = vsp.ReadResultsPlotFile( written )

    plots = list( vsp.GetAllResultsPlotIDs() )
    assert plots == [ plot, first, second ]
    assert [ vsp.GetContainerName( p ) for p in plots ] == [ "Lift", "Lift 1", "Lift 2" ]
    assert describe( first ) == before
    assert describe( second ) == before

    curves = [ c for p in plots for c in vsp.GetResultsCurveIDs( p ) ]
    assert len( set( curves ) ) == 6
    assert pop_errors() == []


def testOnlyAPlotFileGivesAPlot():
    vsp.VSPRenew()
    plot = make_plot()
    folder = tempfile.mkdtemp()

    model = os.path.join( folder, "plot.vsp3" )
    vsp.WriteVSPFile( model )
    assert vsp.ReadResultsPlotFile( model ) == ""
    assert vsp.ReadResultsPlotFile( os.path.join( folder, "missing.vspplot" ) ) == ""
    assert len( pop_errors() ) == 2

    vsp.WriteResultsPlotFile( "NOTAPLOT", os.path.join( folder, "none.vspplot" ) )
    assert len( pop_errors() ) == 1
    assert not os.path.exists( os.path.join( folder, "none.vspplot" ) )
    assert list( vsp.GetAllResultsPlotIDs() ) == [ plot ]
