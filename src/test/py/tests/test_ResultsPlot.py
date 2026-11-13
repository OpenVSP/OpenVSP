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
