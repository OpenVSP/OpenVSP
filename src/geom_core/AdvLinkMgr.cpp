//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
//// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// AdvLinkMgr.h: Parm Adv Link Mgr Singleton.
//
//////////////////////////////////////////////////////////////////////

#include "AdvLinkMgr.h"
#include "AdvLink.h"
#include "FileUtil.h"
#include "ScriptMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VSP_Geom_API.h"


//==== Constructor ====//
AdvLinkMgrSingleton::AdvLinkMgrSingleton()
{
    m_CurrLink = NULL;
}

void AdvLinkMgrSingleton::Init()
{

}


//==== Scan Custom Directory And Return All Possible Types ====//
void AdvLinkMgrSingleton::ReadAdvLinkScripts()
{
    //==== Only Read Once ====//
    static bool init_flag = false;
    if ( init_flag )
        return;
    init_flag = true;

    vector< string > mod_vec = ScriptMgr.ReadScriptsFromDir( "../../../LinkScripts/" );



}

void AdvLinkMgrSingleton::AddAdvLink( const string & script_module_name )
{
    AdvLink* alink = new AdvLink();
    alink->SetModuleName( script_module_name );
    m_LinkVec.push_back( alink );
    m_CurrLink = alink;

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( script_module_name.c_str(), "void AddVars()" );



}


void AdvLinkMgrSingleton::AddInput( const string & geom_name, int geom_index, const string & parm_name, 
                                    const string & parm_group, const string & var_name )
{
    AddInputOutput( geom_name, geom_index, parm_name, parm_group, var_name, true );

}

void AdvLinkMgrSingleton::AddOutput( const string & geom_name, int geom_index, const string & parm_name, 
                                    const string & parm_group, const string & var_name )
{
    AddInputOutput( geom_name, geom_index, parm_name, parm_group, var_name, false );
}

void AdvLinkMgrSingleton:: AddInputOutput( const string & geom_name, int geom_index, const string & parm_name, 
                   const string & parm_group, const string & var_name, bool input_flag )
{
     //==== Find Geom Ptr ===//
    string geom_id = vsp::FindGeom( geom_name, geom_index );
    string parm_id = vsp::GetParm( geom_id, parm_name, parm_group );

    //==== Find Parm Ptr ===//
    Parm* parm_ptr = ParmMgr.FindParm( parm_id );
    if ( !parm_ptr )
        return;

    ParmDef pd;
    pd.m_ParmID = parm_id;
    pd.m_GeomID = geom_id;
    pd.m_GeomName = geom_name;
    pd.m_GeomIndex = geom_index;
    pd.m_ParmName = parm_name;
    pd.m_GroupName = parm_group;
    pd.m_VarName = var_name;

    if ( m_CurrLink )
        m_CurrLink->AddParm( pd, input_flag );

        
}


void AdvLinkMgrSingleton::SetVar( const string & var_name, double val )
{
    if ( !m_CurrLink )
        return;

    m_CurrLink->SetVar( var_name, val );

}

double AdvLinkMgrSingleton::GetVar( const string & var_name )
{
    if ( !m_CurrLink )
        return 0.0;

    return m_CurrLink->GetVar( var_name );
}


//==== Parm Changed ====//
void AdvLinkMgrSingleton::ParmChanged( const string& pid, bool start_flag  )
{
    //==== Find Parm Ptr ===//
    Parm* parm_ptr = ParmMgr.FindParm( pid );
    if ( !parm_ptr )
    {
        return;
    }

    //==== Check All Links And Update If Needed ====//
    bool updated_flag = false;
    for ( int i = 0 ; i < (int)m_LinkVec.size() ; i++ )
    {
        if ( m_LinkVec[i]->UpdateLink( pid ) )
            updated_flag = true;
    }

    if ( !updated_flag )
        return;

    //==== Reset Updated Flags ====//
    if ( start_flag )
    {
        //for ( int i = 0 ; i < ( int )m_UpdatedParmVec.size() ; i++ )
        //{
        //    Parm* parm_ptr = ParmMgr.FindParm( m_UpdatedParmVec[i] );
        //    if ( parm_ptr )
        //    {
        //        parm_ptr->SetLinkUpdateFlag( false );
        //    }
        //}
        //m_UpdatedParmVec.clear();

        Vehicle* veh = VehicleMgr.GetVehicle();
        if ( veh )
        {
            veh->ParmChanged( parm_ptr, Parm::SET );
        }
    }

}
