//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "APIDefines.h"
#include "CustomGeom.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"
#include "APIErrorMgr.h"

#include <cstdlib>

using namespace vsp;

//==== Constructor ====//
CustomGeomMgrSingleton::CustomGeomMgrSingleton()
{
}

//==== Scan Custom Directory And Return All Possible Types ====//
void CustomGeomMgrSingleton::ReadCustomScripts( Vehicle* veh )
{
    //==== Only Read Once ====//
    static bool init_flag = false;
    if ( init_flag )
        return;
    init_flag = true;

//jrg Test Include
//string inc_content = ScriptMgr.ExtractContent( "CustomScripts/TestIncludes.as" );
//string repl_content = ScriptMgr.ReplaceIncludes( inc_content, "CustomScripts/" );

    m_CustomTypeVec.clear();

    vector < string > scriptDirs = veh->GetCustomScriptDirs();

    for ( int k = 0 ; k < scriptDirs.size(); k++ )
    {
        // ReadScriptsFromDir is clever enough to not allow duplicate content.  Duplicate
        // content returns a repeated module name.  Repeated file names with different content
        // are made unique and returned.  This filters duplicate module names to prevent displaying
        // duplicates.
        vector< string > mod_vec = ScriptMgr.ReadScriptsFromDir( scriptDirs[k], ".vsppart" );

        for ( int i = 0 ; i < (int)mod_vec.size() ; i++ )
        {
            if( m_ModuleGeomIDMap.find( mod_vec[i] ) == m_ModuleGeomIDMap.end() )
            {
                m_CustomTypeVec.push_back( GeomType( CUSTOM_GEOM_TYPE, mod_vec[i], false, mod_vec[i], mod_vec[i] ) );
                m_ModuleGeomIDMap[ mod_vec[i] ] = string();
            }
        }
    }
}

//==== Init Custom Geom ====//
void CustomGeomMgrSingleton::InitGeom( const string& id, const string& module_name, const string& display_name )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    Geom* gptr = veh->FindGeom( id );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        m_CurrGeom = id;
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SetType( GeomType( CUSTOM_GEOM_TYPE, display_name, false, module_name, display_name ) );
        custom_geom->SetScriptModuleName( module_name );
        custom_geom->SetDisplayName( display_name );
        custom_geom->InitGeom();

        m_ModuleGeomIDMap[ module_name ] = id;
    }
}

//==== Add Parm To Current Custom Geom ====//
string CustomGeomMgrSingleton::AddParm( int type, const string & name, const string & group )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    Geom* gptr = veh->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        return custom_geom->AddParm( type, name, group );
    }

    return string();
}

string CustomGeomMgrSingleton::GetCustomParm( int index )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        return custom_geom->FindParmID( index );
    }
    return string();
}


//==== Add Gui Device Build Data For Custom Geom ====//
int CustomGeomMgrSingleton::AddGui( int type, const string & arg1, const string & arg2, const string & arg3, double arg4,
                                    const string & arg5 )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        // A choice item's arg5, where given, is the integer value the item sets.  One that is not an integer
        // is reported, and the item keeps its place in the list as its value.
        if ( type == vsp::GDEV_ADD_CHOICE_ITEM && !arg5.empty() )
        {
            char* end = nullptr;
            strtol( arg5.c_str(), &end, 10 );
            if ( end == arg5.c_str() || *end != '\0' )
            {
                vsp::ErrorMgr.AddError( vsp::VSP_INVALID_INPUT_VAL, "AddGui::GDEV_ADD_CHOICE_ITEM " + arg1 +
                                        " value " + arg5 + " is not an integer" );
            }
        }

        // A two input slider shows two Parms, so a Parm named for the first needs one for the second.
        if ( type == vsp::GDEV_SLIDER_ADJ_RANGE_TWO_INPUT && !arg2.empty() && arg5.empty() )
        {
            vsp::ErrorMgr.AddError( vsp::VSP_INVALID_INPUT_VAL, "AddGui::GDEV_SLIDER_ADJ_RANGE_TWO_INPUT " + arg1 +
                                    " has Parm " + arg2 + " for its first input but none for its second" );
            return -1;
        }

        // What each argument means depends on the type; these are the meanings of most controls.
        GuiDef gd;
        gd.m_Type = type;
        gd.m_Label = arg1;
        gd.m_ParmName = arg2;
        gd.m_GroupName = arg3;
        gd.m_Range = arg4;
        gd.m_ParmName2 = arg5;

        return custom_geom->AddGui( gd );
    }
    return -1;
}

