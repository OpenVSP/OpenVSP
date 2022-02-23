//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "APIDefines.h"
#include "CustomGeom.h"
#include "ParmMgr.h"
#include "ScriptMgr.h"
#include "Vehicle.h"
#include "VSP_Geom_API.h"

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
int CustomGeomMgrSingleton::AddGui( int type, const string & label, const string & parm_name, const string & group_name, double range )
{
    Geom* gptr = VehicleMgr.GetVehicle()->FindGeom( m_CurrGeom );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );

        GuiDef gd;
        gd.m_Type = type;
        gd.m_Label = label;
        gd.m_ParmName = parm_name;
        gd.m_GroupName = group_name;
        gd.m_Range = range;

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

    map< string, string >::iterator iter;
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
CustomXSec::CustomXSec( XSecCurve *xsc ) : SkinXSec( xsc)
{
    m_Type = vsp::XSEC_CUSTOM;

}

//==== Update ====//
void CustomXSec::Update()
{

    m_Type = XSEC_CUSTOM;

    m_LateUpdateFlag = false;

    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();

    // apply the needed transformation to get section into body orientation
    Matrix4d mat;
    xsecsurf->GetBasicTransformation( m_XSCurve->GetWidth(), mat );

    VspCurve baseCurve = GetUntransformedCurve();

    baseCurve.Transform( mat );

    //==== Apply Transform ====//
    m_TransformedCurve = baseCurve;

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

    m_Transform.loadIdentity();

    m_Transform.postMult( tran_mat.data() );
    m_Transform.postMult( cent_mat.data() );
    m_Transform.postMult( rotate_mat.data() );
    m_Transform.postMult( inv_cent_mat.data() );

    m_Transform.postMult( xsecsurf->GetGlobalXForm().data() );

    m_TransformedCurve.Transform( m_Transform );
}

//==== Copy position from base class ====//
void CustomXSec::CopyBasePos( XSec* xs )
{
    if ( xs )
    {
        CustomXSec* cxs = dynamic_cast< CustomXSec* > (xs );
        if ( cxs )
        {
            m_Loc = cxs->m_Loc;
            m_Rot = cxs->m_Rot;
        }
    }
}

//==== Set XSec Location - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetLoc( const vec3d & loc )
{
    m_Loc = loc;
    m_LateUpdateFlag = true;
}

//==== Set XSec Rotation - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetRot( const vec3d & rot )
{
    m_Rot = rot;
    m_LateUpdateFlag = true;
}

double CustomXSec::GetScale()
{
    return 1.0;
}

//==================================================================================================//
//==================================================================================================//
//==================================================================================================//
//==================================================================================================//


//==== Constructor ====//
CustomGeom::CustomGeom( Vehicle* vehicle_ptr ) : Geom( vehicle_ptr )
{
    m_InitGeomFlag = false;
    m_Name = "CustomGeom";
    m_Type.m_Name = "Custom";
    m_Type.m_Type = CUSTOM_GEOM_TYPE;
    m_VspSurfType = vsp::NORMAL_SURF;
    m_VspSurfCfdType = vsp::CFD_NORMAL;
    m_ConformalFlag = false;
}

//==== Destructor ====//
CustomGeom::~CustomGeom()
{
    Clear();
}

void CustomGeom::Clear()
{
    //==== Clear Parms ====//
    for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
    {
        delete m_ParmVec[i];
    }
    m_ParmVec.clear();

    //==== Clear XSec Surfs====//
    ClearXSecSurfs();
}

//==== Init Geometry ====//
void CustomGeom::InitGeom( )
{
    Clear();
    m_InitGeomFlag = true;
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void Init()" );
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void InitGui()" );
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
        if ( m_GuiDefVec[i].m_ParmName.size() > 0 && m_GuiDefVec[i].m_GroupName.size() )
        {
            string parm_id = GetParm( m_ID, m_GuiDefVec[i].m_ParmName, m_GuiDefVec[i].m_GroupName );

            GuiUpdate gu;
            gu.m_GuiID = i;
            gu.m_ParmID = parm_id;
            m_UpdateGuiVec.push_back( gu );
        }
    }

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void UpdateGui()" );

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
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void UpdateSurf()" );
}

