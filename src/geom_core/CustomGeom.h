//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// CustomGeom.h:
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

/*
Three ways to create custom components:
1. Read from file with script attached
2. Create from GUI with script (preparsed) stored in local dir
3. Create from API with location of script

Script:

Init()
- Name & type & script_name
- Create/init parms
- Describe GUI
- Set num cuv_vec

UpdateSurf()
- load crv_vecs based on parms
- Skin surfs over cuv_vecs

CustomGeomMgr - store parsed scripts

*/



#if !defined(VSPCUSTOMGEOM__INCLUDED_)
#define VSPCUSTOMGEOM__INCLUDED_


#include "Geom.h"
#include "XSec.h"
#include "XSecSurf.h"

#include <unordered_map>
using std::unordered_map;


//====Custom Geom Manager ====//
/*!
    \ingroup CustomGeom
*/
/*!
    CustomGeomMgrSingleton is what a custom component script talks to.  Every function in this group is
    registered with AngelScript as a global bound to this object, so a .vsppart file calls AddParm or
    AddXSecSurf directly without naming the manager.  Called from an ordinary script, outside the
    building of a component, they return an empty result and raise no error.
*/
class CustomGeomMgrSingleton
{
public:
    static CustomGeomMgrSingleton& getInstance()
    {
        static CustomGeomMgrSingleton instance;
        return instance;
    }
    static void Init()                                         {}

    //==== Init Geom ====//
    void InitGeom( const string& id, const string& module_name, const string& display_name );
    void SetCurrCustomGeom( const string& id )            { m_CurrGeom = id; }
/*!
    \ingroup CustomGeom
*/
/*!
    Get the ID of the custom component currently being built.  Empty when no component is being
    built, which is the case in an ordinary script.
    \forcpponly
    \code{.cpp}
    string gid = GetCurrCustomGeom();
    \endcode
    \endforcpponly
    \return string Geom ID of the current custom component
*/

    string GetCurrCustomGeom()                            { return m_CurrGeom; }

    void ReadCustomScripts( Vehicle* veh );
    vector< GeomType > GetCustomTypes()                    { return m_CustomTypeVec; }

    //==== Adds A Parm To The Current Custom Geom - Return Index of The Added Parm ====//
/*!
    \ingroup CustomGeom
*/
/*!
    Add a Parm to the custom component being built.  Call this from the component's Init function;
    the ID that comes back is what SetParmVal and the GUI definitions refer to later.
    \forcpponly
    \code{.cpp}
    string length = AddParm( PARM_DOUBLE_TYPE, "Length", "Design" );

    SetParmVal( length, 10.0 );
    \endcode
    \endforcpponly
    \sa GetCustomParm, AddGui
    \param [in] type int Parm type enum (PARM_DOUBLE_TYPE, PARM_INT_TYPE, PARM_BOOL_TYPE or PARM_FRACTION_TYPE)
    \param [in] name string Name of the Parm
    \param [in] group string Name of the group the Parm belongs to
    \return string Parm ID
*/

    string AddParm( int type, const string & name, const string & group );
/*!
    \ingroup CustomGeom
*/
/*!
    Get the ID of a Parm of the custom component by the order it was added in.
    \forcpponly
    \code{.cpp}
    string pid = GetCustomParm( 0 );
    \endcode
    \endforcpponly
    \sa AddParm
    \param [in] index int Index of the Parm, in the order the Parms were added
    \return string Parm ID
*/

    string GetCustomParm( int index );

    //==== Add A Gui Device Constructor to Current Custom Geom - Gui Will Be Created By Custom Screen (if connected) ===//
/*!
    \ingroup CustomGeom
*/
/*!
    Add a control to the custom component's GUI.  The value it returns identifies the control, which
    matters for the ones that report back, such as a trigger button.
    \forcpponly
    \code{.cpp}
    AddGui( GDEV_SLIDER_ADJ_RANGE, "Length", "Length", "Design", 10.0 );
    \endcode
    \endforcpponly
    \sa AddUpdateGui, CheckClearTriggerEvent
    \param [in] type int GUI device type enum (GDEV_SLIDER, GDEV_SLIDER_ADJ_RANGE, GDEV_BUTTON and so on)
    \param [in] label string Label shown next to the control
    \param [in] parm_name string Name of the Parm the control drives
    \param [in] group_name string Name of the group the Parm belongs to
    \param [in] range double Range of the slider
    \return int GUI device index
*/

    int AddGui( int type, const string & label = string(), const string & parm_name = string(), const string & group_name = string(), double range = 10 );
    vector< GuiDef > GetGuiDefVec( const string & geom_id );