//==== Update Gui ====//
void CustomGeomMgrSingleton::AddUpdateGui( int gui_id, const string & parm_id )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );
    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        GuiUpdate gu;
        gu.m_GuiID = gui_id;
        gu.m_ParmID = parm_id;
        gu.m_Input = 1;

        custom_geom->AddUpdateGui( gu );
    }
}

//==== Update Two Input Gui ====//
void CustomGeomMgrSingleton::AddUpdateGui( int gui_id, const string & parm_id, const string & parm_id2, int input )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );
    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        GuiUpdate gu;
        gu.m_GuiID = gui_id;
        gu.m_ParmID = parm_id;
        gu.m_ParmID2 = parm_id2;
        gu.m_Input = input;

        custom_geom->AddUpdateGui( gu );
    }
}

//==== Get GuiDef =====//
vector< GuiDef > CustomGeomMgrSingleton::GetGuiDefVec( const string & geom_id )
{
    SetCurrCustomGeom( geom_id );
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        return custom_geom->GetGuiDefVec();
    }
    vector< GuiDef > defvec;
    return defvec;
}


//===== Check And Clear Trigger Event ====//
bool CustomGeomMgrSingleton::CheckClearTriggerEvent( int gui_id )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );
    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        return custom_geom->CheckClearTriggerEvent( gui_id );
    }
    return false;
}

//==== Build Update Gui Instruction Vector ====//
vector< GuiUpdate > CustomGeomMgrSingleton::GetGuiUpdateVec()
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );
    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        return custom_geom->GetGuiUpdateVec();
    }
    vector< GuiUpdate > defvec;
    return defvec;
}

//==== Add XSec Surface To Current Geom - Return ID =====//
string CustomGeomMgrSingleton::AddXSecSurf()
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        return custom_geom->AddXSecSurf();
    }
    return string();
}

//==== Remove XSec Surface To Current Geom ====//
void CustomGeomMgrSingleton::RemoveXSecSurf( const string& id )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->RemoveXSecSurf( id );
    }
}

//==== Clear XSec Surface From Current Geom =====//
void CustomGeomMgrSingleton::ClearXSecSurfs()
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        return custom_geom->ClearXSecSurfs();
    }
}

//==== Skin XSec Surf =====//
void CustomGeomMgrSingleton::SkinXSecSurf( bool closed_flag )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SkinXSecSurf(closed_flag );
    }
}

//==== Clone Surf And Apply Transform =====//
void CustomGeomMgrSingleton::CloneSurf( int index, Matrix4d & mat )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->CloneSurf( index, mat );
    }
}

//==== Clone Surf And Apply Transform =====//
void CustomGeomMgrSingleton::TransformSurf( int index, Matrix4d & mat )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->TransformSurf( index, mat );
    }
}

//==== Set VSP Surf Type For Current Custom Geom =====//
void CustomGeomMgrSingleton::SetVspSurfType( int type, int surf_id )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SetVspSurfType( type, surf_id );
    }
}

//==== Set VSP Surf Type For Current Custom Geom =====//
void CustomGeomMgrSingleton::SetVspSurfCfdType( int type, int surf_id )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SetVspSurfCfdType( type, surf_id );
    }
}

