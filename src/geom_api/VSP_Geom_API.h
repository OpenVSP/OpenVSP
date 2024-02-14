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
    are defined in *.vsppart files included in the /"Custom Scripts/" directory. Examples of Custom Components
    are available in the directory for reference. OpenVSP looks in 3 locations for the /"Custom Scripts/" folder,
    where Custom Components are loaded: the root directory, the VSP executable directory, and the home directory.
    Note, these functions are specific to defining Custom Components and can't be called from standard API scripts
    (i.e. *.vspscript). However, a Custom Component can be created as a *.vsppart file and then accessed through
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

#ifdef SWIG
%feature("autodoc", 1);
%feature("doxygen:ignore:forcpponly", range="end");
%feature("doxygen:ignore:beginPythonOnly", range="end:endPythonOnly", contents="parse");

#endif



#if !defined(VSPAPI__INCLUDED_)
#define VSPAPI__INCLUDED_

#include "Defines.h"

#include "APIErrorMgr.h"
#include "APIUpdateCountMgr.h"
#include "Vec3d.h"
#include "Matrix4d.h"

#include <string>
#include <stack>
#include <vector>


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

    // Continue to do things...

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    VSPCheckSetup()

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

    if ( FindGeoms().size() != 0 ) { Print( "ERROR: VSPRenew" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    SetParmVal( pod_id, "Y_Rel_Location", "XForm", 2.0 )

    VSPRenew()

    if  len(FindGeoms()) != 0 : print( "ERROR: VSPRenew" )

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

    //==== Set Tan Angles At Nose/Tail
    SetXSecTanAngles( GetXSec( xsec_surf, 0 ), XSEC_BOTH_SIDES, 90 );
    SetXSecTanAngles( GetXSec( xsec_surf, num_xsecs - 1 ), XSEC_BOTH_SIDES, -90 );

    Update();       // Force Surface Update
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    xsec_surf = GetXSecSurf( fid, 0 )           # Get First (and Only) XSec Surf

    num_xsecs = GetNumXSec( xsec_surf )

    #==== Set Tan Angles At Nose/Tail
    SetXSecTanAngles( GetXSec( xsec_surf, 0 ), XSEC_BOTH_SIDES, 90, -1.0e12, -1.0e12, -1.0e12 )
    SetXSecTanAngles( GetXSec( xsec_surf, num_xsecs - 1 ), XSEC_BOTH_SIDES, -90, -1.0e12, -1.0e12, -1.0e12 )

    Update()       # Force Surface Update

    \endcode
    \endPythonOnly
    \param update_managers Flag to indicate if managers should be updated
*/

extern void Update( bool update_managers = true );

/*!
    \ingroup Vehicle
*/
/*!
    Exit the program with a specific error code
    \param [in] error_code Error code
*/

extern void VSPExit( int error_code );

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

    Print( GetVSPVersion() );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    print( GetVSPVersion() )

    \endcode
    \endPythonOnly
    \return OpenVSP version string (i.e. "OpenVSP 3.17.1")
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    major = GetVSPVersionMajor()
    minor = GetVSPVersionMinor()
    change = GetVSPVersionChange()

    print( f"{major}.{minor}.{change}" )

    \endcode
    \endPythonOnly
    \return OpenVSP major version number (i.e. 3 in 3.X.Y)
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    major = GetVSPVersionMajor()
    minor = GetVSPVersionMinor()
    change = GetVSPVersionChange()

    print( f"{major}.{minor}.{change}" )

    \endcode
    \endPythonOnly
    \return OpenVSP minor version number (i.e. X in 3.X.Y)
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current OpenVSP version is: ", False )

    major = GetVSPVersionMajor()
    minor = GetVSPVersionMinor()
    change = GetVSPVersionChange()

    print( f"{major}.{minor}.{change}" )

    \endcode
    \endPythonOnly
    \return OpenVSP change version number (i.e. Y in 3.X.Y)
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

    Print( GetVSPExePath() );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "The current VSP executable path is: ", False )

    print( GetVSPExePath() )

    \endcode
    \endPythonOnly
    \sa SetVSPAEROPath, CheckForVSPAERO, GetVSPAEROPath
    \return Path to the OpenVSP executable
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
    if ( !CheckForVSPAERO( GetVSPExePath() ) )
    {
        string vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5";
        SetVSPAEROPath( vspaero_path );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    if  not CheckForVSPAERO( GetVSPExePath() ) :
        vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5"
        SetVSPAEROPath( vspaero_path )

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, CheckForVSPAERO, GetVSPAEROPath
    \param [in] path Absolute path to directory containing VSPAERO executable
    \return Flag that indicates whether or not the path was set correctly
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    if  not CheckForVSPAERO( GetVSPAEROPath() ) :
        print( "VSPAERO is not where OpenVSP thinks it is. I should move the VSPAERO executable or call SetVSPAEROPath." )

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, CheckForVSPAERO, SetVSPAEROPath
    \return Path OpenVSP will look for VSPAERO
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    vspaero_path = "C:/Users/example_user/Documents/OpenVSP_3.4.5"

    if  CheckForVSPAERO( vspaero_path ) :
        SetVSPAEROPath( vspaero_path )

    \endcode
    \endPythonOnly
    \sa GetVSPExePath, GetVSPAEROPath, SetVSPAEROPath
    \param [in] path Absolute path to check for VSPAERO executables
    \return Flag that indicates if all VSPAERO executables are found or not
*/

extern bool CheckForVSPAERO( const std::string & path );


extern void RegisterCFDMeshAnalyses();


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

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    ReadVSPFile( fname );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )

    ClearVSPModel()

    ReadVSPFile( fname )

    \endcode
    \endPythonOnly
    \param [in] file_name *.vsp3 file name
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

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    ReadVSPFile( fname );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )

    ClearVSPModel()

    ReadVSPFile( fname )

    \endcode
    \endPythonOnly
    \param [in] file_name *.vsp3 file name
    \param [in] set Set index to write (i.e. SET_ALL)
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

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );

    //==== Reset Geometry ====//
    Print( string( "--->Resetting VSP model to blank slate\n" ) );

    ClearVSPModel();

    ReadVSPFile( fname );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )

    ClearVSPModel()

    ReadVSPFile( fname )

    \endcode
    \endPythonOnly
    \param [in] file_name File name
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

    Update();

    //==== Save Vehicle to File ====//
    Print( "\tSaving vehicle file to: ", false );

    Print( fname );

    WriteVSPFile( GetVSPFileName(), SET_ALL );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    fname = "example_fuse.vsp3"

    SetVSP3FileName( fname )

    Update()

    #==== Save Vehicle to File ====//
    print( "\tSaving vehicle file to: ", False )

    print( fname )

    WriteVSPFile( GetVSPFileName(), SET_ALL )

    \endcode
    \endPythonOnly
    \return File name for the current OpenVSP project
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    #==== Reset Geometry ====//
    print( "--->Resetting VSP model to blank slate\n" )
    ClearVSPModel()

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
    \param [in] file_name string *.vsp3 filename
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
    DeleteGeom( mesh_id ); // Delete the mesh generated by the GMSH export
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING" )             # Add Wing

    ExportFile( "Airfoil_Metadata.csv", SET_ALL, EXPORT_SELIG_AIRFOIL )

    mesh_id = ExportFile( "Example_Mesh.msh", SET_ALL, EXPORT_GMSH )
    DeleteGeom( mesh_id ) # Delete the mesh generated by the GMSH export

    \endcode
    \endPythonOnly
    \sa EXPORT_TYPE
    \param [in] file_name Export file name
    \param [in] thick_set Set index to export (i.e. SET_ALL)
    \param [in] file_type File type enum (i.e. EXPORT_IGES)
    \param [in] subsFlag Flag to tag subsurfaces if MeshGeom is created
    \param [in] thin_set Set index to export as degenerate geometry (i.e. SET_NONE)
    \return Mesh Geom ID if the export generates a mesh
*/

extern std::string ExportFile( const std::string & file_name, int thick_set, int file_type, int subsFlag = 1, int thin_set = vsp::SET_NONE );

/*!
    \ingroup FileIO
*/
/*!
    Import a file into OpenVSP. Many formats are available, such as NASCART, V2, and BEM). The imported Geom, mesh, or other object is inserted
    as a child of the specified parent. If no parent or an invalid parent is given, the import will be done at the top level.
    \sa IMPORT_TYPE
    \param [in] file_name Import file name
    \param [in] file_type File type enum (i.e. IMPORT_PTS)
    \param [in] parent Parent Geom ID (ignored with empty string)
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    SetBEMPropID( prop_id )

    ExportFile( "ExampleBEM.bem", SET_ALL, EXPORT_BEM )

    \endcode
    \endPythonOnly
    \sa EXPORT_TYPE, ExportFile
    \param [in] prop_id Propeller Geom ID
*/

extern void SetBEMPropID( const string & prop_id );


//======================== Design Files ================================//

/*!
    \ingroup DesignFile
*/
/*!
    Read in and apply a design file (*.des) to the current OpenVSP project
    \param [in] file_name *.des input file
*/

extern void ReadApplyDESFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Write all design variables to a design file (*.des)
    \param [in] file_name *.des output file
*/

extern void WriteDESFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Read in and apply a Cart3D XDDM file (*.xddm) to the current OpenVSP project
    \param [in] file_name *.xddm input file
*/

extern void ReadApplyXDDMFile( const std::string & file_name );

/*!
    \ingroup DesignFile
*/
/*!
    Write all design variables to a Cart3D XDDM file (*.xddm)
    \param [in] file_name *.xddm output file
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
    \param [in] type XDDM type enum (XDDM_VAR or XDDM_CONST)
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
    \param [in] index Index of design variable
    \return Parm ID
*/

extern std::string GetDesignVar( int index );

/*!
    \ingroup DesignFile
*/
/*!
    Get the XDDM type of the specified design variable
    \sa XDDM_QUANTITY_TYPE
    \param [in] index Index of design variable
    \return XDDM type enum (XDDM_VAR or XDDM_CONST)
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
    //==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" );

    //==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" )

    #==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE )

    \endcode
    \endPythonOnly
    \sa COMPUTATION_FILE_TYPE, SetFeaMeshFileName
    \param [in] file_type File type enum (i.e. CFD_TRI_TYPE, COMP_GEOM_TXT_TYPE)
    \param [in] file_name File name
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

    if ( double_arr.size() != 1 )                                    { Print( "---> Error: API ComputeMassProps" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Test Mass Props ====//
    pid = AddGeom( "POD", "" )

    mesh_id = ComputeMassProps( SET_ALL, 20, X_DIR )

    mass_res_id = FindLatestResultsID( "Mass_Properties" )

    double_arr = GetDoubleResults( mass_res_id, "Total_Mass" )

    if  len(double_arr) != 1 : print( "---> Error: API ComputeMassProps" )

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] num_slices Number of slices
    \param [in] idir Direction of slicing for integration
    \return MeshGeom ID
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

    comp_res_id = FindLatestResultsID( "Comp_Geom" )                    # Find Results ID

    double_arr = GetDoubleResults( comp_res_id, "Wet_Area" )    # Extract Results

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis, COMPUTATION_FILE_TYPE
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] half_mesh Flag to ignore surfaces on the negative side of the XZ plane (e.g. symmetry)
    \param [in] file_export_types CompGeom file type to export (supports XOR i.e. COMP_GEOM_CSV_TYPE & COMP_GEOM_TXT_TYPE )
    \return MeshGeom ID
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

    if ( double_arr.size() != 6 )                                    { Print( "---> Error: API ComputePlaneSlice" ); }
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

    if  len(double_arr) != 6 : print( "---> Error: API ComputePlaneSlice" )

    \endcode
    \endPythonOnly
    \sa SetAnalysisInputDefaults, PrintAnalysisInputs, ExecAnalysis
    \param [in] set Set index (i.e. SET_ALL)
    \param [in] num_slices Number of slices
    \param [in] norm Normal axis for all slices
    \param [in] auto_bnd Flag to automatically set the start and end bound locations
    \param [in] start_bnd Location of the first slice along the normal axis (default: 0.0)
    \param [in] end_bnd Location of the last slice along the normal axis (default: 0.0)
    \param [in] measureduct Flag to measure negative area inside positive area (default: false)
    \return MeshGeom ID
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
    //==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" );

    //==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Set File Name ====//
    SetComputationFileName( DEGEN_GEOM_CSV_TYPE, "TestDegenScript.csv" )

    #==== Run Degen Geom ====//
    ComputeDegenGeom( SET_ALL, DEGEN_GEOM_CSV_TYPE )

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
    //==== CFDMesh Method Facet Export =====//
    SetComputationFileName( CFD_FACET_TYPE, "TestCFDMeshFacet_API.facet" );

   Print( "\tComputing CFDMesh..." );

    ComputeCFDMesh( SET_ALL, SET_NONE, CFD_FACET_TYPE );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== CFDMesh Method Facet Export =====//
    SetComputationFileName( CFD_FACET_TYPE, "TestCFDMeshFacet_API.facet" )

   print( "\tComputing CFDMesh..." )

    ComputeCFDMesh( SET_ALL, SET_NONE, CFD_FACET_TYPE )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetCFDMeshVal( CFD_MIN_EDGE_LEN, 1.0 )

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
    # This is equivalent to SetParmValUpdate( wid, "Wake", "Shape", 1.0 )
    # To change the scale: SetParmValUpdate( wid, "WakeScale", "WakeSettings", 10.0 )
    # To change the angle: SetParmValUpdate( wid, "WakeAngle", "WakeSettings", -5.0 )

    \endcode
    \endPythonOnly
    \sa SetParmVal, SetParmValUpdate
    \param [in] geom_id string Geom ID
    \param [in] flag True to activate, false to deactivate
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geom ====//
    pid = AddGeom( "POD", "" )

    AddCFDSource( POINT_SOURCE, pid, 0, 0.25, 2.0, 0.5, 0.5 )      # Add A Point Source

    \endcode
    \endPythonOnly
    \sa CFD_MESH_SOURCE_TYPE
    \param [in] type CFD Mesh source type( i.e.BOX_SOURCE )
    \param [in] geom_id string Geom ID
    \param [in] surf_index Main surface index
    \param [in] l1 Source first edge length
    \param [in] r1 Source first radius
    \param [in] u1 Source first U location
    \param [in] w1 Source first W location
    \param [in] l2 Source second edge length
    \param [in] r2 Source second radius
    \param [in] u2 Source second U location
    \param [in] w2 Source second W location
*/

extern void AddCFDSource( int type, const std::string & geom_id, int surf_index,

                          double l1, double r1, double u1, double w1,
                          double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );

/*!
    \ingroup VSPAERO
*/
/*!
    Get ID of the current VSPAERO reference Geom
    \return Reference Geom ID
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

    \endcode
    \endPythonOnly
    \param [in] geom_id Reference Geom ID
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    nanalysis = GetNumAnalysis()

    print( f"Number of registered analyses: {nanalysis}" )

    \endcode
    \endPythonOnly
    \return Number of analyses
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
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_array = ListAnalysis()

    print( "List of Available Analyses: " )

    for i in range(int( len(analysis_array) )):

        print( "    " + analysis_array[i] )

    \endcode
    \endPythonOnly
    \return Array of analysis names
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

    print("Analysis Inputs: ")

    for i in range(int( len(in_names) )):

        print( ( "\t" + in_names[i] + "\n" ) )

    \endcode
    \endPythonOnly
    \param [in] analysis Analysis name
    \return Array of input names
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_name = "VSPAEROComputeGeometry"

    doc = GetAnalysisDoc( analysis_name )

    \endcode
    \endPythonOnly
    \param [in] analysis Analysis name
    \return Documentation string
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
    \param [in] analysis Analysis name
    \param [in] name Input name
    \return Documentation string
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis_name = "VSPAEROComputeGeometry"

    res_id = ExecAnalysis( analysis_name )

    \endcode
    \endPythonOnly
    \param [in] analysis Analysis name
    \return Result ID
*/

extern std::string ExecAnalysis( const std::string & analysis );


/*!
    \ingroup Analysis
*/
/*!
    Get the documentation string for the particular analysis and input
    \param [in] analysis Analysis name
    \param [in] name Input name
    \return Documentation string
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

    for ( int j = 0; j < int( inp_array.size() ); j++ )
    {
        int typ = GetAnalysisInputType( analysis, inp_array[j] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    analysis = "VSPAEROComputeGeometry"

    inp_array = GetAnalysisInputNames( analysis )

    for j in range(int( len(inp_array) )):

        typ = GetAnalysisInputType( analysis, inp_array[j] )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE
    \param [in] analysis Analysis name
    \param [in] name Input name
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
    array< int > analysis_method = GetIntAnalysisInput( analysis_name, "AnalysisMethod" );

    analysis_method[0] = ( VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );

    SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set to panel method
    analysis_method = GetIntAnalysisInput( analysis_name, "AnalysisMethod" )

    analysis_method = [VORTEX_LATTICE]

    SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetIntAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector<int> Array of analysis input values
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
    \return vector<double> Array of analysis input values
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

    fileNameInput[0] = "ParasiteDragExample";

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" )

    fileNameInput = ["ParasiteDragExample"]

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetStringAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector<string> Array of analysis input values
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

    norm[0].set_xyz( 0.23, 0.6, 0.15 );

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # PlanarSlice
    norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" )

    norm[0].set_xyz( 0.23, 0.6, 0.15 )

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE, SetVec3dAnalysisInput
    \param [in] analysis string Analysis name
    \param [in] name string Input name
    \param [in] index int Data index
    \return vector<vec3d> Array of analysis input values
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

    // Set defaults
    SetAnalysisInputDefaults( analysis_name );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set defaults
    SetAnalysisInputDefaults( analysis_name )

    \endcode
    \endPythonOnly
    \param [in] analysis Analysis name
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
    array< int > analysis_method = GetIntAnalysisInput( analysis_name, "AnalysisMethod" );

    analysis_method[0] = ( VSPAERO_ANALYSIS_METHOD::VORTEX_LATTICE );

    SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # Set to panel method
    analysis_method = GetIntAnalysisInput( analysis_name, "AnalysisMethod" )

    analysis_method = [VORTEX_LATTICE]

    SetIntAnalysisInput( analysis_name, "AnalysisMethod", analysis_method )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fileNameInput = GetStringAnalysisInput( "ParasiteDrag", "FileName" )

    fileNameInput = ["ParasiteDragExample"]

    SetStringAnalysisInput( "ParasiteDrag", "FileName", fileNameInput )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # PlanarSlice
    norm = GetVec3dAnalysisInput( "PlanarSlice", "Norm" )

    norm[0].set_xyz( 0.23, 0.6, 0.15 )

    SetVec3dAnalysisInput( "PlanarSlice", "Norm", norm )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # list inputs, type, and current values
    PrintAnalysisInputs( analysis_name )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Analysis: VSPAero Compute Geometry ====//
    analysis_name = "VSPAEROComputeGeometry"

    # list inputs, type, and documentation
    PrintAnalysisDocs( analysis_name )

    \endcode
    \endPythonOnly
    \param [in] analysis_name string Name of analysis
*/