    //==== Add Gui->Parm Pairing To Update Vec ====//
/*!
    \ingroup CustomGeom
*/
/*!
    Tie a GUI control to a Parm, so the control follows the Parm when it changes.
    \forcpponly
    \code{.cpp}
    string length = AddParm( PARM_DOUBLE_TYPE, "Length", "Design" );

    int gid = AddGui( GDEV_SLIDER_ADJ_RANGE, "Length" );

    AddUpdateGui( gid, length );
    \endcode
    \endforcpponly
    \sa AddGui
    \param [in] gui_id int GUI device index, as returned by AddGui
    \param [in] parm_id string Parm ID
*/

    void AddUpdateGui( int gui_id, const string & parm_id );
    vector< GuiUpdate > GetGuiUpdateVec();
/*!
    \ingroup CustomGeom
*/
/*!
    Test whether a trigger control has been pressed since the last check, and clear it.  Returns true
    once per press, so it can drive an action from the component's Update function.
    \forcpponly
    \code{.cpp}
    int tid = AddGui( GDEV_TRIGGER_BUTTON, "Reset" );

    if ( CheckClearTriggerEvent( tid ) )
    {
        Print( "The Reset button was pressed" );
    }
    \endcode
    \endforcpponly
    \sa AddGui
    \param [in] gui_id int GUI device index, as returned by AddGui
    \return bool True if the trigger has fired since the last call
*/

    bool CheckClearTriggerEvent( int gui_id );

    //==== Add XSec Surface To Current Geom - Return ID =====//
/*!
    \ingroup CustomGeom
*/
/*!
    Add a cross section surface to the custom component.  The cross sections appended to it become
    the skinned surface.
    \forcpponly
    \code{.cpp}
    string xsec_surf = AddXSecSurf();

    AppendCustomXSec( xsec_surf, XS_CIRCLE );
    \endcode
    \endforcpponly
    \sa RemoveXSecSurf, ClearXSecSurfs, SkinXSecSurf
    \return string XSecSurf ID
*/

    string AddXSecSurf();
/*!
    \ingroup CustomGeom
*/
/*!
    Remove one cross section surface from the custom component.
    \forcpponly
    \code{.cpp}
    string xsec_surf = AddXSecSurf();

    RemoveXSecSurf( xsec_surf );
    \endcode
    \endforcpponly
    \sa AddXSecSurf, ClearXSecSurfs
    \param [in] id string XSecSurf ID
*/

    void RemoveXSecSurf( const string& id );
/*!
    \ingroup CustomGeom
*/
/*!
    Remove every cross section surface from the custom component.
    \forcpponly
    \code{.cpp}
    ClearXSecSurfs();
    \endcode
    \endforcpponly
    \sa AddXSecSurf, RemoveXSecSurf
*/

    void ClearXSecSurfs();
/*!
    \ingroup CustomGeom
*/
/*!
    Skin a surface through the cross sections that have been appended, which is what turns them into
    geometry.  Call this from the component's Update function.
    \forcpponly
    \code{.cpp}
    SkinXSecSurf( false );
    \endcode
    \endforcpponly
    \sa AddXSecSurf, AppendCustomXSec
    \param [in] closed_flag bool True to close the surface back onto the first cross section
*/

    void SkinXSecSurf( bool closed_flag );
/*!
    \ingroup CustomGeom
*/
/*!
    Apply a transformation to one of the component's surfaces.
    \forcpponly
    \code{.cpp}
    Matrix4d mat;

    mat.loadIdentity();

    mat.translatef( 0.0, 0.0, 5.0 );

    TransformSurf( 0, mat );
    \endcode
    \endforcpponly
    \sa CloneSurf
    \param [in] index int Index of the surface
    \param [in] mat Matrix4d Transformation to apply
*/

    void TransformSurf( int index, Matrix4d & mat );
/*!
    \ingroup CustomGeom
*/
/*!
    Copy one of the component's surfaces and place the copy by a transformation.  Useful for a component
    made of several instances of the same shape.
    \forcpponly
    \code{.cpp}
    Matrix4d mat;

    mat.loadIdentity();

    mat.translatef( 0.0, 5.0, 0.0 );

    CloneSurf( 0, mat );
    \endcode
    \endforcpponly
    \sa TransformSurf
    \param [in] index int Index of the surface to copy
    \param [in] mat Matrix4d Transformation placing the copy
*/

    void CloneSurf( int index, Matrix4d & mat );