void CustomGeomMgrSingleton::SetupCustomDefaultSource(  int type, int surf_index,
                                                        double l1, double r1, double u1, double w1,
                                                        double l2, double r2, double u2, double w2 )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        SourceData sd;
        sd.m_Type = type;
        sd.m_SurfIndex = surf_index;
        sd.m_Len1 = l1;
        sd.m_Rad1 = r1;
        sd.m_U1 = u1;
        sd.m_W1 = w1;
        sd.m_Len2 = l2;
        sd.m_Rad2 = r2;
        sd.m_U2 = u2;
        sd.m_W2 = w2;

        custom_geom->SetUpDefaultSource( sd );
    }
}

void CustomGeomMgrSingleton::ClearAllCustomDefaultSources()
{
   Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->ClearAllDefaultSources();
    }
}

void CustomGeomMgrSingleton::SetCustomCenter( double x, double y, double z )
{
   Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SetCenter( x, y, z );
    }
}



//==== Custom XSecs Functions =====//
void CustomGeomMgrSingleton::SetCustomXSecLoc( const string & xsec_id, const vec3d & loc )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( xsec_id );

    if ( !pc )
        return;

    CustomXSec* cxs = dynamic_cast<CustomXSec*>( pc );
    if ( !cxs )
        return;

    cxs->SetLoc( loc );
}

//==== Custom XSecs Functions =====//
vec3d CustomGeomMgrSingleton::GetCustomXSecLoc( const string & xsec_id )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( xsec_id );

    if ( !pc )
        return vec3d();

    CustomXSec* cxs = dynamic_cast<CustomXSec*>( pc );
    if ( !cxs )
        return vec3d();

    return cxs->GetLoc();
}

//==== Custom XSecs Functions =====//
void CustomGeomMgrSingleton::SetCustomXSecRot( const string & xsec_id, const vec3d & rot )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( xsec_id );
    if ( !pc )
        return;

    CustomXSec* cxs = dynamic_cast<CustomXSec*>( pc );
    if ( !cxs )
        return;

    cxs->SetRot( rot );
}

//==== Custom XSecs Functions =====//
vec3d CustomGeomMgrSingleton::GetCustomXSecRot( const string & xsec_id )
{
    ParmContainer* pc = ParmMgr.FindParmContainer( xsec_id );
    if ( !pc )
        return vec3d();

    CustomXSec* cxs = dynamic_cast<CustomXSec*>( pc );
    if ( !cxs )
        return vec3d();

    return cxs->GetRot();
}

//=== Append Custom XSec ====//
string CustomGeomMgrSingleton::AppendCustomXSec( const string & xsec_surf_id, int type )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        for ( int i = 0 ; i < custom_geom->GetNumXSecSurfs() ; i++ )
        {
            XSecSurf* xs_surf = custom_geom->GetXSecSurf( i );
            if ( xs_surf && xs_surf->GetID() == xsec_surf_id )
            {
                return xs_surf->AddXSec( type );
            }
        }
    }

    return string();

}

//=== Cut Custom XSec ====//
void CustomGeomMgrSingleton::CutCustomXSec( const string & xsec_surf_id, int index )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        int num = custom_geom->GetNumXSecSurfs();
        for ( int i = 0 ; i < num ; i++ )
        {
            XSecSurf* xs_surf = custom_geom->GetXSecSurf( i );
            if ( xs_surf && xs_surf->GetID() == xsec_surf_id )
            {
                xs_surf->CutXSec( index );
                // Set up flag so Update() knows to regenerate surface.
                // Insert / split cases don't need this because Parms are added,
                // which implicitly triggers this flag.
                // However, cut deletes Parms - requiring an explicit flag.
                gptr->m_SurfDirty = true;
            }
        }
    }
}

//=== Copy Custom XSec ====//
void CustomGeomMgrSingleton::CopyCustomXSec( const string & xsec_surf_id, int index )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        int num = custom_geom->GetNumXSecSurfs();
        for ( int i = 0 ; i < num ; i++ )
        {
            XSecSurf* xs_surf = custom_geom->GetXSecSurf( i );
            if ( xs_surf && xs_surf->GetID() == xsec_surf_id )
            {
                xs_surf->CopyXSec( index );
            }
        }
    }
}

