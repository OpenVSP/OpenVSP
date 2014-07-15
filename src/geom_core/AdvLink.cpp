//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
// AdvLink.cpp: implementation of the AdvLink classes.
//
//////////////////////////////////////////////////////////////////////

#include "AdvLink.h"
#include "AdvLinkMgr.h"
#include "ParmMgr.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VSP_Geom_API.h"
#include "ScriptMgr.h"

//==== Constructor ====//
AdvLink::AdvLink()
{

}

//==== Destructor ====//
AdvLink::~AdvLink()
{

}

void AdvLink::AddParm( const ParmDef & pd, bool input_flag )
{
    if ( input_flag )
        m_InputParms.push_back( pd );
    else
        m_OutputParms.push_back( pd );

}

void AdvLink::SetVar( const string & var_name, double val )
{
    //==== Look For Var ====//
    for ( int i = 0 ; i < (int)m_OutputParms.size() ; i++ )
    {
        if ( m_OutputParms[i].m_VarName == var_name )
        {
            Parm* parm_ptr = ParmMgr.FindParm(  m_OutputParms[i].m_ParmID );
            if ( parm_ptr )
            {
                parm_ptr->Set( val );
                break;
            }
        }
    }
}

double AdvLink::GetVar( const string & var_name )
{
    //==== Look For Var ====//
    for ( int i = 0 ; i < (int)m_InputParms.size() ; i++ )
    {
        if ( m_InputParms[i].m_VarName == var_name )
        {
            Parm* parm_ptr = ParmMgr.FindParm(  m_InputParms[i].m_ParmID );
            if ( parm_ptr )
            {
                return parm_ptr->Get();
            }
        }
    }
    return 0.0;
}

bool AdvLink::UpdateLink( const string & pid )
{
    //==== Check Parm And Update Flag ====//
    Parm* parm_ptr = ParmMgr.FindParm( pid );
    if ( !parm_ptr )
        return false;

    if ( parm_ptr->GetLinkUpdateFlag() == true )
        return false;
 
    //==== Look For Var ====//
    bool run_link = false;
    for ( int i = 0 ; i < (int)m_InputParms.size() ; i++ )
    {
        if ( m_InputParms[i].m_ParmID == pid  )
            run_link = true;
    }

    if ( !run_link )
        return false;

    AdvLinkMgr.SetCurrLink( this );

    //==== Call Script ====//
    ScriptMgr.ExecuteScript( m_ScriptModule.c_str(), "void UpdateLink()" );

    return true;



}