void CustomGeom::UpdateFlags()
{
    //==== Set Surf Type ====//
    for ( int i = 0 ; i < (int)m_MainSurfVec.size() ; i++ )
    {
        m_MainSurfVec[i].SetSurfType( m_VspSurfType );
        m_MainSurfVec[i].SetSurfCfdType( m_VspSurfCfdType );

        if ( m_NegativeVolumeFlag.Get() && m_VspSurfCfdType != vsp::CFD_TRANSPARENT )
        {
            m_MainSurfVec[i].SetSurfCfdType( vsp::CFD_NEGATIVE );
        }
    }

    map< int, int >::const_iterator iter;
    for ( iter = m_VspSurfTypeMap.begin() ; iter != m_VspSurfTypeMap.end() ; ++iter )
    {
        int index = iter->first;
        if ( index >= 0 && index < (int)m_MainSurfVec.size() )
        {
            m_MainSurfVec[index].SetSurfType( iter->second );
        }
    }

    for ( iter = m_VspSurfCfdTypeMap.begin() ; iter != m_VspSurfCfdTypeMap.end() ; ++iter )
    {
        int index = iter->first;
        if ( index >= 0 && index < (int)m_MainSurfVec.size() )
        {
            m_MainSurfVec[index].SetSurfCfdType( iter->second );
        }
    }
}

//==== Create Parm and Add To Vector Of Parms ====//
string CustomGeom::AddParm( int type, const string & name, const string & group )
{
    Parm* p = ParmMgr.CreateParm( type );

    if ( p )
    {
        p->Init( name, group, this, 0.0, -1.0e6, 1.0e6 );
        p->SetDescript( "Custom Descript" );
        m_ParmVec.push_back( p );
        return p->GetID();
    }
    return string();
}

//==== Find Parm String Based On Index ===//
string CustomGeom::FindParmID( int index )
{
    if ( index >= 0 && index < ( int )m_ParmVec.size() )
    {
        return m_ParmVec[index]->GetID();

    }
    return string();
}

//==== Remove All XSec Surfs =====//
void CustomGeom::ClearXSecSurfs()
{
    //==== Clear XSec Surfs ====//
    for ( int i = 0 ; i < (int)m_XSecSurfVec.size() ; i++ )
    {
        delete m_XSecSurfVec[i];
    }
    m_XSecSurfVec.clear();


    m_VspSurfType = vsp::NORMAL_SURF;
    m_VspSurfTypeMap.clear();
    m_VspSurfCfdType = vsp::CFD_NORMAL;
    m_VspSurfCfdTypeMap.clear();
}