//=== Paste Custom XSec ====//
void CustomGeomMgrSingleton::PasteCustomXSec( const string & xsec_surf_id, int index )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        for ( int i = 0 ; i < custom_geom->GetNumXSecSurfs() ; i++ )
        {
            XSecSurf* xs_surf = custom_geom->GetXSecSurf( i );
            if ( xs_surf && xs_surf->GetID() == xsec_surf_id )
            {
                xs_surf->PasteXSec( index );
                custom_geom->m_SurfDirty = true;
            }
        }
    }
}

//=== Insert Custom XSec ====//
string CustomGeomMgrSingleton::InsertCustomXSec( const string & xsec_surf_id, int type, int index )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        for ( int i = 0 ; i < custom_geom->GetNumXSecSurfs() ; i++ )
        {
            XSecSurf* xs_surf = custom_geom->GetXSecSurf( i );
            if ( xs_surf && xs_surf->GetID() == xsec_surf_id )
            {
                return xs_surf->InsertXSec( type, index );
            }
        }
    }
    return string();
}

//==== Get All Custom Script Module Name ====//
vector< string > CustomGeomMgrSingleton::GetCustomScriptModuleNames()
{
    vector< string > module_name_vec;

    unordered_map< string, string >::iterator iter;
    for ( iter = m_ModuleGeomIDMap.begin() ; iter != m_ModuleGeomIDMap.end() ; ++iter )
    {
        module_name_vec.push_back( iter->first );
    }
    return module_name_vec;
}

int CustomGeomMgrSingleton::SaveScriptContentToFile( const string & module_name, const string & file_name )
{
    return ScriptMgr.SaveScriptContentToFile( module_name, file_name );
}

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==== Constructor ====//
CustomXSec::CustomXSec( XSecCurve *xsc ) : BareXSec( xsc )
{
    m_Name = "CustomXSec";
    m_Type = vsp::XSEC_CUSTOM;
}

// The section is turned about the origin of its XSecSurf, then put at its location.
void CustomXSec::GetPlacement( Matrix4d & placement )
{
    Matrix4d tran_mat;
    tran_mat.translatef( m_Loc.x(),  m_Loc.y(),  m_Loc.z() );

    Matrix4d rotate_mat;
    rotate_mat.rotateX( m_Rot.x() );
    rotate_mat.rotateY( m_Rot.y() );
    rotate_mat.rotateZ( m_Rot.z() );

    Matrix4d cent_mat;
    cent_mat.translatef( -m_Loc.x(), -m_Loc.y(), -m_Loc.z() );

    Matrix4d inv_cent_mat;
    inv_cent_mat.translatef(  m_Loc.x(),  m_Loc.y(),  m_Loc.z() );

    placement.loadIdentity();

    placement.postMult( tran_mat.data() );
    placement.postMult( cent_mat.data() );
    placement.postMult( rotate_mat.data() );
    placement.postMult( inv_cent_mat.data() );
}

//==== Copy position from base class ====//
void CustomXSec::CopyBasePos( XSec* xs )
{
    BareXSec::CopyBasePos( xs );

    CustomXSec* cxs = dynamic_cast< CustomXSec* > (xs );
    if ( cxs )
    {
        m_Rot = cxs->m_Rot;
    }
}

//==== Set XSec Rotation - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetRot( const vec3d & rot )
{
    m_Rot = rot;
    m_LateUpdateFlag = true;
}

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//


//==== Constructor ====//
CustomGeom::CustomGeom( Vehicle* vehicle_ptr ) : XSecSurfGeom( vehicle_ptr )
{
    m_InitGeomFlag = false;
    m_Name = "CustomGeom";
    m_Type.m_Name = "Custom";
    m_Type.m_Type = CUSTOM_GEOM_TYPE;

    // A script can build any shape, so its end caps take every type.
    m_CapUMinOption.SetAllowedVals( CapTypes( true ) );
    m_CapUMaxOption.SetAllowedVals( CapTypes( true ) );

    m_NoseOriginXFlag.Init( "NoseOriginXFlag", "Design", this, false, false, true );
    m_NoseOriginXFlag.SetDescript( "Flag to place the nose tip of the first surface at the origin in X" );
    m_NoseOriginYFlag.Init( "NoseOriginYFlag", "Design", this, false, false, true );
    m_NoseOriginYFlag.SetDescript( "Flag to place the nose tip of the first surface at the origin in Y" );
    m_NoseOriginZFlag.Init( "NoseOriginZFlag", "Design", this, false, false, true );
    m_NoseOriginZFlag.SetDescript( "Flag to place the nose tip of the first surface at the origin in Z" );
}