    //==== Surface Attribute (Wing, Fuse...) =====//
/*!
    \ingroup CustomGeom
*/
/*!
    Set the surface type of one of the component's surfaces, or of all of them.  The type is what tells
    the rest of OpenVSP whether the surface is a wing, a body, or something else.
    \forcpponly
    \code{.cpp}
    SetVspSurfType( WING_SURF, 0 );
    \endcode
    \endforcpponly
    \sa SetVspSurfCfdType
    \param [in] type int Surface type enum (NORMAL_SURF, WING_SURF, DISK_SURF, PROP_SURF)
    \param [in] surf_id int Index of the surface, or -1 for all of them
*/

    void SetVspSurfType( int type, int surf_id = -1 );

    //==== Surface Cfd type (NORM, NEGATIVE, TRANSPARENT) ====//
/*!
    \ingroup CustomGeom
*/
/*!
    Set how one of the component's surfaces is treated by CFD Mesh -- as a normal surface, as a
    transparent one, or as something to be stamped out of another.
    \forcpponly
    \code{.cpp}
    SetVspSurfCfdType( CFD_TRANSPARENT, 0 );
    \endcode
    \endforcpponly
    \sa SetVspSurfType
    \param [in] type int CFD surface type enum (CFD_NORMAL, CFD_NEGATIVE, CFD_TRANSPARENT and so on)
    \param [in] surf_id int Index of the surface, or -1 for all of them
*/

    void SetVspSurfCfdType ( int type, int surf_id = -1);

    //==== Set Up Default Sources =====//
/*!
    \ingroup CustomGeom
*/
/*!
    Give the custom component a default CFD Mesh source, so a mesh of it is refined sensibly without
    the user having to add sources by hand.
    \forcpponly
    \code{.cpp}
    SetupCustomDefaultSource( POINT_SOURCE, 0, 0.1, 1.0, 0.5, 0.5 );
    \endcode
    \endforcpponly
    \sa ClearAllCustomDefaultSources
    \param [in] type int Source type enum (POINT_SOURCE, LINE_SOURCE, BOX_SOURCE)
    \param [in] surf_index int Index of the surface the source belongs to
    \param [in] l1 double Source length at the first end
    \param [in] r1 double Source radius at the first end
    \param [in] u1 double U location of the first end
    \param [in] w1 double W location of the first end
    \param [in] l2 double Source length at the second end, for a line or box source
    \param [in] r2 double Source radius at the second end, for a line or box source
    \param [in] u2 double U location of the second end, for a line or box source
    \param [in] w2 double W location of the second end, for a line or box source
*/

    void SetupCustomDefaultSource( int type, int surf_index, double l1, double r1, double u1, double w1,
                                   double l2 = 0, double r2 = 0, double u2 = 0, double w2 = 0 );
/*!
    \ingroup CustomGeom
*/
/*!
    Remove every default CFD Mesh source from the custom component.
    \forcpponly
    \code{.cpp}
    ClearAllCustomDefaultSources();
    \endcode
    \endforcpponly
    \sa SetupCustomDefaultSource
*/

    void ClearAllCustomDefaultSources();

/*!
    \ingroup CustomGeom
*/
/*!
    Set the centre of rotation of the custom component.
    \forcpponly
    \code{.cpp}
    SetCustomCenter( 0.0, 0.0, 0.0 );
    \endcode
    \endforcpponly
    \param [in] x double X coordinate of the centre
    \param [in] y double Y coordinate of the centre
    \param [in] z double Z coordinate of the centre
*/

    void SetCustomCenter( double x, double y, double z );

    //==== Custom XSecs Functions ====//
/*!
    \ingroup CustomGeom
*/
/*!
    Move one of the component's cross sections to a location.
    \forcpponly
    \code{.cpp}
    string xsec_surf = AddXSecSurf();

    string xsec = AppendCustomXSec( xsec_surf, XS_CIRCLE );

    SetCustomXSecLoc( xsec, vec3d( 1.0, 0.0, 0.0 ) );
    \endcode
    \endforcpponly
    \sa GetCustomXSecLoc, SetCustomXSecRot
    \param [in] xsec_id string XSec ID
    \param [in] loc vec3d Location to move the cross section to
*/

    void SetCustomXSecLoc( const string & xsec_id, const vec3d & loc );
/*!
    \ingroup CustomGeom
*/
/*!
    Get the location of one of the component's cross sections.
    \forcpponly
    \code{.cpp}
    vec3d loc = GetCustomXSecLoc( xsec_id );
    \endcode
    \endforcpponly
    \sa SetCustomXSecLoc
    \param [in] xsec_id string XSec ID
    \return vec3d Location of the cross section
*/

