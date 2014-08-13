//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#include "CustomGeom.h"
#include "ParmMgr.h"
#include "LinkMgr.h"
#include "VspSurf.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VSP_Geom_API.h"
#include "ScriptMgr.h"
#include "FileUtil.h"

#include "APIDefines.h"
using namespace vsp;

//==== Constructor ====//
CustomGeomMgrSingleton::CustomGeomMgrSingleton()
{
//    m_ScriptDir = "./CustomScripts/";
    m_ScriptDir = "../../../CustomScripts/";
}

//==== Scan Custom Directory And Return All Possible Types ====//
void CustomGeomMgrSingleton::ReadCustomScripts()
{
    //==== Only Read Once ====//
    static bool init_flag = false;
    if ( init_flag )
        return;
    init_flag = true;

    m_CustomTypeVec.clear();

    vector< string > mod_vec = ScriptMgr.ReadScriptsFromDir( m_ScriptDir );

    for ( int i = 0 ; i < (int)mod_vec.size() ; i++ )
    {
        m_CustomTypeVec.push_back( GeomType( CUSTOM_GEOM_TYPE, mod_vec[i], false, mod_vec[i] ) );
        m_ModuleGeomIDMap[ mod_vec[i] ] = string();
    }
}

//==== Init Custom Geom ====//
void CustomGeomMgrSingleton::InitGeom( const string& id, const string& module_name )
{
    Vehicle* veh = VehicleMgr.GetVehicle();
    Geom* gptr = veh->FindGeom( id );

    //==== Check If Geom is Valid and Correct Type ====//
    if ( gptr && gptr->GetType().m_Type == CUSTOM_GEOM_TYPE )
    {
        m_CurrGeom = id;
        CustomGeom* custom_geom = dynamic_cast<CustomGeom*>( gptr );
        custom_geom->SetScriptModuleName( module_name );
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


//==== Get All Custom Script Module Name ====//
vector< string > CustomGeomMgrSingleton::GetCustomScriptModuleNames()
{
    vector< string > module_name_vec;

    map< string, string >::iterator iter;
    for ( iter = m_ModuleGeomIDMap.begin() ; iter != m_ModuleGeomIDMap.end() ; iter++ )
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
CustomXSec::CustomXSec( XSecCurve *xsc, bool use_left ) : SkinXSec( xsc, use_left)
{
    m_Type = vsp::XSEC_CUSTOM;

};

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
    vec3d cent = m_CenterRot + m_Loc;
    cent_mat.translatef( -cent.x(), -cent.y(), -cent.z() );

    Matrix4d inv_cent_mat;
    inv_cent_mat.translatef(  cent.x(),  cent.y(),  cent.z() );

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
            m_CenterRot = cxs->m_CenterRot;
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

//==== Set XSec Center Rotation - Not Using Parms To Avoid Exposing Dependant Vars ====//
void CustomXSec::SetCenterRot( const vec3d & cent )
{
    m_CenterRot = cent;
    m_LateUpdateFlag = true;
}

double CustomXSec::GetLScale()
{
return 1.0;
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    vec3d prevLoc;
    if( indx == 0 )
    {
        return GetRScale();
    }
    else
    {
        CustomXSec* prevxs = (CustomXSec*) xsecsurf->FindXSec( indx - 1);
        if( prevxs )
        {
            prevLoc = prevxs->GetLoc();
        }
    }

    return dist( m_Loc, prevLoc );
}

double CustomXSec::GetRScale()
{
return 1.0;
    XSecSurf* xsecsurf = (XSecSurf*) GetParentContainerPtr();
    int indx = xsecsurf->FindXSecIndex( m_ID );

    vec3d nextLoc;
    if( indx < ( xsecsurf->NumXSec() - 1 ) )
    {
        CustomXSec* nxtxs = (CustomXSec*) xsecsurf->FindXSec( indx + 1);
        if( nxtxs )
        {
            nextLoc = nxtxs->GetLoc();
        }
    }
    else
    {
        return GetLScale();
    }

    return dist( m_Loc, nextLoc );
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

//==== Match Parm to GUI by Execting UpdateGui Script ====//
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

//==== Add A Gui Tigger Event ====//
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

//==== Check and Clear A Tigger Event ====//
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

//==== Create Parm and Add To Vector Of Parms ====//
string CustomGeom::AddParm( int type, const string & name, const string & group )
{
    Parm* p = ParmMgr.CreateParm( type );

    if ( p )
    {
        p->Init( name, group, this, 0.0, -1.0e6, 1.0e6, true );
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
                    last_crv = xs->GetCurve();;
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
        m_MainSurfVec.push_back( clone );
    }
}

//==== Make A Copy Of MainSurf at Index and Apply XForm ====//
void CustomGeom::TransformSurf( int index, Matrix4d & mat )
{
    if ( index >= 0 && index < (int)m_MainSurfVec.size() )
    {
        m_MainSurfVec[index].Transform( mat );
    }
}

//==== Encode Data Into XML Data Struct ====//
xmlNodePtr CustomGeom::EncodeXml( xmlNodePtr & node )
{
    xmlNodePtr custom_node = xmlNewChild( node, NULL, BAD_CAST "CustomGeom", NULL );
    if ( custom_node )
    {
        string file_contents = ScriptMgr.FindModuleContent( GetScriptModuleName() );

        for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
        {
            m_ParmVec[i]->EncodeXml( custom_node );
        }

        XmlUtil::AddStringNode( custom_node, "ScriptFileModule", GetScriptModuleName()  );
        XmlUtil::AddStringNode( custom_node, "ScriptFileContents", file_contents );
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
        string file_contents = XmlUtil::FindString( custom_node, "ScriptFileContents", string() );

        string new_module_name = ScriptMgr.ReadScriptFromMemory( module_name, file_contents );
         CustomGeomMgr.InitGeom( GetID(), new_module_name );

        for ( int i = 0 ; i < (int)m_ParmVec.size() ; i++ )
        {
            m_ParmVec[i]->DecodeXml( custom_node );
        }


    }
    Geom::DecodeXml( node );

    return custom_node;
}