//==== Destructor ====//
CustomGeom::~CustomGeom()
{
    Clear();
}

void CustomGeom::Clear()
{
    //==== Clear Parms ====//
    for ( int i = 0 ; i < (int)m_CustomGeomParmVec.size() ; i++ )
    {
        delete m_CustomGeomParmVec[i];
    }
    m_CustomGeomParmVec.clear();

    //==== Clear XSec Surfs====//
    ClearXSecSurfs();
}

//==== Add XSec Surface Return ID =====//
string CustomGeom::AddXSecSurf()
{
    return NewXSecSurf( XSEC_CUSTOM )->GetID();
}

//==== Remove XSec Surface  ====//
void CustomGeom::RemoveXSecSurf( const string& id )
{
    vector< XSecSurf* > new_vec;
    for ( int i = 0 ; i < (int)m_XSecSurfVec.size() ; i++ )
    {
        if ( m_XSecSurfVec[i]->GetID() == id )
        {
            delete m_XSecSurfVec[i];
        }
        else
        {
            new_vec.push_back( m_XSecSurfVec[i] );
        }
    }

    m_XSecSurfVec = new_vec;
}

void CustomGeom::ClearXSecSurfs()
{
    XSecSurfGeom::ClearXSecSurfs();

    m_VspSurfTypeMap.clear();
    m_VspSurfCfdTypeMap.clear();
}

//==== Set VSP Surf Type ====//
void CustomGeom::SetVspSurfType( int type, int surf_id )
{
    if ( surf_id == -1 )
    {
        XSecSurfGeom::SetVspSurfType( type );
    }
    else
    {
        m_VspSurfTypeMap[surf_id] = type;
    }
}

// The type surface surf_id is given, or every surface is.
int CustomGeom::GetVspSurfType( int surf_id ) const
{
    unordered_map< int, int >::const_iterator iter = m_VspSurfTypeMap.find( surf_id );
    if ( iter != m_VspSurfTypeMap.end() )
    {
        return iter->second;
    }
    return m_VspSurfType;
}

//==== Set VSP Surf CFD Type ====//
void CustomGeom::SetVspSurfCfdType( int type, int surf_id )
{
    if ( surf_id == -1 )
    {
        XSecSurfGeom::SetVspSurfCfdType( type );
    }
    else
    {
        m_VspSurfCfdTypeMap[surf_id] = type;
    }
}

void CustomGeom::UpdateFlags()
{
    XSecSurfGeom::UpdateFlags();

    unordered_map< int, int >::const_iterator iter;
    for ( iter = m_VspSurfCfdTypeMap.begin() ; iter != m_VspSurfCfdTypeMap.end() ; ++iter )
    {
        int index = iter->first;
        if ( index >= 0 && index < (int)m_MainSurfVec.size() )
        {
            m_MainSurfVec[index].SetSurfCfdType( iter->second );
        }
    }
}

//==== Add All Default Sources Currently in Vec =====//
void CustomGeom::AddDefaultSources( double base_len )
{
    for ( int i = 0 ; i < (int)m_DefaultSourceVec.size() ; i++ )
    {
        SourceData sd = m_DefaultSourceVec[i];
        vsp::AddCFDSource( sd.m_Type, GetID(), sd.m_SurfIndex, sd.m_Len1, sd.m_Rad1, sd.m_U1, sd.m_W1,
                           sd.m_Len2, sd.m_Rad2, sd.m_U2, sd.m_W2 );
    }
}