    vec3d GetCustomXSecLoc( const string & xsec_id );
/*!
    \ingroup CustomGeom
*/
/*!
    Rotate one of the component's cross sections.
    \forcpponly
    \code{.cpp}
    SetCustomXSecRot( xsec_id, vec3d( 0.0, 90.0, 0.0 ) );
    \endcode
    \endforcpponly
    \sa GetCustomXSecRot, SetCustomXSecLoc
    \param [in] xsec_id string XSec ID
    \param [in] rot vec3d Rotation angles about X, Y and Z, in degrees
*/

    void SetCustomXSecRot( const string & xsec_id, const vec3d & rot );
/*!
    \ingroup CustomGeom
*/
/*!
    Get the rotation of one of the component's cross sections.
    \forcpponly
    \code{.cpp}
    vec3d rot = GetCustomXSecRot( xsec_id );
    \endcode
    \endforcpponly
    \sa SetCustomXSecRot
    \param [in] xsec_id string XSec ID
    \return vec3d Rotation angles about X, Y and Z, in degrees
*/

    vec3d GetCustomXSecRot( const string & xsec_id );

    //==== Append Cut Copy Paste Insert ====//
/*!
    \ingroup CustomGeom
*/
/*!
    Add a cross section to the end of one of the component's cross section surfaces.
    \forcpponly
    \code{.cpp}
    string xsec_surf = AddXSecSurf();

    string xsec = AppendCustomXSec( xsec_surf, XS_CIRCLE );
    \endcode
    \endforcpponly
    \sa InsertCustomXSec, CutCustomXSec
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] type int XSec type enum (XS_CIRCLE, XS_ELLIPSE, XS_SUPER_ELLIPSE and so on)
    \return string XSec ID
*/

    string AppendCustomXSec( const string & xsec_surf_id, int type );
/*!
    \ingroup CustomGeom
*/
/*!
    Cut a cross section out of one of the component's cross section surfaces and put it on the
    clipboard, from where PasteCustomXSec can retrieve it.
    \forcpponly
    \code{.cpp}
    CutCustomXSec( xsec_surf_id, 1 );
    \endcode
    \endforcpponly
    \sa CopyCustomXSec, PasteCustomXSec
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] index int Index of the cross section
*/

    void CutCustomXSec( const string & xsec_surf_id, int index );
/*!
    \ingroup CustomGeom
*/
/*!
    Copy one of the component's cross sections to the clipboard, leaving it in place.
    \forcpponly
    \code{.cpp}
    CopyCustomXSec( xsec_surf_id, 1 );
    \endcode
    \endforcpponly
    \sa CutCustomXSec, PasteCustomXSec
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] index int Index of the cross section
*/

    void CopyCustomXSec( const string & xsec_surf_id, int index );
/*!
    \ingroup CustomGeom
*/
/*!
    Paste the cross section on the clipboard over one of the component's cross sections.
    \forcpponly
    \code{.cpp}
    CopyCustomXSec( xsec_surf_id, 1 );

    PasteCustomXSec( xsec_surf_id, 2 );
    \endcode
    \endforcpponly
    \sa CutCustomXSec, CopyCustomXSec
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] index int Index of the cross section to paste over
*/

    void PasteCustomXSec( const string & xsec_surf_id, int index );
/*!
    \ingroup CustomGeom
*/
/*!
    Insert a cross section into one of the component's cross section surfaces, after a given index.
    \forcpponly
    \code{.cpp}
    string xsec = InsertCustomXSec( xsec_surf_id, XS_CIRCLE, 1 );
    \endcode
    \endforcpponly
    \sa AppendCustomXSec
    \param [in] xsec_surf_id string XSecSurf ID
    \param [in] type int XSec type enum (XS_CIRCLE, XS_ELLIPSE, XS_SUPER_ELLIPSE and so on)
    \param [in] index int Index to insert after
    \return string XSec ID
*/

    string InsertCustomXSec( const string & xsec_surf_id, int type, int index );

    //==== Get All Custom Script Module Name ====//
    vector< string > GetCustomScriptModuleNames();

    //==== Save Custom Script Content To File ====//
    static int SaveScriptContentToFile( const string & module_name, const string & file_name );


private:

    CustomGeomMgrSingleton();
    CustomGeomMgrSingleton( CustomGeomMgrSingleton const& copy ) = delete;          // Not Implemented
    CustomGeomMgrSingleton& operator=( CustomGeomMgrSingleton const& copy ) = delete; // Not Implemented

    string m_CurrGeom;
    vector< GeomType > m_CustomTypeVec;
    unordered_map< string, string > m_ModuleGeomIDMap;

};

#define CustomGeomMgr CustomGeomMgrSingleton::getInstance()

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//