//==== Add XSec Surface Return ID =====//
string CustomGeom::AddXSecSurf()
{
    XSecSurf* xsec_surf = new XSecSurf();
    xsec_surf->SetXSecType( XSEC_CUSTOM );
    xsec_surf->SetBasicOrientation( X_DIR, Y_DIR, XS_SHIFT_MID, false );
    xsec_surf->SetParentContainer( GetID() );
    m_XSecSurfVec.push_back( xsec_surf );

    return xsec_surf->GetID();
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


//==== Get XSecSurf At Index =====//
XSecSurf* CustomGeom::GetXSecSurf( int index )
{
    if ( index < 0 || index >= ( int )m_XSecSurfVec.size() )
    {
        return NULL;
    }
    return m_XSecSurfVec[index];
}


//==== Skin XSec Surfs ====//
void CustomGeom::SkinXSecSurf( bool closed_flag )
{
    if ( m_ConformalFlag )
    {
        ApplyConformalOffset( m_ConformalOffset );
        m_ConformalFlag = false;
    }


    m_MainSurfVec.resize( m_XSecSurfVec.size() );
    assert( m_XSecSurfVec.size() == m_MainSurfVec.size() );

    for ( int i = 0 ; i < ( int )m_XSecSurfVec.size() ; i++ )
    {
        //==== Remove Duplicate XSecs ====//
        vector< CustomXSec* > xsec_vec;
        for ( int j = 0 ; j < m_XSecSurfVec[i]->NumXSec() ; j++ )
        {
            VspCurve last_crv;
            CustomXSec* xs = dynamic_cast<CustomXSec*>( m_XSecSurfVec[i]->FindXSec( j ) );
            if ( xs )
            {
                if ( j == 0 )
                {
                    xsec_vec.push_back( xs );
                    last_crv = xs->GetCurve();
                }
                else
                {
                    VspCurve crv = xs->GetCurve();
                    if ( !last_crv.IsEqual( crv ) )
                    {
                        xsec_vec.push_back( xs );
                        last_crv = crv;
                    }
                }
            }
        }
        //===== Make Sure Last XS is Exact ====//
        if ( xsec_vec.size() > 2 && closed_flag )
        {
            xsec_vec[ xsec_vec.size()-1] = xsec_vec[0];
        }

        //==== Cross Section Curves & joint info ====//
        vector< rib_data_type > rib_vec;

        //==== Update XSec Location/Rotation ====//
       for ( int j = 0 ; j < (int)xsec_vec.size() ; j++ )
       {
            CustomXSec* xs = xsec_vec[j];
            xs->SetLateUpdateFlag( true );

            VspCurve crv = xs->GetCurve();

            //==== Load Ribs ====//
            if ( j == 0 )
                rib_vec.push_back( xs->GetRib( true, false ) );
            else if ( j == m_XSecSurfVec[i]->NumXSec() -1 )
                rib_vec.push_back( xs->GetRib( false, true ) );
            else
                rib_vec.push_back( xs->GetRib( false, false ) );
       }

        if ( xsec_vec.size() >= 2 )
        {
            m_MainSurfVec[i].SkinRibs( rib_vec, false );
            m_MainSurfVec[i].SetMagicVParm( false );
        }
    }
}

//==== Make A Copy Of MainSurf at Index and Apply XForm ====//
void CustomGeom::CloneSurf( int index, Matrix4d & mat )
{
    if ( index >= 0 && index < (int)m_MainSurfVec.size() )
    {
        VspSurf clone = m_MainSurfVec[index];
        clone.Transform( mat );
        clone.SetClone( index, mat );
        m_MainSurfVec.push_back( clone );
    }
}

//==== Apply Transformation for a Main Surface ====//
void CustomGeom::TransformSurf( int index, Matrix4d & mat )
{
    if ( index >= 0 && index < (int)m_MainSurfVec.size() )
    {
        m_MainSurfVec[index].Transform( mat );
    }
}

//==== Set VSP Surf Type ====//
void CustomGeom::SetVspSurfType( int type, int surf_id )
{
    if ( surf_id == -1 )
        m_VspSurfType = type;
    else
    {
        m_VspSurfTypeMap[surf_id] = type;
    }
}

//==== Set VSP Surf CFD Type ====//
void CustomGeom::SetVspSurfCfdType( int type, int surf_id )
{
    if ( surf_id == -1 )
        m_VspSurfCfdType = type;
    else
    {
        m_VspSurfCfdTypeMap[surf_id] = type;
    }
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CustomGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr custom_node = xmlNewChild( node, NULL, BAD_CAST "CustomGeom", NULL );
    if ( custom_node )
    {
        string file_contents = ScriptMgr.FindModuleContent( GetScriptModuleName() );
        string incl_contents =  ScriptMgr.ReplaceIncludes( file_contents, "" );

        string safe_file_contents = XmlUtil::ConvertToXMLSafeChars( incl_contents );

        for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
        {
            m_ParmVec[i]->EncodeXml( custom_node );
        }

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

        string new_module_name = ScriptMgr.ReadScriptFromMemory( module_name, file_contents );
        CustomGeomMgr.InitGeom( GetID(), new_module_name, module_name );

        for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
        {
            m_ParmVec[i]->DecodeXml( custom_node );
        }
    }
    Geom::DecodeXml( node );

    return custom_node;
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
    int success = ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void ComputeCenter()" );

    if ( success == 0 )
        return;

    //==== No Custom Script - Use Default ====//
    if ( m_XSecSurfVec.size() < 1 )
         return;

    int index = m_XSecSurfVec[0]->NumXSec() - 1;
    CustomXSec* xs = dynamic_cast<CustomXSec*>( m_XSecSurfVec[0]->FindXSec( index - 1 ) );

    if ( xs )
    {
        m_Center = vec3d(0,0,0);
        m_Center.set_x( m_Origin()*xs->GetLoc().x() );
    }
}

//==== Optional Scale - If Script Does Not Exist Nothing Happens =====//
void CustomGeom::Scale()
{
    if ( !m_InitGeomFlag )
    {
        return;
    }

    CustomGeomMgr.SetCurrCustomGeom( GetID() );

    double curr_scale = m_Scale()/m_LastScale();

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( GetScriptModuleName().c_str(), "void Scale(double s)", true, curr_scale );

    m_LastScale = m_Scale();
}

//==== Trigger Conformal XSec Offset =====//
void CustomGeom::OffsetXSecs( double off )
{
    m_ConformalFlag = true;
    m_ConformalOffset = off;
}

//==== Apply Conformal Offset ====//
void CustomGeom::ApplyConformalOffset( double off )
{

    for ( int i = 0 ; i < ( int )m_XSecSurfVec.size() ; i++ )
    {

        int nxsec = m_XSecSurfVec[i]->NumXSec();
        for ( int j = 0 ; j < nxsec ; j++ )
        {
            XSec* xs = m_XSecSurfVec[i]->FindXSec( j );
            if ( xs )
            {
                XSecCurve* xsc = xs->GetXSecCurve();
                if ( xsc )
                {
                    xsc->OffsetCurve( off );
                }
            }
        }
    }
}