//==== Init Geometry ====//
void CustomGeom::InitGeom( )
{
    Clear();
    m_InitGeomFlag = true;
    RunScript( "void Init()" );
    RunScript( "void InitGui()" );
    SetName( GetScriptModuleName() );
    Update();

}

//==== Add Gui Definition ====//
int CustomGeom::AddGui( const GuiDef & gd )
{
    m_GuiDefVec.push_back( gd );
    return m_GuiDefVec.size() - 1;
}

//==== Add Parm->Gui Match ====//
void CustomGeom::AddUpdateGui( const GuiUpdate & gu )
{
    m_UpdateGuiVec.push_back( gu );
}

//==== Match Parm to GUI by Executing UpdateGui Script ====//
vector< GuiUpdate > CustomGeom::GetGuiUpdateVec()
{
    m_UpdateGuiVec.clear();

    //==== Load Predefined UpdateGui Parm Matches ====//
    for ( int i = 0 ; i < (int)m_GuiDefVec.size() ; i++ )
    {
        // A choice item ties to no Parm.
        if ( m_GuiDefVec[i].m_ParmName.size() > 0 && m_GuiDefVec[i].m_GroupName.size() &&
             m_GuiDefVec[i].m_Type != vsp::GDEV_ADD_CHOICE_ITEM )
        {
            string parm_id = GetParm( m_ID, m_GuiDefVec[i].m_ParmName, m_GuiDefVec[i].m_GroupName );

            GuiUpdate gu;
            gu.m_GuiID = i;
            gu.m_ParmID = parm_id;
            gu.m_Input = 1;
            if ( m_GuiDefVec[i].m_ParmName2.size() > 0 )
            {
                gu.m_ParmID2 = GetParm( m_ID, m_GuiDefVec[i].m_ParmName2, m_GuiDefVec[i].m_GroupName );
            }
            m_UpdateGuiVec.push_back( gu );
        }
    }

    //==== Call Script ====//
    RunScript( "void UpdateGui()" );

    return m_UpdateGuiVec;
}

//==== Add A Gui Trigger Event ====//
void CustomGeom::AddGuiTriggerEvent( int index )
{
    if ( m_TriggerVec.size() != m_GuiDefVec.size() )
    {
        m_TriggerVec.resize( m_GuiDefVec.size(), 0 );
    }

    if ( index >= 0 && index < (int)m_TriggerVec.size() )
    {
        m_TriggerVec[index] = 1;
    }
}

//==== Check and Clear A Trigger Event ====//
bool CustomGeom::CheckClearTriggerEvent( int index )
{
    bool trigger = false;
    if ( index >= 0 && index < (int)m_TriggerVec.size() )
    {
        if ( m_TriggerVec[index] == 1 )
        {
            trigger = true;
            m_TriggerVec[index] = 0;
            ForceUpdate();
        }
    }
    return trigger;
}


//==== Update Surf By Executing Update Surf Script ====//
void CustomGeom::UpdateSurf()
{
    if ( !m_InitGeomFlag )
    {
        return;
    }

    CustomGeomMgr.SetCurrCustomGeom( GetID() );

    //==== Call Script ====//
    RunScript( "void UpdateSurf()" );
}

void CustomGeom::UpdateEndCaps()
{
    if ( m_CappingDone )
    {
        return;
    }

    Geom::UpdateEndCaps();

    ShiftNoseToOrigin( m_NoseOriginXFlag(), m_NoseOriginYFlag(), m_NoseOriginZFlag() );
}

//==== Create Parm and Add To Vector Of Parms ====//
string CustomGeom::AddParm( int type, const string & name, const string & group )
{
    Parm* p = ParmMgr.CreateParm( type );

    if ( p )
    {
        p->Init( name, group, this, 0.0, -1.0e6, 1.0e6 );
        p->SetDescript( "Custom Descript" );
        m_CustomGeomParmVec.push_back( p );
        return p->GetID();
    }
    return string();
}