//==== Custom Geom XSec =====//
/*!
    \ingroup CustomGeom
*/
/*!
    A cross section belonging to a custom component.  Created through AppendCustomXSec or
    InsertCustomXSec rather than directly, and positioned with SetCustomXSecLoc and
    SetCustomXSecRot.
*/
class CustomXSec : public SkinXSec
{
public:

    CustomXSec( XSecCurve *xsc );

    virtual void Update();
    virtual void CopyBasePos( XSec* xs );

    virtual void SetLoc( const vec3d & loc );
    virtual vec3d GetLoc()                                  { return m_Loc; }
    virtual void SetRot( const vec3d & rot );
    virtual vec3d GetRot()                                  { return m_Rot; }

    virtual double GetScale();

protected:

    vec3d m_Loc;
    vec3d m_Rot;

};


//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//

//==== Custom Geom ====//
/*!
    \ingroup CustomGeom
*/
/*!
    The component a .vsppart file builds.  OpenVSP calls the script's Init function once to declare the
    Parms and the GUI, and its Update function whenever something changes, which is where the cross
    sections are placed and SkinXSecSurf is called.
*/
class CustomGeom : public Geom
{
public:
    CustomGeom( Vehicle* vehicle_ptr );
    virtual ~CustomGeom();

    void Clear();
    void InitGeom( );
    void SetScriptModuleName( const string& name )      { m_ScriptModuleName = name; }
    string GetScriptModuleName()                        { return m_ScriptModuleName; }
    void SetDisplayName( const string& name )      { m_DisplayName = name; }
    string GetDisplayName()                        { return m_DisplayName; }

    //==== Add a Parm Return ID ====//
    string AddParm( int type, const string & name, const string & group );
    string FindParmID( int index );

    //==== Add Gui ====//
    int AddGui( const GuiDef & gd );
    vector< GuiDef > GetGuiDefVec()                    { return m_GuiDefVec; }
    void AddGuiTriggerEvent( int gui_index );
    bool CheckClearTriggerEvent( int gui_index );

    void AddUpdateGui( const GuiUpdate & gu );
    vector< GuiUpdate > GetGuiUpdateVec();

    //==== Add XSec Surface Return ID =====//
    string AddXSecSurf();
    void RemoveXSecSurf( const string& id );
    void ClearXSecSurfs();

    virtual int GetNumXSecSurfs() const                   { return ( int )m_XSecSurfVec.size(); }
    virtual XSecSurf* GetXSecSurf( int index );

    //==== Skin XSecs ====//
    virtual void SkinXSecSurf( bool closed_flag );
    virtual void CloneSurf( int index, Matrix4d & mat );
    virtual void TransformSurf( int index, Matrix4d & mat );

    //==== Encode/Decode XML ====//
    virtual xmlNodePtr EncodeXml( xmlNodePtr & node );
    virtual xmlNodePtr DecodeXml( xmlNodePtr & node );

    //==== Set VSP Surf Type ====//
    virtual void SetVspSurfType( int type, int surf_id = -1 );
    virtual void SetVspSurfCfdType( int type, int surf_id = -1);

    //==== CFD Sources =====//
    virtual void AddDefaultSources( double base_len = 1.0);
    virtual void SetUpDefaultSource( SourceData & sd )      { m_DefaultSourceVec.push_back( sd ); }
    virtual void ClearAllDefaultSources()                   { m_DefaultSourceVec.clear(); }

    virtual void ComputeCenter();
    virtual void ApplyScale( double currentScale );

    //==== Offset XSecs For Conformal =====//
    virtual void OffsetXSecs( double off );

protected:

    bool m_InitGeomFlag;
    string m_ScriptModuleName;
    string m_DisplayName;

    vector< Parm* > m_CustomGeomParmVec;    // Storage For Pointers
    vector< GuiDef > m_GuiDefVec;           // Gui Definition
    vector< GuiUpdate > m_UpdateGuiVec;     // Match Gui with Parms
    vector< XSecSurf* > m_XSecSurfVec;
    vector< int > m_TriggerVec;
    vector< SourceData > m_DefaultSourceVec;

    int m_VspSurfType;
    unordered_map< int, int > m_VspSurfTypeMap;
    int m_VspSurfCfdType;
    unordered_map< int, int > m_VspSurfCfdTypeMap;


    virtual void UpdateSurf();
    // Updates the cfd surface types
    // Needed for transparent custom geoms
    virtual void UpdateFlags();

    bool m_ConformalFlag;
    double m_ConformalOffset;
    virtual void ApplyConformalOffset( double off );
};


#endif // !defined(VSPCUSTOMGEOM__INCLUDED_)