extern void PrintAnalysisDocs( const std::string & analysis_name );


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

    for i in range(int( len(results_array) )):

        resid = FindLatestResultsID( results_array[i] )
        PrintResults( resid )

    \endcode
    \endPythonOnly
    \return Array of result names
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

    if ( data_names.size() != 5 )                            { Print( "---> Error: API GetAllDataNames" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    data_names = GetAllDataNames( res_id )

    if  len(data_names) != 5 : print( "---> Error: API GetAllDataNames" )

    \endcode
    \endPythonOnly
    \param [in] results_id Result ID
    \return Array of result names
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

    if ( GetNumResults( "Test_Results" ) != 2 )                { Print( "---> Error: API GetNumResults" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    if ( GetNumResults( "Test_Results" ) != 2 ): print( "---> Error: API GetNumResults" )

    \endcode
    \endPythonOnly
    \param [in] name Input name
    \return Number of results
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

    \endcode
    \endPythonOnly
    \param [in] results_id Result ID
    \return Result name
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

    \endcode
    \endPythonOnly
    \param [in] results_id Result ID
    \return Result documentation string
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

    if ( res_id.size() == 0 )                                { Print( "---> Error: API FindResultsID" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    if  len(res_id) == 0 : print( "---> Error: API FindResultsID" )

    \endcode
    \endPythonOnly
    \param [in] name Result name
    \param [in] index Result index
    \return Result ID
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
        PrintResults( resid )

    \endcode
    \endPythonOnly
    \param [in] name Result name
    \return Result ID
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

    if ( GetNumData( res_id, "Test_Int" ) != 2 )            { Print( "---> Error: API GetNumData " ); }

    array<int> @int_arr = GetIntResults( res_id, "Test_Int", 0 );

    if ( int_arr[0] != 1 )                                    { Print( "---> Error: API GetIntResults" ); }

    int_arr = GetIntResults( res_id, "Test_Int", 1 );

    if ( int_arr[0] != 2 )                                    { Print( "---> Error: API GetIntResults" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    if ( GetNumData( res_id, "Test_Int" ) != 2 ): print( "---> Error: API GetNumData " )

    int_arr = GetIntResults( res_id, "Test_Int", 0 )

    if  int_arr[0] != 1 : print( "---> Error: API GetIntResults" )

    int_arr = GetIntResults( res_id, "Test_Int", 1 )

    if  int_arr[0] != 2 : print( "---> Error: API GetIntResults" )

    \endcode
    \endPythonOnly
    \param [in] results_id Result ID
    \param [in] data_name Data name
    \return Number of data values
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

    for ( int j = 0; j < int( res_array.size() ); j++ )
    {
        int typ = GetResultsType( res_id, res_array[j] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    res_array = GetAllDataNames( res_id )

    for j in range(int( len(res_array) )):

        typ = GetResultsType( res_id, res_array[j] )

    \endcode
    \endPythonOnly
    \sa RES_DATA_TYPE
    \param [in] results_id Result ID
    \param [in] data_name Data name
    \return Data type enum (i.e. DOUBLE_DATA)
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

    if ( GetNumData( res_id, "Test_Int" ) != 2 )            { Print( "---> Error: API GetNumData " ); }

    array<int> @int_arr = GetIntResults( res_id, "Test_Int", 0 );

    if ( int_arr[0] != 1 )                                    { Print( "---> Error: API GetIntResults" ); }

    int_arr = GetIntResults( res_id, "Test_Int", 1 );

    if ( int_arr[0] != 2 )                                    { Print( "---> Error: API GetIntResults" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    if ( GetNumData( res_id, "Test_Int" ) != 2 ): print( "---> Error: API GetNumData " )

    int_arr = GetIntResults( res_id, "Test_Int", 0 )

    if  int_arr[0] != 1 : print( "---> Error: API GetIntResults" )

    int_arr = GetIntResults( res_id, "Test_Int", 1 )

    if  int_arr[0] != 2 : print( "---> Error: API GetIntResults" )

    \endcode
    \endPythonOnly
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
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

    \endcode
    \endPythonOnly
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
*/

extern const std::vector< double > & GetDoubleResults( const std::string & id, const std::string & name, int index = 0 );

/*!
    \ingroup Results
*/
/*!
    Get all matrix (vector<vector<double>>) values for a particular result, name, and index
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return 2D array of data values
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

    if ( str_arr[0] != "This Is A Test" )                    { Print( "---> Error: API GetStringResults" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Write Some Fake Test Results =====//
    WriteTestResults()

    res_id = FindResultsID( "Test_Results" )

    str_arr = GetStringResults( res_id, "Test_String" )

    if ( str_arr[0] != "This Is A Test" ): print( "---> Error: API GetStringResults" )

    \endcode
    \endPythonOnly
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
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

    print( "X: ", False )
    print( vec3d_vec[0].x(), False )

    print( "\tY: ", False )
    print( vec3d_vec[0].y(), False )

    print( "\tZ: ", False )
    print( vec3d_vec[0].z() )

    \endcode
    \endPythonOnly
    \param [in] id Result ID
    \param [in] name Data name
    \param [in] index Data index
    \return Array of data values
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

    if ( int_arr[0] < 4 )                                            { Print( "---> Error: API CreateGeomResults" ); }
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

    if  int_arr[0] < 4 : print( "---> Error: API CreateGeomResults" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name Result name
    \return Result ID
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

    if ( GetNumResults( "Comp_Mesh" ) != 0 )                { Print( "---> Error: API DeleteAllResults" ); }
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

    if ( GetNumResults( "Comp_Mesh" ) != 0 ): print( "---> Error: API DeleteAllResults" )

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

    if ( GetNumResults( "Comp_Mesh" ) != 0 )                { Print( "---> Error: API DeleteResult" ); }
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

    if ( GetNumResults( "Comp_Mesh" ) != 0 ): print( "---> Error: API DeleteResult" )

    \endcode
    \endPythonOnly
    \param [in] id Result ID
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pid = AddGeom( "POD" )

    analysis_name = "VSPAEROComputeGeometry"

    rid = ExecAnalysis( analysis_name )

    WriteResultsCSVFile( rid, "CompGeomRes.csv" )

    \endcode
    \endPythonOnly
    \param [in] id Rsult ID
    \param [in] file_name CSV output file name
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
    PrintResultsDoc( rid );
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

    \endcode
    \endPythonOnly
*/

extern void WriteTestResults();

//======================== GUI Functions ================================//

/*!
    \ingroup Visualization
*/
/*!
    Initialize the GUI so it can be called from the API.  Must be called before other GUI related API calls.
    In a multi-threaded environment, this must be called from the main thread only.
    \forcpponly
    \code{.cpp}

    InitGUI();
    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    StartGUI()

    \endcode
    \endPythonOnly

    \sa StartGUI
*/

extern void InitGUI();

/*!
    \ingroup Visualization
*/
/*!
    Launch the interactive OpenVSP GUI.  In a multi-threaded environment, this must be called from the main thread only.
    This starts the GUI event loop.
    \forcpponly
    \code{.cpp}

    InitGUI();
    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    StartGUI()

    \endcode
    \endPythonOnly

    \sa InitGUI
*/

    extern void StartGUI();

/*!
    \ingroup Visualization
*/
/*!
    Enable Return to API functionality from the OpenVSP GUI.

    Typically used for the blocking-mode OpenVSP GUI from the API.  This should not be enabled for a non-blocking
    use of the OpenVSP GUI.

    This will add a "Return to API" option to the file pulldown menu and will also cause the exit button on the
    window frame to have the same effect.  When selected, these options will stop the OpenVSP GUI event loop,
    returning control to the API program.  OpenVSP will not terminate, the model will remain in memory and will
    be responsive to subsequent API calls.

    \forcpponly
    \code{.cpp}

    InitGUI();
    EnableReturnToAPI();
    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    EnableReturnToAPI()
    StartGUI()

    \endcode
    \endPythonOnly

    \sa InitGUI, DisableReturnToAPI
*/

extern void EnableReturnToAPI();

/*!
    \ingroup Visualization
*/
/*!
    Disable Return to API functionality from the OpenVSP GUI.

    This reverses the operation of EnableReturnToAPI.

    \forcpponly
    \code{.cpp}

    InitGUI();
    EnableReturnToAPI();
    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    EnableReturnToAPI()
    DisableReturnToAPI()
    StartGUI()

    \endcode
    \endPythonOnly

    \sa InitGUI, EnableReturnToAPI
*/

extern void DisableReturnToAPI();

/*!
    \ingroup Visualization
*/
/*!
    Stop OpenVSP GUI event loop.  Keep OpenVSP running and in memory.
    \forcpponly
    \code{.cpp}

    InitGUI();
    StartGUI();

    StopGUI();

    StartGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    StartGUI()

    StopGUI()

    StartGUI()

    \endcode
    \endPythonOnly

    \sa InitGUI, StartGUI
*/

extern void StopGUI();

/*!
    \ingroup Visualization
*/
/*!
    Hide OpenVSP screens.  Keep OpenVSP event loop running and in memory.
    \forcpponly
    \code{.cpp}

    InitGUI();
    StartGUI();

    HideScreens();

    ShowScreens();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    StartGUI()

    HideScreens()

    ShowScreens()

    \endcode
    \endPythonOnly

    \sa InitGUI, StartGUI, ShowScreens
*/

extern void HideScreens();

/*!
    \ingroup Visualization
*/
/*!
    Show OpenVSP screens.  Event loop must be running.
    \forcpponly
    \code{.cpp}

    InitGUI();
    StartGUI();

    HideScreens();

    ShowScreens();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    StartGUI()

    HideScreens()

    ShowScreens()

    \endcode
    \endPythonOnly

    \sa InitGUI, StartGUI, HideScreens
*/

extern void ShowScreens();


/*!
    \ingroup Visualization
*/
/*!
    Cause OpenVSP to display a popup message.
    \forcpponly
    \code{.cpp}

    InitGUI();
    StartGUI();

    PopupMsg( "This is a popup message." );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
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

    InitGUI();
    StartGUI();

    string pod_id = AddGeom( "POD" );

    string length = FindParm( pod_id, "Length", "Design" );

    SetParmVal( length, 13.0 );

    UpdateGUI();

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    InitGUI()
    StartGUI()

    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmVal( length, 13.0 )

    UpdateGUI()

    \endcode
    \endPythonOnly

    \sa InitGUI, StartGUI
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

    InitGUI();
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

    InitGUI()
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

    InitGUI();
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

    InitGUI()
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
    \param [in] vaxis True to show the axis, false to hide the axis
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
    \param [in] brdr True to show the border frame, false to hide the border frame
*/

extern void SetShowBorders( bool brdr );

/*!
    \ingroup Visualization
*/
/*!
    Set the draw type of the specified goemetry
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
    \param [in] type Draw type enum (i.e. GEOM_DRAW_SHADE)
*/

extern void SetGeomDrawType(const string &geom_id, int type);

/*!
    \ingroup Visualization
*/
/*!
    Set the display type of the specified goemetry
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
    \param [in] type Display type enum (i.e. DISPLAY_BEZIER)
*/

extern void SetGeomDisplayType(const string &geom_id, int type);

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
    \param [in] r Red 8-bit unsigned integer (range: 0-255)
    \param [in] g Green 8-bit unsigned integer (range: 0-255)
    \param [in] b Blue 8-bit unsigned integer (range: 0-255)
*/

extern void SetBackground( double r, double g, double b );

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
    \param [in] e int GUI_VSP_SCREEN enum for screen
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
    \param [in] e int GUI_GEOM_SCREEN enum for geom screen
    \param [in] state bool True to disable geom screen
*/
extern void SetGeomScreenDisable( int s, bool state );


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

    if ( type_array[0] != "POD" )                { Print( "---> Error: API GetGeomTypes  " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pod1 = AddGeom( "POD", "" )
    pod2 = AddGeom( "POD", "" )

    type_array = GetGeomTypes()

    if ( type_array[0] != "POD" ): print( "---> Error: API GetGeomTypes  " )

    \endcode
    \endPythonOnly
    \return Array of Geom type names
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    \endcode
    \endPythonOnly
    \param [in] type Geom type (i.e FUSELAGE, POD, etc.)
    \param [in] parent Parent Geom ID
    \return Geom ID
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

    SetParmVal( pod_id, "X_Rel_Location", "XForm", 5.0 );

    UpdateGeom( pod_id ); // Faster than updating the whole vehicle
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    SetParmVal( pod_id, "X_Rel_Location", "XForm", 5.0 )

    UpdateGeom( pod_id ) # Faster than updating the whole vehicle

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

    DeleteGeom( wing_id );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    DeleteGeom( wing_id )

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

    DeleteGeomVec( mesh_id_vec );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    rid = ExecAnalysis( "CompGeom" )

    mesh_id_vec = GetStringResults( rid, "Mesh_GeomID" )

    DeleteGeomVec( mesh_id_vec )

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

    if ( geom_ids.size() != 2 )                { Print( "---> Error: API Cut/Paste Geom  " ); }
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

    if  len(geom_ids) != 2 : print( "---> Error: API Cut/Paste Geom  " )

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

    if ( geom_ids.size() != 3 )                { Print( "---> Error: API Copy/Paste Geom  " ); }
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

    if  len(geom_ids) != 3 : print( "---> Error: API Copy/Paste Geom  " )

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

    if ( geom_ids.size() != 2 )                { Print( "---> Error: API Cut/Paste Geom  " ); }
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

    if  len(geom_ids) != 2 : print( "---> Error: API Cut/Paste Geom  " )

    \endcode
    \endPythonOnly
    \param [in] parent string Parent Geom ID
    \return vector<string> Vector of pasted Geom IDs
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

    if ( geom_ids.size() != 2 )                        { Print( "---> Error: API FindGeoms " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometries ====//
    pod1 = AddGeom( "POD", "" )
    pod2 = AddGeom( "POD", "" )

    #==== There Should Be Two Geoms =====//
    geom_ids = FindGeoms()

    if  len(geom_ids) != 2 : print( "---> Error: API FindGeoms " )

    \endcode
    \endPythonOnly
    \return Array of all Geom IDs
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

    \endcode
    \endPythonOnly
    \sa FindGeom
    \param [in] name Geom name
    \return Array of Geom IDs
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

    \endcode
    \endPythonOnly
    \sa FindGeomsWithName
    \param [in] name Geom name
    \param [in] index
    \return Geom ID with name at specified index
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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name Geom name
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    SetGeomName( pid, "ExamplePodName" )

    name_str = "Geom Name: " + GetGeomName( pid )

    print( name_str )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return Geom name
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

    if ( parm_array.size() < 1 )            { Print( "---> Error: API GetGeomParmIDs " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD", "" )

    print( "---> Test Get Parm Arrays" )

    parm_array = GetGeomParmIDs( pid )

    if  len(parm_array) < 1 : print( "---> Error: API GetGeomParmIDs " )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return Array of Parm IDs
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    print( "Geom Type Name: ", False )

    print( GetGeomTypeName( wing_id ) )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return Geom type name
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

    if ( !ValidParm( lenid ) )                { Print( "---> Error: API GetParm  " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    lenid = GetParm( pid, "Length", "Design" )

    if  not ValidParm( lenid ) : print( "---> Error: API GetParm  " )

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
    //==== Add Parent and Child Geometry ====//
    string pod1 = AddGeom( "POD" );

    string pod2 = AddGeom( "POD", pod1 );

    Print( "Parent ID of Pod #2: ", false );

    Print( GetGeomParent( pod2 ) );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Parent and Child Geometry ====//
    pod1 = AddGeom( "POD" )

    pod2 = AddGeom( "POD", pod1 )

    print( "Parent ID of Pod #2: ", False )

    print( GetGeomParent( pod2 ) )

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

    for i in range(int( len(children) )):

        print( "\t", False )
        print( children[i] )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return vector<string> Vector of child Geom IDs
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

    if ( num_xsec_surfs != 1 )                { Print( "---> Error: API GetNumXSecSurfs  " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Fuselage Geometry ====//
    fuseid = AddGeom( "FUSELAGE", "" )

    num_xsec_surfs = GetNumXSecSurfs( fuseid )

    if  num_xsec_surfs != 1 : print( "---> Error: API GetNumXSecSurfs  " )

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

    print( "Total Number of Wing Surfaces: ", False )

    print( num_surf )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \return Number of main surfaces
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
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    if  GetGeomVSPSurfType( wing_id ) != WING_SURF :
        print( "---> Error: API GetGeomVSPSurfType " )

    \endcode
    \endPythonOnly
    \sa VSP_SURF_TYPE
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind Main surface index
    \return VSP surface type enum (i.e. DISK_SURF)
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
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    if  GetGeomVSPSurfCfdType( wing_id ) != CFD_NORMAL :
        print( "---> Error: API GetGeomVSPSurfCfdType " )

    \endcode
    \endPythonOnly
    \sa VSP_SURF_CFD_TYPE
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind Main surface index
    \return VSP surface CFD type enum (i.e. CFD_TRANSPARENT)
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

    \endcode
    \endPythonOnly
    \sa GetGeomBBoxMin
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind Main surface index
    \param [in] ref_frame_is_absolute Flag to specify absolute or body reference frame
    \return Maximum coordinate of the bounding box
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

    \endcode
    \endPythonOnly
    \sa GetGeomBBoxMax
    \param [in] geom_id string Geom ID
    \param [in] main_surf_ind Main surface index
    \param [in] ref_frame_is_absolute Flag to specify absolute or body reference frame
    \return Minimum coordinate of the bounding box
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
    \param [in] type Sub-surface type enum (i.e. SS_RECTANGLE)
    \param [in] surfindex Main surface index (default: 0)
    \return Sub-surface ID
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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] index Sub-surface index
    \return Sub-surface ID
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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] name string Sub surface name
    \return vector<string> Vector of sub-surface ID
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

    DeleteSubSurf( wid, ss_line_id );

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

    DeleteSubSurf( wid, ss_line_id )

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

    DeleteSubSurf( ss_line_id );

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

    DeleteSubSurf( ss_line_id )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    new_name = "New_SS_Rec_Name"

    SetSubSurfName( wid, ss_rec_id, new_name )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    new_name = "New_SS_Rec_Name"

    SetSubSurfName( ss_rec_id, new_name )

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

    string name_str = string("Current Name of SS_Rectangle: ") + rec_name + string("\n");

    Print( name_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    rec_name = GetSubSurfName( wid, ss_rec_id )

    name_str = "Current Name of SS_Rectangle: " + rec_name + "\n"

    print( name_str )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] sub_id string Sub-surface ID
    \return Sub-surface name
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

    string name_str = string("Current Name of SS_Rectangle: ") + rec_name + string("\n");

    Print( name_str );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    ss_rec_id = AddSubSurf( wid, SS_RECTANGLE )                        # Add Sub Surface Rectangle

    rec_name = GetSubSurfName( wid, ss_rec_id )

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
    \return vector<int> Array of sub-surface IDs
*/

extern std::vector<std::string> GetSubSurfIDVec( const std::string & geom_id );

/*!
    \ingroup SubSurface
*/
/*!
    Get a vector of all sub-surface IDs for the entire model
    \return Array of sub-surface IDs
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

    for i in range(len(parm_id_vec)):

        id_name_str = "\tName: " + GetParmName(parm_id_vec[i]) + ", Group: " + GetParmDisplayGroupName(parm_id_vec[i]) + ", ID: " + str(parm_id_vec[i]) + "\n"


        print( id_name_str )

    \endcode
    \endPythonOnly
    \param [in] sub_id string Sub-surface ID
    \return vector<string> Vector of Parm IDs
*/

extern std::vector<std::string> GetSubSurfParmIDs( const std::string & sub_id );


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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] init_skin Flag to initialize the FEA Structure by creating an FEA Skin from the parent Geom's OML at surfindex
    \param [in] surfindex Main surface index for the FEA Structure
    \return FEA Structure index
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

    if ( FindGeoms().size() != 0 ) { Print( "ERROR: VSPRenew" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    SetFeaMeshStructIndex( struct_ind )

    if  len(FindGeoms()) != 0 : print( "ERROR: VSPRenew" )

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

    DeleteFeaStruct( pod_id, struct_ind_1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind_1 = AddFeaStruct( pod_id )

    struct_ind_2 = AddFeaStruct( pod_id )

    DeleteFeaStruct( pod_id, struct_ind_1 )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    #==== Add FeaStructure to Pod ====//
    struct_ind = AddFeaStruct( pod_id )

    struct_id = GetFeaStructID( pod_id, struct_ind )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \return FEA Structure ID
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

    \endcode
    \endPythonOnly
    \param [in] struct_id FEA Structure ID
    \return FEA Structure index
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

    \endcode
    \endPythonOnly
    \param [in] struct_id FEA Structure ID
    \return Parent Geom ID
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

    display_name = "Current Structure Parm Container Name: " + parm_container_name + "\n"

    print( display_name )

    \endcode
    \endPythonOnly
    \sa FindContainer, SetFeaStructName
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \return Name for the FEA Structure
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

    parm_container_id = FindContainer( "Example_Struct", struct_ind )

    display_id = "New Structure Parm Container ID: " + parm_container_id + "\n"

    print( display_id )

    \endcode
    \endPythonOnly
    \sa GetFeaStructName
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] name New name for the FEA Structure
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

    \endcode
    \endPythonOnly
    \sa NumFeaStructures
    \return Array of FEA Structure IDs
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

    \endcode
    \endPythonOnly
    \sa GetFeaPartName
    \param [in] part_id FEA Part ID
    \param [in] name New name for the FEA Part
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
    \param [in] fea_struct_ind FEA Structure index
    \param [in] type FEA Part type enum (i.e. FEA_RIB)
    \return FEA Part ID
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

    //==== Delete Bulkead ====//
    DeleteFeaPart( pod_id, struct_ind, bulkhead_id );
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

    #==== Delete Bulkead ====//
    DeleteFeaPart( pod_id, struct_ind, bulkhead_id )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] part_id FEA Part ID
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

    \endcode
    \endPythonOnly
    \param [in] fea_struct_id FEA Structure ID
    \param [in] fea_part_index FEA Part index
    \return FEA Part ID
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

    \endcode
    \endPythonOnly
    \sa SetFeaPartName
    \param [in] part_id FEA Part ID
    \return FEA Part name
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

    \endcode
    \endPythonOnly
    \sa FEA_PART_TYPE
    \param [in] part_id FEA Part ID
    \return FEA Part type enum
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

    \endcode
    \endPythonOnly
    \sa NumFeaParts
    \param [in] fea_struct_id FEA Structure ID
    \return Array of FEA Part IDs
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

    \endcode
    \endPythonOnly
    \sa NumFeaSubSurfs
    \param [in] fea_struct_id FEA Structure ID
    \return Array of FEA Part IDs
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

    \endcode
    \endPythonOnly
    \sa FEA_RIB_NORMAL, GetFeaPartPerpendicularSparID
    \param [in] part_id FEA Part ID (Rib or Rib Array Type)
    \param [in] perpendicular_spar_id FEA Spar ID
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

    \endcode
    \endPythonOnly
    \sa FEA_RIB_NORMAL, SetFeaPartPerpendicularSparID
    \param [in] part_id FEA Part ID (Rib or Rib Array Type)
    \return Perpendicular FEA Spar ID
*/

extern std::string GetFeaPartPerpendicularSparID( const std::string& part_id );

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
    \param [in] fea_struct_ind FEA Structure index
    \param [in] type FEA SubSurface type enum (i.e. SS_ELLIPSE)
    \return FEA SubSurface ID
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

    //==== Delete LineArray ====//
    DeleteFeaSubSurf( pod_id, struct_ind, line_array_id );
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

    #==== Delete LineArray ====//
    DeleteFeaSubSurf( pod_id, struct_ind, line_array_id )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] ss_id FEA SubSurface ID
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

    \endcode
    \endPythonOnly
    \param [in] ss_id FEA SubSurface ID
    \return FEA SubSurface Index
*/

extern int GetFeaSubSurfIndex( const string & ss_id );

/*!
    \ingroup FEAMesh
*/
/*!
    Get the total number of FEA Subsurfaces in the vehicle
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

    \endcode
    \endPythonOnly
    \sa GetFeaStructIDVec
    \return Total Number of FEA Structures
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

    \endcode
    \endPythonOnly
    \sa GetFeaPartIDVec
    \param [in] fea_struct_id FEA Structure ID
    \return Number of FEA Parts
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

    \endcode
    \endPythonOnly
    \sa GetFeaSubSurfIDVec
    \param [in] fea_struct_id FEA Structure ID
    \return Number of FEA SubSurfaces
*/

extern int NumFeaSubSurfs( const std::string & fea_struct_id );

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Create FeaMaterial ====//
    mat_id = AddFeaMaterial()

    SetParmVal( FindParm( mat_id, "MassDensity", "FeaMaterial" ), 0.016 )

    \endcode
    \endPythonOnly
    \return FEA Material ID
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
    \param [in] property_type FEA Property type enum (i.e. FEA_SHELL).
    \return FEA Property ID
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

    \endcode
    \endPythonOnly
    \sa CFD_CONTROL_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] type FEA Mesh option type enum (i.e. CFD_MAX_EDGE_LEN)
    \param [in] val Value the option is set to
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

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, export_name );
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

    SetFeaMeshFileName( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME, export_name )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] fea_struct_ind FEA Structure index
    \param [in] file_type FEA output file type enum (i.e. FEA_EXPORT_TYPE)
    \param [in] file_name Name for the output file
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

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );
    // Could also call ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME );
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

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME )

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

    ComputeFeaMesh( parent_id, struct_ind, FEA_CALCULIX_FILE_NAME );
    // Could also call ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME );
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
    print( string( "--> Generating FeaMesh " ) )

    #==== Get Parent Geom ID and Index ====//
    parent_id = GetFeaStructParentGeomID( struct_id ) # same as pod_id

    Could also call ComputeFeaMesh ( struct_id, FEA_CALCULIX_FILE_NAME )

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
    Cut a cross-section from the specified geometry and maintain it in memory
    \forcpponly
    \code{.cpp}
    string fid = AddGeom( "FUSELAGE", "" );             // Add Fuselage

    //==== Insert, Cut, Paste Example ====//
    InsertXSec( fid, 1, XS_ROUNDED_RECTANGLE );         // Insert A Cross-Section

    CopyXSec( fid, 2 );                                 // Copy Just Created XSec To Clipboard

    PasteXSec( fid, 1 );                                // Paste Clipboard

    CutXSec( fid, 2 );                                  // Cut Created XSec
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    #==== Insert, Cut, Paste Example ====//
    InsertXSec( fid, 1, XS_ROUNDED_RECTANGLE )         # Insert A Cross-Section

    CopyXSec( fid, 2 )                                 # Copy Just Created XSec To Clipboard

    PasteXSec( fid, 1 )                                # Paste Clipboard

    CutXSec( fid, 2 )                                  # Cut Created XSec

    \endcode
    \endPythonOnly
    \sa PasteXSec
    \param [in] geom_id string Geom ID
    \param [in] index XSec index
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

    // Copy XSec To Clipboard
    CopyXSec( sid, 1 );

    // Paste To XSec 3
    PasteXSec( sid, 3 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Copy XSec To Clipboard
    CopyXSec( sid, 1 )

    # Paste To XSec 3
    PasteXSec( sid, 3 )

    \endcode
    \endPythonOnly
    \sa PasteXSec
    \param [in] geom_id string Geom ID
    \param [in] index XSec index
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

    // Copy XSec To Clipboard
    CopyXSec( sid, 1 );

    // Paste To XSec 3
    PasteXSec( sid, 3 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Copy XSec To Clipboard
    CopyXSec( sid, 1 )

    # Paste To XSec 3
    PasteXSec( sid, 3 )

    \endcode
    \endPythonOnly
    \sa CutXSec, CopyXSec
    \param [in] geom_id string Geom ID
    \param [in] index XSec index
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

    //===== Add XSec ====//
    InsertXSec( wing_id, 1, XS_SIX_SERIES );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wing_id = AddGeom( "WING" )

    #===== Add XSec ====//
    InsertXSec( wing_id, 1, XS_SIX_SERIES )

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] geom_id string Geom ID
    \param [in] index XSec index
    \param [in] type XSec type enum (i.e. XS_GENERAL_FUSE)
*/

extern void InsertXSec( const std::string & geom_id, int index, int type );


//======================== Wing Section Functions ===================//
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

    \endcode
    \endPythonOnly
    \sa WING_DRIVERS, XSEC_DRIVERS
    \param [in] geom_id string Geom ID
    \param [in] section_index Wing section index
    \param [in] driver_0 First driver enum (i.e. SPAN_WSECT_DRIVER)
    \param [in] driver_1 Second driver enum (i.e. ROOTC_WSECT_DRIVER)
    \param [in] driver_2 Third driver enum (i.e. TIPC_WSECT_DRIVER)
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Stack
    sid = AddGeom( "STACK", "" )

    # Get First (and Only) XSec Surf
    xsec_surf = GetXSecSurf( sid, 0 )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] index XSecSurf index
    \return XSecSurf ID
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

    for ( int i = 0 ; i < num_xsecs ; i++ )
    {
        string xsec = GetXSec( xsec_surf, i );

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0 );       // Set Tangent Angles At Cross Section

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.0 );  // Set Tangent Strengths At Cross Section
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

        SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0, -1.0e12, -1.0e12, -1.0e12 )       # Set Tangent Angles At Cross Section

        SetXSecTanStrengths( xsec, XSEC_BOTH_SIDES, 0.0, -1.0e12, -1.0e12, -1.0e12 )  # Set Tangent Strengths At Cross Section

    \endcode
    \endPythonOnly
    \param [in] xsec_surf_id XSecSurf ID
    \return Number of XSecs
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

    \endcode
    \endPythonOnly
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] xsec_index Xsec index
    \return Xsec ID
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

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] xsec_index Xsec index
    \param [in] type Xsec type enum (i.e. XS_ELLIPSE)
*/

extern void ChangeXSecShape( const std::string & xsec_surf_id, int xsec_index, int type );

/*!
    \ingroup XSecSurf
*/
/*!
    Set the global surface transform matrix for given XSecSurf
    \param [in] xsec_surf_id XSecSurf ID
    \param [in] mat Transformation matrix
*/

extern void SetXSecSurfGlobalXForm( const std::string & xsec_surf_id, const Matrix4d & mat );

/*!
    \ingroup XSecSurf
*/
/*!
    Get the global surface transform matrix for given XSecSurf
    \param [in] xsec_surf_id XSecSurf ID
    \return Transformation matrix
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

    if ( GetXSecShape( xsec ) != XS_EDIT_CURVE ) { Print( "ERROR: GetXSecShape" ); }
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

    if  GetXSecShape( xsec ) != XS_EDIT_CURVE : print( "ERROR: GetXSecShape" )

    \endcode
    \endPythonOnly
    \sa XSEC_CRV_TYPE
    \param [in] xsec_id XSec ID
    \return XSec type enum (i.e. XS_ELLIPSE)
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

    if ( abs( GetXSecWidth( xsec ) - 3.0 ) > 1e-6 )        { Print( "---> Error: API Get/Set Width " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ) # Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 )

    if  abs( GetXSecWidth( xsec ) - 3.0 ) > 1e-6 : print( "---> Error: API Get/Set Width " )

    \endcode
    \endPythonOnly
    \sa SetXSecWidth
    \param [in] xsec_id XSec ID
    \return Xsec width
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

    if ( abs( GetXSecHeight( xsec ) - 6.0 ) > 1e-6 )        { Print( "---> Error: API Get/Set Width " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 2 ) # Get 2nd to last XSec

    SetXSecWidthHeight( xsec, 3.0, 6.0 )

    if  abs( GetXSecHeight( xsec ) - 6.0 ) > 1e-6 : print( "---> Error: API Get/Set Width " )

    \endcode
    \endPythonOnly
    \sa SetXSecHeight
    \param [in] xsec_id XSec ID
    \return Xsec height
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

    \endcode
    \endPythonOnly
    \sa SetXSecWidth, SetXSecHeight
    \param [in] xsec_id XSec ID
    \param [in] w Xsec width
    \param [in] h Xsec height
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

    \endcode
    \endPythonOnly
    \sa GetXSecWidth
    \param [in] xsec_id XSec ID
    \param [in] w Xsec width
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

    \endcode
    \endPythonOnly
    \sa GetXSecHeight
    \param [in] xsec_id XSec ID
    \param [in] h Xsec height
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

    if ( parm_array.size() < 1 )                        { Print( "---> Error: API GetXSecParmIDs " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    xsec_surf = GetXSecSurf( fuseid, 0 )

    xsec = GetXSec( xsec_surf, GetNumXSec( xsec_surf ) - 1 )

    parm_array = GetXSecParmIDs( xsec )

    if  len(parm_array) < 1 : print( "---> Error: API GetXSecParmIDs " )

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \return Array of Parm IDs
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

    if ( !ValidParm( wid ) )                            { Print( "---> Error: API GetXSecParm " ); }
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

    if  not ValidParm( wid ) : print( "---> Error: API GetXSecParm " )

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] name Parm name
    \return Parm ID
*/

extern std::string GetXSecParm( const std::string& xsec_id, const std::string& name );

/*!
    \ingroup XSec
*/
/*!
    Read in XSec shape from fuselage (*.fsx) file and set to the specified XSec. The XSec must be of type XS_FILE_FUSE.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 2, XS_FILE_FUSE );

    string xsec = GetXSec( xsec_surf, 2 );

    array< vec3d > @vec_array = ReadFileXSec( xsec, "TestXSec.fxs" );
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


    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] file_name Fuselage XSec file name
    \return Array of coordinate points read from the file and set to the XSec
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

    if ( vec_array.size() > 0 )
    {
        vec_array[1] = vec_array[1] * 2.0;
        vec_array[3] = vec_array[3] * 2.0;

        SetXSecPnts( xsec, vec_array );
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


    if  len(vec_array) > 0 :
        vec_array[1] = vec_array[1] * 2.0
        vec_array[3] = vec_array[3] * 2.0

        SetXSecPnts( xsec, vec_array )

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

    SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0.0 );       // Set Tangent Angles At Cross Section
    SetXSecContinuity( xsec, 1 );                       // Set Continuity At Cross Section

    ResetXSecSkinParms( xsec );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    fid = AddGeom( "FUSELAGE", "" )             # Add Fuselage

    xsec_surf = GetXSecSurf( fid, 0 )           # Get First (and Only) XSec Surf

    num_xsecs = GetNumXSec( xsec_surf )

    xsec = GetXSec( xsec_surf, 1 )

    SetXSecTanAngles( xsec, XSEC_BOTH_SIDES, 0.0, -1.0e12, -1.0e12, -1.0e12 )       # Set Tangent Angles At Cross Section
    SetXSecContinuity( xsec, 1 )                       # Set Continuity At Cross Section

    ResetXSecSkinParms( xsec )

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

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top angle (degrees)
    \param [in] right Right angle (degrees)
    \param [in] bottom Bottom angle (degrees)
    \param [in] left Left angle (degrees)
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

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top strength
    \param [in] right Right strength
    \param [in] bottom Bottom strength
    \param [in] left Left strength
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

        SetXSecCurvatures( xsec, XSEC_BOTH_SIDES, 0.2, -1.0e12, -1.0e12, -1.0e12 )  # Set Tangent Strengths At Cross Section

    \endcode
    \endPythonOnly
    \sa XSEC_SIDES_TYPE
    \param [in] xsec_id XSec ID
    \param [in] side Side type enum (i.e. XSEC_BOTH_SIDES)
    \param [in] top Top curvature
    \param [in] right Right curvature
    \param [in] bottom Bottom curvature
    \param [in] left Left curvature
*/

extern void SetXSecCurvatures( const std::string& xsec_id, int side, double top, double right, double bottom, double left );

/*!
    \ingroup XSec
*/
/*!
    Read in XSec shape from airfoil file and set to the specified XSec. The XSec must be of type XS_FILE_AIRFOIL. Airfoil files may be in Lednicer or Selig format with *.af or *.dat extensions.
    \forcpponly
    \code{.cpp}
    // Add Fuselage Geom
    string fuseid = AddGeom( "FUSELAGE", "" );

    string xsec_surf = GetXSecSurf( fuseid, 0 );

    ChangeXSecShape( xsec_surf, 1, XS_FILE_AIRFOIL );

    string xsec = GetXSec( xsec_surf, 1 );

    ReadFileAirfoil( xsec, "airfoil/N0012_VSP.af" );
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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] file_name Airfoil XSec file name
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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] up_pnt_vec Array of points defining the upper surface of the airfoil
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

    SetAirfoilUpperPnts( xsec, low_array );
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

    SetAirfoilUpperPnts( xsec, low_array )

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] low_pnt_vec Array of points defining the lower surface of the airfoil
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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] up_pnt_vec Array of points defining the upper surface of the airfoil
    \param [in] low_pnt_vec Array of points defining the lower surface of the airfoil
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

    cd_dist_theo = GetHersheyBarDragDist( int( n_pts ), alpha_deg*pi/180, Vinf, ( 2 * halfAR ), False )

    \endcode
    \endPythonOnly
    \param [in] npts Number of points along the span to assess
    \param [in] alpha Wing angle of attack (Radians)
    \param [in] Vinf Freestream velocity
    \param [in] span Hershey Bar full-span. Note, only half is used in the calculation
    \param [in] full_span_flag Flag to apply symmetry to results
    \return Theoretical coefficient of lift distribution array (size = 2*npts if full_span_flag = true)
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

    \endcode
    \endPythonOnly
    \param [in] npts Number of points along the span to assess
    \param [in] alpha Wing angle of attack (Radians)
    \param [in] Vinf Freestream velocity
    \param [in] span Hershey Bar full-span. Note, only half is used in the calculation
    \param [in] full_span_flag Flag to apply symmetry to results (default: false)
    \return Theoretical coefficient of drag distribution array (size = 2*npts if full_span_flag = true)
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

    cp_dist = GetVKTAirfoilCpDist( alpha, epsilon, kappa, tau*(pi/180), xyz_airfoil )

    \endcode
    \endPythonOnly
    \param [in] npts Number of points along the airfoil to return
    \param [in] alpha Airfoil angle of attack (Radians)
    \param [in] epsilon Airfoil thickness
    \param [in] kappa Airfoil camber
    \param [in] tau Airfoil trailing edge angle (Radians)
    \return Array of points on the VKT airfoil (size = npts)
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

    \endcode
    \endPythonOnly
    \sa GetVKTAirfoilPnts
    \param [in] alpha double Airfoil angle of attack (Radians)
    \param [in] epsilon double Airfoil thickness
    \param [in] kappa double Airfoil camber
    \param [in] tau double Airfoil trailing edge angle (Radians)
    \param [in] xyz_data vector<vec3d> Vector of points on the airfoil to evaluate
    \return vector<double> Vector of Cp values for each point in xydata
*/

extern std::vector<double> GetVKTAirfoilCpDist( const double &alpha, const double &epsilon, const double &kappa, const double &tau, std::vector<vec3d> xyz_data );

/*!
    \ingroup XSec
*/
/*!
    Generate the surface coordinate points for a ellipsoid at specified center of input radius along each axis.
    Based on the MATLAB function ellipsoid (https://in.mathworks.com/help/matlab/ref/ellipsoid.html).
    \sa GetVKTAirfoilPnts
    \param [in] center 3D location of the ellipsoid center
    \param [in] abc_rad Radius along the A (X), B (Y), and C (Z) axes
    \param [in] u_npts Number of points in the U direction
    \param [in] w_npts Number of points in the W direction
    \return Array of coordinates describing the ellipsoid surface
*/

extern std::vector<vec3d> GetEllipsoidSurfPnts( const vec3d &center, const vec3d &abc_rad, int u_npts = 20, int w_npts = 20 );

/*!
    \ingroup XSec
*/
/*!
    Get the points along the feature lines of a particular Geom
    \param [in] geom_id string Geom ID
    \return Array of points along the Geom's feature lines
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

    \endcode
    \endPythonOnly
    \sa GetEllipsoidSurfPnts
    \param [in] surf_pnt_vec vector<vec3d> Vector of points on the ellipsoid surface to assess
    \param [in] abc_rad vec3d Radius along the A (X), B (Y), and C (Z) axes
    \param [in] V_inf vec3d 3D components of freestream velocity
    \return vector<double> Vector of Cp results corresponding to each point in surf_pnt_arr
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

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] xsec_id string XSec ID
    \return vector<vec3d> VectorArray of coordinate points for the upper airfoil surface
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

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] xsec_id string XSec ID
    \return vector<vec3d> Vector of coordinate points for the lower airfoil surface
*/

extern std::vector<vec3d> GetAirfoilLowerPnts( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the CST coefficients for the upper surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] xsec_id string XSec ID
    \return vector<double> Vector of CST coefficients for the upper airfoil surface
*/

extern std::vector<double> GetUpperCSTCoefs( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Get the CST coefficients for the lower surface of an airfoil. The XSec must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] xsec_id string XSec ID
    \return vector<double> Vector of CST coefficients for the lower airfoil surface
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
    \param [in] xsec_id XSec ID
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
    \param [in] xsec_id XSec ID
*/

extern void PromoteCSTUpper( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Promote the CST for the lower airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] xsec_id XSec ID
*/

extern void PromoteCSTLower( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Demote the CST for the upper airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetUpperCSTDegree
    \param [in] xsec_id XSec ID
*/

extern void DemoteCSTUpper( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Demote the CST for the lower airfoil surface. The XSec must be of type XS_CST_AIRFOIL
    \sa GetLowerCSTDegree
    \param [in] xsec_id XSec ID
*/

extern void DemoteCSTLower( const std::string& xsec_id );

/*!
    \ingroup XSec
*/
/*!
    Fit a CST airfoil for an existing airfoil of type XS_FOUR_SERIES, XS_SIX_SERIES, XS_FOUR_DIGIT_MOD, XS_FIVE_DIGIT, XS_FIVE_DIGIT_MOD, XS_ONE_SIX_SERIES, or XS_FILE_AIRFOIL.
    \param [in] xsec_surf_id XsecSurf ID
    \param [in] xsec_index XSec index
    \param [in] deg CST degree
*/

extern void FitAfCST( const std::string & xsec_surf_id, int xsec_index, int deg );


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

    if ( GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE ) { Print( "ERROR: ChangeBORXSecShape" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE )

    if  GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE : print( "ERROR: ChangeBORXSecShape" )

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

    if ( GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE ) { Print( "ERROR: GetBORXSecShape" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_ROUNDED_RECTANGLE )

    if  GetBORXSecShape( bor_id ) != XS_ROUNDED_RECTANGLE : print( "ERROR: GetBORXSecShape" )

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
    \return vector<vec3d> Array of coordinate points read from the file and set to the XSec
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

    if ( vec_array.size() > 0 )
    {
        vec_array[1] = vec_array[1] * 2.0;
        vec_array[3] = vec_array[3] * 2.0;

        SetBORXSecPnts( bor_id, vec_array );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_FUSE )

    vec_array = ReadBORFileXSec( bor_id, "TestXSec.fxs" )

    if  len(vec_array) > 0 :
        vec_array[1] = vec_array[1] * 2.0
        vec_array[3] = vec_array[3] * 2.0

        SetBORXSecPnts( bor_id, vec_array )

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
    Read in shape from airfoil file and set to the specified BOR XSecCurve. The XSecCurve must be of type XS_FILE_AIRFOIL. Airfoil files may be in Lednicer or Selig format with *.af or *.dat extensions.
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    up_array = GetBORAirfoilUpperPnts( bor_id )

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] bor_id string Body of revolution Geom ID
    \return vector<vec3d> Vector of coordinate points for the upper airfoil surface
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Body of Recolution
    bor_id = AddGeom( "BODYOFREVOLUTION", "" )

    ChangeBORXSecShape( bor_id, XS_FILE_AIRFOIL )

    ReadBORFileAirfoil( bor_id, "airfoil/N0012_VSP.af" )

    low_array = GetBORAirfoilLowerPnts( bor_id )

    \endcode
    \endPythonOnly
    \sa SetAirfoilPnts
    \param [in] bor_id string Body of revolution Geom ID
    \return vector<vec3d> Vector of coordinate points for the lower airfoil surface
*/

extern std::vector<vec3d> GetBORAirfoilLowerPnts( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the CST coefficients for the upper surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetUpperCST
    \param [in] bor_id Body of revolution Geom ID
    \return vector<double> Vector of CST coefficients for the upper airfoil surface
*/

extern std::vector<double> GetBORUpperCSTCoefs( const std::string& bor_id );

/*!
    \ingroup BOR
*/
/*!
    Get the CST coefficients for the lower surface of an airfoil of a BOR. The XSecCurve must be of type XS_CST_AIRFOIL
    \sa SetLowerCST
    \param [in] bor_id string Body of revolution Geom ID
    \return vector<double> Vector of CST coefficients for the lower airfoil surface
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
    \param [in] deg CST degree of upper airfoil surface
    \param [in] coefs Array of CST coefficients for the upper airfoil surface
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
    Write out the untwisted unit-length 2D Bezier curve for the specified airfoil in custom *.bz format. The output will describe the analytical shape of the airfoil. See BezierAirfoilExample.m and BezierCtrlToCoordPnts.m for examples of
    discretizing the Bezier curve and generating a Selig airfoil file.
    \forcpponly
    \code{.cpp}
    //==== Add Wing Geometry and Set Parms ====//
    string wing_id = AddGeom( "WING", "" );

    const double u = 0.5; // export airfoil at mid span location

    //==== Write Bezier Airfoil File ====//
    WriteBezierAirfoil( "Example_Bezier.bz", wing_id, u );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry and Set Parms ====//
    wing_id = AddGeom( "WING", "" )

    u = 0.5 # export airfoil at mid span location

    #==== Write Bezier Airfoil File ====//
    WriteBezierAirfoil( "Example_Bezier.bz", wing_id, u )

    \endcode
    \endPythonOnly
    \param [in] file_name Airfoil (*.bz) output file name
    \param [in] geom_id string Geom ID
    \param [in] foilsurf_u U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry and Set Parms ====//
    wing_id = AddGeom( "WING", "" )

    u = 0.5 # export airfoil at mid span location

    #==== Write Selig Airfoil File ====//
    WriteSeligAirfoil( "Example_Selig.dat", wing_id, u )

    \endcode
    \endPythonOnly
    \sa GetAirfoilCoordinates
    \param [in] file_name Airfoil (*.dat) output file name
    \param [in] geom_id string Geom ID
    \param [in] foilsurf_u U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
*/

extern void WriteSeligAirfoil( const std::string & file_name, const std::string & geom_id, const double &foilsurf_u );

/*!
    \ingroup XSec
*/
/*!
    Get the untwisted unit-length 2D coordinate points for the specified airfoil
    \sa WriteSeligAirfoil
    \param [in] geom_id string Geom ID
    \param [in] foilsurf_u U location (range: 0 - 1) along the surface. The foil surface does not include root and tip caps (i.e. 2 section wing -> XSec0 @ u=0, XSec1 @ u=0.5, XSec2 @ u=1.0)
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
    \param [in] xsec_id XSec ID
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
    \param [in] xsec_id XSec ID
    \param [in] newtype New curve type enum (i.e. CEDIT)
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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \return Array of U parameter values
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

    print( f"Normalized Bottom Point of XSecCurve: {xsec1_pts[3].x()}, {xsec1_pts[3].y()}, {xsec1_pts[3].z()}" )

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] non_dimensional True to get the points non-dimensionalized, False to get them scaled by m_Width and m_Height
    \return Array of control points
*/

extern std::vector < vec3d > GetEditXSecCtrlVec( const std::string & xsec_id, const bool non_dimensional = true );

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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] u_vec Array of U parameter values
    \param [in] r_vec Array of R parameter values
    \param [in] control_pts Nondimensionalized array of control points
*/

extern void SetEditXSecPnts( const std::string & xsec_id, std::vector < double > u_vec, std::vector < vec3d > control_pts, std::vector < double > r_vec );

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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] indx Control point index
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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] u U value to split the curve at (0 - 1)
    \return Index of the point added from the split
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

    \endcode
    \endPythonOnly
    \param [in] xsec_id XSec ID
    \param [in] indx Control point index
    \param [in] new_pnt Coordinate of the new point
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
    \param [in] indx XSec index
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
    \param [in] xsec_id XSec ID
    \return Array of bool values for each control point
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
    \sa GetEditXSecFixedUVec, ReparameterizeEditXSec
    \param [in] xsec_id XSec ID
    \param [in] fixed_u_vec Array of fixed U flags
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
    \param [in] xsec_id XSec ID
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
    if ( GetNumSets() <= 0 )                            { Print( "---> Error: API GetNumSets " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    if  GetNumSets() <= 0 : print( "---> Error: API GetNumSets " )

    \endcode
    \endPythonOnly
    \return Number of sets
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

    if ( GetSetName( 3 ) != "SetFromScript" )            { Print( "---> Error: API Get/Set Set Name " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    if GetSetName(3) != "SetFromScript":
        print("---> Error: API Get/Set Set Name")


    \endcode
    \endPythonOnly
    \sa SET_TYPE
    \param [in] index Set index
    \param [in] name Set name
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

    if GetSetName(3) != "SetFromScript":
        print("---> Error: API Get/Set Set Name")
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    if GetSetName(3) != "SetFromScript":
        print("---> Error: API Get/Set Set Name")

    \endcode
    \endPythonOnly
    \sa SET_TYPE
    \param [in] index Set index
    \return Set name
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

    if ( geom_arr1.size() != geom_arr2.size() )            { Print( "---> Error: API GetGeomSet " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    geom_arr1 = GetGeomSetAtIndex( 3 )

    geom_arr2 = GetGeomSet( "SetFromScript" )

    if  len(geom_arr1) != len(geom_arr2) : print( "---> Error: API GetGeomSet " )

    \endcode
    \endPythonOnly
    \sa SET_TYPE
    \param [in] index Set index
    \return Array of Geom IDs
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

    if ( geom_arr1.size() != geom_arr2.size() )            { Print( "---> Error: API GetGeomSet " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    geom_arr1 = GetGeomSetAtIndex( 3 )

    geom_arr2 = GetGeomSet( "SetFromScript" )

    if  len(geom_arr1) != len(geom_arr2) : print( "---> Error: API GetGeomSet " )

    \endcode
    \endPythonOnly
    \param [in] name const string set name
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

    if ( GetSetIndex( "SetFromScript" ) != 3 ) { Print( "ERROR: GetSetIndex" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    SetSetName( 3, "SetFromScript" )

    if GetSetIndex("SetFromScript") != 3:
        print("ERROR: GetSetIndex")


    \endcode
    \endPythonOnly
    \param [in] name Set name
    \return Set index
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

    if ( !GetSetFlag( fuseid, 3 ) )                        { Print( "---> Error: API Set/Get Set Flag " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    SetSetFlag( fuseid, 3, True )

    if not GetSetFlag(fuseid, 3):
        print("---> Error: API Set/Get Set Flag")


    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] set_index Set index
    \return True if geom is in the set, false otherwise
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

    if ( !GetSetFlag( fuseid, 3 ) )                        { Print( "---> Error: API Set/Get Set Flag " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE", "" )

    SetSetFlag( fuseid, 3, True )

    if not GetSetFlag(fuseid, 3):
        print("---> Error: API Set/Get Set Flag")


    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] set_index Set index
    \param [in] flag Flag that indicates set membership
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

    if ( flag_value != true)                      { Print( "---> Error: API CopyPasteSet " ); }
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

    if  flag_value != True: print( "---> Error: API CopyPasteSet " )

    \endcode
    \endPythonOnly
    \param [in] copyIndex Copy Index
    \param [in] pasteIndex Paste Index
*/

extern void CopyPasteSet( int copyIndex, int pasteIndex );


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
    ScaleSet( 3, 2.0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    # Scale by a factor of 2
    ScaleSet( 3, 2.0 )

    \endcode
    \endPythonOnly
    \param [in] set_index Set index
    \param [in] scale Scale factor
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

    // Rotate 90 degrees about Y
    RotateSet( 3, 0, 90, 0 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    # Rotate 90 degrees about Y
    RotateSet( 3, 0, 90, 0 )

    \endcode
    \endPythonOnly
    \param [in] set_index Set index
    \param [in] x_rot_deg Rotation about the X axis (degrees)
    \param [in] y_rot_deg Rotation about the Y axis (degrees)
    \param [in] z_rot_deg Rotation about the Z axis (degrees)
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
    TranslateSet( 3, vec3d( 2, 3, 0 ) );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    # Translate 2 units in X and 3 units in Y
    TranslateSet( 3, vec3d( 2, 3, 0 ) )

    \endcode
    \endPythonOnly
    \param [in] set_index Set index
    \param [in] translation_vec Translation vector
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

    // Translate 2 units in X and 3 units in Y, rotate 90 degrees about Y, and scale by a factor of 2
    TransformSet( 3, vec3d( 2, 3, 0 ), 0, 90, 0, 2.0, true );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Fuselage Geom
    fuseid = AddGeom( "FUSELAGE" )

    SetSetFlag( fuseid, 3, True )

    # Translate 2 units in X and 3 units in Y, rotate 90 degrees about Y, and scale by a factor of 2
    TransformSet( 3, vec3d( 2, 3, 0 ), 0, 90, 0, 2.0, True )

    \endcode
    \endPythonOnly
    \sa TranslateSet, RotateSet, ScaleSet
    \param [in] set_index Set index
    \param [in] translation_vec Translation vector
    \param [in] x_rot_deg Rotation about the X axis (degrees)
    \param [in] y_rot_deg Rotation about the Y axis (degrees)
    \param [in] z_rot_deg Rotation about the Z axis (degrees)
    \param [in] scale Scale factor
    \param [in] scale_translations_flag Flag to apply the scale factor to translations
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

    if ( !ValidParm( lenid ) )                { Print( "---> Error: API GetParm  " ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pid = AddGeom( "POD" )

    lenid = GetParm( pid, "Length", "Design" )

    if  not ValidParm( lenid ) : print( "---> Error: API GetParm  " )

    \endcode
    \endPythonOnly
    \param [in] id Parm ID
    \return True if Parm ID is valid, false otherwise
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

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); }
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

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 : print( "---> Error: API Parm Val Set/Get " )

    \endcode
    \endPythonOnly
    \sa SetParmValUpdate
    \param [in] parm_id string Parm ID
    \param [in] val Parm value to set
    \return Value that the Parm was set to
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

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); }
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

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 : print( "---> Error: API Parm Val Set/Get " )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    SetParmDescript( length, "Total Length of Geom" )

    \endcode
    \endPythonOnly
    \sa SetParmLowerLimit, SetParmUpperLimit
    \param [in] parm_id string Parm ID
    \param [in] val Parm value to set
    \param [in] lower_limit Parm lower limit
    \param [in] upper_limit Parm upper limit
    \return Value that the Parm was set to
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" )

    SetParmValUpdate( parm_id, 5.0 )

    \endcode
    \endPythonOnly
    \sa SetParmVal
    \param [in] parm_id string Parm ID
    \param [in] val Parm value to set
    \return Value that the Parm was set to
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Pod Geometry ====//
    pod_id = AddGeom( "POD" )

    parm_id = GetParm( pod_id, "X_Rel_Location", "XForm" )

    SetParmValUpdate( parm_id, 5.0 )

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

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); }
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

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 : print( "---> Error: API Parm Val Set/Get " )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return Parm value
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

    if ( abs( GetParmVal( wid ) - 23 ) > 1e-6 )                { Print( "---> Error: API Parm Val Set/Get " ); }
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

    if  abs( GetParmVal( wid ) - 23 ) > 1e-6 : print( "---> Error: API Parm Val Set/Get " )

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    num_blade_id = GetParm( prop_id, "NumBlade", "Design" )

    num_blade = GetIntParmVal( num_blade_id )

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

    bool reverse_flag = GetBoolParmVal( rev_flag_id );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    rev_flag_id = GetParm( prop_id, "ReverseFlag", "Design" )

    reverse_flag = GetBoolParmVal( rev_flag_id )

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

    if ( abs( GetParmVal( wid ) - 13 ) > 1e-6 )                { Print( "---> Error: API SetParmUpperLimit " ); }
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

    if  abs( GetParmVal( wid ) - 13 ) > 1e-6 : print( "---> Error: API SetParmUpperLimit " )

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

    if ( abs( GetParmVal( wid ) - 15 ) > 1e-6 )                { Print( "---> Error: API SetParmLowerLimit " ); }
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

    if  abs( GetParmVal( wid ) - 15 ) > 1e-6 : print( "---> Error: API SetParmLowerLimit " )

    \endcode
    \endPythonOnly
    \sa SetParmValLimits
    \param [in] parm_id string Parm ID
    \param [in] val Parm lower limit
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Prop Geometry ====//
    prop_id = AddGeom( "PROP" )

    num_blade_id = GetParm( prop_id, "NumBlade", "Design" )

    min_blade = GetParmLowerLimit( num_blade_id )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return Parm lower limit
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

    if ( GetParmType( wid ) != PARM_DOUBLE_TYPE )        { Print( "---> Error: API GetParmType " ); }
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

    if  GetParmType( wid ) != PARM_DOUBLE_TYPE : print( "---> Error: API GetParmType " )

    \endcode
    \endPythonOnly
    \sa PARM_TYPE
    \param [in] parm_id string Parm ID
    \return Parm data type enum (i.e. PARM_BOOL_TYPE)
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

        print( name_id )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return Parm name
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

        print( group_str )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return Parm group name
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

        print( group_str )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return Parm display group name
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

    if ( cid.size() == 0 )                                { Print( "---> Error: API GetParmContainer " ); }
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

    if  len(cid) == 0 : print( "---> Error: API GetParmContainer " )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return Parm Container ID
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    SetParmDescript( length, "Total Length of Geom" )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] desc Parm description
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
    Print( desc );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    pod_id = AddGeom( "POD" )

    length = FindParm( pod_id, "Length", "Design" )

    SetParmValLimits( length, 10.0, 0.001, 1.0e12 )

    desc = GetParmDescript( length )
    print( desc )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \return desc Parm description
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

    SetParmVal( sym_id, 0.0 ); // Note: bool input not supported in SetParmVal
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #==== Add Wing Geometry ====//
    wing_id = AddGeom( "WING" )

    #==== Turn Symmetry OFF ====//
    sym_id = FindParm( wing_id, "Sym_Planar_Flag", "Sym")

    SetParmVal( sym_id, 0.0 ) # Note: bool input not supported in SetParmVal

    \endcode
    \endPythonOnly
    \param [in] parm_container_id Parm Container ID
    \param [in] parm_name Parm name
    \param [in] group_name Parm group name
    \return Parm ID
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

    print( "---> API Parm Container IDs: " )

    for i in range(int( len(ctr_arr) )):

        message = "\t" + ctr_arr[i] + "\n"

        print( message )

    \endcode
    \endPythonOnly
    \return Array of Parm Container IDs
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

    if ( ctr_arr.size() > 0 )            { Print( ( "UserParms Parm Container ID: " + ctr_arr[0] ) ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    ctr_arr = FindContainersWithName( "UserParms" )

    if  len(ctr_arr) > 0 : print( ( "UserParms Parm Container ID: " + ctr_arr[0] ) )

    \endcode
    \endPythonOnly
    \param [in] name Parm Container name
    \return Array of Parm Container IDs
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #===== Get Vehicle Parm Container ID ====//
    veh_id = FindContainer( "Vehicle", 0 )

    \endcode
    \endPythonOnly
    \sa FindContainersWithName
    \param [in] name Parm Container name
    \param [in] index Parm Container index
    \return Parm Container ID
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

    if ( GetContainerName( veh_id ) != "Vehicle" )         { Print( "---> Error: API GetContainerName" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    veh_id = FindContainer( "Vehicle", 0 )

    if  GetContainerName( veh_id) != "Vehicle":       print( "---> Error: API GetContainerName" )

    \endcode
    \endPythonOnly
    \param [in] parm_container_id Parm Container ID
    \return Parm Container name
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

    print( "---> UserParms Container Group IDs: " )
    for i in range(int( len(grp_arr) )):

        message = "\t" + grp_arr[i] + "\n"

        print( message )

    \endcode
    \endPythonOnly
    \param [in] parm_container_id Parm Container ID
    \return Array of Parm group names
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

    for i in range(len(parm_ids)):

        name_id = GetParmName( parm_ids[i] ) + ": " + parm_ids[i] + "\n"

        print( name_id )

    \endcode
    \endPythonOnly
    \param [in] parm_container_id Parm Container ID
    \return Array of Parm IDs
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    #===== Get Vehicle Parm Container ID ====//
    veh_id = GetVehicleID()

    \endcode
    \endPythonOnly
    \return Vehicle ID
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

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    n = GetNumUserParms()


    \endcode
    \endPythonOnly
    \return Number of user Parms
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

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    n = GetNumPredefinedUserParms()


    \endcode
    \endPythonOnly
    \return Number of pre-defined user Parms
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

    print( "---> User Parm IDs: " )

    for i in range(int( len(id_arr) )):

        message = "\t" + id_arr[i] + "\n"

        print( message )

    \endcode
    \endPythonOnly
    \return Array of user parameter ids
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    up_id = GetUserParmContainer()

    \endcode
    \endPythonOnly
    \return User parm container ID
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
    \param [in] type Parm type enum (i.e. PARM_DOUBLE_TYPE)
    \param [in] name Parm name
    \param [in] group Parm group
    \return Parm ID
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

    if ( id_arr.size() > n )
    {
        DeleteUserParm( id_arr[n] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}

    n = GetNumPredefinedUserParms()
    id_arr = GetAllUserParms()

    if  len(id_arr) > n :
        DeleteUserParm( id_arr[n] )

    \endcode
    \endPythonOnly
*/

extern void DeleteUserParm( const std::string & id );

/*!
    \ingroup ParmContainer
*/
/*!
    Get the user parm container ID
    \forcpponly
    \code{.cpp}
    DeleteAllUserParm();
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    DeleteAllUserParm()

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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Geom ID
    \param [in] set Collision set enum (i.e. SET_ALL)
    \return Minimum clearance distance
*/

extern double ComputeMinClearanceDistance( const std::string & geom_id, int set  = SET_ALL );

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

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] target_min_dist Target minimum clearance distance
    \param [in] inc_flag Direction indication flag. If true, upper parm limit is used and direction is set to positive
    \param [in] set Collision set enum (i.e. SET_ALL)
    \return Minimum clearance distance
*/ // TODO: Validate inc_flag description

extern double SnapParm( const std::string & parm_id, double target_min_dist, bool inc_flag, int set = SET_ALL );


//======================== Variable Preset Functions ======================//
/*!
    \ingroup VariablePreset
*/
/*!
    Add a Variable Presets group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    if ( GetVarPresetGroupNames().size() != 1 )                    { Print( "---> Error: API AddVarPresetGroup" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    if  len(GetVarPresetGroupNames()) != 1 : print( "---> Error: API AddVarPresetGroup" )

    \endcode
    \endPythonOnly
    \param [in] group_name Variable Presets group name
*/

extern void AddVarPresetGroup( const std::string &group_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Add a setting to the currently active Variable Preset
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    if ( GetVarPresetSettingNamesWName( "Tess" ).size() != 1 )            { Print( "---> Error: API AddVarPresetSetting" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    if len(GetVarPresetSettingNamesWName( "Tess" )) != 1 : print( "---> Error: API AddVarPresetSetting" )

    \endcode
    \endPythonOnly
    \param [in] setting_name Variable Presets setting name
*/

extern void AddVarPresetSetting( const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Add a Parm to the currently active Variable Preset
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
*/

extern void AddVarPresetParm( const std::string &parm_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Add a Parm to the currently active Variable Preset
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] group_name string Variable Presets group name
*/

extern void AddVarPresetParm( const std::string &parm_id, const std::string &group_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Edit the value of a Parm in the currently active Variable Preset
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    EditVarPresetParm( p1, 5 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    EditVarPresetParm( p1, 5 )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] parm_val double Parm value
*/

extern void EditVarPresetParm( const std::string &parm_id, double parm_val );

/*!
    \ingroup VariablePreset
*/
/*!
    Edit the value of a Parm in the currently active Variable Preset
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    EditVarPresetParm( p1, 5 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    EditVarPresetParm( p1, 5 )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] parm_val double Parameter value
    \param [in] group_name string Variable Presets group name
    \param [in] setting_name string Variable Presets setting name
*/

extern void EditVarPresetParm( const std::string &parm_id, double parm_val, const std::string &group_name, const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Remove a Parm from the currently active Variable Preset group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    DeleteVarPresetParm( p1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    DeleteVarPresetParm( p1 )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
*/

extern void DeleteVarPresetParm( const std::string &parm_id );

/*!
    \ingroup VariablePreset
*/
/*!
    Remove a Parm from the currently active Variable Preset group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    DeleteVarPresetParm( p1 );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    DeleteVarPresetParm( p1 )

    \endcode
    \endPythonOnly
    \param [in] parm_id string Parm ID
    \param [in] group_name string Variable Presets group name
*/

extern void DeleteVarPresetParm( const std::string &parm_id, const std::string &group_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Change the currently active Variable Preset
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Config" );

    AddVarPresetSetting( "Default" );

    string p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" );
    string p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" );

    AddVarPresetParm( p1 );
    AddVarPresetParm( p2 );

    SwitchVarPreset( "Config", "Default" );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Config" )

    AddVarPresetSetting( "Default" )

    p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" )
    p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" )

    AddVarPresetParm( p1 )
    AddVarPresetParm( p2 )

    SwitchVarPreset( "Config", "Default" )

    \endcode
    \endPythonOnly
    \param [in] group_name Variable Presets group name
    \param [in] setting_name Variable Presets setting name
*/

extern void SwitchVarPreset( const std::string &group_name, const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Delete a Variable Preset
    \forcpponly
    \code{.cpp}
    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Fine" );

    DeleteVarPresetSet( "Tess", "Fine" );

    if ( GetVarPresetSettingNamesWName( "Tess" ).size() != 0 )    { Print ( "---> Error: DeleteVarPresetSet" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Fine" )

    DeleteVarPresetSet( "Tess", "Fine" )

    if  len(GetVarPresetSettingNamesWName( "Tess" )) != 0 : print( "---> Error: DeleteVarPresetSet" )

    \endcode
    \endPythonOnly
    \param [in] group_name Variable Presets group
    \param [in] setting_name Variable Presets setting name
    \return true is successful, false otherwise
*/

extern bool DeleteVarPresetSet( const std::string &group_name, const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Get the currently active Variable Presets group name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting" );

    Print( "Current Group: " );

    Print( GetCurrentGroupName() );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    AddVarPresetGroup( "New_Group" )

    AddVarPresetSetting( "New_Setting" )

    print( "Current Group: " )

    print( GetCurrentGroupName() )

    \endcode
    \endPythonOnly
    \return Variable Presets group name
*/

extern std::string GetCurrentGroupName();

/*!
    \ingroup VariablePreset
*/
/*!
    Get the currently active Variable Presets setting name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting" );

    Print( "Current Setting: " );

    Print( GetCurrentSettingName() );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    AddVarPresetGroup( "New_Group" )

    AddVarPresetSetting( "New_Setting" )

    print( "Current Setting: " )

    print( GetCurrentSettingName() )

    \endcode
    \endPythonOnly
    \return Variable Presets setting name
*/

extern std::string GetCurrentSettingName();

/*!
    \ingroup VariablePreset
*/
/*!
    Get all Variable Preset group names
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    if ( GetVarPresetGroupNames().size() != 1 )                    { Print( "---> Error: API AddVarPresetGroup" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    if  len(GetVarPresetGroupNames()) != 1 : print( "---> Error: API AddVarPresetGroup" )

    \endcode
    \endPythonOnly
    \return Array of Variable Presets group names
*/

extern std::vector< std::string > GetVarPresetGroupNames();

/*!
    \ingroup VariablePreset
*/
/*!
    Get the name of each settings in the specified Variable Presets group name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    if ( GetVarPresetSettingNamesWName( "Tess" ).size() != 1 )            { Print( "---> Error: API AddVarPresetSetting" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    if len(GetVarPresetSettingNamesWName( "Tess" )) != 1 : print( "---> Error: API AddVarPresetSetting" )

    \endcode
    \endPythonOnly
    \param [in] group_name Variable Presets group name
    \return Array of Variable Presets setting names
*/

extern std::vector< std::string > GetVarPresetSettingNamesWName( const std::string &group_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Get the name of each settings in the specified Variable Presets group index
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting_1" );
    AddVarPresetSetting( "New_Setting_2" );

    array < string > group_1_settings = GetVarPresetSettingNamesWIndex( 1 );

    if ( group_1_settings.size() != 2 )            { Print( "---> Error: API GetVarPresetSettingNamesWIndex" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    AddVarPresetGroup( "New_Group" )

    AddVarPresetSetting( "New_Setting_1" )
    AddVarPresetSetting( "New_Setting_2" )

    group_1_settings = GetVarPresetSettingNamesWIndex( 1 )

    if  len(group_1_settings) != 2 : print( "---> Error: API GetVarPresetSettingNamesWIndex" )

    \endcode
    \endPythonOnly
    \param [in] group_index Variable Presets group index
    \return Array of Variable Presets setting names
*/

extern std::vector< std::string > GetVarPresetSettingNamesWIndex( int group_index );

/*!
    \ingroup VariablePreset
*/
/*!
    Get the value of each Parm in the currently active Variable Preset group and setting
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    EditVarPresetParm( p1, 5 );

    array <double> p_vals = GetVarPresetParmVals();

    if ( p_vals[0] != 5 )                                { Print ( "---> Error: API EditVarPresetParm" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    EditVarPresetParm( p1, 5 )

    p_vals = GetVarPresetParmVals()

    if  p_vals[0] != 5 : print ( "---> Error: API EditVarPresetParm" )

    \endcode
    \endPythonOnly
    \return Array of Variable Presets Parm values
*/

extern std::vector< double > GetVarPresetParmVals();

/*!
    \ingroup VariablePreset
*/
/*!
    Get the value of each Parm in the specified Variable Preset group and setting
    param [in] group_name Variable Presets group name
    param [in] setting_name Variable Presets setting name
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting_1" );
    AddVarPresetSetting( "New_Setting_2" );

    string p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" );
    string p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" );

    AddVarPresetParm( p1 );
    AddVarPresetParm( p2 );

    EditVarPresetParm( p2, 2, "New_Group", "New_Setting_2" );

    array < double > parm_vals = GetVarPresetParmValsWNames( "New_Group", "New_Setting_2" );

    if ( parm_vals.size() != 2 )            { Print( "---> Error: API GetVarPresetParmValsWNames" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "New_Group" )

    AddVarPresetSetting( "New_Setting_1" )
    AddVarPresetSetting( "New_Setting_2" )

    p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" )
    p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" )

    AddVarPresetParm( p1 )
    AddVarPresetParm( p2 )

    EditVarPresetParm( p2, 2, "New_Group", "New_Setting_2" )

    parm_vals = GetVarPresetParmValsWNames( "New_Group", "New_Setting_2" )

    if  len(parm_vals) != 2 : print( "---> Error: API GetVarPresetParmValsWNames" )

    \endcode
    \endPythonOnly
    \return Array of Variable Presets Parm values
*/

extern std::vector< double > GetVarPresetParmValsWNames( const std::string &group_name, const std::string &setting_name );

/*!
    \ingroup VariablePreset
*/
/*!
    Get the Parm IDs contained in the currently active Variable Presets group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "Tess" );

    AddVarPresetSetting( "Coarse" );

    string p1 = FindParm( pod1, "Tess_U", "Shape" );

    AddVarPresetParm( p1 );

    array <string> p_IDs = GetVarPresetParmIDs();

    if ( p_IDs.size() != 1 )                                { Print( "---> Error: API AddVarPresetParm" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "Tess" )

    AddVarPresetSetting( "Coarse" )

    p1 = FindParm( pod1, "Tess_U", "Shape" )

    AddVarPresetParm( p1 )

    p_IDs = GetVarPresetParmIDs()

    if  len(p_IDs) != 1 : print( "---> Error: API AddVarPresetParm" )

    \endcode
    \endPythonOnly
    \return Array of Variable Presets Parm IDs
*/

extern std::vector< std::string > GetVarPresetParmIDs();

/*!
    \ingroup VariablePreset
*/
/*!
    Get the Parm IDs contained in the specitied Variable Presets group
    \forcpponly
    \code{.cpp}
    // Add Pod Geom
    string pod1 = AddGeom( "POD", "" );

    AddVarPresetGroup( "New_Group" );

    AddVarPresetSetting( "New_Setting_1" );
    AddVarPresetSetting( "New_Setting_2" );

    string p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" );
    string p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" );

    AddVarPresetParm( p1 );
    AddVarPresetParm( p2 );

    array < string > parm_ids = GetVarPresetParmIDsWName( "New_Group" );

    if ( parm_ids.size() != 2 )            { Print( "---> Error: API GetVarPresetParmIDsWName" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    pod1 = AddGeom( "POD", "" )

    AddVarPresetGroup( "New_Group" )

    AddVarPresetSetting( "New_Setting_1" )
    AddVarPresetSetting( "New_Setting_2" )

    p1 = FindParm( pod1, "Y_Rel_Rotation", "XForm" )
    p2 = FindParm( pod1, "Z_Rel_Rotation", "XForm" )

    AddVarPresetParm( p1 )
    AddVarPresetParm( p2 )

    parm_ids = GetVarPresetParmIDsWName( "New_Group" )

    if  len(parm_ids) != 2 : print( "---> Error: API GetVarPresetParmIDsWName" )

    \endcode
    \endPythonOnly
    \param [in] group_name Variable Presets group name
    \return Array of Parm IDs
*/

extern std::vector< std::string > GetVarPresetParmIDsWName( const std::string &group_name );


//======================== Parametric Curve Functions ======================//
/*!
    \ingroup PCurve
*/
/*!
    Set the parameters, values, and curve type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] tvec Array of parameter values
    \param [in] valvec Array of values
    \param [in] newtype Curve type enum (i.e. CEDIT)
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
    \param [in] pcurveid P Curve index
    \param [in] newtype Curve type enum (i.e. CEDIT)
*/

extern void PCurveConvertTo( const std::string & geom_id, const int & pcurveid, const int & newtype );

/*!
    \ingroup PCurve
*/
/*!
    Get the type of a propeller blade curve (P Curve)
    \sa PCURV_TYPE
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid P Curve index
    \return Curve type enum (i.e. CEDIT)
*/

extern int PCurveGetType( const std::string & geom_id, const int & pcurveid );

/*!
    \ingroup PCurve
*/
/*!
    Get the parameters of a propeller blade curve (P Curve). Each parameter is a fraction of propeller radius.
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid P Curve index
    \return Array of parameters
*/

extern std::vector < double > PCurveGetTVec( const std::string & geom_id, const int & pcurveid );

/*!
    \ingroup PCurve
*/
/*!
    Get the values of a propeller blade curve (P Curve). What the values represent id dependent on the curve type (i.e. twist, chord, etc.).
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid P Curve index
    \return Array of values
*/

extern std::vector < double > PCurveGetValVec( const std::string & geom_id, const int & pcurveid );

/*!
    \ingroup PCurve
*/
/*!
    Delete a propeller blade curve (P Curve) point
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] indx Point index
*/

extern void PCurveDeletePt( const std::string & geom_id, const int & pcurveid, const int & indx );

/*!
    \ingroup PCurve
*/
/*!
    Split a propeller blade curve (P Curve) at the specified 1D parameter
    \param [in] geom_id string Parent Geom ID
    \param [in] pcurveid P Curve index
    \param [in] tsplit 1D parameter split location
    \return Index of new control point
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

    ApproximateAllPropellerPCurves( prop );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Propeller
    prop = AddGeom( "PROP", "" )

    ApproximateAllPropellerPCurves( prop )


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

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Propeller
    prop = AddGeom( "PROP", "" )

    ResetPropellerThicknessCurve( prop )


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

    if ( num_group != 1 ) { Print( "Error: CreateVSPAEROControlSurfaceGroup" ); }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    num_group = GetNumControlSurfaceGroups()

    if  num_group != 1 : print( "Error: CreateVSPAEROControlSurfaceGroup" )

    \endcode
    \endPythonOnly
    \sa AddSelectedToCSGroup
    \return Index of the new VSPAERO control surface group
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

    AddAllToVSPAEROControlSurfaceGroup( group_index );
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index )

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex Index of the control surface group
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

    RemoveAllFromVSPAEROControlSurfaceGroup( group_index ); // Empty control surface group
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" )                             # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL )                      # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    AddAllToVSPAEROControlSurfaceGroup( group_index )

    RemoveAllFromVSPAEROControlSurfaceGroup( group_index ) # Empty control surface group

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex Index of the control surface group
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

    print( "Active CS in Group Index #", False )
    print( group_index )

    for i in range(int( len(cs_name_vec) )):

        print( cs_name_vec[i] )

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex Index of the control surface group
    \return Array of active control surface names
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

    print( "All Control Surfaces: ", False )

    for i in range(int( len(cs_name_vec) )):

        print( cs_name_vec[i] )

    \endcode
    \endPythonOnly
    \return Array of all control surface names
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

    array < int > cs_ind_vec(1);
    cs_ind_vec[0] = 1;

    AddSelectedToCSGroup( cs_ind_vec, group_index ); // Add the first available control surface to the group
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    wid = AddGeom( "WING", "" ) # Add Wing

    aileron_id = AddSubSurf( wid, SS_CONTROL ) # Add Control Surface Sub-Surface

    group_index = CreateVSPAEROControlSurfaceGroup() # Empty control surface group

    cs_name_vec = GetAvailableCSNameVec( group_index )

    cs_ind_vec = [1]

    AddSelectedToCSGroup( cs_ind_vec, group_index ) # Add the first available control surface to the group

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex Index of the control surface group
    \return Array of active control surface names
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

    print( "CS Group name: ", False )

    print( GetVSPAEROControlGroupName( group_index ) )

    \endcode
    \endPythonOnly
    \param [in] name Name to set for the control surface group
    \param [in] CSGroupIndex Index of the control surface group
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

    print( "CS Group name: ", False )

    print( GetVSPAEROControlGroupName( group_index ) )

    \endcode
    \endPythonOnly
    \param [in] CSGroupIndex Index of the control surface group
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
    \param [in] selected Array of control surface indexes to add to the group. Note, the integer values are one based.
    \param [in] CSGroupIndex Index of the control surface group
*/

extern void AddSelectedToCSGroup(vector <int> selected, int CSGroupIndex);

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

    RemoveSelectedFromCSGroup( remove_cs_ind_vec, group_index ); // Remove the first control surface
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

    RemoveSelectedFromCSGroup( remove_cs_ind_vec, group_index ) # Remove the first control surface

    \endcode
    \endPythonOnly
    \sa GetActiveCSNameVec
    \param [in] selected Array of control surface indexes to remove from the group. Note, the integer values are one based.
    \param [in] CSGroupIndex Index of the control surface group
*/

extern void RemoveSelectedFromCSGroup(vector <int> selected, int CSGroupIndex);

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

    if ( num_group != 2 ) { Print( "Error: GetNumControlSurfaceGroups" ); }
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

    if  num_group != 2 : print( "Error: GetNumControlSurfaceGroups" )

    \endcode
    \endPythonOnly
    \return Number of control surface groups
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

    SetParmVal( FindParm( disk_id, "RotorRPM", "Rotor" ), 1234.0 )
    SetParmVal( FindParm( disk_id, "RotorCT", "Rotor" ), 0.35 )
    SetParmVal( FindParm( disk_id, "RotorCP", "Rotor" ), 0.55 )
    SetParmVal( FindParm( disk_id, "RotorHubDiameter", "Rotor" ), 1.0 )

    \endcode
    \endPythonOnly
    \sa PROP_MODE
    \param [in] disk_index Actuator disk index for the current VSPAERO set
    \return Actuator disk ID
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

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK );

    num_disk = GetNumActuatorDisks(); // Should be 1
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

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_DISK )

    num_disk = GetNumActuatorDisks() # Should be 1

    \endcode
    \endPythonOnly
    \sa PROP_MODE
    \return Number of actuator disks in the current VSPAERO set
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

    SetParmVal( FindParm( disk_id, "RPM", "UnsteadyGroup" ), 1234.0 )

    \endcode
    \endPythonOnly
    \sa PROP_MODE
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Unsteady group ID
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

    \endcode
    \endPythonOnly
    \sa SetUnsteadyGroupName
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Unsteady group name
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

    \endcode
    \endPythonOnly
    \sa GetUnsteadyGroupSurfIndexes
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Array of component IDs
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

    \endcode
    \endPythonOnly
    \sa GetUnsteadyGroupCompIDs
    \param [in] group_index Unsteady group index for the current VSPAERO set
    \return Array of surface indexes
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

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES );

    num_group = GetNumUnsteadyGroups(); // Should be 1

    string wing_id = AddGeom( "WING" );

    num_group = GetNumUnsteadyGroups(); // Should be 2 (includes fixed component group)
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

    SetParmValUpdate( prop_id, "PropMode", "Design", PROP_BLADES )

    num_group = GetNumUnsteadyGroups() # Should be 1

    wing_id = AddGeom( "WING" )

    num_group = GetNumUnsteadyGroups() # Should be 2 (includes fixed component group)

    \endcode
    \endPythonOnly
    \sa PROP_MODE, GetNumUnsteadyRotorGroups
    \return Number of unsteady groups in the current VSPAERO set
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
    \return Number of unsteady rotor groups in the current VSPAERO set
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
    \param [in] excresName Name of the Excressence
    \param [in] excresType Excressence type enum (i.e. EXCRESCENCE_PERCENT_GEOM)
    \param [in] excresVal Excressence value
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    AddExcrescence( "Miscellaneous", EXCRESCENCE_COUNT, 8.5 )

    AddExcrescence( "Cowl Boattail", EXCRESCENCE_CD, 0.0003 )

    AddExcrescence( "Percentage Example", EXCRESCENCE_PERCENT_GEOM, 5 )

    DeleteExcrescence( 2 ) # Last Index

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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting USAF Atmosphere 1966 Table Creation. \n" )

    WriteAtmosphereCSVFile( "USAFAtmosphere1966Data.csv", ATMOS_TYPE_HERRINGTON_1966 )

    \endcode
    \endPythonOnly
    \sa ATMOS_TYPE
    \param [in] file_name Output CSV file
    \param [in] atmos_type Atmospheric model enum (i.e. ATMOS_TYPE_HERRINGTON_1966)
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
    \param [in] alt Altitude
    \param [in] delta_temp Deviation in temperature from the value specified in the atmospheric model
    \param [in] atmos_type Atmospheric model enum (i.e. ATMOS_TYPE_HERRINGTON_1966)
    \param [out] temp output Temperature
    \param [out] pres output Pressure
    \param [out] pres_ratio Output pressure ratio
    \param [out] rho_ratio Output density ratio
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Body Form Factor Data Creation. \n" )
    WriteBodyFFCSVFile( "BodyFormFactorData.csv" )

    \endcode
    \endPythonOnly
    \param [in] file_name Output CSV file
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Wing Form Factor Data Creation. \n" )
    WriteWingFFCSVFile( "WingFormFactorData.csv" )

    \endcode
    \endPythonOnly
    \param [in] file_name Output CSV file
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Turbulent Friciton Coefficient Data Creation. \n" )
    WriteCfEqnCSVFile( "FrictionCoefficientData.csv" )

    \endcode
    \endPythonOnly
    \param [in] file_name Output CSV file
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    print( "Starting Partial Friction Method Data Creation. \n" )
    WritePartialCfMethodCSVFile( "PartialFrictionMethodData.csv" )

    \endcode
    \endPythonOnly
    \param [in] file_name Output CSV file
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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Normal vector3D coordinate point
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

    print( "Point: ( {norm.x()}, {norm.y()}, {norm.z()} )" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Normal vector
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

    print( f"Point: ( {tanu.x()}, {tanu.y()}, {tanu.z()} )" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Tangent vector in U direction
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

    print( f"Point: ( {tanw.x()}, {tanw.y()}, {tanw.z()} )" )

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u U (0 - 1) surface coordinate
    \param [in] w W (0 - 1) surface coordinate
    \return Tangent vector in W direction
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
    vec3d p_out;

    double idist = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt, u_out, w_out, p_out);

    Print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out );
    Print( "3D Offset ", false);
    Print( surf_pt - p_out );
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
    pt = surf_pt

    pt.offset_y( -5.0 )

    idist, u_out, w_out, p_out = AxisProjPnt01( geom_id, surf_indx, Y_DIR, pt )

    print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out )
    print( "3D Offset ", False)
    print( surf_pt - p_out )

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt Input 3D coordinate point
    \param [out] u_out Output closest U (0 - 1) surface coordinate
    \param [out] w_out Output closest W (0 - 1) surface coordinate
    \param [out] p_out Output 3D coordinate point
    \return Axis aligned distance between the 3D point and the projected point on the surface
*/

extern double AxisProjPnt01(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, double &u_out, double &w_out, vec3d &p_out);

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
    vec3d p_out;
    int surf_indx_out;

    double idist = AxisProjPnt01I( geom_id, Y_DIR, pt, surf_indx_out, u_out, w_out, p_out);

    Print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out + " surf_index " + surf_indx_out );
    Print( "3D Offset ", false);
    Print( surf_pt - p_out );
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
    pt = surf_pt

    pt.offset_y( -5.0 )


    idist, surf_indx_out, u_out, w_out, p_out = AxisProjPnt01I( geom_id, Y_DIR, pt )

    print( "iDist " + idist + " u_out " + u_out + " w_out " + w_out + " surf_index " + surf_indx_out )
    print( "3D Offset ", False)
    print( surf_pt - p_out )

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt Input 3D coordinate point
    \param [out] surf_indx_out Output main surface index from the parent Geom
    \param [out] u_out Output closest U (0 - 1) surface coordinate
    \param [out] w_out Output closest W (0 - 1) surface coordinate
    \param [out] p_out Output 3D coordinate point
    \return Axis aligned distance between the 3D point and the projected point on the surface
*/

extern double AxisProjPnt01I(const std::string &geom_id, const int &iaxis, const vec3d &pt, int &surf_indx_out, double &u_out, double &w_out, vec3d &p_out);

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
    vec3d p_out;

    double d = AxisProjPnt01Guess( geom_id, surf_indx, Y_DIR, pt, u0, w0, uout, wout, p_out);

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



    surf_pt = CompPnt01( geom_id, surf_indx, u, w )
    pt = surf_pt

    pt.offset_y( -5.0 )

    # Construct initial guesses near actual parameters
    u0 = u + 0.01234
    w0 = w - 0.05678

    d, uout, wout, p_out = AxisProjPnt01Guess( geom_id, surf_indx, Y_DIR, pt, u0, w0 )

    print( f"Dist {d} u {uout} w {wout}" )

    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01I, AxisProjVecPnt01, AxisProjVecPnt01Guess
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pt Input 3D coordinate point
    \param [in] u0 Input U (0 - 1) surface coordinate guess
    \param [in] w0 Input W (0 - 1) surface coordinate guess
    \param [out] u_out Output closest U (0 - 1) surface coordinate
    \param [out] w_out Output closest W (0 - 1) surface coordinate
    \param [out] p_out Output 3D coordinate point
    \return Distance between the 3D point and the closest point of the surface
*/

extern double AxisProjPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const vec3d &pt, const double &u0, const double &w0, double &u_out, double &w_out, vec3d &p_out);

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

    bool res = InsideSurf( geom_id, surf_indx, pt );

    if ( res )
    {
        print( "Inside" );
    }
    else
    {
        print( "Outside" );
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

    if  res :
        print( "Inside" )
    else:
        print( "Outside" )


    \endcode
    \endPythonOnly
    \sa VecInsideSurf
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pt Input 3D coordinate point
    \return Boolean true if the point is inside the surface, false otherwise.
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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r R (0 - 1) volume coordinate
    \param [in] s S (0 - 1) volume coordinate
    \param [in] t T (0 - 1) volume coordinate
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

    double d = FindRST( geom_id, surf_indx, pnt );

    Print( "Dist " + d + " r " + rout + " s " + sout + " t " + tout );
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
    double l, m, n;

    ConvertRSTtoLMN( geom_id, surf_indx, r, s, t, l, m, n );

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

    l, m, n = ConvertRSTtoLMN( geom_id, surf_indx, r, s, t )


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r R (0 - 1) volume coordinate
    \param [in] s S (0 - 1) volume coordinate
    \param [in] t T (0 - 1) volume coordinate
    \param [out] l L (0 - 1) linear volume coordinate
    \param [out] m M (0 - 1) linear volume coordinate
    \param [out] n N (0 - 1) linear volume coordinate
*/

extern void ConvertRSTtoLMN( const std::string &geom_id, const int &surf_indx, const double &r, const double &s, const double &t, double &l, double &m, double &n );

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
    double l;

    ConvertRtoL( geom_id, surf_indx, r, l );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    r = 0.12

    l = ConvertRtoL( geom_id, surf_indx, r )


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r R (0 - 1) volume coordinate
    \param [out] l L (0 - 1) linear volume coordinate
*/

extern void ConvertRtoL( const std::string &geom_id, const int &surf_indx, const double &r, double &l );

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
    double r, s, t;

    ConvertLMNtoRST( geom_id, surf_indx, l, m, n, r, s, t );

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

    r, s, t = ConvertLMNtoRST( geom_id, surf_indx, l, m, n )


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] l L (0 - 1) linear volume coordinate
    \param [in] m M (0 - 1) linear volume coordinate
    \param [in] n N (0 - 1) linear volume coordinate
    \param [out] r R (0 - 1) volume coordinate
    \param [out] s S (0 - 1) volume coordinate
    \param [out] t T (0 - 1) volume coordinate
*/

extern void ConvertLMNtoRST( const std::string &geom_id, const int &surf_indx, const double &l, const double &m, const double &n, double &r, double &s, double &t );
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

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    l = 0.12

    r_out = ConvertLtoR( geom_id, surf_indx, r )


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] l L (0 - 1) volume coordinate
    \param [out] r_out R (0 - 1) linear volume coordinate
*/
extern void ConvertLtoR( const std::string &geom_id, const int &surf_indx, const double &l, double &r );

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

    double u = 0.25;
    double eta_out;

    ConvertUtoEta( geom_id, u, eta_out );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing Geom
    geom_id = AddGeom( "WING", "" )

    surf_indx = 0

    u = 0.25

    eta_out = ConvertUtoEta( geom_id, u )


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] u U (0 - 1) surface coordinate
    \param [out] eta_out Eta (0 - 1) wing spanwise coordinate
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
    double u;

    ConvertEtatoU( geom_id, eta, u );

    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Wing Geom
    geom_id = AddGeom( "WING", "" )

    surf_indx = 0

    eta= 0.25

    u = ConvertEtatoU( geom_id, eta )


    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] eta Eta (0 - 1) wing spanwise coordinate
    \param [out] u U (0 - 1) surface coordinate
*/

extern void ConvertEtatoU( const std::string &geom_id, const double &eta, double &u );


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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] u_in_vec vector<double> Input vector of U (0 - 1) surface coordinates
    \param [in] w_in_vec vector<double> Input vector of W (0 - 1) surface coordinates
    \return vector<vec3d> Vector of 3D coordinate points
*/

extern std::vector < vec3d > CompVecPnt01(const std::string &geom_id, const int &surf_indx, const std::vector < double > &u_in_vec, const std::vector < double > &w_in_vec);

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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] us vector<double> Input vector of U (0 - 1) surface coordinates
    \param [in] ws vector<double> Input vector of W (0 - 1) surface coordinates
    \return vector<vec3d> Vector of 3D normal vectors
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
    array< vec3d > poutv;

    AxisProjVecPnt01( geom_id, surf_indx, Y_DIR, ptvec, uoutv, woutv, poutv, doutv );

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

    uoutv, woutv, poutv, doutv = AxisProjVecPnt01( geom_id, surf_indx, Y_DIR, ptvec )

    # Some of these outputs are expected to be non-zero because the projected point is on the opposite side of
    # the pod from the originally computed point.  I.e. there were multiple solutions and the original point
    # is not the closest intersection point.  We could offset those points in the +Y direction instead of -Y.
    for i in range(n):

        print( i, False )
        print( "U delta ", False )
        print( uvec[i] - uoutv[i], False )
        print( "W delta ", False )
        print( wvec[i] - woutv[i] )


    \endcode
    \endPythonOnly
    \sa AxisProjPnt01, AxisProjPnt01Guess, AxisProjPnt01I, AxisProjVecPnt01Guess
    \param [in] geom_id string Geom ID
    \param [in] surf_indx int Main surface index from the Geom
    \param [in] iaxis int Axis direction to project point along (X_DIR, Y_DIR, or Z_DIR)
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \param [out] u_out_vec vector<double> Output vector of the closest U (0 - 1) surface coordinate for each 3D input point
    \param [out] w_out_vec vector<double> Output vector of the closest W (0 - 1) surface coordinate for each 3D input point
    \param [out] pt_out_vec vector<vec3d> Output array of 3D coordinate points
    \param [out] d_out_vec vector<double> Output vector of axis distances for each 3D point and the projected point of the surface
*/

extern void AxisProjVecPnt01(const std::string &geom_id, const int &surf_indx, const int &iaxis, const std::vector < vec3d > &pts, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < vec3d > &pt_out_vec, std::vector < double > &d_out_vec );

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
    array< vec3d > poutv;

    u0v.resize( n );
    w0v.resize( n );

    for( int i = 0 ; i < n ; i++ )
    {
        u0v[i] = uvec[i] + 0.01234;
        w0v[i] = wvec[i] - 0.05678;
    }

    AxisProjVecPnt01Guess( geom_id, surf_indx, Y_DIR, ptvec, u0v,  w0v,  uoutv, woutv, poutv, doutv );

    for( int i = 0 ; i < n ; i++ )
    {
        Print( i, false );
        Print( "U delta ", false );
        Print( uvec[i] - uoutv[i], false );
        Print( "W delta ", false );
        Print( wvec[i] - woutv[i] );
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

    uoutv, woutv, poutv, doutv = AxisProjVecPnt01Guess( geom_id, surf_indx, Y_DIR, ptvec, u0v,  w0v )

    for i in range(n):

        print( i, False )
        print( "U delta ", False )
        print( uvec[i] - uoutv[i], False )
        print( "W delta ", False )
        print( wvec[i] - woutv[i] )


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
    \param [out] pt_out_vec vector<vec3d> Output array of 3D coordinate points
    \param [out] d_out_vec vector<double> Output vector of axis distances for each 3D point and the projected point of the surface
*/

extern void AxisProjVecPnt01Guess(const std::string &geom_id, const int &surf_indx, const int &iaxis, const std::vector < vec3d > &pts, const std::vector < double > &u0s, const std::vector < double > &w0s, std::vector < double > &u_out_vec, std::vector < double > &w_out_vec, std::vector < vec3d > &pt_out_vec, std::vector < double > &d_out_vec );

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


    \endcode
    \endPythonOnly
    \sa VecInsideSurf
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] pts vector<vec3d> Input vector of 3D coordinate points
    \return Boolean vector for each point.  True if it is inside the surface, false otherwise.
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

    \endcode
    \endPythonOnly
    \param [in] geom_id string Parent Geom ID
    \param [in] surf_indx int Main surface index from the parent Geom
    \param [in] r_in_vec vector<double> Input vector of R (0 - 1.0) volume coordinates
    \param [in] s_in_vec vector<double> Input vector of S (0 - 1.0) volume coordinates
    \param [in] t_in_vec vector<double> Input vector of T (0 - 1.0) volume coordinates
    \return vector<vec3d> Vector of 3D coordinate points
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
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    # Add Pod Geom
    geom_id = AddGeom( "POD", "" )

    surf_indx = 0

    utess, wtess = GetUWTess01( geom_id, surf_indx )

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

    print("Two Rulers")

    for n in range(len(ruler_array)):

        print( ruler_array[n] )

    \endcode
    \endPythonOnly
    \return vector<string> Vector of Ruler IDs
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

    DelRuler( ruler_array[0] );
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

    DelRuler( ruler_array[0] )

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

    print( "One Probe: ", False )

    print( probe_array[0] )

    \endcode
    \endPythonOnly
    \return [in] Array of Probe IDs
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

    if ( probe_array.size() != 1 ) { Print( "Error: DelProbe" ); }
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

    if  len(probe_array) != 1 : print( "Error: DelProbe" )

    \endcode
    \endPythonOnly
    \param [in] id Probe ID
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

    if ( probe_array.size() != 0 ) { Print( "Error: DeleteAllProbes" ); }
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

    if  len(probe_array) != 0 : print( "Error: DeleteAllProbes" )

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
    array< string > @link_array = GetAdvLinkNames();

    for( int n = 0 ; n < int( link_array.length() ) ; n++ )
    {
        Print( link_array[n] );
    }
    \endcode
    \endforcpponly
    \beginPythonOnly
    \code{.py}
    link_array = GetAdvLinkNames()

    for n in range(len(link_array) ):

        print( link_array[n] )

    \endcode
    \endPythonOnly
    \return Array of advanced link names
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


    \endcode
    \endPythonOnly
    \param [in] name string Name for advanced link
    \return index for advanced link

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

    DelAdvLink( indx );

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

    DelAdvLink( indx )

    link_array = GetAdvLinkNames()

    # Should print nothing.
    for n in range(len(link_array) ):

        print( link_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index Index for advanced link
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

    DelAdvLinkInput( indx, "y" );

    BuildAdvLinkScript( indx );

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

    DelAdvLinkInput( indx, "y" )

    BuildAdvLinkScript( indx )


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

    DelAdvLinkOutput( indx, "y" );

    BuildAdvLinkScript( indx );

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

    DelAdvLinkOutput( indx, "y" )

    BuildAdvLinkScript( indx )


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

    for n in range(len(name_array) ):

        print( name_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return Array of advanced link input names
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

    for n in range( len(parm_array) ):

        print( parm_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return Array of advanced link input Parm IDs
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

    for n in range( len(name_array) ):

        print( name_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return Array of advanced link output names
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

    for n in range( len(parm_array) ):

        print( parm_array[n] )


    \endcode
    \endPythonOnly
    \param [in] index int Advanced link index
    \return Array of advanced link output Parm IDs
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

    if  valid :
        print( "Advanced link Parms are valid." )
    else:
        print( "Advanced link Parms are not valid." )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \return Flag indicating whether parms are valid
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

    print( code )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \return String containing advanced link code
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

    BuildAdvLinkScript( indx );

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

    BuildAdvLinkScript( indx )


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

    if  success :
        print( "Advanced link build successful." )
    else:
        print( "Advanced link build not successful." )


    \endcode
    \endPythonOnly
    \param [in] index int Index for advanced link
    \return Flag indicating whether advanced link build was successful
*/

extern bool BuildAdvLinkScript( int index );


}           // End vsp namespace


#endif // !defined(VSPAPI__INCLUDED_)