//==== Find Parm String Based On Index ===//
string CustomGeom::FindParmID( int index )
{
    if ( index >= 0 && index < ( int )m_CustomGeomParmVec.size() )
    {
        return m_CustomGeomParmVec[index]->GetID();

    }
    return string();
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CustomGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr custom_node = xmlNewChild( node, nullptr, BAD_CAST "CustomGeom", nullptr );
    if ( custom_node )
    {
        string file_contents = ScriptMgr.FindModuleContent( GetScriptModuleName() );
        string incl_contents =  ScriptMgr.ReplaceIncludes( file_contents, "" );

        string safe_file_contents = XmlUtil::ConvertToXMLSafeChars( incl_contents );

        XmlUtil::AddStringNode( custom_node, "ScriptFileModule", GetScriptModuleName()  );
        XmlUtil::AddStringNode( custom_node, "ScriptFileContents", safe_file_contents );
    }
    Geom::EncodeXml( node );

    return custom_node;
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CustomGeom::DecodeXml( xmlNodePtr & node )
{

    xmlNodePtr custom_node = XmlUtil::GetNode( node, "CustomGeom", 0 );
    if ( custom_node )
    {
        string module_name = XmlUtil::FindString( custom_node, "ScriptFileModule", GetScriptModuleName() );
        string safe_file_contents = XmlUtil::FindString( custom_node, "ScriptFileContents", string() );
        string file_contents = XmlUtil::ConvertFromXMLSafeChars( safe_file_contents );

        // The type is named after the module the script is loaded as in this session, as AddGeom names it,
        // not the saved name, which can carry the saving session's rename of a module of the same name.
        string new_module_name = ScriptMgr.ReadScriptFromMemory( module_name, file_contents );
        string type_name = new_module_name;
        if ( type_name.empty() )
        {
            type_name = module_name;
        }
        CustomGeomMgr.InitGeom( GetID(), new_module_name, type_name );
    }

    // The script's Parms, made again above, are read with the rest of the Geom's.
    Geom::DecodeXml( node );

    return custom_node;
}

SubSurface* CustomGeom::AddSubSurf( int type, int surfindex )
{
    SubSurface* ss = XSecSurfGeom::AddSubSurf( type, surfindex );
    if ( ss && CustomGeomMgr.GetRunningCustomGeom() == m_ID )
    {
        ss->SetListedFlag( false );
    }
    return ss;
}

//==== Run A Function Of This Component's Script ====//
int CustomGeom::RunScript( const char* function_name, bool arg_flag, double arg )
{
    // A script may update another custom component, whose script then runs within this one's.
    string outer = CustomGeomMgr.GetRunningCustomGeom();
    CustomGeomMgr.SetRunningCustomGeom( GetID() );
    int result = ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), function_name, arg_flag, arg );
    CustomGeomMgr.SetRunningCustomGeom( outer );
    return result;
}

//==== Compute Center of Rotation =====//
void CustomGeom::ComputeCenter()
{
    //==== Try Calling Script Function First ====//
    if ( !m_InitGeomFlag )
    {
        return;
    }

    CustomGeomMgr.SetCurrCustomGeom( GetID() );

    //==== Call Script ====//
    int success = RunScript( "void ComputeCenter()" );

    if ( success == 0 )
        return;

    //==== No Custom Script - Use Default ====//
    // From the last cross section but one of the first surface.
    if ( m_XSecSurfVec.size() < 1 )
    {
        return;
    }

    // The last section's X is the length.
    int index = m_XSecSurfVec[0]->NumXSec() - 1;
    CustomXSec* xs = dynamic_cast<CustomXSec*>( m_XSecSurfVec[0]->FindXSec( index ) );

    if ( xs )
    {
        m_Center = vec3d( 0, 0, 0 );
        m_Center.set_x( m_Origin() * xs->GetLoc().x() );
    }
}

//==== Optional Scale - If Script Does Not Exist Nothing Happens =====//
void CustomGeom::ApplyScale( double currentScale )
{
    if ( !m_InitGeomFlag )
    {
        return;
    }

    CustomGeomMgr.SetCurrCustomGeom( GetID() );

    //==== Call Script ====//
    RunScript( "void Scale(double s)", true, currentScale );
}
