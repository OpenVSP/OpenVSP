//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)

// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

/*!
    \defgroup APIError API Error Functions
    \brief Handling of OpenVSP ErrorObj information is accomplished through this group of API functions.
    \ref index "Click here to return to the main page"

    \defgroup APIUtilities General API Utility Functions
    \brief This group of functions is provided for general API utilities, such as printing to stdout, performing
    basic math functions, and identifying basic OpenVSP information.
    \ref index "Click here to return to the main page"

    \defgroup AdvancedLink Advanced Link Functions
    \brief The following functions are available for the Advanced Link tool.
    \ref index "Click here to return to the main page"

    \defgroup Analysis Analysis Manager Functions
    \brief This group is for functions included in the Analysis Manager. The Analysis Manager allows for
    OpenVSP analyses to be setup and run through the API without having to modify Parms directly. Examples
    are available for every available analysis type. The results of running an analysis can be accessed
    through the functions defined in the Results group.
    \ref index "Click here to return to the main page"

    \defgroup Attributes Attributes Manager Functions
    \brief This group is for functions included in the Attributes Manager. The Attributes Manager stores
    Attributes and provides methods to add, delete, get and set them.
    \ref index "Click here to return to the main page"


    \defgroup Background3D Background3D Functions
    \brief This group of functions is used to work with 3D background images.
    \ref index "Click here to return to the main page"

    \defgroup BOR BOR Functions
    \brief This group of API functions provides capabilities related to the body of revolution (BOR) geometry type in OpenVSP.
    \ref index "Click here to return to the main page"

    \defgroup CFDMesh CFD Mesh Functions
    \brief This group of functions is used to setup and run the CFD Mesh tool through the API.
    \ref index "Click here to return to the main page"

    \defgroup CSGroup VSPAERO Control Surface Group Functions
    \brief This group of functions is available for manipulating VSPAERO control surface groups through the API.
    Note, VSPAERO also includes rectangle type sub-surfaces as possible control surfaces.
    \ref index "Click here to return to the main page"

    \defgroup Computations General Computation Functions
    \brief The following group of API functions are available for general computations. In general, it is best practice to
    perform computations through the the Analysis group instead of calling these functions directly.
    \ref index "Click here to return to the main page"

    \defgroup CustomGeom Custom Geometry Functions
    \brief This functions grouped here are used to create and manipulate Custom Components. Custom components
    are defined in \\*.vsppart files included in the /"Custom Scripts/" directory. Examples of Custom Components
    are available in the directory for reference. OpenVSP looks in 3 locations for the /"Custom Scripts/" folder,
    where Custom Components are loaded: the root directory, the VSP executable directory, and the home directory.
    Note, these functions are specific to defining Custom Components and can't be called from standard API scripts
    (i.e. \\*.vspscript). However, a Custom Component can be created as a \\*.vsppart file and then accessed through
    secondary API scripts.
    \ref index "Click here to return to the main page"

    \defgroup DesignFile Design File Functions
    \brief This group of functions is available for managing Design Variables through the API.
    \ref index "Click here to return to the main page"

    \defgroup EditCurveXSec Edit Curve XSec Functions
    \brief Functions for modifying XSecs of type XS_EDIT_CURVE are defined here.
    \ref index "Click here to return to the main page"

    \defgroup Enumerations Enumerations
    \brief All API enumerations are defined in this group.
    \ref index "Click here to return to the main page"

    \defgroup FEAMesh FEA Mesh Functions
    \brief The following group of API functions supports all functionality of the FEA Mesh Tool. Structures, FEA Parts,
    materials, and properties can be defined and manipulated. Mesh and output file settings can be adjusted, and an FEA
    mesh can be generated.
    \ref index "Click here to return to the main page"

    \defgroup FileIO File Input and Output Functions
    \brief This group of functions provides file input and output interfacing through the API.
    \ref index "Click here to return to the main page"

    \defgroup GearGeom GearGeom Functions
    \brief This group of functions is available for interacting with the Bogie list of a GearGeom through the API.
    \ref index "Click here to return to the main page"

    \defgroup Geom Geom Functions
    \brief This group of functions is available for adding, deleting, and modifying OpenVSP Geoms through the API.
    \ref index "Click here to return to the main page"

    \defgroup GroupMod Group Modification Functions
    \brief The functions in this group allow for sets to be scaled, rotated, and translated.
    \ref index "Click here to return to the main page"

    \defgroup Matrix4d Matrix4d Functions
    \brief API functions that utilize the Matrix4d class are grouped here. For details of the class, including member functions, see Matrix4d.
    \ref index "Click here to return to the main page"

    \defgroup Measure Measure Tool Functions
    \brief This group of API functions can be used to control the Ruler Tool through the API.
    \ref index "Click here to return to the main page"

    \defgroup Mode Mode Functions
    \brief This group of API functions are used to manipulate Modes -- a combination of Sets and Variable Presets
    \ref index "Click here to return to the main page"

    \defgroup PCurve Propeller Blade Curve Functions
    \brief The following group of API functions may be used to control parametric propeller blade curves (PCurves).
    \ref index "Click here to return to the main page"

    \defgroup ParasiteDrag Parasite Drag Functions
    \brief This group of API functions is supplemental to performing a Paraste Drag analysis through the Analysis Manager. They include
    functions to write out Parasite Drag Tool equations, calculate atmospheric properties, and control excrescences.
    \ref index "Click here to return to the main page"

    \defgroup Parm Parm Functions
    \brief Every Parm in OpenVSP can be accessed and modified through the functions defined in this API group.
    Every Parm has an associated ParmContainer.
    \ref index "Click here to return to the main page"

    \defgroup ParmContainer Parm Container Functions
    \brief All Parms in OpenVSP are stored in Parm Containers. The functions in this group can be used to
    work with Parm Containers through the API.
    \ref index "Click here to return to the main page"

    \defgroup ProxyUtitity API Proxy Utility Functions
    \brief The API functions defined in this group enable conversion between AngelScript and OpenVSP C++ data types,
    such as array and vector.
    \ref index "Click here to return to the main page"

    \defgroup Results Results Manager Functions
    \brief This group is for functions included in the Results Manager. The Results Manager stores
    analysis results and provides methods to get, print, and export them.
    \ref index "Click here to return to the main page"

    \defgroup RoutingGeom RoutingGeom Functions
    \brief This group of API functions is used to create and manipulate RoutingGeom objects, which
    define wire routing paths between points in the model.
    \ref index "Click here to return to the main page"

    \defgroup Sets Functions for Sets
    \brief The following group of API functions deals with set manipulation.
    \ref index "Click here to return to the main page"

    \defgroup SnapTo Snap-To Functions
    \brief This group of API functions provide the capabilities available in the Snap-To tool.
    \ref index "Click here to return to the main page"

    \defgroup SubSurface Sub-Surface Functions
    \brief Functions related to Sub-Surfaces are defined in this group.
    \ref index "Click here to return to the main page"

    \defgroup SurfaceQuery Geom Surface Query Functions
    \brief This group of API functions pertains to general surface queries for Geom surfaces, such as computing
    3D location from surface coordinates, identifying curvature, and performing point projections.
    \ref index "Click here to return to the main page"

    \defgroup VSPAERO VSPAERO Functions
    \brief The following group of functions are specific to VSPAERO. However, their relevance has been
    mostly replaced by Analysis Manager capabilities.
    \ref index "Click here to return to the main page"

    \defgroup VSPAERODiskAndProp VSPAERO Actuator Disk and Propeller Functions
    \brief The following group of functions provide API capability for setting up actuator disks (Disk tab
    of VSPAERO GUI) and propellers (Propeller tab of VSPAERO GUI) for VSPAERO analysis. If a propeller
    geometry is used to model the actuator disk, the "PropMode" must be set to PROP_DISK or PROP_BOTH.
    Alternatively, the "PropMode" but be set to PROP_BLADE or PROP_BOTH for unsteady analysis.
    must be set to PROP_DISK or PROP_BOTH.
    \ref index "Click here to return to the main page"

    \defgroup VariablePreset Variable Preset Functions
    \brief This group of functions can be used to add, remove, and modify Variable Presets through the API.
    \ref index "Click here to return to the main page"

    \defgroup Vehicle Vehicle Functions
    \brief The Vehicle group of functions are high-level commands that pertain to the entire OpenVSP model.
    \ref index "Click here to return to the main page"

    \defgroup Visualization Visualization Functions
    \brief The following group of functions allow for the OpenVSP GUI to be manipulated through the API.
    \ref index "Click here to return to the main page"

    \defgroup XSec XSec and Airfoil Functions
    \brief This group of functions provides API control of cross-sections (XSecs). Airfoils are a type of
    XSec included in this group as well. API functions for Body of Revolution XSecs are included in the
    Specialized Geometry group.
    \ref index "Click here to return to the main page"

    \defgroup XSecSurf XSecSurf Functions
    \brief This group of API functions provides capabilities related to the XSecSurf class in OpenVSP.
    \ref index "Click here to return to the main page"

    \defgroup vec3d Vec3D Functions
    \brief API functions that utilize the vec3d class are grouped here. For details of the class, including member functions, see vec3d.
    \ref index "Click here to return to the main page"
*/

#if !defined(VSPAPI__INCLUDED_)
#define VSPAPI__INCLUDED_

#include "APIDefines.h"

#include <string>
#include <stack>
#include <vector>

class vec3d;
class Matrix4d;

using std::string;
using std::stack;
using std::vector;

namespace vsp
{

//======================== API Functions ================================//
/*!
    \ingroup APIUtilities
*/
/*!
    Check if OpenVSP has been initialized successfully. If not, the OpenVSP instance will be exited. This call should be placed at the
    beginning of all API scripts.
    \forcpponly
    \code{.cpp}

    VSPCheckSetup();

    // A failed setup exits OpenVSP outright, so reaching this point is most of
    // the test.  Confirm the model is actually usable.
    array< string > @type_array = GetGeomTypes();

    if ( type_array.length() == 0 )
    {
        Print( "ERROR: VSPCheckSetup did not leave a usable model" );
        __failure++;
    }

    // Continue to do things...

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    VSPCheckSetup()

    # A failed setup exits OpenVSP outright, so reaching this point is most of
    # the test.  Confirm the model is actually usable.
    type_array = GetGeomTypes()

    assert len( type_array ) > 0, "VSPCheckSetup did not leave a usable model"

    # Continue to do things...


    \endcode
    \endPythonOnly
*/

extern void VSPCheckSetup();

/*!
    \ingroup APIUtilities
*/
/*!
    Clear and reinitialize OpenVSP to all default settings
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    SetParmVal( pod_id, "Y_Rel_Location", "XForm", 2.0 );

    VSPRenew();

    if ( FindGeoms().size() != 0 ) { Print( "ERROR: VSPRenew" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    SetParmVal( pod_id, "Y_Rel_Location", "XForm", 2.0 )

    VSPRenew()

    if  len(FindGeoms()) != 0 :
        print( "ERROR: VSPRenew" )
        assert False, "ERROR: VSPRenew"

    \endcode
    \endPythonOnly
*/

extern void VSPRenew();


/*!
    \ingroup Vehicle
*/
/*!
    Update the entire vehicle and all lower level children. An input, which is true by default, is available to specify
    if managers should be updated as well. The managers are typically updated by their respective GUI, so must be
    updated through the API as well to avoid unexpected behavior.
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

    int num_xsecs = GetNumXSec( xsec_surf );

    Update();

    vec3d before_max = GetGeomBBoxMax( fid, 0, false );

    //==== Set Tan Angles At Nose/Tail
    SetXSecTanAngles( GetXSec( xsec_surf, 0 ), XSEC_BOTH_SIDES, 90 );
    SetXSecTanAngles( GetXSec( xsec_surf, num_xsecs - 1 ), XSEC_BOTH_SIDES, -90 );

    Update();       // Force Surface Update

    vec3d after_max = GetGeomBBoxMax( fid, 0, false );

    // Blunting the nose and tail pushes the surface out, which only shows up in
    // the bounding box once Update() has rebuilt it.
    if ( dist( before_max, after_max ) < 1e-9 )
    {
        Print( "ERROR: Update did not rebuild the surface" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    xsec_surf = GetXSecSurf( fid, 0 )           # Get First (and Only) XSec Surf

    num_xsecs = GetNumXSec( xsec_surf )

    Update()

    before_max = GetGeomBBoxMax( fid, 0, False )

    #==== Set Tan Angles At Nose/Tail
    SetXSecTanAngles( GetXSec( xsec_surf, 0 ), XSEC_BOTH_SIDES, 90, -1.0e12, -1.0e12, -1.0e12 )
    SetXSecTanAngles( GetXSec( xsec_surf, num_xsecs - 1 ), XSEC_BOTH_SIDES, -90, -1.0e12, -1.0e12, -1.0e12 )

    Update()       # Force Surface Update

    after_max = GetGeomBBoxMax( fid, 0, False )

    # Blunting the nose and tail pushes the surface out, which only shows up in
    # the bounding box once Update() has rebuilt it.
    assert dist( before_max, after_max ) > 1e-9, "Update did not rebuild the surface"

    \endcode
    \endPythonOnly
    \param update_managers bool Flag to indicate if managers should be updated
*/

extern void Update( bool update_managers = true );

/*!
    \ingroup Vehicle
*/
/*!
    Exit the program with a specific error code
    \param [in] error_code int Error code
*/

extern void VSPExit( int error_code );

/*!
    \ingroup Vehicle
*/
/*!
    Cause OpenVSP to crash in a variety of ways.
    \param [in] crash_type int Type of crash to attempt.
*/

extern void VSPCrash( int crash_type );

/*!
    \ingroup Vehicle
*/
/*!
    Return the OpenVSP update count and also reset it to zero.

    The OpenVSP update count tracks how many times the GUI has been told to update screens (set to dirty).  It
    provides a simple means of testing whether the OpenVSP state has possibly changed (non-zero returned).

    \return int OpenVSP update count
*/

extern int GetAndResetUpdateCount();


/*!
    \ingroup APIUtilities
*/
/*!
    Get the version of the OpenVSP instance currently running
    \forcpponly
    \code{.cpp}
    Print( "The current OpenVSP version is: ", false );

    string ver = GetVSPVersion();

    Print( ver );

    // The string form has to agree with the numeric accessors.
    string num = formatInt( GetVSPVersionMajor() ) + "." +
                 formatInt( GetVSPVersionMinor() ) + "." +
                 formatInt( GetVSPVersionChange() );

    if ( ver.findFirst( num ) < 0 )
    {
        Print( "ERROR: GetVSPVersion does not contain " + num );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    ver = GetVSPVersion()

    print( ver )

    # The string form has to agree with the numeric accessors.
    num = f"{GetVSPVersionMajor()}.{GetVSPVersionMinor()}.{GetVSPVersionChange()}"

    assert num in ver, "GetVSPVersion does not contain " + num

    \endcode
    \endPythonOnly
    \return string OpenVSP version string (i.e. "OpenVSP 3.17.1")
*/

extern std::string GetVSPVersion();

/*!
    \ingroup APIUtilities
*/
/*!
    Get the major version of the OpenVSP instance currently running as an integer
    \forcpponly
    \code{.cpp}
    Print( "The current OpenVSP version is: ", false );

    int major = GetVSPVersionMajor();
    int minor = GetVSPVersionMinor();
    int change = GetVSPVersionChange();

    Print( formatInt(major) + "." + formatInt(minor) + "." + formatInt(change) );

    // OpenVSP 3 and later.  Negative pieces would mean the version was never
    // filled in.
    if ( major < 3 || minor < 0 || change < 0 )
    {
        Print( "ERROR: implausible version number" );
        __failure++;
    }

    // The pieces have to add back up to the string form.
    string num = formatInt(major) + "." + formatInt(minor) + "." + formatInt(change);

    if ( GetVSPVersion().findFirst( num ) < 0 )
    {
        Print( "ERROR: version pieces disagree with GetVSPVersion" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    major = GetVSPVersionMajor()
    minor = GetVSPVersionMinor()
    change = GetVSPVersionChange()

    print( f"{major}.{minor}.{change}" )

    # OpenVSP 3 and later.  Negative pieces would mean the version was never
    # filled in.
    assert major >= 3 and minor >= 0 and change >= 0, "implausible version number"

    # The pieces have to add back up to the string form.
    num = f"{major}.{minor}.{change}"

    assert num in GetVSPVersion(), "version pieces disagree with GetVSPVersion"

    \endcode
    \endPythonOnly
    \return int OpenVSP major version number (i.e. 3 in 3.X.Y)
*/

extern int GetVSPVersionMajor();

/*!
    \ingroup APIUtilities
*/
/*!
    Get the minor version of the OpenVSP instance currently running as an integer
    \forcpponly
    \code{.cpp}
    Print( "The current OpenVSP version is: ", false );

    int major = GetVSPVersionMajor();
    int minor = GetVSPVersionMinor();
    int change = GetVSPVersionChange();

    Print( formatInt(major) + "." + formatInt(minor) + "." + formatInt(change) );

    // OpenVSP 3 and later.  Negative pieces would mean the version was never
    // filled in.
    if ( major < 3 || minor < 0 || change < 0 )
    {
        Print( "ERROR: implausible version number" );
        __failure++;
    }

    // The pieces have to add back up to the string form.
    string num = formatInt(major) + "." + formatInt(minor) + "." + formatInt(change);

    if ( GetVSPVersion().findFirst( num ) < 0 )
    {
        Print( "ERROR: version pieces disagree with GetVSPVersion" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    major = GetVSPVersionMajor()
    minor = GetVSPVersionMinor()
    change = GetVSPVersionChange()

    print( f"{major}.{minor}.{change}" )

    # OpenVSP 3 and later.  Negative pieces would mean the version was never
    # filled in.
    assert major >= 3 and minor >= 0 and change >= 0, "implausible version number"

    # The pieces have to add back up to the string form.
    num = f"{major}.{minor}.{change}"

    assert num in GetVSPVersion(), "version pieces disagree with GetVSPVersion"

    \endcode
    \endPythonOnly
    \return int OpenVSP minor version number (i.e. X in 3.X.Y)
*/

extern int GetVSPVersionMinor();

/*!
    \ingroup APIUtilities
*/
/*!
    Get the change version of the OpenVSP instance currently running as an integer
    \forcpponly
    \code{.cpp}
    Print( "The current OpenVSP version is: ", false );

    int major = GetVSPVersionMajor();
    int minor = GetVSPVersionMinor();
    int change = GetVSPVersionChange();

    Print( formatInt(major) + "." + formatInt(minor) + "." + formatInt(change) );

    // OpenVSP 3 and later.  Negative pieces would mean the version was never
    // filled in.
    if ( major < 3 || minor < 0 || change < 0 )
    {
        Print( "ERROR: implausible version number" );
        __failure++;
    }

    // The pieces have to add back up to the string form.
    string num = formatInt(major) + "." + formatInt(minor) + "." + formatInt(change);

    if ( GetVSPVersion().findFirst( num ) < 0 )
    {
        Print( "ERROR: version pieces disagree with GetVSPVersion" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    major = GetVSPVersionMajor()
    minor = GetVSPVersionMinor()
    change = GetVSPVersionChange()

    print( f"{major}.{minor}.{change}" )

    # OpenVSP 3 and later.  Negative pieces would mean the version was never
    # filled in.
    assert major >= 3 and minor >= 0 and change >= 0, "implausible version number"

    # The pieces have to add back up to the string form.
    num = f"{major}.{minor}.{change}"

    assert num in GetVSPVersion(), "version pieces disagree with GetVSPVersion"

    \endcode
    \endPythonOnly
    \return int OpenVSP change version number (i.e. Y in 3.X.Y)
*/

extern int GetVSPVersionChange();

/*!
    \ingroup APIUtilities
*/
/*!
    Get the path to the OpenVSP executable. OpenVSP will assume that the VSPAERO, VSPSLICER, and VSPVIEWER are in the same directory unless
    instructed otherwise.
    \forcpponly
    \code{.cpp}
    Print( "The current VSP executable path is: ", false );

    string exe_path = GetVSPExePath();

    Print( exe_path );

    if ( exe_path.length() == 0 )
    {
        Print( "ERROR: GetVSPExePath returned an empty path" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current VSP executable path is: ", False )

    exe_path = GetVSPExePath()

    print( exe_path )

    assert len( exe_path ) > 0, "GetVSPExePath returned an empty path"

    \endcode
    \endPythonOnly
    \sa SetVSPAEROPath, CheckForVSPAERO, GetVSPAEROPath
    \return string Path to the OpenVSP executable
*/

extern std::string GetVSPExePath();


/*!
    \ingroup APIUtilities
*/
/*!
    Set the path to the VSPAERO executables (Solver, Viewer, and Slicer). By default, OpenVSP will assume that the VSPAERO executables are in the
    same directory as the VSP executable. However, this may need to be changed when using certain API languages like MATLAB and Python. For example,
    Python may treat the location of the Python executable as the VSP executable path, so either the VSPAERO executable needs to be moved to the same
    directory or this function can be called to tell Python where to look for VSPAERO.
    \forcpponly
    \code{.cpp}
    string orig_path = GetVSPAEROPath();

    if ( !CheckForVSPAERO( GetVSPExePath() ) )
    {
        string vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5";
        SetVSPAEROPath( vspaero_path );
    }

    // A directory with no VSPAERO in it has to be rejected, and rejecting it
    // must leave the stored path alone.
    if ( SetVSPAEROPath( "/no/such/directory/anywhere" ) )
    {
        Print( "ERROR: SetVSPAEROPath accepted a nonexistent directory" );
        __failure++;
    }

    if ( GetVSPAEROPath() != orig_path )
    {
        Print( "ERROR: a rejected SetVSPAEROPath changed the stored path" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    orig_path = GetVSPAEROPath()

    if  not CheckForVSPAERO( GetVSPExePath() ) :
        vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5"
        SetVSPAEROPath( vspaero_path )

    # A directory with no VSPAERO in it has to be rejected, and rejecting it
    # must leave the stored path alone.
    assert not SetVSPAEROPath( "/no/such/directory/anywhere" ), "SetVSPAEROPath accepted a nonexistent directory"

    assert GetVSPAEROPath() == orig_path, "a rejected SetVSPAEROPath changed the stored path"

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, CheckForVSPAERO, GetVSPAEROPath
    \param [in] path string Absolute path to directory containing VSPAERO executable
    \return bool Flag that indicates whether or not the path was set correctly
*/

extern bool SetVSPAEROPath( const std::string & path );

/*!
    \ingroup APIUtilities
*/
/*!
    Get the path that OpenVSP will use to look for all VSPAERO executables (Solver, Slicer, and Viewer) when attempting to execute
    VSPAERO. If the VSPAERO executables are not in this location, they must either be copied there or the VSPAERO path must be set
    using SetVSPAEROPath.
    \forcpponly
    \code{.cpp}
    if ( !CheckForVSPAERO( GetVSPAEROPath() ) )
    {
        Print( "VSPAERO is not where OpenVSP thinks it is. I should move the VSPAERO executable or call SetVSPAEROPath." );
    }

    if ( GetVSPAEROPath().length() == 0 )
    {
        Print( "ERROR: GetVSPAEROPath returned an empty path" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    if  not CheckForVSPAERO( GetVSPAEROPath() ) :
        print( "VSPAERO is not where OpenVSP thinks it is. I should move the VSPAERO executable or call SetVSPAEROPath." )

    assert len( GetVSPAEROPath() ) > 0, "GetVSPAEROPath returned an empty path"

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, CheckForVSPAERO, SetVSPAEROPath
    \return string Path OpenVSP will look for VSPAERO
*/

extern std::string GetVSPAEROPath();

/*!
    \ingroup APIUtilities
*/
/*!
    Check if all VSPAERO executables (Solver, Viewer, and Slicer) are in a given directory. Note that this function will return false
    if only one or two VSPAERO executables are found. An error message will indicate the executables that are missing. This may be
    acceptable, as only the Solver is needed in all cases. The Viewer and Slicer may not be needed.
    \forcpponly
    \code{.cpp}
    string vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5";

    if ( CheckForVSPAERO( vspaero_path ) )
    {
        SetVSPAEROPath( vspaero_path );
    }

    // A directory that cannot exist must not report VSPAERO in it.
    if ( CheckForVSPAERO( "/no/such/directory/anywhere" ) )
    {
        Print( "ERROR: CheckForVSPAERO found VSPAERO in a nonexistent directory" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5"

    if  CheckForVSPAERO( vspaero_path ) :
        SetVSPAEROPath( vspaero_path )

    # A directory that cannot exist must not report VSPAERO in it.
    assert not CheckForVSPAERO( "/no/such/directory/anywhere" ), "CheckForVSPAERO found VSPAERO in a nonexistent directory"

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, GetVSPAEROPath, SetVSPAEROPath
    \param [in] path string Absolute path to check for VSPAERO executables
    \return bool Flag that indicates if all VSPAERO executables are found or not
*/

extern bool CheckForVSPAERO( const std::string & path );

/*!
    \ingroup APIUtilities
*/
/*!
    Set the path to the OpenVSP help files. By default, OpenVSP will assume that the OpenVSP help directory is in the
    same directory as the VSP executable. However, this may need to be changed when using certain API languages like MATLAB and Python. For example,
    Python may treat the location of the Python executable as the VSP executable path, so either the VSPAERO executable needs to be moved to the same
    directory or this function can be called to tell Python where to look for help.
    \forcpponly
    \code{.cpp}
    string orig_path = GetVSPHelpPath();

    if ( !CheckForVSPHelp( GetVSPExePath() ) )
    {
        string vsphelp_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5/help";
        SetVSPHelpPath( vsphelp_path );
    }

    // A directory with no help files in it has to be rejected, and rejecting it
    // must leave the stored path alone.
    if ( SetVSPHelpPath( "/no/such/directory/anywhere" ) )
    {
        Print( "ERROR: SetVSPHelpPath accepted a nonexistent directory" );
        __failure++;
    }

    if ( GetVSPHelpPath() != orig_path )
    {
        Print( "ERROR: a rejected SetVSPHelpPath changed the stored path" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    orig_path = GetVSPHelpPath()

    if  not CheckForVSPHelp( GetVSPExePath() ) :
        vsphelp_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5/help"
        SetVSPHelpPath( vsphelp_path )

    # A directory with no help files in it has to be rejected, and rejecting it
    # must leave the stored path alone.
    assert not SetVSPHelpPath( "/no/such/directory/anywhere" ), "SetVSPHelpPath accepted a nonexistent directory"

    assert GetVSPHelpPath() == orig_path, "a rejected SetVSPHelpPath changed the stored path"

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, CheckForVSPHelp, GetVSPHelpPath
    \param [in] path string Absolute path to directory containing OpenVSP help files
    \return bool Flag that indicates whether or not the path was set correctly
*/

extern bool SetVSPHelpPath( const std::string & path );

/*!
    \ingroup APIUtilities
*/
/*!
    Get the path that OpenVSP will use to look for all OpenVSP help files. If the OpenVSP help files are not in this location,
    they must either be copied there or the VSPHelp path must be set using SetVSPHelpPath.
    \forcpponly
    \code{.cpp}
    if ( !CheckForVSPHelp( GetVSPHelpPath() ) )
    {
        Print( "OpenVSP help is not where OpenVSP thinks it is. I should move the help files or call SetVSPHelpPath." );
    }

    if ( GetVSPHelpPath().length() == 0 )
    {
        Print( "ERROR: GetVSPHelpPath returned an empty path" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    if  not CheckForVSPHelp( GetVSPHelpPath() ) :
        print( "OpenVSP help is not where OpenVSP thinks it is. I should move the help files or call SetVSPHelpPath." )

    assert len( GetVSPHelpPath() ) > 0, "GetVSPHelpPath returned an empty path"

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, CheckForVSPHelp, SetVSPHelpPath
    \return string Path OpenVSP will look for help files
*/

extern std::string GetVSPHelpPath();

/*!
    \ingroup APIUtilities
*/
/*!
    Check if all OpenVSP help files are in a given directory.
    \forcpponly
    \code{.cpp}
    string vsphelp_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5/help";

    if ( CheckForVSPHelp( vsphelp_path ) )
    {
        SetVSPHelpPath( vsphelp_path );
    }

    // A directory that cannot exist must not report help files in it.
    if ( CheckForVSPHelp( "/no/such/directory/anywhere" ) )
    {
        Print( "ERROR: CheckForVSPHelp found help in a nonexistent directory" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    vsphelp_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5/help"

    if  CheckForVSPHelp( vsphelp_path ) :
        SetVSPHelpPath( vsphelp_path )

    # A directory that cannot exist must not report help files in it.
    assert not CheckForVSPHelp( "/no/such/directory/anywhere" ), "CheckForVSPHelp found help in a nonexistent directory"

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, GetVSPAEROPath, SetVSPHelpPath
    \param [in] path string Absolute path to check for VSPAERO executables
    \return bool Flag that indicates if OpenVSP help files are found or not
*/

extern bool CheckForVSPHelp( const std::string & path );

extern void RegisterCFDMeshAnalyses();

extern void LimitedIntersectSurfaces( const vector < string > & geomvec, vector < vector < vec3d > > & ptchains, vector < vector < vec3d > > & uwchains );

//======================== File I/O ================================//
/*!
    \ingroup FileIO
*/
/*!
    Load an OpenVSP project from a VSP3 file
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    // A relative name is resolved against the working directory, so ask for the
    // resolved name rather than assuming it comes back verbatim.
    string full_name = GetVSPFileName();

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    if ( FindGeoms().size() != 0 )
    {
        Print( "ERROR: ClearVSPModel left Geoms behind" );
        __failure++;
    }

    ReadVSPFile( fname );

    // The Fuselage has to come back, and come back the same shape.
    array< string > @geoms = FindGeoms();

    if ( geoms.size() != 1 )
    {
        Print( "ERROR: ReadVSPFile did not restore the model" );
        __failure++;
    }
    else
    {
        if ( GetGeomTypeName( geoms[0] ) != "Fuselage" )
        {
            Print( "ERROR: ReadVSPFile restored the wrong Geom type" );
            __failure++;
        }

        if ( GetVSPFileName() != full_name )
        {
            Print( "ERROR: ReadVSPFile did not set the project file name" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    # A relative name is resolved against the working directory, so ask for the
    # resolved name rather than assuming it comes back verbatim.
    full_name = GetVSPFileName()

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )

    ClearVSPModel()

    assert len( FindGeoms() ) == 0, "ClearVSPModel left Geoms behind"

    ReadVSPFile( fname )

    # The Fuselage has to come back, and come back the same shape.
    geoms = FindGeoms()

    assert len( geoms ) == 1, "ReadVSPFile did not restore the model"
    assert GetGeomTypeName( geoms[0] ) == "Fuselage", "ReadVSPFile restored the wrong Geom type"
    assert GetVSPFileName() == full_name, "ReadVSPFile did not set the project file name"

    \endcode
    \endPythonOnly
    \param [in] file_name string \\*.vsp3 file name
*/

extern void ReadVSPFile( const std::string & file_name );

/*!
    \ingroup FileIO
*/
/*!
    Save the current OpenVSP project to a VSP3 file
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    // A relative name is resolved against the working directory, so ask for the
    // resolved name rather than assuming it comes back verbatim.
    string full_name = GetVSPFileName();

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    if ( FindGeoms().size() != 0 )
    {
        Print( "ERROR: ClearVSPModel left Geoms behind" );
        __failure++;
    }

    ReadVSPFile( fname );

    // The Fuselage has to come back, and come back the same shape.
    array< string > @geoms = FindGeoms();

    if ( geoms.size() != 1 )
    {
        Print( "ERROR: ReadVSPFile did not restore the model" );
        __failure++;
    }
    else
    {
        if ( GetGeomTypeName( geoms[0] ) != "Fuselage" )
        {
            Print( "ERROR: ReadVSPFile restored the wrong Geom type" );
            __failure++;
        }

        if ( GetVSPFileName() != full_name )
        {
            Print( "ERROR: ReadVSPFile did not set the project file name" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    # A relative name is resolved against the working directory, so ask for the
    # resolved name rather than assuming it comes back verbatim.
    full_name = GetVSPFileName()

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )

    ClearVSPModel()

    assert len( FindGeoms() ) == 0, "ClearVSPModel left Geoms behind"

    ReadVSPFile( fname )

    # The Fuselage has to come back, and come back the same shape.
    geoms = FindGeoms()

    assert len( geoms ) == 1, "ReadVSPFile did not restore the model"
    assert GetGeomTypeName( geoms[0] ) == "Fuselage", "ReadVSPFile restored the wrong Geom type"
    assert GetVSPFileName() == full_name, "ReadVSPFile did not set the project file name"

    \endcode
    \endPythonOnly
    \param [in] file_name string \\*.vsp3 file name
    \param [in] set int Set index to write (i.e. SET_ALL)
*/

extern void WriteVSPFile( const std::string & file_name, int set = SET_ALL );

/*!
    \ingroup FileIO
*/
/*!
    Set the file name of a OpenVSP project
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    // A relative name is resolved against the working directory, so ask for the
    // resolved name rather than assuming it comes back verbatim.
    string full_name = GetVSPFileName();

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    if ( FindGeoms().size() != 0 )
    {
        Print( "ERROR: ClearVSPModel left Geoms behind" );
        __failure++;
    }

    ReadVSPFile( fname );

    // The Fuselage has to come back, and come back the same shape.
    array< string > @geoms = FindGeoms();

    if ( geoms.size() != 1 )
    {
        Print( "ERROR: ReadVSPFile did not restore the model" );
        __failure++;
    }
    else
    {
        if ( GetGeomTypeName( geoms[0] ) != "Fuselage" )
        {
            Print( "ERROR: ReadVSPFile restored the wrong Geom type" );
            __failure++;
        }

        if ( GetVSPFileName() != full_name )
        {
            Print( "ERROR: ReadVSPFile did not set the project file name" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    # A relative name is resolved against the working directory, so ask for the
    # resolved name rather than assuming it comes back verbatim.
    full_name = GetVSPFileName()

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )

    ClearVSPModel()

    assert len( FindGeoms() ) == 0, "ClearVSPModel left Geoms behind"

    ReadVSPFile( fname )

    # The Fuselage has to come back, and come back the same shape.
    geoms = FindGeoms()

    assert len( geoms ) == 1, "ReadVSPFile did not restore the model"
    assert GetGeomTypeName( geoms[0] ) == "Fuselage", "ReadVSPFile restored the wrong Geom type"
    assert GetVSPFileName() == full_name, "ReadVSPFile did not set the project file name"

    \endcode
    \endPythonOnly
    \param [in] file_name string File name
*/

extern void SetVSP3FileName( const std::string & file_name );

/*!
    \ingroup Vehicle
*/
/*!
    Get the file name of the current OpenVSP project
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string fname = "example_fuse.vsp3";

    SetVSP3FileName( fname );

    // A relative name is resolved against the working directory, so ask for the
    // resolved name rather than assuming it comes back verbatim.
    string full_name = GetVSPFileName();

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    if ( GetVSPFileName() != full_name || full_name.findLast( fname ) < 0 )
    {
        Print( "ERROR: GetVSPFileName did not report the name that was set" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    # A relative name is resolved against the working directory, so ask for the
    # resolved name rather than assuming it comes back verbatim.
    full_name = GetVSPFileName()

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    assert GetVSPFileName() == full_name and full_name.endswith( fname ), "GetVSPFileName did not report the name that was set"

    \endcode
    \endPythonOnly
    \return string File name for the current OpenVSP project
*/

extern std::string GetVSPFileName();

/*!
    \ingroup Vehicle
*/
/*!
    Clear the current OpenVSP model
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );
    ClearVSPModel();

    if ( FindGeoms().size() != 0 )
    {
        Print( "ERROR: ClearVSPModel left Geoms behind" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )
    ClearVSPModel()

    assert len( FindGeoms() ) == 0, "ClearVSPModel left Geoms behind"

    \endcode
    \endPythonOnly
*/

extern void ClearVSPModel();

/*!
    \ingroup FileIO
*/
/*!
    Insert an external OpenVSP project into the current project. All Geoms in the external project are placed as children of the specified parent.
    If no parent or an invalid parent is given, the Geoms are inserted at the top level.
    \param [in] file_name string \\*.vsp3 filename
    \param [in] parent_geom_id string Parent geom ID (ignored with empty string)
*/

extern void InsertVSPFile( const std::string & file_name, const std::string & parent_geom_id );


/*!
    \ingroup FileIO
*/
/*!
    Export a file from OpenVSP. Many formats are available, such as STL, IGES, and SVG. If a mesh is generated for a particular export,
    the ID of the MeshGeom will be returned. If no mesh is generated an empty string will be returned.
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING" );             // Add Wing

    ExportFile( "Airfoil_Metadata.csv", SET_ALL, EXPORT_SELIG_AIRFOIL );

    string mesh_id = ExportFile( "Example_Mesh.msh", SET_ALL, EXPORT_GMSH );
    if ( mesh_id.length() == 0 )
    {
        Print( "ERROR: ExportFile returned no id" );
        __failure++;
    }

    DeleteGeom( mesh_id ); // Delete the mesh generated by the GMSH export
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING" )             # Add Wing

    ExportFile( "Airfoil_Metadata.csv", SET_ALL, EXPORT_SELIG_AIRFOIL )

    mesh_id = ExportFile( "Example_Mesh.msh", SET_ALL, EXPORT_GMSH )
    assert len( mesh_id ) > 0, "ExportFile returned no id"

    DeleteGeom( mesh_id ) # Delete the mesh generated by the GMSH export

    \endcode
    \endPythonOnly
    \sa EXPORT_TYPE
    \param [in] file_name string Export file name
    \param [in] thick_set int Set index to export (i.e. SET_ALL)
    \param [in] file_type int File type enum (i.e. EXPORT_IGES)
    \param [in] subsFlag int Flag to tag subsurfaces if MeshGeom is created
    \param [in] thin_set int Set index to export as degenerate geometry (i.e. SET_NONE)
    \param [in] useMode bool Flag determine if mode is used instead of sets
    \param [in] modeID string ID of Mode to use
    \return string Mesh Geom ID if the export generates a mesh
*/

extern std::string ExportFile( const std::string & file_name, int thick_set, int file_type, int subsFlag = 1, int thin_set = vsp::SET_NONE, bool useMode = false, const string &modeID = "" );

/*!
    \ingroup FileIO
*/
/*!
    Import a file into OpenVSP. Many formats are available, such as NASCART, V2, and BEM). The imported Geom, mesh, or other object is inserted
    as a child of the specified parent. If no parent or an invalid parent is given, the import will be done at the top level.
    \sa IMPORT_TYPE
    \param [in] file_name string Import file name
    \param [in] file_type int File type enum (i.e. IMPORT_PTS)
    \param [in] parent string Parent Geom ID (ignored with empty string)
*/

extern std::string ImportFile( const std::string & file_name, int file_type, const std::string & parent );


/*!
    \ingroup FileIO
*/
/*!
    Set the ID of the propeller to be exported to a BEM file. Call this function before ExportFile.
    \forcpponly
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    SetBEMPropID( prop_id );

    ExportFile( "ExampleBEM.bem", SET_ALL, EXPORT_BEM );

    // A BEM export of a Geom that is not a propeller has to be rejected.
    string pod_id = AddGeom( "POD" );

    SetBEMPropID( pod_id );

    ExportFile( "ExampleBEM.bem", SET_ALL, EXPORT_BEM );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: BEM export accepted a Geom that is not a propeller" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    SetBEMPropID( prop_id )

    ExportFile( "ExampleBEM.bem", SET_ALL, EXPORT_BEM )

    # A BEM export of a Geom that is not a propeller has to be rejected.  The
    # error queue is reached through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    pod_id = AddGeom( "POD" )

    SetBEMPropID( pod_id )

    ExportFile( "ExampleBEM.bem", SET_ALL, EXPORT_BEM )

    assert err_mgr.GetNumTotalErrors() > 0, "BEM export accepted a Geom that is not a propeller"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \sa EXPORT_TYPE, ExportFile
    \param [in] prop_id string Propeller Geom ID
*/

extern void SetBEMPropID( const string & prop_id );


//======================== Design Files ================================//

/*!
    \ingroup DesignFile
*/
/*!
    Read in and apply a design file (\\*.des) to the current OpenVSP project
    \param [in] file_name string \\*.des input file
*/

extern void ReadApplyDESFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Write all design variables to a design file (\\*.des)
    \param [in] file_name string \\*.des output file
*/

extern void WriteDESFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Read in and apply a Cart3D XDDM file (\\*.xddm) to the current OpenVSP project
    \param [in] file_name string \\*.xddm input file
*/

extern void ReadApplyXDDMFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Write all design variables to a Cart3D XDDM file (\\*.xddm)
    \param [in] file_name string \\*.xddm output file
*/

extern void WriteXDDMFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Get the number of design variables
    \return int Number of design variables
*/

extern int GetNumDesignVars();

/*!
    \ingroup DesignFile
*/
/*!
    Add a design variable
    \sa XDDM_QUANTITY_TYPE
    \param [in] parm_id string Parm ID
    \param [in] type int XDDM type enum (XDDM_VAR or XDDM_CONST)
*/

extern void AddDesignVar( const std::string & parm_id, int type );

/*!
    \ingroup DesignFile
*/
/*!
    Delete all design variables
*/

extern void DeleteAllDesignVars();

/*!
    \ingroup DesignFile
*/
/*!
    Get the Parm ID of the specified design variable
    \param [in] index int Index of design variable
    \return string Parm ID
*/

extern std::string GetDesignVar( int index );

/*!
    \ingroup DesignFile
*/
/*!
    Get the XDDM type of the specified design variable
    \sa XDDM_QUANTITY_TYPE
    \param [in] index int Index of design variable
    \return int XDDM type enum (XDDM_VAR or XDDM_CONST)
*/

extern int GetDesignVarType( int index );


//======================== Computations ================================//
/*!
    \ingroup CFDMesh
*/
/*!
    Get the file name of a specified file type. Note, this function cannot be used to set FEA Mesh file names.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    //==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" );

    //==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE );

    // The degenerate representation is reported through the Results Manager as
    // well as written to file.  A Pod yields a surface, a plate and a stick.
    if ( GetNumResults( "DegenGeom" ) != 1 )
    {
        Print( "ERROR: ComputeDegenGeom produced no DegenGeom result" );
        __failure++;
    }

    if ( GetNumResults( "Degen_surf" ) < 1 || GetNumResults( "Degen_plate" ) < 1 || GetNumResults( "Degen_stick" ) < 1 )
    {
        Print( "ERROR: ComputeDegenGeom did not produce the component results" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    #==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" )

    #==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE )

    # The degenerate representation is reported through the Results Manager as
    # well as written to file.  A Pod yields a surface, a plate and a stick.
    assert GetNumResults( "DegenGeom" ) == 1, "ComputeDegenGeom produced no DegenGeom result"
    assert GetNumResults( "Degen_surf" ) >= 1, "ComputeDegenGeom produced no Degen_surf result"
    assert GetNumResults( "Degen_plate" ) >= 1, "ComputeDegenGeom produced no Degen_plate result"
    assert GetNumResults( "Degen_stick" ) >= 1, "ComputeDegenGeom produced no Degen_stick result"

    \endcode
    \endPythonOnly
    \sa COMPUTATION_FILE_TYPE, SetFeaMeshFileName
    \param [in] file_type int File type enum (i.e. CFD_TRI_TYPE, COMP_GEOM_TXT_TYPE)
    \param [in] file_name string File name
*/

extern void SetComputationFileName( int file_type, const std::string & file_name );

/*!
    \ingroup Computations
*/
/*!
    Compute mass properties for the components in the set. Alternatively can be run through the Analysis Manager with 'MassProp'.
    \forcpponly
    \code{.cpp}
    //==== Test Mass Props ====//
    string pid = AddGeom( "POD", "" );

    string mesh_id = ComputeMassProps( SET_ALL, 20, X_DIR );

    string mass_res_id = FindLatestResultsID( "Mass_Properties" );

    array<double> @double_arr = GetDoubleResults( mass_res_id, "Total_Mass" );

    if ( double_arr.size() != 1 )                                    { Print( "---> Error: API ComputeMassProps" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Test Mass Props ====//
    pid = AddGeom( "POD", "" )

    mesh_id = ComputeMassProps( SET_ALL, 20, X_DIR )

    mass_res_id = FindLatestResultsID( "Mass_Properties" )

    double_arr = GetDoubleResults( mass_res_id, "Total_Mass" )

    if  len(double_arr) != 1 :
        print( "---> Error: API ComputeMassProps" )
        assert False, "---> Error: API ComputeMassProps"

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis
    \param [in] set int Set index (i.e. SET_ALL)
    \param [in] num_slices int Number of slices
    \param [in] idir int Direction of slicing for integration
    \return string MeshGeom ID
*/

extern std::string ComputeMassProps( int set, int num_slices, int idir );

/*!
    \ingroup Computations
*/
/*!
    Mesh, intersect, and trim components in the set. Alternatively can be run through the Analysis Manager with 'CompGeom'.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    //==== Run CompGeom And Get Results ====//
    string mesh_id = ComputeCompGeom( SET_ALL, false, 0 );                      // Half Mesh false and no file export
    if ( mesh_id.length() == 0 )
    {
        Print( "ERROR: ComputeCompGeom returned no id" );
        __failure++;
    }


    string comp_res_id = FindLatestResultsID( "Comp_Geom" );                    // Find Results ID

    array<double> @double_arr = GetDoubleResults( comp_res_id, "Wet_Area" );    // Extract Results
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    #==== Run CompGeom And Get Results ====//
    mesh_id = ComputeCompGeom( SET_ALL, False, 0 )                      # Half Mesh false and no file export
    assert len( mesh_id ) > 0, "ComputeCompGeom returned no id"


    comp_res_id = FindLatestResultsID( "Comp_Geom" )                    # Find Results ID

    double_arr = GetDoubleResults( comp_res_id, "Wet_Area" )    # Extract Results

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis, COMPUTATION_FILE_TYPE
    \param [in] set int Set index (i.e. SET_ALL)
    \param [in] half_mesh bool Flag to ignore surfaces on the negative side of the XZ plane (e.g. symmetry)
    \param [in] file_export_types int CompGeom file type to export (supports XOR i.e. COMP_GEOM_CSV_TYPE & COMP_GEOM_TXT_TYPE )
    \return string MeshGeom ID
*/

extern std::string ComputeCompGeom( int set, bool half_mesh, int file_export_types );

/*!
    \ingroup Computations
*/
/*!
    Slice and mesh the components in the set. Alternatively can be run through the Analysis Manager with 'PlanarSlice'.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    //==== Test Plane Slice ====//
    string slice_mesh_id = ComputePlaneSlice( 0, 6, vec3d( 0.0, 0.0, 1.0 ), true );

    string pslice_results = FindLatestResultsID( "Slice" );

    array<double> @double_arr = GetDoubleResults( pslice_results, "Slice_Area" );

    if ( double_arr.size() != 6 )                                    { Print( "---> Error: API ComputePlaneSlice" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    #==== Test Plane Slice ====//
    slice_mesh_id = ComputePlaneSlice( 0, 6, vec3d( 0.0, 0.0, 1.0 ), True )

    pslice_results = FindLatestResultsID( "Slice" )

    double_arr = GetDoubleResults( pslice_results, "Slice_Area" )

    if  len(double_arr) != 6 :
        print( "---> Error: API ComputePlaneSlice" )
        assert False, "---> Error: API ComputePlaneSlice"

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis
    \param [in] set int Set index (i.e. SET_ALL)
    \param [in] num_slices int Number of slices
    \param [in] norm vec3d Normal axis for all slices
    \param [in] auto_bnd bool Flag to automatically set the start and end bound locations
    \param [in] start_bnd double Location of the first slice along the normal axis (default: 0.0)
    \param [in] end_bnd double Location of the last slice along the normal axis (default: 0.0)
    \param [in] measureduct bool Flag to measure negative area inside positive area (default: false)
    \return string MeshGeom ID
*/

extern std::string ComputePlaneSlice( int set, int num_slices, const vec3d & norm, bool auto_bnd,

                                 double start_bnd = 0, double end_bnd = 0, bool measureduct = false );
/*!
    \ingroup Computations
*/
/*!
    Compute the degenerate geometry representation for the components in the set. Alternatively can be run through the Analysis Manager with 'DegenGeom' or 'VSPAERODegenGeom'.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    //==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" );

    //==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE );

    // The degenerate representation is reported through the Results Manager as
    // well as written to file.  A Pod yields a surface, a plate and a stick.
    if ( GetNumResults( "DegenGeom" ) != 1 )
    {
        Print( "ERROR: ComputeDegenGeom produced no DegenGeom result" );
        __failure++;
    }

    if ( GetNumResults( "Degen_surf" ) < 1 || GetNumResults( "Degen_plate" ) < 1 || GetNumResults( "Degen_stick" ) < 1 )
    {
        Print( "ERROR: ComputeDegenGeom did not produce the component results" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    #==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" )

    #==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE )

    # The degenerate representation is reported through the Results Manager as
    # well as written to file.  A Pod yields a surface, a plate and a stick.
    assert GetNumResults( "DegenGeom" ) == 1, "ComputeDegenGeom produced no DegenGeom result"
    assert GetNumResults( "Degen_surf" ) >= 1, "ComputeDegenGeom produced no Degen_surf result"
    assert GetNumResults( "Degen_plate" ) >= 1, "ComputeDegenGeom produced no Degen_plate result"
    assert GetNumResults( "Degen_stick" ) >= 1, "ComputeDegenGeom produced no Degen_stick result"

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis, COMPUTATION_FILE_TYPE
    \param [in] set int Set index (i.e. SET_ALL)
    \param [in] file_export_types int DegenGeom file type to export (supports XOR i.e DEGEN_GEOM_M_TYPE & DEGEN_GEOM_CSV_TYPE)
*/

extern void ComputeDegenGeom( int set, int file_export_types );

/*!
    \ingroup CFDMesh
*/
/*!
    Create a CFD Mesh for the components in the set. This analysis cannot be run through the Analysis Manager.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    Update();

    //==== Keep the mesh coarse so the example runs quickly ====//
    SetCFDMeshVal( CFD_MAX_EDGE_LEN, 1.0 );
    SetCFDMeshVal( CFD_MIN_EDGE_LEN, 0.1 );

    //==== CFDMesh Method Facet Export =====//
    SetComputationFileName( CFD_FACET_TYPE, "TestCFDMeshFacet_API.facet" );
    SetComputationFileName( CFD_STL_TYPE, "TestCFDMesh_API.stl" );

   Print( "\tComputing CFDMesh..." );

    ComputeCFDMesh( SET_ALL, SET_NONE, CFD_FACET_TYPE | CFD_STL_TYPE );

    // CFD Mesh reports nothing through the Results Manager, so read the mesh it
    // just wrote back in to prove it produced one.
    string mesh_id = ImportFile( "TestCFDMesh_API.stl", IMPORT_STL, "" );

    if ( mesh_id.length() == 0 || GetGeomTypeName( mesh_id ) != "Mesh" )
    {
        Print( "ERROR: ComputeCFDMesh did not write a readable mesh" );
        __failure++;
    }
    else
    {
        DeleteGeom( mesh_id );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    Update()

    #==== Keep the mesh coarse so the example runs quickly ====//
    SetCFDMeshVal( CFD_MAX_EDGE_LEN, 1.0 )
    SetCFDMeshVal( CFD_MIN_EDGE_LEN, 0.1 )

    #==== CFDMesh Method Facet Export =====//
    SetComputationFileName( CFD_FACET_TYPE, "TestCFDMeshFacet_API.facet" )
    SetComputationFileName( CFD_STL_TYPE, "TestCFDMesh_API.stl" )

   print( "\tComputing CFDMesh..." )

    ComputeCFDMesh( SET_ALL, SET_NONE, CFD_FACET_TYPE | CFD_STL_TYPE )

    # CFD Mesh reports nothing through the Results Manager, so read the mesh it
    # just wrote back in to prove it produced one.
    mesh_id = ImportFile( "TestCFDMesh_API.stl", IMPORT_STL, "" )

    assert len( mesh_id ) > 0, "ComputeCFDMesh did not write a readable mesh"
    assert GetGeomTypeName( mesh_id ) == "Mesh", "ComputeCFDMesh did not write a readable mesh"

    DeleteGeom( mesh_id )

    \endcode
    \endPythonOnly
    \sa COMPUTATION_FILE_TYPE
    \param [in] set int Set index (i.e. SET_ALL)
    \param [in] degenset int DegenSet index (i.e. SET_NONE)
    \param [in] file_export_types int CFD Mesh file type to export (supports XOR i.e CFD_SRF_TYPE & CFD_STL_TYPE)
*/

extern void ComputeCFDMesh( int set, int degenset, int file_export_types );

/*!
    \ingroup CFDMesh
*/
/*!
    Set the value of a specific CFD Mesh option
    \forcpponly
    \code{.cpp}
    SetCFDMeshVal( CFD_MIN_EDGE_LEN, 1.0 );

    // The control types are backed by Parms in the CFD grid density container,
    // so the value that was set can be read back.
    string dens_id = FindContainer( "CFDGridDensity", 0 );

    string min_len_id = FindParm( dens_id, "MinLen", "CFDGridDensity" );

    if ( !closeTo( GetParmVal( min_len_id ), 1.0, 1e-12 ) )
    {
        Print( "ERROR: SetCFDMeshVal did not set CFD_MIN_EDGE_LEN" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetCFDMeshVal( CFD_MIN_EDGE_LEN, 1.0 )

    # The control types are backed by Parms in the CFD grid density container,
    # so the value that was set can be read back.
    dens_id = FindContainer( "CFDGridDensity", 0 )

    min_len_id = FindParm( dens_id, "MinLen", "CFDGridDensity" )

    assert abs( GetParmVal( min_len_id ) - 1.0 ) < 1e-12, "SetCFDMeshVal did not set CFD_MIN_EDGE_LEN"

    \endcode
    \endPythonOnly
    \sa CFD_CONTROL_TYPE
    \param [in] type int CFD Mesh control type enum (i.e. CFD_GROWTH_RATIO)
    \param [in] val double Value to set
*/

extern void SetCFDMeshVal( int type, double val );

/*!
    \ingroup CFDMesh
*/
/*!
    Activate or deactivate the CFD Mesh wake for a particular Geom. Note, the wake flag is only applicable for wing-type surfaces.
    Also, this function is simply an alternative to setting the value of the Parm with the available Parm setting API functions.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geom ====//
    string wid = AddGeom( "WING", "" );

    SetCFDWakeFlag( wid, true );

    if ( !closeTo( GetParmVal( wid, "Wake", "Shape" ), 1.0, 1e-12 ) )
    {
        Print( "ERROR: SetCFDWakeFlag did not activate the wake" );
        __failure++;
    }

    SetCFDWakeFlag( wid, false );

    if ( !closeTo( GetParmVal( wid, "Wake", "Shape" ), 0.0, 1e-12 ) )
    {
        Print( "ERROR: SetCFDWakeFlag did not deactivate the wake" );
        __failure++;
    }

    // This is equivalent to SetParmValUpdate( wid, "Wake", "Shape", 1.0 );
    // To change the scale: SetParmValUpdate( wid, "WakeScale", "WakeSettings", 10.0 );
    // To change the angle: SetParmValUpdate( wid, "WakeAngle", "WakeSettings", -5.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geom ====//
    wid = AddGeom( "WING", "" )

    SetCFDWakeFlag( wid, True )

    assert abs( GetParmVal( wid, "Wake", "Shape" ) - 1.0 ) < 1e-12, "SetCFDWakeFlag did not activate the wake"

    SetCFDWakeFlag( wid, False )

    assert abs( GetParmVal( wid, "Wake", "Shape" ) ) < 1e-12, "SetCFDWakeFlag did not deactivate the wake"

    # This is equivalent to SetParmValUpdate( wid, "Wake", "Shape", 1.0 )
    # To change the scale: SetParmValUpdate( wid, "WakeScale", "WakeSettings", 10.0 )
    # To change the angle: SetParmValUpdate( wid, "WakeAngle", "WakeSettings", -5.0 )

    \endcode
    \endPythonOnly
    \sa SetParmVal, SetParmValUpdate
    \param [in] geom_id string Geom ID
    \param [in] flag bool True to activate, false to deactivate
*/

extern void SetCFDWakeFlag( const std::string & geom_id, bool flag );

/*!
    \ingroup CFDMesh
*/
/*!
    Delete all CFD Mesh sources for all Geoms
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 );      // Add A Point Source

    DeleteAllCFDSources();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 )      # Add A Point Source

    DeleteAllCFDSources()

    \endcode
    \endPythonOnly
*/

extern void DeleteAllCFDSources();

/*!
    \ingroup CFDMesh
*/
/*!
    Add default CFD Mesh sources for all Geoms
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    AddDefaultSources(); // 3 Sources: Def_Fwd_PS, Def_Aft_PS, Def_Fwd_Aft_LS
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    AddDefaultSources() # 3 Sources: Def_Fwd_PS, Def_Aft_PS, Def_Fwd_Aft_LS

    \endcode
    \endPythonOnly
*/

extern void AddDefaultSources();

/*!
    \ingroup CFDMesh
*/
/*!
    Add a CFD Mesh default source for the indicated Geom. Note, certain input params may not be used depending on the source type
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 );      // Add A Point Source

    // Sources cannot be read back through the API, but a source attached to a
    // Geom that does not exist has to be rejected.
    AddCFDSource( POINT_SOURCE, "NOSUCHGEOM", 0, 0.25, 2.0, 0.5, 0.5 );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: AddCFDSource accepted a bad Geom ID" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 )      # Add A Point Source

    # Sources cannot be read back through the API, but a source attached to a
    # Geom that does not exist has to be rejected.  The error queue is reached
    # through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    AddCFDSource( POINT_SOURCE, "NOSUCHGEOM", 0, 0.25, 2.0, 0.5, 0.5 )

    assert err_mgr.GetNumTotalErrors() > 0, "AddCFDSource accepted a bad Geom ID"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \sa CFD_MESH_SOURCE_TYPE
    \param [in] type int CFD Mesh source type( i.e.BOX_SOURCE )
    \param [in] geom_id string Geom ID
    \param [in] surf_index int Main surface index
    \param [in] l1 double Source first edge length
    \param [in] r1 double Source first radius
    \param [in] u1 double Source first U location
    \param [in] w1 double Source first W location
    \param [in] l2 double Source second edge length
    \param [in] r2 double Source second radius
    \param [in] u2 double Source second U location
    \param [in] w2 double Source second W location
*/

extern void AddCFDSource( int type, const std::string & geom_id, int surf_index,

                          double l1, double r1, double u1, double w1,
                          double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );

/*!
    \ingroup VSPAERO
*/
/*!
    Get ID of the current VSPAERO reference Geom
    \return string Reference Geom ID
*/

extern string GetVSPAERORefWingID();

/*!
    \ingroup VSPAERO
*/
/*!
    Set the current VSPAERO reference Geom ID
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geom and set some parameters =====//
    string wing_id = AddGeom( "WING" );

    SetGeomName( wing_id, "MainWing" );

    //==== Add Vertical tail and set some parameters =====//
    string vert_id = AddGeom( "WING" );

    SetGeomName( vert_id, "Vert" );

    SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 );
    SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 );
    SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 );

    //==== Set VSPAERO Reference lengths & areas ====//
    SetVSPAERORefWingID( wing_id ); // Set as reference wing for VSPAERO

    Print( "VSPAERO Reference Wing ID: ", false );

    Print( GetVSPAERORefWingID() );

    if ( GetVSPAERORefWingID() != wing_id )
    {
        Print( "ERROR: SetVSPAERORefWingID did not take" );
        __failure++;
    }

    // Naming a Geom that does not exist has to be rejected.
    SetVSPAERORefWingID( "NOSUCHGEOM" );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: SetVSPAERORefWingID accepted a bad Geom ID" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geom and set some parameters =====//
    wing_id = AddGeom( "WING" )

    SetGeomName( wing_id, "MainWing" )

    #==== Add Vertical tail and set some parameters =====//
    vert_id = AddGeom( "WING" )

    SetGeomName( vert_id, "Vert" )

    SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 )
    SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 )
    SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 )

    #==== Set VSPAERO Reference lengths & areas ====//
    SetVSPAERORefWingID( wing_id ) # Set as reference wing for VSPAERO

    print( "VSPAERO Reference Wing ID: ", False )

    print( GetVSPAERORefWingID() )

    assert GetVSPAERORefWingID() == wing_id, "SetVSPAERORefWingID did not take"

    # Naming a Geom that does not exist has to be rejected.  The error queue is
    # reached through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    SetVSPAERORefWingID( "NOSUCHGEOM" )

    assert err_mgr.GetNumTotalErrors() > 0, "SetVSPAERORefWingID accepted a bad Geom ID"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] geom_id string Reference Geom ID
*/

extern string SetVSPAERORefWingID( const std::string & geom_id );


//======================== Analysis ================================//

/*!
    \ingroup Analysis
*/
/*!
    Get the number of analysis types available in the Analysis Manager
    \forcpponly
    \code{.cpp}
    int nanalysis = GetNumAnalysis();

    Print( "Number of registered analyses: " + nanalysis );

    // The count has to match the list of names.
    array< string > @analysis_array = ListAnalysis();

    if ( nanalysis != int( analysis_array.size() ) )
    {
        Print( "ERROR: GetNumAnalysis disagrees with ListAnalysis" );
        __failure++;
    }

    if ( nanalysis < 1 )
    {
        Print( "ERROR: no analyses registered" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    nanalysis = GetNumAnalysis()

    print( f"Number of registered analyses: {nanalysis}" )

    # The count has to match the list of names.
    analysis_array = ListAnalysis()

    assert nanalysis == len( analysis_array ), "GetNumAnalysis disagrees with ListAnalysis"
    assert nanalysis >= 1, "no analyses registered"

    \endcode
    \endPythonOnly
    \return int Number of analyses
*/

extern int GetNumAnalysis();

/*!
    \ingroup Analysis
*/
/*!
    Get the name of every available analysis in the Analysis Manager
    \forcpponly
    \code{.cpp}
    array< string > @analysis_array = ListAnalysis();

    Print( "List of Available Analyses: " );

    for ( int i = 0; i < int( analysis_array.size() ); i++ )
    {
        Print( "    " + analysis_array[i] );

        if ( analysis_array[i].length() == 0 )
        {
            Print( "ERROR: ListAnalysis returned an unnamed analysis" );
            __failure++;
        }
    }

    if ( int( analysis_array.size() ) != GetNumAnalysis() )
    {
        Print( "ERROR: ListAnalysis disagrees with GetNumAnalysis" );
        __failure++;
    }

    // The analyses the rest of these examples lean on have to be there.
    bool found = false;

    for ( int i = 0; i < int( analysis_array.size() ); i++ )
    {
        if ( analysis_array[i] == "VSPAEROComputeGeometry" )
        {
            found = true;
        }
    }

    if ( !found )
    {
        Print( "ERROR: VSPAEROComputeGeometry is not registered" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_array = ListAnalysis()

    print( "List of Available Analyses: " )

    for i in range(int( len(analysis_array) )):

        print( "    " + analysis_array[i] )

        assert len( analysis_array[i] ) > 0, "ListAnalysis returned an unnamed analysis"

    assert len( analysis_array ) == GetNumAnalysis(), "ListAnalysis disagrees with GetNumAnalysis"

    # The analyses the rest of these examples lean on have to be there.
    assert "VSPAEROComputeGeometry" in analysis_array, "VSPAEROComputeGeometry is not registered"

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of analysis names
*/

extern std::vector<std::string> ListAnalysis();

/*!
    \ingroup Analysis
*/
/*!
    Get the name of every available input for a particular analysis
    \forcpponly
    \code{.cpp}
    string analysis_name = "VSPAEROComputeGeometry";

    array<string>@ in_names =  GetAnalysisInputNames( analysis_name );
    if ( in_names.length() == 0 )
    {
        Print( "ERROR: GetAnalysisInputNames returned nothing" );
        __failure++;
    }

    Print("Analysis Inputs: ");

    for ( int i = 0; i < int( in_names.size() ); i++)
    {
        Print( ( "\t" + in_names[i] + "\n" ) );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_name = "VSPAEROComputeGeometry"

    in_names =  GetAnalysisInputNames( analysis_name )
    assert len( in_names ) > 0, "GetAnalysisInputNames returned nothing"

    print("Analysis Inputs: ")

    for i in range(int( len(in_names) )):

        print( ( "\t" + in_names[i] + "\n" ) )

    \endcode
    \endPythonOnly
    \param [in] analysis string Analysis name
    \return vector\<string\> Array of input names
*/

extern std::vector<std::string> GetAnalysisInputNames( const std::string & analysis );

/*!
    \ingroup Analysis
*/
/*!
    Get the analysis documentation string
    \forcpponly
    \code{.cpp}
    string analysis_name = "VSPAEROComputeGeometry";

    string doc = GetAnalysisDoc( analysis_name );
    if ( doc.length() == 0 )
    {
        Print( "ERROR: GetAnalysisDoc returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_name = "VSPAEROComputeGeometry"

    doc = GetAnalysisDoc( analysis_name )
    assert len( doc ) > 0, "GetAnalysisDoc returned nothing"

    \endcode
    \endPythonOnly
    \param [in] analysis string Analysis name
    \return string Documentation string
*/

extern std::string GetAnalysisDoc( const std::string & analysis );

/*!
    \ingroup Analysis
*/
/*!
    Get the documentation string for the particular analysis and input
    \forcpponly

    \endforcpponly
    \beginPythonOnly

    \endPythonOnly
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \return string Documentation string
*/

extern std::string GetAnalysisInputDoc( const std::string & analysis, const std::string & name );

/*!
    \ingroup Analysis
*/
/*!
    Execute an analysis through the Analysis Manager
    \forcpponly
    \code{.cpp}
    string analysis_name = "VSPAEROComputeGeometry";

    string res_id = ExecAnalysis( analysis_name );
    if ( res_id.length() == 0 )
    {
        Print( "ERROR: ExecAnalysis returned no id" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_name = "VSPAEROComputeGeometry"

    res_id = ExecAnalysis( analysis_name )
    assert len( res_id ) > 0, "ExecAnalysis returned no id"


    \endcode
    \endPythonOnly
    \param [in] analysis string Analysis name
    \return string Result ID
*/

extern std::string ExecAnalysis( const std::string & analysis );


/*!
    \ingroup Analysis
*/
/*!
    Get the documentation string for the particular analysis and input
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \return int Documentation string
*/

extern int GetNumAnalysisInputData( const std::string & analysis, const std::string & name );

/*!
    \ingroup Analysis
*/
/*!
    Get the data type for a particulat analysis type and input
    \forcpponly
    \code{.cpp}
    string analysis = "VSPAEROComputeGeometry";

    array < string > @ inp_array = GetAnalysisInputNames( analysis );

    if ( inp_array.size() == 0 )
    {
        Print( "ERROR: GetAnalysisInputNames returned nothing" );
        __failure++;
    }

    for ( int j = 0; j < int( inp_array.size() ); j++ )
    {
        int typ = GetAnalysisInputType( analysis, inp_array[j] );

        // Every input the analysis lists has to report a real data type.
        if ( typ == INVALID_TYPE )
        {
            Print( "ERROR: GetAnalysisInputType returned INVALID_TYPE for " + inp_array[j] );
            __failure++;
        }
    }

    // An input that does not exist has to report INVALID_TYPE.
    if ( GetAnalysisInputType( analysis, "NoSuchInput" ) != INVALID_TYPE )
    {
        Print( "ERROR: GetAnalysisInputType accepted an unknown input" );
        __failure++;
    }

    // That lookup failure was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis = "VSPAEROComputeGeometry"

    inp_array = GetAnalysisInputNames( analysis )

    assert len( inp_array ) > 0, "GetAnalysisInputNames returned nothing"

    for j in range(int( len(inp_array) )):

        typ = GetAnalysisInputType( analysis, inp_array[j] )

        # Every input the analysis lists has to report a real data type.
        assert typ != INVALID_TYPE, "GetAnalysisInputType returned INVALID_TYPE for " + inp_array[j]

    # An input that does not exist has to report INVALID_TYPE.
    assert GetAnalysisInputType( analysis, "NoSuchInput" ) == INVALID_TYPE, "GetAnalysisInputType accepted an unknown input"

    # That lookup failure was raised deliberately, so take it back off the queue.
    err_mgr = ErrorMgrSingleton.getInstance()

    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \return int Data type enum (i.e. DOUBLE_DATA)
*/

extern int GetAnalysisInputType( const std::string & analysis, const std::string & name );

/*!
    \ingroup Analysis
*/
/*!
    Get the current integer values for the particular analysis, input, and data index
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set to panel method
    array< int > thick_set = GetIntAnalysisInput( analysis_name, "GeomSet" );
    if ( thick_set.length() == 0 )
    {
        Print( "ERROR: GetIntAnalysisInput returned nothing" );
        __failure++;
    }
    array< int > thin_set = GetIntAnalysisInput( analysis_name, "ThinGeomSet" );

    thick_set[0] = ( SET_TYPE::SET_NONE );
    thin_set[0] = ( SET_TYPE::SET_ALL );

    SetIntAnalysisInput( analysis_name, "GeomSet", thick_set, 0);
    SetIntAnalysisInput( analysis_name, "ThinGeomSet", thin_set, 0);

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set to panel method
    thick_set = GetIntAnalysisInput( analysis_name, "GeomSet" )
    assert len( thick_set ) > 0, "GetIntAnalysisInput returned nothing"
    thin_set = GetIntAnalysisInput( analysis_name, "ThinGeomSet" )

    thick_set = [vsp.SET_NONE]
    thin_set = [vsp.SET_ALL]

    SetIntAnalysisInput( analysis_name, "GeomSet", thick_set )
    SetIntAnalysisInput( analysis_name, "ThinGeomSet", thin_set )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetIntAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector\<int\> Array of analysis input values
*/

extern const std::vector< int > & GetIntAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );

/*!
    \ingroup Analysis
*/
/*!
    Get the current double values for the particular analysis, input, and data index
    \forcpponly
    \code{.cpp}
    array<double> vinfFCinput = GetDoubleAnalysisInput( "ParasiteDrag", "Vinf" );
    if ( vinfFCinput.length() == 0 )
    {
        Print( "ERROR: GetDoubleAnalysisInput returned nothing" );
        __failure++;
    }

    vinfFCinput[0] = 629;

    SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", vinfFCinput );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    vinfFCinput = list( GetDoubleAnalysisInput( "ParasiteDrag", "Vinf" ) )

    vinfFCinput[0] = 629

    SetDoubleAnalysisInput( "ParasiteDrag", "Vinf", vinfFCinput )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetDoubleAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector\<double\> Array of analysis input values
*/

extern const std::vector< double > & GetDoubleAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );

/*!
    \ingroup Analysis
*/
/*!
    Get the current string values for the particular analysis, input, and data index
    \forcpponly
    \code{.cpp}
    array<string> fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" );
    if ( fileNameInput.length() == 0 )
    {
        Print( "ERROR: GetStringAnalysisInput returned nothing" );
        __failure++;
    }

    fileNameInput[0] = "ParasiteDragExample";

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" )
    assert len( fileNameInput ) > 0, "GetStringAnalysisInput returned nothing"

    fileNameInput = ["ParasiteDragExample"]

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetStringAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector\<string\> Array of analysis input values
*/

extern const std::vector<std::string> & GetStringAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );

/*!
    \ingroup Analysis
*/
/*!
    Get the current vec3d values for the particular analysis, input, and data index
    \forcpponly
    \code{.cpp}
    // PlanarSlice
    array<vec3d> norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" );
    if ( norm.length() == 0 )
    {
        Print( "ERROR: GetVec3dAnalysisInput returned nothing" );
        __failure++;
    }

    norm[0].set_xyz( 0.23, 0.6, 0.15 );

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # PlanarSlice
    norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" )
    assert len( norm ) > 0, "GetVec3dAnalysisInput returned nothing"

    norm[0].set_xyz( 0.23, 0.6, 0.15 )

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetVec3dAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector\<vec3d\> Array of analysis input values
*/

extern const std::vector< vec3d > & GetVec3dAnalysisInput( const std::string & analysis, const std::string & name, int index = 0 );


/*!
    \ingroup Analysis
*/
/*!
   Set all input values to their defaults for a specific analysis
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Change an input away from its default...
    array< int > geom_set = GetIntAnalysisInput( analysis_name, "GeomSet" );

    array< int > new_set = geom_set;
    new_set[0] = new_set[0] + 1;

    SetIntAnalysisInput( analysis_name, "GeomSet", new_set );

    array< int > check_set = GetIntAnalysisInput( analysis_name, "GeomSet" );

    if ( check_set[0] != new_set[0] )
    {
        Print( "ERROR: SetIntAnalysisInput did not take" );
        __failure++;
    }

    // ...and set defaults, which has to put it back.
    SetAnalysisInputDefaults( analysis_name );

    check_set = GetIntAnalysisInput( analysis_name, "GeomSet" );

    if ( check_set[0] != geom_set[0] )
    {
        Print( "ERROR: SetAnalysisInputDefaults did not restore the default" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Change an input away from its default...
    geom_set = GetIntAnalysisInput( analysis_name, "GeomSet" )

    new_set = [ geom_set[0] + 1 ]

    SetIntAnalysisInput( analysis_name, "GeomSet", new_set )

    check_set = GetIntAnalysisInput( analysis_name, "GeomSet" )

    assert check_set[0] == new_set[0], "SetIntAnalysisInput did not take"

    # ...and set defaults, which has to put it back.
    SetAnalysisInputDefaults( analysis_name )

    check_set = GetIntAnalysisInput( analysis_name, "GeomSet" )

    assert check_set[0] == geom_set[0], "SetAnalysisInputDefaults did not restore the default"

    \endcode
    \endPythonOnly
    \param [in] analysis string Analysis name
*/

extern void SetAnalysisInputDefaults( const std::string & analysis );

/*!
    \ingroup Analysis
*/
/*!
    Set the value of a particular analysis input of integer type
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set to panel method
    array< int > thick_set = GetIntAnalysisInput( analysis_name, "GeomSet" );
    array< int > thin_set = GetIntAnalysisInput( analysis_name, "ThinGeomSet" );

    thick_set[0] = ( SET_TYPE::SET_NONE );
    thin_set[0] = ( SET_TYPE::SET_ALL );

    SetIntAnalysisInput( analysis_name, "GeomSet", thick_set, 0);
    if ( GetIntAnalysisInput( analysis_name, "GeomSet", 0 ) != thick_set )
    {
        Print( "ERROR: SetIntAnalysisInput did not take" );
        __failure++;
    }

    SetIntAnalysisInput( analysis_name, "ThinGeomSet", thin_set, 0);

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set to panel method
    thick_set = GetIntAnalysisInput( analysis_name, "GeomSet" )
    thin_set = GetIntAnalysisInput( analysis_name, "ThinGeomSet" )

    thick_set = [vsp.SET_NONE]
    thin_set = [vsp.SET_ALL]

    SetIntAnalysisInput( analysis_name, "GeomSet", thick_set )
    assert list( GetIntAnalysisInput( analysis_name, "GeomSet" ) ) == list( thick_set ), "SetIntAnalysisInput did not take"

    SetIntAnalysisInput( analysis_name, "ThinGeomSet", thin_set )

    \endcode
    \endPythonOnly
    \sa GetIntAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] indata vector<int> Array of integer values to set the input to
    \param [in] index int Data index
*/

extern void SetIntAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< int > & indata, int index = 0 );

/*!
    \ingroup Analysis
*/
/*!
    Set the value of a particular analysis input of double type
    \forcpponly
    \code{.cpp}
    //==== Analysis: CpSlicer ====//
    string analysis_name = "CpSlicer";

    // Setup cuts
    array < double > ycuts;
    ycuts.push_back( 2.0 );
    ycuts.push_back( 4.5 );
    ycuts.push_back( 8.0 );

    SetDoubleAnalysisInput( analysis_name, "YSlicePosVec", ycuts, 0 );
    if ( GetDoubleAnalysisInput( analysis_name, "YSlicePosVec", 0 ) != ycuts )
    {
        Print( "ERROR: SetDoubleAnalysisInput did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: CpSlicer ====//
    analysis_name = "CpSlicer"

    # Setup cuts
    ycuts = []
    ycuts.append( 2.0 )
    ycuts.append( 4.5 )
    ycuts.append( 8.0 )

    SetDoubleAnalysisInput( analysis_name, "YSlicePosVec", ycuts, 0 )
    assert list( GetDoubleAnalysisInput( analysis_name, "YSlicePosVec", 0 ) ) == list( ycuts ), "SetDoubleAnalysisInput did not take"


    \endcode
    \endPythonOnly
    \sa GetDoubleAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] indata vector<double> Array of double values to set the input to
    \param [in] index int Data index
*/

extern void SetDoubleAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< double > & indata, int index = 0 );

/*!
    \ingroup Analysis
*/
/*!
    Set the value of a particular analysis input of string type
    \forcpponly
    \code{.cpp}
    array<string> fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" );

    fileNameInput[0] = "ParasiteDragExample";

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput );
    if ( GetStringAnalysisInput( "ParasiteDrag", "FileName" ) != fileNameInput )
    {
        Print( "ERROR: SetStringAnalysisInput did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" )

    fileNameInput = ["ParasiteDragExample"]

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput )
    assert list( GetStringAnalysisInput( "ParasiteDrag", "FileName" ) ) == list( fileNameInput ), "SetStringAnalysisInput did not take"


    \endcode
    \endPythonOnly
    \sa GetStringAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] indata vector<string> Array of string values to set the input to
    \param [in] index int Data index
*/

extern void SetStringAnalysisInput( const std::string & analysis, const std::string & name, const std::vector<std::string> & indata, int index = 0 );

/*!
    \ingroup Analysis
*/
/*!
    Set the value of a particular analysis input of vec3d type
    \forcpponly
    \code{.cpp}
    // PlanarSlice
    array<vec3d> norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" );

    norm[0].set_xyz( 0.23, 0.6, 0.15 );

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm );
    if ( GetVec3dAnalysisInput( "PlanarSlice", "Norm" ) != norm )
    {
        Print( "ERROR: SetVec3dAnalysisInput did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # PlanarSlice
    norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" )

    norm[0].set_xyz( 0.23, 0.6, 0.15 )

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm )
    # The Python binding does not expose vec3d equality, so compare components.
    norm_back = GetVec3dAnalysisInput( "PlanarSlice", "Norm" )
    assert len( norm_back ) == len( norm ), "SetVec3dAnalysisInput length"
    for i in range( len( norm ) ):
        assert abs( norm_back[i].x() - norm[i].x() ) < 1e-9, "SetVec3dAnalysisInput x"
        assert abs( norm_back[i].y() - norm[i].y() ) < 1e-9, "SetVec3dAnalysisInput y"
        assert abs( norm_back[i].z() - norm[i].z() ) < 1e-9, "SetVec3dAnalysisInput z"


    \endcode
    \endPythonOnly
    \sa GetVec3dAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] indata vector<vec3d> Array of vec3d values to set the input to
    \param [in] index int Data index
*/

extern void SetVec3dAnalysisInput( const std::string & analysis, const std::string & name, const std::vector< vec3d > & indata, int index = 0 );


/*!
    \ingroup Analysis
*/
/*!
    Print to stdout all current input values for a specific analysis
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // list inputs, type, and current values
    PrintAnalysisInputs( analysis_name );

    // There has to be something to list.
    array< string > @inp_array = GetAnalysisInputNames( analysis_name );

    if ( inp_array.size() == 0 )
    {
        Print( "ERROR: the analysis reports no inputs to print" );
        __failure++;
    }

    // Printing an analysis that does not exist has to be rejected.
    PrintAnalysisInputs( "NoSuchAnalysis" );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: PrintAnalysisInputs accepted an unknown analysis" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # list inputs, type, and current values
    PrintAnalysisInputs( analysis_name )

    # There has to be something to list.
    inp_array = GetAnalysisInputNames( analysis_name )

    assert len( inp_array ) > 0, "the analysis reports no inputs to print"

    # Printing an analysis that does not exist has to be rejected.  The error
    # queue is reached through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    PrintAnalysisInputs( "NoSuchAnalysis" )

    assert err_mgr.GetNumTotalErrors() > 0, "PrintAnalysisInputs accepted an unknown analysis"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] analysis_name string Name of analysis
*/

extern void PrintAnalysisInputs( const std::string & analysis_name );

/*!
    \ingroup Analysis
*/
/*!
    Print to stdout all current input documentation for a specific analysis
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // list inputs, type, and documentation
    PrintAnalysisDocs( analysis_name );

    // The analysis itself has to carry documentation to print.
    if ( GetAnalysisDoc( analysis_name ).length() == 0 )
    {
        Print( "ERROR: the analysis carries no documentation" );
        __failure++;
    }

    // Printing an analysis that does not exist has to be rejected.
    PrintAnalysisDocs( "NoSuchAnalysis" );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: PrintAnalysisDocs accepted an unknown analysis" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # list inputs, type, and documentation
    PrintAnalysisDocs( analysis_name )

    # The analysis itself has to carry documentation to print.
    assert len( GetAnalysisDoc( analysis_name ) ) > 0, "the analysis carries no documentation"

    # Printing an analysis that does not exist has to be rejected.  The error
    # queue is reached through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    PrintAnalysisDocs( "NoSuchAnalysis" )

    assert err_mgr.GetNumTotalErrors() > 0, "PrintAnalysisDocs accepted an unknown analysis"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] analysis_name string Name of analysis
*/

extern void PrintAnalysisDocs( const std::string & analysis_name );

/*!
    \ingroup Analysis
*/
/*!
    Add a new Geometry Analysis case to the Geometry Analysis Manager. The Geometry Analysis Manager
    allows users to configure and execute geometric analysis tasks typical of the aircraft design process,
    such as wetted area, projected area, mass properties, clearance, and visibility analyses.
    \forcpponly
    \code{.cpp}
    //==== GeometryAnalysis: Add and configure a case ====//
    string ga_id = AddGeometryAnalysis();
    Print( "Added Geometry Analysis: ", false );
    Print( ga_id );

    if ( ga_id.length() == 0 || ga_id == "NONE" )
    {
        Print( "ERROR: AddGeometryAnalysis returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== GeometryAnalysis: Add and configure a case ====##
    ga_id = AddGeometryAnalysis()
    print( "Added Geometry Analysis: ", ga_id )
    \endcode
    \endPythonOnly
    \sa DeleteGeometryAnalysis, DeleteAllGeometryAnalyses, GetAllGeometryAnalysesIDVec
    \return string ID of the newly created Geometry Analysis case
*/

extern string AddGeometryAnalysis();

/*!
    \ingroup Analysis
*/
/*!
    Delete a specific Geometry Analysis case from the Geometry Analysis Manager.
    \forcpponly
    \code{.cpp}
    //==== GeometryAnalysis: Delete a specific case ====//
    string ga_id = AddGeometryAnalysis();

    if ( GetAllGeometryAnalysesIDVec().size() != 1 )
    {
        Print( "ERROR: AddGeometryAnalysis did not add a case" );
        __failure++;
    }

    DeleteGeometryAnalysis( ga_id );

    if ( GetAllGeometryAnalysesIDVec().size() != 0 )
    {
        Print( "ERROR: DeleteGeometryAnalysis did not remove the case" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== GeometryAnalysis: Delete a specific case ====##
    ga_id = AddGeometryAnalysis()

    assert len( GetAllGeometryAnalysesIDVec() ) == 1, "AddGeometryAnalysis did not add a case"

    DeleteGeometryAnalysis( ga_id )

    assert len( GetAllGeometryAnalysesIDVec() ) == 0, "DeleteGeometryAnalysis did not remove the case"

    \endcode
    \endPythonOnly
    \sa AddGeometryAnalysis, DeleteAllGeometryAnalyses, GetAllGeometryAnalysesIDVec
    \param [in] id string ID of the Geometry Analysis case to delete
*/

extern void DeleteGeometryAnalysis( const string &id );

/*!
    \ingroup Analysis
*/
/*!
    Delete all Geometry Analysis cases from the Geometry Analysis Manager.
    \forcpponly
    \code{.cpp}
    //==== GeometryAnalysis: Delete all cases ====//
    string ga_id_1 = AddGeometryAnalysis();
    string ga_id_2 = AddGeometryAnalysis();

    if ( ga_id_1 == ga_id_2 )
    {
        Print( "ERROR: AddGeometryAnalysis reused an ID" );
        __failure++;
    }

    if ( GetAllGeometryAnalysesIDVec().size() != 2 )
    {
        Print( "ERROR: the two cases were not both added" );
        __failure++;
    }

    DeleteAllGeometryAnalyses();
    array < string > @ga_ids = GetAllGeometryAnalysesIDVec();
    Print( "Number of Geometry Analyses after delete: ", false );
    Print( ga_ids.size() );

    if ( ga_ids.size() != 0 )
    {
        Print( "ERROR: DeleteAllGeometryAnalyses left cases behind" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== GeometryAnalysis: Delete all cases ====##
    ga_id_1 = AddGeometryAnalysis()
    ga_id_2 = AddGeometryAnalysis()

    assert ga_id_1 != ga_id_2, "AddGeometryAnalysis reused an ID"
    assert len( GetAllGeometryAnalysesIDVec() ) == 2, "the two cases were not both added"

    DeleteAllGeometryAnalyses()
    ga_ids = GetAllGeometryAnalysesIDVec()
    print( "Number of Geometry Analyses after delete: ", len( ga_ids ) )

    assert len( ga_ids ) == 0, "DeleteAllGeometryAnalyses left cases behind"
    \endcode
    \endPythonOnly
    \sa AddGeometryAnalysis, DeleteGeometryAnalysis, GetAllGeometryAnalysesIDVec
*/

extern void DeleteAllGeometryAnalyses();

/*!
    \ingroup Analysis
*/
/*!
    Get a vector of all Geometry Analysis case IDs in the Geometry Analysis Manager.
    \forcpponly
    \code{.cpp}
    //==== GeometryAnalysis: List all cases ====//
    string ga_id_1 = AddGeometryAnalysis();
    string ga_id_2 = AddGeometryAnalysis();
    array < string > @ga_ids = GetAllGeometryAnalysesIDVec();
    if ( ga_ids.length() == 0 )
    {
        Print( "ERROR: GetAllGeometryAnalysesIDVec returned nothing" );
        __failure++;
    }
    for ( int i = 0; i < int( ga_ids.size() ); i++ )
    {
        Print( "Geometry Analysis ID: ", false );
        Print( ga_ids[i] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== GeometryAnalysis: List all cases ====##
    ga_id_1 = AddGeometryAnalysis()
    ga_id_2 = AddGeometryAnalysis()
    ga_ids = GetAllGeometryAnalysesIDVec()
    assert len( ga_ids ) > 0, "GetAllGeometryAnalysesIDVec returned nothing"
    for ga_id in ga_ids:
        print( "Geometry Analysis ID: ", ga_id )
    \endcode
    \endPythonOnly
    \sa AddGeometryAnalysis, DeleteGeometryAnalysis, DeleteAllGeometryAnalyses
    \return vector\<string\> Array of all Geometry Analysis case IDs
*/

extern vector < string > GetAllGeometryAnalysesIDVec();

/*!
    \ingroup Analysis
*/
/*!
    Set the active Geometry Analysis case.  The active case is the one drawn in the
    Geometry Analysis GUI (and captured by ScreenGrab).  Setting it does not require the
    GUI to be present.
    \forcpponly
    \code{.cpp}
    string ga_id = AddGeometryAnalysis();
    SetActiveGeometryAnalysis( ga_id );
    if ( GetActiveGeometryAnalysis() != ga_id )
    {
        Print( "ERROR: SetActiveGeometryAnalysis did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ga_id = AddGeometryAnalysis()
    SetActiveGeometryAnalysis( ga_id )
    assert GetActiveGeometryAnalysis() == ga_id, "SetActiveGeometryAnalysis did not take"

    \endcode
    \endPythonOnly
    \sa GetActiveGeometryAnalysis, AddGeometryAnalysis
    \param [in] id string Geometry Analysis case ID (empty string for none)
*/

extern void SetActiveGeometryAnalysis( const string &id );

/*!
    \ingroup Analysis
*/
/*!
    Get the ID of the active Geometry Analysis case.
    \forcpponly
    \code{.cpp}
    string ga_id = AddGeometryAnalysis();
    SetActiveGeometryAnalysis( ga_id );
    string active = GetActiveGeometryAnalysis();
    if ( active.length() == 0 )
    {
        Print( "ERROR: GetActiveGeometryAnalysis returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ga_id = AddGeometryAnalysis()
    SetActiveGeometryAnalysis( ga_id )
    active = GetActiveGeometryAnalysis()
    assert len( active ) > 0, "GetActiveGeometryAnalysis returned nothing"
    \endcode
    \endPythonOnly
    \sa SetActiveGeometryAnalysis
    \return string Active Geometry Analysis case ID (empty string if none)
*/

extern string GetActiveGeometryAnalysis();

/*!
    \ingroup Analysis
*/
/*!
    Create a MeshGeom from the results of the specified Geometry Analysis case and return the new Geom's ID.
    The case must have been evaluated first (run the "GeometryAnalysis" analysis for the case) so that its
    result meshes exist.  The resulting MeshGeom is added to the model and made the active Geom.
    \forcpponly
    \code{.cpp}
    //==== Add a Geom for the case to work on ====//
    string pod_id = AddGeom( "POD" );
    Update();

    string ga_id = AddGeometryAnalysis();

    //==== Configure the case.  Without a primary target the analysis has
    //==== nothing to mesh and reports an empty primary mesh.
    SetParmVal( FindParm( ga_id, "PrimaryType", "InterferenceCase" ), SET_TARGET );
    SetParmVal( FindParm( ga_id, "PrimarySet", "InterferenceCase" ), SET_ALL );
    SetParmVal( FindParm( ga_id, "SecondaryType", "InterferenceCase" ), SET_TARGET );
    SetParmVal( FindParm( ga_id, "SecondarySet", "InterferenceCase" ), SET_ALL );
    Update();

    // Evaluate the case through the Analysis framework so that its result meshes exist.
    SetAnalysisInputDefaults( "GeometryAnalysis" );

    array<string> case_input = { ga_id };
    SetStringAnalysisInput( "GeometryAnalysis", "CaseID", case_input );

    ExecAnalysis( "GeometryAnalysis" );

    // Turn the case's result meshes into a MeshGeom.
    string mesh_id = MakeMeshGeom( ga_id );
    if ( mesh_id.length() == 0 )
    {
        Print( "ERROR: MakeMeshGeom returned no id" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add a Geom for the case to work on ====//
    pod_id = AddGeom( "POD" )
    Update()

    ga_id = AddGeometryAnalysis()

    #==== Configure the case.  Without a primary target the analysis has
    #==== nothing to mesh and reports an empty primary mesh.
    SetParmVal( FindParm( ga_id, "PrimaryType", "InterferenceCase" ), SET_TARGET )
    SetParmVal( FindParm( ga_id, "PrimarySet", "InterferenceCase" ), SET_ALL )
    SetParmVal( FindParm( ga_id, "SecondaryType", "InterferenceCase" ), SET_TARGET )
    SetParmVal( FindParm( ga_id, "SecondarySet", "InterferenceCase" ), SET_ALL )
    Update()

    # Evaluate the case through the Analysis framework so that its result meshes exist.
    SetAnalysisInputDefaults( "GeometryAnalysis" )

    SetStringAnalysisInput( "GeometryAnalysis", "CaseID", [ga_id] )

    ExecAnalysis( "GeometryAnalysis" )

    # Turn the case's result meshes into a MeshGeom.
    mesh_id = MakeMeshGeom( ga_id )
    assert len( mesh_id ) > 0, "MakeMeshGeom returned no id"

    \endcode
    \endPythonOnly
    \sa AddGeometryAnalysis, SetActiveGeometryAnalysis
    \param [in] ga_id string Geometry Analysis case ID
    \return string Geom ID of the newly created MeshGeom
*/

extern string MakeMeshGeom( const string &ga_id );

//======================== Attributes ================================//

/*!
    \ingroup Attributes
*/
/*!
    Print a tab-delimited summary of all Attributes in the vehicle, denoting Name, Type, Data, Description, and path from Root of vehicle to Attribute
    \forcpponly
    \code{.cpp}
    //==== Attributes: SummarizeAttributes ====//
    string SummaryText = SummarizeAttributes();
    if ( SummaryText.length() == 0 )
    {
        Print( "ERROR: SummarizeAttributes returned no id" );
        __failure++;
    }

    Print( SummaryText );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: SummarizeAttributes ====##
    SummaryText = SummarizeAttributes()
    assert len( SummaryText ) > 0, "SummarizeAttributes returned no id"

    print( SummaryText )

    \endcode
    \endPythonOnly
    \return string Tab-delimited summary of all Attributes in vehicle
*/

extern string SummarizeAttributes();

/*!
    \ingroup Attributes
*/
/*!
    Print a plain-text tree summary of all Attribute in the vehicle, each branch node showing the name and ID of the VSP object in the path to the attribute
    \forcpponly
    \code{.cpp}
    //==== Attributes: SummarizeAttributesAsTree ====//
    string SummaryTextTree = SummarizeAttributesAsTree();
    if ( SummaryTextTree.length() == 0 )
    {
        Print( "ERROR: SummarizeAttributesAsTree returned no id" );
        __failure++;
    }

    Print( SummaryTextTree );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: SummarizeAttributesAsTree ====##
    SummaryTextTree = SummarizeAttributesAsTree();
    assert len( SummaryTextTree ) > 0, "SummarizeAttributesAsTree returned no id"

    print( SummaryTextTree )

    \endcode
    \endPythonOnly
    \return string Plain-text attribute tree of vehicle
*/

extern string SummarizeAttributesAsTree();

/*!
    \ingroup Attributes
*/
/*!
    Returns a vector of string IDs for all Attributes in the vehicle
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAllAttributes ====//
    array < string > @AttrIDs = FindAllAttributes();
    if ( AttrIDs.length() == 0 )
    {
        Print( "ERROR: FindAllAttributes found nothing" );
        __failure++;
    }
    for ( int i = 0; i < int( AttrIDs.size() ); ++i )
    {
        Print( AttrIDs[i] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAllAttributes ====##
    AttrIDs = FindAllAttributes()
    assert len( AttrIDs ) > 0, "FindAllAttributes found nothing"
    for AttrID in AttrIDs:
        print( AttrID )

    \endcode
    \endPythonOnly
    \return vector \<string\> Vector of All Attribute IDs
*/

extern vector < string > FindAllAttributes();



/*!
    \ingroup Attributes
*/
/*!
    Returns all attributes that contain the string search_str within their name, case insensitive
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAttributesByName ====//
    array < string > @AttrIDs = FindAttributesByName( "Watermark" );
    if ( AttrIDs.length() == 0 )
    {
        Print( "ERROR: FindAttributesByName found nothing" );
        __failure++;
    }
    for ( int i = 0; i < int( AttrIDs.size() ); ++i )
    {
        Print( AttrIDs[i] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAttributesByName ====##
    AttrIDs = FindAttributesByName( "Watermark" )
    assert len( AttrIDs ) > 0, "FindAttributesByName found nothing"
    for AttrID in AttrIDs:
        print( AttrID )

    \endcode
    \endPythonOnly
    \return vector \<string\> Vector of string IDs of matching Attributes
    \param [in] search_str string for filtering attributes in model
*/

extern vector < string > FindAttributesByName( const string & search_str );

/*!
    \ingroup Attributes
*/
/*!
    Searches all attributes that contain the search string, case insensitive, and returns the user-specified index
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAttributeByName ====//
    string AttrID = FindAttributeByName( "Watermark", 0 );
    if ( AttrID.length() == 0 )
    {
        Print( "ERROR: FindAttributeByName found nothing" );
        __failure++;
    }
    Print( AttrID );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAttributeByName ====##
    AttrID = FindAttributeByName( "Watermark", 0 )
    assert len( AttrID ) > 0, "FindAttributeByName found nothing"
    print( AttrID )

    \endcode
    \endPythonOnly
    \return string Returns a StringID of the attribute indexed/searched by user, if found
    \param [in] search_str string for filtering attributes in model
    \param [in] index int for indexing which of the vector of found attributes to select
*/

extern string FindAttributeByName( const string & search_str, int index );

/*!
    \ingroup Attributes
*/
/*!
    Searches all attributes in an OpenVSP object or AttributeCollection that contain the search string, case insensitive, and returns the user-specified index.
    Works either with the ID of an object that contains an attributeCollection or just the ID of an attributeCollection.
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAttributeInCollection ====//
    string VehID = GetVehicleID();
    string AttrID = FindAttributeInCollection( VehID, "Watermark", 0 );
    if ( AttrID.length() == 0 )
    {
        Print( "ERROR: FindAttributeInCollection found nothing" );
        __failure++;
    }
    Print( AttrID );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAttributeInCollection ====##
    VehID = GetVehicleID()
    AttrID = FindAttributeInCollection( VehID, 'Watermark', 0 )
    assert len( AttrID ) > 0, "FindAttributeInCollection found nothing"
    print( AttrID )

    \endcode
    \endPythonOnly
    \return string Returns a StringID of the attribute indexed/searched by user, if found
    \param [in] obj_id string id of object to search within for attributes
    \param [in] search_str string for filtering attributes in object
    \param [in] index int for indexing which of the vector of found attributes to select
*/

extern string FindAttributeInCollection( const string & obj_id, const string & search_str, int index );

/*!
    \ingroup Attributes
*/
/*!
    Return a list of all attribute Names within an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAttributeNamesInCollection ====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    array < string > @AttrNames = FindAttributeNamesInCollection( CollID );
    if ( AttrNames.length() == 0 )
    {
        Print( "ERROR: FindAttributeNamesInCollection found nothing" );
        __failure++;
    }
    for ( int i = 0; i < int( AttrNames.size() ); ++i )
    {
        Print( AttrNames[i] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAttributeNamesInCollection ====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrNames = FindAttributeNamesInCollection( CollID )
    assert len( AttrNames ) > 0, "FindAttributeNamesInCollection found nothing"
    for AttrName in AttrNames:
        print( AttrName )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of result names
    \param [in] collID string ID of an attribute collection
*/

extern vector< string > FindAttributeNamesInCollection(const string & collID );

/*!
    \ingroup Attributes
*/
/*!
    Get all attribute IDs within a single AttributeCollection, referenced by collID
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAttributesInCollection ====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    array < string > @AttrIDs = FindAttributesInCollection( CollID );
    if ( AttrIDs.length() == 0 )
    {
        Print( "ERROR: FindAttributesInCollection found nothing" );
        __failure++;
    }
    for ( int i = 0; i < int( AttrIDs.size() ); ++i )
    {
        Print( AttrIDs[i] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAttributesInCollection ====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrIDs = FindAttributesInCollection( CollID )
    assert len( AttrIDs ) > 0, "FindAttributesInCollection found nothing"
    for AttrID in AttrIDs:
        print( AttrID )

    \endcode
    \endPythonOnly
    \return vector\<string\> Vector of attribute IDs in an attribute collection.
    \param [in] collID string ID of an attribute collection
*/

extern vector< string > FindAttributesInCollection(const string & collID );

/*!
    \ingroup Attributes
*/
/*!
    Get array of IDs of all OpenVSP entities that have populated attributeCollections
    Includes attributeGroups
    \forcpponly
    \code{.cpp}
    //==== Attributes: FindAttributedObjects ====//
    array < string > @AttachIDs = FindAttributedObjects();
    if ( AttachIDs.length() == 0 )
    {
        Print( "ERROR: FindAttributedObjects found nothing" );
        __failure++;
    }
    for ( int i = 0; i < int( AttachIDs.size() ); ++i )
    {
        Print( AttachIDs[i] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: FindAttributedObjects ====##
    AttachIDs = FindAttributedObjects()
    assert len( AttachIDs ) > 0, "FindAttributedObjects found nothing"
    for AttachID in AttachIDs:
        print( AttachID )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of IDs of entities in OpenVSP that contain populated attribute collections
*/

extern vector< string > FindAttributedObjects();

/*!
    \ingroup Attributes
*/
/*!
    Get the type of an OpenVSP Entity by ID
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetObjectType ====//
    array < string > @AttachIDs = FindAttributedObjects();

    if ( AttachIDs.length() == 0 )
    {
        Print( "ERROR: FindAttributedObjects found nothing" );
        __failure++;
    }

    for ( int i = 0; i < int( AttachIDs.size() ); ++i )
    {
        int ObjType = GetObjectType( AttachIDs[i] );
        Print( ObjType );

        // Every object that carries attributes has to be a recognized kind, and
        // the enum has to agree with the name form.
        if ( ObjType == ATTROBJ_FREE )
        {
            Print( "ERROR: GetObjectType did not recognize " + AttachIDs[i] );
            __failure++;
        }

        if ( GetObjectTypeName( AttachIDs[i] ).length() == 0 )
        {
            Print( "ERROR: GetObjectTypeName returned nothing for " + AttachIDs[i] );
            __failure++;
        }
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetObjectType ====##
    AttachIDs = FindAttributedObjects()

    assert len( AttachIDs ) > 0, "FindAttributedObjects found nothing"

    for AttachID in AttachIDs:
        ObjType = GetObjectType( AttachID )
        print( ObjType )

        # Every object that carries attributes has to be a recognized kind, and
        # the enum has to agree with the name form.
        assert ObjType != ATTROBJ_FREE, "GetObjectType did not recognize " + AttachID
        assert len( GetObjectTypeName( AttachID ) ) > 0, "GetObjectTypeName returned nothing for " + AttachID

    \endcode
    \endPythonOnly
    \return int return string of object name
    \param [in] attachID string ID of an OpenVSP object
*/

extern int GetObjectType(const string & attachID);

/*!
    \ingroup Attributes
*/
/*!
    Get the named type of an OpenVSP Entity by ID
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetObjectTypeName ====//
    array < string > @AttachIDs = FindAttributedObjects();
    for ( int i = 0; i < int( AttachIDs.size() ); ++i )
    {
        string ObjTypeName = GetObjectTypeName( AttachIDs[i] );
        if ( ObjTypeName.length() == 0 )
        {
            Print( "ERROR: GetObjectTypeName returned nothing" );
            __failure++;
        }
        Print( ObjTypeName );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    AttachIDs = FindAttributedObjects()
    for AttachID in AttachIDs:
        ObjTypeName = GetObjectTypeName( AttachID )
        assert len( ObjTypeName ) > 0, "GetObjectTypeName returned nothing"
        print( ObjTypeName )

    \endcode
    \endPythonOnly
    \return string return string of object name
    \param [in] attachID string ID of an OpenVSP object
*/

extern string GetObjectTypeName(const string & attachID);

/*!
    \ingroup Attributes
*/
/*!
    Get the name of an OpenVSP Entity by ID
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetObjectName ====//
    array < string > @AttachIDs = FindAttributedObjects();
    for ( int i = 0; i < int( AttachIDs.size() ); ++i )
    {
        string ObjName = GetObjectName( AttachIDs[i] );
        if ( ObjName.length() == 0 )
        {
            Print( "ERROR: GetObjectName returned nothing" );
            __failure++;
        }
        Print( ObjName );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetObjectName ====##
    AttachIDs = FindAttributedObjects()
    for AttachID in AttachIDs:
        ObjName = GetObjectName( AttachID )
        assert len( ObjName ) > 0, "GetObjectName returned nothing"
        print( ObjName )

    \endcode
    \endPythonOnly
    \return string return string of object name
    \param [in] attachID string ID of an OpenVSP object
*/

extern string GetObjectName(const string & attachID);


/*!
    \ingroup Attributes
*/
/*!
    Get the string ID of the entity's parent
    Attributes -> Attribute Collections
    Attribute Collections -> Objects that contain attribute Collections
    Geoms->Parent Geoms
    Parms->ParmContainers
    etc.
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetObjectParent ====//

    string WingID = AddGeom( "WING" );
    string PodID = AddGeom( "POD", WingID );
    string ParentID = GetObjectParent( PodID );
    if ( ParentID.length() == 0 )
    {
        Print( "ERROR: GetObjectParent returned nothing" );
        __failure++;
    }

    if ( ParentID == WingID )
    {
        Print( "Parent of Pod is Wing");
    }

    // Get first attribute in vehicle as an example
    array < string > @AttrIDs = FindAllAttributes();
    string AttrID = AttrIDs[0];
    string CollID = GetObjectParent( AttrID );
    string CollParentObjID = GetObjectParent( CollID );
    Print( CollParentObjID );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetObjectParent ====##

    WingID = AddGeom( "WING" )
    PodID = AddGeom( "POD", WingID )
    ParentID = GetObjectParent( PodID )
    assert len( ParentID ) > 0, "GetObjectParent returned nothing"

    if ParentID == WingID:
        print( "Parent of Pod is Wing")

    #Get first attribute in vehicle as an example
    AttrID = FindAllAttributes()[0]
    CollID = GetObjectParent( AttrID )
    CollParentObjID = GetObjectParent( CollID )
    print( CollParentObjID )

    \endcode
    \endPythonOnly
    \return string ID of object parent
*/

extern string GetObjectParent( const string & id );

/*!
    \ingroup Attributes
*/
/*!
    Get collection ID from any OpenVSP object
    If ID is an attribute collection, return the same ID back
    If ID is an attribute group, return its nested collection
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetChildCollection =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    if ( CollID.length() == 0 )
    {
        Print( "ERROR: GetChildCollection returned nothing" );
        __failure++;
    }
    Print( CollID );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetChildCollection =====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    assert len( CollID ) > 0, "GetChildCollection returned nothing"
    print( CollID )


    \endcode
    \endPythonOnly
    \return string String ID of attribute collection associated with the attachID
    \param [in] attachID string ID of an OpenVSP object
*/

extern string GetChildCollection(const string & attachID );

/*!
    \ingroup Attributes
*/
/*!
    Get collection ID from a vehicle's GeomSet
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetGeomSetCollection =====//
    string CollID = GetGeomSetCollection( 0 );
    if ( CollID.length() == 0 )
    {
        Print( "ERROR: GetGeomSetCollection returned nothing" );
        __failure++;
    }
    Print( CollID );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetGeomSetCollection =====##
    CollID = GetGeomSetCollection( 0 )
    assert len( CollID ) > 0, "GetGeomSetCollection returned nothing"
    print( CollID )

    \endcode
    \endPythonOnly
    \return string String ID of attribute collection associated with the geom set
    \param [in] index int Geom set
*/

extern string GetGeomSetCollection( const int & index );

/*!
    \ingroup Attributes
*/
/*!
    Return the name of an attribute by its ID
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetAttributeName =====//

    array < string > @AttrIDs = FindAllAttributes();

    for ( int i = 0; i < int( AttrIDs.size() ); ++i )
    {
        string AttrName = GetAttributeName( AttrIDs[i] );
        if ( AttrName.length() == 0 )
        {
            Print( "ERROR: GetAttributeName returned nothing" );
            __failure++;
        }
        Print( AttrName );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetAttributeName =====##

    AttrIDs = FindAllAttributes()

    for AttrID in AttrIDs:
        AttrName = GetAttributeName( AttrID )
        assert len( AttrName ) > 0, "GetAttributeName returned nothing"
        print( AttrName )

    \endcode
    \endPythonOnly
    \param [in] attrID string ID of an attribute
    \return string name of attribute
*/

extern string GetAttributeName( const string & attrID );

/*!
    \ingroup Attributes
*/
/*!
    Return the ID of an attribute by its name and collection ID
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetAttributeID =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    array < string > @AttrNames = FindAttributeNamesInCollection( CollID );
    for ( int i = 0; i < int( AttrNames.size() ); ++i )
    {
        string AttrID = GetAttributeID( CollID, AttrNames[i], 0 );
        if ( AttrID.length() == 0 )
        {
            Print( "ERROR: GetAttributeID returned nothing" );
            __failure++;
        }
        Print( AttrID );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetAttributeID =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrNames = FindAttributeNamesInCollection( CollID )
    for AttrName in AttrNames:
        AttrID = GetAttributeID( CollID, AttrName, 0 )
        assert len( AttrID ) > 0, "GetAttributeID returned nothing"
        print( AttrID )


    \endcode
    \endPythonOnly
    \return string String ID of attribute based on collectionID and name
    \param [in] collID string ID of an attribute collection
    \param [in] attributeName string name of an attribute in that collection
    \param [in] index int index of attribute in collection
*/

extern string GetAttributeID(const string & collID, const string & attributeName, int index);

/*!
    \ingroup Attributes
*/
/*!
    Return string doc of attribute by its ID
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetAttributeDoc =====//
    array < string > @AttrIDs = FindAllAttributes();
    string AttrID = AttrIDs[0];
    string AttrDoc = GetAttributeDoc( AttrID );
    if ( AttrDoc.length() == 0 )
    {
        Print( "ERROR: GetAttributeDoc returned nothing" );
        __failure++;
    }
    Print( AttrDoc );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetAttributeDoc =====##
    AttrID = FindAllAttributes()[0]
    AttrDoc = GetAttributeDoc(AttrID)
    assert len( AttrDoc ) > 0, "GetAttributeDoc returned nothing"
    print( AttrDoc )

    \endcode
    \endPythonOnly
    \return string Return string doc of attribute by its ID
    \param [in] attrID string ID of attribute
*/

extern string GetAttributeDoc(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get int enum type of attribute by ID
    Use in conjunction with GetAttributeTypeName for getting strings or with the following enums

        BOOL_DATA
        INT_DATA
        DOUBLE_DATA
        STRING_DATA
        VEC3D_DATA
        INT_MATRIX_DATA
        DOUBLE_MATRIX_DATA
        NAMEVAL_COLLECTION_DATA
        ATTR_COLLECTION_DATA

    \forcpponly
    \code{.cpp}
    //==== Attributes: GetAttributeType =====//
    array < string > @AttrIDs = FindAllAttributes();

    if ( AttrIDs.length() == 0 )
    {
        Print( "ERROR: FindAllAttributes found nothing" );
        __failure++;
    }

    string AttrID = AttrIDs[0];
    int AttrType = GetAttributeType( AttrID );
    Print( AttrType );

    // The attribute has to report a real type, and the enum has to agree with
    // the name form.
    if ( AttrType == INVALID_TYPE )
    {
        Print( "ERROR: GetAttributeType returned INVALID_TYPE" );
        __failure++;
    }

    if ( GetAttributeTypeName( AttrID ).length() == 0 )
    {
        Print( "ERROR: GetAttributeTypeName returned nothing" );
        __failure++;
    }

    // An ID that is not an attribute has to report INVALID_TYPE.
    if ( GetAttributeType( "NOSUCHATTRIBUTE" ) != INVALID_TYPE )
    {
        Print( "ERROR: GetAttributeType accepted a bad ID" );
        __failure++;
    }

    // That lookup failure was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetAttributeType =====##
    AttrIDs = FindAllAttributes()

    assert len( AttrIDs ) > 0, "FindAllAttributes found nothing"

    AttrID = AttrIDs[0]
    AttrType = GetAttributeType( AttrID )
    print( AttrType )

    # The attribute has to report a real type, and the enum has to agree with
    # the name form.
    assert AttrType != INVALID_TYPE, "GetAttributeType returned INVALID_TYPE"
    assert len( GetAttributeTypeName( AttrID ) ) > 0, "GetAttributeTypeName returned nothing"

    # An ID that is not an attribute has to report INVALID_TYPE.
    assert GetAttributeType( "NOSUCHATTRIBUTE" ) == INVALID_TYPE, "GetAttributeType accepted a bad ID"

    # That lookup failure was raised deliberately, so take it back off the queue.
    err_mgr = ErrorMgrSingleton.getInstance()

    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \return int Int type of attribute
    \param [in] attrID string ID of attribute
*/

extern int GetAttributeType( const string & attrID );

/*!
    \ingroup Attributes
*/
/*!
    Get the attribute's type as a string
    \forcpponly
    \code{.cpp}
    //==== Attributes: GetAttributeTypeName =====//
    array < string > @AttrIDs = FindAllAttributes();
    string AttrID = AttrIDs[0];
    string AttrTypeName = GetAttributeTypeName( AttrID );
    if ( AttrTypeName.length() == 0 )
    {
        Print( "ERROR: GetAttributeTypeName returned nothing" );
        __failure++;
    }
    Print( AttrTypeName );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attributes: GetAttributeTypeName =====##
    AttrID = FindAllAttributes()[0]
    AttributeTypeName = GetAttributeTypeName( AttrID )
    assert len( AttributeTypeName ) > 0, "GetAttributeTypeName returned nothing"
    print( AttributeTypeName )


    \endcode
    \endPythonOnly
    \return string Type of attribute as string
    \param [in] attrID string ID of attribute
*/

extern string GetAttributeTypeName(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the boolean value of a bool-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeBoolVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    bool InitVal = true;
    string AttrID = AddAttributeBool( CollID, "TestBoolAttr", InitVal );

    array < bool > @GetVal = GetAttributeBoolVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Got matching Bool Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeBoolVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeBoolVal  =====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = True
    AttrID = AddAttributeBool( CollID, "TestBoolAttr", InitVal )

    GetVal = GetAttributeBoolVal( AttrID )
    if GetVal[0] == InitVal:
        print( "Got matching Bool Value from Attribute" )
    else:
        print( "GetAttributeBoolVal error!" )
        assert False, "GetAttributeBoolVal error!"

    \endcode
    \endPythonOnly
    \return vector\<int\> Bool value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector< int > GetAttributeBoolVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the integer value of an int-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeIntVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    int InitVal = 55;
    string AttrID = AddAttributeInt( CollID, "TestIntAttr", InitVal );

    array < int > @GetVal = GetAttributeIntVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Got matching Int Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeIntVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeIntVal  =====//
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = 55
    AttrID = AddAttributeInt( CollID, "TestIntAttr", InitVal )

    GetVal = GetAttributeIntVal( AttrID )
    if GetVal[0] == InitVal:
        print( "Got matching Int Value from Attribute" )
    else:
        print( "GetAttributeIntVal error!" )
        assert False, "GetAttributeIntVal error!"

    \endcode
    \endPythonOnly
    \return vector\<int\> Int value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector< int > GetAttributeIntVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the double value of a double-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeDoubleVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    double InitVal = 3.14159;
    string AttrID = AddAttributeDouble( CollID, "TestDoubleAttr", InitVal );

    array < double > @GetVal = GetAttributeDoubleVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Got matching Double Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeDoubleVal error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeDoubleVal  =====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = 3.14159
    AttrID = AddAttributeDouble( CollID, "TestDoubleAttr", InitVal )

    GetVal = GetAttributeDoubleVal( AttrID )
    if GetVal[0] == InitVal:
        print( "Got matching Double Value from Attribute" )
    else:
        print( "GetAttributeDoubleVal error!" )
        assert False, "GetAttributeDoubleVal error!"

    \endcode
    \endPythonOnly
    \return vector\<double\> Double value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector< double > GetAttributeDoubleVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the string value of a string-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeStringVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string InitVal = "Hello_World_of_Attributes";
    string AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal );

    array < string > @GetVal = GetAttributeStringVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Got matching String Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeStringVal error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeStringVal  =====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = "Hello_World_of_Attributes"
    AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal )

    GetVal = GetAttributeStringVal( AttrID )
    if GetVal[0] == InitVal:
        print( "Got matching String Value from Attribute" )
    else:
        print( "GetAttributeStringVal error!" )
        assert False, "GetAttributeStringVal error!"

    \endcode
    \endPythonOnly
    \return vector\<string\> String value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector< string > GetAttributeStringVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the parm value of a parm-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeParmID  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );

    string PodID = AddGeom( "POD", "" );
    Print( "---> Test Get Parm Val" );
    array < string > @ParmArray = GetGeomParmIDs( PodID );

    string ParmID = ParmArray[0];
    string AttrID = AddAttributeParm( CollID, "TestParmAttr", ParmID );

    array < string > @GetID = GetAttributeParmID( AttrID );

    if ( GetID[0] == ParmID )
    {
        Print( "Got matching Parm ID from Attribute" );
    }
    else
    {
        Print( "GetAttributeParmID error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeParmID  =====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )

    PodID = AddGeom( "POD", "" )
    print( "---> Test Get Parm Val" )
    ParmArray = GetGeomParmIDs( PodID )

    ParmID = ParmArray[0]
    AttrID = AddAttributeParm( CollID, "TestParmAttr", ParmID )

    GetID = GetAttributeParmID( AttrID )

    if GetID[0] == ParmID:
        print( "Got matching Parm ID from Attribute" )
    else:
        print( "GetAttributeParmID error!" )
        assert False, "GetAttributeParmID error!"

    \endcode
    \endPythonOnly
    \return vector\<string\> Parm value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector< string > GetAttributeParmID(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the parm value of a parm-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeParmVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );

    string PodID = AddGeom( "POD", "" );
    Print( "---> Test Get Parm Val" );
    array < string > @ParmArray = GetGeomParmIDs( PodID );

    string ParmID = ParmArray[0];
    string AttrID = AddAttributeParm( CollID, "TestParmAttr", ParmID );
    
    double InitVal = GetParmVal( ParmID );
    array < double > @GetVal = GetAttributeParmVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Got matching Parm Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeParmVal error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeParmVal  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )

    PodID = AddGeom( "POD", "" )
    print( "---> Test Get Parm Val" )
    ParmArray = GetGeomParmIDs( PodID )

    ParmID = ParmArray[0]
    AttrID = AddAttributeParm( CollID, "TestParmAttr", ParmID )

    InitVal = GetParmVal( ParmID )
    GetVal = GetAttributeParmVal( AttrID )

    if GetVal[0] == InitVal:
        print( "Got matching Parm Value from Attribute" )
    else:
        print( "GetAttributeParmVal error!" )
        assert False, "GetAttributeParmVal error!"


    \endcode
    \endPythonOnly
    \return vector \<double\> Parm value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector < double > GetAttributeParmVal( const string &attrID );

/*!
    \ingroup Attributes
*/
/*!
    Get the name of the referenced parm of a parm-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeParmName  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );

    string PodID = AddGeom( "POD", "" );
    Print( "---> Test Get Parm Val" );
    array < string > @ParmArray = GetGeomParmIDs( PodID );
    string AttrName = "Example_Parm_Attr";
    string ParmID = ParmArray[0];

    AddAttributeParm( CollID, AttrName, ParmID );
    string AttrID = GetAttributeID( CollID, AttrName, 0 );
    string ParmName = GetAttributeParmName( AttrID )[0];

    if ( ParmName == GetParmName( ParmID ) )
    {
        Print( "Got matching Parm Name from Attribute" );
    }
    else
    {
        Print( "GetAttributeParmName error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeParmName  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    PodID = AddGeom( "POD", "" )
    print( "---> Test Get Parm Val" )
    ParmArray = GetGeomParmIDs( PodID )
    AttrName = 'Example_Parm_Attr'
    ParmID = ParmArray[0]
    AddAttributeParm( CollID, AttrName, ParmID )
    AttrID = GetAttributeID( CollID, AttrName, 0 )
    ParmName = GetAttributeParmName( AttrID )[0]
    if ParmName == GetParmName( ParmID ):
        print( "Got matching Parm Name from Attribute" )
    else:
        print( "GetAttributeParmName error!" )
        assert False, "GetAttributeParmName error!"


    \endcode
    \endPythonOnly
    \return vector \<string\> Parm name of attribute
    \param [in] attrID string ID of attribute
*/

extern vector < string > GetAttributeParmName( const string &attrID );

/*!
    \ingroup Attributes
*/
/*!
    Get the vec3d value of a string-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeVec3dVal  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );

    vec3d InitVal = vec3d( 1., 0.5, -4. );
    string AttrID = AddAttributeVec3d( CollID, "TestVec3dAttr", {InitVal} );

    array < vec3d > @Vec3dVal = GetAttributeVec3dVal( AttrID );
    if ( Vec3dVal[0].x() == InitVal.x() and Vec3dVal[0].y() == InitVal.y() and Vec3dVal[0].z() == InitVal.z() )
    {
        Print( "Got matching Vec3d Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeVec3dVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeVec3dVal  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = vec3d([1., 0.5, -4.])
    AttrID = AddAttributeVec3d( CollID, "TestVec3dAttr", [InitVal] )

    Vec3dVal = GetAttributeVec3dVal( AttrID )
    if ( Vec3dVal[0].x() == InitVal.x() ) and ( Vec3dVal[0].y() == InitVal.y() ) and ( Vec3dVal[0].z() == InitVal.z() ):
        print( "Got matching Vec3d Value from Attribute" )
    else:
        print( "GetAttributeVec3dVal error!" )
        assert False, "GetAttributeVec3dVal error!"

    \endcode
    \endPythonOnly
    \return vector\<vec3d\> Vec3d value of attribute
    \param [in] attrID string ID of attribute
*/

extern vector< vec3d > GetAttributeVec3dVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the Int Matrix of an Int-matrix-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeIntMatrixVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );

    array < array < int > > InitVal = { {0, 1} , {-4, -1000} };
    string AttrID = AddAttributeIntMatrix( CollID, "TestIntMatrixAttr", InitVal );

    array < array < int > > IntMatrixVal = GetAttributeIntMatrixVal( AttrID );

    // can also get object handle to the int array with an @ handle declaration!
    array < array < int > > @IntMatrixValJHandle = GetAttributeIntMatrixVal( AttrID );

    if ( IntMatrixVal == InitVal )
    {
        Print( "Got matching IntMatrix Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeIntMatrixVal error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeIntMatrixVal  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = [[0, 1,],[-4, -1000]]
    AttrID = AddAttributeIntMatrix( CollID, "TestIntMatrixAttr", InitVal )

    IntMatrixVal = GetAttributeIntMatrixVal( AttrID )
    IntMatrixVal = [list(row) for row in IntMatrixVal]

    if IntMatrixVal == InitVal:
        print( "Got matching IntMatrix Value from Attribute" )
    else:
        print( "GetAttributeIntMatrixVal error!" )
        assert False, "GetAttributeIntMatrixVal error!"


    \endcode
    \endPythonOnly
    \return vector\<vector <int\>> Int Matrix value of attribute as vector < vector < int > >
    \param [in] attrID string ID of attribute
*/

extern vector< vector < int > > GetAttributeIntMatrixVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Get the Double Matrix of an Double-matrix-type attribute
    \forcpponly
    \code{.cpp}
    //==== Attribute: GetAttributeDoubleMatrixVal  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    array < array < double > > InitVal = {{0., 1.},{-4., -1000.}};
    string AttrID = AddAttributeDoubleMatrix( CollID, "TestDoubleMatrixAttr", InitVal );

    array < array < double > > DblMatrixVal = GetAttributeDoubleMatrixVal( AttrID );

    if ( DblMatrixVal == InitVal )
    {
        Print( "Got matching DoubleMatrix Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeDoubleMatrixVal error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: GetAttributeDoubleMatrixVal  =====##
    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = [[0., 1.,],[-4., -1000.]]
    AttrID = AddAttributeDoubleMatrix( CollID, "TestDoubleMatrixAttr", InitVal )

    DblMatrixVal = GetAttributeDoubleMatrixVal( AttrID )
    DblMatrixVal = [list(row) for row in DblMatrixVal]

    if DblMatrixVal == InitVal:
        print( "Got matching Double Matrix Value from Attribute" )
    else:
        print( "GetAttributeDoubleMatrixVal error!" )
        assert False, "GetAttributeDoubleMatrixVal error!"

    \endcode
    \endPythonOnly
    \return vector\<vector <double\>> Double Matrix value of attribute as vector < vector < Double > >
    \param [in] attrID string ID of attribute
*/

extern vector< vector < double > > GetAttributeDoubleMatrixVal(const string & attrID);

/*!
    \ingroup Attributes
*/
/*!
    Set the name of an Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeName  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string InitVal = "Hello_World_of_Attributes";
    string AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal );

    string NameString = "NewName_Example";
    SetAttributeName( AttrID, NameString );
    string AttrName = GetAttributeName( AttrID );
    if ( NameString == AttrName )
    {
        Print( "Got matching name from Attribute" );
    }
    else
    {
        Print( "SetAttributeName error!" );
        __failure++;
    }

    //==== Write Some Fake Test Results =====//
    // not implemented
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeName  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = "Hello_World_of_Attributes"
    AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal )

    NameString = 'NewName_Example'
    SetAttributeName( AttrID, NameString )
    AttrName = GetAttributeName( AttrID )
    if NameString == AttrName:
        print( "Got matching name from Attribute")
    else:
        print( "SetAttributeName error!" )
        assert False, "SetAttributeName error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] name string name for attribute
*/

extern void SetAttributeName( const string & attrID, const string & name );

/*!
    \ingroup Attributes
*/
/*!
    Set the docstring of an Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeDoc  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string InitVal = "Hello_World_of_Attributes";
    string AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal );

    string DocString = "New_docstring_for_attribute";

    SetAttributeDoc( AttrID, DocString );
    string NewDocString = GetAttributeDoc( AttrID );

    if ( NewDocString == DocString )
    {
        Print( "Got matching DocString from Attribute" );
    }
    else
    {
        Print( "SetAttributeDoc error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeDoc  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = "Hello_World_of_Attributes"
    AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal )

    DocString = 'New_docstring_for_attribute'

    SetAttributeDoc( AttrID, DocString )
    NewDocString = GetAttributeDoc( AttrID )
    if NewDocString == DocString:
        print( "Got matching DocString from Attribute")
    else:
        print( "SetAttributeDoc error!" )
        assert False, "SetAttributeDoc error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] doc string of documentation for attribute
*/


extern void SetAttributeDoc( const string & attrID, const string & doc );

/*!
    \ingroup Attributes
*/
/*!
    Set the Bool value of a bool-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeBool  =====//
    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    bool InitVal = true;
    string AttrID = AddAttributeBool( CollID, "TestBoolAttr", InitVal );

    bool SetVal = false;
    SetAttributeBool( AttrID, SetVal );

    array < bool > @GetVal = GetAttributeBoolVal( AttrID );
    if ( GetVal[0] == SetVal )
    {
        Print( "Set matching Bool Value from Attribute" );
    }
    else
    {
        Print( "SetAttributeBoolVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeBool  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = True
    AttrID = AddAttributeBool( CollID, "TestBoolAttr", InitVal )

    SetVal = False
    SetAttributeBool( AttrID, SetVal )

    GetVal = GetAttributeBoolVal( AttrID )
    if GetVal[0] == SetVal:
        print( "Set matching Bool Value from Attribute" )
    else:
        print( "SetAttributeBoolVal error!" )
        assert False, "SetAttributeBoolVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value bool boolean value for attribute
*/

extern void SetAttributeBool( const string & attrID, bool value );

/*!
    \ingroup Attributes
*/
/*!
    Set the Int value of an int-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeInt  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    int InitVal = 55;
    string AttrID = AddAttributeInt( CollID, "TestIntAttr", InitVal );

    int NewIntVal = -55;

    SetAttributeInt( AttrID, NewIntVal );
    array < int > @GetVal = GetAttributeIntVal( AttrID );
    if ( GetVal[0] == NewIntVal )
    {
        Print( "Set matching Int Value from Attribute" );
    }
    else
    {
        Print( "SetAttributeIntVal error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeInt  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = 55
    AttrID = AddAttributeInt( CollID, "TestIntAttr", InitVal )

    NewIntVal = -55

    SetAttributeInt( AttrID, NewIntVal )
    GetVal = GetAttributeIntVal( AttrID )
    if GetVal[0] == NewIntVal:
        print( "Set matching Int Value from Attribute" )
    else:
        print( "SetAttributeIntVal error!" )
        assert False, "SetAttributeIntVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value int value for attribute
*/

extern void SetAttributeInt( const string & attrID, int value );

/*!
    \ingroup Attributes
*/
/*!
    Set the Double value of a double-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeDouble  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    double InitVal = 3.14159;
    string AttrID = AddAttributeDouble( CollID, "TestDoubleAttr", InitVal );

    double DoubleVal = 3.15;

    SetAttributeDouble( AttrID, DoubleVal );

    array < double > @GetVal = GetAttributeDoubleVal( AttrID );
    if ( GetVal[0] == DoubleVal )
    {
        Print( "Set matching Double Value from Attribute" );
    }
    else
    {
        Print( "SetAttributeDoubleVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeDouble  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = 3.14159
    AttrID = AddAttributeDouble( CollID, "TestDoubleAttr", InitVal )

    DoubleVal = 3.15

    SetAttributeDouble( AttrID, DoubleVal )

    GetVal = GetAttributeDoubleVal( AttrID )
    if GetVal[0] == DoubleVal:
        print( "Set matching Double Value from Attribute" )
    else:
        print( "SetAttributeDoubleVal error!" )
        assert False, "SetAttributeDoubleVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value double value for attribute
*/

extern void SetAttributeDouble( const string & attrID, double value );

/*!
    \ingroup Attributes
*/
/*!
    Set the String value of a string-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeString  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string InitVal = "Hello_World_of_Attributes";
    string AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal );

    string StringVal = "Du bist supergeil!";
    SetAttributeString( AttrID, StringVal );

    array < string > @GetVal = GetAttributeStringVal( AttrID );
    if ( GetVal[0] == StringVal )
    {
        Print( "Got matching String Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeStringVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeString  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = "Hello_World_of_Attributes"
    AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal )

    StringVal = "Du bist supergeil!"
    SetAttributeString( AttrID, StringVal )

    GetVal = GetAttributeStringVal( AttrID )
    if GetVal[0] == StringVal:
        print( "Got matching String Value from Attribute" )
    else:
        print( "GetAttributeStringVal error!" )
        assert False, "GetAttributeStringVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value string value for attribute
*/

extern void SetAttributeString( const string & attrID, const string & value );

/*!
    \ingroup Attributes
*/
/*!
    Set the ParmID value of a Parm-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeParmID  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );

    string PodID = AddGeom( "POD", "" );
    Print( "---> Test Get Parm Val" );
    array < string > @ParmArray = GetGeomParmIDs( PodID );

    string ParmID = ParmArray[0];
    string AttrID = AddAttributeParm( CollID, "TestParmAttr", ParmID );

    string NewParmID = ParmArray[1];
    SetAttributeParmID( AttrID, NewParmID );
    array < string > @GetID = GetAttributeParmID( AttrID );

    if ( GetID[0] == NewParmID )
    {
        Print( "Set matching Parm ID from Attribute" );
    }
    else
    {
        Print( "SetAttributeParmID error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeParmID  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )

    PodID = AddGeom( "POD", "" )
    print( "---> Test Get Parm Val" )
    ParmArray = GetGeomParmIDs( PodID )

    ParmID = ParmArray[0]
    AttrID = AddAttributeParm( CollID, "TestParmAttr", ParmID )

    NewParmID = ParmArray[1]
    SetAttributeParmID( AttrID, NewParmID )
    GetID = GetAttributeParmID( AttrID )

    if GetID[0] == NewParmID:
        print( "Set matching Parm ID from Attribute" )
    else:
        print( "SetAttributeParmID error!" )
        assert False, "SetAttributeParmID error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value string value for attribute
*/

extern void SetAttributeParmID( const string & attrID, const string & value );

/*!
    \ingroup Attributes
*/
/*!
    Set the Vec3d value of a Vec3d-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeVec3d  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    vec3d InitVal = vec3d( 1., 0.5, -4. );
    string AttrID = AddAttributeVec3d( CollID, "TestVec3dAttr", { InitVal } );

    vec3d Vec3dVal = vec3d( 0.5, 0.75, -0.4 );
    SetAttributeVec3d( AttrID, {Vec3dVal} );

    array < vec3d > @GetVal = GetAttributeVec3dVal( AttrID );
    if ( GetVal[0].x() == Vec3dVal.x() and GetVal[0].y() == Vec3dVal.y() and GetVal[0].z() == Vec3dVal.z() )
    {
        Print( "Set matching Vec3d Value from Attribute" );
    }
    else
    {
        Print( "SetAttributeVec3dVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeVec3d  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = vec3d([1., 0.5, -4.])
    AttrID = AddAttributeVec3d( CollID, "TestVec3dAttr", [InitVal] )

    Vec3dVal = vec3d([0.5, 0.75, -0.4])
    SetAttributeVec3d( AttrID, [Vec3dVal] )

    GetVal = GetAttributeVec3dVal( AttrID )
    if ( GetVal[0].x() == Vec3dVal.x() ) and ( GetVal[0].y() == Vec3dVal.y() ) and ( GetVal[0].z() == Vec3dVal.z() ):
        print( "Set matching Vec3d Value from Attribute" )
    else:
        print( "SetAttributeVec3dVal error!" )
        assert False, "SetAttributeVec3dVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value vec3d value for attribute
*/

extern void SetAttributeVec3d( const string & attrID, const vector < vec3d > & value );

/*!
    \ingroup Attributes
*/
/*!
    Set the int matrix of a int-matrix-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeIntMatrix  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    array < array < int > > InitVal = {{0, 1},{-4, -1000}};
    string AttrID = AddAttributeIntMatrix( CollID, "TestIntMatrixAttr", InitVal );

    array < array < int > > NewImatVal = {{1,5},{-8,0}};
    SetAttributeIntMatrix( AttrID, NewImatVal );

    array < array < int > > IntMatrixVal = GetAttributeIntMatrixVal( AttrID );

    if ( IntMatrixVal == NewImatVal )
    {
        Print( "Set matching IntMatrix Value from Attribute" );
    }
    else
    {
        Print( "SetAttributeIntMatrixVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeIntMatrix  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = [[0, 1,],[-4, -1000]]
    AttrID = AddAttributeIntMatrix( CollID, "TestIntMatrixAttr", InitVal )

    ImatVal = [[1,5],[-8,0]]
    SetAttributeIntMatrix( AttrID, ImatVal )

    IntMatrixVal = GetAttributeIntMatrixVal( AttrID )
    IntMatrixVal = [list(row) for row in IntMatrixVal]

    if IntMatrixVal == ImatVal:
        print( "Set matching IntMatrix Value from Attribute" )
    else:
        print( "SetAttributeIntMatrixVal error!" )
        assert False, "SetAttributeIntMatrixVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value int matrix value for attribute
*/

extern void SetAttributeIntMatrix( const string & attrID, const vector < vector < int > > & value );

/*!
    \ingroup Attributes
*/
/*!
    Set the double matrix of a double-matrix-type Attribute by ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: SetAttributeDoubleMatrix  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    array < array < double > > InitVal = {{0., 1.},{-4., -1000.}};
    string AttrID = AddAttributeDoubleMatrix( CollID, "TestDoubleMatrixAttr", InitVal );

    array < array < double > > NewDmatVal = {{0.,1.5},{8.4,1.1566}};
    SetAttributeDoubleMatrix( AttrID, NewDmatVal );

    array < array < double > > DblMatrixVal = GetAttributeDoubleMatrixVal( AttrID );

    if ( DblMatrixVal == NewDmatVal )
    {
        Print( "Got matching Double Matrix Value from Attribute" );
    }
    else
    {
        Print( "GetAttributeDoubleMatrixVal error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: SetAttributeDoubleMatrix  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = [[0., 1.,],[-4., -1000.]]
    AttrID = AddAttributeDoubleMatrix( CollID, "TestDoubleMatrixAttr", InitVal )

    NewDmatVal = [[0.,1.5],[8.4,1.1566]]
    SetAttributeDoubleMatrix( AttrID, NewDmatVal )

    DblMatrixVal = GetAttributeDoubleMatrixVal( AttrID )
    DblMatrixVal = [list(row) for row in DblMatrixVal]

    if DblMatrixVal == NewDmatVal:
        print( "Got matching Double Matrix Value from Attribute" )
    else:
        print( "GetAttributeDoubleMatrixVal error!" )
        assert False, "GetAttributeDoubleMatrixVal error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
    \param [in] value double matrix value for attribute
*/

extern void SetAttributeDoubleMatrix( const string & attrID, const vector< vector< double > > & value );

/*!
    \ingroup Attributes
*/
/*!
    Delete attribute by attribute ID
    \forcpponly
    \code{.cpp}
    //==== Attribute: DeleteAttribute  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string InitVal = "This_Attribute_Will_Be_Deleted";
    string AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal );

    bool AttrAdded = false;
    bool AttrDeleted = true;

    array < string > @AttrIDs = FindAllAttributes();
    for ( int i = 0; i < int( AttrIDs.size() ); i++ )
    {
        if ( AttrID == AttrIDs[i] )
        {
            AttrAdded = true;
        }
    }

    DeleteAttribute( AttrID );

    array < string > @NewAttrIDs = FindAllAttributes();
    for ( int i = 0; i < int( NewAttrIDs.size() ); i++ )
    {
        if ( AttrID == NewAttrIDs[i] )
        {
            AttrDeleted = false;
        }
    }

    if ( AttrAdded and AttrDeleted )
    {
        Print( "Attribute successfully deleted" );
    }
    else
    {
        Print( "DeleteAttribute error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: DeleteAttribute  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = "This_Attribute_Will_Be_Deleted"
    AttrID = AddAttributeString( CollID, "TestStringAttr", InitVal )


    AttrIDs = FindAllAttributes()
    AttrAdded = AttrID in AttrIDs

    DeleteAttribute( AttrID )
    NewAttrIDs = FindAllAttributes()
    AttrDeleted = AttrID not in NewAttrIDs

    if AttrAdded and AttrDeleted:
        print( "Attribute successfully deleted" )
    else:
        print( "DeleteAttribute error!" )
        assert False, "DeleteAttribute error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string of attribute ID
*/

extern void DeleteAttribute( const string & attrID );

/*!
    \ingroup Attributes
*/
/*!
    Add a boolean attribute by name to an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeBool  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    bool InitVal = true;
    string AttrID = AddAttributeBool( CollID, "TestBoolAttr", InitVal );

    array < bool > @GetVal = GetAttributeBoolVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Added Bool Attribute" );
    }
    else
    {
        Print( "AddAttributeBool error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeBool  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = True
    AttrID = AddAttributeBool( CollID, "TestBoolAttr", InitVal )

    GetVal = GetAttributeBoolVal( AttrID )
    if GetVal[0] == InitVal:
        print( "Added Bool Attribute" )
    else:
        print( "AddAttributeBool error!" )
        assert False, "AddAttributeBool error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value bool boolean value of new attribute
*/

extern string AddAttributeBool( const string & collID, const string & attributeName, bool value );

/*!
    \ingroup Attributes
*/
/*!
    Add a integer attribute by name to an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeInt  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    int InitVal = 55;
    string AttrID = AddAttributeInt( CollID, "TestIntAttr", InitVal );

    array < int > @GetVal = GetAttributeIntVal( AttrID );
    if ( GetVal[0] == InitVal )
    {
        Print( "Added Int Attribute" );
    }
    else
    {
        Print( "AddAttributeInt error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeInt  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    InitVal = 55
    AttrID = AddAttributeInt( CollID, "TestIntAttr", InitVal )

    GetVal = GetAttributeIntVal( AttrID )
    if GetVal[0] == InitVal:
        print( "Added Int Attribute" )
    else:
        print( "AddAttributeInt error!" )
        assert False, "AddAttributeInt error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value int integer value of new attribute
*/

extern string AddAttributeInt( const string & collID, const string & attributeName, int value );

/*!
    \ingroup Attributes
*/
/*!
    Add a double attribute by name to an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeDouble  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = 'Example_Double_Attr';
    double DoubleValue = 3.14159;
    string AttrID = AddAttributeDouble( CollID, AttrName, DoubleValue );

    array < double > @GetVal = GetAttributeDoubleVal( AttrID );
    if ( GetVal[0] == DoubleValue )
    {
        Print( "Added Double Attribute" );
    }
    else
    {
        Print( "AddAttributeDouble error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeDouble  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_Double_Attr'
    DoubleValue = 3.14159
    AttrID = AddAttributeDouble( CollID, AttrName, DoubleValue )

    GetVal = GetAttributeDoubleVal( AttrID )
    if GetVal[0] == DoubleValue:
        print( "Added Double Attribute" )
    else:
        print( "AddAttributeDouble error!" )
        assert False, "AddAttributeDouble error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value double value of new attribute
*/

extern string AddAttributeDouble( const string & collID, const string & attributeName, double value );

/*!
    \ingroup Attributes
*/
/*!
    Add a string attribute by name to an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeString  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_String_Attr";
    string StringValue = "Example_String_Attr_DataVal";
    string AttrID = AddAttributeString( CollID, AttrName, StringValue );

    array < string > @GetVal = GetAttributeStringVal( AttrID );
    if ( GetVal[0] == StringValue )
    {
        Print( "Added String Attribute" );
    }
    else
    {
        Print( "AddAttributeString error!" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeString  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_String_Attr'
    StringValue = 'Example_String_Attr_DataVal'
    AttrID = AddAttributeString( CollID, AttrName, StringValue )

    GetVal = GetAttributeStringVal( AttrID )
    if GetVal[0] == StringValue:
        print( "Added String Attribute" )
    else:
        print( "AddAttributeString error!" )
        assert False, "AddAttributeString error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value string value of new attribute
*/

extern string AddAttributeString( const string & collID, const string & attributeName, const string & value );

/*!
    \ingroup Attributes
*/
/*!
    Add a parm attribute by name to an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeParm  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string PodID = AddGeom( "POD", "" );

    Print( "---> Test Add Parm Attr" );

    array < string > @ParmArray = GetGeomParmIDs( PodID );
    string ParmID = ParmArray[0];

    string AttrName = "Example_Parm_Attr";
    string AttrID = AddAttributeParm( CollID, AttrName, ParmID );

    array < string > @GetVal = GetAttributeParmID( AttrID );
    if ( GetVal[0] == ParmID )
    {
        Print( "Added Parm Attribute" );
    }
    else
    {
        Print( "AddAttributeParm error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeParm  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    pid = AddGeom( "POD", "" )
    print( "---> Test Add Parm Attr" )
    parm_array = GetGeomParmIDs( pid )
    AttrName = 'Example_Parm_Attr'
    ParmID = parm_array[0]
    AttrID = AddAttributeParm( CollID, AttrName, ParmID )

    GetVal = GetAttributeParmID( AttrID )
    if GetVal[0] == ParmID:
        print( "Added Parm Attribute" )
    else:
        print( "AddAttributeParm error!" )
        assert False, "AddAttributeParm error!"


    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] parmID string Parm ID to add to attribute
*/

extern string AddAttributeParm( const string &collID, const string &attributeName, const string &parmID );

/*!
    \ingroup Attributes
*/
/*!
    Add a Vec3d attribute by name to an attribute collection
    use vec3d() to create a vec3d object to pass into the args!
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeVec3d  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_Vec3D_Attr";
    vec3d Vec3dVal = vec3d( 0.5, 0.75, -0.4 );
    string AttrID = AddAttributeVec3d( CollID, AttrName, { Vec3dVal } );

    array < vec3d > @GetVal = GetAttributeVec3dVal( AttrID );

    if ( GetVal[0].x() == Vec3dVal.x() and GetVal[0].y() == Vec3dVal.y() and GetVal[0].z() == Vec3dVal.z() )
    {
        Print( "Added Vec3d Attribute" );
    }
    else
    {
        Print( "AddAttributeVec3d error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeVec3d  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_Vec3D_Attr'
    Vec3dVal = vec3d( 0.5, 0.75, -0.4 )
    AttrID = AddAttributeVec3d( CollID, AttrName, [Vec3dVal] )

    GetVal = GetAttributeVec3dVal( AttrID )
    if ( GetVal[0].x() == Vec3dVal.x() ) and ( GetVal[0].y() == Vec3dVal.y() ) and ( GetVal[0].z() == Vec3dVal.z() ):
        print( "Added Vec3d Attribute" )
    else:
        print( "AddAttributeVec3d error!" )
        assert False, "AddAttributeVec3d error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value vector \<vec3d\> Vec3d value of new attribute
*/

extern string AddAttributeVec3d( const string & collID, const string & attributeName, const vector < vec3d > & value );

/*!
    \ingroup Attributes
*/
/*!
    Add an Int Matrix attribute by name to an attribute collection
    use nested vectors/arrays of ints for matrix argument
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeIntMatrix  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_IntMatrix_Attr";
    array < array < int > > IntMatrix = {{1,5},{-8,0}};
    string AttrID = AddAttributeIntMatrix( CollID, AttrName, IntMatrix );

    array < array < int > > IntMatrixVal = GetAttributeIntMatrixVal( AttrID );

    if ( IntMatrixVal == IntMatrix )
    {
        Print( "Added IntMatrix Attribute" );
    }
    else
    {
        Print( "AddAttributeIntMatrix error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeIntMatrix  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_IntMatrix_Attr'
    IntMatrix = [[1,5],[-8,0]]
    AttrID = AddAttributeIntMatrix( CollID, AttrName, IntMatrix )

    IntMatrixVal = GetAttributeIntMatrixVal( AttrID )
    IntMatrixVal = [list(row) for row in IntMatrixVal]

    if IntMatrixVal == IntMatrix:
        print( "Added IntMatrix Attribute" )
    else:
        print( "AddAttributeIntMatrix error!" )
        assert False, "AddAttributeIntMatrix error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value int matrix value of new attribute
*/

extern string AddAttributeIntMatrix( const string & collID, const string & attributeName, const vector < vector < int > > & value );

/*!
    \ingroup Attributes
*/
/*!
    Add an Double Matrix attribute by name to an attribute collection
    use nested vectors/arrays of ints for matrix argument
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeDoubleMatrix  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_DoubleMat_Attr";
    array < array < double > > DoubleMatrix = {{0.,1.5},{8.4,1.1566}};
    string AttrID = AddAttributeDoubleMatrix( CollID, AttrName, DoubleMatrix );

    array < array < double > > DoubleMatrixVal = GetAttributeDoubleMatrixVal( AttrID );

    if ( DoubleMatrixVal == DoubleMatrix )
    {
        Print( "Added DoubleMatrix Attribute" );
    }
    else
    {
        Print( "AddAttributeDoubleMatrix error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeDoubleMatrix  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_DoubleMat_Attr'
    DoubleMatrix = [[0.,1.5],[8.4,1.1566]]
    AttrID = AddAttributeDoubleMatrix( CollID, AttrName, DoubleMatrix )

    DoubleMatrixVal = GetAttributeDoubleMatrixVal( AttrID )
    DoubleMatrixVal = [list(row) for row in DoubleMatrixVal]

    if DoubleMatrixVal == DoubleMatrix:
        print( "Added DoubleMatrix Attribute" )
    else:
        print( "AddAttributeDoubleMatrix error!" )
        assert False, "AddAttributeDoubleMatrix error!"

    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute
    \param [in] value vector \<vector \<double\>\> Double matrix value of new attribute
*/

extern string AddAttributeDoubleMatrix( const string & collID, const string & attributeName, const vector < vector < double > > & value );

/*!
    \ingroup Attributes
*/
/*!
    Add an empty Attribute Group-type attribute by name to an attribute collection
    \forcpponly
    \code{.cpp}
    //==== Attribute: AddAttributeGroup  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_Attr_Group";
    string AttrID = AddAttributeGroup( CollID, AttrName );

    if ( GetAttributeType( AttrID ) == RES_DATA_TYPE::ATTR_COLLECTION_DATA )
    {
        Print( "Added Attribute Group" );
    }
    else
    {
        Print( "AddAttributeGroup error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: AddAttributeGroup  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_Attr_Group'
    AttrID = AddAttributeGroup( CollID, AttrName )
    if GetAttributeType( AttrID ) == ATTR_COLLECTION_DATA:
        print( "Added Attribute Group" )
    else:
        print( "AddAttributeGroup error!" )
        assert False, "AddAttributeGroup error!"


    \endcode
    \endPythonOnly
    \param [in] collID string ID of attribute collection
    \param [in] attributeName string name of new attribute group
*/

extern string AddAttributeGroup( const string & collID, const string & attributeName );

/*!
    \ingroup Attributes
*/
/*!
    Copy an attribute to the clipboard by attributeID
    \forcpponly
    \code{.cpp}
    //==== Attribute: CopyAttribute  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_String_Attr";
    string StringValue = "Example_String_Attr_DataVal";

    string AttrID = AddAttributeString( CollID, AttrName, StringValue );
    int CopyError = CopyAttribute( AttrID );

    if ( CopyError == 0 )
    {
        Print("Successfully copied Attribute");
    }
    else
    {
        Print("CopyAttribute Error!");
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: CopyAttribute  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_String_Attr'
    StringValue = 'Example_String_Attr_DataVal'
    AttrID = AddAttributeString( CollID, AttrName, StringValue )
    CopyError = CopyAttribute( AttrID )
    if not CopyError:
        print("Successfully copied Attribute")
    else:
        print("CopyAttribute Error!")
        assert False, "CopyAttribute Error!"
    \endcode
    \endPythonOnly
    \param [in] attrID string ID of attribute to be copied
*/

extern int CopyAttribute( const string & attrID );

/*!
    \ingroup Attributes
*/
/*!
    Cut an attribute from its collection to the clipboard by attributeID
    \forcpponly
    \code{.cpp}
    //==== Attribute: CopyAttribute  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_String_Attr";
    string StringValue = "Example_String_Attr_DataVal";

    string AttrID = AddAttributeString( CollID, AttrName, StringValue );
    CutAttribute( AttrID );

    string NewCollID = GetChildCollection( "_AttrWMGroup" );
    array < string > @PastedAttrIDs = PasteAttribute( NewCollID );

    bool MatchIDs = PastedAttrIDs[0] == AttrID;
    bool AttrInColl = false;

    array < string > OldAttrIDs = FindAttributesInCollection( CollID );
    for ( int i = 0; i < int( OldAttrIDs.size() ); i++ )
    {
        if ( AttrID == OldAttrIDs[i] )
        {
            AttrInColl = true;
        }
    }
    
    if ( MatchIDs and not AttrInColl )
    {
        Print( "Successfully Cut Attribute" );
    }
    else
    {
        Print( "CutAttribute Error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: CopyAttribute  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_String_Attr'
    StringValue = 'Example_String_Attr_DataVal'
    AttrID = AddAttributeString( CollID, AttrName, StringValue )
    CutAttribute( AttrID )

    NewCollID = GetChildCollection( "_AttrWMGroup" )
    NewAttrIDs = PasteAttribute( NewCollID )

    MatchIDs = NewAttrIDs[0] == AttrID
    Attr_Cut_Check = AttrID not in FindAttributesInCollection( CollID )
    if MatchIDs and Attr_Cut_Check:
        print("Successfully cut Attribute")
    else:
        print("CutAttribute Error!")
        assert False, "CutAttribute Error!"

    \endcode
    \endPythonOnly
    \param [in] attrID string ID of attribute to be copied
*/

extern void CutAttribute( const string & attrID );

/*!
    \ingroup Attributes
*/
/*!
    Paste the attribute clipboard to the specified objectID
    ObjectID can be any OpenVSP entity that contains a AttributeCollection or simply the attributeCollectionID
    Returns a vector of pasted attributes IDs, if any
    \forcpponly
    \code{.cpp}
    //==== Attribute: PasteAttribute  =====//

    string VehID = GetVehicleID();
    string CollID = GetChildCollection( VehID );
    string AttrName = "Example_String_Attr";
    string StringValue = "Example_String_Attr_DataVal";
    string AttrID = AddAttributeString( CollID, AttrName, StringValue );
    CutAttribute( AttrID );

    string NewCollID = GetChildCollection( "_AttrWMGroup" );
    array < string > @NewAttrIDs = PasteAttribute( NewCollID );

    bool MatchIDs = false;
    bool AttrInOldColl = false;
    bool AttrInNewColl = false;

    array < string > OldCollAttrs = FindAttributesInCollection( CollID );
    array < string > NewCollAttrs = FindAttributesInCollection( NewCollID );

    MatchIDs = NewAttrIDs[0] == AttrID;

    for ( int i = 0; i < int( OldCollAttrs.size() ); i++ )
    {
        if ( AttrID == OldCollAttrs[i] )
        {
            AttrInOldColl = true;
        }
    }

    for ( int i = 0; i < int( NewCollAttrs.size() ); i++ )
    {
        if ( AttrID == NewCollAttrs[i] )
        {
            AttrInNewColl = true;
        }
    }

    if ( MatchIDs and !AttrInOldColl and AttrInNewColl )
    {
        Print("Successfully pasted Attribute");
    }
    else
    {
        Print("PasteAttribute Error!");
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ##==== Attribute: PasteAttribute  =====##

    VehID = GetVehicleID()
    CollID = GetChildCollection( VehID )
    AttrName = 'Example_String_Attr'
    StringValue = 'Example_String_Attr_DataVal'
    AttrID = AddAttributeString( CollID, AttrName, StringValue )
    CutAttribute( AttrID )

    NewCollID = GetChildCollection( "_AttrWMGroup" )
    NewAttrIDs = PasteAttribute( NewCollID )

    MatchIDs = NewAttrIDs[0] == AttrID
    Attr_Cut_Check = AttrID not in FindAttributesInCollection( CollID )
    Attr_Paste_Check = AttrID in FindAttributesInCollection( NewCollID )
    if MatchIDs and Attr_Cut_Check and Attr_Paste_Check:
        print("Successfully pasted Attribute")
    else:
        print("PasteAttribute Error!")
        assert False, "PasteAttribute Error!"

    \endcode
    \endPythonOnly
    \param [in] coll_id string ID of destination for pasting attribute into
*/

extern vector < string > PasteAttribute( const string & coll_id );

//======================== Results ================================//
/*!
    \ingroup Results
*/
/*!
    Get the name of all results in the Results Manager
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    array< string > @results_array = GetAllResultsNames();
    if ( results_array.length() == 0 )
    {
        Print( "ERROR: GetAllResultsNames returned nothing" );
        __failure++;
    }

    for ( int i = 0; i < int( results_array.size() ); i++ )
    {
        string resid = FindLatestResultsID( results_array[i] );
        PrintResults( resid );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    results_array = GetAllResultsNames()
    assert len( results_array ) > 0, "GetAllResultsNames returned nothing"

    for i in range(int( len(results_array) )):

        resid = FindLatestResultsID( results_array[i] )
        PrintResults( resid )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of result names
*/

extern std::vector<std::string> GetAllResultsNames();

/*!
    \ingroup Results
*/
/*!
    Get all data names for a particular result
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    array< string > @data_names = GetAllDataNames( res_id );

    if ( data_names.size() != 5 )                            { Print( "---> Error: API GetAllDataNames" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    data_names = GetAllDataNames( res_id )

    if  len(data_names) != 5 :
        print( "---> Error: API GetAllDataNames" )
        assert False, "---> Error: API GetAllDataNames"

    \endcode
    \endPythonOnly
    \param [in] results_id string Result ID
    \return vector\<string\> Array of result names
*/

extern std::vector<std::string> GetAllDataNames( const std::string & results_id );

/*!
    \ingroup Results
*/
/*!
    Get the number of results for a particular result name
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    if ( GetNumResults( "Test_Results" ) != 2 )                { Print( "---> Error: API GetNumResults" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    if ( GetNumResults( "Test_Results" ) != 2 ):
        print( "---> Error: API GetNumResults" )
        assert False, "---> Error: API GetNumResults"

    \endcode
    \endPythonOnly
    \param [in] name string Input name
    \return int Number of results
*/

extern int GetNumResults( const std::string & name );

/*!
    \ingroup Results
*/
/*!
    Get the name of a result given its ID
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set defaults
    SetAnalysisInputDefaults( analysis_name );

    string res_id = ( ExecAnalysis( analysis_name ) );

    Print( "Results Name: ", false );

    Print( GetResultsName( res_id ) );

    // The name is the Results Manager's handle for this result, so looking the
    // name back up has to lead to the same result.
    if ( GetResultsName( res_id ).length() == 0 )
    {
        Print( "ERROR: GetResultsName returned nothing" );
        __failure++;
    }

    if ( FindLatestResultsID( GetResultsName( res_id ) ) != res_id )
    {
        Print( "ERROR: GetResultsName does not round trip through FindLatestResultsID" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set defaults
    SetAnalysisInputDefaults( analysis_name )

    res_id = ( ExecAnalysis( analysis_name ) )

    print( "Results Name: ", False )

    print( GetResultsName( res_id ) )

    # The name is the Results Manager's handle for this result, so looking the
    # name back up has to lead to the same result.
    assert len( GetResultsName( res_id ) ) > 0, "GetResultsName returned nothing"
    assert FindLatestResultsID( GetResultsName( res_id ) ) == res_id, "GetResultsName does not round trip through FindLatestResultsID"

    \endcode
    \endPythonOnly
    \param [in] results_id string Result ID
    \return string Result name
*/

extern std::string GetResultsName(const std::string & results_id );

/*!
    \ingroup Results
*/
/*!
    Get the documentation string for a result given its ID
    \forcpponly
    \code{.cpp}
    //==== Analysis: VSPAero Compute Geometry ====//
    string analysis_name = "VSPAEROComputeGeometry";

    // Set defaults
    SetAnalysisInputDefaults( analysis_name );

    string res_id = ( ExecAnalysis( analysis_name ) );

    Print( "Results doc: ", false );

    Print( GetResultsSetDoc( res_id ) );

    if ( GetResultsSetDoc( res_id ).length() == 0 )
    {
        Print( "ERROR: the result carries no documentation" );
        __failure++;
    }

    // Every entry in the result has to be documented too.
    array< string > @data_names = GetAllDataNames( res_id );

    for ( int i = 0; i < int( data_names.size() ); i++ )
    {
        if ( GetResultsEntryDoc( res_id, data_names[i] ).length() == 0 )
        {
            Print( "ERROR: " + data_names[i] + " carries no documentation" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set defaults
    SetAnalysisInputDefaults( analysis_name )

    res_id = ( ExecAnalysis( analysis_name ) )

    print( "Results doc: ", False )

    print( GetResultsSetDoc( res_id ) )

    assert len( GetResultsSetDoc( res_id ) ) > 0, "the result carries no documentation"

    # Every entry in the result has to be documented too.
    data_names = GetAllDataNames( res_id )

    for data_name in data_names:
        assert len( GetResultsEntryDoc( res_id, data_name ) ) > 0, data_name + " carries no documentation"

    \endcode
    \endPythonOnly
    \param [in] results_id string Result ID
    \return string Result documentation string
*/

extern std::string GetResultsSetDoc( const std::string & results_id );

extern std::string GetResultsEntryDoc( const std::string & results_id, const std::string & data_name );

/*!
    \ingroup Results
*/
/*!
    Find a results ID given its name and index
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    if ( res_id.size() == 0 )                                { Print( "---> Error: API FindResultsID" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    if  len(res_id) == 0 :
        print( "---> Error: API FindResultsID" )
        assert False, "---> Error: API FindResultsID"

    \endcode
    \endPythonOnly
    \param [in] name string Result name
    \param [in] index int Result index
    \return string Result ID
*/

extern std::string FindResultsID( const std::string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Find the latest results ID for particular result name
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    array< string > @results_array = GetAllResultsNames();

    for ( int i = 0; i < int( results_array.size() ); i++ )
    {
        string resid = FindLatestResultsID( results_array[i] );
        if ( resid.length() == 0 )
        {
            Print( "ERROR: FindLatestResultsID found nothing" );
            __failure++;
        }
        PrintResults( resid );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    results_array = GetAllResultsNames()

    for i in range(int( len(results_array) )):

        resid = FindLatestResultsID( results_array[i] )
        assert len( resid ) > 0, "FindLatestResultsID found nothing"
        PrintResults( resid )

    \endcode
    \endPythonOnly
    \param [in] name string Result name
    \return string Result ID
*/

extern std::string FindLatestResultsID( const std::string & name );

/*!
    \ingroup Results
*/
/*!
    Get the number of data values for a given result ID and data name
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    if ( GetNumData( res_id, "Test_Int" ) != 2 )            { Print( "---> Error: API GetNumData " ); __failure++; }

    array<int> @int_arr = GetIntResults( res_id, "Test_Int", 0 );

    if ( int_arr[0] != 1 )                                    { Print( "---> Error: API GetIntResults" ); __failure++; }

    int_arr = GetIntResults( res_id, "Test_Int", 1 );

    if ( int_arr[0] != 2 )                                    { Print( "---> Error: API GetIntResults" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    if ( GetNumData( res_id, "Test_Int" ) != 2 ):
        print( "---> Error: API GetNumData " )
        assert False, "---> Error: API GetNumData"

    int_arr = GetIntResults( res_id, "Test_Int", 0 )

    if  int_arr[0] != 1 :
        print( "---> Error: API GetIntResults" )
        assert False, "---> Error: API GetIntResults"

    int_arr = GetIntResults( res_id, "Test_Int", 1 )

    if  int_arr[0] != 2 :
        print( "---> Error: API GetIntResults" )
        assert False, "---> Error: API GetIntResults"

    \endcode
    \endPythonOnly
    \param [in] results_id string Result ID
    \param [in] data_name string Data name
    \return int Number of data values
*/

extern int GetNumData( const std::string & results_id, const std::string & data_name );

/*!
    \ingroup Results
*/
/*!
    Get the data type for a given result ID and data name
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    array < string > @ res_array = GetAllDataNames( res_id );

    if ( res_array.size() == 0 )
    {
        Print( "ERROR: GetAllDataNames returned nothing" );
        __failure++;
    }

    for ( int j = 0; j < int( res_array.size() ); j++ )
    {
        int typ = GetResultsType( res_id, res_array[j] );

        // Every entry the result lists has to report a real data type.
        if ( typ == INVALID_TYPE )
        {
            Print( "ERROR: GetResultsType returned INVALID_TYPE for " + res_array[j] );
            __failure++;
        }
    }

    // The fake results are written with known types.
    if ( GetResultsType( res_id, "Test_Int" ) != INT_DATA ||
         GetResultsType( res_id, "Test_Double" ) != DOUBLE_DATA ||
         GetResultsType( res_id, "Test_String" ) != STRING_DATA )
    {
        Print( "ERROR: GetResultsType reported the wrong type" );
        __failure++;
    }

    // An entry that does not exist has to report INVALID_TYPE.
    if ( GetResultsType( res_id, "NoSuchEntry" ) != INVALID_TYPE )
    {
        Print( "ERROR: GetResultsType accepted an unknown entry" );
        __failure++;
    }

    // That lookup failure was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    res_array = GetAllDataNames( res_id )

    assert len( res_array ) > 0, "GetAllDataNames returned nothing"

    for j in range(int( len(res_array) )):

        typ = GetResultsType( res_id, res_array[j] )

        # Every entry the result lists has to report a real data type.
        assert typ != INVALID_TYPE, "GetResultsType returned INVALID_TYPE for " + res_array[j]

    # The fake results are written with known types.
    assert GetResultsType( res_id, "Test_Int" ) == INT_DATA, "GetResultsType reported the wrong type"
    assert GetResultsType( res_id, "Test_Double" ) == DOUBLE_DATA, "GetResultsType reported the wrong type"
    assert GetResultsType( res_id, "Test_String" ) == STRING_DATA, "GetResultsType reported the wrong type"

    # An entry that does not exist has to report INVALID_TYPE.
    assert GetResultsType( res_id, "NoSuchEntry" ) == INVALID_TYPE, "GetResultsType accepted an unknown entry"

    # That lookup failure was raised deliberately, so take it back off the queue.
    err_mgr = ErrorMgrSingleton.getInstance()

    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE
    \param [in] results_id string Result ID
    \param [in] data_name string Data name
    \return int Data type enum (i.e. DOUBLE_DATA)
*/

extern int GetResultsType( const std::string & results_id, const std::string & data_name );

/*!
    \ingroup Results
*/
/*!
    Get all integer values for a particular result, name, and index
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    if ( GetNumData( res_id, "Test_Int" ) != 2 )            { Print( "---> Error: API GetNumData " ); __failure++; }

    array<int> @int_arr = GetIntResults( res_id, "Test_Int", 0 );

    if ( int_arr[0] != 1 )                                    { Print( "---> Error: API GetIntResults" ); __failure++; }

    int_arr = GetIntResults( res_id, "Test_Int", 1 );

    if ( int_arr[0] != 2 )                                    { Print( "---> Error: API GetIntResults" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    if ( GetNumData( res_id, "Test_Int" ) != 2 ):
        print( "---> Error: API GetNumData " )
        assert False, "---> Error: API GetNumData"

    int_arr = GetIntResults( res_id, "Test_Int", 0 )

    if  int_arr[0] != 1 :
        print( "---> Error: API GetIntResults" )
        assert False, "---> Error: API GetIntResults"

    int_arr = GetIntResults( res_id, "Test_Int", 1 )

    if  int_arr[0] != 2 :
        print( "---> Error: API GetIntResults" )
        assert False, "---> Error: API GetIntResults"

    \endcode
    \endPythonOnly
    \param [in] id string Result ID
    \param [in] name string Data name
    \param [in] index int Data index
    \return vector\<int\> Array of data values
*/

extern const std::vector< int > & GetIntResults( const std::string & id, const std::string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Get all double values for a particular result, name, and index
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geom ====//
    string pid = AddGeom( "POD", "" );

    //==== Run CompGeom And View Results ====//
    string mesh_id = ComputeCompGeom( SET_ALL, false, 0 );                      // Half Mesh false and no file export

    string comp_res_id = FindLatestResultsID( "Comp_Geom" );                    // Find Results ID

    array<double> @double_arr = GetDoubleResults( comp_res_id, "Wet_Area" );    // Extract Results
    if ( double_arr.length() == 0 )
    {
        Print( "ERROR: GetDoubleResults returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    #==== Run CompGeom And View Results ====//
    mesh_id = ComputeCompGeom( SET_ALL, False, 0 )                      # Half Mesh false and no file export

    comp_res_id = FindLatestResultsID( "Comp_Geom" )                    # Find Results ID

    double_arr = GetDoubleResults( comp_res_id, "Wet_Area" )    # Extract Results
    assert len( double_arr ) > 0, "GetDoubleResults returned nothing"

    \endcode
    \endPythonOnly
    \param [in] id string Result ID
    \param [in] name string Data name
    \param [in] index int Data index
    \return vector\<double\> Array of data values
*/

extern const std::vector< double > & GetDoubleResults( const std::string & id, const std::string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Get all matrix (vector<vector<double>>) values for a particular result, name, and index
    \param [in] id string Result ID
    \param [in] name string Data name
    \param [in] index int Data index
    \return vector\<vector<double\>> 2D array of data values
*/

extern const std::vector< std::vector< double > > & GetDoubleMatResults( const std::string & id, const std:: string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Get all string values for a particular result, name, and index
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    string res_id = FindResultsID( "Test_Results" );

    array<string> @str_arr = GetStringResults( res_id, "Test_String" );

    if ( str_arr[0] != "This Is A Test" )                    { Print( "---> Error: API GetStringResults" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    str_arr = GetStringResults( res_id, "Test_String" )

    if ( str_arr[0] != "This Is A Test" ):
        print( "---> Error: API GetStringResults" )
        assert False, "---> Error: API GetStringResults"

    \endcode
    \endPythonOnly
    \param [in] id string Result ID
    \param [in] name string Data name
    \param [in] index int Data index
    \return vector\<string\> Array of data values
*/

extern const std::vector<std::string> & GetStringResults( const std::string & id, const std::string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Get all vec3d values for a particular result, name, and index
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//

    double tol = 0.00001;

    WriteTestResults();

    string res_id = FindLatestResultsID( "Test_Results" );

    array<vec3d> @vec3d_vec = GetVec3dResults( res_id, "Test_Vec3d" );
    if ( vec3d_vec.length() == 0 )
    {
        Print( "ERROR: GetVec3dResults returned nothing" );
        __failure++;
    }

    Print( "X: ", false );
    Print( vec3d_vec[0].x(), false );

    Print( "\tY: ", false );
    Print( vec3d_vec[0].y(), false );

    Print( "\tZ: ", false );
    Print( vec3d_vec[0].z() );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//

    tol = 0.00001

    WriteTestResults()

    res_id = FindLatestResultsID( "Test_Results" )

    vec3d_vec = GetVec3dResults( res_id, "Test_Vec3d" )
    assert len( vec3d_vec ) > 0, "GetVec3dResults returned nothing"

    print( "X: ", False )
    print( vec3d_vec[0].x(), False )

    print( "\tY: ", False )
    print( vec3d_vec[0].y(), False )

    print( "\tZ: ", False )
    print( vec3d_vec[0].z() )

    \endcode
    \endPythonOnly
    \param [in] id string Result ID
    \param [in] name string Data name
    \param [in] index int Data index
    \return vector\<vec3d\> Array of data values
*/

extern const std::vector< vec3d > & GetVec3dResults( const std::string & id, const std::string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Create a new result for a Geom
    \forcpponly
    \code{.cpp}
    //==== Test Comp Geom ====//
    string gid1 = AddGeom( "POD", "" );

    string mesh_id = ComputeCompGeom( 0, false, 0 );

    //==== Test Comp Geom Mesh Results ====//
    string mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" );

    array<int> @int_arr = GetIntResults( mesh_geom_res_id, "Num_Tris" );

    if ( int_arr[0] < 4 )                                            { Print( "---> Error: API CreateGeomResults" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Test Comp Geom ====//
    gid1 = AddGeom( "POD", "" )

    mesh_id = ComputeCompGeom( 0, False, 0 )

    #==== Test Comp Geom Mesh Results ====//
    mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" )

    int_arr = GetIntResults( mesh_geom_res_id, "Num_Tris" )

    if  int_arr[0] < 4 :
        print( "---> Error: API CreateGeomResults" )
        assert False, "---> Error: API CreateGeomResults"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Result name
    \return string Result ID
*/

extern std::string CreateGeomResults( const std::string & geom_id, const std::string & name );

/*!
    \ingroup Results
*/
/*!
    Delete all results
    \forcpponly
    \code{.cpp}
    //==== Test Comp Geom ====//
    string gid1 = AddGeom( "POD", "" );

    string mesh_id = ComputeCompGeom( 0, false, 0 );

    //==== Test Comp Geom Mesh Results ====//
    string mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" );

    DeleteAllResults();

    if ( GetNumResults( "Comp_Mesh" ) != 0 )                { Print( "---> Error: API DeleteAllResults" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Test Comp Geom ====//
    gid1 = AddGeom( "POD", "" )

    mesh_id = ComputeCompGeom( 0, False, 0 )

    #==== Test Comp Geom Mesh Results ====//
    mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" )

    DeleteAllResults()

    if ( GetNumResults( "Comp_Mesh" ) != 0 ):
        print( "---> Error: API DeleteAllResults" )
        assert False, "---> Error: API DeleteAllResults"

    \endcode
    \endPythonOnly
*/

extern void DeleteAllResults();

/*!
    \ingroup Results
*/
/*!
    Delete a particular result
    \forcpponly
    \code{.cpp}
    //==== Test Comp Geom ====//
    string gid1 = AddGeom( "POD", "" );

    string mesh_id = ComputeCompGeom( 0, false, 0 );

    //==== Test Comp Geom Mesh Results ====//
    string mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" );

    DeleteResult( mesh_geom_res_id );

    if ( GetNumResults( "Comp_Mesh" ) != 0 )                { Print( "---> Error: API DeleteResult" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Test Comp Geom ====//
    gid1 = AddGeom( "POD", "" )

    mesh_id = ComputeCompGeom( 0, False, 0 )

    #==== Test Comp Geom Mesh Results ====//
    mesh_geom_res_id = CreateGeomResults( mesh_id, "Comp_Mesh" )

    DeleteResult( mesh_geom_res_id )

    if ( GetNumResults( "Comp_Mesh" ) != 0 ):
        print( "---> Error: API DeleteResult" )
        assert False, "---> Error: API DeleteResult"

    \endcode
    \endPythonOnly
    \param [in] id string Result ID
*/

extern void DeleteResult( const std::string & id );

/*!
    \ingroup Results
*/
/*!
    Export a result to CSV
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string analysis_name = "VSPAEROComputeGeometry";

    string rid = ExecAnalysis( analysis_name );

    WriteResultsCSVFile( rid, "CompGeomRes.csv" );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "CompGeomRes.csv", "r" ) < 0 )
    {
        Print( "ERROR: WriteResultsCSVFile wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteResultsCSVFile wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pid = AddGeom( "POD" )

    analysis_name = "VSPAEROComputeGeometry"

    rid = ExecAnalysis( analysis_name )

    WriteResultsCSVFile( rid, "CompGeomRes.csv" )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "CompGeomRes.csv" ) > 0, "WriteResultsCSVFile wrote no file"


    \endcode
    \endPythonOnly
    \param [in] id string Rsult ID
    \param [in] file_name string CSV output file name
*/

extern void WriteResultsCSVFile( const std::string & id, const std::string & file_name );

/*!
    \ingroup Results
*/
/*!
    Print a result's name value pairs to stdout
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string analysis_name = "VSPAEROComputeGeometry";

    string rid = ExecAnalysis( analysis_name );

    // Get & Display Results
    PrintResults( rid );

    // There has to be something to display.
    if ( GetAllDataNames( rid ).size() == 0 )
    {
        Print( "ERROR: the result carries no data to print" );
        __failure++;
    }

    // Every entry that gets printed has to have a value behind it.
    array< string > @data_names = GetAllDataNames( rid );

    for ( int i = 0; i < int( data_names.size() ); i++ )
    {
        if ( GetNumData( rid, data_names[i] ) < 1 )
        {
            Print( "ERROR: " + data_names[i] + " has no data to print" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pid = AddGeom( "POD" )

    analysis_name = "VSPAEROComputeGeometry"

    rid = ExecAnalysis( analysis_name )

    # Get & Display Results
    PrintResults( rid )

    # There has to be something to display.
    assert len( GetAllDataNames( rid ) ) > 0, "the result carries no data to print"

    # Every entry that gets printed has to have a value behind it.
    data_names = GetAllDataNames( rid )

    for data_name in data_names:
        assert GetNumData( rid, data_name ) >= 1, data_name + " has no data to print"

    \endcode
    \endPythonOnly
    \param [in] results_id string Result ID
*/

extern void PrintResults( const std::string &results_id );

/*!
    \ingroup Results
*/
/*!
    Print a result's names and documentation to stdout
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pid = AddGeom( "POD" );

    string analysis_name = "VSPAEROComputeGeometry";

    string rid = ExecAnalysis( analysis_name );

    // Get & Display Results Docs
    PrintResultsDocs( rid );

    // There has to be documentation to display.
    if ( GetResultsSetDoc( rid ).length() == 0 )
    {
        Print( "ERROR: the result carries no documentation to print" );
        __failure++;
    }

    // Every entry that gets printed has to carry documentation of its own.
    array< string > @data_names = GetAllDataNames( rid );

    for ( int i = 0; i < int( data_names.size() ); i++ )
    {
        if ( GetResultsEntryDoc( rid, data_names[i] ).length() == 0 )
        {
            Print( "ERROR: " + data_names[i] + " carries no documentation" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pid = AddGeom( "POD" )

    analysis_name = "VSPAEROComputeGeometry"

    rid = ExecAnalysis( analysis_name )

    # Get & Display Results Docs
    PrintResultsDocs( rid )

    # There has to be documentation to display.
    assert len( GetResultsSetDoc( rid ) ) > 0, "the result carries no documentation to print"

    # Every entry that gets printed has to carry documentation of its own.
    data_names = GetAllDataNames( rid )

    for data_name in data_names:
        assert len( GetResultsEntryDoc( rid, data_name ) ) > 0, data_name + " carries no documentation"

    \endcode
    \endPythonOnly
    \param [in] results_id string Result ID
*/

extern void PrintResultsDocs( const std::string &results_id );

/*!
    \ingroup Results
*/
/*!
    Generate some example results for testing.
    \forcpponly
    \code{.cpp}
    //==== Write Some Fake Test Results =====//
    WriteTestResults();

    array< string > @results_array = GetAllResultsNames();

    for ( int i = 0; i < int( results_array.size() ); i++ )
    {
        string resid = FindLatestResultsID( results_array[i] );
        PrintResults( resid );
    }

    // Two copies of Test_Results are written, carrying five named entries with
    // known values.
    if ( GetNumResults( "Test_Results" ) != 2 )
    {
        Print( "ERROR: WriteTestResults did not write two results" );
        __failure++;
    }

    string res_id = FindResultsID( "Test_Results" );

    if ( GetAllDataNames( res_id ).size() != 5 )
    {
        Print( "ERROR: WriteTestResults wrote the wrong number of entries" );
        __failure++;
    }

    array< int > @int_arr = GetIntResults( res_id, "Test_Int", 0 );
    array< double > @dbl_arr = GetDoubleResults( res_id, "Test_Double", 0 );
    array< string > @str_arr = GetStringResults( res_id, "Test_String", 0 );

    if ( int_arr[0] != 1 || !closeTo( dbl_arr[0], 0.1, 1e-12 ) || str_arr[0] != "This Is A Test" )
    {
        Print( "ERROR: WriteTestResults wrote the wrong values" );
        __failure++;
    }

    // The second copy carries the next set of values.
    string res_id_1 = FindResultsID( "Test_Results", 1 );

    int_arr = GetIntResults( res_id_1, "Test_Int", 0 );

    if ( int_arr[0] != 2 )
    {
        Print( "ERROR: WriteTestResults did not vary the second result" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    results_array = GetAllResultsNames()

    for i in range( len( results_array ) ):
        resid = FindLatestResultsID( results_array[i] )
        PrintResults( resid )

    # Two copies of Test_Results are written, carrying five named entries with
    # known values.
    assert GetNumResults( "Test_Results" ) == 2, "WriteTestResults did not write two results"

    res_id = FindResultsID( "Test_Results" )

    assert len( GetAllDataNames( res_id ) ) == 5, "WriteTestResults wrote the wrong number of entries"

    int_arr = GetIntResults( res_id, "Test_Int", 0 )
    dbl_arr = GetDoubleResults( res_id, "Test_Double", 0 )
    str_arr = GetStringResults( res_id, "Test_String", 0 )

    assert int_arr[0] == 1, "WriteTestResults wrote the wrong values"
    assert abs( dbl_arr[0] - 0.1 ) < 1e-12, "WriteTestResults wrote the wrong values"
    assert str_arr[0] == "This Is A Test", "WriteTestResults wrote the wrong values"

    # The second copy carries the next set of values.
    res_id_1 = FindResultsID( "Test_Results", 1 )

    int_arr = GetIntResults( res_id_1, "Test_Int", 0 )

    assert int_arr[0] == 2, "WriteTestResults did not vary the second result"

    \endcode
    \endPythonOnly
*/

extern void WriteTestResults();

//======================== GUI Functions ================================//

/*!
    \ingroup Visualization
*/
/*!
    Low level routine that should be called to set up GUI before running StartGUI()
    \forcpponly
    \code{.cpp}

    InitGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()

    \endcode
    \endPythonOnly
*/

extern void InitGUI();

/*!
    \ingroup Visualization
*/
/*!
    Launch the interactive OpenVSP GUI.  In a multi-threaded environment, this must be called from the main thread only.
    This starts the GUI event loop.  It will also show the main screen and screens displayed when StopGUI() was
    previously called.
    \forcpponly
    \code{.cpp}

    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    \endcode
    \endPythonOnly
*/

extern void StartGUI();

/*!
    \ingroup Visualization
*/
/*!
    Enable Stop GUI Menu Item from the OpenVSP GUI.

    Typically used for the blocking-mode OpenVSP GUI from the API.

    This will add a "Stop GUI" option to the file pulldown menu and will also cause the exit button on the
    window frame to have the same effect.  When selected, these options will stop the OpenVSP GUI event loop,
    returning control to the API program.  OpenVSP will not terminate, the model will remain in memory and will
    be responsive to subsequent API calls.

    \forcpponly
    \code{.cpp}

    EnableStopGUIMenuItem();
    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    EnableStopGUIMenuItem()
    StartGUI()

    \endcode
    \endPythonOnly

    \sa DisableStopGUIMenuItem
*/

extern void EnableStopGUIMenuItem();

/*!
    \ingroup Visualization
*/
/*!
    Disable Stop GUI Menu Item from the OpenVSP GUI.

    This reverses the operation of EnableStopGUIMenuItem.

    \forcpponly
    \code{.cpp}

    EnableStopGUIMenuItem();
    DisableStopGUIMenuItem();
    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    EnableStopGUIMenuItem()
    DisableStopGUIMenuItem()
    StartGUI()

    \endcode
    \endPythonOnly

    \sa EnableStopGUIMenuItem
*/

extern void DisableStopGUIMenuItem();

/*!
    \ingroup Visualization
*/
/*!
    Stop OpenVSP GUI event loop and hide screens.  Keep OpenVSP running and in memory.
    \forcpponly
    \code{.cpp}

    StartGUI();

    StopGUI();

    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    StopGUI()

    StartGUI()

    \endcode
    \endPythonOnly

    \sa StartGUI
*/

extern void StopGUI();

/*!
    \ingroup Visualization
*/
/*!
    Cause OpenVSP to display a popup message.
    \forcpponly
    \code{.cpp}

    StartGUI();

    PopupMsg( "This is a popup message." );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    PopupMsg( "This is a popup message." )

    \endcode
    \endPythonOnly

    \param [in] msg string Message to display.
*/

extern void PopupMsg( const std::string &msg );

/*!
    \ingroup Visualization
*/
/*!
    Tell OpenVSP that the GUI needs to be updated.
    \forcpponly
    \code{.cpp}

    StartGUI();

    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmVal( length, 13.0 );

    UpdateGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmVal( length, 13.0 )

    UpdateGUI()

    \endcode
    \endPythonOnly

    \sa StartGUI
*/

extern void UpdateGUI();

/*!
    \ingroup Visualization
*/
/*!
    Test if the current OpenVSP build includes graphics capabilities.
    \forcpponly
    \code{.cpp}

    if ( IsGUIBuild() )
    {
        Print( "OpenVSP build is graphics capable." );
    }
    else
    {
        Print( "OpenVSP build is not graphics capable." );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    if ( IsGUIBuild() ):
        print( "OpenVSP build is graphics capable." )
    else:
        print( "OpenVSP build is not graphics capable." )

    \endcode
    \endPythonOnly

    \return bool True if the current OpenVSP build includes graphics capabilities.  False otherwise.
*/

extern bool IsGUIBuild();

/*!
    \ingroup Visualization
*/
/*!
    Obtain the lock on the OpenVSP GUI event loop.  This will prevent the interactive GUI from
    updating or accepting user input until the lock is released -- thereby allowing longer-time
    commands including analyses to execute without the chance of the OpenVSP state changing during
    execution.

    \forcpponly
    \code{.cpp}

    StartGUI();

    string pod_id = AddGeom( "POD" );

    Lock();
    string rid = ExecAnalysis( "CompGeom" );

    array<string>@ mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" );

    DeleteGeomVec( mesh_id_vec );
    Unlock();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    pod_id = AddGeom( "POD" )

    Lock()
    rid = ExecAnalysis( "CompGeom" )

    mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" )

    DeleteGeomVec( mesh_id_vec )
    Unlock()

    \endcode
    \endPythonOnly

    \sa Unlock
*/

extern void Lock( );

/*!
    \ingroup Visualization
*/
/*!
    Release the lock on the OpenVSP GUI event loop.

    \forcpponly
    \code{.cpp}

    StartGUI();

    string pod_id = AddGeom( "POD" );

    Lock();
    string rid = ExecAnalysis( "CompGeom" );

    array<string>@ mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" );

    DeleteGeomVec( mesh_id_vec );
    Unlock();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    pod_id = AddGeom( "POD" )

    Lock()
    rid = ExecAnalysis( "CompGeom" )

    mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" )

    DeleteGeomVec( mesh_id_vec )
    Unlock()

    \endcode
    \endPythonOnly

    \sa Lock
*/

extern void Unlock( );


/*!
    \ingroup Visualization
*/
/*!
    Test if the OpenVSP GUI event loop is running.

    \forcpponly
    \code{.cpp}

    StartGUI();

    if ( IsEventLoopRunning() )
    {
        Print( "Event loop is running." );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    StartGUI()

    if ( IsEventLoopRunning() ):
        print( "Event loop is running." )

    \endcode
    \endPythonOnly
    \return bool True if the OpenVSP GUI event loop is running.  False otherwise.
*/

extern bool IsEventLoopRunning( );

/*!
    \ingroup Visualization
*/
/*!
    Capture the specified screen and save to file. Note, VSP_USE_FLTK must be defined
    \forcpponly
    \code{.cpp}
    int screenw = 2000;                                             // Set screenshot width and height
    int screenh = 2000;

    string fname = "test_screen_grab.png";

    ScreenGrab( fname, screenw, screenh, true, true );                // Take PNG screenshot
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    screenw = 2000                                             # Set screenshot width and height
    screenh = 2000

    fname = "test_screen_grab.png"

    ScreenGrab( fname, screenw, screenh, True, True )                # Take PNG screenshot

    \endcode
    \endPythonOnly
    \param [in] fname string Output file name
    \param [in] w int Width of screen grab
    \param [in] h int Height of screen grab
    \param [in] transparentBG bool Transparent background flag
    \param [in] autocrop bool Automatically crop transparent background flag
*/

extern void ScreenGrab( const string & fname, int w, int h, bool transparentBG, bool autocrop = false );

/*!
    \ingroup Visualization
*/
/*!
    Toggle viewing the axis
    \forcpponly
    \code{.cpp}
    SetViewAxis( false );                                           // Turn off axis marker in corner of viewscreen
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetViewAxis( False )                                           # Turn off axis marker in corner of viewscreen

    \endcode
    \endPythonOnly
    \param [in] vaxis bool True to show the axis, false to hide the axis
*/

extern void SetViewAxis( bool vaxis );

/*!
    \ingroup Visualization
*/
/*!
    Toggle viewing the border frame
    \forcpponly
    \code{.cpp}
    SetShowBorders( false );                                        // Turn off red/black border on active window
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetShowBorders( False )                                        # Turn off red/black border on active window

    \endcode
    \endPythonOnly
    \param [in] brdr bool True to show the border frame, false to hide the border frame
*/

extern void SetShowBorders( bool brdr );

/*!
    \ingroup Visualization
*/
/*!
    Set the draw type of the specified geometry
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD", "" );                             // Add Pod for testing

    SetGeomDrawType( pid, GEOM_DRAW_SHADE );                       // Make pod appear as shaded
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD", "" )                             # Add Pod for testing

    SetGeomDrawType( pid, GEOM_DRAW_SHADE )                       # Make pod appear as shaded

    \endcode
    \endPythonOnly
    \sa DRAW_TYPE
    \param [in] geom_id string Geom ID
    \param [in] type int Draw type enum (i.e. GEOM_DRAW_SHADE)
*/

extern void SetGeomDrawType(const string &geom_id, int type);

/*!
    \ingroup Visualization
*/
/*!
    Set the wireframe color of the specified geometry
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD", "" );

    SetGeomWireColor( pid, 0, 0, 255 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD", "" )

    SetGeomWireColor( pid, 0, 0, 255 )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] r int Red component of color [0, 255]
    \param [in] g int Green component of color [0, 255]
    \param [in] b int Blue component of color [0, 255]
*/

extern void SetGeomWireColor( const string &geom_id, int r, int g, int b );

/*!
    \ingroup Visualization
*/
/*!
    Set the display type of the specified geometry
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD" );                             // Add Pod for testing

    SetGeomDisplayType( pid, DISPLAY_DEGEN_PLATE );                       // Make pod appear as Bezier plate (Degen Geom)
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD" )                             # Add Pod for testing

    SetGeomDisplayType( pid, DISPLAY_DEGEN_PLATE )                       # Make pod appear as Bezier plate (Degen Geom)

    \endcode
    \endPythonOnly
    \sa DISPLAY_TYPE
    \param [in] geom_id string Geom ID
    \param [in] type int Display type enum (i.e. DISPLAY_BEZIER)
*/

extern void SetGeomDisplayType(const string &geom_id, int type);

/*!
    \ingroup Visualization
*/
/*!
    Set the visualization material the specified geometry
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD" );

    SetGeomMaterialName( pid, "Ruby" );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD" )

    SetGeomMaterialName( pid, "Ruby" )
    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Material name
*/

extern void SetGeomMaterialName( const string &geom_id, const string &name );

/*!
    \ingroup Visualization
*/
/*!
    Set the visualization material the specified geometry
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD" );

    AddMaterial( "RedGlass", vec3d( 44, 2, 2 ), vec3d( 156, 10, 10 ), vec3d( 185, 159, 159 ), vec3d( 44, 2, 2 ), 30, 0.4 );

    SetGeomMaterialName( pid, "RedGlass" );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD" )

    AddMaterial( "RedGlass", vec3d( 44, 2, 2 ), vec3d( 156, 10, 10 ), vec3d( 185, 159, 159 ), vec3d( 44, 2, 2 ), 30, 0.4 )

    SetGeomMaterialName( pid, "RedGlass" )
    \endcode
    \endPythonOnly
    \param [in] name string Material name
    \param [in] ambient vec3d Ambient color RGB triple on scale [0, 255]
    \param [in] diffuse vec3d Diffuse color RGB triple on scale [0, 255]
    \param [in] specular vec3d Specular color RGB triple on scale [0, 255]
    \param [in] emissive vec3d Emissive color RGB triple on scale [0, 255]
    \param [in] shininess double Shininess exponent on scale [0, 127]
    \param [in] alpha double Transparency factor on scale [0, 1]
*/

extern void AddMaterial( const string &name, const vec3d & ambient, const vec3d & diffuse, const vec3d & specular, const vec3d & emissive, const double & alpha, const double & shininess );

/*!
    \ingroup Visualization
*/
/*!
    Get the names of all visualization materials
    \forcpponly
    \code{.cpp}
    array< string > @mat_array = GetMaterialNames();
    if ( mat_array.length() == 0 )
    {
        Print( "ERROR: GetMaterialNames returned nothing" );
        __failure++;
    }

    for ( int i = 0; i < int( mat_array.size() ); i++ )
    {
        Print( mat_array[i] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    mat_array = GetMaterialNames()
    assert len( mat_array ) > 0, "GetMaterialNames returned nothing"

    for i in range(int( len(mat_array) )):
        print( mat_array[i] )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of material names
*/

extern vector < string > GetMaterialNames();

/*!
    \ingroup Visualization
*/
/*!
    Set the background color
    \forcpponly
    \code{.cpp}
    SetBackground( 1.0, 1.0, 1.0 );                                 // Set background to bright white
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetBackground( 1.0, 1.0, 1.0 )                                 # Set background to bright white

    \endcode
    \endPythonOnly
    \param [in] r double Red 8-bit unsigned integer (range: 0-255)
    \param [in] g double Green 8-bit unsigned integer (range: 0-255)
    \param [in] b double Blue 8-bit unsigned integer (range: 0-255)
*/

extern void SetBackground( double r, double g, double b );

/*!
    \ingroup Visualization
*/
/*!
    Set the view of all viewports
    \forcpponly
    \code{.cpp}
    SetAllViews( CAM_CENTER );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetAllViews( CAM_CENTER )

    \endcode
    \endPythonOnly
    \param [in] view int CAMERA_VIEW enum
*/

extern void SetAllViews( int view );

/*!
    \ingroup Visualization
*/
/*!
    Set the view of a particular viewports
    \forcpponly
    \code{.cpp}
    SetView( 0, CAM_CENTER );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetView( 0, CAM_CENTER )

    \endcode
    \endPythonOnly
    \param [in] viewport int Viewport to set view
    \param [in] view int CAMERA_VIEW enum
*/

extern void SetView( int viewport, int view );

/*!
    \ingroup Visualization
*/
/*!
    Fit contents to all viewports
    \forcpponly
    \code{.cpp}
    FitAllViews();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    FitAllViews()

    \endcode
    \endPythonOnly
*/

extern void FitAllViews();

/*!
    \ingroup Visualization
*/
/*!
    Reset views of all viewports
    \forcpponly
    \code{.cpp}
    ResetViews();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ResetViews()

    \endcode
    \endPythonOnly
*/

extern void ResetViews();

/*!
    \ingroup Visualization
*/
/*!
    Set the rows and columns of the window layout
    \forcpponly
    \code{.cpp}
    SetWindowLayout( 2, 2 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetWindowLayout( 2, 2 )

    \endcode
    \endPythonOnly
    \param [in] r int Number of viewport rows
    \param [in] c int Number of viewport columns
*/

extern void SetWindowLayout( int r, int c );

/*!
    \ingroup Visualization
*/
/*!
    Set whether all instances of GUI device type are disabled
    \forcpponly
    \code{.cpp}
    SetGUIElementDisable( GDEV_INPUT, true );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetGUIElementDisable( GDEV_INPUT, True )
    \endcode
    \endPythonOnly
    \param [in] e int GDEV enum for GUI device type
    \param [in] state bool True to disable GUI device type
*/

extern void SetGUIElementDisable( int e, bool state );

/*!
    \ingroup Visualization
*/
/*!
    Set whether screen is disabled
    \forcpponly
    \code{.cpp}
    SetGUIScreenDisable( VSP_CFD_MESH_SCREEN, true );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetGUIScreenDisable( VSP_CFD_MESH_SCREEN, True )
    \endcode
    \endPythonOnly
    \param [in] s int GUI_VSP_SCREEN enum for screen
    \param [in] state bool True to disable screen
*/

extern void SetGUIScreenDisable( int s, bool state );

/*!
    \ingroup Visualization
*/
/*!
    Set whether geom screen is disabled
    \forcpponly
    \code{.cpp}
    SetGeomScreenDisable( ALL_GEOM_SCREENS, true );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetGeomScreenDisable( ALL_GEOM_SCREENS, True )
    \endcode
    \endPythonOnly
    \param [in] s int GUI_GEOM_SCREEN enum for geom screen
    \param [in] state bool True to disable geom screen
*/
extern void SetGeomScreenDisable( int s, bool state );

/*!
    \ingroup Visualization
*/
/*!
    Hide an OpenVSP GUI screen
    \forcpponly
    \code{.cpp}
    HideScreen( VSP_CFD_MESH_SCREEN );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    HideScreen( VSP_CFD_MESH_SCREEN )
    \endcode
    \endPythonOnly
    \param [in] s int GUI_VSP_SCREEN enum for screen
*/

extern void HideScreen( int s );

/*!
    \ingroup Visualization
*/
/*!
    Show an OpenVSP GUI screen
    \forcpponly
    \code{.cpp}
    ShowScreen( VSP_CFD_MESH_SCREEN );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ShowScreen( VSP_CFD_MESH_SCREEN )
    \endcode
    \endPythonOnly
    \param [in] s int GUI_VSP_SCREEN enum for screen
*/

extern void ShowScreen( int s );


//======================== Geom Functions ================================//
/*!
    \ingroup Geom
*/
/*!
    Get an array of all Geom types (i.e FUSELAGE, POD, etc.)
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pod1 = AddGeom( "POD", "" );
    string pod2 = AddGeom( "POD", "" );

    array< string > @type_array = GetGeomTypes();

    if ( type_array[0] != "POD" )                { Print( "---> Error: API GetGeomTypes  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pod1 = AddGeom( "POD", "" )
    pod2 = AddGeom( "POD", "" )

    type_array = GetGeomTypes()

    if ( type_array[0] != "POD" ):
        print( "---> Error: API GetGeomTypes  " )
        assert False, "---> Error: API GetGeomTypes"

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of Geom type names
*/

extern std::vector<std::string> GetGeomTypes();

/*!
    \ingroup Geom
*/
/*!
    Add a new Geom of given type as a child of the specified parent. If no parent or an invalid parent is given, the Geom is placed at the top level
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    if ( wing_id.length() == 0 || wing_id == "NONE" )
    {
        Print( "ERROR: AddGeom returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    \endcode
    \endPythonOnly
    \param [in] type string Geom type (i.e FUSELAGE, POD, etc.)
    \param [in] parent string Parent Geom ID
    \return string Geom ID
*/

extern std::string AddGeom( const std::string & type, const std::string & parent = std::string() );

/*!
    \ingroup Geom
*/
/*!
    Perform an update for the specified Geom
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    Update();

    vec3d before_min = GetGeomBBoxMin( pod_id, 0, true );

    SetParmVal( pod_id, "X_Rel_Location", "XForm", 5.0 );

    UpdateGeom( pod_id ); // Faster than updating the whole vehicle

    // Updating just this Geom has to move it, the same as a full Update would.
    vec3d after_min = GetGeomBBoxMin( pod_id, 0, true );

    if ( !closeTo( after_min.x() - before_min.x(), 5.0, 1e-6 ) )
    {
        Print( "ERROR: UpdateGeom did not rebuild the Geom" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    Update()

    before_min = GetGeomBBoxMin( pod_id, 0, True )

    SetParmVal( pod_id, "X_Rel_Location", "XForm", 5.0 )

    UpdateGeom( pod_id ) # Faster than updating the whole vehicle

    # Updating just this Geom has to move it, the same as a full Update would.
    after_min = GetGeomBBoxMin( pod_id, 0, True )

    assert abs( ( after_min.x() - before_min.x() ) - 5.0 ) < 1e-6, "UpdateGeom did not rebuild the Geom"

    \endcode
    \endPythonOnly
    \sa Update()
    \param [in] geom_id string Geom ID
*/

extern void UpdateGeom( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Delete a particular Geom
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    int num_before_del = FindGeoms().length();
    DeleteGeom( wing_id );
    if ( FindGeoms().length() >= num_before_del )
    {
        Print( "ERROR: DeleteGeom removed nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    num_before_del = len( FindGeoms() )
    DeleteGeom( wing_id )
    assert len( FindGeoms() ) < num_before_del, "DeleteGeom removed nothing"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
*/

extern void DeleteGeom( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Delete multiple Geoms
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    string rid = ExecAnalysis( "CompGeom" );

    array<string>@ mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" );

    int num_before_del = FindGeoms().length();
    DeleteGeomVec( mesh_id_vec );
    if ( FindGeoms().length() >= num_before_del )
    {
        Print( "ERROR: DeleteGeomVec removed nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    rid = ExecAnalysis( "CompGeom" )

    mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" )

    num_before_del = len( FindGeoms() )
    DeleteGeomVec( mesh_id_vec )
    assert len( FindGeoms() ) < num_before_del, "DeleteGeomVec removed nothing"


    \endcode
    \endPythonOnly
    \param [in] del_vec vector<string> Vector of Geom IDs
*/

extern void DeleteGeomVec( const std::vector< std::string > & del_vec );

/*!
    \ingroup Geom
*/
/*!
    Cut Geom from current location and store on clipboard
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pid1 = AddGeom( "POD", "" );
    string pid2 = AddGeom( "POD", "" );

    CutGeomToClipboard( pid1 );

    PasteGeomClipboard( pid2 ); // Paste Pod 1 as child of Pod 2

    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 2 )                { Print( "---> Error: API Cut/Paste Geom  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pid1 = AddGeom( "POD", "" )
    pid2 = AddGeom( "POD", "" )

    CutGeomToClipboard( pid1 )

    PasteGeomClipboard( pid2 ) # Paste Pod 1 as child of Pod 2

    geom_ids = FindGeoms()

    if  len(geom_ids) != 2 :
        print( "---> Error: API Cut/Paste Geom  " )
        assert False, "---> Error: API Cut/Paste Geom"

    \endcode
    \endPythonOnly
    \sa PasteGeomClipboard
    \param [in] geom_id string Geom ID
*/

extern void CutGeomToClipboard( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Copy Geom from current location and store on clipboard
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pid1 = AddGeom( "POD", "" );
    string pid2 = AddGeom( "POD", "" );

    CopyGeomToClipboard( pid1 );

    PasteGeomClipboard( pid2 ); // Paste Pod 1 as child of Pod 2

    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 3 )                { Print( "---> Error: API Copy/Paste Geom  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pid1 = AddGeom( "POD", "" )
    pid2 = AddGeom( "POD", "" )

    CopyGeomToClipboard( pid1 )

    PasteGeomClipboard( pid2 ) # Paste Pod 1 as child of Pod 2

    geom_ids = FindGeoms()

    if  len(geom_ids) != 3 :
        print( "---> Error: API Copy/Paste Geom  " )
        assert False, "---> Error: API Copy/Paste Geom"

    \endcode
    \endPythonOnly
    \sa PasteGeomClipboard
    \param [in] geom_id string Geom ID
*/

extern void CopyGeomToClipboard( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Paste Geom from clipboard into the model. The Geom is pasted as a child of the specified parent, but will be placed at top level if no parent or an invalid one is provided.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pid1 = AddGeom( "POD", "" );
    string pid2 = AddGeom( "POD", "" );

    CutGeomToClipboard( pid1 );

    PasteGeomClipboard( pid2 ); // Paste Pod 1 as child of Pod 2

    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 2 )                { Print( "---> Error: API Cut/Paste Geom  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pid1 = AddGeom( "POD", "" )
    pid2 = AddGeom( "POD", "" )

    CutGeomToClipboard( pid1 )

    PasteGeomClipboard( pid2 ) # Paste Pod 1 as child of Pod 2

    geom_ids = FindGeoms()

    if  len(geom_ids) != 2 :
        print( "---> Error: API Cut/Paste Geom  " )
        assert False, "---> Error: API Cut/Paste Geom"

    \endcode
    \endPythonOnly
    \param [in] parent string Parent Geom ID
    \return vector\<string\> Vector of pasted Geom IDs
*/

extern std::vector<std::string> PasteGeomClipboard( const std::string & parent = std::string() );

/*!
    \ingroup Geom
*/
/*!
    Find and return all Geom IDs in the model
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometries ====//
    string pod1 = AddGeom( "POD", "" );
    string pod2 = AddGeom( "POD", "" );

    //==== There Should Be Two Geoms =====//
    array< string > @geom_ids = FindGeoms();

    if ( geom_ids.size() != 2 )                        { Print( "---> Error: API FindGeoms " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pod1 = AddGeom( "POD", "" )
    pod2 = AddGeom( "POD", "" )

    #==== There Should Be Two Geoms =====//
    geom_ids = FindGeoms()

    if  len(geom_ids) != 2 :
        print( "---> Error: API FindGeoms " )
        assert False, "---> Error: API FindGeoms"

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of all Geom IDs
*/

extern std::vector<std::string> FindGeoms();

/*!
    \ingroup Geom
*/
/*!
    Find and return all Geom IDs with the specified name
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    array< string > @geom_ids = FindGeomsWithName( "ExamplePodName" );

    if ( geom_ids.size() != 1 )
    {
        Print( "---> Error: API FindGeomsWithName " );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    SetGeomName( pid, "ExamplePodName" )

    geom_ids = FindGeomsWithName( "ExamplePodName" )

    if  len(geom_ids) != 1 :
        print( "---> Error: API FindGeomsWithName " )
        assert False, "---> Error: API FindGeomsWithName"

    \endcode
    \endPythonOnly
    \sa FindGeom
    \param [in] name string Geom name
    \return vector\<string\> Array of Geom IDs
*/

extern std::vector<std::string> FindGeomsWithName( const std::string & name );

/*!
    \ingroup Geom
*/
/*!
    Find and return the Geom ID with the specified name at given index. Equivalent to FindGeomsWithName( name )[index].
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    string geom_id = FindGeom( "ExamplePodName", 0 );

    array< string > @geom_ids = FindGeomsWithName( "ExamplePodName" );

    if ( geom_ids[0] != geom_id )
    {
        Print( "---> Error: API FindGeom & FindGeomsWithName" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    SetGeomName( pid, "ExamplePodName" )

    geom_id = FindGeom( "ExamplePodName", 0 )

    geom_ids = FindGeomsWithName( "ExamplePodName" )

    if  geom_ids[0] != geom_id :
        print( "---> Error: API FindGeom & FindGeomsWithName" )
        assert False, "---> Error: API FindGeom & FindGeomsWithName"

    \endcode
    \endPythonOnly
    \sa FindGeomsWithName
    \param [in] name string Geom name
    \param [in] index
    \return string Geom ID with name at specified index
*/

extern std::string FindGeom( const std::string & name, int index );

/*!
    \ingroup Geom
*/
/*!
    Set the name of the specified Geom
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    array< string > @geom_ids = FindGeomsWithName( "ExamplePodName" );

    if ( geom_ids.size() != 1 )
    {
        Print( "---> Error: API FindGeomsWithName " );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    SetGeomName( pid, "ExamplePodName" )

    geom_ids = FindGeomsWithName( "ExamplePodName" )

    if  len(geom_ids) != 1 :
        print( "---> Error: API FindGeomsWithName " )
        assert False, "---> Error: API FindGeomsWithName"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Geom name
*/

extern void SetGeomName( const std::string & geom_id, const std::string & name );

/*!
    \ingroup Geom
*/
/*!
    Get the name of a specific Geom
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    SetGeomName( pid, "ExamplePodName" );

    string name_str = "Geom Name: " + GetGeomName( pid );

    Print( name_str );

    if ( GetGeomName( pid ) != "ExamplePodName" )
    {
        Print( "ERROR: GetGeomName did not report the name that was set" );
        __failure++;
    }

    // The name is how FindGeomsWithName looks Geoms up.
    array< string > @found = FindGeomsWithName( "ExamplePodName" );

    if ( found.size() != 1 || found[0] != pid )
    {
        Print( "ERROR: GetGeomName disagrees with FindGeomsWithName" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    SetGeomName( pid, "ExamplePodName" )

    name_str = "Geom Name: " + GetGeomName( pid )

    print( name_str )

    assert GetGeomName( pid ) == "ExamplePodName", "GetGeomName did not report the name that was set"

    # The name is how FindGeomsWithName looks Geoms up.
    found = FindGeomsWithName( "ExamplePodName" )

    assert len( found ) == 1, "GetGeomName disagrees with FindGeomsWithName"
    assert found[0] == pid, "GetGeomName disagrees with FindGeomsWithName"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return string Geom name
*/

extern std::string GetGeomName( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Get all Parm IDs associated with this Geom Parm container
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD", "" );

    Print( string( "---> Test Get Parm Arrays" ) );

    array< string > @parm_array = GetGeomParmIDs( pid );

    if ( parm_array.size() < 1 )            { Print( "---> Error: API GetGeomParmIDs " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    print( "---> Test Get Parm Arrays" )

    parm_array = GetGeomParmIDs( pid )

    if  len(parm_array) < 1 :
        print( "---> Error: API GetGeomParmIDs " )
        assert False, "---> Error: API GetGeomParmIDs"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return vector\<string\> Array of Parm IDs
*/

extern std::vector<std::string> GetGeomParmIDs( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Get the type name of specified Geom (i.e. FUSELAGE)
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    Print( "Geom Type Name: ", false );

    Print( GetGeomTypeName( wing_id ) );

    if ( GetGeomTypeName( wing_id ) != "Wing" )
    {
        Print( "ERROR: GetGeomTypeName did not report the type that was added" );
        __failure++;
    }

    // A Geom of a different type has to report a different name.
    string pod_id = AddGeom( "POD" );

    if ( GetGeomTypeName( pod_id ) == GetGeomTypeName( wing_id ) )
    {
        Print( "ERROR: GetGeomTypeName gave two types the same name" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    print( "Geom Type Name: ", False )

    print( GetGeomTypeName( wing_id ) )

    assert GetGeomTypeName( wing_id ) == "Wing", "GetGeomTypeName did not report the type that was added"

    # A Geom of a different type has to report a different name.
    pod_id = AddGeom( "POD" )

    assert GetGeomTypeName( pod_id ) != GetGeomTypeName( wing_id ), "GetGeomTypeName gave two types the same name"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return string Geom type name
*/

extern std::string GetGeomTypeName( const std::string & geom_id );

/*!
    \ingroup Parm
*/
/*!
    Get Parm ID
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    string lenid = GetParm( pid, "Length", "Design" );

    if ( !ValidParm( lenid ) )                { Print( "---> Error: API GetParm  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    lenid = GetParm( pid, "Length", "Design" )

    if  not ValidParm( lenid ) :
        print( "---> Error: API GetParm  " )
        assert False, "---> Error: API GetParm"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Parm name
    \param [in] group string Parm group name
    \return string Parm ID
*/

extern std::string GetParm( const std::string & geom_id, const std::string & name, const std::string & group );

/*!
    \ingroup Geom
*/
/*!
    Get the parent Geom ID for the input child Geom. "NONE" is returned if the Geom has no parent.
    \forcpponly
    \code{.cpp}
    //==== Reparent two PodGeoms ====//
    string pod1 = AddGeom( "POD" );
    string pod2 = AddGeom( "POD", pod1 );
    string pod3 = AddGeom ("POD" );

    string veh_id = GetVehicleID();

    SetGeomParent( pod2, veh_id );
    SetGeomParent( pod3, pod1 );

    string pod2_parent = GetGeomParent( pod2 );
    string pod3_parent = GetGeomParent( pod3 );

    if ( pod2_parent != string("NONE") || pod3_parent != pod1 )
    {
        Print( "SetGeomParent error!" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Reparent two PodGeoms ====#
    pod1 = AddGeom( "POD" )
    pod2 = AddGeom( "POD", pod1 )
    pod3 = AddGeom ("POD" )

    veh_id = GetVehicleID()

    SetGeomParent( pod2, veh_id )
    SetGeomParent( pod3, pod1 )

    pod2_parent = GetGeomParent( pod2 )
    pod3_parent = GetGeomParent( pod3 )

    if ( pod2_parent != "NONE" or pod3_parent != pod1 ):
        print( "SetGeomParent error!" )
        assert False, "SetGeomParent error!"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] parent_id string Parent Geom ID
*/

extern void SetGeomParent( const std::string& geom_id, const std::string& parent_id );

/*!
    \ingroup Geom
*/
/*!
    Get the parent Geom ID for the input child Geom. "NONE" is returned if the Geom has no parent.
    \forcpponly
    \code{.cpp}
    //==== Add Parent and Child Geometry ====//
    string pod1 = AddGeom( "POD" );

    string pod2 = AddGeom( "POD", pod1 );

    Print( "Parent ID of Pod #2: ", false );

    Print( GetGeomParent( pod2 ) );

    if ( GetGeomParent( pod2 ) != pod1 )
    {
        Print( "ERROR: GetGeomParent did not report the parent it was given" );
        __failure++;
    }

    // The relationship has to read the same from the other end.
    array< string > @children = GetGeomChildren( pod1 );

    if ( children.size() != 1 || children[0] != pod2 )
    {
        Print( "ERROR: GetGeomParent disagrees with GetGeomChildren" );
        __failure++;
    }

    // A Geom added with no parent sits at the top level.
    if ( GetGeomParent( pod1 ) != "NONE" )
    {
        Print( "ERROR: GetGeomParent did not report NONE for a top level Geom" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Parent and Child Geometry ====//
    pod1 = AddGeom( "POD" )

    pod2 = AddGeom( "POD", pod1 )

    print( "Parent ID of Pod #2: ", False )

    print( GetGeomParent( pod2 ) )

    assert GetGeomParent( pod2 ) == pod1, "GetGeomParent did not report the parent it was given"

    # The relationship has to read the same from the other end.
    children = GetGeomChildren( pod1 )

    assert len( children ) == 1, "GetGeomParent disagrees with GetGeomChildren"
    assert children[0] == pod2, "GetGeomParent disagrees with GetGeomChildren"

    # A Geom added with no parent sits at the top level.
    assert GetGeomParent( pod1 ) == "NONE", "GetGeomParent did not report NONE for a top level Geom"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return string Parent Geom ID
*/

extern std::string GetGeomParent( const std::string& geom_id );

/*!
    \ingroup Geom
*/
/*!
    Get the IDs for each child of the input parent Geom.
    \forcpponly
    \code{.cpp}
    //==== Add Parent and Child Geometry ====//
    string pod1 = AddGeom( "POD" );

    string pod2 = AddGeom( "POD", pod1 );

    string pod3 = AddGeom( "POD", pod2 );

    Print( "Children of Pod #1: " );

    array<string> children = GetGeomChildren( pod1 );
    if ( children.length() == 0 )
    {
        Print( "ERROR: GetGeomChildren returned nothing" );
        __failure++;
    }

    for ( int i = 0; i < int( children.size() ); i++ )
    {
        Print( "\t", false );
        Print( children[i] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Parent and Child Geometry ====//
    pod1 = AddGeom( "POD" )

    pod2 = AddGeom( "POD", pod1 )

    pod3 = AddGeom( "POD", pod2 )

    print( "Children of Pod #1: " )

    children = GetGeomChildren( pod1 )
    assert len( children ) > 0, "GetGeomChildren returned nothing"

    for i in range(int( len(children) )):

        print( "\t", False )
        print( children[i] )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return vector\<string\> Vector of child Geom IDs
*/

extern std::vector< std::string > GetGeomChildren( const std::string& geom_id );

/*!
    \ingroup XSecSurf
*/
/*!
    Get the number of XSecSurfs for the specified Geom
    \forcpponly
    \code{.cpp}
    //==== Add Fuselage Geometry ====//
    string fuseid = AddGeom( "FUSELAGE", "" );

    int num_xsec_surfs = GetNumXSecSurfs( fuseid );

    if ( num_xsec_surfs != 1 )                { Print( "---> Error: API GetNumXSecSurfs  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Fuselage Geometry ====//
    fuseid = AddGeom( "FUSELAGE", "" )

    num_xsec_surfs = GetNumXSecSurfs( fuseid )

    if  num_xsec_surfs != 1 :
        print( "---> Error: API GetNumXSecSurfs  " )
        assert False, "---> Error: API GetNumXSecSurfs"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return int Number of XSecSurfs
*/

extern int GetNumXSecSurfs( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Get the number of main surfaces for the specified Geom. Multiple main surfaces may exist for CustoGeoms, propellors, etc., but
    does not include surfaces created due to symmetry.
    \forcpponly
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    int num_surf = 0;

    num_surf = GetNumMainSurfs( prop_id ); // Should be the same as the number of blades

    if ( num_surf != GetIntParmVal( FindParm( prop_id, "NumBlade", "Design" ) ) )
    {
        Print( "ERROR: GetNumMainSurfs does not match the blade count" );
        __failure++;
    }

    Print( "Number of Propeller Surfaces: ", false );

    Print( num_surf );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    num_surf = 0

    num_surf = GetNumMainSurfs( prop_id ) # Should be the same as the number of blades

    assert num_surf == GetIntParmVal( FindParm( prop_id, "NumBlade", "Design" ) ), "GetNumMainSurfs does not match the blade count"

    print( "Number of Propeller Surfaces: ", False )

    print( num_surf )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return int Number of main surfaces
*/

extern int GetNumMainSurfs( const std::string & geom_id );

/*!
    \ingroup Geom
*/
/*!
    Get the total number of surfaces for the specified Geom. This is equivalent to the number of main surface multiplied
    by the number of symmetric copies.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    int num_surf = 0;

    num_surf = GetTotalNumSurfs( wing_id ); // Wings default with XZ symmetry on -> 2 surfaces

    if ( num_surf != 2 )
    {
        Print( "ERROR: GetTotalNumSurfs, expected 2 for a symmetric wing" );
        __failure++;
    }

    Print( "Total Number of Wing Surfaces: ", false );

    Print( num_surf );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    num_surf = 0

    num_surf = GetTotalNumSurfs( wing_id ) # Wings default with XZ symmetry on -> 2 surfaces

    assert num_surf == 2, "GetTotalNumSurfs, expected 2 for a symmetric wing"

    print( "Total Number of Wing Surfaces: ", False )

    print( num_surf )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return int Number of main surfaces
*/

extern int GetTotalNumSurfs( const std::string& geom_id );

/*!
    \ingroup Geom
*/
/*!
    Get the VSP surface type of the specified Geom
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    if ( GetGeomVSPSurfType( wing_id ) != WING_SURF )
    {
        Print( "---> Error: API GetGeomVSPSurfType " );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    if  GetGeomVSPSurfType( wing_id ) != WING_SURF :
        print( "---> Error: API GetGeomVSPSurfType " )
        assert False, "---> Error: API GetGeomVSPSurfType"

    \endcode
    \endPythonOnly
    \sa VSP_SURF_TYPE
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind int Main surface index
    \return int VSP surface type enum (i.e. DISK_SURF)
*/

extern int GetGeomVSPSurfType( const std::string& geom_id, int main_surf_ind = 0 );

/*!
    \ingroup Geom
*/
/*!
    Get the VSP surface CFD type of the specified Geom
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    if ( GetGeomVSPSurfCfdType( wing_id ) != CFD_NORMAL )
    {
        Print( "---> Error: API GetGeomVSPSurfCfdType " );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    if  GetGeomVSPSurfCfdType( wing_id ) != CFD_NORMAL :
        print( "---> Error: API GetGeomVSPSurfCfdType " )
        assert False, "---> Error: API GetGeomVSPSurfCfdType"

    \endcode
    \endPythonOnly
    \sa VSP_SURF_CFD_TYPE
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind int Main surface index
    \return int VSP surface CFD type enum (i.e. CFD_TRANSPARENT)
*/

extern int GetGeomVSPSurfCfdType( const std::string& geom_id, int main_surf_ind = 0 );

/*!
    \ingroup Geom
*/
/*!
    Get the the maximum coordinate of the bounding box of a Geom with given main surface index. The Geom bounding
    box may be specified in absolute or body reference frame.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    SetParmVal( FindParm( pid, "Y_Rotation", "XForm" ), 45 );
    SetParmVal( FindParm( pid, "Z_Rotation", "XForm" ), 25 );

    Update();

    vec3d max_pnt = GetGeomBBoxMax( pid, 0, false );

    vec3d min_pnt = GetGeomBBoxMin( pid, 0, false );

    if ( max_pnt.x() <= min_pnt.x() || max_pnt.y() <= min_pnt.y() || max_pnt.z() <= min_pnt.z() )
    {
        Print( "ERROR: GetGeomBBoxMax is not above the minimum corner" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    SetParmVal( FindParm( pid, "Y_Rotation", "XForm" ), 45 )
    SetParmVal( FindParm( pid, "Z_Rotation", "XForm" ), 25 )

    Update()

    max_pnt = GetGeomBBoxMax( pid, 0, False )

    min_pnt = GetGeomBBoxMin( pid, 0, False )

    assert max_pnt.x() > min_pnt.x() and max_pnt.y() > min_pnt.y() and max_pnt.z() > min_pnt.z(), "GetGeomBBoxMax is not above the minimum corner"

    \endcode
    \endPythonOnly
    \sa GetGeomBBoxMin
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind int Main surface index
    \param [in] ref_frame_is_absolute bool Flag to specify absolute or body reference frame
    \return vec3d Maximum coordinate of the bounding box
*/

extern vec3d GetGeomBBoxMax( const std::string& geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true );

/*!
    \ingroup Geom
*/
/*!
    Get the the minimum coordinate of the bounding box of a Geom with given main surface index. The Geom bounding
    box may be specified in absolute or body reference frame.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    SetParmVal( FindParm( pid, "Y_Rotation", "XForm" ), 45 );
    SetParmVal( FindParm( pid, "Z_Rotation", "XForm" ), 25 );

    Update();

    vec3d min_pnt = GetGeomBBoxMin( pid, 0, false );

    vec3d max_pnt = GetGeomBBoxMax( pid, 0, false );

    if ( min_pnt.x() >= max_pnt.x() || min_pnt.y() >= max_pnt.y() || min_pnt.z() >= max_pnt.z() )
    {
        Print( "ERROR: GetGeomBBoxMin is not below the maximum corner" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    SetParmVal( FindParm( pid, "Y_Rotation", "XForm" ), 45 )
    SetParmVal( FindParm( pid, "Z_Rotation", "XForm" ), 25 )

    Update()

    min_pnt = GetGeomBBoxMin( pid, 0, False )

    max_pnt = GetGeomBBoxMax( pid, 0, False )

    assert min_pnt.x() < max_pnt.x() and min_pnt.y() < max_pnt.y() and min_pnt.z() < max_pnt.z(), "GetGeomBBoxMin is not below the maximum corner"

    \endcode
    \endPythonOnly
    \sa GetGeomBBoxMax
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind int Main surface index
    \param [in] ref_frame_is_absolute bool Flag to specify absolute or body reference frame
    \return vec3d Minimum coordinate of the bounding box
*/

extern vec3d GetGeomBBoxMin( const std::string& geom_id, int main_surf_ind = 0, bool ref_frame_is_absolute = true );


//======================== SubSurface Functions ================================//
/*!
    \ingroup SubSurface
*/
/*!
    Add a sub-surface to the specified Geom
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    // Note: Parm Group for SubSurfaces in the form: "SS_" + type + "_" + count (initialized at 1)
    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line

    SetParmVal( wid, "Const_Line_Value", "SubSurface_1", 0.4 );     // Change Location

    if ( ss_line_id.length() == 0 || ss_line_id == "NONE" )
    {
        Print( "ERROR: AddSubSurf returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    # Note: Parm Group for SubSurfaces in the form: "SS_" + type + "_" + count (initialized at 1)
    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line

    SetParmVal( wid, "Const_Line_Value", "SubSurface_1", 0.4 )     # Change Location

    \endcode
    \endPythonOnly
    \sa SUBSURF_TYPE
    \param [in] geom_id string Geom ID
    \param [in] type int Sub-surface type enum (i.e. SS_RECTANGLE)
    \param [in] surfindex int Main surface index (default: 0)
    \return string Sub-surface ID
*/

extern std::string AddSubSurf( const std::string & geom_id, int type, int surfindex = 0 );

/*!
    \ingroup SubSurface
*/
/*!
    Get the ID of the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string ss_rec_1 = AddSubSurf( wid, SS_RECTANGLE ); // Add Sub Surface Rectangle #1

    string ss_rec_2 = AddSubSurf( wid, SS_RECTANGLE ); // Add Sub Surface Rectangle #2

    Print( ss_rec_2, false );

    Print( " = ", false );

    Print( GetSubSurf( wid, 1 ) );

    // Sub-surfaces come back in the order they were added.
    if ( GetSubSurf( wid, 0 ) != ss_rec_1 || GetSubSurf( wid, 1 ) != ss_rec_2 )
    {
        Print( "ERROR: GetSubSurf did not report the sub-surfaces in order" );
        __failure++;
    }

    // The index form and the ID vector have to agree.
    array< string > @id_vec = GetSubSurfIDVec( wid );

    if ( id_vec.size() != 2 || id_vec[1] != GetSubSurf( wid, 1 ) )
    {
        Print( "ERROR: GetSubSurf disagrees with GetSubSurfIDVec" );
        __failure++;
    }

    // An index past the end has to be rejected.
    GetSubSurf( wid, 2 );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: GetSubSurf accepted an index past the end" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    ss_rec_1 = AddSubSurf( wid, SS_RECTANGLE ) # Add Sub Surface Rectangle #1

    ss_rec_2 = AddSubSurf( wid, SS_RECTANGLE ) # Add Sub Surface Rectangle #2

    print( ss_rec_2, False )

    print( " = ", False )

    print( GetSubSurf( wid, 1 ) )

    # Sub-surfaces come back in the order they were added.
    assert GetSubSurf( wid, 0 ) == ss_rec_1, "GetSubSurf did not report the sub-surfaces in order"
    assert GetSubSurf( wid, 1 ) == ss_rec_2, "GetSubSurf did not report the sub-surfaces in order"

    # The index form and the ID vector have to agree.
    id_vec = GetSubSurfIDVec( wid )

    assert len( id_vec ) == 2, "GetSubSurf disagrees with GetSubSurfIDVec"
    assert id_vec[1] == GetSubSurf( wid, 1 ), "GetSubSurf disagrees with GetSubSurfIDVec"

    # An index past the end has to be rejected.  The error queue is reached
    # through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    GetSubSurf( wid, 2 )

    assert err_mgr.GetNumTotalErrors() > 0, "GetSubSurf accepted an index past the end"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] index int Sub-surface index
    \return string Sub-surface ID
*/

extern std::string GetSubSurf( const std::string & geom_id, int index );

/*!
    \ingroup SubSurface
*/
/*!
    Get the ID of the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string ss_rec_1 = AddSubSurf( wid, SS_RECTANGLE ); // Add Sub Surface Rectangle #1

    string ss_rec_2 = AddSubSurf( wid, SS_RECTANGLE ); // Add Sub Surface Rectangle #2

    Print( ss_rec_2, false );

    Print( " = ", false );

    Print( GetSubSurf( wid, 1 ) );

    // Sub-surfaces come back in the order they were added.
    if ( GetSubSurf( wid, 0 ) != ss_rec_1 || GetSubSurf( wid, 1 ) != ss_rec_2 )
    {
        Print( "ERROR: GetSubSurf did not report the sub-surfaces in order" );
        __failure++;
    }

    // The index form and the ID vector have to agree.
    array< string > @id_vec = GetSubSurfIDVec( wid );

    if ( id_vec.size() != 2 || id_vec[1] != GetSubSurf( wid, 1 ) )
    {
        Print( "ERROR: GetSubSurf disagrees with GetSubSurfIDVec" );
        __failure++;
    }

    // An index past the end has to be rejected.
    GetSubSurf( wid, 2 );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: GetSubSurf accepted an index past the end" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    ss_rec_1 = AddSubSurf( wid, SS_RECTANGLE ) # Add Sub Surface Rectangle #1

    ss_rec_2 = AddSubSurf( wid, SS_RECTANGLE ) # Add Sub Surface Rectangle #2

    print( ss_rec_2, False )

    print( " = ", False )

    print( GetSubSurf( wid, 1 ) )

    # Sub-surfaces come back in the order they were added.
    assert GetSubSurf( wid, 0 ) == ss_rec_1, "GetSubSurf did not report the sub-surfaces in order"
    assert GetSubSurf( wid, 1 ) == ss_rec_2, "GetSubSurf did not report the sub-surfaces in order"

    # The index form and the ID vector have to agree.
    id_vec = GetSubSurfIDVec( wid )

    assert len( id_vec ) == 2, "GetSubSurf disagrees with GetSubSurfIDVec"
    assert id_vec[1] == GetSubSurf( wid, 1 ), "GetSubSurf disagrees with GetSubSurfIDVec"

    # An index past the end has to be rejected.  The error queue is reached
    # through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    GetSubSurf( wid, 2 )

    assert err_mgr.GetNumTotalErrors() > 0, "GetSubSurf accepted an index past the end"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Sub surface name
    \return vector\<string\> Vector of sub-surface ID
*/

extern std::vector<std::string> GetSubSurf( const std::string & geom_id, const std::string & name );

/*!
    \ingroup SubSurface
*/
/*!
    Delete the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    Print("Delete SS_Line\n");

    int num_before_del = GetNumSubSurf( wid );
    DeleteSubSurf( wid, ss_line_id );
    if ( GetNumSubSurf( wid ) >= num_before_del )
    {
        Print( "ERROR: DeleteSubSurf removed nothing" );
        __failure++;
    }


    int num_ss = GetNumSubSurf( wid );

    string num_str = string("Number of SubSurfaces: ") + formatInt( num_ss, '' ) + string("\n");

    Print( num_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line
    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    print("Delete SS_Line\n")

    num_before_del = GetNumSubSurf( wid )
    DeleteSubSurf( wid, ss_line_id )
    assert GetNumSubSurf( wid ) < num_before_del, "DeleteSubSurf removed nothing"


    num_ss = GetNumSubSurf( wid )

    num_str = f"Number of SubSurfaces: {num_ss}\n"

    print( num_str )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] sub_id string Sub-surface ID
*/

extern void DeleteSubSurf( const std::string & geom_id, const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Delete the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    Print("Delete SS_Line\n");

    int num_before_del = GetNumSubSurf( wid );
    DeleteSubSurf( ss_line_id );
    if ( GetNumSubSurf( wid ) >= num_before_del )
    {
        Print( "ERROR: DeleteSubSurf removed nothing" );
        __failure++;
    }


    int num_ss = GetNumSubSurf( wid );

    string num_str = string("Number of SubSurfaces: ") + formatInt( num_ss, '' ) + string("\n");

    Print( num_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line
    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    print("Delete SS_Line\n")

    num_before_del = GetNumSubSurf( wid )
    DeleteSubSurf( ss_line_id )
    assert GetNumSubSurf( wid ) < num_before_del, "DeleteSubSurf removed nothing"


    num_ss = GetNumSubSurf( wid )

    num_str = f"Number of SubSurfaces: {num_ss}\n"

    print( num_str )

    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
*/

extern void DeleteSubSurf( const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Set the name of the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string new_name = string("New_SS_Rec_Name");

    SetSubSurfName( wid, ss_rec_id, new_name );
    if ( GetSubSurfName( wid, ss_rec_id ) != new_name )
    {
        Print( "ERROR: SetSubSurfName did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    new_name = "New_SS_Rec_Name"

    SetSubSurfName( wid, ss_rec_id, new_name )
    assert GetSubSurfName( wid, ss_rec_id ) == new_name, "SetSubSurfName did not take"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] sub_id string Sub-surface ID
    \param [in] name string Sub-surface name
*/

extern void SetSubSurfName(const std::string & geom_id, const std::string & sub_id, const std::string & name);

/*!
    \ingroup SubSurface
*/
/*!
    Set the name of the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string new_name = string("New_SS_Rec_Name");

    SetSubSurfName( ss_rec_id, new_name );
    if ( GetSubSurfName( ss_rec_id ) != new_name )
    {
        Print( "ERROR: SetSubSurfName did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    new_name = "New_SS_Rec_Name"

    SetSubSurfName( ss_rec_id, new_name )
    assert GetSubSurfName( ss_rec_id ) == new_name, "SetSubSurfName did not take"


    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
    \param [in] name string Sub-surface name
*/

extern void SetSubSurfName( const std::string & sub_id, const std::string & name );

/*!
    \ingroup SubSurface
*/
/*!
    Get the name of the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string rec_name = GetSubSurfName( wid, ss_rec_id );
    if ( rec_name.length() == 0 )
    {
        Print( "ERROR: GetSubSurfName returned nothing" );
        __failure++;
    }

    string name_str = string("Current Name of SS_Rectangle: ") + rec_name + string("\n");

    Print( name_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    rec_name = GetSubSurfName( wid, ss_rec_id )
    assert len( rec_name ) > 0, "GetSubSurfName returned nothing"

    name_str = "Current Name of SS_Rectangle: " + rec_name + "\n"

    print( name_str )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] sub_id string Sub-surface ID
    \return string Sub-surface name
*/

extern std::string GetSubSurfName( const std::string & geom_id, const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get the name of the specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    string rec_name = GetSubSurfName( wid, ss_rec_id );
    if ( rec_name.length() == 0 )
    {
        Print( "ERROR: GetSubSurfName returned nothing" );
        __failure++;
    }

    string name_str = string("Current Name of SS_Rectangle: ") + rec_name + string("\n");

    Print( name_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    rec_name = GetSubSurfName( wid, ss_rec_id )
    assert len( rec_name ) > 0, "GetSubSurfName returned nothing"

    name_str = "Current Name of SS_Rectangle: " + rec_name + "\n"

    print( name_str )

    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
    \return string Sub-surface name
*/

extern std::string GetSubSurfName( const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get the index of the specified sub-surface in its parent Geom's sub-surface vector
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    int ind = GetSubSurfIndex( ss_rec_id );

    string ind_str = string("Index of SS_Rectangle: ") + ind + string("\n");

    Print( ind_str );

    // The rectangle was added second, so it sits at index 1.
    if ( ind != 1 || GetSubSurfIndex( ss_line_id ) != 0 )
    {
        Print( "ERROR: GetSubSurfIndex did not report the order they were added" );
        __failure++;
    }

    // The index has to lead back to the same sub-surface.
    if ( GetSubSurf( wid, ind ) != ss_rec_id )
    {
        Print( "ERROR: GetSubSurfIndex disagrees with GetSubSurf" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line
    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    ind = GetSubSurfIndex( ss_rec_id )

    ind_str = f"Index of SS_Rectangle: {ind}"

    print( ind_str )

    # The rectangle was added second, so it sits at index 1.
    assert ind == 1, "GetSubSurfIndex did not report the order they were added"
    assert GetSubSurfIndex( ss_line_id ) == 0, "GetSubSurfIndex did not report the order they were added"

    # The index has to lead back to the same sub-surface.
    assert GetSubSurf( wid, ind ) == ss_rec_id, "GetSubSurfIndex disagrees with GetSubSurf"

    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
    \return int Sub-surface index
*/

extern int GetSubSurfIndex( const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get a vector of all sub-surface IDs for the specified geometry
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    array<string> id_vec = GetSubSurfIDVec( wid );
    if ( id_vec.length() == 0 )
    {
        Print( "ERROR: GetSubSurfIDVec returned nothing" );
        __failure++;
    }

    string id_type_str = string( "SubSurface IDs and Type Indexes -> ");

    for ( uint i = 0; i < uint(id_vec.length()); i++ )
    {
        id_type_str += id_vec[i];

        id_type_str += string(": ");

        id_type_str += GetSubSurfType(id_vec[i]);

        id_type_str += string("\t");
    }

    id_type_str += string("\n");

    Print( id_type_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line
    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    id_vec = GetSubSurfIDVec( wid )
    assert len( id_vec ) > 0, "GetSubSurfIDVec returned nothing"

    id_type_str = "SubSurface IDs and Type Indexes -> "

    for i in range(len(id_vec)):

        id_type_str += id_vec[i]

        id_type_str += ": "

        id_type_str += f'{GetSubSurfType(id_vec[i])}'

        id_type_str += "\t"

    id_type_str += "\n"

    print( id_type_str )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return vector\<int\> Array of sub-surface IDs
*/

extern std::vector<std::string> GetSubSurfIDVec( const std::string & geom_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get a vector of all sub-surface IDs for the entire model
    \return vector\<string\> Array of sub-surface IDs
*/

extern std::vector<std::string> GetAllSubSurfIDs();

/*!
    \ingroup SubSurface
*/
/*!
    Get the number of sub-surfaces for the specified Geom
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    int num_ss = GetNumSubSurf( wid );

    if ( num_ss != 2 )
    {
        Print( "ERROR: GetNumSubSurf, two were added" );
        __failure++;
    }

    string num_str = string("Number of SubSurfaces: ") + num_ss + string("\n");

    Print( num_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line
    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    num_ss = GetNumSubSurf( wid )

    assert num_ss == 2, "GetNumSubSurf, two were added"

    num_str = "Number of SubSurfaces: {num_ss}"

    print( num_str )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return int Number of Sub-surfaces
*/

extern int GetNumSubSurf( const std::string & geom_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get the type for the specified sub-surface (i.e. SS_RECTANGLE)
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line
    string ss_rec_id = AddSubSurf( wid, SS_RECTANGLE );                        // Add Sub Surface Rectangle

    array<string> id_vec = GetSubSurfIDVec( wid );

    // Each sub-surface has to report the type it was created as.
    if ( GetSubSurfType( ss_line_id ) != SS_LINE || GetSubSurfType( ss_rec_id ) != SS_RECTANGLE )
    {
        Print( "ERROR: GetSubSurfType did not report the type that was added" );
        __failure++;
    }

    string id_type_str = string( "SubSurface IDs and Type Indexes -> ");

    for ( uint i = 0; i < uint(id_vec.length()); i++ )
    {
        id_type_str += id_vec[i];

        id_type_str += string(": ");

        id_type_str += GetSubSurfType(id_vec[i]);

        id_type_str += string("\t");
    }

    id_type_str += string("\n");

    Print( id_type_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line
    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    id_vec = GetSubSurfIDVec( wid )

    # Each sub-surface has to report the type it was created as.
    assert GetSubSurfType( ss_line_id ) == SS_LINE, "GetSubSurfType did not report the type that was added"
    assert GetSubSurfType( ss_rec_id ) == SS_RECTANGLE, "GetSubSurfType did not report the type that was added"

    id_type_str = "SubSurface IDs and Type Indexes -> "

    for i in range(len(id_vec)):

        id_type_str += id_vec[i]

        id_type_str += ": "

        id_type_str += f'{GetSubSurfType(id_vec[i])}'

        id_type_str += "\t"

    id_type_str += "\n"

    print( id_type_str )

    \endcode
    \endPythonOnly
    \sa SUBSURF_TYPE
    \param [in] sub_id string Sub-surface ID
    \return int Sub-surface type enum (i.e. SS_RECTANGLE)
*/

extern int GetSubSurfType( const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get the vector of Parm IDs for specified sub-surface
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string ss_line_id = AddSubSurf( wid, SS_LINE );                      // Add Sub Surface Line

    // Get and list all Parm info for SS_Line
    array<string> parm_id_vec = GetSubSurfParmIDs( ss_line_id );
    if ( parm_id_vec.length() == 0 )
    {
        Print( "ERROR: GetSubSurfParmIDs returned nothing" );
        __failure++;
    }

    for ( uint i = 0; i < uint(parm_id_vec.length()); i++ )
    {
        string id_name_str = string("\tName: ") + GetParmName( parm_id_vec[i] ) + string(", Group: ") + GetParmDisplayGroupName( parm_id_vec[i] ) +
            string(", ID: ") + parm_id_vec[i] + string("\n");

        Print( id_name_str );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_line_id = AddSubSurf( wid, SS_LINE )                      # Add Sub Surface Line

    # Get and list all Parm info for SS_Line
    parm_id_vec = GetSubSurfParmIDs( ss_line_id )
    assert len( parm_id_vec ) > 0, "GetSubSurfParmIDs returned nothing"

    for i in range(len(parm_id_vec)):

        id_name_str = "\tName: " + GetParmName(parm_id_vec[i]) + ", Group: " + GetParmDisplayGroupName(parm_id_vec[i]) + ", ID: " + str(parm_id_vec[i]) + "\n"


        print( id_name_str )

    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
    \return vector\<string\> Vector of Parm IDs
*/

extern std::vector<std::string> GetSubSurfParmIDs( const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Trigger intersection for SS_INTERSECT type subsurfaces
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD", "" );
    string p2id = AddGeom( "POD", "" );

    string xpod2 = GetParm( p2id, "X_Rel_Location", "XForm" );
    SetParmVal( xpod2, 4.0 );

    string zrotpod2 = GetParm( p2id, "Z_Rel_Rotation", "XForm" );
    SetParmVal( zrotpod2, 60.0 );

    string sub_id = AddSubSurf( pid, SS_INTERSECT );

    Update();

    SetIntersectSubSurfGeomID( sub_id, p2id );

    IntersectSubSurf( sub_id );


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD", "" )
    p2id = AddGeom( "POD", "" )

    xpod2 = GetParm( p2id, "X_Rel_Location", "XForm" )
    SetParmVal( xpod2, 4.0 )

    zrotpod2 = GetParm( p2id, "Z_Rel_Rotation", "XForm" )
    SetParmVal( zrotpod2, 60.0 )

    sub_id = AddSubSurf( pid, SS_INTERSECT )

    Update()

    SetIntersectSubSurfGeomID( sub_id, p2id )

    IntersectSubSurf( sub_id )
    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
*/

extern void IntersectSubSurf( const std::string & sub_id );

/*!
    \ingroup SubSurface
*/
/*!
    Set Geom ID to intersect parent with to create intersection SS_INTERSECT type subsurfaces
    \forcpponly
    \code{.cpp}
    string pid = AddGeom( "POD", "" );
    string p2id = AddGeom( "POD", "" );

    string xpod2 = GetParm( p2id, "X_Rel_Location", "XForm" );
    SetParmVal( xpod2, 4.0 );

    string zrotpod2 = GetParm( p2id, "Z_Rel_Rotation", "XForm" );
    SetParmVal( zrotpod2, 60.0 );

    string sub_id = AddSubSurf( pid, SS_INTERSECT );

    Update();

    SetIntersectSubSurfGeomID( sub_id, p2id );

    IntersectSubSurf( sub_id );


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid = AddGeom( "POD", "" )
    p2id = AddGeom( "POD", "" )

    xpod2 = GetParm( p2id, "X_Rel_Location", "XForm" )
    SetParmVal( xpod2, 4.0 )

    zrotpod2 = GetParm( p2id, "Z_Rel_Rotation", "XForm" )
    SetParmVal( zrotpod2, 60.0 )

    sub_id = AddSubSurf( pid, SS_INTERSECT )

    Update()

    SetIntersectSubSurfGeomID( sub_id, p2id )

    IntersectSubSurf( sub_id )
    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
    \param [in] geom_id string Geom ID to intersect with parent to create subsurface
*/

extern void SetIntersectSubSurfGeomID( const std::string & sub_id, const std::string & geom_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Add an FEA Structure to a specified Geom
    \warning init_skin should ALWAYS be set to true.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    // The first structure on this Geom lands at index 0, and its ID has to lead
    // back to that index.
    if ( struct_ind != 0 || NumFeaStructures() != 1 )
    {
        Print( "ERROR: AddFeaStruct did not add a structure" );
        __failure++;
    }

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    if ( struct_id.length() == 0 || GetFeaStructIndex( struct_id ) != struct_ind )
    {
        Print( "ERROR: AddFeaStruct did not give the structure a usable ID" );
        __failure++;
    }

    // init_skin defaults to true, so the structure starts with an FEA Skin.
    if ( NumFeaParts( struct_id ) != 1 )
    {
        Print( "ERROR: AddFeaStruct did not initialize the skin" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    # The first structure on this Geom lands at index 0, and its ID has to lead
    # back to that index.
    assert struct_ind == 0, "AddFeaStruct did not add a structure"
    assert NumFeaStructures() == 1, "AddFeaStruct did not add a structure"

    struct_id = GetFeaStructID( pod_id, struct_ind )

    assert len( struct_id ) > 0, "AddFeaStruct did not give the structure a usable ID"
    assert GetFeaStructIndex( struct_id ) == struct_ind, "AddFeaStruct did not give the structure a usable ID"

    # init_skin defaults to true, so the structure starts with an FEA Skin.
    assert NumFeaParts( struct_id ) == 1, "AddFeaStruct did not initialize the skin"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] init_skin bool Flag to initialize the FEA Structure by creating an FEA Skin from the parent Geom's OML at surfindex
    \param [in] surfindex int Main surface index for the FEA Structure
    \return int FEA Structure index
*/

extern int AddFeaStruct( const std::string & geom_id, bool init_skin = true, int surfindex = 0 );

/*!
    \ingroup FEAMesh
*/
/*!
    Sets FeaMeshMgr m_FeaMeshStructIndex member using passed in index of a FeaStructure
    \forcpponly
    \code{.cpp}

    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    SetFeaMeshStructIndex( struct_ind );

    if ( FindGeoms().size() != 1 ) { Print( "ERROR: SetFeaMeshStructIndex" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    SetFeaMeshStructIndex( struct_ind )

    if  len(FindGeoms()) != 1 :
        print( "ERROR: SetFeaMeshStructIndex" )
        assert False, "ERROR: SetFeaMeshStructIndex"

    \endcode
    \endPythonOnly
*/

extern void SetFeaMeshStructIndex( int struct_index );

/*!
    \ingroup FEAMesh
*/
/*!
    Delete an FEA Structure and all FEA Parts and FEA SubSurfaces associated with it
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind_1 = AddFeaStruct( pod_id );

    int struct_ind_2 = AddFeaStruct( pod_id );

    string struct_id_2 = GetFeaStructID( pod_id, struct_ind_2 );

    if ( NumFeaStructures() != 2 )
    {
        Print( "ERROR: the two structures were not both added" );
        __failure++;
    }

    DeleteFeaStruct( pod_id, struct_ind_1 );

    // Deleting the first structure leaves the second one, which slides down to
    // take its index.
    if ( NumFeaStructures() != 1 )
    {
        Print( "ERROR: DeleteFeaStruct did not remove the structure" );
        __failure++;
    }

    if ( GetFeaStructID( pod_id, 0 ) != struct_id_2 )
    {
        Print( "ERROR: DeleteFeaStruct removed the wrong structure" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind_1 = AddFeaStruct( pod_id )

    struct_ind_2 = AddFeaStruct( pod_id )

    struct_id_2 = GetFeaStructID( pod_id, struct_ind_2 )

    assert NumFeaStructures() == 2, "the two structures were not both added"

    DeleteFeaStruct( pod_id, struct_ind_1 )

    # Deleting the first structure leaves the second one, which slides down to
    # take its index.
    assert NumFeaStructures() == 1, "DeleteFeaStruct did not remove the structure"
    assert GetFeaStructID( pod_id, 0 ) == struct_id_2, "DeleteFeaStruct removed the wrong structure"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
*/

extern void DeleteFeaStruct( const std::string & geom_id, int fea_struct_ind );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the ID of an FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );
    if ( struct_id.length() == 0 )
    {
        Print( "ERROR: GetFeaStructID returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )
    assert len( struct_id ) > 0, "GetFeaStructID returned nothing"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \return string FEA Structure ID
*/

extern std::string GetFeaStructID( const std::string & geom_id, int fea_struct_ind );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the index of an FEA Structure in its Parent Geom's vector of Structures
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind_1 = AddFeaStruct( pod_id );

    int struct_ind_2 = AddFeaStruct( pod_id );

    string struct_id_2 = GetFeaStructID( pod_id, struct_ind_2 );

    DeleteFeaStruct( pod_id, struct_ind_1 );

    int struct_ind_2_new = GetFeaStructIndex( struct_id_2 );

    // The second structure slides down to fill the gap the first one left.
    if ( struct_ind_2 != 1 || struct_ind_2_new != 0 )
    {
        Print( "ERROR: GetFeaStructIndex did not follow the delete" );
        __failure++;
    }

    // The index has to lead back to the same structure.
    if ( GetFeaStructID( pod_id, struct_ind_2_new ) != struct_id_2 )
    {
        Print( "ERROR: GetFeaStructIndex disagrees with GetFeaStructID" );
        __failure++;
    }

    // An ID that is not a structure has to report -1.
    if ( GetFeaStructIndex( "NOSUCHSTRUCT" ) != -1 )
    {
        Print( "ERROR: GetFeaStructIndex accepted a bad ID" );
        __failure++;
    }

    // That lookup failure was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind_1 = AddFeaStruct( pod_id )

    struct_ind_2 = AddFeaStruct( pod_id )

    struct_id_2 = GetFeaStructID( pod_id, struct_ind_2 )

    DeleteFeaStruct( pod_id, struct_ind_1 )

    struct_ind_2_new = GetFeaStructIndex( struct_id_2 )

    # The second structure slides down to fill the gap the first one left.
    assert struct_ind_2 == 1, "GetFeaStructIndex did not follow the delete"
    assert struct_ind_2_new == 0, "GetFeaStructIndex did not follow the delete"

    # The index has to lead back to the same structure.
    assert GetFeaStructID( pod_id, struct_ind_2_new ) == struct_id_2, "GetFeaStructIndex disagrees with GetFeaStructID"

    # An ID that is not a structure has to report -1.
    assert GetFeaStructIndex( "NOSUCHSTRUCT" ) == -1, "GetFeaStructIndex accepted a bad ID"

    # That lookup failure was raised deliberately, so take it back off the queue.
    err_mgr = ErrorMgrSingleton.getInstance()

    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] struct_id string FEA Structure ID
    \return int FEA Structure index
*/

extern int GetFeaStructIndex( const std::string & struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the Parent Geom ID for an FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id );
    if ( parent_id.length() == 0 )
    {
        Print( "ERROR: GetFeaStructParentGeomID returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Get Parent Geom ID and Index ====//
    parent_id = GetFeaStructParentGeomID( struct_id )
    assert len( parent_id ) > 0, "GetFeaStructParentGeomID returned nothing"

    \endcode
    \endPythonOnly
    \param [in] struct_id string FEA Structure ID
    \return string Parent Geom ID
*/

extern std::string GetFeaStructParentGeomID( const std::string & struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the name of an FEA Structure. The FEA Structure name functions as the the Parm Container name
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Get Structure Name ====//
    string parm_container_name = GetFeaStructName( pod_id, struct_ind );
    if ( parm_container_name.length() == 0 )
    {
        Print( "ERROR: GetFeaStructName returned nothing" );
        __failure++;
    }

    string display_name = string("Current Structure Parm Container Name: ") + parm_container_name + string("\n");

    Print( display_name );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Get Structure Name ====//
    parm_container_name = GetFeaStructName( pod_id, struct_ind )
    assert len( parm_container_name ) > 0, "GetFeaStructName returned nothing"

    display_name = "Current Structure Parm Container Name: " + parm_container_name + "\n"

    print( display_name )

    \endcode
    \endPythonOnly
    \sa FindContainer, SetFeaStructName
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \return string Name for the FEA Structure
*/

extern std::string GetFeaStructName( const std::string & geom_id, int fea_struct_ind );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the name of an FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Change the Structure Name ====//
    SetFeaStructName( pod_id, struct_ind, "Example_Struct" );
    if ( GetFeaStructName( pod_id, struct_ind ) != "Example_Struct" )
    {
        Print( "ERROR: SetFeaStructName did not take" );
        __failure++;
    }


    string parm_container_id = FindContainer( "Example_Struct", struct_ind );

    string display_id = string("New Structure Parm Container ID: ") + parm_container_id + string("\n");

    Print( display_id );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Change the Structure Name ====//
    SetFeaStructName( pod_id, struct_ind, "Example_Struct" )
    assert GetFeaStructName( pod_id, struct_ind ) == "Example_Struct", "SetFeaStructName did not take"


    parm_container_id = FindContainer( "Example_Struct", struct_ind )

    display_id = "New Structure Parm Container ID: " + parm_container_id + "\n"

    print( display_id )

    \endcode
    \endPythonOnly
    \sa GetFeaStructName
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] name string New name for the FEA Structure
*/

extern void SetFeaStructName( const std::string & geom_id, int fea_struct_ind, const std::string & name );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the IDs of all FEA Structures in the vehicle
    \forcpponly
    \code{.cpp}
    //==== Add Geometries ====//
    string pod_id = AddGeom( "POD" );
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructures ====//
    int pod_struct_ind = AddFeaStruct( pod_id );
    int wing_struct_ind = AddFeaStruct( wing_id );

    array < string > struct_id_vec = GetFeaStructIDVec();
    if ( struct_id_vec.length() == 0 )
    {
        Print( "ERROR: GetFeaStructIDVec returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Geometries ====//
    pod_id = AddGeom( "POD" )
    wing_id = AddGeom( "WING" )

    #==== Add FeaStructures ====//
    pod_struct_ind = AddFeaStruct( pod_id )
    wing_struct_ind = AddFeaStruct( wing_id )

    struct_id_vec = GetFeaStructIDVec()
    assert len( struct_id_vec ) > 0, "GetFeaStructIDVec returned nothing"

    \endcode
    \endPythonOnly
    \sa NumFeaStructures
    \return vector\<string\> Array of FEA Structure IDs
*/

extern std::vector< std::string > GetFeaStructIDVec();

/*!
    \ingroup FEAMesh
*/
/*!
    Set the name of an FEA Part
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    SetFeaPartName( bulkhead_id, "Bulkhead" );
    if ( GetFeaPartName( bulkhead_id ) != "Bulkhead" )
    {
        Print( "ERROR: SetFeaPartName did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add Bulkead ====//
    bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )

    SetFeaPartName( bulkhead_id, "Bulkhead" )
    assert GetFeaPartName( bulkhead_id ) == "Bulkhead", "SetFeaPartName did not take"


    \endcode
    \endPythonOnly
    \sa GetFeaPartName
    \param [in] part_id string FEA Part ID
    \param [in] name string New name for the FEA Part
*/

extern void SetFeaPartName( const std::string & part_id, const std::string & name );

/*!
    \ingroup FEAMesh
*/
/*!
    Add an FEA Part to a Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    SetParmVal( FindParm( bulkhead_id, "IncludedElements", "FeaPart" ), FEA_SHELL_AND_BEAM );

    SetParmVal( FindParm( bulkhead_id, "RelCenterLocation", "FeaPart" ), 0.15 );

    if ( bulkhead_id.length() == 0 || bulkhead_id == "NONE" )
    {
        Print( "ERROR: AddFeaPart returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add Bulkead ====//
    bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )

    SetParmVal( FindParm( bulkhead_id, "IncludedElements", "FeaPart" ), FEA_SHELL_AND_BEAM )

    SetParmVal( FindParm( bulkhead_id, "RelCenterLocation", "FeaPart" ), 0.15 )

    \endcode
    \endPythonOnly
    \sa FEA_PART_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] type int FEA Part type enum (i.e. FEA_RIB)
    \return string FEA Part ID
*/

extern std::string AddFeaPart( const std::string & geom_id, int fea_struct_ind, int type );

/*!
    \ingroup FEAMesh
*/
/*!
    Delete an FEA Part from a Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    //==== Add Fixed Point ====//
    string fixed_id = AddFeaPart( pod_id, struct_ind, FEA_FIX_POINT );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    // The skin, the bulkhead and the fixed point.
    int num_before = NumFeaParts( struct_id );

    if ( num_before != 3 )
    {
        Print( "ERROR: the parts were not all added" );
        __failure++;
    }

    //==== Delete Bulkead ====//
    DeleteFeaPart( pod_id, struct_ind, bulkhead_id );

    // Only the named part goes; the fixed point stays.
    if ( NumFeaParts( struct_id ) != num_before - 1 )
    {
        Print( "ERROR: DeleteFeaPart did not remove the part" );
        __failure++;
    }

    if ( GetFeaPartName( fixed_id ).length() == 0 )
    {
        Print( "ERROR: DeleteFeaPart removed the wrong part" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add Bulkead ====//
    bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )

    #==== Add Fixed Point ====//
    fixed_id = AddFeaPart( pod_id, struct_ind, FEA_FIX_POINT )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    # The skin, the bulkhead and the fixed point.
    num_before = NumFeaParts( struct_id )

    assert num_before == 3, "the parts were not all added"

    #==== Delete Bulkead ====//
    DeleteFeaPart( pod_id, struct_ind, bulkhead_id )

    # Only the named part goes; the fixed point stays.
    assert NumFeaParts( struct_id ) == num_before - 1, "DeleteFeaPart did not remove the part"
    assert len( GetFeaPartName( fixed_id ) ) > 0, "DeleteFeaPart removed the wrong part"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] part_id string FEA Part ID
*/

extern void DeleteFeaPart( const std::string & geom_id, int fea_struct_ind, const std::string & part_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the Parm ID of an FEA Part, identified from a FEA Structure Parm ID and FEA Part index.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    Update();

    if ( bulkhead_id != GetFeaPartID( struct_id, 1 ) ) // These should be equivalent (index 0 is skin)
    {
        Print( "Error: GetFeaPartID" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Add Bulkead ====//
    bulkhead_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )

    Update()

    if  bulkhead_id != GetFeaPartID( struct_id, 1 ) : # These should be equivalent (index 0 is skin)

        print( "Error: GetFeaPartID" )
        assert False, "Error: GetFeaPartID"

    \endcode
    \endPythonOnly
    \param [in] fea_struct_id string FEA Structure ID
    \param [in] fea_part_index int FEA Part index
    \return string FEA Part ID
*/

extern std::string GetFeaPartID( const std::string & fea_struct_id, int fea_part_index );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the name of an FEA Part
    \forcpponly
    \code{.cpp}
    //==== Add Fuselage Geometry ====//
    string fuse_id = AddGeom( "FUSELAGE" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( fuse_id );

    //==== Add Bulkead ====//
    string bulkhead_id = AddFeaPart( fuse_id, struct_ind, FEA_SLICE );

    string name = "example_name";
    SetFeaPartName( bulkhead_id, name );

    if ( name != GetFeaPartName( bulkhead_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaPartName" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Fuselage Geometry ====//
    fuse_id = AddGeom( "FUSELAGE" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( fuse_id )

    #==== Add Bulkead ====//
    bulkhead_id = AddFeaPart( fuse_id, struct_ind, FEA_SLICE )

    name = "example_name"
    SetFeaPartName( bulkhead_id, name )

    if  name != GetFeaPartName( bulkhead_id ) : # These should be equivalent

        print( "Error: GetFeaPartName" )
        assert False, "Error: GetFeaPartName"

    \endcode
    \endPythonOnly
    \sa SetFeaPartName
    \param [in] part_id string FEA Part ID
    \return string FEA Part name
*/

extern std::string GetFeaPartName( const std::string & part_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the type of an FEA Part
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Slice ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    if ( FEA_SLICE != GetFeaPartType( slice_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaPartType" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add Slice ====//
    slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )

    if  FEA_SLICE != GetFeaPartType( slice_id ) : # These should be equivalent

        print( "Error: GetFeaPartType" )
        assert False, "Error: GetFeaPartType"

    \endcode
    \endPythonOnly
    \sa FEA_PART_TYPE
    \param [in] part_id string FEA Part ID
    \return int FEA Part type enum
*/

extern int GetFeaPartType( const std::string & part_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the IDs of all FEA Parts in the given FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Geometries ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add FEA Parts ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );
    string dome_id = AddFeaPart( pod_id, struct_ind, FEA_DOME );

    array < string > part_id_vec = GetFeaPartIDVec( struct_id ); // Should include slice_id & dome_id
    if ( part_id_vec.length() == 0 )
    {
        Print( "ERROR: GetFeaPartIDVec returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Geometries ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Add FEA Parts ====//
    slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )
    dome_id = AddFeaPart( pod_id, struct_ind, FEA_DOME )

    part_id_vec = GetFeaPartIDVec( struct_id ) # Should include slice_id & dome_id
    assert len( part_id_vec ) > 0, "GetFeaPartIDVec returned nothing"

    \endcode
    \endPythonOnly
    \sa NumFeaParts
    \param [in] fea_struct_id string FEA Structure ID
    \return vector\<string\> Array of FEA Part IDs
*/

extern std::vector< std::string > GetFeaPartIDVec( const std::string & fea_struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the IDs of all FEA SubSurfaces in the given FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Geometries ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add SubSurfaces ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );
    string rectangle_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE );

    array < string > part_id_vec = GetFeaSubSurfIDVec( struct_id ); // Should include line_array_id & rectangle_id
    if ( part_id_vec.length() == 0 )
    {
        Print( "ERROR: GetFeaSubSurfIDVec returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Geometries ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Add SubSurfaces ====//
    line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY )
    rectangle_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE )

    part_id_vec = GetFeaSubSurfIDVec( struct_id ) # Should include line_array_id & rectangle_id
    assert len( part_id_vec ) > 0, "GetFeaSubSurfIDVec returned nothing"

    \endcode
    \endPythonOnly
    \sa NumFeaSubSurfs
    \param [in] fea_struct_id string FEA Structure ID
    \return vector\<string\> Array of FEA Part IDs
*/

extern std::vector< std::string > GetFeaSubSurfIDVec( const std::string & fea_struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the ID of the perpendicular spar for an FEA Rib or Rib Array. Note, the FEA Rib or Rib Array should have "SPAR_NORMAL"
    set for the "PerpendicularEdgeType" Parm. If it is not, the ID will still be set, but the orientation of the Rib or Rib
    Array will not change.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Wing ====//
    int struct_ind = AddFeaStruct( wing_id );

    //==== Add Rib ====//
    string rib_id = AddFeaPart( wing_id, struct_ind, FEA_RIB );

    //==== Add Spars ====//
    string spar_id_1 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );
    string spar_id_2 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );

    SetParmVal( FindParm( spar_id_1, "RelCenterLocation", "FeaPart" ), 0.25 );
    SetParmVal( FindParm( spar_id_2, "RelCenterLocation", "FeaPart" ), 0.75 );

    //==== Set Perpendicular Edge type to SPAR ====//
    SetParmVal( FindParm( rib_id, "PerpendicularEdgeType", "FeaRib" ), SPAR_NORMAL );

    SetFeaPartPerpendicularSparID( rib_id, spar_id_2 );

    if ( spar_id_2 != GetFeaPartPerpendicularSparID( rib_id ) )
    {
        Print( "Error: SetFeaPartPerpendicularSparID" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add FeaStructure to Wing ====//
    struct_ind = AddFeaStruct( wing_id )

    #==== Add Rib ====//
    rib_id = AddFeaPart( wing_id, struct_ind, FEA_RIB )

    #==== Add Spars ====//
    spar_id_1 = AddFeaPart( wing_id, struct_ind, FEA_SPAR )
    spar_id_2 = AddFeaPart( wing_id, struct_ind, FEA_SPAR )

    SetParmVal( FindParm( spar_id_1, "RelCenterLocation", "FeaPart" ), 0.25 )
    SetParmVal( FindParm( spar_id_2, "RelCenterLocation", "FeaPart" ), 0.75 )

    #==== Set Perpendicular Edge type to SPAR ====//
    SetParmVal( FindParm( rib_id, "PerpendicularEdgeType", "FeaRib" ), SPAR_NORMAL )

    SetFeaPartPerpendicularSparID( rib_id, spar_id_2 )

    if  spar_id_2 != GetFeaPartPerpendicularSparID( rib_id ) :
        print( "Error: SetFeaPartPerpendicularSparID" )
        assert False, "Error: SetFeaPartPerpendicularSparID"

    \endcode
    \endPythonOnly
    \sa FEA_RIB_NORMAL, GetFeaPartPerpendicularSparID
    \param [in] part_id string FEA Part ID (Rib or Rib Array Type)
    \param [in] perpendicular_spar_id string FEA Spar ID
*/

extern void SetFeaPartPerpendicularSparID( const std::string& part_id, const std::string& perpendicular_spar_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the ID of the perpendicular spar for an FEA Rib or Rib Array. Note, the FEA Rib or Rib Array doesn't have to have "SPAR_NORMAL"
    set for the "PerpendicularEdgeType" Parm for this function to still return a value.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Wing ====//
    int struct_ind = AddFeaStruct( wing_id );

    //==== Add Rib ====//
    string rib_id = AddFeaPart( wing_id, struct_ind, FEA_RIB );

    //==== Add Spars ====//
    string spar_id_1 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );
    string spar_id_2 = AddFeaPart( wing_id, struct_ind, FEA_SPAR );

    SetParmVal( FindParm( spar_id_1, "RelCenterLocation", "FeaPart" ), 0.25 );
    SetParmVal( FindParm( spar_id_2, "RelCenterLocation", "FeaPart" ), 0.75 );

    //==== Set Perpendicular Edge type to SPAR ====//
    SetParmVal( FindParm( rib_id, "PerpendicularEdgeType", "FeaRib" ), SPAR_NORMAL );

    SetFeaPartPerpendicularSparID( rib_id, spar_id_2 );

    if ( spar_id_2 != GetFeaPartPerpendicularSparID( rib_id ) )
    {
        Print( "Error: GetFeaPartPerpendicularSparID" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add FeaStructure to Wing ====//
    struct_ind = AddFeaStruct( wing_id )

    #==== Add Rib ====//
    rib_id = AddFeaPart( wing_id, struct_ind, FEA_RIB )

    #==== Add Spars ====//
    spar_id_1 = AddFeaPart( wing_id, struct_ind, FEA_SPAR )
    spar_id_2 = AddFeaPart( wing_id, struct_ind, FEA_SPAR )

    SetParmVal( FindParm( spar_id_1, "RelCenterLocation", "FeaPart" ), 0.25 )
    SetParmVal( FindParm( spar_id_2, "RelCenterLocation", "FeaPart" ), 0.75 )

    #==== Set Perpendicular Edge type to SPAR ====//
    SetParmVal( FindParm( rib_id, "PerpendicularEdgeType", "FeaRib" ), SPAR_NORMAL )

    SetFeaPartPerpendicularSparID( rib_id, spar_id_2 )

    if  spar_id_2 != GetFeaPartPerpendicularSparID( rib_id ) :
        print( "Error: GetFeaPartPerpendicularSparID" )
        assert False, "Error: GetFeaPartPerpendicularSparID"

    \endcode
    \endPythonOnly
    \sa FEA_RIB_NORMAL, SetFeaPartPerpendicularSparID
    \param [in] part_id string FEA Part ID (Rib or Rib Array Type)
    \return string Perpendicular FEA Spar ID
*/

extern std::string GetFeaPartPerpendicularSparID( const std::string& part_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the name of an FEA SubSurface
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    SetFeaSubSurfName( line_array_id, "Stiffener_array" );
    if ( GetFeaSubSurfName( line_array_id ) != "Stiffener_array" )
    {
        Print( "ERROR: SetFeaSubSurfName did not take" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add LineArray ====//
    line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY )

    SetFeaSubSurfName( line_array_id, "Stiffener_array" )
    assert GetFeaSubSurfName( line_array_id ) == "Stiffener_array", "SetFeaSubSurfName did not take"


    \endcode
    \endPythonOnly
    \param [in] subsurf_id string FEA SubSurface ID
    \param [in] name string New name for the FEA SubSurface
*/

extern void SetFeaSubSurfName( const std::string & subsurf_id, const std::string & name );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the name of an FEA SubSurface
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    string name = "example_name";
    SetFeaSubSurfName( line_array_id, name );

    if ( name != GetFeaSubSurfName( line_array_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaSubSurfName" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add LineArray ====//
    line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY )

    name = "example_name"
    SetFeaSubSurfName( line_array_id, name )

    if  name != GetFeaSubSurfName( line_array_id ) : # These should be equivalent
        print( "Error: GetFeaSubSurfName" )
        assert False, "Error: GetFeaSubSurfName"

    \endcode
    \endPythonOnly
    \param [in] subsurf_id string FEA SubSurface ID
    \return string FEA SubSurf name
*/

extern std::string GetFeaSubSurfName( const std::string & subsurf_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Add an FEA SubSurface to a Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    SetParmVal( FindParm( line_array_id, "ConstLineType", "SS_LineArray" ), 1 ); // Constant W

    SetParmVal( FindParm( line_array_id, "Spacing", "SS_LineArray" ), 0.25 );

    if ( line_array_id.length() == 0 || line_array_id == "NONE" )
    {
        Print( "ERROR: AddFeaSubSurf returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add LineArray ====//
    line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY )

    SetParmVal( FindParm( line_array_id, "ConstLineType", "SS_LineArray" ), 1 ) # Constant W

    SetParmVal( FindParm( line_array_id, "Spacing", "SS_LineArray" ), 0.25 )

    \endcode
    \endPythonOnly
    \sa SUBSURF_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] type int FEA SubSurface type enum (i.e. SS_ELLIPSE)
    \return string FEA SubSurface ID
*/

extern std::string AddFeaSubSurf( const std::string & geom_id, int fea_struct_ind, int type );

/*!
    \ingroup FEAMesh
*/
/*!
    Delete an FEA SubSurface from a Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    //==== Add Rectangle ====//
    string rect_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    int num_before = NumFeaSubSurfs( struct_id );

    //==== Delete LineArray ====//
    DeleteFeaSubSurf( pod_id, struct_ind, line_array_id );

    if ( NumFeaSubSurfs( struct_id ) != num_before - 1 )
    {
        Print( "ERROR: DeleteFeaSubSurf did not remove the sub-surface" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add LineArray ====//
    line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY )

    #==== Add Rectangle ====//
    rect_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    num_before = NumFeaSubSurfs( struct_id )

    #==== Delete LineArray ====//
    DeleteFeaSubSurf( pod_id, struct_ind, line_array_id )

    assert NumFeaSubSurfs( struct_id ) == num_before - 1, "DeleteFeaSubSurf did not remove the sub-surface"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] ss_id string FEA SubSurface ID
*/

extern void DeleteFeaSubSurf( const std::string & geom_id, int fea_struct_ind, const std::string & ss_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the index of an FEA SubSurface give the SubSurface ID
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Add Slice ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );

    //==== Add LineArray ====//
    string line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY );

    //==== Add Rectangle ====//
    string rect_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE );

    if ( 1 != GetFeaSubSurfIndex( rect_id ) ) // These should be equivalent
    {
        Print( "Error: GetFeaSubSurfIndex" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Add Slice ====//
    slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )

    #==== Add LineArray ====//
    line_array_id = AddFeaSubSurf( pod_id, struct_ind, SS_LINE_ARRAY )

    #==== Add Rectangle ====//
    rect_id = AddFeaSubSurf( pod_id, struct_ind, SS_RECTANGLE )

    if  1 != GetFeaSubSurfIndex( rect_id ) : # These should be equivalent

        print( "Error: GetFeaSubSurfIndex" )
        assert False, "Error: GetFeaSubSurfIndex"

    \endcode
    \endPythonOnly
    \param [in] ss_id string FEA SubSurface ID
    \return int FEA SubSurface Index
*/

extern int GetFeaSubSurfIndex( const string & ss_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the number of points in an FEA Poly Spar. A newly created Poly Spar contains two points
    (the inboard and outboard endpoints).
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    // A new Poly Spar starts with 2 points
    if ( GetFeaPolySparNumPt( pspar_id ) != 2 )
    {
        Print( "Error: GetFeaPolySparNumPt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    # A new Poly Spar starts with 2 points
    if GetFeaPolySparNumPt( pspar_id ) != 2:
        print( "Error: GetFeaPolySparNumPt" )
        assert False, "Error: GetFeaPolySparNumPt"

    \endcode
    \endPythonOnly
    \sa AddFeaPolySparPt, InsertFeaPolySparPt, DelFeaPolySparPt, DelAllFeaPolySparPt
    \param [in] pspar_id string FEA Poly Spar part ID
    \return int Number of points in the Poly Spar
*/

extern int GetFeaPolySparNumPt( const string & pspar_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Add a point to the end of an FEA Poly Spar and return its ID. The new point is appended
    after all existing points.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    string pt_id = AddFeaPolySparPt( pspar_id );

    if ( GetFeaPolySparNumPt( pspar_id ) != 3 )
    {
        Print( "Error: AddFeaPolySparPt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    pt_id = AddFeaPolySparPt( pspar_id )

    if GetFeaPolySparNumPt( pspar_id ) != 3:
        print( "Error: AddFeaPolySparPt" )
        assert False, "Error: AddFeaPolySparPt"

    \endcode
    \endPythonOnly
    \sa InsertFeaPolySparPt, DelFeaPolySparPt, GetFeaPolySparNumPt
    \param [in] pspar_id string FEA Poly Spar part ID
    \return string ID of the newly added Poly Spar point
*/

extern string AddFeaPolySparPt( const string & pspar_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Insert a point into an FEA Poly Spar before the given index and return its ID.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    // Insert a new intermediate point between the two default endpoints
    string pt_id = InsertFeaPolySparPt( pspar_id, 1 );

    if ( GetFeaPolySparNumPt( pspar_id ) != 3 )
    {
        Print( "Error: InsertFeaPolySparPt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    # Insert a new intermediate point between the two default endpoints
    pt_id = InsertFeaPolySparPt( pspar_id, 1 )

    if GetFeaPolySparNumPt( pspar_id ) != 3:
        print( "Error: InsertFeaPolySparPt" )
        assert False, "Error: InsertFeaPolySparPt"

    \endcode
    \endPythonOnly
    \sa AddFeaPolySparPt, DelFeaPolySparPt, GetFeaPolySparNumPt
    \param [in] pspar_id string FEA Poly Spar part ID
    \param [in] index int Index before which the new point is inserted
    \return string ID of the newly inserted Poly Spar point
*/

extern string InsertFeaPolySparPt( const string & pspar_id, int index );

/*!
    \ingroup FEAMesh
*/
/*!
    Delete the point at the given index from an FEA Poly Spar.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    AddFeaPolySparPt( pspar_id );
    AddFeaPolySparPt( pspar_id );

    // Delete the point at index 1 (leaving 3 points)
    DelFeaPolySparPt( pspar_id, 2 );

    if ( GetFeaPolySparNumPt( pspar_id ) != 3 )
    {
        Print( "Error: DelFeaPolySparPt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    AddFeaPolySparPt( pspar_id )
    AddFeaPolySparPt( pspar_id )

    # Delete the point at index 2 (leaving 3 points)
    DelFeaPolySparPt( pspar_id, 2 )

    if GetFeaPolySparNumPt( pspar_id ) != 3:
        print( "Error: DelFeaPolySparPt" )
        assert False, "Error: DelFeaPolySparPt"

    \endcode
    \endPythonOnly
    \sa DelAllFeaPolySparPt, AddFeaPolySparPt, GetFeaPolySparNumPt
    \param [in] pspar_id string FEA Poly Spar part ID
    \param [in] index int Index of the point to delete
*/

extern void DelFeaPolySparPt( const string & pspar_id, int index );

/*!
    \ingroup FEAMesh
*/
/*!
    Delete all points from an FEA Poly Spar.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    AddFeaPolySparPt( pspar_id );
    AddFeaPolySparPt( pspar_id );

    DelAllFeaPolySparPt( pspar_id );

    if ( GetFeaPolySparNumPt( pspar_id ) != 0 )
    {
        Print( "Error: DelAllFeaPolySparPt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    AddFeaPolySparPt( pspar_id )
    AddFeaPolySparPt( pspar_id )

    DelAllFeaPolySparPt( pspar_id )

    if GetFeaPolySparNumPt( pspar_id ) != 0:
        print( "Error: DelAllFeaPolySparPt" )
        assert False, "Error: DelAllFeaPolySparPt"

    \endcode
    \endPythonOnly
    \sa DelFeaPolySparPt, AddFeaPolySparPt, GetFeaPolySparNumPt
    \param [in] pspar_id string FEA Poly Spar part ID
*/

extern void DelAllFeaPolySparPt( const string & pspar_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Move a point within an FEA Poly Spar using a reorder type and return the new index of the moved point.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    AddFeaPolySparPt( pspar_id );

    // Move point at index 2 up one position
    int new_index = MoveFeaPolySparPt( pspar_id, 2, REORDER_MOVE_UP );

    if ( new_index != 1 )
    {
        Print( "Error: MoveFeaPolySparPt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    AddFeaPolySparPt( pspar_id )

    # Move point at index 2 up one position
    new_index = MoveFeaPolySparPt( pspar_id, 2, REORDER_MOVE_UP )

    if new_index != 1:
        print( "Error: MoveFeaPolySparPt" )
        assert False, "Error: MoveFeaPolySparPt"

    \endcode
    \endPythonOnly
    \sa REORDER_TYPE, AddFeaPolySparPt, GetFeaPolySparNumPt
    \param [in] pspar_id string FEA Poly Spar part ID
    \param [in] index int Index of the point to move
    \param [in] reorder_type int Reorder type enum (i.e. REORDER_MOVE_UP, REORDER_MOVE_DOWN, REORDER_MOVE_TOP, REORDER_MOVE_BOTTOM)
    \return int New index of the moved point
*/

extern int MoveFeaPolySparPt( const string & pspar_id, int index, int reorder_type );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the name of a point in an FEA Poly Spar.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    SetFeaPolySparPtName( pspar_id, 0, "InboardPt" );
    SetFeaPolySparPtName( pspar_id, 1, "OutboardPt" );

    if ( GetFeaPolySparPtName( pspar_id, 0 ) != "InboardPt" )
    {
        Print( "Error: SetFeaPolySparPtName" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    SetFeaPolySparPtName( pspar_id, 0, "InboardPt" )
    SetFeaPolySparPtName( pspar_id, 1, "OutboardPt" )

    if GetFeaPolySparPtName( pspar_id, 0 ) != "InboardPt":
        print( "Error: SetFeaPolySparPtName" )
        assert False, "Error: SetFeaPolySparPtName"

    \endcode
    \endPythonOnly
    \sa GetFeaPolySparPtName
    \param [in] pspar_id string FEA Poly Spar part ID
    \param [in] index int Index of the point
    \param [in] name string New name for the point
*/

extern void SetFeaPolySparPtName( const string & pspar_id, int index, const string & name );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the name of a point in an FEA Poly Spar.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    SetFeaPolySparPtName( pspar_id, 0, "InboardPt" );

    string name = GetFeaPolySparPtName( pspar_id, 0 );
    if ( name.length() == 0 )
    {
        Print( "ERROR: GetFeaPolySparPtName returned nothing" );
        __failure++;
    }

    Print( "Point 0 name: " + name );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    SetFeaPolySparPtName( pspar_id, 0, "InboardPt" )

    name = GetFeaPolySparPtName( pspar_id, 0 )
    assert len( name ) > 0, "GetFeaPolySparPtName returned nothing"

    print( "Point 0 name: " + name )

    \endcode
    \endPythonOnly
    \sa SetFeaPolySparPtName
    \param [in] pspar_id string FEA Poly Spar part ID
    \param [in] index int Index of the point
    \return string Name of the point
*/

extern string GetFeaPolySparPtName( const string & pspar_id, int index );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the ID of a point in an FEA Poly Spar. The returned ID is a ParmContainer ID that can
    be used with FindParm to access the point's parameters such as Eta, U01, U0N, and XoC.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    string pt_id = GetFeaPolySparPtID( pspar_id, 0 );
    if ( pt_id.length() == 0 )
    {
        Print( "ERROR: GetFeaPolySparPtID returned nothing" );
        __failure++;
    }

    // Set the spanwise location of the inboard point to eta = 0.1
    SetParmVal( FindParm( pt_id, "Eta", "FeaPolySparPoint" ), 0.1 );

    string pt_id_1 = GetFeaPolySparPtID( pspar_id, 1 );

    // Set the spanwise location of the outboard point to eta = 0.9
    SetParmVal( FindParm( pt_id_1, "Eta", "FeaPolySparPoint" ), 0.9 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    pt_id = GetFeaPolySparPtID( pspar_id, 0 )
    assert len( pt_id ) > 0, "GetFeaPolySparPtID returned nothing"

    # Set the spanwise location of the inboard point to eta = 0.1
    SetParmVal( FindParm( pt_id, "Eta", "FeaPolySparPoint" ), 0.1 )

    pt_id_1 = GetFeaPolySparPtID( pspar_id, 1 )

    # Set the spanwise location of the outboard point to eta = 0.9
    SetParmVal( FindParm( pt_id_1, "Eta", "FeaPolySparPoint" ), 0.9 )

    \endcode
    \endPythonOnly
    \sa GetAllFeaPolySparPtIDVec, FindParm, SetParmVal
    \param [in] pspar_id string FEA Poly Spar part ID
    \param [in] index int Index of the point
    \return string ID of the Poly Spar point ParmContainer
*/

extern string GetFeaPolySparPtID( const string & pspar_id, int index );

/*!
    \ingroup FEAMesh
*/
/*!
    Get a vector of IDs for all points in an FEA Poly Spar. Each ID is a ParmContainer ID that
    can be used with FindParm to access the point's parameters such as Eta, U01, U0N, and XoC.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    int struct_ind = AddFeaStruct( wing_id );

    string pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR );

    AddFeaPolySparPt( pspar_id );

    array < string > pt_ids = GetAllFeaPolySparPtIDVec( pspar_id );

    if ( pt_ids.size() != 3 )
    {
        Print( "Error: GetAllFeaPolySparPtIDVec" );
        __failure++;
    }

    // Set each point's spanwise eta location
    SetParmVal( FindParm( pt_ids[0], "Eta", "FeaPolySparPoint" ), 0.1 );
    SetParmVal( FindParm( pt_ids[1], "Eta", "FeaPolySparPoint" ), 0.5 );
    SetParmVal( FindParm( pt_ids[2], "Eta", "FeaPolySparPoint" ), 0.9 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    struct_ind = AddFeaStruct( wing_id )

    pspar_id = AddFeaPart( wing_id, struct_ind, FEA_POLY_SPAR )

    AddFeaPolySparPt( pspar_id )

    pt_ids = GetAllFeaPolySparPtIDVec( pspar_id )

    if len( pt_ids ) != 3:
        print( "Error: GetAllFeaPolySparPtIDVec" )
        assert False, "Error: GetAllFeaPolySparPtIDVec"

    # Set each point's spanwise eta location
    SetParmVal( FindParm( pt_ids[0], "Eta", "FeaPolySparPoint" ), 0.1 )
    SetParmVal( FindParm( pt_ids[1], "Eta", "FeaPolySparPoint" ), 0.5 )
    SetParmVal( FindParm( pt_ids[2], "Eta", "FeaPolySparPoint" ), 0.9 )

    \endcode
    \endPythonOnly
    \sa GetFeaPolySparPtID, FindParm, SetParmVal
    \param [in] pspar_id string FEA Poly Spar part ID
    \return vector \<string\> Vector of Poly Spar point ParmContainer IDs
*/

extern vector < string > GetAllFeaPolySparPtIDVec( const string & pspar_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the total number of FEA Structures in the vehicle
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Pod ====//
    int struct_1 = AddFeaStruct( wing_id );
    int struct_2 = AddFeaStruct( wing_id );

    if ( NumFeaStructures() != 2 )
    {
        Print( "Error: NumFeaStructures" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add FeaStructure to Pod ====//
    struct_1 = AddFeaStruct( wing_id )
    struct_2 = AddFeaStruct( wing_id )

    if  NumFeaStructures() != 2 :
        print( "Error: NumFeaStructures" )
        assert False, "Error: NumFeaStructures"

    \endcode
    \endPythonOnly
    \sa GetFeaStructIDVec
    \return int Total Number of FEA Structures
*/

extern int NumFeaStructures();

/*!
    \ingroup FEAMesh
*/
/*!
    Get the number of FEA Parts for a particular FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add FEA Parts ====//
    string slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE );
    string dome_id = AddFeaPart( pod_id, struct_ind, FEA_DOME );

    if ( NumFeaParts( struct_id ) != 3 ) // Includes FeaSkin
    {
        Print( "Error: NumFeaParts" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Add FEA Parts ====//
    slice_id = AddFeaPart( pod_id, struct_ind, FEA_SLICE )
    dome_id = AddFeaPart( pod_id, struct_ind, FEA_DOME )

    if  NumFeaParts( struct_id ) != 3 : # Includes FeaSkin

        print( "Error: NumFeaParts" )
        assert False, "Error: NumFeaParts"

    \endcode
    \endPythonOnly
    \sa GetFeaPartIDVec
    \param [in] fea_struct_id string FEA Structure ID
    \return int Number of FEA Parts
*/

extern int NumFeaParts( const std::string & fea_struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the number of FEA Subsurfaces for a particular FEA Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( wing_id );

    string struct_id = GetFeaStructID( wing_id, struct_ind );

    //==== Add SubSurfaces ====//
    string line_array_id = AddFeaSubSurf( wing_id, struct_ind, SS_LINE_ARRAY );
    string rectangle_id = AddFeaSubSurf( wing_id, struct_ind, SS_RECTANGLE );

    if ( NumFeaSubSurfs( struct_id ) != 2 )
    {
        Print( "Error: NumFeaSubSurfs" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( wing_id )

    struct_id = GetFeaStructID( wing_id, struct_ind )

    #==== Add SubSurfaces ====//
    line_array_id = AddFeaSubSurf( wing_id, struct_ind, SS_LINE_ARRAY )
    rectangle_id = AddFeaSubSurf( wing_id, struct_ind, SS_RECTANGLE )

    if  NumFeaSubSurfs( struct_id ) != 2 :
        print( "Error: NumFeaSubSurfs" )
        assert False, "Error: NumFeaSubSurfs"

    \endcode
    \endPythonOnly
    \sa GetFeaSubSurfIDVec
    \param [in] fea_struct_id string FEA Structure ID
    \return int Number of FEA SubSurfaces
*/

extern int NumFeaSubSurfs( const std::string & fea_struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Add an FEA BC to a Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add BC ====//
    string bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE );

    if ( bc_id.length() == 0 || bc_id == "NONE" )
    {
        Print( "ERROR: AddFeaBC returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind );

    #==== Add BC ====//
    bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE )

    \endcode
    \endPythonOnly
    \sa FEA_BC_TYPE
    \param [in] fea_struct_id string FEA Structure ID
    \param [in] type int FEA BC type enum ( i.e. FEA_BC_STRUCTURE )
    \return string FEA BC ID
*/

extern std::string AddFeaBC( const string & fea_struct_id, int type = -1 );

/*!
    \ingroup FEAMesh
*/
/*!
    Delete an FEA BC from a Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add BC ====//
    string bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE );

    if ( NumFeaBCs( struct_id ) != 1 )
    {
        Print( "ERROR: AddFeaBC did not add a boundary condition" );
        __failure++;
    }

    DelFeaBC( struct_id, bc_id );

    if ( NumFeaBCs( struct_id ) != 0 )
    {
        Print( "ERROR: DelFeaBC did not remove the boundary condition" );
        __failure++;
    }

    if ( GetFeaBCIDVec( struct_id ).size() != 0 )
    {
        Print( "ERROR: DelFeaBC left the boundary condition in the ID list" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind );

    #==== Add BC ====//
    bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE )

    assert NumFeaBCs( struct_id ) == 1, "AddFeaBC did not add a boundary condition"

    DelFeaBC( struct_id, bc_id )

    assert NumFeaBCs( struct_id ) == 0, "DelFeaBC did not remove the boundary condition"
    assert len( GetFeaBCIDVec( struct_id ) ) == 0, "DelFeaBC left the boundary condition in the ID list"

    \endcode
    \endPythonOnly
    \sa FEA_BC_TYPE
    \param [in] fea_struct_id string FEA Structure ID
    \param [in] bc_id int FEA BC ID
*/

extern void DelFeaBC( const string & fea_struct_id, const std::string &bc_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Return a vector of FEA BC ID's for a structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add BC ====//
    string bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE );

    array < string > bc_id_vec = GetFeaBCIDVec( struct_id );
    if ( bc_id_vec.length() == 0 )
    {
        Print( "ERROR: GetFeaBCIDVec returned nothing" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind );

    #==== Add BC ====//
    bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE )

    bc_id_vec = GetFeaBCIDVec( struct_id )
    assert len( bc_id_vec ) > 0, "GetFeaBCIDVec returned nothing"

    \endcode
    \endPythonOnly
    \param [in] fea_struct_id string FEA Structure ID
    \return vector\<string\> Array of FEA BC IDs
*/

extern std::vector< std::string > GetFeaBCIDVec( const string & fea_struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Return number of FEA BC's in a structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Add BC ====//
    string bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE );

    int nbc = NumFeaBCs( struct_id );

    // The count has to match the list of IDs, and follow another add.
    if ( nbc != 1 || nbc != int( GetFeaBCIDVec( struct_id ).size() ) )
    {
        Print( "ERROR: NumFeaBCs disagrees with GetFeaBCIDVec" );
        __failure++;
    }

    AddFeaBC( struct_id, FEA_BC_STRUCTURE );

    if ( NumFeaBCs( struct_id ) != nbc + 1 )
    {
        Print( "ERROR: NumFeaBCs did not follow AddFeaBC" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind );

    #==== Add BC ====//
    bc_id = AddFeaBC( struct_id, FEA_BC_STRUCTURE )

    nbc = NumFeaBCs( struct_id )

    # The count has to match the list of IDs, and follow another add.
    assert nbc == 1, "NumFeaBCs did not count the boundary condition"
    assert nbc == len( GetFeaBCIDVec( struct_id ) ), "NumFeaBCs disagrees with GetFeaBCIDVec"

    AddFeaBC( struct_id, FEA_BC_STRUCTURE )

    assert NumFeaBCs( struct_id ) == nbc + 1, "NumFeaBCs did not follow AddFeaBC"

    \endcode
    \endPythonOnly
    \param [in] fea_struct_id string FEA Structure ID
    \return int Number of FEA BCs
*/

extern int NumFeaBCs( const string & fea_struct_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Add an FEA Material the FEA Mesh material library. Materials are available across all Geoms and Structures.
    \forcpponly
    \code{.cpp}
    //==== Create FeaMaterial ====//
    string mat_id = AddFeaMaterial();

    SetParmVal( FindParm( mat_id, "MassDensity", "FeaMaterial" ), 0.016 );

    if ( mat_id.length() == 0 || mat_id == "NONE" )
    {
        Print( "ERROR: AddFeaMaterial returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Create FeaMaterial ====//
    mat_id = AddFeaMaterial()

    SetParmVal( FindParm( mat_id, "MassDensity", "FeaMaterial" ), 0.016 )

    \endcode
    \endPythonOnly
    \return string FEA Material ID
*/

extern std::string AddFeaMaterial();

/*!
    \ingroup FEAMesh
*/
/*!
    Add aa FEA Property the FEA Mesh property library. Properties are available across all Geoms and Structures. Currently only beam and
    shell properties are available. Note FEA_SHELL_AND_BEAM is not a valid property type.
    \forcpponly
    \code{.cpp}
    //==== Create FeaProperty ====//
    string prop_id = AddFeaProperty();

    SetParmVal( FindParm( prop_id, "Thickness", "FeaProperty" ), 0.01 );

    if ( prop_id.length() == 0 || prop_id == "NONE" )
    {
        Print( "ERROR: AddFeaProperty returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Create FeaProperty ====//
    prop_id = AddFeaProperty()

    SetParmVal( FindParm( prop_id, "Thickness", "FeaProperty" ), 0.01 )

    \endcode
    \endPythonOnly
    \sa FEA_PART_ELEMENT_TYPE
    \param [in] property_type int FEA Property type enum (i.e. FEA_SHELL).
    \return string FEA Property ID
*/

extern std::string AddFeaProperty( int property_type = 0 );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the value of a particular FEA Mesh option for the specified Structure. Note, FEA Mesh makes use of enums initially created for CFD Mesh
    but not all CFD Mesh options are available for FEA Mesh.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Adjust FeaMeshSettings ====//
    SetFeaMeshVal( pod_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 );

    SetFeaMeshVal( pod_id, struct_ind, CFD_MIN_EDGE_LEN, 0.2 );

    // The options are backed by Parms on the Structure, in its grid density
    // group, so the values that were set can be read back.
    string struct_id = GetFeaStructID( pod_id, struct_ind );

    if ( !closeTo( GetParmVal( FindParm( struct_id, "BaseLen", "FEAGridDensity" ) ), 0.75, 1e-12 ) )
    {
        Print( "ERROR: SetFeaMeshVal did not set CFD_MAX_EDGE_LEN" );
        __failure++;
    }

    if ( !closeTo( GetParmVal( FindParm( struct_id, "MinLen", "FEAGridDensity" ) ), 0.2, 1e-12 ) )
    {
        Print( "ERROR: SetFeaMeshVal did not set CFD_MIN_EDGE_LEN" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Adjust FeaMeshSettings ====//
    SetFeaMeshVal( pod_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 )

    SetFeaMeshVal( pod_id, struct_ind, CFD_MIN_EDGE_LEN, 0.2 )

    # The options are backed by Parms on the Structure, in its grid density
    # group, so the values that were set can be read back.
    struct_id = GetFeaStructID( pod_id, struct_ind )

    assert abs( GetParmVal( FindParm( struct_id, "BaseLen", "FEAGridDensity" ) ) - 0.75 ) < 1e-12, "SetFeaMeshVal did not set CFD_MAX_EDGE_LEN"
    assert abs( GetParmVal( FindParm( struct_id, "MinLen", "FEAGridDensity" ) ) - 0.2 ) < 1e-12, "SetFeaMeshVal did not set CFD_MIN_EDGE_LEN"

    \endcode
    \endPythonOnly
    \sa CFD_CONTROL_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] type int FEA Mesh option type enum (i.e. CFD_MAX_EDGE_LEN)
    \param [in] val double Value the option is set to
*/

extern void SetFeaMeshVal( const std::string & geom_id, int fea_struct_ind, int type, double val );

/*!
    \ingroup FEAMesh
*/
/*!
    Set the name of a particular FEA Mesh output file for a specified Structure
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //=== Set Export File Name ===//
    string export_name = "FEAMeshTest_calculix.dat";

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id ); // same as pod_id

    if ( parent_id != pod_id )
    {
        Print( "ERROR: GetFeaStructParentGeomID did not report the parent Geom" );
        __failure++;
    }

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, export_name );

    // Keep the mesh coarse so the example runs quickly, then mesh to prove the
    // name that was set is the name that gets written.
    SetFeaMeshVal( parent_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 );

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );

    file __f;

    if ( __f.open( export_name, "r" ) < 0 )
    {
        Print( "ERROR: SetFeaMeshFileName did not name the output file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: the FEA mesh output file is empty" );
            __failure++;
        }
        __f.close();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #=== Set Export File Name ===//
    export_name = "FEAMeshTest_calculix.dat"

    #==== Get Parent Geom ID and Index ====//
    parent_id = GetFeaStructParentGeomID( struct_id ) # same as pod_id

    assert parent_id == pod_id, "GetFeaStructParentGeomID did not report the parent Geom"

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, export_name )

    # Keep the mesh coarse so the example runs quickly, then mesh to prove the
    # name that was set is the name that gets written.
    SetFeaMeshVal( parent_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 )

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME )

    import os
    assert os.path.getsize( export_name ) > 0, "SetFeaMeshFileName did not name the output file"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] file_type int FEA output file type enum (i.e. FEA_EXPORT_TYPE)
    \param [in] file_name string Name for the output file
*/

extern void SetFeaMeshFileName( const std::string & geom_id, int fea_struct_ind, int file_type, const string & file_name );

/*!
    \ingroup FEAMesh
*/
/*!
    Compute an FEA Mesh for a Structure. Only a single output file can be generated with this function.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Generate FEA Mesh and Export ====//
    Print( string( "--> Generating FeaMesh " ) );

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id ); // same as pod_id

    //==== Keep the mesh coarse so the example runs quickly ====//
    SetFeaMeshVal( parent_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 );

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, "FEAMeshTest_calculix.dat" );

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );
    // Could also call ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME );

    // FEA Mesh reports nothing through the Results Manager, so the output file
    // is the evidence that it ran.
    file __f;

    if ( __f.open( "FEAMeshTest_calculix.dat", "r" ) < 0 )
    {
        Print( "ERROR: ComputeFeaMesh wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: ComputeFeaMesh wrote an empty file" );
            __failure++;
        }
        __f.close();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Generate FEA Mesh and Export ====//
    print( "--> Generating FeaMesh " )

    #==== Get Parent Geom ID and Index ====//
    parent_id = GetFeaStructParentGeomID( struct_id ) # same as pod_id

    #==== Keep the mesh coarse so the example runs quickly ====//
    SetFeaMeshVal( parent_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 )

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, "FEAMeshTest_calculix.dat" )

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME )

    # FEA Mesh reports nothing through the Results Manager, so the output file is
    # the evidence that it ran.
    import os
    assert os.path.getsize( "FEAMeshTest_calculix.dat" ) > 0, "ComputeFeaMesh wrote no file"

    \endcode
    \endPythonOnly
    \sa SetFeaMeshFileName, FEA_EXPORT_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind int FEA Structure index
    \param [in] file_type int FEA output file type enum (i.e. FEA_EXPORT_TYPE)
*/

extern void ComputeFeaMesh( const std::string & geom_id, int fea_struct_ind, int file_type );

/*!
    \ingroup FEAMesh
*/
/*!
    Compute an FEA Mesh for a Structure. Only a single output file can be generated with this function.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    string struct_id = GetFeaStructID( pod_id, struct_ind );

    //==== Generate FEA Mesh and Export ====//
    Print( string( "--> Generating FeaMesh " ) );

    //==== Get Parent Geom ID and Index ====//
    string parent_id = GetFeaStructParentGeomID( struct_id ); // same as pod_id

    //==== Keep the mesh coarse so the example runs quickly ====//
    SetFeaMeshVal( parent_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 );

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, "FEAMeshTest_calculix.dat" );

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );
    // Could also call ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME );

    // FEA Mesh reports nothing through the Results Manager, so the output file
    // is the evidence that it ran.
    file __f;

    if ( __f.open( "FEAMeshTest_calculix.dat", "r" ) < 0 )
    {
        Print( "ERROR: ComputeFeaMesh wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: ComputeFeaMesh wrote an empty file" );
            __failure++;
        }
        __f.close();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    #==== Generate FEA Mesh and Export ====//
    print( "--> Generating FeaMesh " )

    #==== Get Parent Geom ID and Index ====//
    parent_id = GetFeaStructParentGeomID( struct_id ) # same as pod_id

    #==== Keep the mesh coarse so the example runs quickly ====//
    SetFeaMeshVal( parent_id, struct_ind, CFD_MAX_EDGE_LEN, 0.75 )

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, "FEAMeshTest_calculix.dat" )

    # This form names the Structure directly rather than its parent and index.
    ComputeFeaMesh( struct_id, FEA_CALCULIX_FILE_NAME )

    # FEA Mesh reports nothing through the Results Manager, so the output file is
    # the evidence that it ran.
    import os
    assert os.path.getsize( "FEAMeshTest_calculix.dat" ) > 0, "ComputeFeaMesh wrote no file"

    \endcode
    \endPythonOnly
    \sa SetFeaMeshFileName, FEA_EXPORT_TYPE
    \param [in] struct_id string FEA Structure index
    \param [in] file_type int FEA output file type enum (i.e. FEA_EXPORT_TYPE)
*/

extern void ComputeFeaMesh( const std::string & struct_id, int file_type );

/*!
    \ingroup XSec
*/
/*!
    Set XSec Alias by ID
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Set Alias and verify alias match
    string alias = "XSec_One_Alias";

    SetXSecAlias( xsec_1, alias );

    string get_alias = GetXSecAlias( xsec_1 );

    if ( alias != get_alias )
    {
        Print("SetXSecAlias/GetXSecAlias error!");
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Set Alias and verify alias match
    alias = "XSec_One_Alias"

    SetXSecAlias( xsec_1, alias )

    get_alias = GetXSecAlias( xsec_1 )

    if alias != get_alias:
        print("SetXSecAlias/GetXSecAlias error!")
        assert False, "SetXSecAlias/GetXSecAlias error!"

    \endcode
    \endPythonOnly
    \param [in] id string XSec ID
    \param [in] alias string Xsec alias
*/

extern void SetXSecAlias( const string & id, const string & alias );

/*!
    \ingroup XSec
*/
/*!
    Get XSec Alias by ID
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Set Alias and verify alias match
    string alias = "XSec_One_Alias";

    SetXSecAlias( xsec_1, alias );

    string get_alias = GetXSecAlias( xsec_1 );

    if ( alias != get_alias )
    {
        Print("SetXSecAlias/GetXSecAlias error!");
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Set Alias and verify alias match
    alias = "XSec_One_Alias"

    SetXSecAlias( xsec_1, alias )

    get_alias = GetXSecAlias( xsec_1 )

    if alias != get_alias:
        print("SetXSecAlias/GetXSecAlias error!")
        assert False, "SetXSecAlias/GetXSecAlias error!"

    \endcode
    \endPythonOnly
    \param [in] id string XSec ID
    \return string Xsec alias
*/

extern string GetXSecAlias( const string & id );

/*!
    \ingroup XSec
*/
/*!
    Set XSecCurve Alias by XSec ID
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Set Alias and verify alias match
    string alias = "XSecCurve_One_Alias";

    SetXSecCurveAlias( xsec_1, alias );

    string get_alias = GetXSecCurveAlias( xsec_1 );

    if ( alias != get_alias )
    {
        Print("SetXSecCurveAlias/GetXSecCurveAlias error!");
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Set Alias and verify alias match
    alias = "XSecCurve_One_Alias"

    SetXSecCurveAlias( xsec_1, alias )

    get_alias = GetXSecCurveAlias( xsec_1 )

    if alias != get_alias:
        print("SetXSecCurveAlias/GetXSecCurveAlias error!")
        assert False, "SetXSecCurveAlias/GetXSecCurveAlias error!"

    \endcode
    \endPythonOnly
    \param [in] id string XSec ID
    \param [in] alias string XsecCurve alias
*/

extern void SetXSecCurveAlias( const string & id, const string & alias );

/*!
    \ingroup XSec
*/
/*!
    Get XSecCurve Alias by XSec ID
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Set Alias and verify alias match
    string alias = "XSecCurve_One_Alias";

    SetXSecCurveAlias( xsec_1, alias );

    string get_alias = GetXSecCurveAlias( xsec_1 );

    if ( alias != get_alias )
    {
        Print("SetXSecCurveAlias/GetXSecCurveAlias error!");
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Set Alias and verify alias match
    alias = "XSecCurve_One_Alias"

    SetXSecCurveAlias( xsec_1, alias )

    get_alias = GetXSecCurveAlias( xsec_1 )

    if alias != get_alias:
        print("SetXSecCurveAlias/GetXSecCurveAlias error!")
        assert False, "SetXSecCurveAlias/GetXSecCurveAlias error!"

    \endcode
    \endPythonOnly
    \param [in] id string XSec ID
*/

extern string GetXSecCurveAlias( const string & id );

/*!
    \ingroup XSec
*/
/*!
    Cut a cross-section from the specified geometry and maintain it in memory
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    //==== Insert, Cut, Paste Example ====//
    string xsec_surf = GetXSecSurf( fid, 0 );

    int num_start = GetNumXSec( xsec_surf );

    InsertXSec( fid, 1, XS_ROUNDED_RECTANGLE );         // Insert A Cross-Section

    if ( GetNumXSec( xsec_surf ) != num_start + 1 )
    {
        Print( "ERROR: InsertXSec did not add a section" );
        __failure++;
    }

    if ( GetXSecShape( GetXSec( xsec_surf, 2 ) ) != XS_ROUNDED_RECTANGLE )
    {
        Print( "ERROR: InsertXSec did not insert after the given index" );
        __failure++;
    }

    CopyXSec( fid, 2 );                                 // Copy Just Created XSec To Clipboard

    PasteXSec( fid, 1 );                                // Paste Clipboard

    // Pasting replaces a section rather than adding one, and section 1 now
    // carries the shape that was copied.
    if ( GetNumXSec( xsec_surf ) != num_start + 1 )
    {
        Print( "ERROR: PasteXSec changed the number of sections" );
        __failure++;
    }

    if ( GetXSecShape( GetXSec( xsec_surf, 1 ) ) != XS_ROUNDED_RECTANGLE )
    {
        Print( "ERROR: PasteXSec did not paste the copied section" );
        __failure++;
    }

    CutXSec( fid, 2 );                                  // Cut Created XSec

    if ( GetNumXSec( xsec_surf ) != num_start )
    {
        Print( "ERROR: CutXSec did not remove a section" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    #==== Insert, Cut, Paste Example ====//
    xsec_surf = GetXSecSurf( fid, 0 )

    num_start = GetNumXSec( xsec_surf )

    InsertXSec( fid, 1, XS_ROUNDED_RECTANGLE )         # Insert A Cross-Section

    assert GetNumXSec( xsec_surf ) == num_start + 1, "InsertXSec did not add a section"
    assert GetXSecShape( GetXSec( xsec_surf, 2 ) ) == XS_ROUNDED_RECTANGLE, "InsertXSec did not insert after the given index"

    CopyXSec( fid, 2 )                                 # Copy Just Created XSec To Clipboard

    PasteXSec( fid, 1 )                                # Paste Clipboard

    # Pasting replaces a section rather than adding one, and section 1 now
    # carries the shape that was copied.
    assert GetNumXSec( xsec_surf ) == num_start + 1, "PasteXSec changed the number of sections"
    assert GetXSecShape( GetXSec( xsec_surf, 1 ) ) == XS_ROUNDED_RECTANGLE, "PasteXSec did not paste the copied section"

    CutXSec( fid, 2 )                                  # Cut Created XSec

    assert GetNumXSec( xsec_surf ) == num_start, "CutXSec did not remove a section"

    \endcode
    \endPythonOnly
    \sa PasteXSec
    \param [in] geom_id string Geom ID
    \param [in] index int XSec index
*/

extern void CutXSec( const std::string & geom_id, int index );

/*!
    \ingroup XSec
*/
/*!
    Copy a cross-section from the specified geometry and maintain it in memory
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    string xsec_surf = GetXSecSurf( sid, 0 );

    // Give XSec 1 a shape that XSec 3 does not have.
    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE );

    Update();

    int num_start = GetNumXSec( xsec_surf );

    // Copy XSec To Clipboard
    CopyXSec( sid, 1 );

    // Paste To XSec 3
    PasteXSec( sid, 3 );

    Update();

    // Pasting replaces the target section, so the count is unchanged and XSec 3
    // now carries the shape that was copied.
    if ( GetNumXSec( xsec_surf ) != num_start )
    {
        Print( "ERROR: PasteXSec changed the number of sections" );
        __failure++;
    }

    if ( GetXSecShape( GetXSec( xsec_surf, 3 ) ) != XS_ROUNDED_RECTANGLE )
    {
        Print( "ERROR: PasteXSec did not paste the copied section" );
        __failure++;
    }

    // The section that was copied has to be left alone.
    if ( GetXSecShape( GetXSec( xsec_surf, 1 ) ) != XS_ROUNDED_RECTANGLE )
    {
        Print( "ERROR: CopyXSec disturbed the section it copied" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    xsec_surf = GetXSecSurf( sid, 0 )

    # Give XSec 1 a shape that XSec 3 does not have.
    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE )

    Update()

    num_start = GetNumXSec( xsec_surf )

    # Copy XSec To Clipboard
    CopyXSec( sid, 1 )

    # Paste To XSec 3
    PasteXSec( sid, 3 )

    Update()

    # Pasting replaces the target section, so the count is unchanged and XSec 3
    # now carries the shape that was copied.
    assert GetNumXSec( xsec_surf ) == num_start, "PasteXSec changed the number of sections"
    assert GetXSecShape( GetXSec( xsec_surf, 3 ) ) == XS_ROUNDED_RECTANGLE, "PasteXSec did not paste the copied section"

    # The section that was copied has to be left alone.
    assert GetXSecShape( GetXSec( xsec_surf, 1 ) ) == XS_ROUNDED_RECTANGLE, "CopyXSec disturbed the section it copied"

    \endcode
    \endPythonOnly
    \sa PasteXSec
    \param [in] geom_id string Geom ID
    \param [in] index int XSec index
*/

extern void CopyXSec( const std::string & geom_id, int index );

/*!
    \ingroup XSec
*/
/*!
    Paste the cross-section currently held in memory to the specified geometry
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    string xsec_surf = GetXSecSurf( sid, 0 );

    // Give XSec 1 a shape that XSec 3 does not have.
    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE );

    Update();

    int num_start = GetNumXSec( xsec_surf );

    // Copy XSec To Clipboard
    CopyXSec( sid, 1 );

    // Paste To XSec 3
    PasteXSec( sid, 3 );

    Update();

    // Pasting replaces the target section, so the count is unchanged and XSec 3
    // now carries the shape that was copied.
    if ( GetNumXSec( xsec_surf ) != num_start )
    {
        Print( "ERROR: PasteXSec changed the number of sections" );
        __failure++;
    }

    if ( GetXSecShape( GetXSec( xsec_surf, 3 ) ) != XS_ROUNDED_RECTANGLE )
    {
        Print( "ERROR: PasteXSec did not paste the copied section" );
        __failure++;
    }

    // The section that was copied has to be left alone.
    if ( GetXSecShape( GetXSec( xsec_surf, 1 ) ) != XS_ROUNDED_RECTANGLE )
    {
        Print( "ERROR: CopyXSec disturbed the section it copied" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    xsec_surf = GetXSecSurf( sid, 0 )

    # Give XSec 1 a shape that XSec 3 does not have.
    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE )

    Update()

    num_start = GetNumXSec( xsec_surf )

    # Copy XSec To Clipboard
    CopyXSec( sid, 1 )

    # Paste To XSec 3
    PasteXSec( sid, 3 )

    Update()

    # Pasting replaces the target section, so the count is unchanged and XSec 3
    # now carries the shape that was copied.
    assert GetNumXSec( xsec_surf ) == num_start, "PasteXSec changed the number of sections"
    assert GetXSecShape( GetXSec( xsec_surf, 3 ) ) == XS_ROUNDED_RECTANGLE, "PasteXSec did not paste the copied section"

    # The section that was copied has to be left alone.
    assert GetXSecShape( GetXSec( xsec_surf, 1 ) ) == XS_ROUNDED_RECTANGLE, "CopyXSec disturbed the section it copied"

    \endcode
    \endPythonOnly
    \sa CutXSec, CopyXSec
    \param [in] geom_id string Geom ID
    \param [in] index int XSec index
*/

extern void PasteXSec( const std::string & geom_id, int index );

/*!
    \ingroup XSec
*/
/*!
    Insert a cross-section of particular type to the specified geometry after the given index
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );

    string xsec_surf = GetXSecSurf( wing_id, 0 );

    int num_start = GetNumXSec( xsec_surf );

    //===== Add XSec ====//
    InsertXSec( wing_id, 1, XS_SIX_SERIES );

    Update();

    // The new section lands after the given index.
    if ( GetNumXSec( xsec_surf ) != num_start + 1 )
    {
        Print( "ERROR: InsertXSec did not add a section" );
        __failure++;
    }

    if ( GetXSecShape( GetXSec( xsec_surf, 2 ) ) != XS_SIX_SERIES )
    {
        Print( "ERROR: InsertXSec did not insert the requested shape" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    xsec_surf = GetXSecSurf( wing_id, 0 )

    num_start = GetNumXSec( xsec_surf )

    #===== Add XSec ====//
    InsertXSec( wing_id, 1, XS_SIX_SERIES )

    Update()

    # The new section lands after the given index.
    assert GetNumXSec( xsec_surf ) == num_start + 1, "InsertXSec did not add a section"
    assert GetXSecShape( GetXSec( xsec_surf, 2 ) ) == XS_SIX_SERIES, "InsertXSec did not insert the requested shape"

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] geom_id string Geom ID
    \param [in] index int XSec index
    \param [in] type int XSec type enum (i.e. XS_GENERAL_FUSE)
*/

extern void InsertXSec( const std::string & geom_id, int index, int type );


//======================== Wing Section Functions ===================//

/*!
    \ingroup Geom
*/
/*!
    Split a given wing section.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING", "" );

    string xsec_surf = GetXSecSurf( wing_id, 0 );

    int num_start = GetNumXSec( xsec_surf );

    double span_start = GetParmVal( wing_id, "Span", "XSec_1" );

    //==== Set Wing Section Controls ====//
    SplitWingXSec( wing_id, 1 );

    Update();

    // Splitting a section turns one section into two, and the two halves span
    // what the one section spanned.
    if ( GetNumXSec( xsec_surf ) != num_start + 1 )
    {
        Print( "ERROR: SplitWingXSec did not split the section" );
        __failure++;
    }
    else
    {
        double span_1 = GetParmVal( wing_id, "Span", "XSec_1" );
        double span_2 = GetParmVal( wing_id, "Span", "XSec_2" );

        if ( !closeTo( span_1 + span_2, span_start, 1e-6 ) )
        {
            Print( "ERROR: SplitWingXSec changed the span of the wing" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING", "" )

    xsec_surf = GetXSecSurf( wing_id, 0 )

    num_start = GetNumXSec( xsec_surf )

    span_start = GetParmVal( wing_id, "Span", "XSec_1" )

    #==== Set Wing Section Controls ====//
    SplitWingXSec( wing_id, 1 )

    Update()

    # Splitting a section turns one section into two, and the two halves span
    # what the one section spanned.
    assert GetNumXSec( xsec_surf ) == num_start + 1, "SplitWingXSec did not split the section"

    span_1 = GetParmVal( wing_id, "Span", "XSec_1" )
    span_2 = GetParmVal( wing_id, "Span", "XSec_2" )

    assert abs( ( span_1 + span_2 ) - span_start ) < 1e-6, "SplitWingXSec changed the span of the wing"
    \endcode
    \endPythonOnly
    \sa WING_DRIVERS, XSEC_DRIVERS
    \param [in] wing_id string Geom ID
    \param [in] section_index int Wing section index
*/

extern void SplitWingXSec( const string & wing_id, int section_index );

/*!
    \ingroup Geom
*/
/*!
    Set the driver group for a wing section or a XSecCurve. Care has to be taken when setting these driver groups to ensure a valid combination.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    //==== Set Wing Section Controls ====//
    SetDriverGroup( wing_id, 1, AR_WSECT_DRIVER, ROOTC_WSECT_DRIVER, TIPC_WSECT_DRIVER );

    Update();

    //==== Set Parms ====//
    SetParmVal( wing_id, "Root_Chord", "XSec_1", 2 );
    SetParmVal( wing_id, "Tip_Chord", "XSec_1", 1 );

    Update();

    // The three chosen drivers are the values that hold; span is now solved for
    // from the aspect ratio and the two chords.
    double ar = GetParmVal( wing_id, "Aspect", "XSec_1" );
    double span = GetParmVal( wing_id, "Span", "XSec_1" );

    if ( !closeTo( GetParmVal( wing_id, "Root_Chord", "XSec_1" ), 2.0, 1e-6 ) ||
         !closeTo( GetParmVal( wing_id, "Tip_Chord", "XSec_1" ), 1.0, 1e-6 ) )
    {
        Print( "ERROR: the driving Parms did not hold their values" );
        __failure++;
    }

    // A section of this planform has area span * ( root + tip ) / 2, and aspect
    // ratio span * span / area.
    double area = span * ( 2.0 + 1.0 ) * 0.5;

    if ( !closeTo( ar, span * span / area, 1e-6 ) )
    {
        Print( "ERROR: SetDriverGroup left the section inconsistent" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry and Set Parms ====//
    wing_id = AddGeom( "WING", "" )

    #==== Set Wing Section Controls ====//
    SetDriverGroup( wing_id, 1, AR_WSECT_DRIVER, ROOTC_WSECT_DRIVER, TIPC_WSECT_DRIVER )

    Update()

    #==== Set Parms ====//
    SetParmVal( wing_id, "Root_Chord", "XSec_1", 2 )
    SetParmVal( wing_id, "Tip_Chord", "XSec_1", 1 )

    Update()

    # The three chosen drivers are the values that hold; span is now solved for
    # from the aspect ratio and the two chords.
    ar = GetParmVal( wing_id, "Aspect", "XSec_1" )
    span = GetParmVal( wing_id, "Span", "XSec_1" )

    assert abs( GetParmVal( wing_id, "Root_Chord", "XSec_1" ) - 2.0 ) < 1e-6, "the driving Parms did not hold their values"
    assert abs( GetParmVal( wing_id, "Tip_Chord", "XSec_1" ) - 1.0 ) < 1e-6, "the driving Parms did not hold their values"

    # A section of this planform has area span * ( root + tip ) / 2, and aspect
    # ratio span * span / area.
    area = span * ( 2.0 + 1.0 ) * 0.5

    assert abs( ar - span * span / area ) < 1e-6, "SetDriverGroup left the section inconsistent"

    \endcode
    \endPythonOnly
    \sa WING_DRIVERS, XSEC_DRIVERS
    \param [in] geom_id string Geom ID
    \param [in] section_index int Wing section index
    \param [in] driver_0 int First driver enum (i.e. SPAN_WSECT_DRIVER)
    \param [in] driver_1 int Second driver enum (i.e. ROOTC_WSECT_DRIVER)
    \param [in] driver_2 int Third driver enum (i.e. TIPC_WSECT_DRIVER)
    */

extern void SetDriverGroup( const std::string & geom_id, int section_index, int driver_0, int driver_1 = -1, int driver_2 = -1 );


//======================== XSecSurf ================================//
/*!
    \ingroup XSecSurf
*/
/*!
    Get the XSecSurf ID for a particular Geom and XSecSurf index
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );
    if ( xsec_surf.length() == 0 )
    {
        Print( "ERROR: GetXSecSurf returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )
    assert len( xsec_surf ) > 0, "GetXSecSurf returned nothing"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] index int XSecSurf index
    \return string XSecSurf ID
*/

extern std::string GetXSecSurf( const std::string & geom_id, int index );

/*!
    \ingroup XSecSurf
*/
/*!
    Get number of XSecs in an XSecSurf
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Flatten ends
    int num_xsecs = GetNumXSec( xsec_surf );

    // A Stack starts with five sections, and every index below the count has to
    // name a real XSec.
    if ( num_xsecs != 5 )
    {
        Print( "ERROR: GetNumXSec did not report the sections of a new Stack" );
        __failure++;
    }

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        if ( xsec.length() == 0 )
        {
            Print( "ERROR: GetNumXSec counted a section that GetXSec cannot find" );
            __failure++;
        }

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0 );       // Set Tangent Angles At Cross Section

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.0 );  // Set Tangent Strengths At Cross Section
    }

    // Inserting a section has to move the count.
    InsertXSec( sid, 1, XS_ROUNDED_RECTANGLE );

    if ( GetNumXSec( xsec_surf ) != num_xsecs + 1 )
    {
        Print( "ERROR: GetNumXSec did not follow InsertXSec" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Flatten ends
    num_xsecs = GetNumXSec( xsec_surf )

    # A Stack starts with five sections, and every index below the count has to
    # name a real XSec.
    assert num_xsecs == 5, "GetNumXSec did not report the sections of a new Stack"

    for i in range(num_xsecs):

        xsec = GetXSec( xsec_surf, i )

        assert len( xsec ) > 0, "GetNumXSec counted a section that GetXSec cannot find"

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0, -1.0e12, -1.0e12, -1.0e12 )       # Set Tangent Angles At Cross Section

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.0, -1.0e12, -1.0e12, -1.0e12 )  # Set Tangent Strengths At Cross Section

    # Inserting a section has to move the count.
    InsertXSec( sid, 1, XS_ROUNDED_RECTANGLE )

    assert GetNumXSec( xsec_surf ) == num_xsecs + 1, "GetNumXSec did not follow InsertXSec"

    \endcode
    \endPythonOnly
    \param [in] xsec_surf_id string XSecSurf ID
    \return int Number of XSecs
*/

extern int GetNumXSec( const std::string & xsec_surf_id );

/*!
    \ingroup XSecSurf
*/
/*!
    Get Xsec ID for a particular XSecSurf at given index
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );
    if ( xsec_1.length() == 0 )
    {
        Print( "ERROR: GetXSec returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )
    assert len( xsec_1 ) > 0, "GetXSec returned nothing"

    \endcode
    \endPythonOnly
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] xsec_index int Xsec index
    \return string Xsec ID
*/

extern std::string GetXSec( const std::string & xsec_surf_id, int xsec_index );

/*!
    \ingroup XSecSurf
*/
/*!
    Change the shape of a particular XSec, identified by an XSecSurf ID and XSec index
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Set XSec 1 & 2 to Edit Curve type
    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );
    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    string xsec_2 = GetXSec( xsec_surf, 2 );

    if ( GetXSecShape( xsec_2 ) != XS_EDIT_CURVE )
    {
        Print( "Error: ChangeXSecShape" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Set XSec 1 & 2 to Edit Curve type
    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )
    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE )

    xsec_2 = GetXSec( xsec_surf, 2 )

    if  GetXSecShape( xsec_2 ) != XS_EDIT_CURVE :
        print( "Error: ChangeXSecShape" )
        assert False, "Error: ChangeXSecShape"

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] xsec_index int Xsec index
    \param [in] type int Xsec type enum (i.e. XS_ELLIPSE)
*/

extern void ChangeXSecShape( const std::string & xsec_surf_id, int xsec_index, int type );

/*!
    \ingroup XSecSurf
*/
/*!
    Set the global surface transform matrix for given XSecSurf
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] mat Matrix4d Transformation matrix
*/

extern void SetXSecSurfGlobalXForm( const std::string & xsec_surf_id, const Matrix4d & mat );

/*!
    \ingroup XSecSurf
*/
/*!
    Get the global surface transform matrix for given XSecSurf
    \param [in] xsec_surf_id string XSecSurf ID
    \return Matrix4d Transformation matrix
*/

extern Matrix4d GetXSecSurfGlobalXForm( const std::string & xsec_surf_id );


//======================== XSec ================================//
/*!
    \ingroup XSec
*/
/*!
    Get the shape of an XSec
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    string xsec = GetXSec( xsec_surf, 1 );

    if ( GetXSecShape( xsec ) != XS_EDIT_CURVE ) { Print( "ERROR: GetXSecShape" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    xsec = GetXSec( xsec_surf, 1 )

    if  GetXSecShape( xsec ) != XS_EDIT_CURVE :
        print( "ERROR: GetXSecShape" )
        assert False, "ERROR: GetXSecShape"

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] xsec_id string XSec ID
    \return int XSec type enum (i.e. XS_ELLIPSE)
*/

extern int GetXSecShape( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the width of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what width and height it is set to.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ); // Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 );

    if ( abs( GetXSecWidth( xsec ) - 3.0 ) > 1e-6 )        { Print( "---> Error: API Get/Set Width " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ) # Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 )

    if  abs( GetXSecWidth( xsec ) - 3.0 ) > 1e-6 :
        print( "---> Error: API Get/Set Width " )
        assert False, "---> Error: API Get/Set Width"

    \endcode
    \endPythonOnly
    \sa SetXSecWidth
    \param [in] xsec_id string XSec ID
    \return double Xsec width
*/

extern double GetXSecWidth( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the height of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what width and height it is set to.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ); // Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 );

    if ( abs( GetXSecHeight( xsec ) - 6.0 ) > 1e-6 )        { Print( "---> Error: API Get/Set Width " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ) # Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 )

    if  abs( GetXSecHeight( xsec ) - 6.0 ) > 1e-6 :
        print( "---> Error: API Get/Set Width " )
        assert False, "---> Error: API Get/Set Width"

    \endcode
    \endPythonOnly
    \sa SetXSecHeight
    \param [in] xsec_id string XSec ID
    \return double Xsec height
*/

extern double GetXSecHeight( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Set the width and height of an XSec. Note, if the XSec is an EDIT_CURVE type and PreserveARFlag is true, the input width value will be
    ignored and instead set from on the input height and aspect ratio. Use SetXSecWidth and SetXSecHeight directly to avoid this.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    SetXSecWidthHeight( xsec_2, 1.5, 1.5 );

    Update();

    if ( !closeTo( GetXSecWidth( xsec_2 ), 1.5, 1e-6 ) ||
         !closeTo( GetXSecHeight( xsec_2 ), 1.5, 1e-6 ) )
    {
        Print( "ERROR: SetXSecWidthHeight did not take" );
        __failure++;
    }

    // The neighbouring sections have to be left alone.
    string xsec_1 = GetXSec( xsec_surf, 1 );

    if ( closeTo( GetXSecWidth( xsec_1 ), 1.5, 1e-6 ) &&
         closeTo( GetXSecHeight( xsec_1 ), 1.5, 1e-6 ) )
    {
        Print( "ERROR: SetXSecWidthHeight reached a section it was not given" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    SetXSecWidthHeight( xsec_2, 1.5, 1.5 )

    Update()

    assert abs( GetXSecWidth( xsec_2 ) - 1.5 ) < 1e-6, "SetXSecWidthHeight did not take"
    assert abs( GetXSecHeight( xsec_2 ) - 1.5 ) < 1e-6, "SetXSecWidthHeight did not take"

    # The neighbouring sections have to be left alone.
    xsec_1 = GetXSec( xsec_surf, 1 )

    assert abs( GetXSecWidth( xsec_1 ) - 1.5 ) > 1e-6 or abs( GetXSecHeight( xsec_1 ) - 1.5 ) > 1e-6, "SetXSecWidthHeight reached a section it was not given"

    \endcode
    \endPythonOnly
    \sa SetXSecWidth, SetXSecHeight
    \param [in] xsec_id string XSec ID
    \param [in] w double Xsec width
    \param [in] h double Xsec height
*/

extern void SetXSecWidthHeight( const std::string& xsec_id, double w, double h );

/*!
    \ingroup XSec
*/
/*!
    Set the width of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what is input to SetXSecWidth.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    SetXSecWidth( xsec_2, 1.5 );
    if ( !closeTo( GetXSecWidth( xsec_2 ), 1.5, 1e-9 ) )
    {
        Print( "ERROR: SetXSecWidth did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    SetXSecWidth( xsec_2, 1.5 )
    assert abs( GetXSecWidth( xsec_2 ) - 1.5 ) < 1e-9, "SetXSecWidth did not take"


    \endcode
    \endPythonOnly
    \sa GetXSecWidth
    \param [in] xsec_id string XSec ID
    \param [in] w double Xsec width
*/

extern void SetXSecWidth( const std::string& xsec_id, double w );

/*!
    \ingroup XSec
*/
/*!
    Set the height of an XSec. Note that POINT type XSecs have a width and height of 0, regardless of what is input to SetXSecHeight.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    SetXSecHeight( xsec_2, 1.5 );
    if ( !closeTo( GetXSecHeight( xsec_2 ), 1.5, 1e-9 ) )
    {
        Print( "ERROR: SetXSecHeight did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    SetXSecHeight( xsec_2, 1.5 )
    assert abs( GetXSecHeight( xsec_2 ) - 1.5 ) < 1e-9, "SetXSecHeight did not take"


    \endcode
    \endPythonOnly
    \sa GetXSecHeight
    \param [in] xsec_id string XSec ID
    \param [in] h double Xsec height
*/

extern void SetXSecHeight( const std::string& xsec_id, double h );

/*!
    \ingroup XSec
*/
/*!
    Get all Parm IDs for specified XSec Parm Container
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    array< string > @parm_array = GetXSecParmIDs( xsec );

    if ( parm_array.size() < 1 )                        { Print( "---> Error: API GetXSecParmIDs " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    parm_array = GetXSecParmIDs( xsec )

    if  len(parm_array) < 1 :
        print( "---> Error: API GetXSecParmIDs " )
        assert False, "---> Error: API GetXSecParmIDs"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \return vector\<string\> Array of Parm IDs
*/

extern std::vector<std::string> GetXSecParmIDs( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get a specific Parm ID from an Xsec
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    if ( !ValidParm( wid ) )                            { Print( "---> Error: API GetXSecParm " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    if  not ValidParm( wid ) :
        print( "---> Error: API GetXSecParm " )
        assert False, "---> Error: API GetXSecParm"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] name string Parm name
    \return string Parm ID
*/

extern std::string GetXSecParm( const std::string& xsec_id, const std::string& name );

/*!
    \ingroup XSec
*/
/*!
    Read in XSec shape from fuselage (\\*.fsx) file and set to the specified XSec. The XSec must be of type XS_FILE_FUSE.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE );

    string xsec = GetXSec( xsec_surf, 2 );

    array< vec3d > @vec_array = ReadFileXSec( xsec, "TestXSec.fxs" );

    Update();

    // The file holds a closed diamond, so the first and last points coincide
    // and the section takes its size from their extents.
    if ( vec_array.size() < 3 )
    {
        Print( "ERROR: ReadFileXSec returned too few points" );
        __failure++;
    }
    else
    {
        if ( dist( vec_array[0], vec_array[vec_array.size() - 1] ) > 1e-8 )
        {
            Print( "ERROR: ReadFileXSec returned an open curve" );
            __failure++;
        }

        // The shape is normalized on the way in and then scaled by the
        // section's own width and height, so the curve has to fit inside them.
        double w = GetXSecWidth( xsec );
        double h = GetXSecHeight( xsec );

        for ( int i = 0; i < 11; i++ )
        {
            vec3d p = ComputeXSecPnt( xsec, i * 0.1 );

            if ( abs( p.y() ) > 0.5 * w + 1e-6 || abs( p.z() ) > 0.5 * h + 1e-6 )
            {
                Print( "ERROR: ReadFileXSec left the curve outside the section" );
                __failure++;
            }
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE )

    xsec = GetXSec( xsec_surf, 2 )

    vec_array = ReadFileXSec(xsec, "TestXSec.fxs")

    Update()

    # The file holds a closed diamond, so the first and last points coincide and
    # the section takes its size from their extents.
    assert len( vec_array ) >= 3, "ReadFileXSec returned too few points"
    assert dist( vec_array[0], vec_array[-1] ) < 1e-8, "ReadFileXSec returned an open curve"

    # The shape is normalized on the way in and then scaled by the section's own
    # width and height, so the curve has to fit inside them.
    w = GetXSecWidth( xsec )
    h = GetXSecHeight( xsec )

    for i in range( 11 ):
        p = ComputeXSecPnt( xsec, i * 0.1 )

        assert abs( p.y() ) <= 0.5 * w + 1e-6, "ReadFileXSec left the curve outside the section"
        assert abs( p.z() ) <= 0.5 * h + 1e-6, "ReadFileXSec left the curve outside the section"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] file_name string Fuselage XSec file name
    \return vector\<vec3d\> Array of coordinate points read from the file and set to the XSec
*/

extern std::vector<vec3d> ReadFileXSec( const std::string& xsec_id, const std::string& file_name );

/*!
    \ingroup XSec
*/
/*!
    Set the coordinate points for a specific XSec. The XSec must be of type XS_FILE_FUSE.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE );

    string xsec = GetXSec( xsec_surf, 2 );

    array< vec3d > @vec_array = ReadFileXSec( xsec, "TestXSec.fxs" );

    if ( vec_array.size() == 0 )
    {
        Print( "ERROR: ReadFileXSec returned no points" );
        __failure++;
    }
    else
    {
        vec_array[1] = vec_array[1] * 2.0;
        vec_array[3] = vec_array[3] * 2.0;

        // A file XSec takes its width and height from the extents of the
        // points it was given, in X and Y respectively.
        double wmin = vec_array[0].x(), wmax = vec_array[0].x();
        double hmin = vec_array[0].y(), hmax = vec_array[0].y();

        for ( int i = 1; i < int( vec_array.size() ); i++ )
        {
            if ( vec_array[i].x() < wmin ) { wmin = vec_array[i].x(); }
            if ( vec_array[i].x() > wmax ) { wmax = vec_array[i].x(); }
            if ( vec_array[i].y() < hmin ) { hmin = vec_array[i].y(); }
            if ( vec_array[i].y() > hmax ) { hmax = vec_array[i].y(); }
        }

        SetXSecPnts( xsec, vec_array );

        Update();

        if ( !closeTo( GetXSecWidth( xsec ), wmax - wmin, 1e-6 ) )
        {
            Print( "ERROR: SetXSecPnts did not set the section width" );
            __failure++;
        }

        if ( !closeTo( GetXSecHeight( xsec ), hmax - hmin, 1e-6 ) )
        {
            Print( "ERROR: SetXSecPnts did not set the section height" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE )

    xsec = GetXSec( xsec_surf, 2 )

    # ReadFileXSec hands back a tuple, so copy it into a list to edit it.
    vec_array = list( ReadFileXSec(xsec, "TestXSec.fxs") )

    assert len( vec_array ) > 0, "ReadFileXSec returned no points"

    # The Python vec3d carries no arithmetic operators, so scale by component.
    vec_array[1] = vec3d( vec_array[1].x() * 2.0, vec_array[1].y() * 2.0, vec_array[1].z() * 2.0 )
    vec_array[3] = vec3d( vec_array[3].x() * 2.0, vec_array[3].y() * 2.0, vec_array[3].z() * 2.0 )

    # A file XSec takes its width and height from the extents of the points it
    # was given, in X and Y respectively.
    wmin = min( [ p.x() for p in vec_array ] )
    wmax = max( [ p.x() for p in vec_array ] )
    hmin = min( [ p.y() for p in vec_array ] )
    hmax = max( [ p.y() for p in vec_array ] )

    SetXSecPnts( xsec, vec_array )

    Update()

    assert abs( GetXSecWidth( xsec ) - ( wmax - wmin ) ) < 1e-6, "SetXSecPnts did not set the section width"
    assert abs( GetXSecHeight( xsec ) - ( hmax - hmin ) ) < 1e-6, "SetXSecPnts did not set the section height"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] pnt_vec vector<vec3d> Vector of XSec coordinate points
*/

extern void SetXSecPnts( const std::string& xsec_id, std::vector< vec3d > & pnt_vec );

/*!
    \ingroup XSec
*/
/*!
    Compute 3D coordinate for a point on an XSec curve given the parameter value (U) along the curve
    \forcpponly
    \code{.cpp}
    //==== Add Geom ====//
    string stack_id = AddGeom( "STACK" );

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( stack_id, 0 );

    string xsec = GetXSec( xsec_surf, 2 );

    double u_fract = 0.25;

    vec3d pnt = ComputeXSecPnt( xsec, u_fract );

    // The section is a closed curve, so the ends meet.
    if ( dist( ComputeXSecPnt( xsec, 0.0 ), ComputeXSecPnt( xsec, 1.0 ) ) > 1e-6 )
    {
        Print( "ERROR: the XSec curve does not close" );
        __failure++;
    }

    // The point has to lie on the section, which is sized by its width and
    // height about the section origin.
    double w = GetXSecWidth( xsec );
    double h = GetXSecHeight( xsec );

    if ( abs( pnt.y() ) > 0.5 * w + 1e-6 || abs( pnt.z() ) > 0.5 * h + 1e-6 )
    {
        Print( "ERROR: ComputeXSecPnt returned a point off the section" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Geom ====//
    stack_id = AddGeom( "STACK" )

    #==== Get The XSec Surf ====//
    xsec_surf = GetXSecSurf( stack_id, 0 )

    xsec = GetXSec( xsec_surf, 2 )

    u_fract = 0.25

    pnt = ComputeXSecPnt(xsec, u_fract)

    # The section is a closed curve, so the ends meet.
    assert dist( ComputeXSecPnt( xsec, 0.0 ), ComputeXSecPnt( xsec, 1.0 ) ) < 1e-6, "the XSec curve does not close"

    # The point has to lie on the section, which is sized by its width and
    # height about the section origin.
    w = GetXSecWidth( xsec )
    h = GetXSecHeight( xsec )

    assert abs( pnt.y() ) <= 0.5 * w + 1e-6, "ComputeXSecPnt returned a point off the section"
    assert abs( pnt.z() ) <= 0.5 * h + 1e-6, "ComputeXSecPnt returned a point off the section"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] fract double Curve parameter value (range: 0 - 1)
    \return vec3d 3D coordinate point
*/

extern vec3d ComputeXSecPnt( const std::string& xsec_id, double fract );

/*!
    \ingroup XSec
*/
/*!
    Compute the tangent vector of a point on an XSec curve given the parameter value (U) along the curve
    \forcpponly
    \code{.cpp}
    //==== Add Geom ====//
    string stack_id = AddGeom( "STACK" );

    //==== Get The XSec Surf ====//
    string xsec_surf = GetXSecSurf( stack_id, 0 );

    string xsec = GetXSec( xsec_surf, 2 );

    double u_fract = 0.25;

    vec3d tan = ComputeXSecTan( xsec, u_fract );

    // A tangent is a direction, so it has to have some length.
    if ( tan.mag() < 1e-9 )
    {
        Print( "ERROR: ComputeXSecTan returned a degenerate tangent" );
        __failure++;
    }

    // The tangent has to follow the curve, so stepping along the curve from the
    // point has to line up with it.
    double du = 1.0e-5;

    vec3d p0 = ComputeXSecPnt( xsec, u_fract );
    vec3d p1 = ComputeXSecPnt( xsec, u_fract + du );

    vec3d fd = p1 - p0;

    if ( fd.mag() < 1e-12 )
    {
        Print( "ERROR: the XSec curve does not advance" );
        __failure++;
    }
    else
    {
        double align = dot( fd, tan ) / ( fd.mag() * tan.mag() );

        if ( align < 0.999 )
        {
            Print( "ERROR: ComputeXSecTan does not follow the curve" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Geom ====//
    stack_id = AddGeom( "STACK" )

    #==== Get The XSec Surf ====//
    xsec_surf = GetXSecSurf( stack_id, 0 )

    xsec = GetXSec( xsec_surf, 2 )

    u_fract = 0.25

    tan = ComputeXSecTan( xsec, u_fract )

    # A tangent is a direction, so it has to have some length.
    assert tan.mag() > 1e-9, "ComputeXSecTan returned a degenerate tangent"

    # The tangent has to follow the curve, so stepping along the curve from the
    # point has to line up with it.
    du = 1.0e-5

    p0 = ComputeXSecPnt( xsec, u_fract )
    p1 = ComputeXSecPnt( xsec, u_fract + du )

    fd = vec3d( p1.x() - p0.x(), p1.y() - p0.y(), p1.z() - p0.z() )

    assert fd.mag() > 1e-12, "the XSec curve does not advance"

    align = ( fd.x() * tan.x() + fd.y() * tan.y() + fd.z() * tan.z() ) / ( fd.mag() * tan.mag() )

    assert align > 0.999, "ComputeXSecTan does not follow the curve"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] fract double Curve parameter value (range: 0 - 1)
    \return vec3d Tangent vector
*/

extern vec3d ComputeXSecTan( const std::string& xsec_id, double fract );

/*!
    \ingroup XSec
*/
/*!
    Reset all skinning Parms for a specified XSec. Set top, bottom, left, and right strengths, slew, angle, and curvature to 0. Set all symmetry and equality conditions to false.
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

    int num_xsecs = GetNumXSec( xsec_surf );

    string xsec = GetXSec( xsec_surf, 1 );

    SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 15.0 );      // Set Tangent Angles At Cross Section
    SetXSecContinuity( xsec, 1 );                       // Set Continuity At Cross Section

    if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TopLAngle" ) ), 15.0, 1e-6 ) )
    {
        Print( "ERROR: the skin Parms were never set" );
        __failure++;
    }

    ResetXSecSkinParms( xsec );

    // Resetting zeroes every skin value on all four sides and turns the
    // symmetry flags back on.  Continuity is left alone.
    if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TopLAngle" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "TopRAngle" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "TopLSlew" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "TopLStrength" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "TopLCurve" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "RightLAngle" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "BottomLAngle" ) ), 0.0, 1e-6 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "LeftLAngle" ) ), 0.0, 1e-6 ) )
    {
        Print( "ERROR: ResetXSecSkinParms did not zero the skin Parms" );
        __failure++;
    }

    if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TBSym" ) ), 1.0, 1e-12 ) ||
         !closeTo( GetParmVal( GetXSecParm( xsec, "RLSym" ) ), 1.0, 1e-12 ) )
    {
        Print( "ERROR: ResetXSecSkinParms did not restore the symmetry flags" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    xsec_surf = GetXSecSurf( fid, 0 )           # Get First (and Only) XSec Surf

    num_xsecs = GetNumXSec( xsec_surf )

    xsec = GetXSec( xsec_surf, 1 )

    SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 15.0, -1.0e12, -1.0e12, -1.0e12 )      # Set Tangent Angles At Cross Section
    SetXSecContinuity( xsec, 1 )                       # Set Continuity At Cross Section

    assert abs( GetParmVal( GetXSecParm( xsec, "TopLAngle" ) ) - 15.0 ) < 1e-6, "the skin Parms were never set"

    ResetXSecSkinParms( xsec )

    # Resetting zeroes every skin value on all four sides and turns the symmetry
    # flags back on.  Continuity is left alone.
    for skin_parm in [ "TopLAngle", "TopRAngle", "TopLSlew", "TopLStrength", "TopLCurve",
                       "RightLAngle", "BottomLAngle", "LeftLAngle" ]:
        assert abs( GetParmVal( GetXSecParm( xsec, skin_parm ) ) ) < 1e-6, "ResetXSecSkinParms did not zero " + skin_parm

    assert abs( GetParmVal( GetXSecParm( xsec, "TBSym" ) ) - 1.0 ) < 1e-12, "ResetXSecSkinParms did not restore the symmetry flags"
    assert abs( GetParmVal( GetXSecParm( xsec, "RLSym" ) ) - 1.0 ) < 1e-12, "ResetXSecSkinParms did not restore the symmetry flags"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
*/

extern void ResetXSecSkinParms( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Set C-type continuity enforcement for a particular XSec
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string xsec_surf = GetXSecSurf( fid, 0 );           // Get First (and Only) XSec Surf

    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecContinuity( xsec, 1 );                       // Set Continuity At Cross Section

        // The setter is shorthand for the section's continuity Parm.
        if ( !closeTo( GetParmVal( GetXSecParm( xsec, "ContinuityTop" ) ), 1.0, 1e-12 ) )
        {
            Print( "ERROR: SetXSecContinuity did not set section " + i );
            __failure++;
        }

        SetXSecContinuity( xsec, 0 );

        if ( !closeTo( GetParmVal( GetXSecParm( xsec, "ContinuityTop" ) ), 0.0, 1e-12 ) )
        {
            Print( "ERROR: SetXSecContinuity did not clear section " + i );
            __failure++;
        }

        SetXSecContinuity( xsec, 1 );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    xsec_surf = GetXSecSurf( fid, 0 )           # Get First (and Only) XSec Surf

    num_xsecs = GetNumXSec( xsec_surf )

    for i in range(num_xsecs):

        xsec = GetXSec( xsec_surf, i )

        SetXSecContinuity( xsec, 1 )                       # Set Continuity At Cross Section

        # The setter is shorthand for the section's continuity Parm.
        assert abs( GetParmVal( GetXSecParm( xsec, "ContinuityTop" ) ) - 1.0 ) < 1e-12, "SetXSecContinuity did not set section " + str( i )

        SetXSecContinuity( xsec, 0 )

        assert abs( GetParmVal( GetXSecParm( xsec, "ContinuityTop" ) ) ) < 1e-12, "SetXSecContinuity did not clear section " + str( i )

        SetXSecContinuity( xsec, 1 )

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] cx int Continuity level (0, 1, or 2)
*/

extern void SetXSecContinuity( const std::string& xsec_id, int cx );

/*!
    \ingroup XSec
*/
/*!
    Set the tangent angles for the specified XSec
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 10.0 );       // Set Tangent Angles At Cross Section

        // The setter is shorthand for the skin Parms, so the value has to show
        // up on both sides of the section.
        if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TopLAngle" ) ), 10.0, 1e-6 ) ||
             !closeTo( GetParmVal( GetXSecParm( xsec, "TopRAngle" ) ), 10.0, 1e-6 ) )
        {
            Print( "ERROR: SetXSecTanAngles did not set the top of section " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    num_xsecs = GetNumXSec( xsec_surf )

    for i in range(num_xsecs):

        xsec = GetXSec( xsec_surf, i )

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 10.0, -1.0e12, -1.0e12, -1.0e12 )       # Set Tangent Angles At Cross Section

        # The setter is shorthand for the skin Parms, so the value has to show up
        # on both sides of the section.
        assert abs( GetParmVal( GetXSecParm( xsec, "TopLAngle" ) ) - 10.0 ) < 1e-6, "SetXSecTanAngles did not set the top of section " + str( i )
        assert abs( GetParmVal( GetXSecParm( xsec, "TopRAngle" ) ) - 10.0 ) < 1e-6, "SetXSecTanAngles did not set the top of section " + str( i )

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id string XSec ID
    \param [in] side int Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top double Top angle (degrees)
    \param [in] right double Right angle (degrees)
    \param [in] bottom double Bottom angle (degrees)
    \param [in] left double Left angle (degrees)
*/

extern void SetXSecTanAngles( const std::string& xsec_id, int side, double top, double right, double bottom, double left );

/*!
    \ingroup XSec
*/
/*!
    Set the tangent slew angles for the specified XSec
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanSlews( xsec, XSEC_BOTH_SIDES, 5.0 );       // Set Tangent Slews At Cross Section

        // The setter is shorthand for the skin Parms, so the value has to show
        // up on both sides of the section.
        if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TopLSlew" ) ), 5.0, 1e-6 ) ||
             !closeTo( GetParmVal( GetXSecParm( xsec, "TopRSlew" ) ), 5.0, 1e-6 ) )
        {
            Print( "ERROR: SetXSecTanSlews did not set the top of section " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    num_xsecs = GetNumXSec( xsec_surf )

    for i in range(num_xsecs):

        xsec = GetXSec( xsec_surf, i )

        SetXSecTanSlews( xsec, XSEC_BOTH_SIDES, 5.0, -1.0e12, -1.0e12, -1.0e12 )       # Set Tangent Slews At Cross Section

        # The setter is shorthand for the skin Parms, so the value has to show up
        # on both sides of the section.
        assert abs( GetParmVal( GetXSecParm( xsec, "TopLSlew" ) ) - 5.0 ) < 1e-6, "SetXSecTanSlews did not set the top of section " + str( i )
        assert abs( GetParmVal( GetXSecParm( xsec, "TopRSlew" ) ) - 5.0 ) < 1e-6, "SetXSecTanSlews did not set the top of section " + str( i )

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id string XSec ID
    \param [in] side int Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top double Top angle (degrees)
    \param [in] right double Right angle (degrees)
    \param [in] bottom double Bottom angle (degrees)
    \param [in] left double Left angle (degrees)
*/

extern void SetXSecTanSlews( const std::string& xsec_id, int side, double top, double right, double bottom, double left );

/*!
    \ingroup XSec
*/
/*!
    Set the tangent strengths for the specified XSec
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Flatten ends
    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.8 );  // Set Tangent Strengths At Cross Section

        // The setter is shorthand for the skin Parms, so the value has to show
        // up on both sides of the section.
        if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TopLStrength" ) ), 0.8, 1e-6 ) ||
             !closeTo( GetParmVal( GetXSecParm( xsec, "TopRStrength" ) ), 0.8, 1e-6 ) )
        {
            Print( "ERROR: SetXSecTanStrengths did not set the top of section " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Flatten ends
    num_xsecs = GetNumXSec( xsec_surf )

    for i in range(num_xsecs):

        xsec = GetXSec( xsec_surf, i )

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.8, -1.0e12, -1.0e12, -1.0e12 )  # Set Tangent Strengths At Cross Section

        # The setter is shorthand for the skin Parms, so the value has to show up
        # on both sides of the section.
        assert abs( GetParmVal( GetXSecParm( xsec, "TopLStrength" ) ) - 0.8 ) < 1e-6, "SetXSecTanStrengths did not set the top of section " + str( i )
        assert abs( GetParmVal( GetXSecParm( xsec, "TopRStrength" ) ) - 0.8 ) < 1e-6, "SetXSecTanStrengths did not set the top of section " + str( i )

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id string XSec ID
    \param [in] side int Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top double Top strength
    \param [in] right double Right strength
    \param [in] bottom double Bottom strength
    \param [in] left double Left strength
*/

extern void SetXSecTanStrengths( const std::string& xsec_id, int side, double top, double right, double bottom, double left );

/*!
    \ingroup XSec
*/
/*!
    Set curvatures for the specified XSec
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    // Flatten ends
    int num_xsecs = GetNumXSec( xsec_surf );

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecCurvatures( xsec, XSEC_BOTH_SIDES, 0.2 );  // Set Tangent Strengths At Cross Section

        // The setter is shorthand for the skin Parms, so the value has to show
        // up on both sides of the section.
        if ( !closeTo( GetParmVal( GetXSecParm( xsec, "TopLCurve" ) ), 0.2, 1e-6 ) ||
             !closeTo( GetParmVal( GetXSecParm( xsec, "TopRCurve" ) ), 0.2, 1e-6 ) )
        {
            Print( "ERROR: SetXSecCurvatures did not set the top of section " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    # Flatten ends
    num_xsecs = GetNumXSec( xsec_surf )

    for i in range(num_xsecs):

        xsec = GetXSec( xsec_surf, i )

        SetXSecCurvatures( xsec, XSEC_BOTH_SIDES, 0.2, -1.0e12, -1.0e12, -1.0e12 )  # Set Curvatures At Cross Section

        # The setter is shorthand for the skin Parms, so the value has to show up
        # on both sides of the section.
        assert abs( GetParmVal( GetXSecParm( xsec, "TopLCurve" ) ) - 0.2 ) < 1e-6, "SetXSecCurvatures did not set the top of section " + str( i )
        assert abs( GetParmVal( GetXSecParm( xsec, "TopRCurve" ) ) - 0.2 ) < 1e-6, "SetXSecCurvatures did not set the top of section " + str( i )

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id string XSec ID
    \param [in] side int Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top double Top curvature
    \param [in] right double Right curvature
    \param [in] bottom double Bottom curvature
    \param [in] left double Left curvature
*/

extern void SetXSecCurvatures( const std::string& xsec_id, int side, double top, double right, double bottom, double left );

/*!
    \ingroup XSec
*/
/*!
    Read in XSec shape from airfoil file and set to the specified XSec. The XSec must be of type XS_FILE_AIRFOIL. Airfoil files may be in Lednicer or Selig format with \\*.af or \\*.dat extensions.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );
    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );

    if ( up_array.size() == 0 || up_array.size() != low_array.size() )
    {
        Print( "ERROR: ReadFileAirfoil did not read matching surfaces" );
        __failure++;
    }
    else
    {
        // The points run from the leading edge to the trailing edge on a unit
        // chord.
        if ( !closeTo( up_array[0].x(), 0.0, 1e-6 ) ||
             !closeTo( up_array[up_array.size() - 1].x(), 1.0, 1e-6 ) ||
             !closeTo( low_array[0].x(), 0.0, 1e-6 ) )
        {
            Print( "ERROR: ReadFileAirfoil did not normalize the chord" );
            __failure++;
        }

        // A NACA 0012 is symmetric, so the lower surface mirrors the upper, and
        // the section is twelve percent thick.
        double max_up = 0.0;

        for ( int i = 0; i < int( up_array.size() ); i++ )
        {
            if ( !closeTo( low_array[i].y(), -up_array[i].y(), 1e-6 ) )
            {
                Print( "ERROR: ReadFileAirfoil did not read a symmetric section" );
                __failure++;
            }

            if ( up_array[i].y() > max_up )
            {
                max_up = up_array[i].y();
            }
        }

        if ( !closeTo( 2.0 * max_up, 0.12, 1e-3 ) )
        {
            Print( "ERROR: ReadFileAirfoil did not read a twelve percent section" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL )

    xsec = GetXSec( xsec_surf, 1 )

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" )

    up_array = GetAirfoilUpperPnts( xsec )
    low_array = GetAirfoilLowerPnts( xsec )

    assert len( up_array ) > 0, "ReadFileAirfoil did not read matching surfaces"
    assert len( up_array ) == len( low_array ), "ReadFileAirfoil did not read matching surfaces"

    # The points run from the leading edge to the trailing edge on a unit chord.
    assert abs( up_array[0].x() ) < 1e-6, "ReadFileAirfoil did not normalize the chord"
    assert abs( up_array[-1].x() - 1.0 ) < 1e-6, "ReadFileAirfoil did not normalize the chord"
    assert abs( low_array[0].x() ) < 1e-6, "ReadFileAirfoil did not normalize the chord"

    # A NACA 0012 is symmetric, so the lower surface mirrors the upper, and the
    # section is twelve percent thick.
    for i in range( len( up_array ) ):
        assert abs( low_array[i].y() + up_array[i].y() ) < 1e-6, "ReadFileAirfoil did not read a symmetric section"

    max_up = max( [ p.y() for p in up_array ] )

    assert abs( 2.0 * max_up - 0.12 ) < 1e-3, "ReadFileAirfoil did not read a twelve percent section"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] file_name string Airfoil XSec file name
*/

extern void ReadFileAirfoil( const std::string& xsec_id, const std::string& file_name );

/*!
    \ingroup XSec
*/
/*!
    Set the upper points for an airfoil. The XSec must be of type XS_FILE_AIRFOIL.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );
    }

    SetAirfoilUpperPnts( xsec, up_array );

    array< vec3d > @check_array = GetAirfoilUpperPnts( xsec );

    if ( check_array.length() != up_array.length() )
    {
        Print( "ERROR: SetAirfoilUpperPnts point count" );
        __failure++;
    }
    else
    {
        // The doubled upper surface has to come back doubled, and the lower
        // surface has to be left alone.
        for ( int i = 0; i < int( up_array.size() ); i++ )
        {
            if ( dist( check_array[i], up_array[i] ) > 1e-6 )
            {
                Print( "ERROR: SetAirfoilUpperPnts did not store point " + i );
                __failure++;
            }
        }

        array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );

        double max_up = 0.0;
        double min_low = 0.0;

        for ( int i = 0; i < int( check_array.size() ); i++ )
        {
            if ( check_array[i].y() > max_up ) { max_up = check_array[i].y(); }
            if ( low_array[i].y() < min_low ) { min_low = low_array[i].y(); }
        }

        if ( !closeTo( max_up, -2.0 * min_low, 1e-6 ) )
        {
            Print( "ERROR: SetAirfoilUpperPnts did not leave the lower surface alone" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL )

    xsec = GetXSec( xsec_surf, 1 )

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" )

    up_array = GetAirfoilUpperPnts( xsec )

    for i in range(int( len(up_array) )):

        up_array[i].scale_y( 2.0 )

    SetAirfoilUpperPnts( xsec, up_array )

    check_array = GetAirfoilUpperPnts( xsec )

    assert len( check_array ) == len( up_array ), "SetAirfoilUpperPnts point count"

    # The doubled upper surface has to come back doubled, and the lower surface
    # has to be left alone.
    for i in range( len( up_array ) ):
        assert dist( check_array[i], up_array[i] ) < 1e-6, "SetAirfoilUpperPnts did not store point " + str( i )

    low_array = GetAirfoilLowerPnts( xsec )

    max_up = max( [ p.y() for p in check_array ] )
    min_low = min( [ p.y() for p in low_array ] )

    assert abs( max_up + 2.0 * min_low ) < 1e-6, "SetAirfoilUpperPnts did not leave the lower surface alone"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] up_pnt_vec vector<vec3d> Array of points defining the upper surface of the airfoil
*/

extern void SetAirfoilUpperPnts( const std::string& xsec_id, const std::vector< vec3d > & up_pnt_vec );

/*!
    \ingroup XSec
*/
/*!
    Set the lower points for an airfoil. The XSec must be of type XS_FILE_AIRFOIL.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );

    for ( int i = 0 ; i < int( low_array.size() ) ; i++ )
    {
        low_array[i].scale_y( 0.5 );
    }

    SetAirfoilLowerPnts( xsec, low_array );

    array< vec3d > @check_array = GetAirfoilLowerPnts( xsec );

    if ( check_array.length() != low_array.length() )
    {
        Print( "ERROR: SetAirfoilLowerPnts point count" );
        __failure++;
    }
    else
    {
        // The halved lower surface has to come back halved, and the upper
        // surface has to be left alone.
        for ( int i = 0; i < int( low_array.size() ); i++ )
        {
            if ( dist( check_array[i], low_array[i] ) > 1e-6 )
            {
                Print( "ERROR: SetAirfoilLowerPnts did not store point " + i );
                __failure++;
            }
        }

        array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );

        double max_up = 0.0;
        double min_low = 0.0;

        for ( int i = 0; i < int( check_array.size() ); i++ )
        {
            if ( up_array[i].y() > max_up ) { max_up = up_array[i].y(); }
            if ( check_array[i].y() < min_low ) { min_low = check_array[i].y(); }
        }

        if ( !closeTo( 0.5 * max_up, -min_low, 1e-6 ) )
        {
            Print( "ERROR: SetAirfoilLowerPnts did not leave the upper surface alone" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL )

    xsec = GetXSec( xsec_surf, 1 )

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" )

    low_array = GetAirfoilLowerPnts( xsec )

    for i in range(int( len(low_array) )):

        low_array[i].scale_y( 0.5 )

    SetAirfoilLowerPnts( xsec, low_array )

    check_array = GetAirfoilLowerPnts( xsec )

    assert len( check_array ) == len( low_array ), "SetAirfoilLowerPnts point count"

    # The halved lower surface has to come back halved, and the upper surface
    # has to be left alone.
    for i in range( len( low_array ) ):
        assert dist( check_array[i], low_array[i] ) < 1e-6, "SetAirfoilLowerPnts did not store point " + str( i )

    up_array = GetAirfoilUpperPnts( xsec )

    max_up = max( [ p.y() for p in up_array ] )
    min_low = min( [ p.y() for p in check_array ] )

    assert abs( 0.5 * max_up + min_low ) < 1e-6, "SetAirfoilLowerPnts did not leave the upper surface alone"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] low_pnt_vec vector<vec3d> Array of points defining the lower surface of the airfoil
*/

extern void SetAirfoilLowerPnts( const std::string& xsec_id, const std::vector< vec3d > & low_pnt_vec );

/*!
    \ingroup XSec
*/
/*!
    Set the upper and lower points for an airfoil. The XSec must be of type XS_FILE_AIRFOIL.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );

    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );

        low_array[i].scale_y( 0.5 );
    }

    SetAirfoilPnts( xsec, up_array, low_array );

    array< vec3d > @check_up = GetAirfoilUpperPnts( xsec );
    array< vec3d > @check_low = GetAirfoilLowerPnts( xsec );

    if ( check_up.length() != up_array.length() || check_low.length() != low_array.length() )
    {
        Print( "ERROR: SetAirfoilPnts point count" );
        __failure++;
    }
    else
    {
        for ( int i = 0; i < int( up_array.size() ); i++ )
        {
            if ( dist( check_up[i], up_array[i] ) > 1e-6 || dist( check_low[i], low_array[i] ) > 1e-6 )
            {
                Print( "ERROR: SetAirfoilPnts did not store point " + i );
                __failure++;
            }
        }

        // The section started symmetric; doubling the top and halving the
        // bottom leaves the top four times as deep as the bottom.
        double max_up = 0.0;
        double min_low = 0.0;

        for ( int i = 0; i < int( check_up.size() ); i++ )
        {
            if ( check_up[i].y() > max_up ) { max_up = check_up[i].y(); }
            if ( check_low[i].y() < min_low ) { min_low = check_low[i].y(); }
        }

        if ( !closeTo( max_up, -4.0 * min_low, 1e-6 ) )
        {
            Print( "ERROR: SetAirfoilPnts did not scale the two surfaces apart" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL )

    xsec = GetXSec( xsec_surf, 1 )

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" )

    up_array = GetAirfoilUpperPnts( xsec )

    low_array = GetAirfoilLowerPnts( xsec )

    for i in range(int( len(up_array) )):

        up_array[i].scale_y( 2.0 )

        low_array[i].scale_y( 0.5 )

    SetAirfoilPnts( xsec, up_array, low_array )

    check_up = GetAirfoilUpperPnts( xsec )
    check_low = GetAirfoilLowerPnts( xsec )

    assert len( check_up ) == len( up_array ), "SetAirfoilPnts point count"
    assert len( check_low ) == len( low_array ), "SetAirfoilPnts point count"

    for i in range( len( up_array ) ):
        assert dist( check_up[i], up_array[i] ) < 1e-6, "SetAirfoilPnts did not store point " + str( i )
        assert dist( check_low[i], low_array[i] ) < 1e-6, "SetAirfoilPnts did not store point " + str( i )

    # The section started symmetric; doubling the top and halving the bottom
    # leaves the top four times as deep as the bottom.
    max_up = max( [ p.y() for p in check_up ] )
    min_low = min( [ p.y() for p in check_low ] )

    assert abs( max_up + 4.0 * min_low ) < 1e-6, "SetAirfoilPnts did not scale the two surfaces apart"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] up_pnt_vec vector<vec3d> Array of points defining the upper surface of the airfoil
    \param [in] low_pnt_vec vector<vec3d> Array of points defining the lower surface of the airfoil
*/

extern void SetAirfoilPnts( const std::string& xsec_id, const std::vector< vec3d > & up_pnt_vec, const std::vector< vec3d > & low_pnt_vec );

/*!
    \ingroup XSec
*/
/*!
    Get the theoretical lift (Cl) distribution for a Hershey Bar wing with unit chord length using Glauert's Method. This function was initially created to compare VSPAERO results to Lifting Line Theory.
    If full_span_flag is set to true symmetry is applied to the results.
    \forcpponly
    \code{.cpp}
    // Compute theoretical lift and drag distributions using 100 points
    double Vinf = 100;

    double halfAR = 20;

    double alpha_deg = 10;

    int n_pts = 100;

    array<vec3d> cl_dist_theo = GetHersheyBarLiftDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );
    if ( cl_dist_theo.length() == 0 )
    {
        Print( "ERROR: GetHersheyBarLiftDist returned nothing" );
        __failure++;
    }

    array<vec3d> cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pi = 3.14159265358979323846
    # Compute theoretical lift and drag distributions using 100 points
    Vinf = 100

    halfAR = 20

    alpha_deg = 10

    n_pts = 100

    cl_dist_theo = GetHersheyBarLiftDist( int( n_pts ), alpha_deg*pi/180, Vinf, ( 2 * halfAR ), False )
    assert len( cl_dist_theo ) > 0, "GetHersheyBarLiftDist returned nothing"

    cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), alpha_deg*pi/180, Vinf, ( 2 * halfAR ), False )

    \endcode
    \endPythonOnly
    \param [in] npts int Number of points along the span to assess
    \param [in] alpha double Wing angle of attack (Radians)
    \param [in] Vinf double Freestream velocity
    \param [in] span double Hershey Bar full-span. Note, only half is used in the calculation
    \param [in] full_span_flag bool Flag to apply symmetry to results
    \return vector\<vec3d\> Theoretical coefficient of lift distribution array (size = 2*npts if full_span_flag = true)
*/

extern std::vector<vec3d> GetHersheyBarLiftDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag = false );

/*!
    \ingroup XSec
*/
/*!
    Get the theoretical drag (Cd) distribution for a Hershey Bar wing with unit chord length using Glauert's Method. This function was initially created to compare VSPAERO results to Lifting Line Theory.
    If full_span_flag is set to true symmetry is applied to the results.
    \forcpponly
    \code{.cpp}
    // Compute theoretical lift and drag distributions using 100 points
    double Vinf = 100;

    double halfAR = 20;

    double alpha_deg = 10;

    int n_pts = 100;

    array<vec3d> cl_dist_theo = GetHersheyBarLiftDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );

    array<vec3d> cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), Deg2Rad( alpha_deg ), Vinf, ( 2 * halfAR ), false );
    if ( cd_dist_theo.length() == 0 )
    {
        Print( "ERROR: GetHersheyBarDragDist returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pi = 3.14159265358979323846
    # Compute theoretical lift and drag distributions using 100 points
    Vinf = 100

    halfAR = 20

    alpha_deg = 10

    n_pts = 100

    cl_dist_theo = GetHersheyBarLiftDist( int( n_pts ), alpha_deg*pi/180, Vinf, ( 2 * halfAR ), False )

    cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), alpha_deg*pi/180, Vinf, ( 2 * halfAR ), False )
    assert len( cd_dist_theo ) > 0, "GetHersheyBarDragDist returned nothing"

    \endcode
    \endPythonOnly
    \param [in] npts int Number of points along the span to assess
    \param [in] alpha double Wing angle of attack (Radians)
    \param [in] Vinf double Freestream velocity
    \param [in] span double Hershey Bar full-span. Note, only half is used in the calculation
    \param [in] full_span_flag bool Flag to apply symmetry to results (default: false)
    \return vector\<vec3d\> Theoretical coefficient of drag distribution array (size = 2*npts if full_span_flag = true)
*/

extern std::vector<vec3d> GetHersheyBarDragDist( const int &npts, const double &alpha, const double &Vinf, const double &span, bool full_span_flag = false );

/*!
    \ingroup XSec
*/
/*!
    Get the 2D coordinates an input number of points along a Von K�rm�n-Trefftz airfoil of specified shape
    \forcpponly
    \code{.cpp}
    const double pi = 3.14159265358979323846;

    const int npts = 122;

    const double alpha = 0.0;

    const double epsilon = 0.1;

    const double kappa = 0.1;

    const double tau = 10;

    array<vec3d> xyz_airfoil = GetVKTAirfoilPnts(npts, alpha, epsilon, kappa, tau*(pi/180) );
    if ( xyz_airfoil.length() == 0 )
    {
        Print( "ERROR: GetVKTAirfoilPnts returned nothing" );
        __failure++;
    }

    array<double> cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pi = 3.14159265358979323846

    npts = 122

    alpha = 0.0

    epsilon = 0.1

    kappa = 0.1

    tau = 10

    xyz_airfoil = GetVKTAirfoilPnts(npts, alpha, epsilon, kappa, tau*(pi/180) )
    assert len( xyz_airfoil ) > 0, "GetVKTAirfoilPnts returned nothing"

    cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil )

    \endcode
    \endPythonOnly
    \param [in] npts int Number of points along the airfoil to return
    \param [in] alpha double Airfoil angle of attack (Radians)
    \param [in] epsilon double Airfoil thickness
    \param [in] kappa double Airfoil camber
    \param [in] tau double Airfoil trailing edge angle (Radians)
    \return vector\<vec3d\> Array of points on the VKT airfoil (size = npts)
*/

extern std::vector<vec3d> GetVKTAirfoilPnts( const int &npts, const double &alpha, const double &epsilon, const double &kappa, const double &tau );

/*!
    \ingroup XSec
*/
/*!
    Get the pressure coefficient (Cp) along a Von Kármán-Trefftz airfoil of specified shape at specified points along the airfoil
    \forcpponly
    \code{.cpp}
    const double pi = 3.14159265358979323846;

    const int npts = 122;

    const double alpha = 0.0;

    const double epsilon = 0.1;

    const double kappa = 0.1;

    const double tau = 10;

    array<vec3d> xyz_airfoil = GetVKTAirfoilPnts(npts, alpha, epsilon, kappa, tau*(pi/180) );

    array<double> cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil );
    if ( cp_dist.length() == 0 )
    {
        Print( "ERROR: GetVKTAirfoilCpDist returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pi = 3.14159265358979323846

    npts = 122

    alpha = 0.0

    epsilon = 0.1

    kappa = 0.1

    tau = 10

    xyz_airfoil = GetVKTAirfoilPnts(npts, alpha, epsilon, kappa, tau*(pi/180) )

    cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil )
    assert len( cp_dist ) > 0, "GetVKTAirfoilCpDist returned nothing"

    \endcode
    \endPythonOnly
    \sa GetVKTAirfoilPnts
    \param [in] alpha double Airfoil angle of attack (Radians)
    \param [in] epsilon double Airfoil thickness
    \param [in] kappa double Airfoil camber
    \param [in] tau double Airfoil trailing edge angle (Radians)
    \param [in] xyz_data vector<vec3d> Vector of points on the airfoil to evaluate
    \return vector\<double\> Vector of Cp values for each point in xydata
*/

extern std::vector<double> GetVKTAirfoilCpDist( const double &alpha, const double &epsilon, const double &kappa, const double &tau, const std::vector<vec3d> &xyz_data );

/*!
    \ingroup XSec
*/
/*!
    Generate the surface coordinate points for a ellipsoid at specified center of input radius along each axis.
    Based on the MATLAB function ellipsoid (https://in.mathworks.com/help/matlab/ref/ellipsoid.html).
    \sa GetVKTAirfoilPnts
    \param [in] center vec3d 3D location of the ellipsoid center
    \param [in] abc_rad vec3d Radius along the A (X), B (Y), and C (Z) axes
    \param [in] u_npts int Number of points in the U direction
    \param [in] w_npts int Number of points in the W direction
    \return vector\<vec3d\> Array of coordinates describing the ellipsoid surface
*/

extern std::vector<vec3d> GetEllipsoidSurfPnts( const vec3d &center, const vec3d &abc_rad, int u_npts = 20, int w_npts = 20 );

/*!
    \ingroup XSec
*/
/*!
    Get the points along the feature lines of a particular Geom
    \param [in] geom_id string Geom ID
    \return vector\<vec3d\> Array of points along the Geom's feature lines
*/

extern std::vector<vec3d> GetFeatureLinePnts( const string& geom_id );

/*!
    \ingroup XSec
*/
/*!
    Generate Analytical Solution for Potential Flow for specified ellipsoid shape at input surface points for input velocity vector.
    Based on Munk, M. M., 'Remarks on the Pressure Distribution over the Surface of an Ellipsoid, Moving Translationally Through a Perfect
    Fluid,' NACA TN-196, June 1924. Function initially created to compare VSPAERO results to theory.
    \forcpponly
    \code{.cpp}
    const double pi = 3.14159265358979323846;

    const int npts = 101;

    const vec3d abc_rad = vec3d(1.0, 2.0, 3.0);

    const double alpha = 5; // deg

    const double beta = 5; // deg

    const double V_inf = 100.0;

    array < vec3d > x_slice_pnt_vec(npts);
    array<double> theta_vec(npts);

    theta_vec[0] = 0;

    for ( int i = 1; i < npts; i++ )
    {
        theta_vec[i] = theta_vec[i-1] + (2 * pi / ( npts - 1) );
    }

    for ( int i = 0; i < npts; i++ )
    {
        x_slice_pnt_vec[i] = vec3d( 0, abc_rad[1] * cos( theta_vec[i] ), abc_rad[2] *sin( theta_vec[i] ) );
    }

    vec3d V_vec = vec3d( ( V_inf * cos( Deg2Rad( alpha ) ) * cos( Deg2Rad( beta ) ) ), ( V_inf * sin( Deg2Rad( beta ) ) ), ( V_inf * sin( Deg2Rad( alpha ) ) * cos( Deg2Rad( beta ) ) ) );

    array < double > cp_dist = GetEllipsoidCpDist( x_slice_pnt_vec, abc_rad, V_vec );
    if ( cp_dist.length() == 0 )
    {
        Print( "ERROR: GetEllipsoidCpDist returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    import math
    pi = 3.14159265358979323846

    npts = 101

    abc_rad = vec3d(1.0, 2.0, 3.0)

    alpha = 5 # deg

    beta = 5 # deg

    V_inf = 100.0

    x_slice_pnt_vec = [None]*npts
    theta_vec = [None]*npts

    theta_vec[0] = 0

    for i in range(1, npts):
        theta_vec[i] = theta_vec[i-1] + (2 * pi / (npts - 1))


    for i in range(npts):

        x_slice_pnt_vec[i] = vec3d( 0, abc_rad.y() * math.cos( theta_vec[i] ), abc_rad.z() * math.sin( theta_vec[i] ) )

    V_vec = vec3d( ( V_inf * math.cos( alpha*pi/180 ) * math.cos( beta*pi/180 ) ), ( V_inf * math.sin( beta*pi/180 ) ), ( V_inf * math.sin( alpha*pi/180 ) * math.cos( beta*pi/180 ) ) )

    cp_dist = GetEllipsoidCpDist( x_slice_pnt_vec, abc_rad, V_vec )
    assert len( cp_dist ) > 0, "GetEllipsoidCpDist returned nothing"

    \endcode
    \endPythonOnly
    \sa GetEllipsoidSurfPnts
    \param [in] surf_pnt_vec vector<vec3d> Vector of points on the ellipsoid surface to assess
    \param [in] abc_rad vec3d Radius along the A (X), B (Y), and C (Z) axes
    \param [in] V_inf vec3d 3D components of freestream velocity
    \return vector\<double\> Vector of Cp results corresponding to each point in surf_pnt_arr
*/

extern std::vector<double> GetEllipsoidCpDist( const std::vector<vec3d> &surf_pnt_vec, const vec3d &abc_rad, const vec3d &V_inf );

extern double IntegrateEllipsoidFlow( const vec3d &abc_rad, const int &abc_index );

/*!
    \ingroup XSec
*/
/*!
    Get the coordinate points for the upper surface of an airfoil. The XSec must be of type XS_FILE_AIRFOIL
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetAirfoilUpperPnts( xsec );
    if ( up_array.length() == 0 )
    {
        Print( "ERROR: GetAirfoilUpperPnts returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL )

    xsec = GetXSec( xsec_surf, 1 )

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" )

    up_array = GetAirfoilUpperPnts( xsec )
    assert len( up_array ) > 0, "GetAirfoilUpperPnts returned nothing"

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] xsec_id string XSec ID
    \return vector\<vec3d\> VectorArray of coordinate points for the upper airfoil surface
*/

extern std::vector<vec3d> GetAirfoilUpperPnts( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the coordinate points for the lower surface of an airfoil. The XSec must be of type XS_FILE_AIRFOIL
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetAirfoilLowerPnts( xsec );
    if ( low_array.length() == 0 )
    {
        Print( "ERROR: GetAirfoilLowerPnts returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL )

    xsec = GetXSec( xsec_surf, 1 )

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" )

    low_array = GetAirfoilLowerPnts( xsec )
    assert len( low_array ) > 0, "GetAirfoilLowerPnts returned nothing"

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] xsec_id string XSec ID
    \return vector\<vec3d\> Vector of coordinate points for the lower airfoil surface
*/

extern std::vector<vec3d> GetAirfoilLowerPnts( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the CST coefficients for the upper surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] xsec_id string XSec ID
    \return vector\<double\> Vector of CST coefficients for the upper airfoil surface
*/

extern std::vector<double> GetUpperCSTCoefs( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the CST coefficients for the lower surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] xsec_id string XSec ID
    \return vector\<double\> Vector of CST coefficients for the lower airfoil surface
*/

extern std::vector<double> GetLowerCSTCoefs( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the CST degree for the upper surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] xsec_id string XSec ID
    \return int CST Degree for upper airfoil surface
*/

extern int GetUpperCSTDegree( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the CST degree for the lower surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] xsec_id string XSec ID
    \return int CST Degree for lower airfoil surface
*/

extern int GetLowerCSTDegree( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Set the CST degree and coefficients for the upper surface of an airfoil. The number of coefficients should be one more than the CST degree. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree, GetUpperCSTCoefs
    \param [in] xsec_id string XSec ID
    \param [in] deg int CST degree of upper airfoil surface
    \param [in] coefs vector<double> Vector of CST coefficients for the upper airfoil surface
*/

extern void SetUpperCST( const std::string& xsec_id, int deg, const std::vector<double> &coefs );

/*!
    \ingroup XSec
*/
/*!
    Set the CST degree and coefficients for the lower surface of an airfoil. The number of coefficients should be one more than the CST degree. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree, GetLowerCSTCoefs
    \param [in] xsec_id string XSec ID
    \param [in] deg int CST degree of lower airfoil surface
    \param [in] coefs vector<double> Vector of CST coefficients for the lower airfoil surface
*/

extern void SetLowerCST( const std::string& xsec_id, int deg, const std::vector<double> &coefs );

/*!
    \ingroup XSec
*/
/*!
    Promote the CST for the upper airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] xsec_id string XSec ID
*/

extern void PromoteCSTUpper( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Promote the CST for the lower airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] xsec_id string XSec ID
*/

extern void PromoteCSTLower( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Demote the CST for the upper airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] xsec_id string XSec ID
*/

extern void DemoteCSTUpper( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Demote the CST for the lower airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] xsec_id string XSec ID
*/

extern void DemoteCSTLower( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Fit a CST airfoil for an existing airfoil of type XS_FOUR_SERIES, XS_SIX_SERIES, XS_FOUR_DIGIT_MOD, XS_FIVE_DIGIT, XS_FIVE_DIGIT_MOD, XS_ONE_SIX_SERIES, or XS_FILE_AIRFOIL.
    \param [in] xsec_surf_id string XsecSurf ID
    \param [in] xsec_index int XSec index
    \param [in] deg int CST degree
*/

extern void FitAfCST( const std::string & xsec_surf_id, int xsec_index, int deg );

//======================== Background3D Functions ======================//

/*!
    \ingroup Background3D
*/
/*!
    Add a Background3D to model
    \forcpponly
    \code{.cpp}
    int nbg = GetNumBackground3Ds();

    // Add Background3D
    string bg_id = AddBackground3D();

    if ( GetNumBackground3Ds() != nbg + 1 )
    {
        Print( "ERROR: AddBackground3D" );
        __failure++;
    }

    DelBackground3D( bg_id );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    nbg = GetNumBackground3Ds()

    # Add Background3D
    bg_id = AddBackground3D()

    if GetNumBackground3Ds() != nbg + 1 :
        print( "ERROR: AddBackground3D" )
        assert False, "ERROR: AddBackground3D"

    DelBackground3D( bg_id )
    \endcode
    \endPythonOnly
    \return string ID for added Background3D
*/

extern string AddBackground3D();

/*!
    \ingroup Background3D
*/
/*!
    Get Number of Background3D's in a model
    \forcpponly
    \code{.cpp}
    int nbg = GetNumBackground3Ds();

    // Add Background3D
    string bg_id = AddBackground3D();

    if ( GetNumBackground3Ds() != nbg + 1 )
    {
        Print( "ERROR: AddBackground3D" );
        __failure++;
    }

    DelBackground3D( bg_id );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    nbg = GetNumBackground3Ds()

    # Add Background3D
    bg_id = AddBackground3D()

    if GetNumBackground3Ds() != nbg + 1 :
        print( "ERROR: AddBackground3D" )
        assert False, "ERROR: AddBackground3D"

    DelBackground3D( bg_id )
    \endcode
    \endPythonOnly
    \return int Number of Background3D's in model
*/

extern int GetNumBackground3Ds();

/*!
    \ingroup Background3D
*/
/*!
    Get id's of all Background3Ds in model
    \forcpponly
    \code{.cpp}
    int nbg = GetNumBackground3Ds();

    // Add Background3D
    AddBackground3D();
    AddBackground3D();
    AddBackground3D();

    if ( GetNumBackground3Ds() != nbg + 3 )
    {
        Print( "ERROR: AddBackground3D" );
        __failure++;
    }

    array< string > @bg_array = GetAllBackground3Ds();

    for( int n = 0; n < int( bg_array.length() ); n++ )
    {
        Print( bg_array[n] );
    }

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    nbg = GetNumBackground3Ds()

    # Add Background3D
    AddBackground3D()
    AddBackground3D()
    AddBackground3D()

    if GetNumBackground3Ds() != nbg + 3 :
        print( "ERROR: AddBackground3D" )
        assert False, "ERROR: AddBackground3D"

    bg_array = GetAllBackground3Ds()

    for n in range( len( bg_array ) ):
        print( bg_array[n] )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \return vector\<string\> Vector of Background3D IDs
*/

extern vector < string > GetAllBackground3Ds();

/*!
    \ingroup Background3D
*/
/*!
    Show all Background3Ds in model
    \forcpponly
    \code{.cpp}
    // Add Background3D
    AddBackground3D();
    AddBackground3D();
    AddBackground3D();

    ShowAllBackground3Ds();

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    AddBackground3D()
    AddBackground3D()
    AddBackground3D()

    ShowAllBackground3Ds()

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
*/

extern void ShowAllBackground3Ds();

/*!
    \ingroup Background3D
*/
/*!
    Hide all Background3Ds in model
    \forcpponly
    \code{.cpp}
    // Add Background3D
    AddBackground3D();
    AddBackground3D();
    AddBackground3D();

    HideAllBackground3Ds();

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    AddBackground3D()
    AddBackground3D()
    AddBackground3D()

    HideAllBackground3Ds()

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
*/

extern void HideAllBackground3Ds();

/*!
    \ingroup Background3D
*/
/*!
    Delete all Background3Ds in model
    \forcpponly
    \code{.cpp}
    // Add Background3D
    AddBackground3D();
    AddBackground3D();
    AddBackground3D();

    DelAllBackground3Ds();

    int nbg = GetNumBackground3Ds();

    if ( nbg != 0 )
    {
        Print( "ERROR: DelAllBackground3Ds" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    AddBackground3D()
    AddBackground3D()
    AddBackground3D()

    DelAllBackground3Ds()

    nbg = GetNumBackground3Ds()

    if nbg != 0 :
        print( "ERROR: DelAllBackground3Ds" )
        assert False, "ERROR: DelAllBackground3Ds"

    \endcode
    \endPythonOnly
*/

extern void DelAllBackground3Ds();

/*!
    \ingroup Background3D
*/
/*!
    Delete specific Background3D frommodel
    \forcpponly
    \code{.cpp}
    // Add Background3D
    AddBackground3D();
    string bg_id = AddBackground3D();
    AddBackground3D();

    int nbg = GetNumBackground3Ds();

    DelBackground3D( bg_id );

    if ( GetNumBackground3Ds() != nbg - 1 )
    {
        Print( "ERROR: DelBackground3D" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    AddBackground3D()
    bg_id = AddBackground3D()
    AddBackground3D()

    nbg = GetNumBackground3Ds()

    DelBackground3D( bg_id )

    if GetNumBackground3Ds() != nbg -1 :
        print( "ERROR: DelBackground3D" )
        assert False, "ERROR: DelBackground3D"

    \endcode
    \endPythonOnly
    \param [in] id string Background3D ID to delete
*/

extern void DelBackground3D( const string &id );

/*!
    \ingroup Background3D
*/
/*!
    Get relative paths to all Background3D images in model.  Note that path is relative to the model's \\*.vsp3 file.
    Consequently, if a file has not yet been saved or assigned a file name, the relative path is meaningless.
    \forcpponly
    \code{.cpp}
    // Add Background3D
    AddBackground3D();
    AddBackground3D();
    AddBackground3D();

    array< string > @bg_file_array = GetAllBackground3DRelativePaths();
    if ( bg_file_array.length() == 0 )
    {
        Print( "ERROR: GetAllBackground3DRelativePaths returned nothing" );
        __failure++;
    }

    for( int n = 0; n < int( bg_file_array.length() ); n++ )
    {
        Print( bg_file_array[n] );
    }

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    AddBackground3D()
    AddBackground3D()
    AddBackground3D()

    bg_file_array = GetAllBackground3DRelativePaths()
    assert len( bg_file_array ) > 0, "GetAllBackground3DRelativePaths returned nothing"

    for n in range( len( bg_file_array ) ):
        print( bg_file_array[n] )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \return vector\<string\> Vector of relative paths to Background3D image files
*/

extern vector < string > GetAllBackground3DRelativePaths();

/*!
    \ingroup Background3D
*/
/*!
    Get absolute paths to all Background3D images in model.
    \forcpponly
    \code{.cpp}
    // Add Background3D
    AddBackground3D();
    AddBackground3D();
    AddBackground3D();

    array< string > @bg_file_array = GetAllBackground3DAbsolutePaths();
    if ( bg_file_array.length() == 0 )
    {
        Print( "ERROR: GetAllBackground3DAbsolutePaths returned nothing" );
        __failure++;
    }

    for( int n = 0; n < int( bg_file_array.length() ); n++ )
    {
        Print( bg_file_array[n] );
    }

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    AddBackground3D()
    AddBackground3D()
    AddBackground3D()

    bg_file_array = GetAllBackground3DAbsolutePaths()
    assert len( bg_file_array ) > 0, "GetAllBackground3DAbsolutePaths returned nothing"

    for n in range( len( bg_file_array ) ):
        print( bg_file_array[n] )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \return vector\<string\> Vector of absolute paths to Background3D image files
*/

extern vector < string > GetAllBackground3DAbsolutePaths();

/*!
    \ingroup Background3D
*/
/*!
    Get relative path to specified Background3D's image.  Note that path is relative to the model's \\*.vsp3 file.
    Consequently, if a file has not yet been saved or assigned a file name, the relative path is meaningless.
    \forcpponly
    \code{.cpp}
    // Add Background3D
    string bg_id = AddBackground3D();

    SetBackground3DRelativePath( bg_id, "front.png" );
    string bg_file = GetBackground3DRelativePath( bg_id );
    if ( bg_file.length() == 0 )
    {
        Print( "ERROR: GetBackground3DRelativePath returned nothing" );
        __failure++;
    }

    Print( bg_file );

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    bg_id = AddBackground3D()

    SetBackground3DRelativePath( bg_id, "front.png" )
    bg_file = GetBackground3DRelativePath( bg_id )
    assert len( bg_file ) > 0, "GetBackground3DRelativePath returned nothing"

    print( bg_file )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \param [in] id string Background3D ID
    \return string Relative path to Background3D image file
*/

extern string GetBackground3DRelativePath( const string &id );

/*!
    \ingroup Background3D
*/
/*!
    Get absolute path to specified Background3D's image.
    \forcpponly
    \code{.cpp}
    // Add Background3D
    string bg_id = AddBackground3D();

    SetBackground3DAbsolutePath( bg_id, "/user/me/vsp_work/front.png" );
    string bg_file = GetBackground3DAbsolutePath( bg_id );
    if ( bg_file.length() == 0 )
    {
        Print( "ERROR: GetBackground3DAbsolutePath returned nothing" );
        __failure++;
    }

    Print( bg_file );

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    bg_id = AddBackground3D()

    SetBackground3DAbsolutePath( bg_id, "/user/me/vsp_work/front.png" )
    bg_file = GetBackground3DAbsolutePath( bg_id )
    assert len( bg_file ) > 0, "GetBackground3DAbsolutePath returned nothing"

    print( bg_file )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \param [in] id string Background3D ID
    \return string Absolute path to Background3D image file
*/

extern string GetBackground3DAbsolutePath( const string &id );

/*!
    \ingroup Background3D
*/
/*!
    Set relative path to specified Background3D's image.  Note that path is relative to the model's \\*.vsp3 file.
    Consequently, if a file has not yet been saved or assigned a file name, the relative path is meaningless.
    \forcpponly
    \code{.cpp}
    // Add Background3D
    string bg_id = AddBackground3D();

    SetBackground3DRelativePath( bg_id, "front.png" );
    if ( GetBackground3DRelativePath( bg_id ) != "front.png" )
    {
        Print( "ERROR: SetBackground3DRelativePath did not take" );
        __failure++;
    }

    string bg_file = GetBackground3DRelativePath( bg_id );

    Print( bg_file );

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    bg_id = AddBackground3D()

    SetBackground3DRelativePath( bg_id, "front.png" )
    assert GetBackground3DRelativePath( bg_id ) == "front.png", "SetBackground3DRelativePath did not take"

    bg_file = GetBackground3DRelativePath( bg_id )

    print( bg_file )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \param [in] id string Background3D ID
    \param [in] fname string Relative path to Background3D image file
*/

extern void SetBackground3DRelativePath( const string &id, const string &fname );

/*!
    \ingroup Background3D
*/
/*!
    Set absolute path to specified Background3D's image.
    \forcpponly
    \code{.cpp}
    // Add Background3D
    string bg_id = AddBackground3D();

    SetBackground3DAbsolutePath( bg_id, "front.png" );
    string bg_file = GetBackground3DAbsolutePath( bg_id );

    Print( bg_file );

    DelAllBackground3Ds();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Background3D
    bg_id = AddBackground3D()

    SetBackground3DAbsolutePath( bg_id, "front.png" )
    bg_file = GetBackground3DAbsolutePath( bg_id )

    print( bg_file )

    DelAllBackground3Ds()
    \endcode
    \endPythonOnly
    \param [in] id string Background3D ID
    \param [in] fname string Absolute path to Background3D image file
*/

extern void SetBackground3DAbsolutePath( const string &id, const string &fname );


//======================== GearGeom Functions ======================//
/*!
    \ingroup GearGeom
*/
/*!
    Create a new Bogie and add it to the end of the Bogie list of the specified GearGeom.  A GearGeom's landing
    gear is described by its list of Bogies.  Each Bogie is a ParmContainer, so once you have the returned Bogie
    ID you can query and set its Parms with the normal Parm API functions.
    \forcpponly
    \code{.cpp}
    string gear_id = AddGeom( "GEAR", "" );             // Add a landing gear Geom

    string bogie_id = CreateAndAddBogie( gear_id );     // Create and add a Bogie

    // Bogies are ParmContainers -- work with their Parms once you have the ID.
    SetParmVal( bogie_id, "NumAcross", "Bogie", 2 );    // Two wheels across
    SetParmVal( bogie_id, "NumTandem", "Bogie", 2 );    // Two wheels in tandem

    if ( bogie_id.length() == 0 || bogie_id == "NONE" )
    {
        Print( "ERROR: CreateAndAddBogie returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    gear_id = AddGeom( "GEAR", "" )             # Add a landing gear Geom

    bogie_id = CreateAndAddBogie( gear_id )     # Create and add a Bogie

    # Bogies are ParmContainers -- work with their Parms once you have the ID.
    SetParmVal( bogie_id, "NumAcross", "Bogie", 2 )    # Two wheels across
    SetParmVal( bogie_id, "NumTandem", "Bogie", 2 )    # Two wheels in tandem
    \endcode
    \endPythonOnly
    \sa GetAllBogies, GetNumBogies, DelBogie, DelAllBogies
    \param [in] gear_id string GearGeom Geom ID
    \return string ParmContainer ID for the newly added Bogie
*/

extern string CreateAndAddBogie( const string &gear_id );

/*!
    \ingroup GearGeom
*/
/*!
    Get the number of Bogies in the specified GearGeom.
    \forcpponly
    \code{.cpp}
    string gear_id = AddGeom( "GEAR", "" );

    CreateAndAddBogie( gear_id );
    CreateAndAddBogie( gear_id );

    int num_bogie = GetNumBogies( gear_id );            // num_bogie == 2

    if ( num_bogie != 2 )
    {
        Print( "ERROR: GetNumBogies, two were added" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    gear_id = AddGeom( "GEAR", "" )

    CreateAndAddBogie( gear_id )
    CreateAndAddBogie( gear_id )

    num_bogie = GetNumBogies( gear_id )            # num_bogie == 2

    assert num_bogie == 2, "GetNumBogies, two were added"
    \endcode
    \endPythonOnly
    \sa CreateAndAddBogie, GetAllBogies
    \param [in] gear_id string GearGeom Geom ID
    \return int Number of Bogies in the specified GearGeom
*/

extern int GetNumBogies( const string &gear_id );

/*!
    \ingroup GearGeom
*/
/*!
    Get the ParmContainer IDs of all the Bogies in the specified GearGeom, in list order.
    \forcpponly
    \code{.cpp}
    string gear_id = AddGeom( "GEAR", "" );

    CreateAndAddBogie( gear_id );
    CreateAndAddBogie( gear_id );

    array<string> @bogie_ids = GetAllBogies( gear_id );
    if ( bogie_ids.length() == 0 )
    {
        Print( "ERROR: GetAllBogies returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    gear_id = AddGeom( "GEAR", "" )

    CreateAndAddBogie( gear_id )
    CreateAndAddBogie( gear_id )

    bogie_ids = GetAllBogies( gear_id )
    assert len( bogie_ids ) > 0, "GetAllBogies returned nothing"
    \endcode
    \endPythonOnly
    \sa CreateAndAddBogie, GetNumBogies, DelBogie
    \param [in] gear_id string GearGeom Geom ID
    \return vector\<string\> Vector of Bogie ParmContainer IDs
*/

extern vector < string > GetAllBogies( const string &gear_id );

/*!
    \ingroup GearGeom
*/
/*!
    Delete the specified Bogie from the specified GearGeom.
    \forcpponly
    \code{.cpp}
    string gear_id = AddGeom( "GEAR", "" );

    string bogie_id = CreateAndAddBogie( gear_id );

    int num_before_del = GetNumBogies( gear_id );
    DelBogie( gear_id, bogie_id );
    if ( GetNumBogies( gear_id ) >= num_before_del )
    {
        Print( "ERROR: DelBogie removed nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    gear_id = AddGeom( "GEAR", "" )

    bogie_id = CreateAndAddBogie( gear_id )

    num_before_del = GetNumBogies( gear_id )
    DelBogie( gear_id, bogie_id )
    assert GetNumBogies( gear_id ) < num_before_del, "DelBogie removed nothing"

    \endcode
    \endPythonOnly
    \sa CreateAndAddBogie, DelAllBogies
    \param [in] gear_id string GearGeom Geom ID
    \param [in] bogie_id string Bogie ParmContainer ID
*/

extern void DelBogie( const string &gear_id, const string &bogie_id );

/*!
    \ingroup GearGeom
*/
/*!
    Delete all Bogies from the specified GearGeom.
    \forcpponly
    \code{.cpp}
    string gear_id = AddGeom( "GEAR", "" );

    CreateAndAddBogie( gear_id );
    CreateAndAddBogie( gear_id );

    DelAllBogies( gear_id );                            // GetNumBogies( gear_id ) == 0
    if ( GetNumBogies( gear_id ) != 0 )
    {
        Print( "ERROR: DelAllBogies left something behind" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    gear_id = AddGeom( "GEAR", "" )

    CreateAndAddBogie( gear_id )
    CreateAndAddBogie( gear_id )

    DelAllBogies( gear_id )                            # GetNumBogies( gear_id ) == 0
    assert GetNumBogies( gear_id ) == 0, "DelAllBogies left something behind"

    \endcode
    \endPythonOnly
    \sa CreateAndAddBogie, DelBogie
    \param [in] gear_id string GearGeom Geom ID
*/

extern void DelAllBogies( const string &gear_id );


//======================== RoutingGeom Functions ======================//
/*!
    \ingroup RoutingGeom
*/
/*!
    Get the number of routing points in a RoutingGeom
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    int npt = GetNumRoutingPts(routing_geom);
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    npt = vsp.GetNumRoutingPts(routing_geom)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt
    \param [in] routing_id string RoutingGeom Geom ID
    \return int Number of routing points in specified RoutingGeom
*/

extern int GetNumRoutingPts( const string &routing_id );

/*!
    \ingroup RoutingGeom
*/
/*!
    Add a routing point to a RoutingGeom.  The new point will be the last point in the route.  The new point will be
    anchored to the specified geom and surface.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    if ( rpt0.length() == 0 || rpt0 == "NONE" )
    {
        Print( "ERROR: AddRoutingPt returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt, InsertRoutingPt
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] geom_id string Geom ID of geom to anchor new routing point to
    \param [in] surf_index int Index of surf to anchor new routing point to
    \return string ParmContainer ID for the newly added routing point
*/

extern string AddRoutingPt( const string &routing_id, const string &geom_id, int surf_index );

/*!
    \ingroup RoutingGeom
*/
/*!
    Add a routing point to a RoutingGeom.  The new point will be inserted before the specified index.  The new point
    will be anchored to the specified geom and surface.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    int npt = GetNumRoutingPts(routing_geom);

    string rptPre2 = InsertRoutingPt(routing_geom, 2, pod2, 0);
    string uPre2 = GetParm( rptPre2, "U", "RoutePt");
    SetParmVal(uPre2, 0.);

    if ( rptPre2.length() == 0 || rptPre2 == "NONE" )
    {
        Print( "ERROR: InsertRoutingPt returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    npt = vsp.GetNumRoutingPts(routing_geom)
    rptPre2 = vsp.InsertRoutingPt(routing_geom, 2, pod2, 0)
    uPre2 = vsp.GetParm( rptPre2, 'U', 'RoutePt')
    vsp.SetParmVal(uPre2, 0.)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] index int Index of routing point to insert new point before
    \param [in] geom_id string Geom ID of geom to anchor new routing point to
    \param [in] surf_index int Index of surf to anchor new routing point to
    \return string ParmContainer ID for the newly added routing point
*/

extern string InsertRoutingPt( const string &routing_id, int index, const string &geom_id, int surf_index );

/*!
    \ingroup RoutingGeom
*/
/*!
    Delete a specified routing point from a RoutingGeom.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    int num_before_del = GetNumRoutingPts( routing_geom );
    DelRoutingPt( routing_geom, 1 );
    if ( GetNumRoutingPts( routing_geom ) >= num_before_del )
    {
        Print( "ERROR: DelRoutingPt removed nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    num_before_del = vsp.GetNumRoutingPts( routing_geom )
    vsp.DelRoutingPt( routing_geom, 1 )
    assert vsp.GetNumRoutingPts( routing_geom ) < num_before_del, "DelRoutingPt removed nothing"

    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelAllRoutingPt
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] index int Index of routing point to delete
*/

extern void DelRoutingPt( const string &routing_id, int index );

/*!
    \ingroup RoutingGeom
*/
/*!
    Delete all routing points from a RoutingGeom.
    \forcpponly
    \code{.cpp}

    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    DelAllRoutingPt( routing_geom );
    if ( GetNumRoutingPts( routing_geom ) != 0 )
    {
        Print( "ERROR: DelAllRoutingPt left something behind" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.DelAllRoutingPt( routing_geom )

    assert vsp.GetNumRoutingPts( routing_geom ) == 0, "DelAllRoutingPt left something behind"
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt
    \param [in] routing_id string RoutingGeom Geom ID
*/

extern void DelAllRoutingPt( const string &routing_id );

/*!
    \ingroup RoutingGeom
*/
/*!
    Move a specified routing point within the route of a RoutingGeom.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    int newindx = MoveRoutingPt( routing_geom, 1, REORDER_MOVE_DOWN );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    newindx = vsp.MoveRoutingPt( routing_geom, 1, vsp.REORDER_MOVE_DOWN )
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt, REORDER_TYPE
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] index int Index of routing point to move
    \param [in] reorder_type int Enum specifying reordering type (i.e. REORDER_MOVE_UP, REORDER_MOVE_DOWN, REORDER_MOVE_TOP, REORDER_MOVE_BOTTOM)
*/

extern int MoveRoutingPt( const string &routing_id, int index, int reorder_type );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get the ParmContainer ID of a routing point within a RoutingGeom.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    string rid = GetRoutingPtID(routing_geom, 2);
    if ( rid.length() == 0 )
    {
        Print( "ERROR: GetRoutingPtID returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    rid = vsp.GetRoutingPtID(routing_geom, 2)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelAllRoutingPt, GetAllRoutingPtIds
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] index int Index of routing point to ID to retreive
    \return string ParmContainer ID for the specified routing point
*/

extern string GetRoutingPtID( const string &routing_id, int index );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get the ParmContainer IDs of all the routing points within a RoutingGeom.  The vector will contain the IDs in order.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    array<string> @rpts = GetAllRoutingPtIds(routing_geom);
    if ( rpts.length() == 0 )
    {
        Print( "ERROR: GetAllRoutingPtIds returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    rpts = vsp.GetAllRoutingPtIds(routing_geom)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelAllRoutingPt, GetRoutingPtId
    \param [in] routing_id string RoutingGeom Geom ID
    \return vector\<string\> Vector of routing point ParmConatiner IDs
*/

extern vector < string > GetAllRoutingPtIds( const string &routing_id );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get the Geom ID of the geom a routing point is anchored to.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    string gid = GetRoutingPtParentID(rpt1);
    if ( gid.length() == 0 )
    {
        Print( "ERROR: GetRoutingPtParentID returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    gid = vsp.GetRoutingPtParentID(rpt1)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelAllRoutingPt, GetAllRoutingPtIds
    \param [in] pt_id string ParmContainer ID of desired routing point
    \return string Geom ID for the geom the routing point is anchored to
*/

extern string GetRoutingPtParentID( const string & pt_id );

/*!
    \ingroup RoutingGeom
*/
/*!
    Set the Geom ID of the geom a routing point is anchored to.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    SetRoutingPtParentID(rpt1, pod1);
    if ( GetRoutingPtParentID( rpt1 ) != pod1 )
    {
        Print( "ERROR: SetRoutingPtParentID did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.SetRoutingPtParentID(rpt1, pod1)
    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelAllRoutingPt, GetAllRoutingPtIds
    \param [in] pt_id string ParmContainer ID of desired routing point
    \param [in] parent_id string Geom ID for the geom to anchor the routing point to
*/

extern void SetRoutingPtParentID( const string & pt_id, const string &parent_id );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get the main coordinate location a routing point.  The main location is the location of the base copy before
    symmetry has been applied.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    Update();
    vec3d p1 = GetMainRoutingPtCoord(rpt1);

    // rpt1 rides on pod2, which was moved two units out in Y.
    if ( !closeTo( p1.y(), 2.0, 1e-6 ) )
    {
        Print( "ERROR: GetMainRoutingPtCoord did not follow the parent Geom" );
        __failure++;
    }

    // With no symmetry applied, the main copy is also symm_index 0.
    if ( dist( p1, GetRoutingPtCoord( routing_geom, 1, 0 ) ) > 1e-6 )
    {
        Print( "ERROR: GetMainRoutingPtCoord disagrees with GetRoutingPtCoord" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.Update()
    p1 = vsp.GetMainRoutingPtCoord(rpt1)

    # rpt1 rides on pod2, which was moved two units out in Y.
    assert abs( p1.y() - 2.0 ) < 1e-6, "GetMainRoutingPtCoord did not follow the parent Geom"

    # With no symmetry applied, the main copy is also symm_index 0.
    assert vsp.dist( p1, vsp.GetRoutingPtCoord( routing_geom, 1, 0 ) ) < 1e-6, "GetMainRoutingPtCoord disagrees with GetRoutingPtCoord"

    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt, GetRoutingPtCoord, GetAllRoutingPtCoords, GetRoutingCurve
    \param [in] pt_id string ParmContainer ID of desired routing point
    \return vec3d coordinate of main routing point
*/

extern vec3d GetMainRoutingPtCoord( const string &pt_id );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get the coordinate location a routing point in a RoutingGeom.  The main location is symm_index = 0.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    Update();
    vec3d p1 = GetRoutingPtCoord(routing_geom, 1, 0);

    // rpt1 rides on pod2, which was moved two units out in Y.
    if ( !closeTo( p1.y(), 2.0, 1e-6 ) )
    {
        Print( "ERROR: GetRoutingPtCoord did not follow the parent Geom" );
        __failure++;
    }

    // The indexed point has to be the second of the three that were added.
    array<vec3d> pvec = GetAllRoutingPtCoords(routing_geom, 0);

    if ( pvec.length() != 3 || dist( p1, pvec[1] ) > 1e-6 )
    {
        Print( "ERROR: GetRoutingPtCoord disagrees with GetAllRoutingPtCoords" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.Update()
    p1 = vsp.GetRoutingPtCoord(routing_geom, 1, 0)

    # rpt1 rides on pod2, which was moved two units out in Y.
    assert abs( p1.y() - 2.0 ) < 1e-6, "GetRoutingPtCoord did not follow the parent Geom"

    # The indexed point has to be the second of the three that were added.
    pvec = vsp.GetAllRoutingPtCoords(routing_geom, 0)

    assert len( pvec ) == 3, "GetAllRoutingPtCoords returned the wrong number of points"
    assert vsp.dist( p1, pvec[1] ) < 1e-6, "GetRoutingPtCoord disagrees with GetAllRoutingPtCoords"

    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt, GetMainRoutingPtCoord, GetAllRoutingPtCoords, GetRoutingCurve
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] index int Index of routing point to get cordinate of
    \param [in] symm_index int Symmetry index to get coordinate of
    \return vec3d coordinate of routing point
*/

extern vec3d GetRoutingPtCoord( const string &routing_id, int index, int symm_index );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get the coordinate locations along a RoutingGeom.  The main copy is symm_index = 0.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    Update();
    array<vec3d> pvec = GetAllRoutingPtCoords(routing_geom, 0);

    // One coordinate per routing point, in the order they were added.
    if ( pvec.length() != 3 )
    {
        Print( "ERROR: GetAllRoutingPtCoords returned the wrong number of points" );
        __failure++;
    }
    else
    {
        // rpt0 sits at the nose of pod1 and rpt2 at its tail, both on the
        // centerline.  rpt1 rides on pod2, two units out in Y.
        if ( !closeTo( pvec[0].y(), 0.0, 1e-6 ) ||
             !closeTo( pvec[1].y(), 2.0, 1e-6 ) ||
             !closeTo( pvec[2].y(), 0.0, 1e-6 ) )
        {
            Print( "ERROR: GetAllRoutingPtCoords did not follow the parent Geoms" );
            __failure++;
        }

        if ( pvec[2].x() <= pvec[0].x() )
        {
            Print( "ERROR: GetAllRoutingPtCoords did not order the points by U" );
            __failure++;
        }

        if ( dist( pvec[1], GetMainRoutingPtCoord( rpt1 ) ) > 1e-6 )
        {
            Print( "ERROR: GetAllRoutingPtCoords disagrees with GetMainRoutingPtCoord" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.Update()
    pvec = vsp.GetAllRoutingPtCoords(routing_geom, 0)

    # One coordinate per routing point, in the order they were added.
    assert len( pvec ) == 3, "GetAllRoutingPtCoords returned the wrong number of points"

    # rpt0 sits at the nose of pod1 and rpt2 at its tail, both on the
    # centerline.  rpt1 rides on pod2, two units out in Y.
    assert abs( pvec[0].y() ) < 1e-6, "GetAllRoutingPtCoords did not follow the parent Geoms"
    assert abs( pvec[1].y() - 2.0 ) < 1e-6, "GetAllRoutingPtCoords did not follow the parent Geoms"
    assert abs( pvec[2].y() ) < 1e-6, "GetAllRoutingPtCoords did not follow the parent Geoms"

    assert pvec[2].x() > pvec[0].x(), "GetAllRoutingPtCoords did not order the points by U"

    assert vsp.dist( pvec[1], vsp.GetMainRoutingPtCoord( rpt1 ) ) < 1e-6, "GetAllRoutingPtCoords disagrees with GetMainRoutingPtCoord"

    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt, GetMainRoutingPtCoord, GetRoutingPtCoord, GetRoutingCurve
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] symm_index int Symmetry index to get coordinate of
    \return vector \< vec3d \> coordinate of routing points along RoutingGeom
*/

extern vector < vec3d > GetAllRoutingPtCoords( const string &routing_id, int symm_index );

/*!
    \ingroup RoutingGeom
*/
/*!
    Get points along a RoutingGeom.  These points will follow the curve of a RoutingGeom with radiused
    routing points.  The main copy is symm_index = 0.
    \forcpponly
    \code{.cpp}
    string pod1 = AddGeom("POD", "");

    string pod2 = AddGeom("POD", "");
    string ypod2 = GetParm(pod2, "Y_Rel_Location", "XForm");
    SetParmVal(ypod2, 2.0);

    string routing_geom = AddGeom("ROUTING", "");

    string rpt0 = AddRoutingPt(routing_geom, pod1, 0);
    string u0 = GetParm( rpt0, "U", "RoutePt");
    SetParmVal(u0, 0.0);

    string rpt1 = AddRoutingPt(routing_geom, pod2, 0);

    string rpt2 = AddRoutingPt(routing_geom, pod1, 0);
    string u2 = GetParm( rpt2, "U", "RoutePt");
    SetParmVal(u2, 1.0);

    Update();
    array<vec3d> pvec = GetRoutingCurve(routing_geom, 0);

    // The curve is tessellated, so it carries at least as many points as there
    // are routing points, and it has to start and end on them.
    array<vec3d> rpts = GetAllRoutingPtCoords(routing_geom, 0);

    if ( pvec.length() < rpts.length() )
    {
        Print( "ERROR: GetRoutingCurve returned too few points" );
        __failure++;
    }
    else
    {
        if ( dist( pvec[0], rpts[0] ) > 1e-6 ||
             dist( pvec[pvec.length() - 1], rpts[rpts.length() - 1] ) > 1e-6 )
        {
            Print( "ERROR: GetRoutingCurve does not end on the routing points" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod1 = vsp.AddGeom('POD', '')

    pod2 = vsp.AddGeom('POD', '')
    ypod2 = vsp.GetParm(pod2, 'Y_Rel_Location', 'XForm')
    vsp.SetParmVal(ypod2, 2.0)

    routing_geom = vsp.AddGeom('ROUTING', '')

    rpt0 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u0 = vsp.GetParm( rpt0, 'U', 'RoutePt')
    vsp.SetParmVal(u0, 0.0)

    rpt1 = vsp.AddRoutingPt(routing_geom, pod2, 0)

    rpt2 = vsp.AddRoutingPt(routing_geom, pod1, 0)
    u2 = vsp.GetParm( rpt2, 'U', 'RoutePt')
    vsp.SetParmVal(u2, 1.0)

    vsp.Update()
    pvec = vsp.GetRoutingCurve(routing_geom, 0)

    # The curve is tessellated, so it carries at least as many points as there
    # are routing points, and it has to start and end on them.
    rpts = vsp.GetAllRoutingPtCoords(routing_geom, 0)

    assert len( pvec ) >= len( rpts ), "GetRoutingCurve returned too few points"
    assert vsp.dist( pvec[0], rpts[0] ) < 1e-6, "GetRoutingCurve does not start on the first routing point"
    assert vsp.dist( pvec[-1], rpts[-1] ) < 1e-6, "GetRoutingCurve does not end on the last routing point"

    \endcode
    \endPythonOnly
    \sa AddRoutingPt, DelRoutingPt, GetMainRoutingPtCoord, GetRoutingPtCoord, GetAllRoutingPtCoords
    \param [in] routing_id string RoutingGeom Geom ID
    \param [in] symm_index int Symmetry index to get coordinate of
    \return vector \< vec3d \> coordinate of points along RoutingGeom curve
*/

extern vector < vec3d > GetRoutingCurve( const string &routing_id, int symm_index );

//======================== BOR Functions ======================//
/*!
    \ingroup BOR
*/
/*!
    Set the XSec type for a BOR component
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE );

    if ( GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE ) { Print( "ERROR: ChangeBORXSecShape" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE )

    if  GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE :
        print( "ERROR: ChangeBORXSecShape" )
        assert False, "ERROR: ChangeBORXSecShape"

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] type int XSec type enum (i.e. XS_ROUNDED_RECTANGLE)
*/

extern void ChangeBORXSecShape( const string & bor_id, int type );

/*!
    \ingroup BOR
*/
/*!
    Get the XSec type for a BOR component
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE );

    if ( GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE ) { Print( "ERROR: GetBORXSecShape" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE )

    if  GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE :
        print( "ERROR: GetBORXSecShape" )
        assert False, "ERROR: GetBORXSecShape"

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \return int XSec type enum (i.e. XS_ROUNDED_RECTANGLE)
*/

extern int GetBORXSecShape( const string & bor_id );

/*!
    \ingroup BOR
*/
/*!
    Set the coordinate points for a specific BOR. The BOR XSecCurve must be of type XS_FILE_FUSE.
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE );

    array< vec3d > @vec_array = ReadBORFileXSec( bor_id, "TestXSec.fxs" );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE )

    vec_array = ReadBORFileXSec( bor_id, "TestXSec.fxs" )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] file_name string Fuselage XSec file name
    \return vector\<vec3d\> Array of coordinate points read from the file and set to the XSec
*/

extern std::vector<vec3d> ReadBORFileXSec( const std::string& bor_id, const std::string& file_name );

/*!
    \ingroup BOR
*/
/*!
    Set the coordinate points for a specific BOR. The BOR XSecCurve must be of type XS_FILE_FUSE.
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE );

    array< vec3d > @vec_array = ReadBORFileXSec( bor_id, "TestXSec.fxs" );

    Update();

    vec3d before = ComputeBORXSecPnt( bor_id, 0.0 );

    if ( vec_array.size() == 0 )
    {
        Print( "ERROR: ReadBORFileXSec returned no points" );
        __failure++;
    }
    else
    {
        vec_array[1] = vec_array[1] * 2.0;
        vec_array[3] = vec_array[3] * 2.0;

        SetBORXSecPnts( bor_id, vec_array );

        Update();

        vec3d after = ComputeBORXSecPnt( bor_id, 0.0 );

        // Points 1 and 3 are the top and bottom of the diamond read from file,
        // so doubling them doubles the height of the section while leaving its
        // width alone.  The section is normalized to the body diameter by its
        // height, so the section grows half as wide as it was.
        if ( !closeTo( after.x(), 0.5 * before.x(), 1e-6 ) )
        {
            Print( "ERROR: SetBORXSecPnts did not reshape the section" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE )

    # ReadBORFileXSec hands back a tuple, so copy it into a list to edit it.
    vec_array = list( ReadBORFileXSec( bor_id, "TestXSec.fxs" ) )

    Update()

    before = ComputeBORXSecPnt( bor_id, 0.0 )

    assert len( vec_array ) > 0, "ReadBORFileXSec returned no points"

    # The Python vec3d carries no arithmetic operators, so scale by component.
    vec_array[1] = vec3d( vec_array[1].x() * 2.0, vec_array[1].y() * 2.0, vec_array[1].z() * 2.0 )
    vec_array[3] = vec3d( vec_array[3].x() * 2.0, vec_array[3].y() * 2.0, vec_array[3].z() * 2.0 )

    SetBORXSecPnts( bor_id, vec_array )

    Update()

    after = ComputeBORXSecPnt( bor_id, 0.0 )

    # Points 1 and 3 are the top and bottom of the diamond read from file, so
    # doubling them doubles the height of the section while leaving its width
    # alone.  The section is normalized to the body diameter by its height, so
    # the section grows half as wide as it was.
    assert abs( after.x() - 0.5 * before.x() ) < 1e-6, "SetBORXSecPnts did not reshape the section"

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] pnt_vec vector<vec3d> Vector of XSec coordinate points
*/

extern void SetBORXSecPnts( const std::string& bor_id, std::vector< vec3d > & pnt_vec );

/*!
    \ingroup BOR
*/
/*!
    Compute 3D coordinate for a point on a BOR XSecCurve given the parameter value (U) along the curve
    \forcpponly
    \code{.cpp}
    //==== Add Geom ====//
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    double u_fract = 0.25;

    vec3d pnt = ComputeBORXSecPnt( bor_id, u_fract );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Geom ====//
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    u_fract = 0.25

    pnt = ComputeBORXSecPnt( bor_id, u_fract )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] fract double Curve parameter value (range: 0 - 1)
    \return vec3d Coordinate point on curve
*/

extern vec3d ComputeBORXSecPnt( const std::string& bor_id, double fract );

/*!
    \ingroup BOR
*/
/*!
    Compute the tangent vector of a point on a BOR XSecCurve given the parameter value (U) along the curve
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    double u_fract = 0.25;

    vec3d tan = ComputeBORXSecTan( bor_id, u_fract );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    u_fract = 0.25

    tan = ComputeBORXSecTan( bor_id, u_fract )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] fract double Curve parameter value (range: 0 - 1)
    \return vec3d Tangent vector on curve
*/

extern vec3d ComputeBORXSecTan( const std::string& bor_id, double fract );

/*!
    \ingroup BOR
*/
/*!
    Read in shape from airfoil file and set to the specified BOR XSecCurve. The XSecCurve must be of type XS_FILE_AIRFOIL. Airfoil files may be in Lednicer or Selig format with \\*.af or \\*.dat extensions.
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] file_name string Airfoil XSec file name
*/

extern void ReadBORFileAirfoil( const std::string& bor_id, const std::string& file_name );

/*!
    \ingroup BOR
*/
/*!
    Set the upper points for an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL.
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetBORAirfoilUpperPnts( bor_id );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );
    }

    SetBORAirfoilUpperPnts( bor_id, up_array );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    up_array = GetBORAirfoilUpperPnts( bor_id )

    for i in range(int( len(up_array) )):

        up_array[i].scale_y( 2.0 )

    SetBORAirfoilUpperPnts( bor_id, up_array )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] up_pnt_vec vector<vec3d> Vector of points defining the upper surface of the airfoil
*/

extern void SetBORAirfoilUpperPnts( const std::string& bor_id, const std::vector< vec3d > & up_pnt_vec );

/*!
    \ingroup BOR
*/
/*!
    Set the lower points for an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL.
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetBORAirfoilLowerPnts( bor_id );

    for ( int i = 0 ; i < int( low_array.size() ) ; i++ )
    {
        low_array[i].scale_y( 0.5 );
    }

    SetBORAirfoilLowerPnts( bor_id, low_array );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    low_array = GetBORAirfoilLowerPnts( bor_id )

    for i in range(int( len(low_array) )):

        low_array[i].scale_y( 0.5 )

    SetBORAirfoilLowerPnts( bor_id, low_array )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] low_pnt_vec vector<vec3d> Vector of points defining the lower surface of the airfoil
*/

extern void SetBORAirfoilLowerPnts( const std::string& bor_id, const std::vector< vec3d > & low_pnt_vec );

/*!
    \ingroup BOR
*/
/*!
    Set the upper and lower points for an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL.
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetBORAirfoilUpperPnts( bor_id );

    array< vec3d > @low_array = GetBORAirfoilLowerPnts( bor_id );

    for ( int i = 0 ; i < int( up_array.size() ) ; i++ )
    {
        up_array[i].scale_y( 2.0 );

        low_array[i].scale_y( 0.5 );
    }

    SetBORAirfoilPnts( bor_id, up_array, low_array );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    up_array = GetBORAirfoilUpperPnts( bor_id )

    low_array = GetBORAirfoilLowerPnts( bor_id )

    for i in range(int( len(up_array) )):

        up_array[i].scale_y( 2.0 )

        low_array[i].scale_y( 0.5 )

    SetBORAirfoilPnts( bor_id, up_array, low_array )

    \endcode
    \endPythonOnly
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] up_pnt_vec vector<vec3d> Vector of points defining the upper surface of the airfoil
    \param [in] low_pnt_vec vector<_>vec3d> Vector of points defining the lower surface of the airfoil
*/

extern void SetBORAirfoilPnts( const std::string& bor_id, const std::vector< vec3d > & up_pnt_vec, const std::vector< vec3d > & low_pnt_vec );

/*!
    \ingroup BOR
*/
/*!
    Get the coordinate points for the upper surface of an airfoil on a BOR. The BOR XSecCurve must be of type XS_FILE_AIRFOIL
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @up_array = GetBORAirfoilUpperPnts( bor_id );
    if ( up_array.length() == 0 )
    {
        Print( "ERROR: GetBORAirfoilUpperPnts returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    up_array = GetBORAirfoilUpperPnts( bor_id )
    assert len( up_array ) > 0, "GetBORAirfoilUpperPnts returned nothing"

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] bor_id string Body of revolution Geom ID
    \return vector\<vec3d\> Vector of coordinate points for the upper airfoil surface
*/

extern std::vector<vec3d> GetBORAirfoilUpperPnts( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the coordinate points for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_FILE_AIRFOIL
    \forcpponly
    \code{.cpp}
    // Add Body of Recolution
    string bor_id = AddGeom( "BODYOFREVOLUTION", "" );

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL );

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" );

    array< vec3d > @low_array = GetBORAirfoilLowerPnts( bor_id );
    if ( low_array.length() == 0 )
    {
        Print( "ERROR: GetBORAirfoilLowerPnts returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    low_array = GetBORAirfoilLowerPnts( bor_id )
    assert len( low_array ) > 0, "GetBORAirfoilLowerPnts returned nothing"

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] bor_id string Body of revolution Geom ID
    \return vector\<vec3d\> Vector of coordinate points for the lower airfoil surface
*/

extern std::vector<vec3d> GetBORAirfoilLowerPnts( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the CST coefficients for the upper surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] bor_id string Body of revolution Geom ID
    \return vector\<double\> Vector of CST coefficients for the upper airfoil surface
*/

extern std::vector<double> GetBORUpperCSTCoefs( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the CST coefficients for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] bor_id string Body of revolution Geom ID
    \return vector\<double\> Vector of CST coefficients for the lower airfoil surface
*/

extern std::vector<double> GetBORLowerCSTCoefs( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the CST degree for the upper surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] bor_id string Body of revolution Geom ID
    \return int CST Degree for upper airfoil surface
*/

extern int GetBORUpperCSTDegree( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the CST degree for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] bor_id string Body of revolution Geom ID
    \return int CST Degree for lower airfoil surface
*/

extern int GetBORLowerCSTDegree( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Set the CST degree and coefficients for the upper surface of an airfoil of a BOR. The number of coefficients should be one more than the CST degree. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree, GetUpperCSTCoefs
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] deg int CST degree of upper airfoil surface
    \param [in] coefs vector<double> Array of CST coefficients for the upper airfoil surface
*/

extern void SetBORUpperCST( const std::string& bor_id, int deg, const std::vector<double> &coefs );

/*!
    \ingroup BOR
*/
/*!
    Set the CST degree and coefficients for the lower surface of an airfoil of a BOR. The number of coefficients should be one more than the CST degree. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree, GetLowerCSTCoefs
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] deg int CST degree of lower airfoil surface
    \param [in] coefs vector<double> Vector of CST coefficients for the lower airfoil surface
*/

extern void SetBORLowerCST( const std::string& bor_id, int deg, const std::vector<double> &coefs );

/*!
    \ingroup BOR
*/
/*!
    Promote the CST for the upper airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] bor_id string Body of revolution Geom ID
*/

extern void PromoteBORCSTUpper( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Promote the CST for the lower airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] bor_id string Body of revolution Geom ID
*/

extern void PromoteBORCSTLower( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Demote the CST for the upper airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] bor_id string Body of revolution Geom ID
*/

extern void DemoteBORCSTUpper( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Demote the CST for the lower airfoil surface of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] bor_id string Body of revolution Geom ID
*/

extern void DemoteBORCSTLower( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Fit a CST airfoil for an existing airfoil of a BOR of type XS_FOUR_SERIES, XS_SIX_SERIES, XS_FOUR_DIGIT_MOD, XS_FIVE_DIGIT, XS_FIVE_DIGIT_MOD, XS_ONE_SIX_SERIES, or XS_FILE_AIRFOIL.
    \param [in] bor_id string Body of revolution Geom ID
    \param [in] deg int CST degree
*/

extern void FitBORAfCST( const std::string & bor_id, int deg );


//======================== FoilSurf Functions ======================//
/*!
    \ingroup XSec
*/
/*!
    Write out the untwisted unit-length 2D Bezier curve for the specified airfoil in custom\\*.bz format. The output will describe the analytical shape of the airfoil. See BezierAirfoilExample.m and BezierCtrlToCoordPnts.m for examples of
    discretizing the Bezier curve and generating a Selig airfoil file.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    const double u = 0.5; // export airfoil at mid span location

    //==== Write Bezier Airfoil File ====//
    WriteBezierAirfoil( "Example_Bezier.bz", wing_id, u );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "Example_Bezier.bz", "r" ) < 0 )
    {
        Print( "ERROR: WriteBezierAirfoil wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteBezierAirfoil wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry and Set Parms ====//
    wing_id = AddGeom( "WING", "" )

    u = 0.5 # export airfoil at mid span location

    #==== Write Bezier Airfoil File ====//
    WriteBezierAirfoil( "Example_Bezier.bz", wing_id, u )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "Example_Bezier.bz" ) > 0, "WriteBezierAirfoil wrote no file"


    \endcode
    \endPythonOnly
    \param [in] file_name string Airfoil (\\*.bz) output file name
    \param [in] geom_id string Geom ID
    \param [in] foilsurf_u double U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/

extern void WriteBezierAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u );

/*!
    \ingroup XSec
*/
/*!
    Write out the untwisted unit-length 2D coordinate points for the specified airfoil in Selig format. Coordinate points follow the on-screen wire frame W tessellation.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    const double u = 0.5; // export airfoil at mid span location

    //==== Write Selig Airfoil File ====//
    WriteSeligAirfoil( "Example_Selig.dat", wing_id, u );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "Example_Selig.dat", "r" ) < 0 )
    {
        Print( "ERROR: WriteSeligAirfoil wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteSeligAirfoil wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry and Set Parms ====//
    wing_id = AddGeom( "WING", "" )

    u = 0.5 # export airfoil at mid span location

    #==== Write Selig Airfoil File ====//
    WriteSeligAirfoil( "Example_Selig.dat", wing_id, u )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "Example_Selig.dat" ) > 0, "WriteSeligAirfoil wrote no file"


    \endcode
    \endPythonOnly
    \sa GetAirfoilCoordinates
    \param [in] file_name string Airfoil (\\*.dat) output file name
    \param [in] geom_id string Geom ID
    \param [in] foilsurf_u double U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/

extern void WriteSeligAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u );

/*!
    \ingroup XSec
*/
/*!
    Get the untwisted unit-length 2D coordinate points for the specified airfoil
    \sa WriteSeligAirfoil
    \param [in] geom_id string Geom ID
    \param [in] foilsurf_u double U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/

extern std::vector < vec3d > GetAirfoilCoordinates( const std::string & geom_id, const double &foilsurf_u );


//======================== Edit Curve XSec Functions ======================//
/*!
    \ingroup EditCurveXSec
*/
/*!
    Initialize the EditCurveXSec to the current value of m_ShapeType (i.e. EDIT_XSEC_ELLIPSE)
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR );

    EditXSecInitShape( xsec_2 ); // Change back to default ellipse
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    # Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR )

    EditXSecInitShape( xsec_2 ) # Change back to default ellipse

    \endcode
    \endPythonOnly
    \sa INIT_EDIT_XSEC_TYPE
    \param [in] xsec_id string XSec ID
*/

extern void EditXSecInitShape( const std::string & xsec_id );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Convert the EditCurveXSec curve type to the specified new type. Note, EditCurveXSec uses the same enumerations for PCurve to identify curve type,
    but APPROX_CEDIT is not supported at this time.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Set XSec 1 to Linear
    EditXSecConvertTo( xsec_1, LINEAR );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Set XSec 1 to Linear
    EditXSecConvertTo( xsec_1, LINEAR )

    \endcode
    \endPythonOnly
    \sa PCURV_TYPE
    \param [in] xsec_id string XSec ID
    \param [in] newtype int New curve type enum (i.e. CEDIT)
*/

extern void EditXSecConvertTo( const std::string & xsec_id, const int & newtype );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Get the U parameter vector for an EditCurveXSec. The vector will be in increasing order with a range of 0 - 1.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR );

    array < double > u_vec = GetEditXSecUVec( xsec_2 );

    if ( u_vec[1] - 0.25 > 1e-6 )
    {
        Print( "Error: GetEditXSecUVec" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    # Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR )

    u_vec = GetEditXSecUVec( xsec_2 )

    if  u_vec[1] - 0.25 > 1e-6 :
        print( "Error: GetEditXSecUVec" )
        assert False, "Error: GetEditXSecUVec"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \return vector \<double\> Array of U parameter values
*/

extern std::vector < double > GetEditXSecUVec( const std::string& xsec_id );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Get the control point vector for an EditCurveXSec. Note, the returned array of vec3d values will be represented in 2D with Z set to 0.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Get the control points for the default shape
    array < vec3d > xsec1_pts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height
    if ( xsec1_pts.length() == 0 )
    {
        Print( "ERROR: GetEditXSecCtrlVec returned nothing" );
        __failure++;
    }

    Print( "Normalized Bottom Point of XSecCurve: " + xsec1_pts[3].x() + ", " + xsec1_pts[3].y() + ", " + xsec1_pts[3].z() );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Get the control points for the default shape
    xsec1_pts = GetEditXSecCtrlVec( xsec_1, True ) # The returned control points will not be scaled by width and height
    assert len( xsec1_pts ) > 0, "GetEditXSecCtrlVec returned nothing"

    print( f"Normalized Bottom Point of XSecCurve: {xsec1_pts[3].x()}, {xsec1_pts[3].y()}, {xsec1_pts[3].z()}" )

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] non_dimensional bool True to get the points non-dimensionalized, False to get them scaled by m_Width and m_Height
    \return vector \<vec3d\> Array of control points
*/

extern std::vector < vec3d > GetEditXSecCtrlVec( const std::string & xsec_id, bool non_dimensional = true );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Set the U parameter vector and the control point vector for an EditCurveXSec. The arrays must be of equal length, with the values for U defined in
    increasing order and range 0 - 1. The input control points to SetEditXSecPnts must be nondimensionalized in the approximate range of [-0.5, 0.5].
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE );

    // Define a square
    array < vec3d > xsec2_pts(5);

    xsec2_pts[0] = vec3d( 0.5, 0.5, 0.0 );
    xsec2_pts[1] = vec3d( 0.5, -0.5, 0.0 );
    xsec2_pts[2] = vec3d( -0.5, -0.5, 0.0 );
    xsec2_pts[3] = vec3d( -0.5, 0.5, 0.0 );
    xsec2_pts[4] = vec3d( 0.5, 0.5, 0.0 );

    // u vec must start at 0.0 and end at 1.0
    array < double > u_vec(5);

    u_vec[0] = 0.0;
    u_vec[1] = 0.25;
    u_vec[2] = 0.5;
    u_vec[3] = 0.75;
    u_vec[4] = 1.0;

    array < double > r_vec(5);

    r_vec[0] = 0.0;
    r_vec[1] = 0.0;
    r_vec[2] = 0.0;
    r_vec[3] = 0.0;
    r_vec[4] = 0.0;

    SetEditXSecPnts( xsec_2, u_vec, xsec2_pts, r_vec ); // Note: points are unscaled by the width and height parms

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height

    if ( dist( new_pnts[3], xsec2_pts[3] ) > 1e-6 )
    {
        Print( "Error: SetEditXSecPnts");
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    # Set XSec 2 to linear
    EditXSecConvertTo( xsec_2, LINEAR )

    # Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE )

    # Define a square
    xsec2_pts = [vec3d(0.5, 0.5, 0.0),
             vec3d(0.5, -0.5, 0.0),
             vec3d(-0.5, -0.5, 0.0),
             vec3d(-0.5, 0.5, 0.0),
             vec3d(0.5, 0.5, 0.0)]

    # u vec must start at 0.0 and end at 1.0
    u_vec = [0.0, 0.25, 0.5, 0.75, 1.0]

    r_vec = [0.0, 0.0, 0.0, 0.0, 0.0]

    SetEditXSecPnts( xsec_2, u_vec, xsec2_pts, r_vec ) # Note: points are unscaled by the width and height parms

    new_pnts = GetEditXSecCtrlVec( xsec_2, True ) # The returned control points will not be scaled by width and height

    if  dist( new_pnts[3], xsec2_pts[3] ) > 1e-6 :
        print( "Error: SetEditXSecPnts")
        assert False, "Error: SetEditXSecPnts"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] u_vec vector \<double\> Array of U parameter values
    \param [in] r_vec vector \<double\> Array of R parameter values
    \param [in] control_pts vector \<vec3d\> Nondimensionalized array of control points
*/

extern void SetEditXSecPnts( const std::string & xsec_id, const std::vector < double > &u_vec, const std::vector < vec3d > &control_pts, const std::vector < double > &r_vec );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Delete an EditCurveXSec control point. Note, cubic Bezier intermediate control points (those not on the curve) cannot be deleted.
    The previous and next Bezier control point will be deleted along with the point on the curve. Regardless of curve type, the first
    and last points may not be deleted.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE );

    array < vec3d > old_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height

    EditXSecDelPnt( xsec_2, 3 ); // Remove control point at bottom of circle

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height

    if ( old_pnts.size() - new_pnts.size() != 3  )
    {
        Print( "Error: EditXSecDelPnt");
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    # Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE )

    old_pnts = GetEditXSecCtrlVec( xsec_2, True ) # The returned control points will not be scaled by width and height

    EditXSecDelPnt( xsec_2, 3 ) # Remove control point at bottom of circle

    new_pnts = GetEditXSecCtrlVec( xsec_2, True ) # The returned control points will not be scaled by width and height

    if  len(old_pnts) - len(new_pnts) != 3  :
        print( "Error: EditXSecDelPnt")
        assert False, "Error: EditXSecDelPnt"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] indx int Control point index
*/

extern void EditXSecDelPnt( const std::string & xsec_id, const int & indx );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Split the EditCurveXSec at the specified U value
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE );

    // Identify XSec 2
    string xsec_2 = GetXSec( xsec_surf, 2 );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE );

    array < vec3d > old_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height

    int new_pnt_ind = EditXSecSplit01( xsec_2, 0.375 );

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_2, true ); // The returned control points will not be scaled by width and height

    if ( new_pnts.size() - old_pnts.size() != 3  )
    {
        Print( "Error: EditXSecSplit01");
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 2, XS_EDIT_CURVE )

    # Identify XSec 2
    xsec_2 = GetXSec( xsec_surf, 2 )

    # Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_2, "SymType"), SYM_NONE )

    old_pnts = GetEditXSecCtrlVec( xsec_2, True ) # The returned control points will not be scaled by width and height

    new_pnt_ind = EditXSecSplit01( xsec_2, 0.375 )

    new_pnts = GetEditXSecCtrlVec( xsec_2, True ) # The returned control points will not be scaled by width and height

    if  len(new_pnts) - len(old_pnts) != 3  :
        print( "Error: EditXSecSplit01")
        assert False, "Error: EditXSecSplit01"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] u double U value to split the curve at (0 - 1)
    \return int Index of the point added from the split
*/

extern int EditXSecSplit01( const std::string & xsec_id, const double & u );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Move an EditCurveXSec control point. The XSec points are nondimensionalized by m_Width and m_Height and
    defined in 2D, so the Z value of the new coordinate point will be ignored.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_1, "SymType"), SYM_NONE );

    // Get the control points for the default shape
    array < vec3d > xsec1_pts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height

    // Identify a control point that lies on the curve and shift it in Y
    int move_pnt_ind = 3;

    vec3d new_pnt = vec3d( xsec1_pts[move_pnt_ind].x(), 2 * xsec1_pts[move_pnt_ind].y(), 0.0 );

    // Move the control point
    MoveEditXSecPnt( xsec_1, move_pnt_ind, new_pnt );

    array < vec3d > new_pnts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height

    if ( dist( new_pnt, new_pnts[move_pnt_ind] ) > 1e-6 )
    {
        Print( "Error: MoveEditXSecPnt" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Turn off R/L symmetry
    SetParmVal( GetXSecParm( xsec_1, "SymType"), SYM_NONE )

    # Get the control points for the default shape
    xsec1_pts = GetEditXSecCtrlVec( xsec_1, True ) # The returned control points will not be scaled by width and height

    # Identify a control point that lies on the curve and shift it in Y
    move_pnt_ind = 3

    new_pnt = vec3d( xsec1_pts[move_pnt_ind].x(), 2 * xsec1_pts[move_pnt_ind].y(), 0.0 )

    # Move the control point
    MoveEditXSecPnt( xsec_1, move_pnt_ind, new_pnt )

    new_pnts = GetEditXSecCtrlVec( xsec_1, True ) # The returned control points will not be scaled by width and height

    if  dist( new_pnt, new_pnts[move_pnt_ind] ) > 1e-6 :
        print( "Error: MoveEditXSecPnt" )
        assert False, "Error: MoveEditXSecPnt"

    \endcode
    \endPythonOnly
    \param [in] xsec_id string XSec ID
    \param [in] indx int Control point index
    \param [in] new_pnt vec3d Coordinate of the new point
*/

extern void MoveEditXSecPnt( const std::string & xsec_id, const int & indx, const vec3d & new_pnt );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Convert any XSec type into an EditCurveXSec. This function will work for BOR Geoms, in which case the input XSec index is ignored.
    \forcpponly
    \code{.cpp}
    // Add Stack
    string sid = AddGeom( "STACK", "" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( sid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE );

    // Convert Rounded Rectangle to Edit Curve type XSec
    ConvertXSecToEdit( sid, 1 );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    // Get the control points for the default shape
    array < vec3d > xsec1_pts = GetEditXSecCtrlVec( xsec_1, true ); // The returned control points will not be scaled by width and height
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_ROUNDED_RECTANGLE )

    # Convert Rounded Rectangle to Edit Curve type XSec
    ConvertXSecToEdit( sid, 1 )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    # Get the control points for the default shape
    xsec1_pts = GetEditXSecCtrlVec( xsec_1, True ) # The returned control points will not be scaled by width and height

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] indx int XSec index
*/

extern void ConvertXSecToEdit( const std::string & geom_id, const int & indx = 0 );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Get the vector of fixed U flags for each control point in an EditCurveXSec. The fixed U flag is used to hold the
    U parameter of the control point constant when performing an equal arc length reparameterization of the curve.
    \forcpponly
    \code{.cpp}
    // Add Wing
    string wid = AddGeom( "WING" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( wid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    array < bool > @ fixed_u_vec = GetEditXSecFixedUVec( xsec_1 );
    if ( fixed_u_vec.length() == 0 )
    {
        Print( "ERROR: GetEditXSecFixedUVec returned nothing" );
        __failure++;
    }

    fixed_u_vec[3] = true; // change a flag

    SetEditXSecFixedUVec( xsec_1, fixed_u_vec );

    ReparameterizeEditXSec( xsec_1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing
    wid = AddGeom( "WING" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( wid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    fixed_u_vec = list(GetEditXSecFixedUVec( xsec_1 ))

    fixed_u_vec[3] = True # change a flag

    SetEditXSecFixedUVec( xsec_1, fixed_u_vec )

    ReparameterizeEditXSec( xsec_1 )

    \endcode
    \endPythonOnly
    \sa SetEditXSecFixedUVec, ReparameterizeEditXSec
    \param [in] xsec_id string XSec ID
    \return vector \<bool\> Array of bool values for each control point
*/

extern std::vector < bool > GetEditXSecFixedUVec( const std::string& xsec_id );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Set the vector of fixed U flags for each control point in an EditCurveXSec. The fixed U flag is used to hold the
    U parameter of the control point constant when performing an equal arc length reparameterization of the curve.
    \forcpponly
    \code{.cpp}
    // Add Wing
    string wid = AddGeom( "WING" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( wid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    array < bool > @ fixed_u_vec = GetEditXSecFixedUVec( xsec_1 );

    fixed_u_vec[3] = true; // change a flag

    SetEditXSecFixedUVec( xsec_1, fixed_u_vec );

    if ( GetEditXSecFixedUVec( xsec_1 ).length() != fixed_u_vec.length() )
    {
        Print( "ERROR: SetEditXSecFixedUVec length" );
        __failure++;
    }

    ReparameterizeEditXSec( xsec_1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing
    wid = AddGeom( "WING" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( wid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    fixed_u_vec = list(GetEditXSecFixedUVec( xsec_1 ))

    fixed_u_vec[3] = True # change a flag

    SetEditXSecFixedUVec( xsec_1, fixed_u_vec )

    assert len( GetEditXSecFixedUVec( xsec_1 ) ) == len( fixed_u_vec ), "SetEditXSecFixedUVec length"

    ReparameterizeEditXSec( xsec_1 )

    \endcode
    \endPythonOnly
    \sa GetEditXSecFixedUVec, ReparameterizeEditXSec
    \param [in] xsec_id string XSec ID
    \param [in] fixed_u_vec vector \<bool\> Array of fixed U flags
*/

extern void SetEditXSecFixedUVec( const std::string& xsec_id, std::vector < bool > fixed_u_vec );

/*!
    \ingroup EditCurveXSec
*/
/*!
    Perform an equal arc length repareterization on an EditCurveXSec. The reparameterization is performed between
    specific U values if the Fixed U flag is true. This allows corners, such as at 0.25, 0.5, and 0.75 U, to be held
    constant while everything between them is reparameterized.
    \forcpponly
    \code{.cpp}
    // Add Wing
    string wid = AddGeom( "WING" );

    // Get First (and Only) XSec Surf
    string xsec_surf = GetXSecSurf( wid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE );

    // Identify XSec 1
    string xsec_1 = GetXSec( xsec_surf, 1 );

    array < bool > @ fixed_u_vec = GetEditXSecFixedUVec( xsec_1 );

    fixed_u_vec[3] = true; // change a flag

    SetEditXSecFixedUVec( xsec_1, fixed_u_vec );

    ReparameterizeEditXSec( xsec_1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing
    wid = AddGeom( "WING" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( wid, 0 )

    ChangeXSecShape( xsec_surf, 1, XS_EDIT_CURVE )

    # Identify XSec 1
    xsec_1 = GetXSec( xsec_surf, 1 )

    fixed_u_vec = list(GetEditXSecFixedUVec( xsec_1 ))

    fixed_u_vec[3] = True # change a flag

    SetEditXSecFixedUVec( xsec_1, fixed_u_vec )

    ReparameterizeEditXSec( xsec_1 )

    \endcode
    \endPythonOnly
    \sa SetEditXSecFixedUVec, GetEditXSecFixedUVec
    \param [in] xsec_id string XSec ID
*/

extern void ReparameterizeEditXSec( const std::string & xsec_id );


//======================== Sets ================================//
/*!
    \ingroup Sets
*/
/*!
    Get the total number of defined sets. Named sets are used to group components and read/write on them. The number of named
    sets will be 10 for OpenVSP versions up to 3.17.1 and 20 for later versions.
    \forcpponly
    \code{.cpp}
    if ( GetNumSets() <= 0 )                            { Print( "---> Error: API GetNumSets " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    if  GetNumSets() <= 0 :
        print( "---> Error: API GetNumSets " )
        assert False, "---> Error: API GetNumSets"

    \endcode
    \endPythonOnly
    \return int Number of sets
*/

extern int GetNumSets();

/*!
    \ingroup Sets
*/
/*!
    Set the name of a set at specified index
    \forcpponly
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    if ( GetSetName( 3 ) != "SetFromScript" )            { Print( "---> Error: API Get/Set Set Name " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    if GetSetName(3) != "SetFromScript":
        print("---> Error: API Get/Set Set Name")
        assert False, "---> Error: API Get/Set Set Name"


    \endcode
    \endPythonOnly
    \sa SET_TYPE
    \param [in] index int Set index
    \param [in] name string Set name
*/

extern void SetSetName( int index, const std::string& name );

/*!
    \ingroup Sets
*/
/*!
    Get the name of a set at specified index
    \forcpponly
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    if (GetSetName(3) != "SetFromScript" )
    {
        Print("---> Error: API Get/Set Set Name");
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    if GetSetName(3) != "SetFromScript":
        print("---> Error: API Get/Set Set Name")
        assert False, "---> Error: API Get/Set Set Name"

    \endcode
    \endPythonOnly
    \sa SET_TYPE
    \param [in] index int Set index
    \return string Set name
*/

extern std::string GetSetName( int index );

/*!
    \ingroup Sets
*/
/*!
    Get an array of Geom IDs for the specified set index
    \forcpponly
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    array<string> @geom_arr1 = GetGeomSetAtIndex( 3 );

    array<string> @geom_arr2 = GetGeomSet( "SetFromScript" );

    if ( geom_arr1.size() != geom_arr2.size() )            { Print( "---> Error: API GetGeomSet " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    geom_arr1 = GetGeomSetAtIndex( 3 )

    geom_arr2 = GetGeomSet( "SetFromScript" )

    if  len(geom_arr1) != len(geom_arr2) :
        print( "---> Error: API GetGeomSet " )
        assert False, "---> Error: API GetGeomSet"

    \endcode
    \endPythonOnly
    \sa SET_TYPE
    \param [in] index int Set index
    \return vector\<string\> Array of Geom IDs
*/

extern std::vector<std::string> GetGeomSetAtIndex( int index );

/*!
    \ingroup Sets
*/
/*!
    Get an array of Geom IDs for the specified set name
    \forcpponly
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    array<string> @geom_arr1 = GetGeomSetAtIndex( 3 );

    array<string> @geom_arr2 = GetGeomSet( "SetFromScript" );

    if ( geom_arr1.size() != geom_arr2.size() )            { Print( "---> Error: API GetGeomSet " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    geom_arr1 = GetGeomSetAtIndex( 3 )

    geom_arr2 = GetGeomSet( "SetFromScript" )

    if  len(geom_arr1) != len(geom_arr2) :
        print( "---> Error: API GetGeomSet " )
        assert False, "---> Error: API GetGeomSet"

    \endcode
    \endPythonOnly
    \param [in] name string const string set name
    \return array<string> array of Geom IDs
*/

extern std::vector<std::string> GetGeomSet( const std::string & name );

/*!
    \ingroup Sets
*/
/*!
    Get the set index for the specified set name
    \forcpponly
    \code{.cpp}
    SetSetName( 3, "SetFromScript" );

    if ( GetSetIndex( "SetFromScript" ) != 3 ) { Print( "ERROR: GetSetIndex" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    if GetSetIndex("SetFromScript") != 3:
        print("ERROR: GetSetIndex")
        assert False, "ERROR: GetSetIndex"


    \endcode
    \endPythonOnly
    \param [in] name string Set name
    \return int Set index
*/

extern int GetSetIndex( const std::string & name );

/*!
    \ingroup Sets
*/
/*!
    Check if a Geom is in the set at the specified set index
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    SetSetFlag( fuseid, 3, true );

    if ( !GetSetFlag( fuseid, 3 ) )                        { Print( "---> Error: API Set/Get Set Flag " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    SetSetFlag( fuseid, 3, True )

    if not GetSetFlag(fuseid, 3):
        print("---> Error: API Set/Get Set Flag")
        assert False, "---> Error: API Set/Get Set Flag"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] set_index int Set index
    \return bool True if geom is in the set, false otherwise
*/

extern bool GetSetFlag( const std::string & geom_id, int set_index );

/*!
    \ingroup Sets
*/
/*!
    Set whether or not a Geom is a member of the set at specified set index
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    SetSetFlag( fuseid, 3, true );

    if ( !GetSetFlag( fuseid, 3 ) )                        { Print( "---> Error: API Set/Get Set Flag " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    SetSetFlag( fuseid, 3, True )

    if not GetSetFlag(fuseid, 3):
        print("---> Error: API Set/Get Set Flag")
        assert False, "---> Error: API Set/Get Set Flag"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] set_index int Set index
    \param [in] flag bool Flag that indicates set membership
*/

extern void SetSetFlag( const std::string & geom_id, int set_index, bool flag );

/*!
    \ingroup Sets
*/
/*!
    Copies all the states of a geom set and pastes them into a specific set based on passed in indexs
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    //set fuseid's state for set 3 to true
    SetSetFlag( fuseid, 3, true );

    //Copy set 3 and Paste into set 4
    CopyPasteSet( 3, 4 );

    //get fuseid's state for set 4
    bool flag_value = GetSetFlag( fuseid, 4 );

    if ( flag_value != true)                      { Print( "---> Error: API CopyPasteSet " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    #set fuseid's state for set 3 to true
    SetSetFlag( fuseid, 3, True )

    #Copy set 3 and Paste into set 4
    CopyPasteSet( 3, 4 )

    #get fuseid's state for set 4
    flag_value = GetSetFlag( fuseid, 4 )

    if  flag_value != True:
        print( "---> Error: API CopyPasteSet " )
        assert False, "---> Error: API CopyPasteSet"

    \endcode
    \endPythonOnly
    \param [in] copyIndex int Copy Index
    \param [in] pasteIndex int Paste Index
*/

extern void CopyPasteSet( int copyIndex, int pasteIndex );

/*!
    \ingroup Sets
*/
/*!
    Get the corners of the bounding box of the specified Set.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    Update();

    double xmin, ymin, zmin, xlen, ylen, zlen;
    bool sethasmembers = GetBBoxSet( SET_ALL, xmin, ymin, zmin, xlen, ylen, zlen );

    // A pod was added above, so the set is populated and the box has real size.
    if ( !sethasmembers || xlen <= 0.0 || ylen <= 0.0 || zlen <= 0.0 )
    {
        Print( "ERROR: GetBBoxSet" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    Update()

    sethasmembers, xmin, ymin, zmin, xlen, ylen, zlen = GetBBoxSet( SET_ALL )

    # A pod was added above, so the set is populated and the box has real size.
    assert sethasmembers, "GetBBoxSet"
    assert xlen > 0.0 and ylen > 0.0 and zlen > 0.0, "GetBBoxSet extent"
    \endcode
    \endPythonOnly
    \param [in] set int Desired Set
    \param [out] xmin_out double Minimum bounding box X coordinate
    \param [out] ymin_out double Minimum bounding box Y coordinate
    \param [out] zmin_out double Minimum bounding box Z coordinate
    \param [out] xlen_out double Maximum bounding box X length
    \param [out] ylen_out double Maximum bounding box Y length
    \param [out] zlen_out double Maximum bounding box Z length
    \return bool Flag indicating whether the set has members (is non-empty)
*/

extern bool GetBBoxSet( int set, double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out );

/*!
    \ingroup Sets
*/
/*!
    Get the corners of the scale independent bounding box of the specified Set.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    Update();

    double xmin, ymin, zmin, xlen, ylen, zlen;
    bool sethasmembers = GetScaleIndependentBBoxSet( SET_ALL, xmin, ymin, zmin, xlen, ylen, zlen );

    // A pod was added above, so the set is populated and the box has real size.
    if ( !sethasmembers || xlen <= 0.0 || ylen <= 0.0 || zlen <= 0.0 )
    {
        Print( "ERROR: GetScaleIndependentBBoxSet" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    Update()

    sethasmembers, xmin, ymin, zmin, xlen, ylen, zlen = GetScaleIndependentBBoxSet( SET_ALL )

    # A pod was added above, so the set is populated and the box has real size.
    assert sethasmembers, "GetScaleIndependentBBoxSet"
    assert xlen > 0.0 and ylen > 0.0 and zlen > 0.0, "GetScaleIndependentBBoxSet extent"
    \endcode
    \endPythonOnly
    \param [in] set int Desired Set
    \param [out] xmin_out double Minimum bounding box X coordinate
    \param [out] ymin_out double Minimum bounding box Y coordinate
    \param [out] zmin_out double Minimum bounding box Z coordinate
    \param [out] xlen_out double Maximum bounding box X length
    \param [out] ylen_out double Maximum bounding box Y length
    \param [out] zlen_out double Maximum bounding box Z length
    \return bool Flag indicating whether the set has members (is non-empty)
*/

extern bool GetScaleIndependentBBoxSet( int set, double & xmin_out, double & ymin_out, double & zmin_out, double & xlen_out, double & ylen_out, double & zlen_out );

//======================== Group Modifications ================================//
/*!
    \ingroup GroupMod
*/
/*!
    Apply a scale factor to a set
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    // Scale by a factor of 2
    Update();

    vec3d before_max = GetGeomBBoxMax( fuseid, 0, false );
    vec3d before_min = GetGeomBBoxMin( fuseid, 0, false );

    ScaleSet( 3, 2.0 );

    Update();

    vec3d after_max = GetGeomBBoxMax( fuseid, 0, false );
    vec3d after_min = GetGeomBBoxMin( fuseid, 0, false );

    if ( !closeTo( after_max.x() - after_min.x(), 2.0 * ( before_max.x() - before_min.x() ), 1e-6 ) )
    {
        Print( "ERROR: ScaleSet did not double the extent" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    # Scale by a factor of 2
    Update()

    before_max = GetGeomBBoxMax( fuseid, 0, False )
    before_min = GetGeomBBoxMin( fuseid, 0, False )

    ScaleSet( 3, 2.0 )

    Update()

    after_max = GetGeomBBoxMax( fuseid, 0, False )
    after_min = GetGeomBBoxMin( fuseid, 0, False )

    assert abs( ( after_max.x() - after_min.x() ) - 2.0 * ( before_max.x() - before_min.x() ) ) < 1e-6, "ScaleSet did not double the extent"

    \endcode
    \endPythonOnly
    \param [in] set_index int Set index
    \param [in] scale double Scale factor
*/

extern void ScaleSet( int set_index, double scale );

/*!
    \ingroup GroupMod
*/
/*!
    Rotate a set about the global X, Y, and Z axes
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    Update();

    vec3d before_max = GetGeomBBoxMax( fuseid, 0, true );
    vec3d before_min = GetGeomBBoxMin( fuseid, 0, true );

    // Rotate 90 degrees about Y
    RotateSet( 3, 0, 90, 0 );

    Update();

    vec3d after_max = GetGeomBBoxMax( fuseid, 0, true );
    vec3d after_min = GetGeomBBoxMin( fuseid, 0, true );

    // Turning the Geom on its nose trades the X extent for the Z extent.
    if ( !closeTo( after_max.z() - after_min.z(), before_max.x() - before_min.x(), 1e-6 ) ||
         !closeTo( after_max.x() - after_min.x(), before_max.z() - before_min.z(), 1e-6 ) )
    {
        Print( "ERROR: RotateSet did not rotate the geometry" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    Update()

    before_max = GetGeomBBoxMax( fuseid, 0, True )
    before_min = GetGeomBBoxMin( fuseid, 0, True )

    # Rotate 90 degrees about Y
    RotateSet( 3, 0, 90, 0 )

    Update()

    after_max = GetGeomBBoxMax( fuseid, 0, True )
    after_min = GetGeomBBoxMin( fuseid, 0, True )

    # Turning the Geom on its nose trades the X extent for the Z extent.
    assert abs( ( after_max.z() - after_min.z() ) - ( before_max.x() - before_min.x() ) ) < 1e-6, "RotateSet did not rotate the geometry"
    assert abs( ( after_max.x() - after_min.x() ) - ( before_max.z() - before_min.z() ) ) < 1e-6, "RotateSet did not rotate the geometry"

    \endcode
    \endPythonOnly
    \param [in] set_index int Set index
    \param [in] x_rot_deg double Rotation about the X axis (degrees)
    \param [in] y_rot_deg double Rotation about the Y axis (degrees)
    \param [in] z_rot_deg double Rotation about the Z axis (degrees)
*/

extern void RotateSet( int set_index, double x_rot_deg, double y_rot_deg, double z_rot_deg );

/*!
    \ingroup GroupMod
*/
/*!
    Translate a set along a given vector
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    // Translate 2 units in X and 3 units in Y
    Update();

    // The bounding box has to be asked for in the absolute frame.  A body frame
    // box travels with the Geom, so it would not see the translation at all.
    vec3d before_min = GetGeomBBoxMin( fuseid, 0, true );

    TranslateSet( 3, vec3d( 2, 3, 0 ) );

    Update();

    vec3d after_min = GetGeomBBoxMin( fuseid, 0, true );

    if ( !closeTo( after_min.x() - before_min.x(), 2.0, 1e-6 ) || !closeTo( after_min.y() - before_min.y(), 3.0, 1e-6 ) )
    {
        Print( "ERROR: TranslateSet did not move the geometry" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    # Translate 2 units in X and 3 units in Y
    Update()

    # The bounding box has to be asked for in the absolute frame.  A body frame
    # box travels with the Geom, so it would not see the translation at all.
    before_min = GetGeomBBoxMin( fuseid, 0, True )

    TranslateSet( 3, vec3d( 2, 3, 0 ) )

    Update()

    after_min = GetGeomBBoxMin( fuseid, 0, True )

    assert abs( ( after_min.x() - before_min.x() ) - 2.0 ) < 1e-6 and abs( ( after_min.y() - before_min.y() ) - 3.0 ) < 1e-6, "TranslateSet did not move the geometry"

    \endcode
    \endPythonOnly
    \param [in] set_index int Set index
    \param [in] translation_vec vec3d Translation vector
*/

extern void TranslateSet( int set_index, const vec3d &translation_vec );

/*!
    \ingroup GroupMod
*/
/*!
    Apply translation, rotation, and scale transformations to a set
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE" );

    SetSetFlag( fuseid, 3, true );

    Update();

    vec3d before_max = GetGeomBBoxMax( fuseid, 0, true );
    vec3d before_min = GetGeomBBoxMin( fuseid, 0, true );

    // Translate 2 units in X and 3 units in Y, rotate 90 degrees about Y, and scale by a factor of 2
    TransformSet( 3, vec3d( 2, 3, 0 ), 0, 90, 0, 2.0, true );

    Update();

    vec3d after_max = GetGeomBBoxMax( fuseid, 0, true );
    vec3d after_min = GetGeomBBoxMin( fuseid, 0, true );

    // Turning the Geom on its nose trades the X extent for the Z, and the scale
    // doubles both.
    if ( !closeTo( after_max.z() - after_min.z(), 2.0 * ( before_max.x() - before_min.x() ), 1e-6 ) ||
         !closeTo( after_max.x() - after_min.x(), 2.0 * ( before_max.z() - before_min.z() ), 1e-6 ) )
    {
        Print( "ERROR: TransformSet did not rotate and scale the geometry" );
        __failure++;
    }

    // The Y extent only scales, and the whole thing moves three units out in Y.
    if ( !closeTo( after_max.y() - after_min.y(), 2.0 * ( before_max.y() - before_min.y() ), 1e-6 ) )
    {
        Print( "ERROR: TransformSet did not scale the geometry in Y" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    Update()

    before_max = GetGeomBBoxMax( fuseid, 0, True )
    before_min = GetGeomBBoxMin( fuseid, 0, True )

    # Translate 2 units in X and 3 units in Y, rotate 90 degrees about Y, and scale by a factor of 2
    TransformSet( 3, vec3d( 2, 3, 0 ), 0, 90, 0, 2.0, True )

    Update()

    after_max = GetGeomBBoxMax( fuseid, 0, True )
    after_min = GetGeomBBoxMin( fuseid, 0, True )

    # Turning the Geom on its nose trades the X extent for the Z, and the scale
    # doubles both.
    assert abs( ( after_max.z() - after_min.z() ) - 2.0 * ( before_max.x() - before_min.x() ) ) < 1e-6, "TransformSet did not rotate and scale the geometry"
    assert abs( ( after_max.x() - after_min.x() ) - 2.0 * ( before_max.z() - before_min.z() ) ) < 1e-6, "TransformSet did not rotate and scale the geometry"

    # The Y extent only scales.
    assert abs( ( after_max.y() - after_min.y() ) - 2.0 * ( before_max.y() - before_min.y() ) ) < 1e-6, "TransformSet did not scale the geometry in Y"

    \endcode
    \endPythonOnly
    \sa TranslateSet, RotateSet, ScaleSet
    \param [in] set_index int Set index
    \param [in] translation_vec vec3d Translation vector
    \param [in] x_rot_deg double Rotation about the X axis (degrees)
    \param [in] y_rot_deg double Rotation about the Y axis (degrees)
    \param [in] z_rot_deg double Rotation about the Z axis (degrees)
    \param [in] scale double Scale factor
    \param [in] scale_translations_flag bool Flag to apply the scale factor to translations
*/

extern void TransformSet( int set_index, const vec3d &translation_vec, double x_rot_deg, double y_rot_deg, double z_rot_deg, double scale, bool scale_translations_flag );


//======================== Parm Functions ================================//
/*!
    \ingroup Parm
*/
/*!
    Check if given Parm is valid
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pid = AddGeom( "POD" );

    string lenid = GetParm( pid, "Length", "Design" );

    if ( !ValidParm( lenid ) )                { Print( "---> Error: API GetParm  " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    lenid = GetParm( pid, "Length", "Design" )

    if  not ValidParm( lenid ) :
        print( "---> Error: API GetParm  " )
        assert False, "---> Error: API GetParm"

    \endcode
    \endPythonOnly
    \param [in] id string Parm ID
    \return bool True if Parm ID is valid, false otherwise
*/

extern bool ValidParm( const std::string & id );

/*!
    \ingroup Parm
*/
/*!
    Set the value of the specified Parm.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    SetParmVal( wid, 23.0 )

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 :
        print( "---> Error: API Parm Val Set/Get " )
        assert False, "---> Error: API Parm Val Set/Get"

    \endcode
    \endPythonOnly
    \sa SetParmValUpdate
    \param [in] parm_id string Parm ID
    \param [in] val double Parm value to set
    \return double Value that the Parm was set to
*/

extern double SetParmVal( const std::string & parm_id, double val );

/*!
    \ingroup Parm
*/
/*!
    Set the value of the specified Parm.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    SetParmVal( wid, 23.0 )

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 :
        print( "---> Error: API Parm Val Set/Get " )
        assert False, "---> Error: API Parm Val Set/Get"

    \endcode
    \endPythonOnly
    \sa SetParmValUpdate
    \param [in] geom_id string Geom ID
    \param [in] name string Parm name
    \param [in] group string Parm group name
    \param [in] val double Parm value to set
    \return double Value that the Parm was set to
*/

extern double SetParmVal( const std::string & geom_id, const std::string & name, const std::string & group, double val );

/*!
    \ingroup Parm
*/
/*!
    Set the value along with the upper and lower limits of the specified Parm
    \forcpponly
    \code{.cpp}
    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    SetParmDescript( length, "Total Length of Geom" );

    // The value and both limits have to take.
    if ( !closeTo( GetParmVal( length ), 10.0, 1e-9 ) ||
         !closeTo( GetParmLowerLimit( length ), 0.001, 1e-9 ) ||
         !closeTo( GetParmUpperLimit( length ), 1.0e12, 1.0 ) )
    {
        Print( "ERROR: SetParmValLimits did not take" );
        __failure++;
    }

    if ( GetParmDescript( length ) != "Total Length of Geom" )
    {
        Print( "ERROR: SetParmDescript did not take" );
        __failure++;
    }

    // The limits are limits, so a value outside them gets clamped.
    SetParmVal( length, -1.0 );

    if ( !closeTo( GetParmVal( length ), 0.001, 1e-9 ) )
    {
        Print( "ERROR: the lower limit did not hold" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    SetParmDescript( length, "Total Length of Geom" )

    # The value and both limits have to take.
    assert abs( GetParmVal( length ) - 10.0 ) < 1e-9, "SetParmValLimits did not take"
    assert abs( GetParmLowerLimit( length ) - 0.001 ) < 1e-9, "SetParmValLimits did not take"
    assert abs( GetParmUpperLimit( length ) - 1.0e12 ) < 1.0, "SetParmValLimits did not take"

    assert GetParmDescript( length ) == "Total Length of Geom", "SetParmDescript did not take"

    # The limits are limits, so a value outside them gets clamped.
    SetParmVal( length, -1.0 )

    assert abs( GetParmVal( length ) - 0.001 ) < 1e-9, "the lower limit did not hold"

    \endcode
    \endPythonOnly
    \sa SetParmLowerLimit, SetParmUpperLimit
    \param [in] parm_id string Parm ID
    \param [in] val double Parm value to set
    \param [in] lower_limit double Parm lower limit
    \param [in] upper_limit double Parm upper limit
    \return double Value that the Parm was set to
*/

extern double SetParmValLimits( const std::string & parm_id, double val, double lower_limit, double upper_limit );

/*!
    \ingroup Parm
*/
/*!
    Set the value of the specified Parm and force an Update.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    string parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" );

    SetParmValUpdate( parm_id, 5.0 );

    if ( !closeTo( GetParmVal( parm_id ), 5.0, 1e-9 ) )
    {
        Print( "ERROR: SetParmValUpdate" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" )

    SetParmValUpdate( parm_id, 5.0 )

    assert abs( GetParmVal( parm_id ) - 5.0 ) < 1e-9, "SetParmValUpdate"

    \endcode
    \endPythonOnly
    \sa SetParmVal
    \param [in] parm_id string Parm ID
    \param [in] val double Parm value to set
    \return double Value that the Parm was set to
*/

extern double SetParmValUpdate( const std::string & parm_id, double val );

/*!
    \ingroup Parm
*/
/*!
    Set the value of the specified Parm and force an Update.
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    string parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" );

    SetParmValUpdate( parm_id, 5.0 );

    if ( !closeTo( GetParmVal( parm_id ), 5.0, 1e-9 ) )
    {
        Print( "ERROR: SetParmValUpdate" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" )

    SetParmValUpdate( parm_id, 5.0 )

    assert abs( GetParmVal( parm_id ) - 5.0 ) < 1e-9, "SetParmValUpdate"

    \endcode
    \endPythonOnly
    \sa SetParmVal
    \param [in] geom_id string Geom ID
    \param [in] parm_name string Parm name
    \param [in] parm_group_name string Parm group name
    \param [in] val double Parm value to set
    \return double Value that the Parm was set to
*/

extern double SetParmValUpdate( const std::string & geom_id, const std::string & parm_name, const std::string & parm_group_name, double val );

/*!
    \ingroup Parm
*/
/*!
    Get the value of the specified Parm. The data type of the Parm value will be cast to a double
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    SetParmVal( wid, 23.0 )

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 :
        print( "---> Error: API Parm Val Set/Get " )
        assert False, "---> Error: API Parm Val Set/Get"

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return double Parm value
*/

extern double GetParmVal( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get the value of the specified Parm. The data type of the Parm value will be cast to a double
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    SetParmVal( wid, 23.0 )

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 :
        print( "---> Error: API Parm Val Set/Get " )
        assert False, "---> Error: API Parm Val Set/Get"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Parm name
    \param [in] group string Parm group name
    \return double Parm value
*/

extern double GetParmVal( const std::string & geom_id, const std::string & name, const std::string & group );

/*!
    \ingroup Parm
*/
/*!
    Get the value of the specified int type Parm
    \forcpponly
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string num_blade_id = GetParm( prop_id, "NumBlade", "Design" );

    int num_blade = GetIntParmVal( num_blade_id );

    // A new propeller has three blades, and the int form has to agree with the
    // double form.
    if ( num_blade != 3 || num_blade != int( GetParmVal( num_blade_id ) ) )
    {
        Print( "ERROR: GetIntParmVal did not report the blade count" );
        __failure++;
    }

    // Setting it has to move both.
    SetParmVal( num_blade_id, 5 );

    if ( GetIntParmVal( num_blade_id ) != 5 )
    {
        Print( "ERROR: GetIntParmVal did not follow the Parm" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    num_blade_id = GetParm( prop_id, "NumBlade", "Design" )

    num_blade = GetIntParmVal( num_blade_id )

    # A new propeller has three blades, and the int form has to agree with the
    # double form.
    assert num_blade == 3, "GetIntParmVal did not report the blade count"
    assert num_blade == int( GetParmVal( num_blade_id ) ), "GetIntParmVal disagrees with GetParmVal"

    # Setting it has to move both.
    SetParmVal( num_blade_id, 5 )

    assert GetIntParmVal( num_blade_id ) == 5, "GetIntParmVal did not follow the Parm"

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return double Parm value
*/

extern int GetIntParmVal( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get the value of the specified bool type Parm
    \forcpponly
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string rev_flag_id = GetParm( prop_id, "ReverseFlag", "Design" );

    SetParmVal( rev_flag_id, 1.0 );

    bool reverse_flag = GetBoolParmVal( rev_flag_id );

    if ( !reverse_flag )
    {
        Print( "ERROR: GetBoolParmVal did not read back a set flag" );
        __failure++;
    }

    SetParmVal( rev_flag_id, 0.0 );

    if ( GetBoolParmVal( rev_flag_id ) )
    {
        Print( "ERROR: GetBoolParmVal did not read back a cleared flag" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    rev_flag_id = GetParm( prop_id, "ReverseFlag", "Design" )

    SetParmVal( rev_flag_id, 1.0 )

    reverse_flag = GetBoolParmVal( rev_flag_id )

    assert reverse_flag, "GetBoolParmVal did not read back a set flag"

    SetParmVal( rev_flag_id, 0.0 )

    assert not GetBoolParmVal( rev_flag_id ), "GetBoolParmVal did not read back a cleared flag"

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return bool Parm value
*/

extern bool GetBoolParmVal( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Set the upper limit value for the specified Parm
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 23.0 );

    SetParmUpperLimit( wid, 13.0 );

    if ( abs( GetParmVal( wid ) - 13 ) > 1e-6 )                { Print( "---> Error: API SetParmUpperLimit " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    SetParmVal( wid, 23.0 )

    SetParmUpperLimit( wid, 13.0 )

    if  abs( GetParmVal( wid ) - 13 ) > 1e-6 :
        print( "---> Error: API SetParmUpperLimit " )
        assert False, "---> Error: API SetParmUpperLimit"

    \endcode
    \endPythonOnly
    \sa SetParmValLimits
    \param [in] parm_id string Parm ID
    \param [in] val double Parm upper limit
*/

extern void SetParmUpperLimit( const std::string & parm_id, double val );

/*!
    \ingroup Parm
*/
/*!
    Get the upper limit value for the specified Parm
    \forcpponly
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string num_blade_id = GetParm( prop_id, "NumBlade", "Design" );

    double max_blade = GetParmUpperLimit( num_blade_id );

    if ( max_blade <= GetParmLowerLimit( num_blade_id ) || max_blade < GetParmVal( num_blade_id ) )
    {
        Print( "ERROR: GetParmUpperLimit" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    num_blade_id = GetParm( prop_id, "NumBlade", "Design" )

    max_blade = GetParmUpperLimit( num_blade_id )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return double Parm upper limit
*/

extern double GetParmUpperLimit( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Set the lower limit value for the specified Parm
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    SetParmVal( wid, 13.0 );

    SetParmLowerLimit( wid, 15.0 );

    if ( abs( GetParmVal( wid ) - 15 ) > 1e-6 )                { Print( "---> Error: API SetParmLowerLimit " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    SetParmVal( wid, 13.0 )

    SetParmLowerLimit( wid, 15.0 )

    if  abs( GetParmVal( wid ) - 15 ) > 1e-6 :
        print( "---> Error: API SetParmLowerLimit " )
        assert False, "---> Error: API SetParmLowerLimit"

    \endcode
    \endPythonOnly
    \sa SetParmValLimits
    \param [in] parm_id string Parm ID
    \param [in] val double Parm lower limit
*/

extern void SetParmLowerLimit( const std::string & parm_id, double val );

/*!
    \ingroup Parm
*/
/*!
    Get the lower limit value for the specified Parm
    \forcpponly
    \code{.cpp}
    //==== Add Prop Geometry ====//
    string prop_id = AddGeom( "PROP" );

    string num_blade_id = GetParm( prop_id, "NumBlade", "Design" );

    double min_blade = GetParmLowerLimit( num_blade_id );

    // A propeller needs at least one blade, and the limits have to bracket the
    // current value.
    if ( min_blade < 1.0 )
    {
        Print( "ERROR: GetParmLowerLimit allows a propeller with no blades" );
        __failure++;
    }

    if ( min_blade > GetParmVal( num_blade_id ) || min_blade > GetParmUpperLimit( num_blade_id ) )
    {
        Print( "ERROR: the lower limit is above the value or the upper limit" );
        __failure++;
    }

    // Asking for less than the limit allows has to clamp to it.
    SetParmVal( num_blade_id, min_blade - 10.0 );

    if ( !closeTo( GetParmVal( num_blade_id ), min_blade, 1e-9 ) )
    {
        Print( "ERROR: the lower limit did not hold" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    num_blade_id = GetParm( prop_id, "NumBlade", "Design" )

    min_blade = GetParmLowerLimit( num_blade_id )

    # A propeller needs at least one blade, and the limits have to bracket the
    # current value.
    assert min_blade >= 1.0, "GetParmLowerLimit allows a propeller with no blades"
    assert min_blade <= GetParmVal( num_blade_id ), "the lower limit is above the value"
    assert min_blade <= GetParmUpperLimit( num_blade_id ), "the lower limit is above the upper limit"

    # Asking for less than the limit allows has to clamp to it.
    SetParmVal( num_blade_id, min_blade - 10.0 )

    assert abs( GetParmVal( num_blade_id ) - min_blade ) < 1e-9, "the lower limit did not hold"

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return double Parm lower limit
*/

extern double GetParmLowerLimit( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get the data type for the specified Parm
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    if ( GetParmType( wid ) != PARM_DOUBLE_TYPE )        { Print( "---> Error: API GetParmType " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    if  GetParmType( wid ) != PARM_DOUBLE_TYPE :
        print( "---> Error: API GetParmType " )
        assert False, "---> Error: API GetParmType"

    \endcode
    \endPythonOnly
    \sa PARM_TYPE
    \param [in] parm_id string Parm ID
    \return int Parm data type enum (i.e. PARM_BOOL_TYPE)
*/

extern int GetParmType( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get the name for the specified Parm
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Get Structure Name and Parm Container ID ====//
    string parm_container_name = GetFeaStructName( pod_id, struct_ind );

    string parm_container_id = FindContainer( parm_container_name, struct_ind );

    //==== Get and List All Parms in the Container ====//
    array<string> parm_ids = FindContainerParmIDs( parm_container_id );

    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string name_id = GetParmName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");
        if ( name_id.length() == 0 )
        {
            Print( "ERROR: GetParmName returned nothing" );
            __failure++;
        }

        Print( name_id );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Get Structure Name and Parm Container ID ====//
    parm_container_name = GetFeaStructName( pod_id, struct_ind )

    parm_container_id = FindContainer( parm_container_name, struct_ind )

    #==== Get and List All Parms in the Container ====//
    parm_ids = FindContainerParmIDs( parm_container_id )

    for i in range(len(parm_ids)):

        name_id = GetParmName( parm_ids[i] ) + ": " + parm_ids[i] + "\n"
        assert len( name_id ) > 0, "GetParmName returned nothing"

        print( name_id )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return string Parm name
*/

extern std::string GetParmName( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get the group name for the specified Parm
    \forcpponly
    \code{.cpp}
    string veh_id = FindContainer( "Vehicle", 0 );

    //==== Get and List All Parms in the Container ====//
    array<string> parm_ids = FindContainerParmIDs( veh_id );

    Print( "Parm Groups and IDs in Vehicle Parm Container: " );

    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string group_str = GetParmGroupName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");
        if ( group_str.length() == 0 )
        {
            Print( "ERROR: GetParmGroupName returned nothing" );
            __failure++;
        }

        Print( group_str );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    veh_id = FindContainer( "Vehicle", 0 )

    #==== Get and List All Parms in the Container ====//
    parm_ids = FindContainerParmIDs( veh_id )

    print( "Parm Groups and IDs in Vehicle Parm Container: " )

    for i in range(len(parm_ids)):

        group_str = GetParmGroupName( parm_ids[i] ) + ": " + parm_ids[i] + "\n"
        assert len( group_str ) > 0, "GetParmGroupName returned nothing"

        print( group_str )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return string Parm group name
*/

extern std::string GetParmGroupName( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get the display group name for the specified Parm
    \forcpponly
    \code{.cpp}
    string veh_id = FindContainer( "Vehicle", 0 );

    //==== Get and List All Parms in the Container ====//
    array<string> parm_ids = FindContainerParmIDs( veh_id );

    Print( "Parm Group Display Names and IDs in Vehicle Parm Container: " );

    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string group_str = GetParmDisplayGroupName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");
        if ( group_str.length() == 0 )
        {
            Print( "ERROR: GetParmDisplayGroupName returned nothing" );
            __failure++;
        }

        Print( group_str );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    veh_id = FindContainer( "Vehicle", 0 )

    #==== Get and List All Parms in the Container ====//
    parm_ids = FindContainerParmIDs( veh_id )

    print( "Parm Group Display Names and IDs in Vehicle Parm Container: " )

    for i in range(len(parm_ids)):

        group_str = GetParmDisplayGroupName( parm_ids[i] ) + ": " + parm_ids[i] + "\n"
        assert len( group_str ) > 0, "GetParmDisplayGroupName returned nothing"

        print( group_str )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return string Parm display group name
*/

extern std::string GetParmDisplayGroupName( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Get Parm Container ID for the specified Parm
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE );

    string xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 );

    string wid = GetXSecParm( xsec, "RoundedRect_Width" );

    string cid = GetParmContainer( wid );

    if ( cid.size() == 0 )                                { Print( "---> Error: API GetParmContainer " ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    ChangeXSecShape( xsec_surf, GetNumXSec( xsec_surf ) - 1, XS_ROUNDED_RECTANGLE )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    wid = GetXSecParm( xsec, "RoundedRect_Width" )

    cid = GetParmContainer( wid )

    if  len(cid) == 0 :
        print( "---> Error: API GetParmContainer " )
        assert False, "---> Error: API GetParmContainer"

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return string Parm Container ID
*/

extern std::string GetParmContainer( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Set the description of the specified Parm
    \forcpponly
    \code{.cpp}
    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    SetParmDescript( length, "Total Length of Geom" );
    if ( GetParmDescript( length ) != "Total Length of Geom" )
    {
        Print( "ERROR: SetParmDescript did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    SetParmDescript( length, "Total Length of Geom" )
    assert GetParmDescript( length ) == "Total Length of Geom", "SetParmDescript did not take"


    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] desc string Parm description
*/

extern void SetParmDescript( const std::string & parm_id, const std::string & desc );

/*!
    \ingroup Parm
*/
/*!
    Get the description of the specified Parm
    \forcpponly
    \code{.cpp}
    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    string desc = GetParmDescript( length );
    if ( desc.length() == 0 )
    {
        Print( "ERROR: GetParmDescript returned nothing" );
        __failure++;
    }
    Print( desc );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    desc = GetParmDescript( length )
    assert len( desc ) > 0, "GetParmDescript returned nothing"
    print( desc )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return string desc Parm description
*/

extern std::string GetParmDescript( const std::string & parm_id );

/*!
    \ingroup Parm
*/
/*!
    Find a Parm ID given the Parm Container ID, Parm name, and Parm group
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry ====//
    string wing_id = AddGeom( "WING" );

    //==== Turn Symmetry OFF ====//
    string sym_id = FindParm( wing_id, "Sym_Planar_Flag", "Sym");
    if ( sym_id.length() == 0 )
    {
        Print( "ERROR: FindParm found nothing" );
        __failure++;
    }

    SetParmVal( sym_id, 0.0 ); // Note: bool input not supported in SetParmVal
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Turn Symmetry OFF ====//
    sym_id = FindParm( wing_id, "Sym_Planar_Flag", "Sym")
    assert len( sym_id ) > 0, "FindParm found nothing"

    SetParmVal( sym_id, 0.0 ) # Note: bool input not supported in SetParmVal

    \endcode
    \endPythonOnly
    \param [in] parm_container_id string Parm Container ID
    \param [in] parm_name string Parm name
    \param [in] group_name string Parm group name
    \return string Parm ID
*/

extern std::string FindParm( const std::string & parm_container_id, const std::string& parm_name, const std::string& group_name );


//======================== Parm Container Functions ======================//

/*!
    \ingroup ParmContainer
*/
/*!
    Get an array of all Parm Container IDs
    \forcpponly
    \code{.cpp}
    array<string> @ctr_arr = FindContainers();
    if ( ctr_arr.length() == 0 )
    {
        Print( "ERROR: FindContainers found nothing" );
        __failure++;
    }

    Print( "---> API Parm Container IDs: " );

    for ( int i = 0; i < int( ctr_arr.size() ); i++ )
    {
        string message = "\t" + ctr_arr[i] + "\n";

        Print( message );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ctr_arr = FindContainers()
    assert len( ctr_arr ) > 0, "FindContainers found nothing"

    print( "---> API Parm Container IDs: " )

    for i in range(int( len(ctr_arr) )):

        message = "\t" + ctr_arr[i] + "\n"

        print( message )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of Parm Container IDs
*/

extern std::vector<std::string> FindContainers();

/*!
    \ingroup ParmContainer
*/
/*!
    Get an array of Parm Container IDs for Containers with the specified name
    \forcpponly
    \code{.cpp}
    array<string> @ctr_arr = FindContainersWithName( "UserParms" );
    if ( ctr_arr.length() == 0 )
    {
        Print( "ERROR: FindContainersWithName found nothing" );
        __failure++;
    }

    if ( ctr_arr.size() > 0 )            { Print( ( "UserParms Parm Container ID: " + ctr_arr[0] ) ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ctr_arr = FindContainersWithName( "UserParms" )
    assert len( ctr_arr ) > 0, "FindContainersWithName found nothing"

    if  len(ctr_arr) > 0 : print( ( "UserParms Parm Container ID: " + ctr_arr[0] ) )

    \endcode
    \endPythonOnly
    \param [in] name string Parm Container name
    \return vector\<string\> Array of Parm Container IDs
*/

extern std::vector<std::string> FindContainersWithName( const std::string & name );

/*!
    \ingroup ParmContainer
*/
/*!
    Get the ID of a Parm Container with specified name at input index
    \forcpponly
    \code{.cpp}
    //===== Get Vehicle Parm Container ID ====//
    string veh_id = FindContainer( "Vehicle", 0 );
    if ( veh_id.length() == 0 )
    {
        Print( "ERROR: FindContainer found nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #===== Get Vehicle Parm Container ID ====//
    veh_id = FindContainer( "Vehicle", 0 )
    assert len( veh_id ) > 0, "FindContainer found nothing"

    \endcode
    \endPythonOnly
    \sa FindContainersWithName
    \param [in] name string Parm Container name
    \param [in] index int Parm Container index
    \return string Parm Container ID
*/

extern std::string FindContainer( const std::string & name, int index );

/*!
    \ingroup ParmContainer
*/
/*!
    Get the name of the specified Parm Container
    \forcpponly
    \code{.cpp}
    string veh_id = FindContainer( "Vehicle", 0 );

    if ( GetContainerName( veh_id ) != "Vehicle" )         { Print( "---> Error: API GetContainerName" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    veh_id = FindContainer( "Vehicle", 0 )

    if  GetContainerName( veh_id) != "Vehicle":
        print( "---> Error: API GetContainerName" )
        assert False, "---> Error: API GetContainerName"

    \endcode
    \endPythonOnly
    \param [in] parm_container_id string Parm Container ID
    \return string Parm Container name
*/

extern std::string GetContainerName( const std::string & parm_container_id );

/*!
    \ingroup ParmContainer
*/
/*!
    Get an array of Parm group names included in the specified Container
    \forcpponly
    \code{.cpp}
    string user_ctr = FindContainer( "UserParms", 0 );

    array<string> @grp_arr = FindContainerGroupNames( user_ctr );
    if ( grp_arr.length() == 0 )
    {
        Print( "ERROR: FindContainerGroupNames found nothing" );
        __failure++;
    }

    Print( "---> UserParms Container Group IDs: " );
    for ( int i = 0; i < int( grp_arr.size() ); i++ )
    {
        string message = "\t" + grp_arr[i] + "\n";

        Print( message );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    user_ctr = FindContainer( "UserParms", 0 )

    grp_arr = FindContainerGroupNames( user_ctr )
    assert len( grp_arr ) > 0, "FindContainerGroupNames found nothing"

    print( "---> UserParms Container Group IDs: " )
    for i in range(int( len(grp_arr) )):

        message = "\t" + grp_arr[i] + "\n"

        print( message )

    \endcode
    \endPythonOnly
    \param [in] parm_container_id string Parm Container ID
    \return vector\<string\> Array of Parm group names
*/

extern std::vector<std::string> FindContainerGroupNames( const std::string & parm_container_id );

/*!
    \ingroup ParmContainer
*/
/*!
    Get an array of Parm IDs included in the specified Container
    \forcpponly
    \code{.cpp}
    //==== Add Pod Geometry ====//
    string pod_id = AddGeom( "POD" );

    //==== Add FeaStructure to Pod ====//
    int struct_ind = AddFeaStruct( pod_id );

    //==== Get Structure Name and Parm Container ID ====//
    string parm_container_name = GetFeaStructName( pod_id, struct_ind );

    string parm_container_id = FindContainer( parm_container_name, struct_ind );

    //==== Get and List All Parms in the Container ====//
    array<string> parm_ids = FindContainerParmIDs( parm_container_id );
    if ( parm_ids.length() == 0 )
    {
        Print( "ERROR: FindContainerParmIDs found nothing" );
        __failure++;
    }

    for ( uint i = 0; i < uint(parm_ids.length()); i++ )
    {
        string name_id = GetParmName( parm_ids[i] ) + string(": ") + parm_ids[i] + string("\n");

        Print( name_id );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    #==== Get Structure Name and Parm Container ID ====//
    parm_container_name = GetFeaStructName( pod_id, struct_ind )

    parm_container_id = FindContainer( parm_container_name, struct_ind )

    #==== Get and List All Parms in the Container ====//
    parm_ids = FindContainerParmIDs( parm_container_id )
    assert len( parm_ids ) > 0, "FindContainerParmIDs found nothing"

    for i in range(len(parm_ids)):

        name_id = GetParmName( parm_ids[i] ) + ": " + parm_ids[i] + "\n"

        print( name_id )

    \endcode
    \endPythonOnly
    \param [in] parm_container_id string Parm Container ID
    \return vector\<string\> Array of Parm IDs
*/

extern std::vector<std::string> FindContainerParmIDs( const std::string & parm_container_id );

/*!
    \ingroup ParmContainer
*/
/*!
    Get the ID of the Vehicle Parm Container
    \forcpponly
    \code{.cpp}
    //===== Get Vehicle Parm Container ID ====//
    string veh_id = GetVehicleID();
    if ( veh_id.length() == 0 )
    {
        Print( "ERROR: GetVehicleID returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #===== Get Vehicle Parm Container ID ====//
    veh_id = GetVehicleID()
    assert len( veh_id ) > 0, "GetVehicleID returned nothing"

    \endcode
    \endPythonOnly
    \return string Vehicle ID
*/

extern std::string GetVehicleID();


//======================== User Parm Functions ======================//
/*!
    \ingroup ParmContainer
*/
/*!
    Get the number of user parameters
    \forcpponly
    \code{.cpp}
    int n = GetNumUserParms();

    // A fresh model already carries the predefined user Parms, and the count has
    // to match the list.
    if ( n != GetNumPredefinedUserParms() || n != int( GetAllUserParms().size() ) )
    {
        Print( "ERROR: GetNumUserParms disagrees with the user Parm list" );
        __failure++;
    }

    // Adding one has to move the count.
    AddUserParm( PARM_DOUBLE_TYPE, "ExampleParm", "ExampleGroup" );

    if ( GetNumUserParms() != n + 1 )
    {
        Print( "ERROR: GetNumUserParms did not follow AddUserParm" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    n = GetNumUserParms()

    # A fresh model already carries the predefined user Parms, and the count has
    # to match the list.
    assert n == GetNumPredefinedUserParms(), "GetNumUserParms does not match the predefined count"
    assert n == len( GetAllUserParms() ), "GetNumUserParms disagrees with the user Parm list"

    # Adding one has to move the count.
    AddUserParm( PARM_DOUBLE_TYPE, "ExampleParm", "ExampleGroup" )

    assert GetNumUserParms() == n + 1, "GetNumUserParms did not follow AddUserParm"

    \endcode
    \endPythonOnly
    \return int Number of user Parms
*/

extern int GetNumUserParms();

/*!
    \ingroup ParmContainer
*/
/*!
    Get the number of pre-defined user parameters
    \forcpponly
    \code{.cpp}
    int n = GetNumPredefinedUserParms();

    if ( n <= 0 )
    {
        Print( "ERROR: GetNumPredefinedUserParms" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    n = GetNumPredefinedUserParms()

    assert n > 0, "GetNumPredefinedUserParms"


    \endcode
    \endPythonOnly
    \return int Number of pre-defined user Parms
*/

extern int GetNumPredefinedUserParms();

/*!
    \ingroup ParmContainer
*/
/*!
    Get the vector of id's for all user parameters
    \forcpponly
    \code{.cpp}
    array<string> @id_arr = GetAllUserParms();
    if ( id_arr.length() == 0 )
    {
        Print( "ERROR: GetAllUserParms returned nothing" );
        __failure++;
    }

    Print( "---> User Parm IDs: " );

    for ( int i = 0; i < int( id_arr.size() ); i++ )
    {
        string message = "\t" + id_arr[i] + "\n";

        Print( message );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    id_arr = GetAllUserParms()
    assert len( id_arr ) > 0, "GetAllUserParms returned nothing"

    print( "---> User Parm IDs: " )

    for i in range(int( len(id_arr) )):

        message = "\t" + id_arr[i] + "\n"

        print( message )

    \endcode
    \endPythonOnly
    \return vector \<string\> Array of user parameter ids
*/

extern std::vector < std::string > GetAllUserParms();

/*!
    \ingroup ParmContainer
*/
/*!
    Get the user parm container ID
    \forcpponly
    \code{.cpp}
    string up_id = GetUserParmContainer();
    if ( up_id.length() == 0 )
    {
        Print( "ERROR: GetUserParmContainer returned nothing" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    up_id = GetUserParmContainer()
    assert len( up_id ) > 0, "GetUserParmContainer returned nothing"

    \endcode
    \endPythonOnly
    \return string User parm container ID
*/

extern std::string GetUserParmContainer();

/*!
    \ingroup ParmContainer
*/
  /*!
    Function to add a new user Parm of input type, name, and group
    \forcpponly
    \code{.cpp}
    string length = AddUserParm( PARM_DOUBLE_TYPE, "Length", "Design" );

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 );

    SetParmDescript( length, "Length user parameter" );

    if ( length.length() == 0 || length == "NONE" )
    {
        Print( "ERROR: AddUserParm returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    length = AddUserParm( PARM_DOUBLE_TYPE, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    SetParmDescript( length, "Length user parameter" )

    \endcode
    \endPythonOnly
    \sa PARM_TYPE
    \param [in] type int Parm type enum (i.e. PARM_DOUBLE_TYPE)
    \param [in] name string Parm name
    \param [in] group string Parm group
    \return string Parm ID
  */

extern string AddUserParm(int type, const string & name, const string & group );

/*!
    \ingroup ParmContainer
*/
/*!
    Get the user parm container ID
    \forcpponly
    \code{.cpp}

    int n = GetNumPredefinedUserParms();
    array<string> @id_arr = GetAllUserParms();

    if ( int( id_arr.size() ) > n )
    {
        int num_before_del = GetNumUserParms();
        DeleteUserParm( id_arr[n] );
        if ( GetNumUserParms() >= num_before_del )
        {
            Print( "ERROR: DeleteUserParm removed nothing" );
            __failure++;
        }

    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    n = GetNumPredefinedUserParms()
    id_arr = GetAllUserParms()

    if  len(id_arr) > n :
        num_before_del = GetNumUserParms()
        DeleteUserParm( id_arr[n] )
        assert GetNumUserParms() < num_before_del, "DeleteUserParm removed nothing"


    \endcode
    \endPythonOnly
*/

extern void DeleteUserParm( const std::string & id );

/*!
    \ingroup ParmContainer
*/
/*!
    Delete all user created Parms.  The predefined User_0 through User_15 Parms
    belong to the vehicle and are not removed.
    \forcpponly
    \code{.cpp}
    // A fresh model already carries the predefined user Parms, so record the
    // starting count rather than expecting to end at zero.
    int num_before = GetNumUserParms();

    AddUserParm( PARM_DOUBLE_TYPE, "Param1", "Group1" );
    AddUserParm( PARM_DOUBLE_TYPE, "Param2", "Group1" );

    if ( GetNumUserParms() != num_before + 2 )
    {
        Print( "ERROR: AddUserParm" );
        __failure++;
    }

    DeleteAllUserParm();

    if ( GetNumUserParms() != num_before )
    {
        Print( "ERROR: DeleteAllUserParm" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # A fresh model already carries the predefined user Parms, so record the
    # starting count rather than expecting to end at zero.
    num_before = GetNumUserParms()

    AddUserParm( PARM_DOUBLE_TYPE, "Param1", "Group1" )
    AddUserParm( PARM_DOUBLE_TYPE, "Param2", "Group1" )

    assert GetNumUserParms() == num_before + 2, "AddUserParm"

    DeleteAllUserParm()

    assert GetNumUserParms() == num_before, "DeleteAllUserParm"


    \endcode
    \endPythonOnly
*/

extern void DeleteAllUserParm();


//======================== Snap To Functions ======================//
/*!
    \ingroup SnapTo
*/
/*!
    Compute the minimum clearance distance for the specified geometry
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string pid = AddGeom( "POD", "" );                     // Add Pod

    string x = GetParm( pid, "X_Rel_Location", "XForm" );

    SetParmVal( x, 3.0 );

    Update();

    double min_dist = ComputeMinClearanceDistance( pid, SET_ALL );

    // The Pod was moved clear of the Fuselage, so there is a real gap between
    // them.  Sliding it back into the Fuselage has to close that gap.
    if ( min_dist <= 0.0 )
    {
        Print( "ERROR: ComputeMinClearanceDistance reports no clearance between separated Geoms" );
        __failure++;
    }

    SetParmVal( x, 0.0 );

    Update();

    if ( ComputeMinClearanceDistance( pid, SET_ALL ) >= min_dist )
    {
        Print( "ERROR: ComputeMinClearanceDistance did not close as the Geoms came together" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    pid = AddGeom( "POD", "" )                     # Add Pod

    x = GetParm( pid, "X_Rel_Location", "XForm" )

    SetParmVal( x, 3.0 )

    Update()

    min_dist = ComputeMinClearanceDistance( pid, SET_ALL )

    # The Pod was moved clear of the Fuselage, so there is a real gap between
    # them.  Sliding it back into the Fuselage has to close that gap.
    assert min_dist > 0.0, "ComputeMinClearanceDistance reports no clearance between separated Geoms"

    SetParmVal( x, 0.0 )

    Update()

    assert ComputeMinClearanceDistance( pid, SET_ALL ) < min_dist, "ComputeMinClearanceDistance did not close as the Geoms came together"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] set int Collision set enum (i.e. SET_ALL)
    \param [in] useMode bool Flag determine if mode is used instead of sets
    \param [in] modeID string ID of Mode to use
    \return double Minimum clearance distance
*/

extern double ComputeMinClearanceDistance( const std::string & geom_id, int set  = SET_ALL, bool useMode = false, const string &modeID = string() );

/*!
    \ingroup SnapTo
*/
/*!
    Snap the specified Parm to input target minimum clearance distance
    \forcpponly
    \code{.cpp}
    //Add Geoms
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    string pid = AddGeom( "POD", "" );                     // Add Pod

    string x = GetParm( pid, "X_Rel_Location", "XForm" );

    SetParmVal( x, 3.0 );

    Update();

    double min_dist = SnapParm( x, 0.1, true, SET_ALL );

    Update();

    // Snapping moves the Parm until the clearance reaches the target, so the
    // Parm has to have moved and the clearance has to end up where it was
    // asked for.
    if ( closeTo( GetParmVal( x ), 3.0, 1e-9 ) )
    {
        Print( "ERROR: SnapParm did not move the Parm" );
        __failure++;
    }

    if ( !closeTo( ComputeMinClearanceDistance( pid, SET_ALL ), 0.1, 1e-4 ) )
    {
        Print( "ERROR: SnapParm did not reach the target clearance" );
        __failure++;
    }

    if ( !closeTo( min_dist, ComputeMinClearanceDistance( pid, SET_ALL ), 1e-4 ) )
    {
        Print( "ERROR: SnapParm reported a clearance it did not reach" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #Add Geoms
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    pid = AddGeom( "POD", "" )                     # Add Pod

    x = GetParm( pid, "X_Rel_Location", "XForm" )

    SetParmVal( x, 3.0 )

    Update()

    min_dist = SnapParm( x, 0.1, True, SET_ALL )

    Update()

    # Snapping moves the Parm until the clearance reaches the target, so the Parm
    # has to have moved and the clearance has to end up where it was asked for.
    assert abs( GetParmVal( x ) - 3.0 ) > 1e-9, "SnapParm did not move the Parm"
    assert abs( ComputeMinClearanceDistance( pid, SET_ALL ) - 0.1 ) < 1e-4, "SnapParm did not reach the target clearance"
    assert abs( min_dist - ComputeMinClearanceDistance( pid, SET_ALL ) ) < 1e-4, "SnapParm reported a clearance it did not reach"

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] target_min_dist double Target minimum clearance distance
    \param [in] inc_flag bool Direction indication flag. If true, upper parm limit is used and direction is set to positive
    \param [in] set int Collision set enum (i.e. SET_ALL)
    \param [in] useMode bool Flag determine if mode is used instead of sets
    \param [in] modeID string ID of Mode to use
    \return double Minimum clearance distance
*/ // TODO: Validate inc_flag description

extern double SnapParm( const std::string & parm_id, double target_min_dist, bool inc_flag, int set = SET_ALL, bool useMode = false, const string &modeID = string() );


//======================== Variable Preset Functions ======================//

/*!
    \ingroup VariablePreset
*/
/*!
    Add a Variable Preset Group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    if ( gid.length() == 0 || gid == "NONE" )
    {
        Print( "ERROR: AddVarPresetGroup returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )
    \endcode
    \endPythonOnly
    \param [in] group_name string Name for Var Preset Group
    \return string Var Preset Group ID
*/

extern string AddVarPresetGroup( const std::string &group_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Add a Setting to the Variable Preset Group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid =AddVarPresetSetting( gid, "Coarse" );

    if ( sid.length() == 0 || sid == "NONE" )
    {
        Print( "ERROR: AddVarPresetSetting returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] setting_name string Var Preset Setting Name
    \return string Var Preset Setting ID
*/

extern string AddVarPresetSetting( const std::string &group_id, const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Add a Parm to the Variable Preset Group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    // The Parm has to show up in the group's list, and only once.
    array< string > @parm_ids = GetVarPresetParmIDs( gid );

    if ( parm_ids.size() != 1 || parm_ids[0] != p1 )
    {
        Print( "ERROR: AddVarPresetParm did not add the Parm to the group" );
        __failure++;
    }

    // Adding it again has to be rejected rather than duplicating it.
    AddVarPresetParm( gid, p1 );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: AddVarPresetParm accepted the same Parm twice" );
        __failure++;
    }

    if ( GetVarPresetParmIDs( gid ).size() != 1 )
    {
        Print( "ERROR: AddVarPresetParm added the same Parm twice" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    # The Parm has to show up in the group's list, and only once.
    parm_ids = GetVarPresetParmIDs( gid )

    assert len( parm_ids ) == 1, "AddVarPresetParm did not add the Parm to the group"
    assert parm_ids[0] == p1, "AddVarPresetParm added the wrong Parm"

    # Adding it again has to be rejected rather than duplicating it.  The error
    # queue is reached through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    AddVarPresetParm( gid, p1 )

    assert err_mgr.GetNumTotalErrors() > 0, "AddVarPresetParm accepted the same Parm twice"
    assert len( GetVarPresetParmIDs( gid ) ) == 1, "AddVarPresetParm added the same Parm twice"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] parm_id string Parm ID
*/

extern void AddVarPresetParm( const std::string &group_id, const std::string &parm_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Delete Variable Preset Group (and all contained settings)
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    int num_before_del = GetVarPresetGroups().length();
    DeleteVarPresetGroup( gid );
    if ( GetVarPresetGroups().length() >= num_before_del )
    {
        Print( "ERROR: DeleteVarPresetGroup removed nothing" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    num_before_del = len( GetVarPresetGroups() )
    DeleteVarPresetGroup( gid )
    assert len( GetVarPresetGroups() ) < num_before_del, "DeleteVarPresetGroup removed nothing"


    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
*/

extern void DeleteVarPresetGroup( const std::string &group_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Delete Variable Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    int num_before_del = GetVarPresetSettings( gid ).length();
    DeleteVarPresetSetting( gid, sid );
    if ( GetVarPresetSettings( gid ).length() >= num_before_del )
    {
        Print( "ERROR: DeleteVarPresetSetting removed nothing" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    num_before_del = len( GetVarPresetSettings( gid ) )
    DeleteVarPresetSetting( gid, sid )
    assert len( GetVarPresetSettings( gid ) ) < num_before_del, "DeleteVarPresetSetting removed nothing"


    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] setting_id string Var Preset Setting ID
*/

extern void DeleteVarPresetSetting( const std::string &group_id, const std::string &setting_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Delete Parm from Variable Preset Group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    int num_before_del = GetVarPresetParmIDs( gid ).length();
    DeleteVarPresetParm( gid, p1 );
    if ( GetVarPresetParmIDs( gid ).length() >= num_before_del )
    {
        Print( "ERROR: DeleteVarPresetParm removed nothing" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    num_before_del = len( GetVarPresetParmIDs( gid ) )
    DeleteVarPresetParm( gid, p1 )
    assert len( GetVarPresetParmIDs( gid ) ) < num_before_del, "DeleteVarPresetParm removed nothing"


    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] parm_id string Var Parm ID
*/

extern void DeleteVarPresetParm( const std::string &group_id, const std::string &parm_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Set value for Parm in Var Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    SetVarPresetParmVal( gid, sid, p1, 51 );
    if ( !closeTo( GetVarPresetParmVal( gid, sid, p1 ), 51, 1e-9 ) )
    {
        Print( "ERROR: SetVarPresetParmVal did not take" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    SetVarPresetParmVal( gid, sid, p1, 51 )
    assert abs( GetVarPresetParmVal( gid, sid, p1 ) - 51 ) < 1e-9, "SetVarPresetParmVal did not take"


    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] setting_id string Var Preset Setting ID
    \param [in] parm_id string Var Parm ID
    \param [in] parm_val double Parm value
*/

extern void SetVarPresetParmVal( const std::string &group_id, const std::string &setting_id, const std::string &parm_id, double parm_val );

/*!
    \ingroup VariablePreset
*/
/*!
    Get value for Parm in Var Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    double val = GetVarPresetParmVal( gid, sid, p1 );

    // A Parm joins the group at its current value.
    if ( !closeTo( val, GetParmVal( p1 ), 1e-9 ) )
    {
        Print( "ERROR: GetVarPresetParmVal does not match the Parm" );
        __failure++;
    }

    // Storing a different value has to be what comes back, without disturbing
    // the Parm itself.
    SetVarPresetParmVal( gid, sid, p1, val + 3.0 );

    if ( !closeTo( GetVarPresetParmVal( gid, sid, p1 ), val + 3.0, 1e-9 ) )
    {
        Print( "ERROR: GetVarPresetParmVal did not follow SetVarPresetParmVal" );
        __failure++;
    }

    if ( !closeTo( GetParmVal( p1 ), val, 1e-9 ) )
    {
        Print( "ERROR: storing a preset value changed the Parm" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    val = GetVarPresetParmVal( gid, sid, p1 )

    # A Parm joins the group at its current value.
    assert abs( val - GetParmVal( p1 ) ) < 1e-9, "GetVarPresetParmVal does not match the Parm"

    # Storing a different value has to be what comes back, without disturbing the
    # Parm itself.
    SetVarPresetParmVal( gid, sid, p1, val + 3.0 )

    assert abs( GetVarPresetParmVal( gid, sid, p1 ) - ( val + 3.0 ) ) < 1e-9, "GetVarPresetParmVal did not follow SetVarPresetParmVal"
    assert abs( GetParmVal( p1 ) - val ) < 1e-9, "storing a preset value changed the Parm"

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] setting_id string Var Preset Setting ID
    \param [in] parm_id string Var Parm ID
    \return double Var Preset Parm value
*/

extern double GetVarPresetParmVal( const std::string &group_id, const std::string &setting_id, const std::string &parm_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Get Variable Preset group name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string name = GetGroupName( gid );
    if ( name.length() == 0 )
    {
        Print( "ERROR: GetGroupName returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    name = GetGroupName( gid )
    assert len( name ) > 0, "GetGroupName returned nothing"

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \return string Var Preset Group name
*/

extern std::string GetGroupName( const std::string &group_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Get Variable Preset Setting name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string name = GetSettingName( sid );
    if ( name.length() == 0 )
    {
        Print( "ERROR: GetSettingName returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    name = GetSettingName( sid )
    assert len( name ) > 0, "GetSettingName returned nothing"

    \endcode
    \endPythonOnly
    \param [in] setting_id string Var Preset Setting ID
    \return string Var Preset Setting name
*/

extern std::string GetSettingName( const std::string &setting_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Set Variable Preset group name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    SetGroupName( gid, "Resolution" );
    if ( GetGroupName( gid ) != "Resolution" )
    {
        Print( "ERROR: SetGroupName did not take" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    SetGroupName( gid, "Resolution" )
    assert GetGroupName( gid ) == "Resolution", "SetGroupName did not take"


    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] group_name string New Var Preset Group name
*/

extern void SetGroupName( const std::string &group_id, const std::string &group_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Set Variable Preset Setting name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    SetSettingName( sid, "Low" );
    if ( GetSettingName( sid ) != "Low" )
    {
        Print( "ERROR: SetSettingName did not take" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    SetSettingName( sid, "Low" )
    assert GetSettingName( sid ) == "Low", "SetSettingName did not take"


    \endcode
    \endPythonOnly
    \param [in] setting_id string Var Preset Setting ID
    \param [in] setting_name string New Var Preset Setting name
*/

extern void SetSettingName( const std::string &setting_id, const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Get group_ids for Variable Preset Groups
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    array <string> group_ids = GetVarPresetGroups();
    if ( group_ids.length() == 0 )
    {
        Print( "ERROR: GetVarPresetGroups returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    group_ids = GetVarPresetGroups()
    assert len( group_ids ) > 0, "GetVarPresetGroups returned nothing"

    \endcode
    \endPythonOnly
    \return array<string> Array of Variable Preset Group IDs
*/

extern std::vector< std::string > GetVarPresetGroups();

/*!
    \ingroup VariablePreset
*/
/*!
    Get Setting IDs for Variable Preset Group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    array <string> settingids = GetVarPresetSettings( gid );
    if ( settingids.length() == 0 )
    {
        Print( "ERROR: GetVarPresetSettings returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    settingds = GetVarPresetSettings( gid )
    assert len( settingds ) > 0, "GetVarPresetSettings returned nothing"

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \return array<string> Array of Variable Preset Group ParmIDs
*/

extern std::vector< std::string > GetVarPresetSettings( const std::string &group_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Get ParmIDs for Variable Preset Group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    array <string> parmids = GetVarPresetParmIDs( gid );
    if ( parmids.length() == 0 )
    {
        Print( "ERROR: GetVarPresetParmIDs returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    parmids = GetVarPresetParmIDs( gid )
    assert len( parmids ) > 0, "GetVarPresetParmIDs returned nothing"

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \return array<string> Array of Variable Preset Group ParmIDs
*/

extern std::vector< std::string > GetVarPresetParmIDs( const std::string &group_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Get Parm values for Variable Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    array < double > parmval_vec = GetVarPresetParmVals( sid );
    if ( parmval_vec.length() == 0 )
    {
        Print( "ERROR: GetVarPresetParmVals returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    parmval_vec = GetVarPresetParmVals( sid )
    assert len( parmval_vec ) > 0, "GetVarPresetParmVals returned nothing"

    \endcode
    \endPythonOnly
    \param [in] setting_id string Var Preset Setting ID
    \return array<double> Var Preset Parm values for Setting

*/

extern std::vector< double > GetVarPresetParmVals( const std::string &setting_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Set Parm values for Variable Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    array <double> vals = { 45 };

    SetVarPresetParmVals( sid, vals );

    if ( GetVarPresetParmVals( sid ) != vals )
    {
        Print( "ERROR: SetVarPresetParmVals did not take" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    vals = [ 45 ]

    SetVarPresetParmVals( sid, vals )

    assert list( GetVarPresetParmVals( sid ) ) == list( vals ), "SetVarPresetParmVals did not take"

    \endcode
    \endPythonOnly
    \param [in] setting_id string Var Preset Setting ID
    \param [in] parm_vals array<double> Array of Variable Preset Group Parm values
*/

extern void SetVarPresetParmVals( const std::string &setting_id, const std::vector< double > &parm_vals );

/*!
    \ingroup VariablePreset
*/
/*!
    Save current Parm values to Variable Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    // Move the Parm away from whatever the setting holds, then save.
    SetParmVal( p1, GetParmVal( p1 ) + 4.0 );

    Update();

    SaveVarPresetParmVals( gid, sid );

    // The setting now holds the Parm's current value.
    if ( !closeTo( GetVarPresetParmVal( gid, sid, p1 ), GetParmVal( p1 ), 1e-9 ) )
    {
        Print( "ERROR: SaveVarPresetParmVals did not capture the current value" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    # Move the Parm away from whatever the setting holds, then save.
    SetParmVal( p1, GetParmVal( p1 ) + 4.0 )

    Update()

    SaveVarPresetParmVals( gid, sid )

    # The setting now holds the Parm's current value.
    assert abs( GetVarPresetParmVal( gid, sid, p1 ) - GetParmVal( p1 ) ) < 1e-9, "SaveVarPresetParmVals did not capture the current value"

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] setting_id string Var Preset Setting ID
*/

extern void SaveVarPresetParmVals( const std::string &group_id, const std::string &setting_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Apply Parm values for Var Preset Setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    string gid = AddVarPresetGroup( "Tess" );

    string sid = AddVarPresetSetting( gid, "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( gid, p1 );

    // Store a value in the setting, move the Parm somewhere else, then apply.
    double target = GetParmVal( p1 ) + 5.0;

    SetVarPresetParmVal( gid, sid, p1, target );

    SetParmVal( p1, GetParmVal( p1 ) - 2.0 );

    Update();

    ApplyVarPresetSetting( gid, sid );

    Update();

    // Applying the setting drives the Parm to the stored value.
    if ( !closeTo( GetParmVal( p1 ), target, 1e-9 ) )
    {
        Print( "ERROR: ApplyVarPresetSetting did not apply the stored value" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    gid = AddVarPresetGroup( "Tess" )

    sid = AddVarPresetSetting( gid, "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( gid, p1 )

    # Store a value in the setting, move the Parm somewhere else, then apply.
    target = GetParmVal( p1 ) + 5.0

    SetVarPresetParmVal( gid, sid, p1, target )

    SetParmVal( p1, GetParmVal( p1 ) - 2.0 )

    Update()

    ApplyVarPresetSetting( gid, sid )

    Update()

    # Applying the setting drives the Parm to the stored value.
    assert abs( GetParmVal( p1 ) - target ) < 1e-9, "ApplyVarPresetSetting did not apply the stored value"

    \endcode
    \endPythonOnly
    \param [in] group_id string Var Preset Group ID
    \param [in] setting_id string Var Preset Setting ID
*/

extern void ApplyVarPresetSetting( const std::string &group_id, const std::string &setting_id );

//======================== Mode Functions ======================//

/*!
    \ingroup Mode
*/
/*!
    Create a Mode -- a combination of Sets and Variable Presets
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    if ( mid1.length() == 0 || mid1 == "NONE" )
    {
        Print( "ERROR: CreateAndAddMode returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    assert len( mid1 ) > 0 and mid1 != "NONE", "CreateAndAddMode returned no id"
    assert mid1 != mid2, "CreateAndAddMode reused an ID"

    \endcode
    \endPythonOnly
    \param [in] name string Name for new Mode
    \param [in] normal_set int Normal set for Mode
    \param [in] degen_set int Degen set for Mode
    \return string Mode ID for new Mode
*/

extern string CreateAndAddMode( const string & name, int normal_set, int degen_set );

/*!
    \ingroup Mode
*/
/*!
    Get number of Modes in model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    int nmod = GetNumModes();

    if ( nmod != 2 )
    {
        Print( "ERROR: GetNumModes, two were created" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    nmod = GetNumModes()

    assert nmod == 2, "GetNumModes, two were created"

    \endcode
    \endPythonOnly
    \return int Number of Modes in model.
*/

extern int GetNumModes();

/*!
    \ingroup Mode
*/
/*!
    Get all ModeID's in model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    array<string> modids = GetAllModes();
    if ( modids.length() == 0 )
    {
        Print( "ERROR: GetAllModes returned nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    modids = GetAllModes();
    assert len( modids ) > 0, "GetAllModes returned nothing"

    \endcode
    \endPythonOnly
    \return array<string> array of Mode IDs
*/

extern vector < string > GetAllModes();

/*!
    \ingroup Mode
*/
/*!
    Delete a mode from the model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    int num_before_del = GetNumModes();
    DelMode( mid1 );
    if ( GetNumModes() >= num_before_del )
    {
        Print( "ERROR: DelMode removed nothing" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    num_before_del = GetNumModes()
    DelMode( mid1 )
    assert GetNumModes() < num_before_del, "DelMode removed nothing"


    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID of mode to delete
*/

extern void DelMode( const string &mid );

/*!
    \ingroup Mode
*/
/*!
    Delete all modes from the model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    DelAllModes();
    if ( GetNumModes() != 0 )
    {
        Print( "ERROR: DelAllModes left something behind" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    DelAllModes()
    assert GetNumModes() == 0, "DelAllModes left something behind"


    \endcode
    \endPythonOnly
*/

extern void DelAllModes();

/*!
    \ingroup Mode
*/
/*!
    Apply Parm settings corresponding to a Mode.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    // Each Mode carries the pairings it was given, in order.
    array<string> gids = ModeGetAllGroups( mid1 );
    array<string> sids = ModeGetAllSettings( mid1 );

    if ( gids.size() != 2 || gids[0] != gid || gids[1] != gid2 ||
         sids.size() != 2 || sids[0] != sid1 || sids[1] != sid4 )
    {
        Print( "ERROR: the Mode did not record the pairings it was given" );
        __failure++;
    }

    if ( ModeGetAllGroups( mid2 ).size() != 2 )
    {
        Print( "ERROR: the second Mode did not record its pairings" );
        __failure++;
    }

    // ThinAero carries the Fine tessellation and the LongThin design, so
    // applying it has to drive all four Parms to those stored values.
    if ( !closeTo( GetParmVal( p1 ), 35, 1e-9 ) || !closeTo( GetParmVal( p2 ), 21, 1e-9 ) )
    {
        Print( "ERROR: the tessellation group was not applied" );
        __failure++;
    }

    if ( !closeTo( GetParmVal( p3 ), 20.0, 1e-9 ) || !closeTo( GetParmVal( p4 ), 35.0, 1e-9 ) )
    {
        Print( "ERROR: the design group was not applied" );
        __failure++;
    }

    // The other Mode holds different values, so switching has to move them.
    ApplyModeSettings( mid1 );
    Update();

    if ( !closeTo( GetParmVal( p1 ), 3, 1e-9 ) || !closeTo( GetParmVal( p3 ), 3.0, 1e-9 ) )
    {
        Print( "ERROR: switching Modes did not change the Parms" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    # Each Mode carries the pairings it was given, in order.
    gids = ModeGetAllGroups( mid1 )
    sids = ModeGetAllSettings( mid1 )

    assert len( gids ) == 2 and gids[0] == gid and gids[1] == gid2, "the Mode did not record the pairings it was given"
    assert len( sids ) == 2 and sids[0] == sid1 and sids[1] == sid4, "the Mode did not record the pairings it was given"
    assert len( ModeGetAllGroups( mid2 ) ) == 2, "the second Mode did not record its pairings"

    # ThinAero carries the Fine tessellation and the LongThin design, so applying
    # it has to drive all four Parms to those stored values.
    assert abs( GetParmVal( p1 ) - 35 ) < 1e-9, "the tessellation group was not applied"
    assert abs( GetParmVal( p2 ) - 21 ) < 1e-9, "the tessellation group was not applied"
    assert abs( GetParmVal( p3 ) - 20.0 ) < 1e-9, "the design group was not applied"
    assert abs( GetParmVal( p4 ) - 35.0 ) < 1e-9, "the design group was not applied"

    # The other Mode holds different values, so switching has to move them.
    ApplyModeSettings( mid1 )
    Update()

    assert abs( GetParmVal( p1 ) - 3 ) < 1e-9, "switching Modes did not change the Parms"
    assert abs( GetParmVal( p3 ) - 3.0 ) < 1e-9, "switching Modes did not change the Parms"

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID of mode to apply
*/

extern void ApplyModeSettings( const string &mid );

/*!
    \ingroup Mode
*/
/*!
    Show-only a mode in a model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    ShowOnlyMode( mid1 );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    ShowOnlyMode( mid1 )

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID of mode to show-only
*/

extern void ShowOnlyMode( const string &mid );

/*!
    \ingroup Mode
*/
/*!
    Add a variable preset group and setting to a mode.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    // Each Mode carries the pairings it was given, in order.
    array<string> gids = ModeGetAllGroups( mid1 );
    array<string> sids = ModeGetAllSettings( mid1 );

    if ( gids.size() != 2 || gids[0] != gid || gids[1] != gid2 ||
         sids.size() != 2 || sids[0] != sid1 || sids[1] != sid4 )
    {
        Print( "ERROR: the Mode did not record the pairings it was given" );
        __failure++;
    }

    if ( ModeGetAllGroups( mid2 ).size() != 2 )
    {
        Print( "ERROR: the second Mode did not record its pairings" );
        __failure++;
    }

    // ThinAero carries the Fine tessellation and the LongThin design, so
    // applying it has to drive all four Parms to those stored values.
    if ( !closeTo( GetParmVal( p1 ), 35, 1e-9 ) || !closeTo( GetParmVal( p2 ), 21, 1e-9 ) )
    {
        Print( "ERROR: the tessellation group was not applied" );
        __failure++;
    }

    if ( !closeTo( GetParmVal( p3 ), 20.0, 1e-9 ) || !closeTo( GetParmVal( p4 ), 35.0, 1e-9 ) )
    {
        Print( "ERROR: the design group was not applied" );
        __failure++;
    }

    // The other Mode holds different values, so switching has to move them.
    ApplyModeSettings( mid1 );
    Update();

    if ( !closeTo( GetParmVal( p1 ), 3, 1e-9 ) || !closeTo( GetParmVal( p3 ), 3.0, 1e-9 ) )
    {
        Print( "ERROR: switching Modes did not change the Parms" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    # Each Mode carries the pairings it was given, in order.
    gids = ModeGetAllGroups( mid1 )
    sids = ModeGetAllSettings( mid1 )

    assert len( gids ) == 2 and gids[0] == gid and gids[1] == gid2, "the Mode did not record the pairings it was given"
    assert len( sids ) == 2 and sids[0] == sid1 and sids[1] == sid4, "the Mode did not record the pairings it was given"
    assert len( ModeGetAllGroups( mid2 ) ) == 2, "the second Mode did not record its pairings"

    # ThinAero carries the Fine tessellation and the LongThin design, so applying
    # it has to drive all four Parms to those stored values.
    assert abs( GetParmVal( p1 ) - 35 ) < 1e-9, "the tessellation group was not applied"
    assert abs( GetParmVal( p2 ) - 21 ) < 1e-9, "the tessellation group was not applied"
    assert abs( GetParmVal( p3 ) - 20.0 ) < 1e-9, "the design group was not applied"
    assert abs( GetParmVal( p4 ) - 35.0 ) < 1e-9, "the design group was not applied"

    # The other Mode holds different values, so switching has to move them.
    ApplyModeSettings( mid1 )
    Update()

    assert abs( GetParmVal( p1 ) - 3 ) < 1e-9, "switching Modes did not change the Parms"
    assert abs( GetParmVal( p3 ) - 3.0 ) < 1e-9, "switching Modes did not change the Parms"

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to add variable preset to
    \param [in] gid string Variable preset group ID to add to mode
    \param [in] sid string Variable preset setting ID to add to mode
*/

extern void ModeAddGroupSetting( const string &mid, const string &gid, const string &sid );

/*!
    \ingroup Mode
*/
/*!
    Get the group ID of var preset indx from a mode.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    string gid3 = ModeGetGroup( mid1, 0 );
    if ( gid3.length() == 0 )
    {
        Print( "ERROR: ModeGetGroup returned no id" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    gid3 = ModeGetGroup( mid1, 0 )
    assert len( gid3 ) > 0, "ModeGetGroup returned no id"


    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to return GroupID
    \param [in] indx int Index of Variable preset to return GroupID
    \return string Group ID for Mode Variable preset indx
*/

extern string ModeGetGroup( const string &mid, int indx );

/*!
    \ingroup Mode
*/
/*!
    Get the setting ID of var preset indx from a mode.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    string sid6 = ModeGetSetting( mid1, 0 );
    if ( sid6.length() == 0 )
    {
        Print( "ERROR: ModeGetSetting returned no id" );
        __failure++;
    }


    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    sid6 = ModeGetSetting( mid1, 0 )
    assert len( sid6 ) > 0, "ModeGetSetting returned no id"


    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to return settingID
    \param [in] indx int Index of Variable preset to return SettingID
    \return string Setting ID for Mode Variable preset indx
*/

extern string ModeGetSetting( const string &mid, int indx );

/*!
    \ingroup Mode
*/
/*!
    Get all var preset group IDs in model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    array<string> gids = ModeGetAllGroups( mid1 );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    gids = ModeGetAllGroups( mid1 )

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to return all group IDs
    \return array<string> array of Group IDs
*/

extern vector < string >  ModeGetAllGroups( const string &mid );

/*!
    \ingroup Mode
*/
/*!
    Get all var preset setting IDs in model.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    array<string> sids = ModeGetAllSettings( mid1 );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    sids = ModeGetAllSettings( mid1 )

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to return all group IDs
    \return array<string> array of Group IDs
*/

extern vector < string >  ModeGetAllSettings( const string &mid );

/*!
    \ingroup Mode
*/
/*!
    Remove the indx'th variable preset group and setting from the specified mode.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    RemoveGroupSetting( mid1, 0 );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    RemoveGroupSetting( mid1, 0 )

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to remove varible preset group and setting from
    \param [in] indx int Index of Variable preset to remove
*/

extern void RemoveGroupSetting( const string &mid, int indx );

/*!
    \ingroup Mode
*/
/*!
    Remove all variable preset groups and settings from mode.
    \forcpponly
    \code{.cpp}
    // Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    //
    // Setup boiler plate.
    string pod1 = AddGeom( "POD", "" );
    string wing = AddGeom( "WING", pod1 );

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN );
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 );

    SetSetName( SET_FIRST_USER, "NonLifting" );
    SetSetName( SET_FIRST_USER + 1, "Lifting" );

    SetSetFlag( pod1, SET_FIRST_USER, true );
    SetSetFlag( wing, SET_FIRST_USER + 1, true );


    string gid = AddVarPresetGroup( "Tess" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );
    AddVarPresetParm( gid, p1 );

    string p2 = FindParm( pod1, "Tess_W", "Shape" );
    AddVarPresetParm( gid, p2 );

    string sid = AddVarPresetSetting( gid, "Default" );
    SaveVarPresetParmVals( gid, sid );

    string sid1 = AddVarPresetSetting( gid, "Coarse" );
    SetVarPresetParmVal( gid, sid1, p1, 3 );
    SetVarPresetParmVal( gid, sid1, p2, 5 );

    string sid2 = AddVarPresetSetting( gid, "Fine" );
    SetVarPresetParmVal( gid, sid2, p1, 35 );
    SetVarPresetParmVal( gid, sid2, p2, 21 );


    string gid2 = AddVarPresetGroup( "Design" );

    string p3 = FindParm( pod1, "Length", "Design" );
    AddVarPresetParm( gid2, p3 );

    string p4 = FindParm( pod1, "FineRatio", "Design" );
    AddVarPresetParm( gid2, p4 );

    string sid3 = AddVarPresetSetting( gid2, "Normal" );
    SaveVarPresetParmVals( gid2, sid3 );

    string sid4 = AddVarPresetSetting( gid2, "ShortFat" );
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 );
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 );

    string sid5 = AddVarPresetSetting( gid2, "LongThin" );
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 );
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 );

    // End of setup boiler plate.

    string mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE );
    ModeAddGroupSetting( mid1, gid, sid1 );
    ModeAddGroupSetting( mid1, gid2, sid4 );

    string mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 );
    ModeAddGroupSetting( mid2, gid, sid2 );
    ModeAddGroupSetting( mid2, gid2, sid5 );

    ApplyModeSettings( mid2 );
    Update();

    RemoveAllGroupSettings( mid1 );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Illustrating use of Modes requires substantial setup of the model including components, sets, and variable presets.
    #
    # Setup boiler plate.
    pod1 = AddGeom( "POD", "" )
    wing = AddGeom( "WING", pod1 )

    SetParmVal( wing, "Trans_Attach_Flag", "Attach", ATTACH_TRANS_LMN )
    SetParmVal( wing, "L_Attach_Location", "Attach", 0.35 )

    SetSetName( SET_FIRST_USER, "NonLifting" )
    SetSetName( SET_FIRST_USER + 1, "Lifting" )

    SetSetFlag( pod1, SET_FIRST_USER, True )
    SetSetFlag( wing, SET_FIRST_USER + 1, True )


    gid = AddVarPresetGroup( "Tess" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )
    AddVarPresetParm( gid, p1 )

    p2 = FindParm( pod1, "Tess_W", "Shape" )
    AddVarPresetParm( gid, p2 )

    sid = AddVarPresetSetting( gid, "Default" )
    SaveVarPresetParmVals( gid, sid )

    sid1 = AddVarPresetSetting( gid, "Coarse" )
    SetVarPresetParmVal( gid, sid1, p1, 3 )
    SetVarPresetParmVal( gid, sid1, p2, 5 )

    sid2 = AddVarPresetSetting( gid, "Fine" )
    SetVarPresetParmVal( gid, sid2, p1, 35 )
    SetVarPresetParmVal( gid, sid2, p2, 21 )


    gid2 = AddVarPresetGroup( "Design" )

    p3 = FindParm( pod1, "Length", "Design" )
    AddVarPresetParm( gid2, p3 )

    p4 = FindParm( pod1, "FineRatio", "Design" )
    AddVarPresetParm( gid2, p4 )

    sid3 = AddVarPresetSetting( gid2, "Normal" )
    SaveVarPresetParmVals( gid2, sid3 )

    sid4 = AddVarPresetSetting( gid2, "ShortFat" )
    SetVarPresetParmVal( gid2, sid4, p3, 3.0 )
    SetVarPresetParmVal( gid2, sid4, p4, 5.0 )

    sid5 = AddVarPresetSetting( gid2, "LongThin" )
    SetVarPresetParmVal( gid2, sid5, p3, 20.0 )
    SetVarPresetParmVal( gid2, sid5, p4, 35.0 )

    # End of setup boiler plate.

    mid1 = CreateAndAddMode( "FatWetAreas", SET_ALL, SET_NONE )
    ModeAddGroupSetting( mid1, gid, sid1 )
    ModeAddGroupSetting( mid1, gid2, sid4 )

    mid2 = CreateAndAddMode( "ThinAero", SET_FIRST_USER, SET_FIRST_USER + 1 )
    ModeAddGroupSetting( mid2, gid, sid2 )
    ModeAddGroupSetting( mid2, gid2, sid5 )

    ApplyModeSettings( mid2 )
    Update()

    RemoveAllGroupSettings( mid1 )

    \endcode
    \endPythonOnly
    \param [in] mid string Mode ID to remove all variable presets from
*/

extern void RemoveAllGroupSettings( const string &mid );

//======================== Parametric Curve Functions ======================//
/*!
    \ingroup PCurve
*/
/*!
    Set the parameters, values, and curve type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \param [in] tvec vector \<double\> Array of parameter values
    \param [in] valvec vector \<double\> Array of values
    \param [in] newtype int Curve type enum (i.e. CEDIT)
*/

extern void SetPCurve( const std::string & geom_id, const int & pcurveid, const std::vector < double > & tvec,

    const std::vector < double > & valvec, const int & newtype );
/*!
    \ingroup PCurve
*/
/*!
    Change the type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \param [in] newtype int Curve type enum (i.e. CEDIT)
*/

extern void PCurveConvertTo( const std::string & geom_id, const int & pcurveid, const int & newtype );

/*!
    \ingroup PCurve
*/
/*!
    Get the type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \return int Curve type enum (i.e. CEDIT)
*/

extern int PCurveGetType( const std::string & geom_id, const int & pcurveid );

/*!
    \ingroup PCurve
*/
/*!
    Get the parameters of a propeller blade curve (P Curve). Each parameter is a fraction of propeller radius.
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \return vector \<double\> Array of parameters
*/

extern std::vector < double > PCurveGetTVec( const std::string & geom_id, const int & pcurveid );

/*!
    \ingroup PCurve
*/
/*!
    Get the values of a propeller blade curve (P Curve). What the values represent id dependent on the curve type (i.e. twist, chord, etc.).
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \return vector \<double\> Array of values
*/

extern std::vector < double > PCurveGetValVec( const std::string & geom_id, const int & pcurveid );

/*!
    \ingroup PCurve
*/
/*!
    Delete a propeller blade curve (P Curve) point
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \param [in] indx int Point index
*/

extern void PCurveDeletePt( const std::string & geom_id, const int & pcurveid, const int & indx );

/*!
    \ingroup PCurve
*/
/*!
    Split a propeller blade curve (P Curve) at the specified 1D parameter
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid int P Curve index
    \param [in] tsplit double 1D parameter split location
    \return int Index of new control point
*/

extern int PCurveSplit( const std::string & geom_id, const int & pcurveid, const double & tsplit );

/*!
    \ingroup PCurve
*/
/*!
    Approximate all propeller blade curves with cubic Bezier curves.
    \forcpponly
    \code{.cpp}
    // Add Propeller
    string prop = AddGeom( "PROP", "" );

    // Take the blade curves off Bezier so the approximation has something to do.
    PCurveConvertTo( prop, PROP_CHORD, PCHIP );
    PCurveConvertTo( prop, PROP_TWIST, PCHIP );
    PCurveConvertTo( prop, PROP_THICK, PCHIP );

    Update();

    if ( PCurveGetType( prop, PROP_CHORD ) != PCHIP )
    {
        Print( "ERROR: the blade curves would not convert to PCHIP" );
        __failure++;
    }

    ApproximateAllPropellerPCurves( prop );

    // Every blade curve is now a cubic Bezier.
    if ( PCurveGetType( prop, PROP_CHORD ) != CEDIT ||
         PCurveGetType( prop, PROP_TWIST ) != CEDIT ||
         PCurveGetType( prop, PROP_THICK ) != CEDIT )
    {
        Print( "ERROR: ApproximateAllPropellerPCurves did not convert the curves" );
        __failure++;
    }

    // A cubic Bezier is stored in groups of three, so the control point count
    // is one more than a multiple of three.
    array< double > tvec = PCurveGetTVec( prop, PROP_CHORD );

    if ( tvec.size() < 4 || ( tvec.size() - 1 ) % 3 != 0 )
    {
        Print( "ERROR: the converted curve is not a cubic Bezier" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Propeller
    prop = AddGeom( "PROP", "" )

    # Take the blade curves off Bezier so the approximation has something to do.
    PCurveConvertTo( prop, PROP_CHORD, PCHIP )
    PCurveConvertTo( prop, PROP_TWIST, PCHIP )
    PCurveConvertTo( prop, PROP_THICK, PCHIP )

    Update()

    assert PCurveGetType( prop, PROP_CHORD ) == PCHIP, "the blade curves would not convert to PCHIP"

    ApproximateAllPropellerPCurves( prop )

    # Every blade curve is now a cubic Bezier.
    assert PCurveGetType( prop, PROP_CHORD ) == CEDIT, "ApproximateAllPropellerPCurves did not convert the curves"
    assert PCurveGetType( prop, PROP_TWIST ) == CEDIT, "ApproximateAllPropellerPCurves did not convert the curves"
    assert PCurveGetType( prop, PROP_THICK ) == CEDIT, "ApproximateAllPropellerPCurves did not convert the curves"

    # A cubic Bezier is stored in groups of three, so the control point count is
    # one more than a multiple of three.
    tvec = PCurveGetTVec( prop, PROP_CHORD )

    assert len( tvec ) >= 4 and ( len( tvec ) - 1 ) % 3 == 0, "the converted curve is not a cubic Bezier"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    */

extern void ApproximateAllPropellerPCurves( const std::string & geom_id );

/*!
    \ingroup PCurve
*/
/*!
    Reset propeller T/C curve to match basic thickness of file-type airfoils.  Typically only used for a propeller that
    has been constructed with file-type airfoils across the blade.  The new thickness curve will be a PCHIP curve
    with t/c matching the propeller's XSecs -- unless it is a file XSec, then the Base thickness is used.
    \forcpponly
    \code{.cpp}
    // Add Propeller
    string prop = AddGeom( "PROP", "" );

    ResetPropellerThicknessCurve( prop );

    Update();

    // The curve is rebuilt from the blade XSecs: one PCHIP station per XSec,
    // each carrying that XSec's thickness to chord ratio.
    array< double > tvec = PCurveGetTVec( prop, PROP_THICK );
    array< double > vvec = PCurveGetValVec( prop, PROP_THICK );

    string xsec_surf = GetXSecSurf( prop, 0 );

    int num_xsec = GetNumXSec( xsec_surf );

    if ( PCurveGetType( prop, PROP_THICK ) != PCHIP )
    {
        Print( "ERROR: ResetPropellerThicknessCurve did not make a PCHIP curve" );
        __failure++;
    }

    if ( int( vvec.size() ) != num_xsec || tvec.size() != vvec.size() )
    {
        Print( "ERROR: ResetPropellerThicknessCurve did not follow the XSecs" );
        __failure++;
    }
    else
    {
        for ( int i = 0; i < num_xsec; i++ )
        {
            string tc = GetXSecParm( GetXSec( xsec_surf, i ), "ThickChord" );

            // A circular XSec carries no thickness Parm; its base thickness is
            // used instead.
            if ( tc.length() > 0 )
            {
                if ( !closeTo( vvec[i], GetParmVal( tc ), 1e-6 ) )
                {
                    Print( "ERROR: station " + i + " does not match its XSec" );
                    __failure++;
                }
            }
        }

        // The stations run out along the blade.
        for ( int i = 1; i < int( tvec.size() ); i++ )
        {
            if ( tvec[i] <= tvec[i - 1] )
            {
                Print( "ERROR: the thickness stations are not increasing" );
                __failure++;
            }
        }
    }

    // Looking up the circular XSec's thickness raises an error on purpose, so
    // take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Propeller
    prop = AddGeom( "PROP", "" )

    ResetPropellerThicknessCurve( prop )

    Update()

    # The curve is rebuilt from the blade XSecs: one PCHIP station per XSec, each
    # carrying that XSec's thickness to chord ratio.
    tvec = PCurveGetTVec( prop, PROP_THICK )
    vvec = PCurveGetValVec( prop, PROP_THICK )

    xsec_surf = GetXSecSurf( prop, 0 )

    num_xsec = GetNumXSec( xsec_surf )

    assert PCurveGetType( prop, PROP_THICK ) == PCHIP, "ResetPropellerThicknessCurve did not make a PCHIP curve"
    assert len( vvec ) == num_xsec, "ResetPropellerThicknessCurve did not follow the XSecs"
    assert len( tvec ) == len( vvec ), "ResetPropellerThicknessCurve did not follow the XSecs"

    for i in range( num_xsec ):
        tc = GetXSecParm( GetXSec( xsec_surf, i ), "ThickChord" )

        # A circular XSec carries no thickness Parm; its base thickness is used
        # instead.
        if len( tc ) > 0:
            assert abs( vvec[i] - GetParmVal( tc ) ) < 1e-6, "station " + str( i ) + " does not match its XSec"

    # The stations run out along the blade.
    for i in range( 1, len( tvec ) ):
        assert tvec[i] > tvec[i - 1], "the thickness stations are not increasing"

    # Looking up the circular XSec's thickness raises an error on purpose, so take
    # it back off the queue.
    err_mgr = ErrorMgrSingleton.getInstance()

    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    */

extern void ResetPropellerThicknessCurve( const std::string & geom_id );


//======================== VSPAERO Functions ======================//
/*!
    \ingroup CSGroup
*/
/*!
    Creates the initial default grouping for the control surfaces.
    The initial grouping collects all surface copies of the sub-surface into a single group.
    For example if a wing is defined with an aileron and that wing is symmetrical about the
    xz plane there will be a surface copy of the master wing surface as well as a copy of
    the sub-surface. The two sub-surfaces may get deflected differently during analysis
    routines and can be identified uniquely by their full name.
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    //==== Add Vertical tail and set some parameters =====//
    string vert_id = AddGeom( "WING" );

    SetGeomName( vert_id, "Vert" );

    SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 );
    SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 );
    SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 );

    string rudder_id = AddSubSurf( vert_id, SS_CONTROL );                      // Add Control Surface Sub-Surface

    AutoGroupVSPAEROControlSurfaces();

    Update();

    Print( "COMPLETE\n" );
    string control_group_settings_container_id = FindContainer( "VSPAEROSettings", 0 );   // auto grouping produces parm containers within VSPAEROSettings

    //==== Set Control Surface Group Deflection Angle ====//
    Print( "\tSetting control surface group deflection angles..." );

    //  setup asymmetric deflection for aileron
    string deflection_gain_id;

    // subsurfaces get added to groups with "CSGQualities_[geom_name]_[control_surf_name]"
    // subsurfaces gain parm name is "Surf[surfndx]_Gain" starting from 0 to NumSymmetricCopies-1

    deflection_gain_id = FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_0_Gain", "ControlSurfaceGroup_0" );
    deflection_gain_id = FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_1_Gain", "ControlSurfaceGroup_0" );

    //  deflect aileron
    string deflection_angle_id = FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0" );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    #==== Add Vertical tail and set some parameters =====//
    vert_id = AddGeom( "WING" )

    SetGeomName( vert_id, "Vert" )

    SetParmValUpdate( vert_id, "TotalArea", "WingGeom", 10.0 )
    SetParmValUpdate( vert_id, "X_Rel_Location", "XForm", 8.5 )
    SetParmValUpdate( vert_id, "X_Rel_Rotation", "XForm", 90 )

    rudder_id = AddSubSurf( vert_id, SS_CONTROL )                      # Add Control Surface Sub-Surface

    AutoGroupVSPAEROControlSurfaces()

    Update()

    print( "COMPLETE\n" )
    control_group_settings_container_id = FindContainer( "VSPAEROSettings", 0 )   # auto grouping produces parm containers within VSPAEROSettings

    #==== Set Control Surface Group Deflection Angle ====//
    print( "\tSetting control surface group deflection angles..." )

    # subsurfaces get added to groups with "CSGQualities_[geom_name]_[control_surf_name]"
    # subsurfaces gain parm name is "Surf[surfndx]_Gain" starting from 0 to NumSymmetricCopies-1

    deflection_gain_id = FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_0_Gain", "ControlSurfaceGroup_0" )
    deflection_gain_id = FindParm( control_group_settings_container_id, "Surf_" + aileron_id + "_1_Gain", "ControlSurfaceGroup_0" )

    #  deflect aileron
    deflection_angle_id = FindParm( control_group_settings_container_id, "DeflectionAngle", "ControlSurfaceGroup_0" )

    \endcode
    \endPythonOnly
    \sa CreateVSPAEROControlSurfaceGroup
*/

extern void AutoGroupVSPAEROControlSurfaces();

/*!
    \ingroup CSGroup
*/
/*!
    Add a new VSPAERO control surface group using the default naming convention. The control surface group will not contain any
    control surfaces until they are added.
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    int num_group = GetNumControlSurfaceGroups();

    if ( num_group != 1 ) { Print( "Error: CreateVSPAEROControlSurfaceGroup" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    num_group = GetNumControlSurfaceGroups()

    if  num_group != 1 :
        print( "Error: CreateVSPAEROControlSurfaceGroup" )
        assert False, "Error: CreateVSPAEROControlSurfaceGroup"

    \endcode
    \endPythonOnly
    \sa AddSelectedToCSGroup
    \return int Index of the new VSPAERO control surface group
*/

extern int CreateVSPAEROControlSurfaceGroup();

/*!
    \ingroup CSGroup
*/
/*!
    Add all available control surfaces to the control surface group at the specified index
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    // The group starts empty and the wing offers one control surface, mirrored
    // by the wing's own symmetry.
    if ( GetNumControlSurfaceGroups() < 1 )
    {
        Print( "ERROR: CreateVSPAEROControlSurfaceGroup did not add a group" );
        __failure++;
    }

    int num_avail = int( GetAvailableCSNameVec( group_index ).size() );

    if ( num_avail < 1 )
    {
        Print( "ERROR: no control surfaces are available to add" );
        __failure++;
    }

    if ( GetActiveCSNameVec( group_index ).size() != 0 )
    {
        Print( "ERROR: a new control surface group is not empty" );
        __failure++;
    }

    AddAllToVSPAEROControlSurfaceGroup( group_index );

    // Everything that was available is now active, and nothing is left over.
    if ( int( GetActiveCSNameVec( group_index ).size() ) != num_avail )
    {
        Print( "ERROR: AddAllToVSPAEROControlSurfaceGroup did not add them all" );
        __failure++;
    }

    if ( GetAvailableCSNameVec( group_index ).size() != 0 )
    {
        Print( "ERROR: control surfaces are still available after adding them all" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    # The group starts empty and the wing offers one control surface, mirrored
    # by the wing's own symmetry.
    assert GetNumControlSurfaceGroups() >= 1, "CreateVSPAEROControlSurfaceGroup did not add a group"

    num_avail = len( GetAvailableCSNameVec( group_index ) )

    assert num_avail >= 1, "no control surfaces are available to add"
    assert len( GetActiveCSNameVec( group_index ) ) == 0, "a new control surface group is not empty"

    AddAllToVSPAEROControlSurfaceGroup( group_index )

    # Everything that was available is now active, and nothing is left over.
    assert len( GetActiveCSNameVec( group_index ) ) == num_avail, "AddAllToVSPAEROControlSurfaceGroup did not add them all"
    assert len( GetAvailableCSNameVec( group_index ) ) == 0, "control surfaces are still available after adding them all"

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex int Index of the control surface group
*/

extern void AddAllToVSPAEROControlSurfaceGroup( int CSGroupIndex );

/*!
    \ingroup CSGroup
*/
/*!
    Remove all used control surfaces from the control surface group at the specified index
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index );

    int num_active = int( GetActiveCSNameVec( group_index ).size() );

    if ( num_active < 1 )
    {
        Print( "ERROR: nothing was added to the group to remove" );
        __failure++;
    }

    RemoveAllFromVSPAEROControlSurfaceGroup( group_index ); // Empty control surface group

    // Everything that was active goes back to being available.
    if ( GetActiveCSNameVec( group_index ).size() != 0 )
    {
        Print( "ERROR: RemoveAllFromVSPAEROControlSurfaceGroup left surfaces in the group" );
        __failure++;
    }

    if ( int( GetAvailableCSNameVec( group_index ).size() ) != num_active )
    {
        Print( "ERROR: the removed surfaces did not become available again" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index )

    num_active = len( GetActiveCSNameVec( group_index ) )

    assert num_active >= 1, "nothing was added to the group to remove"

    RemoveAllFromVSPAEROControlSurfaceGroup( group_index ) # Empty control surface group

    # Everything that was active goes back to being available.
    assert len( GetActiveCSNameVec( group_index ) ) == 0, "RemoveAllFromVSPAEROControlSurfaceGroup left surfaces in the group"
    assert len( GetAvailableCSNameVec( group_index ) ) == num_active, "the removed surfaces did not become available again"

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex int Index of the control surface group
*/

extern void RemoveAllFromVSPAEROControlSurfaceGroup( int CSGroupIndex );

/*!
    \ingroup CSGroup
*/
/*!
    Get the names of each active (used) control surface in the control surface group at the specified index
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index );

    array<string> @cs_name_vec = GetActiveCSNameVec( group_index );
    if ( cs_name_vec.length() == 0 )
    {
        Print( "ERROR: GetActiveCSNameVec returned nothing" );
        __failure++;
    }

    Print( "Active CS in Group Index #", false );
    Print( group_index );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        Print( cs_name_vec[i] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index )

    cs_name_vec = GetActiveCSNameVec( group_index )
    assert len( cs_name_vec ) > 0, "GetActiveCSNameVec returned nothing"

    print( "Active CS in Group Index #", False )
    print( group_index )

    for i in range(int( len(cs_name_vec) )):

        print( cs_name_vec[i] )

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex int Index of the control surface group
    \return vector \<string\> Array of active control surface names
*/

extern std::vector < std::string > GetActiveCSNameVec( int CSGroupIndex );

/*!
    \ingroup CSGroup
*/
/*!
    Get the names of all control surfaces. Some may be active (used) while others may be available.
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array<string> @cs_name_vec = GetCompleteCSNameVec();
    if ( cs_name_vec.length() == 0 )
    {
        Print( "ERROR: GetCompleteCSNameVec returned nothing" );
        __failure++;
    }

    Print( "All Control Surfaces: ", false );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        Print( cs_name_vec[i] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    cs_name_vec = GetCompleteCSNameVec()
    assert len( cs_name_vec ) > 0, "GetCompleteCSNameVec returned nothing"

    print( "All Control Surfaces: ", False )

    for i in range(int( len(cs_name_vec) )):

        print( cs_name_vec[i] )

    \endcode
    \endPythonOnly
    \return vector \<string\> Array of all control surface names
*/

extern std::vector < std::string > GetCompleteCSNameVec();

/*!
    \ingroup CSGroup
*/
/*!
    Get the names of each available (not used) control surface in the control surface group at the specified index
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array<string> @cs_name_vec = GetAvailableCSNameVec( group_index );
    if ( cs_name_vec.length() == 0 )
    {
        Print( "ERROR: GetAvailableCSNameVec returned nothing" );
        __failure++;
    }

    array < int > cs_ind_vec(1);
    cs_ind_vec[0] = 1;

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add the first available control surface to the group

    // One surface moved from available to active.
    array < string > active_vec = GetActiveCSNameVec( group_index );

    if ( active_vec.size() != 1 || active_vec[0] != cs_name_vec[0] )
    {
        Print( "ERROR: AddSelectedToCSGroup did not add the surface that was named" );
        __failure++;
    }

    if ( GetAvailableCSNameVec( group_index ).size() != cs_name_vec.size() - 1 )
    {
        Print( "ERROR: the added surface is still available" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL ) # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    cs_name_vec = GetAvailableCSNameVec( group_index )
    assert len( cs_name_vec ) > 0, "GetAvailableCSNameVec returned nothing"

    cs_ind_vec = [1]

    AddSelectedToCSGroup( cs_ind_vec, group_index ) # Add the first available control surface to the group

    # One surface moved from available to active.
    active_vec = GetActiveCSNameVec( group_index )

    assert len( active_vec ) == 1, "AddSelectedToCSGroup did not add the surface that was named"
    assert active_vec[0] == cs_name_vec[0], "AddSelectedToCSGroup added the wrong surface"
    assert len( GetAvailableCSNameVec( group_index ) ) == len( cs_name_vec ) - 1, "the added surface is still available"

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex int Index of the control surface group
    \return vector \<string\> Array of active control surface names
*/

extern std::vector < std::string > GetAvailableCSNameVec( int CSGroupIndex );

/*!
    \ingroup CSGroup
*/
/*!
    Set the name for the control surface group at the specified index
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    SetVSPAEROControlGroupName( "Example_CS_Group", group_index );

    if ( GetVSPAEROControlGroupName( group_index ) != "Example_CS_Group" )
    {
        Print( "ERROR: SetVSPAEROControlGroupName did not take" );
        __failure++;
    }

    Print( "CS Group name: ", false );

    Print( GetVSPAEROControlGroupName( group_index ) );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL ) # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    SetVSPAEROControlGroupName( "Example_CS_Group", group_index )

    assert GetVSPAEROControlGroupName( group_index ) == "Example_CS_Group", "SetVSPAEROControlGroupName did not take"

    print( "CS Group name: ", False )

    print( GetVSPAEROControlGroupName( group_index ) )

    \endcode
    \endPythonOnly
    \param [in] name string Name to set for the control surface group
    \param [in] CSGroupIndex int Index of the control surface group
*/

extern void SetVSPAEROControlGroupName(const string & name, int CSGroupIndex);

/*!
    \ingroup CSGroup
*/
/*!
    Get the name of the control surface group at the specified index
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    SetVSPAEROControlGroupName( "Example_CS_Group", group_index );

    Print( "CS Group name: ", false );

    Print( GetVSPAEROControlGroupName( group_index ) );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL ) # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    SetVSPAEROControlGroupName( "Example_CS_Group", group_index )

    assert GetVSPAEROControlGroupName( group_index ) == "Example_CS_Group", "SetVSPAEROControlGroupName did not take"

    print( "CS Group name: ", False )

    print( GetVSPAEROControlGroupName( group_index ) )

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex int Index of the control surface group
*/

extern std::string GetVSPAEROControlGroupName( int CSGroupIndex );

/*!
    \ingroup CSGroup
*/
/*!
    Add each control surfaces in the array of control surface indexes to the control surface group at the specified index.

    \warning The indexes in input "selected" must be matched with available control surfaces identified by GetAvailableCSNameVec.
    The "selected" input uses one- based indexing to associate available control surfaces.

    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array < string > cs_name_vec = GetAvailableCSNameVec( group_index );

    array < int > cs_ind_vec( cs_name_vec.size() );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        cs_ind_vec[i] = i + 1;
    }

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add all available control surfaces to the group
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL ) # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    cs_name_vec = GetAvailableCSNameVec( group_index )

    cs_ind_vec = [0] * len(cs_name_vec)

    for i in range(int( len(cs_name_vec) )):

        cs_ind_vec[i] = i + 1

    AddSelectedToCSGroup( cs_ind_vec, group_index ) # Add all available control surfaces to the group

    \endcode
    \endPythonOnly
    \sa GetAvailableCSNameVec
    \param [in] selected vector \<int\> Array of control surface indexes to add to the group. Note, the integer values are one based.
    \param [in] CSGroupIndex int Index of the control surface group
*/

extern void AddSelectedToCSGroup( const vector <int> &selected, int CSGroupIndex);

/*!
    \ingroup CSGroup
*/
/*!
    Remove each control surfaces in the array of control surface indexes from the control surface group at the specified index.

    \warning The indexes in input "selected" must be matched with active control surfaces identified by GetActiveCSNameVec. The
    "selected" input uses one-based indexing to associate available control surfaces.

    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" ); // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL ); // Add Control Surface Sub-Surface

    int group_index = CreateVSPAEROControlSurfaceGroup(); // Empty control surface group

    array < string > cs_name_vec = GetAvailableCSNameVec( group_index );

    array < int > cs_ind_vec( cs_name_vec.size() );

    for ( int i = 0; i < int( cs_name_vec.size() ); i++ )
    {
        cs_ind_vec[i] = i + 1;
    }

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add the available control surfaces to the group

    array < int > remove_cs_ind_vec( 1 );
    remove_cs_ind_vec[0] = 1;

    int num_active = int( GetActiveCSNameVec( group_index ).size() );

    if ( num_active != int( cs_name_vec.size() ) )
    {
        Print( "ERROR: not everything was added to the group" );
        __failure++;
    }

    RemoveSelectedFromCSGroup( remove_cs_ind_vec, group_index ); // Remove the first control surface

    // One surface moved back from active to available.
    if ( int( GetActiveCSNameVec( group_index ).size() ) != num_active - 1 )
    {
        Print( "ERROR: RemoveSelectedFromCSGroup did not remove a surface" );
        __failure++;
    }

    if ( GetAvailableCSNameVec( group_index ).size() != 1 )
    {
        Print( "ERROR: the removed surface did not become available again" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL ) # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    cs_name_vec = GetAvailableCSNameVec( group_index )

    cs_ind_vec = [0] * len(cs_name_vec)

    for i in range(int( len(cs_name_vec) )):

        cs_ind_vec[i] = i + 1

    AddSelectedToCSGroup( cs_ind_vec, group_index ) # Add the available control surfaces to the group

    remove_cs_ind_vec = [1]

    num_active = len( GetActiveCSNameVec( group_index ) )

    assert num_active == len( cs_name_vec ), "not everything was added to the group"

    RemoveSelectedFromCSGroup( remove_cs_ind_vec, group_index ) # Remove the first control surface

    # One surface moved back from active to available.
    assert len( GetActiveCSNameVec( group_index ) ) == num_active - 1, "RemoveSelectedFromCSGroup did not remove a surface"
    assert len( GetAvailableCSNameVec( group_index ) ) == 1, "the removed surface did not become available again"

    \endcode
    \endPythonOnly
    \sa GetActiveCSNameVec
    \param [in] selected vector \<int\> Array of control surface indexes to remove from the group. Note, the integer values are one based.
    \param [in] CSGroupIndex int Index of the control surface group
*/

extern void RemoveSelectedFromCSGroup( const vector <int> &selected, int CSGroupIndex);

/*!
    \ingroup CSGroup
*/
/*!
    Get the total number of control surface groups
    \forcpponly
    \code{.cpp}
    string wid = AddGeom( "WING", "" );                             // Add Wing

    string aileron_id = AddSubSurf( wid, SS_CONTROL );                      // Add Control Surface Sub-Surface

    //==== Add Horizontal tail and set some parameters =====//
    string horiz_id = AddGeom( "WING", "" );

    SetGeomName( horiz_id, "Vert" );

    SetParmValUpdate( horiz_id, "TotalArea", "WingGeom", 10.0 );
    SetParmValUpdate( horiz_id, "X_Rel_Location", "XForm", 8.5 );

    string elevator_id = AddSubSurf( horiz_id, SS_CONTROL );                      // Add Control Surface Sub-Surface

    AutoGroupVSPAEROControlSurfaces();

    int num_group = GetNumControlSurfaceGroups();

    if ( num_group != 2 ) { Print( "Error: GetNumControlSurfaceGroups" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    #==== Add Horizontal tail and set some parameters =====//
    horiz_id = AddGeom( "WING", "" )

    SetGeomName( horiz_id, "Vert" )

    SetParmValUpdate( horiz_id, "TotalArea", "WingGeom", 10.0 )
    SetParmValUpdate( horiz_id, "X_Rel_Location", "XForm", 8.5 )

    elevator_id = AddSubSurf( horiz_id, SS_CONTROL )                      # Add Control Surface Sub-Surface

    AutoGroupVSPAEROControlSurfaces()

    num_group = GetNumControlSurfaceGroups()

    if  num_group != 2 :
        print( "Error: GetNumControlSurfaceGroups" )
        assert False, "Error: GetNumControlSurfaceGroups"

    \endcode
    \endPythonOnly
    \return int Number of control surface groups
*/

extern int GetNumControlSurfaceGroups();


//================ VSPAERO Actuator Disk and Unsteady Functions ==============//
/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get the ID of a VSPAERO actuator disk at the specified index. An empty string is returned if
    the index is out of range.
    \forcpponly
    \code{.cpp}
    // Add a propeller
    string prop_id = AddGeom( "PROP", "" );
    SetParmVal( prop_id, "PropMode", "Design", PROP_DISK );
    SetParmVal( prop_id, "Diameter", "Design", 6.0 );

    Update();

    // Setup the actuator disk VSPAERO parms
    string disk_id = FindActuatorDisk( 0 );
    if ( disk_id.length() == 0 )
    {
        Print( "ERROR: FindActuatorDisk found nothing" );
        __failure++;
    }

    SetParmVal( FindParm( disk_id, "RotorRPM", "Rotor" ), 1234.0 );
    SetParmVal( FindParm( disk_id, "RotorCT", "Rotor" ), 0.35 );
    SetParmVal( FindParm( disk_id, "RotorCP", "Rotor" ), 0.55 );
    SetParmVal( FindParm( disk_id, "RotorHubDiameter", "Rotor" ), 1.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add a propeller
    prop_id = AddGeom( "PROP", "" )
    SetParmVal( prop_id, "PropMode", "Design", PROP_DISK )
    SetParmVal( prop_id, "Diameter", "Design", 6.0 )

    Update()

    # Setup the actuator disk VSPAERO parms
    disk_id = FindActuatorDisk( 0 )
    assert len( disk_id ) > 0, "FindActuatorDisk found nothing"

    SetParmVal( FindParm( disk_id, "RotorRPM", "Rotor" ), 1234.0 )
    SetParmVal( FindParm( disk_id, "RotorCT", "Rotor" ), 0.35 )
    SetParmVal( FindParm( disk_id, "RotorCP", "Rotor" ), 0.55 )
    SetParmVal( FindParm( disk_id, "RotorHubDiameter", "Rotor" ), 1.0 )

    \endcode
    \endPythonOnly
    \sa PROP_MODE
    \param [in] disk_index int Actuator disk index for the current VSPAERO set
    \return string Actuator disk ID
*/

extern std::string FindActuatorDisk( int disk_index );

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get the number of actuator disks in the current VSPAERO set. This is equivalent to the number of disk surfaces in the VSPAERO set.
    \forcpponly
    \code{.cpp}
    // Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL );

    // Add a propeller
    string prop_id = AddGeom( "PROP", "" );
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    int num_disk = GetNumActuatorDisks(); // Should be 0

    if ( num_disk != 0 )
    {
        Print( "ERROR: a bladed propeller counts as an actuator disk" );
        __failure++;
    }

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    num_disk = GetNumActuatorDisks(); // Should be 1

    if ( num_disk != 1 )
    {
        Print( "ERROR: GetNumActuatorDisks did not count the disk" );
        __failure++;
    }

    // The disk has to be findable at every index it claims.
    for ( int i = 0; i < num_disk; i++ )
    {
        if ( FindActuatorDisk( i ).length() == 0 )
        {
            Print( "ERROR: GetNumActuatorDisks counted a disk that cannot be found" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL )

    # Add a propeller
    prop_id = AddGeom( "PROP", "" )
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES )

    num_disk = GetNumActuatorDisks() # Should be 0

    assert num_disk == 0, "a bladed propeller counts as an actuator disk"

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK )

    num_disk = GetNumActuatorDisks() # Should be 1

    assert num_disk == 1, "GetNumActuatorDisks did not count the disk"

    # The disk has to be findable at every index it claims.
    for i in range( num_disk ):
        assert len( FindActuatorDisk( i ) ) > 0, "GetNumActuatorDisks counted a disk that cannot be found"

    \endcode
    \endPythonOnly
    \sa PROP_MODE
    \return int Number of actuator disks in the current VSPAERO set
*/

extern int GetNumActuatorDisks();

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get the ID of the VSPAERO unsteady group at the specified index. An empty string is returned if
    the index is out of range.
    \forcpponly
    \code{.cpp}
    string wing_id = AddGeom( "WING" );
    string pod_id = AddGeom( "POD" );

    // Create an actuator disk
    string prop_id = AddGeom( "PROP", "" );
    SetParmVal( prop_id, "PropMode", "Design", PROP_BLADES );

    Update();

    // Setup the unsteady group VSPAERO parms
    string disk_id = FindUnsteadyGroup( 1 ); // fixed components are in group 0 (wing & pod)
    if ( disk_id.length() == 0 )
    {
        Print( "ERROR: FindUnsteadyGroup found nothing" );
        __failure++;
    }

    SetParmVal( FindParm( disk_id, "RPM", "UnsteadyGroup" ), 1234.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )
    pod_id = AddGeom( "POD" )

    # Create an actuator disk
    prop_id = AddGeom( "PROP", "" )
    SetParmVal( prop_id, "PropMode", "Design", PROP_BLADES )

    Update()

    # Setup the unsteady group VSPAERO parms
    disk_id = FindUnsteadyGroup( 1 ) # fixed components are in group 0 (wing & pod)
    assert len( disk_id ) > 0, "FindUnsteadyGroup found nothing"

    SetParmVal( FindParm( disk_id, "RPM", "UnsteadyGroup" ), 1234.0 )

    \endcode
    \endPythonOnly
    \sa PROP_MODE
    \param [in] group_index int Unsteady group index for the current VSPAERO set
    \return string Unsteady group ID
*/

extern std::string FindUnsteadyGroup( int group_index );

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get the name of the unsteady group at the specified index.
    \forcpponly
    \code{.cpp}
    // Add a pod and wing
    string pod_id = AddGeom( "POD", "" );
    string wing_id = AddGeom( "WING", pod_id );

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 );
    Update();

    Print( GetUnsteadyGroupName( 0 ) );

    // The pod and wing are fixed components, so they share the one fixed group.
    if ( GetUnsteadyGroupName( 0 ) != "Fixed_Group" )
    {
        Print( "ERROR: GetUnsteadyGroupName did not name the fixed component group" );
        __failure++;
    }

    // That group holds the pod and both wing surfaces.
    if ( GetUnsteadyGroupCompIDs( 0 ).size() != 3 )
    {
        Print( "ERROR: the fixed group does not hold the components" );
        __failure++;
    }

    // A group index past the end has to be rejected.
    GetUnsteadyGroupName( GetNumUnsteadyGroups() );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: GetUnsteadyGroupName accepted an index past the end" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add a pod and wing
    pod_id = AddGeom( "POD", "" )
    wing_id = AddGeom( "WING", pod_id )

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 )
    Update()

    print( GetUnsteadyGroupName( 0 ) )

    # The pod and wing are fixed components, so they share the one fixed group.
    assert GetUnsteadyGroupName( 0 ) == "Fixed_Group", "GetUnsteadyGroupName did not name the fixed component group"

    # That group holds the pod and both wing surfaces.
    assert len( GetUnsteadyGroupCompIDs( 0 ) ) == 3, "the fixed group does not hold the components"

    # A group index past the end has to be rejected.  The error queue is reached
    # through the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    GetUnsteadyGroupName( GetNumUnsteadyGroups() )

    assert err_mgr.GetNumTotalErrors() > 0, "GetUnsteadyGroupName accepted an index past the end"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \sa SetUnsteadyGroupName
    \param [in] group_index int Unsteady group index for the current VSPAERO set
    \return string Unsteady group name
*/

extern std::string GetUnsteadyGroupName( int group_index );

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get an array of IDs for all components in the unsteady group at the specified index.
    \forcpponly
    \code{.cpp}
    // Add a pod and wing
    string pod_id = AddGeom( "POD", "" );
    string wing_id = AddGeom( "WING", pod_id ); // Default with symmetry on -> 2 surfaces

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 );
    Update();

    array < string > comp_ids = GetUnsteadyGroupCompIDs( 0 );

    if ( comp_ids.size() != 3 )
    {
        Print( "ERROR: GetUnsteadyGroupCompIDs" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add a pod and wing
    pod_id = AddGeom( "POD", "" )
    wing_id = AddGeom( "WING", pod_id ) # Default with symmetry on -> 2 surfaces

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 )
    Update()

    comp_ids = GetUnsteadyGroupCompIDs( 0 )

    if  len(comp_ids) != 3 :
        print( "ERROR: GetUnsteadyGroupCompIDs" )
        assert False, "ERROR: GetUnsteadyGroupCompIDs"

    \endcode
    \endPythonOnly
    \sa GetUnsteadyGroupSurfIndexes
    \param [in] group_index int Unsteady group index for the current VSPAERO set
    \return vector \<string\> Array of component IDs
*/

extern std::vector < std::string > GetUnsteadyGroupCompIDs( int group_index );

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get an array of surface indexes for all components in the unsteady group at the specified index.
    \forcpponly
    \code{.cpp}
    // Add a pod and wing
    string pod_id = AddGeom( "POD", "" );
    string wing_id = AddGeom( "WING", pod_id ); // Default with symmetry on -> 2 surfaces

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 );
    Update();

    array < int > surf_indexes = GetUnsteadyGroupSurfIndexes( 0 );

    if ( surf_indexes.size() != 3 )
    {
        Print( "ERROR: GetUnsteadyGroupSurfIndexes" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add a pod and wing
    pod_id = AddGeom( "POD", "" )
    wing_id = AddGeom( "WING", pod_id ) # Default with symmetry on -> 2 surfaces

    SetParmVal( wing_id, "X_Rel_Location", "XForm", 2.5 )
    Update()

    surf_indexes = GetUnsteadyGroupSurfIndexes( 0 )

    if  len(surf_indexes) != 3 :
        print( "ERROR: GetUnsteadyGroupSurfIndexes" )
        assert False, "ERROR: GetUnsteadyGroupSurfIndexes"

    \endcode
    \endPythonOnly
    \sa GetUnsteadyGroupCompIDs
    \param [in] group_index int Unsteady group index for the current VSPAERO set
    \return vector \<int\> Array of surface indexes
*/

extern std::vector < int > GetUnsteadyGroupSurfIndexes( int group_index );

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get the number of unsteady groups in the current VSPAERO set. Each propeller is placed in its own unsteady group. All symmetric copies
    of propellers are also placed in an unsteady group. All other component types are placed in a single fixed component unsteady group.
    \forcpponly
    \code{.cpp}
    // Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL );

    // Add a propeller
    string prop_id = AddGeom( "PROP" );
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    int num_group = GetNumUnsteadyGroups(); // Should be 0

    if ( num_group != 0 )
    {
        Print( "ERROR: an actuator disk counts as an unsteady group" );
        __failure++;
    }

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    num_group = GetNumUnsteadyGroups(); // Should be 1

    if ( num_group != 1 )
    {
        Print( "ERROR: the bladed propeller was not counted" );
        __failure++;
    }

    string wing_id = AddGeom( "WING" );

    num_group = GetNumUnsteadyGroups(); // Should be 2 (includes fixed component group)

    if ( num_group != 2 )
    {
        Print( "ERROR: the fixed component group was not counted" );
        __failure++;
    }

    // Only the propeller is a rotor group; the wing shares the fixed group.
    if ( GetNumUnsteadyRotorGroups() != 1 )
    {
        Print( "ERROR: GetNumUnsteadyGroups disagrees with GetNumUnsteadyRotorGroups" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL )

    # Add a propeller
    prop_id = AddGeom( "PROP" )
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK )

    num_group = GetNumUnsteadyGroups() # Should be 0

    assert num_group == 0, "an actuator disk counts as an unsteady group"

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES )

    num_group = GetNumUnsteadyGroups() # Should be 1

    assert num_group == 1, "the bladed propeller was not counted"

    wing_id = AddGeom( "WING" )

    num_group = GetNumUnsteadyGroups() # Should be 2 (includes fixed component group)

    assert num_group == 2, "the fixed component group was not counted"

    # Only the propeller is a rotor group; the wing shares the fixed group.
    assert GetNumUnsteadyRotorGroups() == 1, "GetNumUnsteadyGroups disagrees with GetNumUnsteadyRotorGroups"

    \endcode
    \endPythonOnly
    \sa PROP_MODE, GetNumUnsteadyRotorGroups
    \return int Number of unsteady groups in the current VSPAERO set
*/

extern int GetNumUnsteadyGroups();

/*!
    \ingroup VSPAERODiskAndProp
*/
/*!
    Get the number of unsteady rotor groups in the current VSPAERO set. This is equivalent to the total number of propeller Geoms,
    including each symmetric copy, in the current VSPAERO set. While all fixed components (wings, fuseleage, etc.) are placed in
    their own unsteady group, this function does not consider them.
    \forcpponly
    \code{.cpp}
    // Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL );

    // Add a propeller
    string prop_id = AddGeom( "PROP" );
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    int num_group = GetNumUnsteadyRotorGroups(); // Should be 0

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    num_group = GetNumUnsteadyRotorGroups(); // Should be 1

    string wing_id = AddGeom( "WING" );

    num_group = GetNumUnsteadyRotorGroups(); // Should be 1 still (fixed group not included)
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Set VSPAERO set index to SET_ALL
    SetParmVal( FindParm( FindContainer( "VSPAEROSettings", 0 ), "GeomSet", "VSPAERO" ), SET_ALL )

    # Add a propeller
    prop_id = AddGeom( "PROP" )
    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK )

    num_group = GetNumUnsteadyRotorGroups() # Should be 0

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES )

    num_group = GetNumUnsteadyRotorGroups() # Should be 1

    wing_id = AddGeom( "WING" )

    num_group = GetNumUnsteadyRotorGroups() # Should be 1 still (fixed group not included)

    \endcode
    \endPythonOnly
    \sa PROP_MODE, GetNumUnsteadyGroups
    \return int Number of unsteady rotor groups in the current VSPAERO set
*/

extern int GetNumUnsteadyRotorGroups();


//======================== Parasite Drag Tool Functions ======================//
/*!
    \ingroup ParasiteDrag
*/
/*!
    Add an Excresence to the Parasite Drag Tool
    \forcpponly
    \code{.cpp}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 );

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 )

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 )

    \endcode
    \endPythonOnly
    \sa EXCRES_TYPE
    \param [in] excresName string Name of the Excressence
    \param [in] excresType int Excressence type enum (i.e. EXCRESCENCE_PERCENT_GEOM)
    \param [in] excresVal double Excressence value
*/

extern void AddExcrescence(const std::string & excresName, const int & excresType, const double & excresVal);

/*!
    \ingroup ParasiteDrag
*/
/*!
    Delete an Excresence from the Parasite Drag Tool
    \forcpponly
    \code{.cpp}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 );

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 );

    AddExcrescence( "Percentage Example", EXCRESCENCE_PERCENT_GEOM, 5 );

    DeleteExcrescence( 2 ); // Last Index

    // Three were added and one was deleted, so index 2 no longer exists and
    // asking for it again has to be rejected.
    DeleteExcrescence( 2 );

    if ( GetNumTotalErrors() == 0 )
    {
        Print( "ERROR: DeleteExcrescence accepted an index past the end" );
        __failure++;
    }

    // That error was raised deliberately, so take it back off the queue.
    while ( GetNumTotalErrors() > 0 )
    {
        ErrorObj err = PopLastError();
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 )

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 )

    AddExcrescence( "Percentage Example", EXCRESCENCE_PERCENT_GEOM, 5 )

    DeleteExcrescence( 2 ) # Last Index

    # Three were added and one was deleted, so index 2 no longer exists and
    # asking for it again has to be rejected.  The error queue is reached through
    # the error manager singleton in Python.
    err_mgr = ErrorMgrSingleton.getInstance()

    DeleteExcrescence( 2 )

    assert err_mgr.GetNumTotalErrors() > 0, "DeleteExcrescence accepted an index past the end"

    # That error was raised deliberately, so take it back off the queue.
    while err_mgr.GetNumTotalErrors() > 0 :
        err = err_mgr.PopLastError()

    \endcode
    \endPythonOnly
    \param [in] index int Index of the Excressence to delete
*/

extern void DeleteExcrescence(const int & index);

/*!
    \ingroup ParasiteDrag
*/
/*!
    Update any reference geometry, atmospheric properties, excressences, etc. in the Parasite Drag Tool
*/

extern void UpdateParasiteDrag();

/*!
    \ingroup ParasiteDrag
*/
/*!
    Calculate the atmospheric properties determined by a specified model for a preset array of altitudes ranging from 0 to 90000 m and
    write the results to a CSV output file
    \forcpponly
    \code{.cpp}
    Print( "Starting USAF Atmosphere 1966 Table Creation. \n" );

    WriteAtmosphereCSVFile( "USAFAtmosphere1966Data.csv", ATMOS_TYPE_HERRINGTON_1966 );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "USAFAtmosphere1966Data.csv", "r" ) < 0 )
    {
        Print( "ERROR: WriteAtmosphereCSVFile wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteAtmosphereCSVFile wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting USAF Atmosphere 1966 Table Creation. \n" )

    WriteAtmosphereCSVFile( "USAFAtmosphere1966Data.csv", ATMOS_TYPE_HERRINGTON_1966 )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "USAFAtmosphere1966Data.csv" ) > 0, "WriteAtmosphereCSVFile wrote no file"


    \endcode
    \endPythonOnly
    \sa ATMOS_TYPE
    \param [in] file_name string Output CSV file
    \param [in] atmos_type int Atmospheric model enum (i.e. ATMOS_TYPE_HERRINGTON_1966)
*/

extern void WriteAtmosphereCSVFile( const std::string & file_name, const int &atmos_type );

/*!
    \ingroup ParasiteDrag
*/
/*!
    Calculate the atmospheric properties determined by a specified model at input altitude and temperature deviation. This function may
    not be used for any manual atmospheric model types (i.e. ATMOS_TYPE_MANUAL_P_T). This function assumes freestream units are metric,
    temperature units are Kelvin, and pressure units are kPA.
    \forcpponly
    \code{.cpp}
    double temp, pres, pres_ratio, rho_ratio;

    double alt = 4000;

    double delta_temp = 0;

    CalcAtmosphere( alt, delta_temp, ATMOS_TYPE_US_STANDARD_1976, temp, pres, pres_ratio, rho_ratio );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    alt = 4000

    delta_temp = 0

    temp, pres, pres_ratio, rho_ratio = CalcAtmosphere( alt, delta_temp, ATMOS_TYPE_US_STANDARD_1976)

    \endcode
    \endPythonOnly
    \sa ATMOS_TYPE
    \param [in] alt double Altitude
    \param [in] delta_temp double Deviation in temperature from the value specified in the atmospheric model
    \param [in] atmos_type int Atmospheric model enum (i.e. ATMOS_TYPE_HERRINGTON_1966)
    \param [out] temp double output Temperature
    \param [out] pres double output Pressure
    \param [out] pres_ratio double Output pressure ratio
    \param [out] rho_ratio double Output density ratio
*/

extern void CalcAtmosphere( const double & alt, const double & delta_temp, const int & atmos_type,

    double & temp, double & pres, double & pres_ratio, double & rho_ratio );
/*!
    \ingroup ParasiteDrag
*/
/*!
    Calculate the form factor from each body FF equation (i.e. Hoerner Streamlined Body) and write the results to a CSV output file
    \forcpponly
    \code{.cpp}
    Print( "Starting Body Form Factor Data Creation. \n" );
    WriteBodyFFCSVFile( "BodyFormFactorData.csv" );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "BodyFormFactorData.csv", "r" ) < 0 )
    {
        Print( "ERROR: WriteBodyFFCSVFile wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteBodyFFCSVFile wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Body Form Factor Data Creation. \n" )
    WriteBodyFFCSVFile( "BodyFormFactorData.csv" )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "BodyFormFactorData.csv" ) > 0, "WriteBodyFFCSVFile wrote no file"


    \endcode
    \endPythonOnly
    \param [in] file_name string Output CSV file
*/

extern void WriteBodyFFCSVFile( const std::string & file_name );

/*!
    \ingroup ParasiteDrag
*/
/*!
    Calculate the form factor from each wing FF equation (i.e. Schemensky 4 Series Airfoil) and write the results to a CSV output file
    \forcpponly
    \code{.cpp}
    Print( "Starting Wing Form Factor Data Creation. \n" );
    WriteWingFFCSVFile( "WingFormFactorData.csv" );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "WingFormFactorData.csv", "r" ) < 0 )
    {
        Print( "ERROR: WriteWingFFCSVFile wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteWingFFCSVFile wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Wing Form Factor Data Creation. \n" )
    WriteWingFFCSVFile( "WingFormFactorData.csv" )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "WingFormFactorData.csv" ) > 0, "WriteWingFFCSVFile wrote no file"


    \endcode
    \endPythonOnly
    \param [in] file_name string Output CSV file
*/

extern void WriteWingFFCSVFile( const std::string & file_name );

/*!
    \ingroup ParasiteDrag
*/
/*!
    Calculate the coefficient of friction from each Cf equation (i.e. Power Law Blasius) and write the results to a CSV output file
    \forcpponly
    \code{.cpp}
    Print( "Starting Turbulent Friciton Coefficient Data Creation. \n" );
    WriteCfEqnCSVFile( "FrictionCoefficientData.csv" );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "FrictionCoefficientData.csv", "r" ) < 0 )
    {
        Print( "ERROR: WriteCfEqnCSVFile wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WriteCfEqnCSVFile wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Turbulent Friciton Coefficient Data Creation. \n" )
    WriteCfEqnCSVFile( "FrictionCoefficientData.csv" )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "FrictionCoefficientData.csv" ) > 0, "WriteCfEqnCSVFile wrote no file"


    \endcode
    \endPythonOnly
    \param [in] file_name string Output CSV file
*/ // TODO: Improve description

extern void WriteCfEqnCSVFile( const std::string & file_name );

/*!
    \ingroup ParasiteDrag
*/
/*!
    Calculate the partial coefficient of friction and write the results to a CSV output file
    \forcpponly
    \code{.cpp}
    Print( "Starting Partial Friction Method Data Creation. \n" );
    WritePartialCfMethodCSVFile( "PartialFrictionMethodData.csv" );
    // The call above should have produced a file with content in it.
    file __f;
    if ( __f.open( "PartialFrictionMethodData.csv", "r" ) < 0 )
    {
        Print( "ERROR: WritePartialCfMethodCSVFile wrote no file" );
        __failure++;
    }
    else
    {
        if ( __f.getSize() <= 0 )
        {
            Print( "ERROR: WritePartialCfMethodCSVFile wrote an empty file" );
            __failure++;
        }
        __f.close();
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Partial Friction Method Data Creation. \n" )
    WritePartialCfMethodCSVFile( "PartialFrictionMethodData.csv" )
    # The call above should have produced a file with content in it.
    import os
    assert os.path.getsize( "PartialFrictionMethodData.csv" ) > 0, "WritePartialCfMethodCSVFile wrote no file"


    \endcode
    \endPythonOnly
    \param [in] file_name string Output CSV file
*/ // TODO: Improve description

extern void WritePartialCfMethodCSVFile( const std::string & file_name );


//======================== Surface Query Functions ======================//
/*!
    \ingroup SurfaceQuery
*/
/*!
    Calculate the 3D coordinate equivalent for the input surface coordinate point
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    Print( "Point: ( " + pnt.x() + ', ' + pnt.y() + ', ' + pnt.z() + ' )' );

    // The point is on the surface, so projecting it back has to land on it and
    // recover the coordinates it came from.
    double u_out, w_out;

    double d = ProjPnt01( geom_id, surf_indx, pnt, u_out, w_out );

    if ( d > 1e-6 )
    {
        Print( "ERROR: CompPnt01 returned a point off the surface" );
        __failure++;
    }

    if ( !closeTo( u_out, u, 1e-6 ) || !closeTo( w_out, w, 1e-6 ) )
    {
        Print( "ERROR: CompPnt01 does not round trip through ProjPnt01" );
        __failure++;
    }

    // W wraps around the section, so 0 and 1 are the same place.
    if ( dist( CompPnt01( geom_id, surf_indx, u, 0.0 ), CompPnt01( geom_id, surf_indx, u, 1.0 ) ) > 1e-6 )
    {
        Print( "ERROR: the surface does not close in W" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    pnt = CompPnt01( geom_id, surf_indx, u, w )

    print( f"Point: ( {pnt.x()}, {pnt.y()}, {pnt.z()} )" )

    # The point is on the surface, so projecting it back has to land on it and
    # recover the coordinates it came from.
    d, u_out, w_out = ProjPnt01( geom_id, surf_indx, pnt )

    assert d < 1e-6, "CompPnt01 returned a point off the surface"
    assert abs( u_out - u ) < 1e-6, "CompPnt01 does not round trip through ProjPnt01"
    assert abs( w_out - w ) < 1e-6, "CompPnt01 does not round trip through ProjPnt01"

    # W wraps around the section, so 0 and 1 are the same place.
    assert dist( CompPnt01( geom_id, surf_indx, u, 0.0 ), CompPnt01( geom_id, surf_indx, u, 1.0 ) ) < 1e-6, "the surface does not close in W"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u double U (0 - 1) surface coordinate
    \param [in] w double W (0 - 1) surface coordinate
    \return vec3d Normal vector3D coordinate point
*/

extern vec3d CompPnt01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Calculate the normal vector on the specified surface at input surface coordinate
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    if ( !closeTo( norm.mag(), 1.0, 1e-9 ) )
    {
        Print( "ERROR: CompNorm01 is not a unit vector" );
        __failure++;
    }

    Print( "Point: ( " + norm.x() + ', ' + norm.y() + ', ' + norm.z() + ' )' );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    norm = CompNorm01( geom_id, surf_indx, u, w )

    assert abs( norm.mag() - 1.0 ) < 1e-9, "CompNorm01 is not a unit vector"

    print( "Point: ( {norm.x()}, {norm.y()}, {norm.z()} )" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u double U (0 - 1) surface coordinate
    \param [in] w double W (0 - 1) surface coordinate
    \return vec3d Normal vector
*/

extern vec3d CompNorm01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Calculate the vector tangent to the specified surface at input surface coordinate in the U direction
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d tanu = CompTanU01( geom_id, surf_indx, u, w );

    if ( tanu.mag() <= 0.0 )
    {
        Print( "ERROR: CompTanU01 is degenerate" );
        __failure++;
    }

    Print( "Point: ( " + tanu.x() + ', ' + tanu.y() + ', ' + tanu.z() + ' )' );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    tanu = CompTanU01( geom_id, surf_indx, u, w )

    assert tanu.mag() > 0.0, "CompTanU01 is degenerate"

    print( f"Point: ( {tanu.x()}, {tanu.y()}, {tanu.z()} )" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u double U (0 - 1) surface coordinate
    \param [in] w double W (0 - 1) surface coordinate
    \return vec3d Tangent vector in U direction
*/

extern vec3d CompTanU01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Calculate the vector tangent to the specified surface at input surface coordinate in the W direction
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d tanw = CompTanW01( geom_id, surf_indx, u, w );

    if ( tanw.mag() <= 0.0 )
    {
        Print( "ERROR: CompTanW01 is degenerate" );
        __failure++;
    }

    Print( "Point: ( " + tanw.x() + ', ' + tanw.y() + ', ' + tanw.z() + ' )' );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    tanw = CompTanW01( geom_id, surf_indx, u, w )

    assert tanw.mag() > 0.0, "CompTanW01 is degenerate"

    print( f"Point: ( {tanw.x()}, {tanw.y()}, {tanw.z()} )" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u double U (0 - 1) surface coordinate
    \param [in] w double W (0 - 1) surface coordinate
    \return vec3d Tangent vector in W direction
*/

extern vec3d CompTanW01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the curvature of a specified surface at the input surface coordinate point
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double k1, k2, ka, kg;

    double u, w;
    u = 0.25;
    w = 0.75;

    CompCurvature01( geom_id, surf_indx, u, w, k1, k2, ka, kg );

    Print( "Curvature : k1 " + k1 + " k2 " + k2 + " ka " + ka + " kg " + kg );

    // The mean curvature is the average of the principal curvatures, and the
    // Gaussian curvature is their product.
    if ( !closeTo( ka, 0.5 * ( k1 + k2 ), 1e-9 ) )
    {
        Print( "ERROR: the mean curvature does not match the principal curvatures" );
        __failure++;
    }

    if ( !closeTo( kg, k1 * k2, 1e-9 ) )
    {
        Print( "ERROR: the Gaussian curvature does not match the principal curvatures" );
        __failure++;
    }

    // A Pod is convex everywhere, so the Gaussian curvature is positive.
    if ( kg <= 0.0 )
    {
        Print( "ERROR: a Pod should be convex here" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0


    u = 0.25
    w = 0.75

    k1, k2, ka, kg = CompCurvature01( geom_id, surf_indx, u, w )

    print( f"Curvature : k1 {k1} k2 {k2} ka {ka} kg {kg}" )

    # The mean curvature is the average of the principal curvatures, and the
    # Gaussian curvature is their product.
    assert abs( ka - 0.5 * ( k1 + k2 ) ) < 1e-9, "the mean curvature does not match the principal curvatures"
    assert abs( kg - k1 * k2 ) < 1e-9, "the Gaussian curvature does not match the principal curvatures"

    # A Pod is convex everywhere, so the Gaussian curvature is positive.
    assert kg > 0.0, "a Pod should be convex here"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u double U (0 - 1) surface coordinate
    \param [in] w double W (0 - 1) surface coordinate
    \param [out] k1_out double Output value of maximum principal curvature
    \param [out] k2_out double Output value of minimum principal curvature
    \param [out] ka_out double Output value of mean curvature
    \param [out] kg_out double Output value of Gaussian curvature
*/

extern void CompCurvature01(const std::string &geom_id, const int &surf_indx, const double &u, const double &w,
                            double &k1_out, double &k2_out, double &ka_out, double &kg_out);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest surface coordinate for an input 3D coordinate point and calculate the distance between the
    3D point and the closest point of the surface.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    double uout, wout;

    // Offset point from surface
    pnt = pnt + norm;

    double d = ProjPnt01( geom_id, surf_indx, pnt, uout, wout );

    // pnt sits one unit off the surface along its own normal, so the
    // projection comes back to where it started at a distance of one.
    if ( !closeTo( d, 1.0, 1e-6 ) || !closeTo( uout, u, 1e-6 ) || !closeTo( wout, w, 1e-6 ) )
    {
        Print( "ERROR: ProjPnt01" );
        __failure++;
    }

    Print( "Dist " + d + " u " + uout + " w " + wout );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    pnt = CompPnt01( geom_id, surf_indx, u, w )

    norm = CompNorm01( geom_id, surf_indx, u, w )


    # Offset point from surface
    pnt.set_xyz( pnt.x() + norm.x(), pnt.y() + norm.y(), pnt.z() + norm.z() )

    d, uout, wout = ProjPnt01( geom_id, surf_indx, pnt )

    # pnt sits one unit off the surface along its own normal, so the
    # projection comes back to where it started at a distance of one.
    assert abs( d - 1.0 ) < 1e-6, "ProjPnt01 distance"
    assert abs( uout - u ) < 1e-6 and abs( wout - w ) < 1e-6, "ProjPnt01 u, w"

    print( f"Dist {d} u {uout} w {wout}" )

    \endcode
    \endPythonOnly
    \sa ProjPnt01Guess, ProjPnt01I
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pt vec3d Input 3D coordinate point
    \param [out] u_out double Output closest U (0 - 1) surface coordinate
    \param [out] w_out double Output closest W (0 - 1) surface coordinate
    \return double Distance between the 3D point and the closest point of the surface
*/

extern double ProjPnt01(const std::string &geom_id, const int &surf_indx, const vec3d &pt, double &u_out, double &w_out);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest surface coordinate and corresponding parent Geom main surface index for an input 3D coordinate point. Return the distance between
    the closest point and the input.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    double d = 0;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    double uout, wout;

    int surf_indx_out;

    // Offset point from surface
    pnt = pnt + norm;

    d = ProjPnt01I( geom_id, pnt, surf_indx_out, uout, wout );

    // pnt sits one unit off the surface along its own normal, so the
    // projection comes back to where it started at a distance of one.
    if ( !closeTo( d, 1.0, 1e-6 ) || !closeTo( uout, u, 1e-6 ) || !closeTo( wout, w, 1e-6 ) )
    {
        Print( "ERROR: ProjPnt01I" );
        __failure++;
    }

    Print( "Dist " + d + " u " + uout + " w " + wout + " surf_index " + surf_indx_out );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    d = 0

    pnt = CompPnt01( geom_id, surf_indx, u, w )

    norm = CompNorm01( geom_id, surf_indx, u, w )



    # Offset point from surface
    pnt.set_xyz( pnt.x() + norm.x(), pnt.y() + norm.y(), pnt.z() + norm.z() )

    d, surf_indx_out, uout, wout = ProjPnt01I( geom_id, pnt )

    # pnt sits one unit off the surface along its own normal, so the
    # projection comes back to where it started at a distance of one.
    assert abs( d - 1.0 ) < 1e-6, "ProjPnt01I distance"
    assert abs( uout - u ) < 1e-6 and abs( wout - w ) < 1e-6, "ProjPnt01I u, w"

    print( f"Dist {d} u {uout} w {wout} surf_index {surf_indx_out}" )

    \endcode
    \endPythonOnly
    \sa ProjPnt01, ProjPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] pt vec3d Input 3D coordinate point
    \param [out] surf_indx_out int Output main surface index from the parent Geom
    \param [out] u_out double Output closest U (0 - 1) surface coordinat
    \param [out] w_out double Output closest W (0 - 1) surface coordinat
    \return double Distance between the 3D point and the closest point of the surface
*/

extern double ProjPnt01I(const std::string &geom_id, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest surface coordinate for an input 3D coordinate point and calculate the distance between the
    3D point and the closest point of the surface. This function takes an input surface coordinate guess for, offering
    a potential decrease in computation time compared to ProjPnt01.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    double d = 0;

    vec3d pnt = CompPnt01( geom_id, surf_indx, u, w );

    vec3d norm = CompNorm01( geom_id, surf_indx, u, w );

    double uout, wout;

    // Offset point from surface
    pnt = pnt + norm;

    d = ProjPnt01Guess( geom_id, surf_indx, pnt, u + 0.1, w + 0.1, uout, wout );

    // pnt sits one unit off the surface along its own normal, so the
    // projection comes back to where it started at a distance of one.
    if ( !closeTo( d, 1.0, 1e-6 ) || !closeTo( uout, u, 1e-6 ) || !closeTo( wout, w, 1e-6 ) )
    {
        Print( "ERROR: ProjPnt01Guess" );
        __failure++;
    }

    Print( "Dist " + d + " u " + uout + " w " + wout );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    d = 0

    pnt = CompPnt01( geom_id, surf_indx, u, w )

    norm = CompNorm01( geom_id, surf_indx, u, w )


    # Offset point from surface
    pnt.set_xyz( pnt.x() + norm.x(), pnt.y() + norm.y(), pnt.z() + norm.z() )

    d, uout, wout = ProjPnt01Guess( geom_id, surf_indx, pnt, u + 0.1, w + 0.1 )

    # pnt sits one unit off the surface along its own normal, so the
    # projection comes back to where it started at a distance of one.
    assert abs( d - 1.0 ) < 1e-6, "ProjPnt01Guess distance"
    assert abs( uout - u ) < 1e-6 and abs( wout - w ) < 1e-6, "ProjPnt01Guess u, w"

    print( f"Dist {d} u {uout} w {wout}" )

    \endcode
    \endPythonOnly
    \sa ProjPnt01, ProjPnt01I
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pt vec3d Input 3D coordinate point
    \param [in] u0 double Input U (0 - 1) surface coordinate guess
    \param [in] w0 double Input W (0 - 1) surface coordinate guess
    \param [out] u_out double Output closest U (0 - 1) surface coordinate
    \param [out] w_out double Output closest W (0 - 1) surface coordinate
    \return double Distance between the 3D point and the closest point of the surface
*/

extern double ProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out);


/*!
    \ingroup SurfaceQuery
*/
/*!
    Project an input 3D coordinate point onto a surface along a specified axis.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d surf_pt = CompPnt01( geom_id, surf_indx, u, w );
    vec3d pt = surf_pt;

    pt.offset_y( -5.0 );

    double u_out, w_out;

    double idist = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt, u_out, w_out);

    // pt is the surface point pushed off in -Y, so projecting back along Y
    // has to land on the point it came from.
    if ( ( surf_pt - CompPnt01( geom_id, surf_indx, u_out, w_out ) ).mag() > 1e-6 )
    {
        Print( "ERROR: AxisProjPnt01 did not recover the original point" );
        __failure++;
    }

    Print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out );
    Print( "3D Offset ", false);
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    surf_pt = CompPnt01( geom_id, surf_indx, u, w )

    # Assignment binds a reference in Python rather than copying as it does in
    # AngelScript, so build a separate point instead of offsetting surf_pt.
    pt = vec3d( surf_pt.x(), surf_pt.y(), surf_pt.z() )

    pt.offset_y( -5.0 )

    idist, u_out, w_out = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt )

    # pt is the surface point pushed off in -Y, so projecting back along Y
    # has to land on the point it came from.
    p_out = CompPnt01( geom_id, surf_indx, u_out, w_out )
    assert abs( surf_pt.x() - p_out.x() ) < 1e-6 and abs( surf_pt.y() - p_out.y() ) < 1e-6 and abs( surf_pt.z() - p_out.z() ) < 1e-6, "AxisProjPnt01 did not recover the original point"

    print( f"iDist {idist} u_out {u_out} w_out {w_out}" )
    print( "3D Offset ", False)

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt vec3d Input 3D coordinate point
    \param [out] u_out double Output closest U (0 - 1) surface coordinate
    \param [out] w_out double Output closest W (0 - 1) surface coordinate
    \return double Axis aligned distance between the 3D point and the projected point on the surface
*/

extern double AxisProjPnt01(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, double &u_out, double &w_out);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Project an input 3D coordinate point onto a Geom along a specified axis.  The intersecting surface index is also returned.  If the axis-aligned ray from the point intersects the Geom multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the Geom, the original point is returned and -1 is returned in the other output parameters.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;

    vec3d surf_pt = CompPnt01( geom_id, surf_indx, u, w );
    vec3d pt = surf_pt;

    pt.offset_y( -5.0 );

    double u_out, w_out;
    int surf_indx_out;

    double idist = AxisProjPnt01I( geom_id, Y_DIR, pt, surf_indx_out, u_out, w_out);

    // pt is the surface point pushed off in -Y, so projecting back along Y
    // has to land on the point it came from.
    if ( ( surf_pt - CompPnt01( geom_id, surf_indx, u_out, w_out ) ).mag() > 1e-6 )
    {
        Print( "ERROR: AxisProjPnt01I did not recover the original point" );
        __failure++;
    }

    Print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out + " surf_index " + surf_indx_out );
    Print( "3D Offset ", false);
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890

    surf_pt = CompPnt01( geom_id, surf_indx, u, w )

    # Assignment binds a reference in Python rather than copying as it does in
    # AngelScript, so build a separate point instead of offsetting surf_pt.
    pt = vec3d( surf_pt.x(), surf_pt.y(), surf_pt.z() )

    pt.offset_y( -5.0 )


    idist, surf_indx_out, u_out, w_out = AxisProjPnt01I( geom_id, Y_DIR, pt )

    # pt is the surface point pushed off in -Y, so projecting back along Y
    # has to land on the point it came from.
    p_out = CompPnt01( geom_id, surf_indx, u_out, w_out )
    assert abs( surf_pt.x() - p_out.x() ) < 1e-6 and abs( surf_pt.y() - p_out.y() ) < 1e-6 and abs( surf_pt.z() - p_out.z() ) < 1e-6, "AxisProjPnt01I did not recover the original point"

    print( "iDist {idist} u_out {u_out} w_out {w_out} surf_index {surf_indx_out}" )
    print( "3D Offset ", False)

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt vec3d Input 3D coordinate point
    \param [out] surf_indx_out int Output main surface index from the parent Geom
    \param [out] u_out double Output closest U (0 - 1) surface coordinate
    \param [out] w_out double Output closest W (0 - 1) surface coordinate
    \return double Axis aligned distance between the 3D point and the projected point on the surface
*/

extern double AxisProjPnt01I(const std::string &geom_id, const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Project an input 3D coordinate point onto a surface along a specified axis given an initial guess of surface parameter.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.  The surface parameter guess should allow this call to be faster than calling AxisProjPnt01 without a guess.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double u = 0.12345;
    double w = 0.67890;



    vec3d surf_pt = CompPnt01( geom_id, surf_indx, u, w );
    vec3d pt = surf_pt;

    pt.offset_y( -5.0 );

    // Construct initial guesses near actual parameters
    double u0 = u + 0.01234;
    double w0 = w - 0.05678;

    double uout, wout;

    double d = AxisProjPnt01Guess( geom_id, surf_indx, Y_DIR, pt, u0, w0, uout, wout);

    Print( "Dist " + d + " u " + uout + " w " + wout );

    // The test point sits five units away along Y from a known surface point,
    // so projecting back along Y has to recover that point and that distance.
    if ( !closeTo( uout, u, 1e-6 ) || !closeTo( wout, w, 1e-6 ) )
    {
        Print( "ERROR: AxisProjPnt01Guess did not recover the surface point" );
        __failure++;
    }

    if ( !closeTo( d, 5.0, 1e-6 ) )
    {
        Print( "ERROR: AxisProjPnt01Guess reported the wrong distance" );
        __failure++;
    }

    // Starting from a guess must not change the answer.
    double uout_ng, wout_ng;

    double d_ng = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt, uout_ng, wout_ng );

    if ( !closeTo( uout_ng, uout, 1e-6 ) || !closeTo( wout_ng, wout, 1e-6 ) || !closeTo( d_ng, d, 1e-6 ) )
    {
        Print( "ERROR: the guess changed the answer" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    u = 0.12345
    w = 0.67890



    surf_pt = CompPnt01( geom_id, surf_indx, u, w )

    # Assignment binds a reference in Python rather than copying as it does in
    # AngelScript, so build a separate point instead of offsetting surf_pt.
    pt = vec3d( surf_pt.x(), surf_pt.y(), surf_pt.z() )

    pt.offset_y( -5.0 )

    # Construct initial guesses near actual parameters
    u0 = u + 0.01234
    w0 = w - 0.05678

    d, uout, wout = AxisProjPnt01Guess( geom_id, surf_indx, Y_DIR, pt, u0, w0 )

    print( f"Dist {d} u {uout} w {wout}" )

    # The test point sits five units away along Y from a known surface point, so
    # projecting back along Y has to recover that point and that distance.
    assert abs( uout - u ) < 1e-6, "AxisProjPnt01Guess did not recover the surface point"
    assert abs( wout - w ) < 1e-6, "AxisProjPnt01Guess did not recover the surface point"
    assert abs( d - 5.0 ) < 1e-6, "AxisProjPnt01Guess reported the wrong distance"

    # Starting from a guess must not change the answer.
    d_ng, uout_ng, wout_ng = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt )

    assert abs( uout_ng - uout ) < 1e-6, "the guess changed the answer"
    assert abs( wout_ng - wout ) < 1e-6, "the guess changed the answer"
    assert abs( d_ng - d ) < 1e-6, "the guess changed the answer"

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01I, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt vec3d Input 3D coordinate point
    \param [in] u0 double Input U (0 - 1) surface coordinate guess
    \param [in] w0 double Input W (0 - 1) surface coordinate guess
    \param [out] u_out double Output closest U (0 - 1) surface coordinate
    \param [out] w_out double Output closest W (0 - 1) surface coordinate
    \return double Distance between the 3D point and the closest point of the surface
*/

extern double AxisProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Test whether a given point is inside a specified surface.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.68;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    bool res = InsideSurf( geom_id, surf_indx, pnt );

    // pnt was built by CompPntRST at r = 0.12, which is inside the surface.
    if ( !res )
    {
        Print( "ERROR: InsideSurf says an interior point is outside" );
        __failure++;
    }

    if ( res )
    {
        Print( "Inside" );
    }
    else
    {
        Print( "Outside" );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12
    s = 0.68
    t = 0.56

    pnt = CompPntRST( geom_id, surf_indx, r, s, t )

    res = InsideSurf( geom_id, surf_indx, pnt )

    # pnt was built by CompPntRST at r = 0.12, which is inside the surface.
    assert res, "InsideSurf says an interior point is outside"

    if  res :
        print( "Inside" )
    else:
        print( "Outside" )


    \endcode
    \endPythonOnly
    \sa VecInsideSurf
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pt vec3d Input 3D coordinate point
    \return bool Boolean true if the point is inside the surface, false otherwise.
*/

extern bool InsideSurf( const std::string &geom_id, const int &surf_indx, const vec3d &pt );


/*!
    \ingroup SurfaceQuery
*/
/*!
    Calculate the (X, Y, Z) coordinate for the input volume (R, S, T) coordinate point
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.68;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    Print( "Point: ( " + pnt.x() + ', ' + pnt.y() + ', ' + pnt.z() + ' )' );

    // T runs from the interior out to the skin, so t = 1 lands on the surface
    // and anything short of that lands inside it.
    double u_out, w_out;

    if ( ProjPnt01( geom_id, surf_indx, CompPntRST( geom_id, surf_indx, r, s, 1.0 ), u_out, w_out ) > 1e-6 )
    {
        Print( "ERROR: CompPntRST at t = 1 is not on the surface" );
        __failure++;
    }

    if ( !InsideSurf( geom_id, surf_indx, pnt ) )
    {
        Print( "ERROR: CompPntRST at t < 1 is not inside the surface" );
        __failure++;
    }

    // The point has to round trip back through the coordinates it came from.
    double r_out, s_out, t_out;

    FindRST( geom_id, surf_indx, pnt, r_out, s_out, t_out );

    if ( dist( CompPntRST( geom_id, surf_indx, r_out, s_out, t_out ), pnt ) > 1e-6 )
    {
        Print( "ERROR: CompPntRST does not round trip through FindRST" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12
    s = 0.68
    t = 0.56

    pnt = CompPntRST( geom_id, surf_indx, r, s, t )

    print( f"Point: ( {pnt.x()}, {pnt.y()}, {pnt.z()} )" )

    # T runs from the interior out to the skin, so t = 1 lands on the surface and
    # anything short of that lands inside it.
    d, u_out, w_out = ProjPnt01( geom_id, surf_indx, CompPntRST( geom_id, surf_indx, r, s, 1.0 ) )

    assert d < 1e-6, "CompPntRST at t = 1 is not on the surface"
    assert InsideSurf( geom_id, surf_indx, pnt ), "CompPntRST at t < 1 is not inside the surface"

    # The point has to round trip back through the coordinates it came from.
    d_rst, r_out, s_out, t_out = FindRST( geom_id, surf_indx, pnt )

    assert dist( CompPntRST( geom_id, surf_indx, r_out, s_out, t_out ), pnt ) < 1e-6, "CompPntRST does not round trip through FindRST"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r double R (0 - 1) volume coordinate
    \param [in] s double S (0 - 1) volume coordinate
    \param [in] t double T (0 - 1) volume coordinate
    \return vec3d coordinate point
*/

extern vec3d CompPntRST( const std::string &geom_id, const int &surf_indx, const double &r, const double &s, const double &t );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest (R, S, T) volume coordinate for an input (X, Y, Z) 3D coordinate point and calculate the distance between the
    3D point and the found volume point.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.68;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    double rout, sout, tout;

    double d = FindRST( geom_id, surf_indx, pnt, rout, sout, tout );

    Print( "Dist " + d + " r " + rout + " s " + sout + " t " + tout );

    // pnt came from CompPntRST at r, s, t, so the search has to land back on it.
    if ( !closeTo( d, 0.0, 1e-6 ) || !closeTo( rout, r, 1e-6 ) || !closeTo( sout, s, 1e-6 ) || !closeTo( tout, t, 1e-6 ) )
    {
        Print( "ERROR: FindRST did not recover the original r, s, t" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12
    s = 0.68
    t = 0.56

    pnt = CompPntRST( geom_id, surf_indx, r, s, t )


    d, rout, sout, tout = FindRST( geom_id, surf_indx, pnt )

    print( f"Dist {d} r {rout} s {sout} t {tout}" )

    # pnt came from CompPntRST at r, s, t, so the search has to land back on it.
    assert abs( d ) < 1e-6, "FindRST distance"
    assert abs( rout - r ) < 1e-6 and abs( sout - s ) < 1e-6 and abs( tout - t ) < 1e-6, "FindRST r, s, t"

    \endcode
    \endPythonOnly
    \sa FindRSTGuess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pt vec3d Input 3D coordinate point
    \param [out] r_out double Output closest R (0 - 1.0) volume coordinate
    \param [out] s_out double Output closest S (0 - 1.0) volume coordinate
    \param [out] t_out double Output closest T (0 - 1.0) volume coordinate
    \return double Distance between the 3D point and the closest point of the volume
*/

extern double FindRST( const std::string &geom_id, const int &surf_indx, const vec3d &pt, double &r_out, double &s_out, double &t_out );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest (R, S, T) volume coordinate for an input (X, Y, Z) 3D coordinate point given an initial guess of volume coordinates.  Also calculate the distance between the
    3D point and the found volume point.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.68;
    double t = 0.56;

    vec3d pnt = CompPntRST( geom_id, surf_indx, r, s, t );

    double rout, sout, tout;

    double r0 = 0.1;
    double s0 = 0.6;
    double t0 = 0.5;

    double d = FindRSTGuess( geom_id, surf_indx, pnt, r0, s0, t0, rout, sout, tout );

    Print( "Dist " + d + " r " + rout + " s " + sout + " t " + tout );

    // pnt came from CompPntRST at r, s, t, so the search has to land back on it.
    if ( !closeTo( d, 0.0, 1e-6 ) || !closeTo( rout, r, 1e-6 ) || !closeTo( sout, s, 1e-6 ) || !closeTo( tout, t, 1e-6 ) )
    {
        Print( "ERROR: FindRSTGuess did not recover the original r, s, t" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12
    s = 0.68
    t = 0.56

    pnt = CompPntRST( geom_id, surf_indx, r, s, t )


    r0 = 0.1
    s0 = 0.6
    t0 = 0.5

    d, rout, sout, tout = FindRSTGuess( geom_id, surf_indx, pnt, r0, s0, t0 )

    print( f"Dist {d} r {rout} s {sout} t {tout}" )

    # pnt came from CompPntRST at r, s, t, so the search has to land back on it.
    assert abs( d ) < 1e-6, "FindRSTGuess distance"
    assert abs( rout - r ) < 1e-6 and abs( sout - s ) < 1e-6 and abs( tout - t ) < 1e-6, "FindRSTGuess r, s, t"

    \endcode
    \endPythonOnly
    \sa FindRST
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pt vec3d Input 3D coordinate point
    \param [in] r0 double Input R (0 - 1.0) volume coordinate guess
    \param [in] s0 double Input S (0 - 1.0) volume coordinate guess
    \param [in] t0 double Input T (0 - 1.0) volume coordinate guess
    \param [out] r_out double Output closest R (0 - 1.0) volume coordinate
    \param [out] s_out double Output closest S (0 - 1.0) volume coordinate
    \param [out] t_out double Output closest T (0 - 1.0) volume coordinate
    \return double Distance between the 3D point and the closest point of the volume
*/

extern double FindRSTGuess( const std::string &geom_id, const int &surf_indx, const vec3d &pt, const double &r0, const double &s0, const double &t0, double &r_out, double &s_out, double &t_out );


/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert RST volumetric coordinates to LMN coordinates.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double s = 0.68;
    double t = 0.56;
    double l_out, m_out, n_out;

    ConvertRSTtoLMN( geom_id, surf_indx, r, s, t, l_out, m_out, n_out );

    // Converting back has to return the coordinates we started from.
    double r_back, s_back, t_back;
    ConvertLMNtoRST( geom_id, surf_indx, l_out, m_out, n_out, r_back, s_back, t_back );

    if ( !closeTo( r_back, r, 1e-6 ) || !closeTo( s_back, s, 1e-6 ) || !closeTo( t_back, t, 1e-6 ) )
    {
        Print( "ERROR: ConvertRSTtoLMN does not round trip" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12
    s = 0.68
    t = 0.56

    l_out, m_out, n_out = ConvertRSTtoLMN( geom_id, surf_indx, r, s, t )

    # Converting back has to return the coordinates we started from.
    r_back, s_back, t_back = ConvertLMNtoRST( geom_id, surf_indx, l_out, m_out, n_out )

    assert abs( r_back - r ) < 1e-6 and abs( s_back - s ) < 1e-6 and abs( t_back - t ) < 1e-6, "ConvertRSTtoLMN does not round trip"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r double R (0 - 1) volume coordinate
    \param [in] s double S (0 - 1) volume coordinate
    \param [in] t double T (0 - 1) volume coordinate
    \param [out] l_out double L (0 - 1) linear volume coordinate
    \param [out] m_out double M (0 - 1) linear volume coordinate
    \param [out] n_out double N (0 - 1) linear volume coordinate
*/

extern void ConvertRSTtoLMN( const std::string &geom_id, const int &surf_indx, const double &r, const double &s, const double &t, double &l_out, double &m_out, double &n_out );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert R volumetric coordinate to L coordinate.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double r = 0.12;
    double l_out;

    ConvertRtoL( geom_id, surf_indx, r, l_out );

    // Converting back has to return the coordinate we started from.
    double r_back;
    ConvertLtoR( geom_id, surf_indx, l_out, r_back );

    if ( !closeTo( r_back, r, 1e-6 ) )
    {
        Print( "ERROR: ConvertRtoL does not round trip" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12

    l_out = ConvertRtoL( geom_id, surf_indx, r )

    # Converting back has to return the coordinate we started from.
    r_back = ConvertLtoR( geom_id, surf_indx, l_out )

    assert abs( r_back - r ) < 1e-6, "ConvertRtoL does not round trip"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r double R (0 - 1) volume coordinate
    \param [out] l_out double L (0 - 1) linear volume coordinate
*/

extern void ConvertRtoL( const std::string &geom_id, const int &surf_indx, const double &r, double &l_out );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert LMN volumetric coordinates to RST coordinates.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double l = 0.12;
    double m = 0.34;
    double n = 0.56;
    double r_out, s_out, t_out;

    ConvertLMNtoRST( geom_id, surf_indx, l, m, n, r_out, s_out, t_out );

    // The conversion is invertible, so going back has to return the input.
    double l_out, m_out, n_out;

    ConvertRSTtoLMN( geom_id, surf_indx, r_out, s_out, t_out, l_out, m_out, n_out );

    if ( !closeTo( l_out, l, 1e-6 ) || !closeTo( m_out, m, 1e-6 ) || !closeTo( n_out, n, 1e-6 ) )
    {
        Print( "ERROR: ConvertLMNtoRST does not invert" );
        __failure++;
    }

    // Both coordinate systems run over the unit cube.
    if ( r_out < 0.0 || r_out > 1.0 || s_out < 0.0 || s_out > 1.0 || t_out < 0.0 || t_out > 1.0 )
    {
        Print( "ERROR: ConvertLMNtoRST left the unit cube" );
        __failure++;
    }

    // The coordinates name a point in the volume, so evaluating them has to
    // land inside the surface.
    if ( !InsideSurf( geom_id, surf_indx, CompPntRST( geom_id, surf_indx, r_out, s_out, t_out ) ) )
    {
        Print( "ERROR: ConvertLMNtoRST did not name a point in the volume" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    l = 0.12
    m = 0.34
    n = 0.56

    r_out, s_out, t_out = ConvertLMNtoRST( geom_id, surf_indx, l, m, n )

    # The conversion is invertible, so going back has to return the input.
    l_out, m_out, n_out = ConvertRSTtoLMN( geom_id, surf_indx, r_out, s_out, t_out )

    assert abs( l_out - l ) < 1e-6, "ConvertLMNtoRST does not invert"
    assert abs( m_out - m ) < 1e-6, "ConvertLMNtoRST does not invert"
    assert abs( n_out - n ) < 1e-6, "ConvertLMNtoRST does not invert"

    # Both coordinate systems run over the unit cube.
    assert 0.0 <= r_out <= 1.0, "ConvertLMNtoRST left the unit cube"
    assert 0.0 <= s_out <= 1.0, "ConvertLMNtoRST left the unit cube"
    assert 0.0 <= t_out <= 1.0, "ConvertLMNtoRST left the unit cube"

    # The coordinates name a point in the volume, so evaluating them has to land
    # inside the surface.
    assert InsideSurf( geom_id, surf_indx, CompPntRST( geom_id, surf_indx, r_out, s_out, t_out ) ), "ConvertLMNtoRST did not name a point in the volume"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] l double L (0 - 1) linear volume coordinate
    \param [in] m double M (0 - 1) linear volume coordinate
    \param [in] n double N (0 - 1) linear volume coordinate
    \param [out] r_out double R (0 - 1) volume coordinate
    \param [out] s_out double S (0 - 1) volume coordinate
    \param [out] t_out double T (0 - 1) volume coordinate
*/

extern void ConvertLMNtoRST( const std::string &geom_id, const int &surf_indx, const double &l, const double &m, const double &n, double &r_out, double &s_out, double &t_out );
/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert L volumetric coordinate to R coordinate.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    double l = 0.12;
    double r_out;

    ConvertLtoR( geom_id, surf_indx, l, r_out );

    // The conversion is invertible, so going back has to return the input.
    double l_out;

    ConvertRtoL( geom_id, surf_indx, r_out, l_out );

    if ( !closeTo( l_out, l, 1e-6 ) )
    {
        Print( "ERROR: ConvertLtoR does not invert" );
        __failure++;
    }

    if ( r_out < 0.0 || r_out > 1.0 )
    {
        Print( "ERROR: ConvertLtoR left the unit interval" );
        __failure++;
    }

    // Both coordinates run nose to tail, so the mapping is increasing.
    double r_more;

    ConvertLtoR( geom_id, surf_indx, l + 0.1, r_more );

    if ( r_more <= r_out )
    {
        Print( "ERROR: ConvertLtoR is not increasing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    l = 0.12

    r_out = ConvertLtoR( geom_id, surf_indx, l )

    # The conversion is invertible, so going back has to return the input.
    l_out = ConvertRtoL( geom_id, surf_indx, r_out )

    assert abs( l_out - l ) < 1e-6, "ConvertLtoR does not invert"
    assert 0.0 <= r_out <= 1.0, "ConvertLtoR left the unit interval"

    # Both coordinates run nose to tail, so the mapping is increasing.
    r_more = ConvertLtoR( geom_id, surf_indx, l + 0.1 )

    assert r_more > r_out, "ConvertLtoR is not increasing"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] l double L (0 - 1) volume coordinate
    \param [out] r_out double R (0 - 1) linear volume coordinate
*/
extern void ConvertLtoR( const std::string &geom_id, const int &surf_indx, const double &l, double &r_out );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert U coordinate to eta wing coordinate.
    \forcpponly
    \code{.cpp}
    // Add Wing Geom
    string geom_id = AddGeom( "WING", "" );

    int surf_indx = 0;

    // U runs from 1 to N+1 over an N section wing when the root end cap is on,
    // so a U below 1 sits in the cap and does not map to a span station.
    double u = 1.25;
    double eta_out;

    ConvertUtoEta( geom_id, u, eta_out );

    // Converting back has to return the coordinate we started from.
    double u_back;
    ConvertEtatoU( geom_id, eta_out, u_back );

    if ( !closeTo( u_back, u, 1e-6 ) )
    {
        Print( "ERROR: ConvertUtoEta does not round trip" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing Geom
    geom_id = AddGeom( "WING", "" )

    surf_indx = 0

    # U runs from 1 to N+1 over an N section wing when the root end cap is on,
    # so a U below 1 sits in the cap and does not map to a span station.
    u = 1.25

    eta_out = ConvertUtoEta( geom_id, u )

    # Converting back has to return the coordinate we started from.
    u_back = ConvertEtatoU( geom_id, eta_out )

    assert abs( u_back - u ) < 1e-6, "ConvertUtoEta does not round trip"


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] u double U (0 - 1) surface coordinate
    \param [out] eta_out double Eta (0 - 1) wing spanwise coordinate
*/

extern void ConvertUtoEta( const std::string &geom_id, const double &u, double &eta_out );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert eta wing coordinate to u coordinate.
    \forcpponly
    \code{.cpp}
    // Add Wing Geom
    string geom_id = AddGeom( "WING", "" );

    int surf_indx = 0;

    double eta= 0.25;
    double u_out;

    ConvertEtatoU( geom_id, eta, u_out );

    // Converting back has to return the coordinate we started from.
    double eta_back;
    ConvertUtoEta( geom_id, u_out, eta_back );

    if ( !closeTo( eta_back, eta, 1e-6 ) )
    {
        Print( "ERROR: ConvertEtatoU does not round trip" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing Geom
    geom_id = AddGeom( "WING", "" )

    surf_indx = 0

    eta= 0.25

    u = ConvertEtatoU( geom_id, eta )

    # Converting back has to return the coordinate we started from.
    eta_back = ConvertUtoEta( geom_id, u )

    assert abs( eta_back - eta ) < 1e-6, "ConvertEtatoU does not round trip"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] eta double Eta (0 - 1) wing spanwise coordinate
    \param [out] u_out double U (0 - 1) surface coordinate
*/

extern void ConvertEtatoU( const std::string &geom_id, const double &eta, double &u_out );


/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine 3D coordinate for each surface coordinate point in the input arrays
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, 0, uvec, wvec );

    // One point per coordinate pair, each the same point the scalar form gives.
    if ( int( ptvec.size() ) != n )
    {
        Print( "ERROR: CompVecPnt01 returned the wrong number of points" );
        __failure++;
    }
    else
    {
        for ( int i = 0 ; i < n ; i++ )
        {
            if ( dist( ptvec[i], CompPnt01( geom_id, 0, uvec[i], wvec[i] ) ) > 1e-9 )
            {
                Print( "ERROR: CompVecPnt01 disagrees with CompPnt01 at " + i );
                __failure++;
            }
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    ptvec = CompVecPnt01( geom_id, 0, uvec, wvec )

    # One point per coordinate pair, each the same point the scalar form gives.
    assert len( ptvec ) == n, "CompVecPnt01 returned the wrong number of points"

    for i in range(n):
        assert dist( ptvec[i], CompPnt01( geom_id, 0, uvec[i], wvec[i] ) ) < 1e-9, "CompVecPnt01 disagrees with CompPnt01 at " + str( i )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u_in_vec vector<double> Input vector of U (0 - 1) surface coordinates
    \param [in] w_in_vec vector<double> Input vector of W (0 - 1) surface coordinates
    \return vector\<vec3d\> Vector of 3D coordinate points
*/

extern std::vector < vec3d > CompVecPnt01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &u_in_vec, const std::vector < double > &w_in_vec);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine 3D coordinate for each degen surface coordinate point in the input arrays
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecDegenPnt01( geom_id, 0, 0, uvec, wvec );

    // One point per coordinate pair.  Degen type 0 is the surface itself, so
    // those points are the ones the surface form gives.
    if ( int( ptvec.size() ) != n )
    {
        Print( "ERROR: CompVecDegenPnt01 returned the wrong number of points" );
        __failure++;
    }
    else
    {
        array< vec3d > surfvec = CompVecPnt01( geom_id, 0, uvec, wvec );

        for ( int i = 0 ; i < n ; i++ )
        {
            if ( dist( ptvec[i], surfvec[i] ) > 1e-6 )
            {
                Print( "ERROR: the degen surface does not follow the surface at " + i );
                __failure++;
            }
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    ptvec = CompVecDegenPnt01( geom_id, 0, 0, uvec, wvec )

    # One point per coordinate pair.  Degen type 0 is the surface itself, so
    # those points are the ones the surface form gives.
    assert len( ptvec ) == n, "CompVecDegenPnt01 returned the wrong number of points"

    surfvec = CompVecPnt01( geom_id, 0, uvec, wvec )

    for i in range(n):
        assert dist( ptvec[i], surfvec[i] ) < 1e-6, "the degen surface does not follow the surface at " + str( i )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] degen_type int Type of degen surface (0-S, 1-V, 2-H, 3-C)
    \param [in] u_in_vec vector<double> Input vector of U (0 - 1) surface coordinates
    \param [in] w_in_vec vector<double> Input vector of W (0 - 1) surface coordinates
    \return vector\<vec3d\> Vector of 3D coordinate points
*/

extern std::vector < vec3d > CompVecDegenPnt01(const std::string &geom_id, const int &surf_indx, const int &degen_type, const std::vector < double > &u_in_vec, const std::vector < double > &w_in_vec);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the normal vector on a surface for each surface coordinate point in the input arrays
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > normvec = CompVecNorm01( geom_id, 0, uvec, wvec );

    for( int i = 0 ; i < int( normvec.length() ) ; i++ )
    {
        if ( !closeTo( normvec[i].mag(), 1.0, 1e-9 ) )
        {
            Print( "ERROR: CompVecNorm01 entry " + i + " is not a unit vector" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    normvec = CompVecNorm01( geom_id, 0, uvec, wvec )

    for i in range( len( normvec ) ):
        assert abs( normvec[i].mag() - 1.0 ) < 1e-9, "CompVecNorm01 is not a unit vector"

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] us vector<double> Input vector of U (0 - 1) surface coordinates
    \param [in] ws vector<double> Input vector of W (0 - 1) surface coordinates
    \return vector\<vec3d\> Vector of 3D normal vectors
*/

extern std::vector < vec3d > CompVecNorm01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the curvature of a specified surface at each surface coordinate point in the input arrays
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array<double> k1vec, k2vec, kavec, kgvec;

    CompVecCurvature01( geom_id, 0, uvec, wvec, k1vec, k2vec, kavec, kgvec );

    // One value per coordinate pair, matching what the scalar form gives, and
    // holding the same relationships between the four curvatures.
    if ( int( k1vec.size() ) != n || int( k2vec.size() ) != n ||
         int( kavec.size() ) != n || int( kgvec.size() ) != n )
    {
        Print( "ERROR: CompVecCurvature01 returned the wrong number of values" );
        __failure++;
    }
    else
    {
        for ( int i = 0 ; i < n ; i++ )
        {
            double k1, k2, ka, kg;

            CompCurvature01( geom_id, 0, uvec[i], wvec[i], k1, k2, ka, kg );

            if ( !closeTo( k1vec[i], k1, 1e-9 ) || !closeTo( k2vec[i], k2, 1e-9 ) ||
                 !closeTo( kavec[i], ka, 1e-9 ) || !closeTo( kgvec[i], kg, 1e-9 ) )
            {
                Print( "ERROR: CompVecCurvature01 disagrees with CompCurvature01 at " + i );
                __failure++;
            }

            if ( !closeTo( kavec[i], 0.5 * ( k1vec[i] + k2vec[i] ), 1e-9 ) ||
                 !closeTo( kgvec[i], k1vec[i] * k2vec[i], 1e-9 ) )
            {
                Print( "ERROR: the curvatures are inconsistent at " + i );
                __failure++;
            }
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)



    k1vec, k2vec, kavec, kgvec = CompVecCurvature01( geom_id, 0, uvec, wvec )

    # One value per coordinate pair, matching what the scalar form gives, and
    # holding the same relationships between the four curvatures.
    assert len( k1vec ) == n, "CompVecCurvature01 returned the wrong number of values"
    assert len( k2vec ) == n, "CompVecCurvature01 returned the wrong number of values"
    assert len( kavec ) == n, "CompVecCurvature01 returned the wrong number of values"
    assert len( kgvec ) == n, "CompVecCurvature01 returned the wrong number of values"

    for i in range(n):
        k1, k2, ka, kg = CompCurvature01( geom_id, 0, uvec[i], wvec[i] )

        assert abs( k1vec[i] - k1 ) < 1e-9, "CompVecCurvature01 disagrees with CompCurvature01 at " + str( i )
        assert abs( k2vec[i] - k2 ) < 1e-9, "CompVecCurvature01 disagrees with CompCurvature01 at " + str( i )
        assert abs( kavec[i] - ka ) < 1e-9, "CompVecCurvature01 disagrees with CompCurvature01 at " + str( i )
        assert abs( kgvec[i] - kg ) < 1e-9, "CompVecCurvature01 disagrees with CompCurvature01 at " + str( i )

        assert abs( kavec[i] - 0.5 * ( k1vec[i] + k2vec[i] ) ) < 1e-9, "the curvatures are inconsistent at " + str( i )
        assert abs( kgvec[i] - k1vec[i] * k2vec[i] ) < 1e-9, "the curvatures are inconsistent at " + str( i )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] us vector<double> Input vector of U (0 - 1) surface coordinates
    \param [in] ws vector<double> Input vector of W (0 - 1) surface coordinates
    \param [out] k1_out_vec vector<double> Output vector of maximum principal curvatures
    \param [out] k2_out_vec vector<double> Output vector of minimum principal curvatures
    \param [out] ka_out_vec vector<double> Output vector of mean curvatures
    \param [out] kg_out_vec vector<double> Output vector of Gaussian curvatures
*/

extern void CompVecCurvature01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &us, const std::vector < double > &ws, std::vector < double > &k1_out_vec, std::vector < double > &k2_out_vec, std::vector < double > &ka_out_vec, std::vector < double > &kg_out_vec);

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest surface coordinates for an input array of 3D coordinate points and calculate the distance between each
    3D point and the closest point of the surface.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, 0, uvec, wvec );

    array< vec3d > normvec = CompVecNorm01( geom_id, 0, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i] = ptvec[i] + normvec[i];
    }

    array<double> uoutv, woutv, doutv;

    ProjVecPnt01( geom_id, 0, ptvec, uoutv, woutv, doutv );

    // Each point was pushed one unit along its own normal, so each projects
    // back to where it came from at a distance of one.
    for( int i = 0 ; i < n ; i++ )
    {
        if ( !closeTo( doutv[i], 1.0, 1e-6 ) || !closeTo( uoutv[i], uvec[i], 1e-6 ) || !closeTo( woutv[i], wvec[i], 1e-6 ) )
        {
            Print( "ERROR: ProjVecPnt01 did not recover point " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    ptvec = CompVecPnt01( geom_id, 0, uvec, wvec )

    normvec = CompVecNorm01( geom_id, 0, uvec, wvec )

    for i in range(n):

        ptvec[i].set_xyz( ptvec[i].x() + normvec[i].x(), ptvec[i].y() + normvec[i].y(), ptvec[i].z() + normvec[i].z() )

    uoutv, woutv, doutv = ProjVecPnt01( geom_id, 0, ptvec )

    \endcode
    \endPythonOnly
    \sa ProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [out] u_out_vec vector<double> Output vector of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] w_out_vec vector<double> Output vector of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] d_out_vec vector<double> Output vector of distances for each 3D point and the closest point of the surface
*/

extern void ProjVecPnt01(const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest surface coordinates for an input array of 3D coordinate points and calculate the distance between each
    3D point and the closest point of the surface. This function takes an input array of surface coordinate guesses for each 3D
    coordinate, offering a potential decrease in computation time compared to ProjVecPnt01.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, 0, uvec, wvec );

    array< vec3d > normvec = CompVecNorm01( geom_id, 0, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i] = ptvec[i] + normvec[i];
    }

    array<double> uoutv, woutv, doutv, u0v, w0v;

    u0v.resize( n );
    w0v.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        u0v[i] = uvec[i] + 0.01234;

        w0v[i] = wvec[i] - 0.05678;
    }

    ProjVecPnt01Guess( geom_id, 0, ptvec, u0v,  w0v,  uoutv, woutv, doutv );

    // Each point was pushed one unit along its own normal, so each projects back
    // to where it came from at a distance of one.  Starting from a guess must
    // not change that.
    array<double> uv_ng, wv_ng, dv_ng;

    ProjVecPnt01( geom_id, 0, ptvec, uv_ng, wv_ng, dv_ng );

    for( int i = 0 ; i < n ; i++ )
    {
        if ( !closeTo( doutv[i], 1.0, 1e-6 ) ||
             !closeTo( uoutv[i], uvec[i], 1e-6 ) ||
             !closeTo( woutv[i], wvec[i], 1e-6 ) )
        {
            Print( "ERROR: ProjVecPnt01Guess did not recover point " + i );
            __failure++;
        }

        if ( !closeTo( uoutv[i], uv_ng[i], 1e-6 ) ||
             !closeTo( woutv[i], wv_ng[i], 1e-6 ) ||
             !closeTo( doutv[i], dv_ng[i], 1e-6 ) )
        {
            Print( "ERROR: the guess changed the answer at " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    ptvec = CompVecPnt01( geom_id, 0, uvec, wvec )

    normvec = CompVecNorm01( geom_id, 0, uvec, wvec )

    for i in range(n):

        ptvec[i].set_xyz( ptvec[i].x() + normvec[i].x(), ptvec[i].y() + normvec[i].y(), ptvec[i].z() + normvec[i].z() )

    u0v = [0]*n
    w0v = [0]*n

    for i in range(n):

        u0v[i] = uvec[i] + 0.01234

        w0v[i] = wvec[i] - 0.05678

    uoutv, woutv, doutv = ProjVecPnt01Guess( geom_id, 0, ptvec, u0v,  w0v )

    # Each point was pushed one unit along its own normal, so each projects back
    # to where it came from at a distance of one.  Starting from a guess must not
    # change that.
    uv_ng, wv_ng, dv_ng = ProjVecPnt01( geom_id, 0, ptvec )

    for i in range(n):
        assert abs( doutv[i] - 1.0 ) < 1e-6, "ProjVecPnt01Guess did not recover point " + str( i )
        assert abs( uoutv[i] - uvec[i] ) < 1e-6, "ProjVecPnt01Guess did not recover point " + str( i )
        assert abs( woutv[i] - wvec[i] ) < 1e-6, "ProjVecPnt01Guess did not recover point " + str( i )

        assert abs( uoutv[i] - uv_ng[i] ) < 1e-6, "the guess changed the answer at " + str( i )
        assert abs( woutv[i] - wv_ng[i] ) < 1e-6, "the guess changed the answer at " + str( i )
        assert abs( doutv[i] - dv_ng[i] ) < 1e-6, "the guess changed the answer at " + str( i )

    \endcode
    \endPythonOnly
    \sa ProjVecPnt01,
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [in] u0s vector<double> Input vector of U (0 - 1) surface coordinate guesses
    \param [in] w0s vector<double> Input vector of W (0 - 1) surface coordinate guesses
    \param [out] u_out_vec vector<double> Output vector of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] w_out_vec vector<double> Output vector of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] d_out_vec vector<double> Output array of distances for each 3D point and the closest point of the surface
*/

extern void ProjVecPnt01Guess(const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, const std::vector < double > &u0s, const std::vector < double > &w0s, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec );


/*!
    \ingroup SurfaceQuery
*/
/*!
    Project an input array of 3D coordinate points onto a surface along a specified axis.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.


    \forcpponly
    \code{.cpp}
       // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );
    int surf_indx = 0;

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, surf_indx, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i].offset_y( -5.0 );
    }

    array<double> uoutv, woutv, doutv;

    AxisProjVecPnt01( geom_id, surf_indx, Y_DIR, ptvec, uoutv, woutv, doutv );

    // Some of these outputs are expected to be non-zero because the projected point is on the opposite side of
    // the pod from the originally computed point.  I.e. there were multiple solutions and the original point
    // is not the closest intersection point.  We could offset those points in the +Y direction instead of -Y.
    for( int i = 0 ; i < n ; i++ )
    {
        Print( i, false );
        Print( "U delta ", false );
        Print( uvec[i] - uoutv[i], false );
        Print( "W delta ", false );
        Print( wvec[i] - woutv[i] );
    }

    // Whichever intersection was found, it has to be a real one: the reported
    // coordinates have to name a point on the surface, and that point has to
    // sit on the same Y ray the test point was offset along.
    for( int i = 0 ; i < n ; i++ )
    {
        if ( uoutv[i] < 0.0 || woutv[i] < 0.0 )
        {
            Print( "ERROR: no intersection was found for point " + i );
            __failure++;
        }
        else
        {
            vec3d hit = CompPnt01( geom_id, surf_indx, uoutv[i], woutv[i] );

            if ( !closeTo( hit.x(), ptvec[i].x(), 1e-6 ) || !closeTo( hit.z(), ptvec[i].z(), 1e-6 ) )
            {
                Print( "ERROR: the intersection left the Y ray at point " + i );
                __failure++;
            }

            if ( !closeTo( doutv[i], abs( hit.y() - ptvec[i].y() ), 1e-6 ) )
            {
                Print( "ERROR: the wrong distance was reported at point " + i );
                __failure++;
            }
        }
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )
    surf_indx = 0

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    ptvec = CompVecPnt01( geom_id, surf_indx, uvec, wvec )

    for i in range(n):

        ptvec[i].offset_y( -5.0 )

    uoutv, woutv, doutv = AxisProjVecPnt01( geom_id, surf_indx, Y_DIR, ptvec )

    # Some of these outputs are expected to be non-zero because the projected point is on the opposite side of
    # the pod from the originally computed point.  I.e. there were multiple solutions and the original point
    # is not the closest intersection point.  We could offset those points in the +Y direction instead of -Y.
    for i in range(n):

        print( i, False )
        print( "U delta ", False )
        print( uvec[i] - uoutv[i], False )
        print( "W delta ", False )
        print( wvec[i] - woutv[i] )

    # Whichever intersection was found, it has to be a real one: the reported
    # coordinates have to name a point on the surface, and that point has to sit
    # on the same Y ray the test point was offset along.
    for i in range(n):

        assert uoutv[i] >= 0.0 and woutv[i] >= 0.0, "no intersection was found for point " + str( i )

        hit = CompPnt01( geom_id, surf_indx, uoutv[i], woutv[i] )

        assert abs( hit.x() - ptvec[i].x() ) < 1e-6, "the intersection left the Y ray at point " + str( i )
        assert abs( hit.z() - ptvec[i].z() ) < 1e-6, "the intersection left the Y ray at point " + str( i )
        assert abs( doutv[i] - abs( hit.y() - ptvec[i].y() ) ) < 1e-6, "the wrong distance was reported at point " + str( i )

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01Guess
    \param [in] geom_id string Geom ID
    \param [in] surf_indx int Main surface index from the Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [out] u_out_vec vector<double> Output vector of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] w_out_vec vector<double> Output vector of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] d_out_vec vector<double> Output vector of axis distances for each 3D point and the projected point of the surface
*/

extern void AxisProjVecPnt01(const std::string &geom_id, const int &surf_indx, const int &iaxis, const std::vector < vec3d > &pts, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Project an input array of 3D coordinate points onto a surface along a specified axis given initial guess arrays of surface parameter.  If the axis-aligned ray from the point intersects the surface multiple times, the nearest intersection is returned.  If the axis-aligned ray from the point does not intersect the surface, the original point is returned and -1 is returned in the other output parameters.  The surface parameter guess should allow this call to be faster than calling AxisProjVecPnt01 without a guess.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );
    int surf_indx = 0;

    int n = 5;

    array<double> uvec, wvec;

    uvec.resize( n );
    wvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        uvec[i] = (i+1)*1.0/(n+1);

        wvec[i] = (n-i)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPnt01( geom_id, surf_indx, uvec, wvec );

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i].offset_y( -5.0 );
    }

    array<double> uoutv, woutv, doutv, u0v, w0v;

    u0v.resize( n );
    w0v.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        u0v[i] = uvec[i] + 0.01234;
        w0v[i] = wvec[i] - 0.05678;
    }

    AxisProjVecPnt01Guess( geom_id, surf_indx, Y_DIR, ptvec, u0v,  w0v,  uoutv, woutv, doutv );

    for( int i = 0 ; i < n ; i++ )
    {
        Print( i, false );
        Print( "U delta ", false );
        Print( uvec[i] - uoutv[i], false );
        Print( "W delta ", false );
        Print( wvec[i] - woutv[i] );
    }

    // Whichever intersection was found, it has to be a real one: the reported
    // coordinates have to name a point on the surface, and that point has to
    // sit on the same Y ray the test point was offset along.
    for( int i = 0 ; i < n ; i++ )
    {
        if ( uoutv[i] < 0.0 || woutv[i] < 0.0 )
        {
            Print( "ERROR: no intersection was found for point " + i );
            __failure++;
        }
        else
        {
            vec3d hit = CompPnt01( geom_id, surf_indx, uoutv[i], woutv[i] );

            if ( !closeTo( hit.x(), ptvec[i].x(), 1e-6 ) || !closeTo( hit.z(), ptvec[i].z(), 1e-6 ) )
            {
                Print( "ERROR: the intersection left the Y ray at point " + i );
                __failure++;
            }

            if ( !closeTo( doutv[i], abs( hit.y() - ptvec[i].y() ), 1e-6 ) )
            {
                Print( "ERROR: the wrong distance was reported at point " + i );
                __failure++;
            }
        }
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )
    surf_indx = 0

    n = 5

    uvec = [0]*n
    wvec = [0]*n

    for i in range(n):

        uvec[i] = (i+1)*1.0/(n+1)

        wvec[i] = (n-i)*1.0/(n+1)

    ptvec = CompVecPnt01( geom_id, surf_indx, uvec, wvec )

    for i in range(n):

        ptvec[i].offset_y( -5.0 )

    u0v = [0]*n
    w0v = [0]*n

    for i in range(n):

        u0v[i] = uvec[i] + 0.01234
        w0v[i] = wvec[i] - 0.05678

    uoutv, woutv, doutv = AxisProjVecPnt01Guess( geom_id, surf_indx, Y_DIR, ptvec, u0v,  w0v )

    for i in range(n):

        print( i, False )
        print( "U delta ", False )
        print( uvec[i] - uoutv[i], False )
        print( "W delta ", False )
        print( wvec[i] - woutv[i] )

    # Whichever intersection was found, it has to be a real one: the reported
    # coordinates have to name a point on the surface, and that point has to sit
    # on the same Y ray the test point was offset along.
    for i in range(n):

        assert uoutv[i] >= 0.0 and woutv[i] >= 0.0, "no intersection was found for point " + str( i )

        hit = CompPnt01( geom_id, surf_indx, uoutv[i], woutv[i] )

        assert abs( hit.x() - ptvec[i].x() ) < 1e-6, "the intersection left the Y ray at point " + str( i )
        assert abs( hit.z() - ptvec[i].z() ) < 1e-6, "the intersection left the Y ray at point " + str( i )
        assert abs( doutv[i] - abs( hit.y() - ptvec[i].y() ) ) < 1e-6, "the wrong distance was reported at point " + str( i )

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [in] u0s vector<double> Input vector of U (0 - 1) surface coordinate guesses
    \param [in] w0s vector<double> Input vector of W (0 - 1) surface coordinate guesses
    \param [out] u_out_vec vector<double> Output vector of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] w_out_vec vector<double> Output vector of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] d_out_vec vector<double> Output vector of axis distances for each 3D point and the projected point of the surface
*/

extern void AxisProjVecPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const std::vector < vec3d > &pts, const std::vector < double > &u0s, const std::vector < double > &w0s, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < double > &d_out_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Test whether a vector of points are inside a specified surface.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*1.0/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    array<bool> res;
    res = VecInsideSurf( geom_id, surf_indx, ptvec );

    // Every point came from CompVecPntRST with r below one, so all are inside.
    for( int i = 0 ; i < int( res.length() ) ; i++ )
    {
        if ( !res[i] )
        {
            Print( "ERROR: VecInsideSurf says interior point " + i + " is outside" );
            __failure++;
        }
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    n = 5

    rvec = [0]*n
    svec = [0]*n
    tvec = [0]*n

    for i in range(n):

        rvec[i] = (i+1)*1.0/(n+1)

        svec[i] = (n-i)*1.0/(n+1)

        tvec[i] = (i+1)*1.0/(n+1)

    ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec )


    res = VecInsideSurf( geom_id, surf_indx, ptvec )

    # Every point came from CompVecPntRST with r below one, so all are inside.
    for i in range( len( res ) ):
        assert res[i], "VecInsideSurf says an interior point is outside"


    \endcode
    \endPythonOnly
    \sa VecInsideSurf
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \return vector \<bool\> Boolean vector for each point.  True if it is inside the surface, false otherwise.
*/

extern std::vector < bool > VecInsideSurf( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts );


/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine 3D coordinate for each volume coordinate point in the input arrays
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*1.0/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    // One point per coordinate triple, each the same point the scalar form
    // gives, and every one of them inside the surface.
    if ( int( ptvec.size() ) != n )
    {
        Print( "ERROR: CompVecPntRST returned the wrong number of points" );
        __failure++;
    }
    else
    {
        for( int i = 0 ; i < n ; i++ )
        {
            if ( dist( ptvec[i], CompPntRST( geom_id, 0, rvec[i], svec[i], tvec[i] ) ) > 1e-9 )
            {
                Print( "ERROR: CompVecPntRST disagrees with CompPntRST at " + i );
                __failure++;
            }

            if ( !InsideSurf( geom_id, 0, ptvec[i] ) )
            {
                Print( "ERROR: CompVecPntRST returned a point outside the surface at " + i );
                __failure++;
            }
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    rvec = [0]*n
    svec = [0]*n
    tvec = [0]*n

    for i in range(n):

        rvec[i] = (i+1)*1.0/(n+1)

        svec[i] = (n-i)*1.0/(n+1)

        tvec[i] = (i+1)*1.0/(n+1)

    ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec )

    # One point per coordinate triple, each the same point the scalar form
    # gives, and every one of them inside the surface.
    assert len( ptvec ) == n, "CompVecPntRST returned the wrong number of points"

    for i in range(n):
        assert dist( ptvec[i], CompPntRST( geom_id, 0, rvec[i], svec[i], tvec[i] ) ) < 1e-9, "CompVecPntRST disagrees with CompPntRST at " + str( i )
        assert InsideSurf( geom_id, 0, ptvec[i] ), "CompVecPntRST returned a point outside the surface at " + str( i )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r_in_vec vector<double> Input vector of R (0 - 1.0) volume coordinates
    \param [in] s_in_vec vector<double> Input vector of S (0 - 1.0) volume coordinates
    \param [in] t_in_vec vector<double> Input vector of T (0 - 1.0) volume coordinates
    \return vector\<vec3d\> Vector of 3D coordinate points
*/

extern std::vector < vec3d > CompVecPntRST( const std::string &geom_id, const int &surf_indx, const std::vector < double > &r_in_vec, const std::vector < double > &s_in_vec, const std::vector < double > &t_in_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest volume coordinates for an input array of 3D coordinate points and calculate the distance between each
    3D point and the found point in the volume.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*1.0/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    array<double> routv, soutv, toutv, doutv;

    FindRSTVec( geom_id, 0, ptvec, routv, soutv, toutv, doutv );

    // Every point came from CompVecPntRST, so each search lands back on its own.
    for( int i = 0 ; i < n ; i++ )
    {
        if ( !closeTo( doutv[i], 0.0, 1e-6 ) || !closeTo( routv[i], rvec[i], 1e-6 ) ||
             !closeTo( soutv[i], svec[i], 1e-6 ) || !closeTo( toutv[i], tvec[i], 1e-6 ) )
        {
            Print( "ERROR: FindRSTVec did not recover point " + i );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    rvec = [0]*n
    svec = [0]*n
    tvec = [0]*n

    for i in range(n):

        rvec[i] = (i+1)*1.0/(n+1)

        svec[i] = (n-i)*1.0/(n+1)

        tvec[i] = (i+1)*1.0/(n+1)

    ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec )



    routv, soutv, toutv, doutv = FindRSTVec( geom_id, 0, ptvec )

    # Every point came from CompVecPntRST, so each search lands back on its own.
    for i in range( n ):
        assert abs( doutv[i] ) < 1e-6, "FindRSTVec distance"
        assert abs( routv[i] - rvec[i] ) < 1e-6, "FindRSTVec r"
        assert abs( soutv[i] - svec[i] ) < 1e-6, "FindRSTVec s"
        assert abs( toutv[i] - tvec[i] ) < 1e-6, "FindRSTVec t"

    \endcode
    \endPythonOnly
    \sa FindRSTVecGuess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [out] r_out_vec vector<double> Output vector of the closest R (0 - 1.0) volume coordinate for each 3D input point
    \param [out] s_out_vec vector<double> Output vector of the closest S (0 - 1.0) volume coordinate for each 3D input point
    \param [out] t_out_vec vector<double> Output vector of the closest T (0 - 1.0) volume coordinate for each 3D input point
    \param [out] d_out_vec vector<double> Output vector of distances for each 3D point and the closest point of the volume
*/

extern void FindRSTVec( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, std::vector < double > &r_out_vec, std::vector < double > &s_out_vec, std::vector < double > &t_out_vec, std::vector < double > &d_out_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Determine the nearest volume coordinates for an input array of 3D coordinate points and calculate the distance between each
    3D point and the closest point of the volume. This function takes an input array of volume coordinate guesses for each 3D
    coordinate, offering a potential decrease in computation time compared to FindRSTVec.
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);

        svec[i] = (n-i)*1.0/(n+1);

        tvec[i] = (i+1)*1.0/(n+1);
    }

    array< vec3d > ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec );

    array<double> routv, soutv, toutv, doutv;

    for( int i = 0 ; i < n ; i++ )
    {
        ptvec[i] = ptvec[i] * 0.9;
    }

    FindRSTVecGuess( geom_id, 0, ptvec, rvec, svec, tvec, routv, soutv, toutv, doutv );

    // The points above were scaled off the surface on purpose, so the search
    // does not return to the original r, s, t.  What must hold is that every
    // point got an answer and that the reported distances are real.
    if ( int( routv.length() ) != n || int( soutv.length() ) != n ||
         int( toutv.length() ) != n || int( doutv.length() ) != n )
    {
        Print( "ERROR: FindRSTVecGuess returned the wrong number of results" );
        __failure++;
    }

    for( int i = 0 ; i < int( doutv.length() ) ; i++ )
    {
        if ( doutv[i] < 0.0 )
        {
            Print( "ERROR: FindRSTVecGuess distance " + i + " is negative" );
            __failure++;
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    rvec = [0]*n
    svec = [0]*n
    tvec = [0]*n

    for i in range(n):

        rvec[i] = (i+1)*1.0/(n+1)

        svec[i] = (n-i)*1.0/(n+1)

        tvec[i] = (i+1)*1.0/(n+1)

    ptvec = CompVecPntRST( geom_id, 0, rvec, svec, tvec )

    for i in range(n):

        ptvec[i].set_xyz(ptvec[i].x() * 0.9, ptvec[i].y() * 0.9, ptvec[i].z() * 0.9)

     routv, soutv, toutv, doutv = FindRSTVecGuess( geom_id, 0, ptvec, rvec, svec, tvec )

     # The points above were scaled off the surface on purpose, so the search
     # does not return to the original r, s, t.  What must hold is that every
     # point got an answer and that the reported distances are real.
     assert len( routv ) == n and len( soutv ) == n and len( toutv ) == n and len( doutv ) == n, "FindRSTVecGuess result count"
     for i in range( len( doutv ) ):
         assert doutv[i] >= 0.0, "FindRSTVecGuess distance"

    \endcode
    \endPythonOnly
    \sa FindRSTVec,
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [in] r0s vector<double> Input vector of U (0 - 1.0) volume coordinate guesses
    \param [in] s0s vector<double> Input vector of S (0 - 1.0) volume coordinate guesses
    \param [in] t0s vector<double> Input vector of T (0 - 1.0) volume coordinate guesses
    \param [out] r_out_vec vector<double> Output vector of the closest R (0 - 1.0) volume coordinate for each 3D input point
    \param [out] s_out_vec vector<double> Output vector of the closest S (0 - 1.0) volume coordinate for each 3D input point
    \param [out] t_out_vec vector<double> Output vector of the closest T (0 - 1.0) volume coordinate for each 3D input point
    \param [out] d_out_vec vector<double> Output vector of distances for each 3D point and the closest point of the volume
*/

extern void FindRSTVecGuess( const std::string &geom_id, const int &surf_indx, const std::vector < vec3d > &pts, const std::vector < double > &r0s, const std::vector < double > &s0s, const std::vector < double > &t0s, std::vector < double > &r_out_vec, std::vector < double > &s_out_vec, std::vector < double > &t_out_vec, std::vector < double > &d_out_vec );


/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert vector of RST volumetric coordinates to LMN coordinates.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> rvec, svec, tvec;

    rvec.resize( n );
    svec.resize( n );
    tvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        rvec[i] = (i+1)*1.0/(n+1);
        svec[i] = (n-i)*1.0/(n+1);
        tvec[i] = (i+1)*1.0/(n+1);
    }

    array<double> lvec, mvec, nvec;

    ConvertRSTtoLMNVec( geom_id, 0, rvec, svec, tvec, lvec, mvec, nvec );

    // Converting back has to return the coordinates we started from.
    array<double> rback, sback, tback;
    ConvertLMNtoRSTVec( geom_id, 0, lvec, mvec, nvec, rback, sback, tback );

    for( int i = 0 ; i < n ; i++ )
    {
        if ( !closeTo( rback[i], rvec[i], 1e-6 ) || !closeTo( sback[i], svec[i], 1e-6 ) || !closeTo( tback[i], tvec[i], 1e-6 ) )
        {
            Print( "ERROR: ConvertRSTtoLMNVec does not round trip at " + i );
            __failure++;
        }
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    rvec = [0]*n
    svec = [0]*n
    tvec = [0]*n

    for i in range(n):

        rvec[i] = (i+1)*1.0/(n+1)
        svec[i] = (n-i)*1.0/(n+1)
        tvec[i] = (i+1)*1.0/(n+1)



    lvec, mvec, nvec = ConvertRSTtoLMNVec( geom_id, 0, rvec, svec, tvec )


    \endcode
    \endPythonOnly
    \sa ConvertLMNtoRSTVec, ConvertRSTtoLMN, ConvertLMNtoRST
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r_vec vector<double> Input vector of R (0 - 1) volumetric coordinate
    \param [in] s_vec vector<double> Input vector of S (0 - 1) volumetric coordinate
    \param [in] t_vec vector<double> Input vector of T (0 - 1) volumetric coordinate
    \param [out] l_out_vec vector<double> Output vector of L (0 - 1) linear volumetric coordinate
    \param [out] m_out_vec vector<double> Output vector of M (0 - 1) linear volumetric coordinate
    \param [out] n_out_vec vector<double> Output vector of N (0 - 1) linear volumetric coordinate
*/

extern void ConvertRSTtoLMNVec( const std::string &geom_id, const int &surf_indx, const std::vector < double > &r_vec, const std::vector < double > &s_vec, const std::vector < double > &t_vec,
                                std::vector < double > &l_out_vec, std::vector < double > &m_out_vec, std::vector < double > &n_out_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Convert vector of LMN volumetric coordinates to RST coordinates.

    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int n = 5;

    array<double> lvec, mvec, nvec;

    lvec.resize( n );
    mvec.resize( n );
    nvec.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        lvec[i] = (i+1)*1.0/(n+1);
        mvec[i] = (n-i)*1.0/(n+1);
        nvec[i] = (i+1)*1.0/(n+1);
    }

    array<double> rvec, svec, tvec;

    ConvertLMNtoRSTVec( geom_id, 0, lvec, mvec, nvec, rvec, svec, tvec );

    // One triple out per triple in, each matching the scalar form, and the whole
    // conversion invertible.
    array<double> lback, mback, nback;

    ConvertRSTtoLMNVec( geom_id, 0, rvec, svec, tvec, lback, mback, nback );

    if ( int( rvec.size() ) != n || int( svec.size() ) != n || int( tvec.size() ) != n )
    {
        Print( "ERROR: ConvertLMNtoRSTVec returned the wrong number of values" );
        __failure++;
    }
    else
    {
        for( int i = 0 ; i < n ; i++ )
        {
            double r_one, s_one, t_one;

            ConvertLMNtoRST( geom_id, 0, lvec[i], mvec[i], nvec[i], r_one, s_one, t_one );

            if ( !closeTo( rvec[i], r_one, 1e-9 ) || !closeTo( svec[i], s_one, 1e-9 ) || !closeTo( tvec[i], t_one, 1e-9 ) )
            {
                Print( "ERROR: ConvertLMNtoRSTVec disagrees with ConvertLMNtoRST at " + i );
                __failure++;
            }

            if ( !closeTo( lback[i], lvec[i], 1e-6 ) || !closeTo( mback[i], mvec[i], 1e-6 ) || !closeTo( nback[i], nvec[i], 1e-6 ) )
            {
                Print( "ERROR: ConvertLMNtoRSTVec does not invert at " + i );
                __failure++;
            }
        }
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    n = 5

    lvec = [0]*n
    mvec = [0]*n
    nvec = [0]*n

    for i in range(n):

        lvec[i] = (i+1)*1.0/(n+1)
        mvec[i] = (n-i)*1.0/(n+1)
        nvec[i] = (i+1)*1.0/(n+1)

    rvec, svec, tvec = ConvertLMNtoRSTVec( geom_id, 0, lvec, mvec, nvec )

    # One triple out per triple in, each matching the scalar form, and the whole
    # conversion invertible.
    lback, mback, nback = ConvertRSTtoLMNVec( geom_id, 0, rvec, svec, tvec )

    assert len( rvec ) == n, "ConvertLMNtoRSTVec returned the wrong number of values"
    assert len( svec ) == n, "ConvertLMNtoRSTVec returned the wrong number of values"
    assert len( tvec ) == n, "ConvertLMNtoRSTVec returned the wrong number of values"

    for i in range(n):
        r_one, s_one, t_one = ConvertLMNtoRST( geom_id, 0, lvec[i], mvec[i], nvec[i] )

        assert abs( rvec[i] - r_one ) < 1e-9, "ConvertLMNtoRSTVec disagrees with ConvertLMNtoRST at " + str( i )
        assert abs( svec[i] - s_one ) < 1e-9, "ConvertLMNtoRSTVec disagrees with ConvertLMNtoRST at " + str( i )
        assert abs( tvec[i] - t_one ) < 1e-9, "ConvertLMNtoRSTVec disagrees with ConvertLMNtoRST at " + str( i )

        assert abs( lback[i] - lvec[i] ) < 1e-6, "ConvertLMNtoRSTVec does not invert at " + str( i )
        assert abs( mback[i] - mvec[i] ) < 1e-6, "ConvertLMNtoRSTVec does not invert at " + str( i )
        assert abs( nback[i] - nvec[i] ) < 1e-6, "ConvertLMNtoRSTVec does not invert at " + str( i )

    \endcode
    \endPythonOnly
    \sa ConvertRSTtoLMNVec, ConvertRSTtoLMN, ConvertLMNtoRST
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] l_vec vector<double> Input vector of L (0 - 1) linear volumetric coordinate
    \param [in] m_vec vector<double> Input vector of M (0 - 1) linear volumetric coordinate
    \param [in] n_vec vector<double> Input vector of N (0 - 1) linear volumetric coordinate
    \param [out] r_out_vec vector<double> Output vector of R (0 - 1) volumetric coordinate
    \param [out] s_out_vec vector<double> Output vector of S (0 - 1) volumetric coordinate
    \param [out] t_out_vec vector<double> Output vector of T (0 - 1) volumetric coordinate

*/

extern void ConvertLMNtoRSTVec( const std::string &geom_id, const int &surf_indx, const std::vector < double > &l_vec, const std::vector < double > &m_vec, const std::vector < double > &n_vec,
                                std::vector < double > &r_out_vec, std::vector < double > &s_out_vec, std::vector < double > &t_out_vec );

/*!
    \ingroup SurfaceQuery
*/
/*!
    Get the surface coordinate point of each intersection of the tessellated wireframe for a particular surface
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string geom_id = AddGeom( "POD", "" );

    int surf_indx = 0;

    array<double> utess, wtess;

    GetUWTess01( geom_id, surf_indx, utess, wtess );

    // The wireframe spans the whole surface and never runs backwards.
    if ( utess.size() < 2 || wtess.size() < 2 )
    {
        Print( "ERROR: GetUWTess01 returned too few stations" );
        __failure++;
    }
    else
    {
        if ( !closeTo( utess[0], 0.0, 1e-9 ) || !closeTo( utess[utess.size() - 1], 1.0, 1e-9 ) ||
             !closeTo( wtess[0], 0.0, 1e-9 ) || !closeTo( wtess[wtess.size() - 1], 1.0, 1e-9 ) )
        {
            Print( "ERROR: GetUWTess01 does not span the surface" );
            __failure++;
        }

        for ( int i = 1; i < int( utess.size() ); i++ )
        {
            if ( utess[i] <= utess[i - 1] )
            {
                Print( "ERROR: the U stations are not increasing at " + i );
                __failure++;
            }
        }

        for ( int i = 1; i < int( wtess.size() ); i++ )
        {
            if ( wtess[i] <= wtess[i - 1] )
            {
                Print( "ERROR: the W stations are not increasing at " + i );
                __failure++;
            }
        }
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    utess, wtess = GetUWTess01( geom_id, surf_indx )

    # The wireframe spans the whole surface and never runs backwards.
    assert len( utess ) >= 2, "GetUWTess01 returned too few stations"
    assert len( wtess ) >= 2, "GetUWTess01 returned too few stations"

    assert abs( utess[0] ) < 1e-9 and abs( utess[-1] - 1.0 ) < 1e-9, "GetUWTess01 does not span the surface"
    assert abs( wtess[0] ) < 1e-9 and abs( wtess[-1] - 1.0 ) < 1e-9, "GetUWTess01 does not span the surface"

    for i in range( 1, len( utess ) ):
        assert utess[i] > utess[i - 1], "the U stations are not increasing at " + str( i )

    for i in range( 1, len( wtess ) ):
        assert wtess[i] > wtess[i - 1], "the W stations are not increasing at " + str( i )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [out] u_out_vec vector<double> Output vector of U (0 - 1) surface coordinates
    \param [out] w_out_vec vector<double> Output vector of W (0 - 1) surface coordinates
*/

extern void GetUWTess01(const std::string &geom_id, const int &surf_indx, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec);


//======================= Measure Functions ============================//
/*!
    \ingroup Measure
*/
/*!
    Create a new Ruler and add it to the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    SetParmVal( FindParm( rid, "X_Offset", "Measure" ), 6.0 );

    if ( rid.length() == 0 || rid == "NONE" )
    {
        Print( "ERROR: AddRuler returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    pid2 = AddGeom( "POD", "" )

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 )

    rid = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" )

    SetParmVal( FindParm( rid, "X_Offset", "Measure" ), 6.0 )

    \endcode
    \endPythonOnly
    \param [in] startgeomid string Start parent Geom ID
    \param [in] startsurfindx int Main surface index from the staring parent Geom
    \param [in] startu double Surface u (0 - 1) start coordinate
    \param [in] startw double Surface w (0 - 1) start coordinate
    \param [in] endgeomid string End parent Geom ID
    \param [in] endsurfindx int Main surface index on the end parent Geom
    \param [in] endu double Surface u (0 - 1) end coordinate
    \param [in] endw double Surface w (0 - 1) end coordinate
    \param [in] name string Ruler name
    \return string Ruler ID
*/

extern string AddRuler( const string & startgeomid, int startsurfindx, double startu, double startw,
                        const string & endgeomid, int endsurfindx, double endu, double endw, const string & name );
/*!
    \ingroup Measure
*/
/*!
    Get an array of all Ruler IDs from the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    string rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" );

    array< string > @ruler_array = GetAllRulers();
    if ( ruler_array.length() == 0 )
    {
        Print( "ERROR: GetAllRulers returned nothing" );
        __failure++;
    }

    Print("Two Rulers");

    for( int n = 0 ; n < int( ruler_array.length() ) ; n++ )
    {
        Print( ruler_array[n] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    pid2 = AddGeom( "POD", "" )

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 )

    rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" )

    rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" )

    ruler_array = GetAllRulers()
    assert len( ruler_array ) > 0, "GetAllRulers returned nothing"

    print("Two Rulers")

    for n in range(len(ruler_array)):

        print( ruler_array[n] )

    \endcode
    \endPythonOnly
    \return vector\<string\> Vector of Ruler IDs
*/

extern std::vector < string > GetAllRulers();

/*!
    \ingroup Measure
*/
/*!
    Delete a particular Ruler from the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    string rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" );

    array< string > @ruler_array = GetAllRulers();

    int num_before_del = GetAllRulers().length();
    DelRuler( ruler_array[0] );
    if ( GetAllRulers().length() >= num_before_del )
    {
        Print( "ERROR: DelRuler removed nothing" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    pid2 = AddGeom( "POD", "" )

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 )

    rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" )

    rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" )

    ruler_array = GetAllRulers()

    num_before_del = len( GetAllRulers() )
    DelRuler( ruler_array[0] )
    assert len( GetAllRulers() ) < num_before_del, "DelRuler removed nothing"


    \endcode
    \endPythonOnly
    \param [in] id string Ruler ID
*/

extern void DelRuler( const string &id );

/*!
    \ingroup Measure
*/
/*!
    Delete all Rulers from the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string pid2 = AddGeom( "POD", "" );

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 );

    string rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" );

    string rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" );

    DeleteAllRulers();
    if ( GetAllRulers().length() != 0 )
    {
        Print( "ERROR: DeleteAllRulers left something behind" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    pid2 = AddGeom( "POD", "" )

    SetParmVal( pid2, "Z_Rel_Location", "XForm", 4.0 )

    rid1 = AddRuler( pid1, 1, 0.2, 0.3, pid2, 0, 0.2, 0.3, "Ruler 1" )

    rid2 = AddRuler( pid1, 0, 0.4, 0.6, pid1, 1, 0.8, 0.9, "Ruler 2" )

    DeleteAllRulers()
    assert len( GetAllRulers() ) == 0, "DeleteAllRulers left something behind"


    \endcode
    \endPythonOnly
*/

extern void DeleteAllRulers();


/*!
    \ingroup Measure
*/
/*!
    Create a new Probe and add it to the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );

    SetParmVal( FindParm( probe_id, "Len", "Measure" ), 3.0 );

    if ( probe_id.length() == 0 || probe_id == "NONE" )
    {
        Print( "ERROR: AddProbe returned no id" );
        __failure++;
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    probe_id = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" )

    SetParmVal( FindParm( probe_id, "Len", "Measure" ), 3.0 )

    \endcode
    \endPythonOnly
    \param [in] geomid string Parent Geom ID
    \param [in] surfindx int Main surface index from the parent Geom
    \param [in] u double Surface u (0 - 1) coordinate
    \param [in] w double Surface w (0 - 1) coordinate
    \param [in] name string Probe name
    \return string Probe ID
*/

extern string AddProbe( const string & geomid, int surfindx, double u, double w, const string & name );

/*!
    \ingroup Measure
*/
/*!
    Get an array of all Probe IDs from the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );

    array< string > @probe_array = GetAllProbes();
    if ( probe_array.length() == 0 )
    {
        Print( "ERROR: GetAllProbes returned nothing" );
        __failure++;
    }

    Print( "One Probe: ", false );

    Print( probe_array[0] );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    probe_id = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" )

    probe_array = GetAllProbes()
    assert len( probe_array ) > 0, "GetAllProbes returned nothing"

    print( "One Probe: ", False )

    print( probe_array[0] )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of Probe IDs
*/

extern std::vector < string > GetAllProbes();

/*!
    \ingroup Measure
*/
/*!
    Delete a specific Probe from the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id_1 = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );
    string probe_id_2 = AddProbe( pid1, 0, 0.2, 0.3, "Probe 2" );

    DelProbe( probe_id_1 );

    array< string > @probe_array = GetAllProbes();

    if ( probe_array.size() != 1 ) { Print( "Error: DelProbe" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    probe_id_1 = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" )
    probe_id_2 = AddProbe( pid1, 0, 0.2, 0.3, "Probe 2" )

    DelProbe( probe_id_1 )

    probe_array = GetAllProbes()

    if  len(probe_array) != 1 :
        print( "Error: DelProbe" )
        assert False, "Error: DelProbe"

    \endcode
    \endPythonOnly
    \param [in] id string Probe ID
*/

extern void DelProbe( const string &id );

/*!
    \ingroup Measure
*/
/*!
    Delete all Probes from the Measure Tool
    \forcpponly
    \code{.cpp}
    string pid1 = AddGeom( "POD", "" );

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 );

    string probe_id_1 = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" );
    string probe_id_2 = AddProbe( pid1, 0, 0.2, 0.3, "Probe 2" );

    DeleteAllProbes();

    array< string > @probe_array = GetAllProbes();

    if ( probe_array.size() != 0 ) { Print( "Error: DeleteAllProbes" ); __failure++; }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pid1 = AddGeom( "POD", "" )

    SetParmVal( pid1, "Y_Rel_Location", "XForm", 2.0 )

    probe_id_1 = AddProbe( pid1, 0, 0.5, 0.8, "Probe 1" )
    probe_id_2 = AddProbe( pid1, 0, 0.2, 0.3, "Probe 2" )

    DeleteAllProbes()

    probe_array = GetAllProbes()

    if  len(probe_array) != 0 :
        print( "Error: DeleteAllProbes" )
        assert False, "Error: DeleteAllProbes"

    \endcode
    \endPythonOnly
*/

extern void DeleteAllProbes();


//======================= Advanced Link Functions ============================//

/*!
    \ingroup AdvancedLink
*/
/*!
    Get an array of all advanced link names
    \forcpponly
    \code{.cpp}
    //==== Set up an advanced link so there is something to find ====//
    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    array< string > @link_array = GetAdvLinkNames();
    if ( link_array.length() == 0 )
    {
        Print( "ERROR: GetAdvLinkNames returned nothing" );
        __failure++;
    }

    for( int n = 0 ; n < int( link_array.length() ) ; n++ )
    {
        Print( link_array[n] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Set up an advanced link so there is something to find ====//
    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    link_array = GetAdvLinkNames()
    assert len( link_array ) > 0, "GetAdvLinkNames returned nothing"

    for n in range(len(link_array) ):

        print( link_array[n] )

    \endcode
    \endPythonOnly
    \return vector\<string\> Array of advanced link names
*/

extern std::vector< std::string > GetAdvLinkNames();

/*!
    \ingroup AdvancedLink
*/
/*!
    Find the index of a specific advanced link.
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] name string Name for advanced link
    \return int index for advanced link

*/

extern int GetLinkIndex( const string & name );

/*!
    \ingroup AdvancedLink
*/
/*!
    Delete an advanced link specified by index
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    int num_before_del = GetAdvLinkNames().length();
    DelAdvLink( indx );
    if ( GetAdvLinkNames().length() >= num_before_del )
    {
        Print( "ERROR: DelAdvLink removed nothing" );
        __failure++;
    }


    array< string > @link_array = GetAdvLinkNames();

    // Should print nothing.
    for( int n = 0 ; n < int( link_array.length() ) ; n++ )
    {
        Print( link_array[n] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    num_before_del = len( GetAdvLinkNames() )
    DelAdvLink( indx )
    assert len( GetAdvLinkNames() ) < num_before_del, "DelAdvLink removed nothing"


    link_array = GetAdvLinkNames()

    # Should print nothing.
    for n in range(len(link_array) ):

        print( link_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
*/

extern void DelAdvLink( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Delete all advanced links
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    DelAllAdvLinks();

    array< string > @link_array = GetAdvLinkNames();

    // Should print nothing.
    for( int n = 0 ; n < int( link_array.length() ) ; n++ )
    {
        Print( link_array[n] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    DelAllAdvLinks()

    link_array = GetAdvLinkNames()

    # Should print nothing.
    for n in range( len(link_array) ):

        print( link_array[n] )


    \endcode
    \endPythonOnly
*/

extern void DelAllAdvLinks();

/*!
    \ingroup AdvancedLink
*/
/*!
    Add an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] name string Name for advanced link
*/

extern void AddAdvLink( const string & name );

/*!
    \ingroup AdvancedLink
*/
/*!
    Add an input variable to an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \param [in] parm_id string Parameter ID for advanced link input variable
    \param [in] var_name string Name for advanced link input variable
*/

extern void AddAdvLinkInput( int index, const string & parm_id, const string & var_name );

/*!
    \ingroup AdvancedLink
*/
/*!
    Add an output variable to an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \param [in] parm_id string Parameter ID for advanced link output variable
    \param [in] var_name string Name for advanced link output variable
*/

extern void AddAdvLinkOutput( int index, const string & parm_id, const string & var_name );

/*!
    \ingroup AdvancedLink
*/
/*!
    Delete an input variable from an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );
    string y_pos = GetParm( pod, "Y_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );
    AddAdvLinkInput( indx, y_pos, "y" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    int num_before_del = GetAdvLinkInputNames( indx ).length();
    DelAdvLinkInput( indx, "y" );
    if ( GetAdvLinkInputNames( indx ).length() >= num_before_del )
    {
        Print( "ERROR: DelAdvLinkInput removed nothing" );
        __failure++;
    }


    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )
    y_pos = GetParm( pod, "Y_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )
    AddAdvLinkInput( indx, y_pos, "y" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    num_before_del = len( GetAdvLinkInputNames( indx ) )
    DelAdvLinkInput( indx, "y" )
    assert len( GetAdvLinkInputNames( indx ) ) < num_before_del, "DelAdvLinkInput removed nothing"


    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \param [in] var_name string Name for advanced link input variable to delete
*/

extern void DelAdvLinkInput( int index, const string & var_name );

/*!
    \ingroup AdvancedLink
*/
/*!
    Delete an output variable from an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );
    string y_pos = GetParm( pod, "Y_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );
    AddAdvLinkOutput( indx, y_pos, "y" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    int num_before_del = GetAdvLinkOutputNames( indx ).length();
    DelAdvLinkOutput( indx, "y" );
    if ( GetAdvLinkOutputNames( indx ).length() >= num_before_del )
    {
        Print( "ERROR: DelAdvLinkOutput removed nothing" );
        __failure++;
    }


    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )
    y_pos = GetParm( pod, "Y_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )
    AddAdvLinkOutput( indx, y_pos, "y" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    num_before_del = len( GetAdvLinkOutputNames( indx ) )
    DelAdvLinkOutput( indx, "y" )
    assert len( GetAdvLinkOutputNames( indx ) ) < num_before_del, "DelAdvLinkOutput removed nothing"


    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \param [in] var_name string Name for advanced link output variable to delete
*/

extern void DelAdvLinkOutput( int index, const string & var_name );

/*!
    \ingroup AdvancedLink
*/
/*!
    Get the name of all the inputs to a specified advanced link index
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    array< string > @name_array = GetAdvLinkInputNames( indx );
    if ( name_array.length() == 0 )
    {
        Print( "ERROR: GetAdvLinkInputNames returned nothing" );
        __failure++;
    }

    for( int n = 0 ; n < int( name_array.length() ) ; n++ )
    {
        Print( name_array[n] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    name_array = GetAdvLinkInputNames( indx )
    assert len( name_array ) > 0, "GetAdvLinkInputNames returned nothing"

    for n in range(len(name_array) ):

        print( name_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return vector\<string\> Array of advanced link input names
*/

extern std::vector< std::string > GetAdvLinkInputNames( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Get the Parm IDs of all the inputs to a specified advanced link index
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    array< string > @parm_array = GetAdvLinkInputParms( indx );
    if ( parm_array.length() == 0 )
    {
        Print( "ERROR: GetAdvLinkInputParms returned nothing" );
        __failure++;
    }

    for( int n = 0 ; n < int( parm_array.length() ) ; n++ )
    {
        Print( parm_array[n] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    parm_array = GetAdvLinkInputParms( indx )
    assert len( parm_array ) > 0, "GetAdvLinkInputParms returned nothing"

    for n in range( len(parm_array) ):

        print( parm_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return vector\<string\> Array of advanced link input Parm IDs
*/

extern std::vector< std::string > GetAdvLinkInputParms( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Get the Parm IDs of all the outputs to a specified advanced link index
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    array< string > @name_array = GetAdvLinkOutputNames( indx );
    if ( name_array.length() == 0 )
    {
        Print( "ERROR: GetAdvLinkOutputNames returned nothing" );
        __failure++;
    }

    for( int n = 0 ; n < int( name_array.length() ) ; n++ )
    {
        Print( name_array[n] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    name_array = GetAdvLinkOutputNames( indx )
    assert len( name_array ) > 0, "GetAdvLinkOutputNames returned nothing"

    for n in range( len(name_array) ):

        print( name_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return vector\<string\> Array of advanced link output names
*/

extern std::vector< std::string > GetAdvLinkOutputNames( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Get the Parm IDs of all the outputs to a specified advanced link index
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    array< string > @parm_array = GetAdvLinkOutputParms( indx );
    if ( parm_array.length() == 0 )
    {
        Print( "ERROR: GetAdvLinkOutputParms returned nothing" );
        __failure++;
    }

    for( int n = 0 ; n < int( parm_array.length() ) ; n++ )
    {
        Print( parm_array[n] );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    parm_array = GetAdvLinkOutputParms( indx )
    assert len( parm_array ) > 0, "GetAdvLinkOutputParms returned nothing"

    for n in range( len(parm_array) ):

        print( parm_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return vector\<string\> Array of advanced link output Parm IDs
*/

extern std::vector< std::string > GetAdvLinkOutputParms( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Validate the input and output parameters for an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    bool valid = ValidateAdvLinkParms( indx );

    // The link built above is well formed, so this has to come back true.
    if ( !valid )
    {
        Print( "ERROR: ValidateAdvLinkParms did not report success" );
        __failure++;
    }

    if ( valid )
    {
        Print( "Advanced link Parms are valid." );
    }
    else
    {
        Print( "Advanced link Parms are not valid." );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    valid = ValidateAdvLinkParms( indx )

    # The link built above is well formed, so this has to come back true.
    assert valid, "ValidateAdvLinkParms did not report success"

    if  valid :
        print( "Advanced link Parms are valid." )
    else:
        print( "Advanced link Parms are not valid." )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \return bool Flag indicating whether parms are valid
*/

extern bool ValidateAdvLinkParms( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Get the code from an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output: x is 10 minus the Pod's
    // length, so changing the length has to move the Pod.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 4.0, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output: x is 10 minus the Pod's
    # length, so changing the length has to move the Pod.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 4.0 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \param [in] code string Code for advanced link
*/

extern void SetAdvLinkCode( int index, const string & code );

/*!
    \ingroup AdvancedLink
*/
/*!
    Get the code from an advanced link
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    BuildAdvLinkScript( indx );

    string code = GetAdvLinkCode( indx );
    if ( code.length() == 0 )
    {
        Print( "ERROR: GetAdvLinkCode returned nothing" );
        __failure++;
    }

    Print( code );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    BuildAdvLinkScript( indx )

    code = GetAdvLinkCode( indx )
    assert len( code ) > 0, "GetAdvLinkCode returned nothing"

    print( code )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \return string String containing advanced link code
*/

extern std::string GetAdvLinkCode( int index );

/*!
    \ingroup AdvancedLink
*/
/*!
    Search and replace strings in the advanced link code
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );
    SearchReplaceAdvLinkCode( indx, "10.0", "12.3" );

    string code = GetAdvLinkCode( indx );

    Print( code );

    if ( code.find( "12.3" ) < 0 || code.find( "10.0" ) >= 0 )
    {
        Print( "ERROR: SearchReplaceAdvLinkCode did not replace" );
        __failure++;
    }

    BuildAdvLinkScript( indx );

    // The link was named, so it has to be findable by that name and by that
    // index, and it has to actually drive its output.  The replacement rewrote
    // the code, so x is now 12.3 minus the Pod's length.
    if ( indx < 0 || GetAdvLinkNames().size() != 1 || GetAdvLinkNames()[0] != "ExampleLink" )
    {
        Print( "ERROR: the advanced link was not registered under its name" );
        __failure++;
    }

    SetParmValUpdate( length, 6.0 );

    Update();

    if ( !closeTo( GetParmVal( x_pos ), 6.3, 1e-6 ) )
    {
        Print( "ERROR: the advanced link did not drive its output" );
        __failure++;
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )
    SearchReplaceAdvLinkCode( indx, "10.0", "12.3" )

    code = GetAdvLinkCode( indx )

    print( code )

    assert "12.3" in code and "10.0" not in code, "SearchReplaceAdvLinkCode did not replace"

    BuildAdvLinkScript( indx )

    # The link was named, so it has to be findable by that name and by that
    # index, and it has to actually drive its output.  The replacement rewrote
    # the code, so x is now 12.3 minus the Pod's length.
    assert indx >= 0, "the advanced link was not registered under its name"
    assert len( GetAdvLinkNames() ) == 1, "the advanced link was not registered under its name"
    assert GetAdvLinkNames()[0] == "ExampleLink", "the advanced link was not registered under its name"

    SetParmValUpdate( length, 6.0 )

    Update()

    assert abs( GetParmVal( x_pos ) - 6.3 ) < 1e-6, "the advanced link did not drive its output"

    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \param [in] from string Search token
    \param [in] to string Replace token
*/

extern void SearchReplaceAdvLinkCode( int index, const string & from, const string & to );

/*!
    \ingroup AdvancedLink
*/
/*!
    Build (ready for execution and perform syntax check) an advanced link.
    \forcpponly
    \code{.cpp}

    string pod = AddGeom( "POD", "" );
    string length = FindParm( pod, "Length", "Design" );
    string x_pos = GetParm( pod, "X_Rel_Location", "XForm" );

    AddAdvLink( "ExampleLink" );
    int indx = GetLinkIndex( "ExampleLink" );
    AddAdvLinkInput( indx, length, "len" );
    AddAdvLinkOutput( indx, x_pos, "x" );

    SetAdvLinkCode( indx, "x = 10.0 - len;" );

    bool success = BuildAdvLinkScript( indx );

    // The link built above is well formed, so this has to come back true.
    if ( !success )
    {
        Print( "ERROR: BuildAdvLinkScript did not report success" );
        __failure++;
    }

    if ( success )
    {
        Print( "Advanced link build successful." );
    }
    else
    {
        Print( "Advanced link build not successful." );
    }

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    pod = AddGeom( "POD", "" )
    length = FindParm( pod, "Length", "Design" )
    x_pos = GetParm( pod, "X_Rel_Location", "XForm" )

    AddAdvLink( "ExampleLink" )
    indx = GetLinkIndex( "ExampleLink" )
    AddAdvLinkInput( indx, length, "len" )
    AddAdvLinkOutput( indx, x_pos, "x" )

    SetAdvLinkCode( indx, "x = 10.0 - len;" )

    success = BuildAdvLinkScript( indx )

    # The link built above is well formed, so this has to come back true.
    assert success, "BuildAdvLinkScript did not report success"

    if  success :
        print( "Advanced link build successful." )
    else:
        print( "Advanced link build not successful." )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \return bool Flag indicating whether advanced link build was successful
*/

extern bool BuildAdvLinkScript( int index );


}           // End vsp namespace

#endif // !defined(VSPAPI__INCLUDED_)
